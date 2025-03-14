//
// Created by Fx Kaze on 25-3-13.
//

#include "private/AudioEncrypto.h"
#include "ImageCrypto.h"
#include "ThreadPool.h"
#include <iostream>

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <audio_file> <audio_out_file>\n";
        return -1;
    }

    ThreadPool threadPool;
    ImageCrypto crypto(threadPool);

    // avformat_network_init(); // 初始化 FFmpeg 网络功能（如果需要读取网络流）

    AVFormatContext* format_ctx = nullptr;
    // format_ctx -> codec_params
    AVCodecParameters* codec_params = nullptr;
    // codec_params -> codec / codec_ctx
    const AVCodec* codec = nullptr;
    AVCodecContext* codec_ctx = nullptr;

    AVFrame *frame = nullptr, *converted_frame = nullptr;


    AVFormatContext* out_fmt_ctx = nullptr;
    AVStream* out_audio_stream = nullptr;
    AVCodecContext* out_codec_ctx = nullptr;
    const AVCodec* out_codec = nullptr;

    // OPEN INPUT
    // FormatCtx
    if (avformat_open_input(&format_ctx, argv[1], nullptr, nullptr) != 0)
    {
        std::cerr << "Failed to open audio file!\n";
        return -1;
    }

    if (avformat_find_stream_info(format_ctx, nullptr) < 0)
    {
        std::cerr << "Could not find stream info!\n";
        avformat_close_input(&format_ctx);
        return -1;
    }

    // 查找音频流
    int audioStreamIndex = -1;
    for (int i = 0; i < format_ctx->nb_streams; i++)
    {
        if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audioStreamIndex = i;
            break;
        }
    }

    if (audioStreamIndex == -1)
    {
        std::cerr << "No audio stream found!\n";
        avformat_close_input(&format_ctx);
        return -1;
    }

    // CodecPar
    codec_params = format_ctx->streams[audioStreamIndex]->codecpar;
    // Codec
    codec = avcodec_find_decoder(codec_params->codec_id);
    if (!codec)
    {
        std::cerr << "Unsupported codec!\n";
        avformat_close_input(&format_ctx);
        return -1;
    }

    // CodecCtx
    codec_ctx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codec_ctx, codec_params);

    if (avcodec_open2(codec_ctx, codec, nullptr) < 0)
    {
        std::cerr << "Could not open codec!\n";
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&format_ctx);
        return -1;
    }

    codec_ctx->pkt_timebase = format_ctx->streams[audioStreamIndex]->time_base;

    std::cout << "Audio Format: " << codec->name
        << "Sample Size: " << av_get_bytes_per_sample(codec_ctx->sample_fmt)
        // must be 4
        << ", Sample Rate: " << codec_ctx->sample_rate
        << ", Channels: " << codec_ctx->ch_layout.nb_channels << "\n";

    // OPEN OUTPUT
    // 1. 初始化输出格式
    avformat_alloc_output_context2(&out_fmt_ctx, nullptr, nullptr, argv[2]);
    if (!out_fmt_ctx)
    {
        fprintf(stderr, "Could not create output context\n");
        return -1;
    }

    // 2. 查找编码器，设置音频流参数
    out_codec = avcodec_find_encoder(out_fmt_ctx->oformat->audio_codec);
    if (!out_codec)
    {
        fprintf(stderr, "Could not find suitable codec\n");
        return -1;
    }

    // 创建音频流
    out_audio_stream = avformat_new_stream(out_fmt_ctx, nullptr);
    if (!out_audio_stream)
    {
        fprintf(stderr, "Failed to create audio stream\n");
        return -1;
    }

    // 设置编码器参数
    out_codec_ctx = avcodec_alloc_context3(out_codec);
    out_codec_ctx->pkt_timebase = codec_ctx->pkt_timebase;
    // out_codec_ctx->sample_rate = 44100;
    out_codec_ctx->sample_rate = codec_ctx->sample_rate;
    out_codec_ctx->channel_layout = AV_CH_LAYOUT_STEREO;
    out_codec_ctx->channels = av_get_channel_layout_nb_channels(out_codec_ctx->channel_layout);
    av_channel_layout_default(&out_codec_ctx->ch_layout, out_codec_ctx->ch_layout.nb_channels);
    out_codec_ctx->bit_rate = 128000;
    out_codec_ctx->sample_fmt = out_codec->sample_fmts ? out_codec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP; // 选择兼容格式

    // 打开编码器
    if (avcodec_open2(out_codec_ctx, out_codec, nullptr) < 0)
    {
        fprintf(stderr, "Failed to open codec\n");
        return -1;
    }

    // ✅ 绑定 `AVCodecContext` 参数到 `AVStream`
    avcodec_parameters_from_context(out_audio_stream->codecpar, out_codec_ctx);

    // 3. 打开输出文件
    if (!(out_fmt_ctx->flags & AVFMT_NOFILE))
    {
        if (avio_open(&out_fmt_ctx->pb, argv[2], AVIO_FLAG_WRITE) < 0)
        {
            fprintf(stderr, "Failed to open output file\n");
            return -1;
        }
    }

    // 4. 写文件头
    if (avformat_write_header(out_fmt_ctx, nullptr) < 0)
    {
        fprintf(stderr, "Failed to write header\n");
        return -1;
    }

    AVPacket packet, out_packet;
    av_init_packet(&packet);
    av_init_packet(&out_packet);
    frame = av_frame_alloc();

    SwrContext* swr_ctx = swr_alloc();
    swr_alloc_set_opts2(
        &swr_ctx,
        &out_codec_ctx->ch_layout, // 输出格式
        out_codec_ctx->sample_fmt, // MP3 需要的 sample_fmt
        out_codec_ctx->sample_rate,
        &codec_ctx->ch_layout, // 输入格式
        codec_ctx->sample_fmt,
        codec_ctx->sample_rate,
        0, nullptr);
    swr_init(swr_ctx);

    while (av_read_frame(format_ctx, &packet) >= 0)
    {
        if (packet.stream_index == audioStreamIndex)
        {
            if (avcodec_send_packet(codec_ctx, &packet) == 0)
            {
                while (avcodec_receive_frame(codec_ctx, frame) == 0)
                {
                    std::cout << "Decoded frame with " << frame->nb_samples << " samples.\n";
                    converted_frame = av_frame_alloc();
                    converted_frame->format = out_codec_ctx->sample_fmt;
                    converted_frame->ch_layout = out_codec_ctx->ch_layout;
                    converted_frame->sample_rate = out_codec_ctx->sample_rate;
                    converted_frame->nb_samples = frame->nb_samples;
                    av_frame_get_buffer(converted_frame, 0);
                    swr_convert(swr_ctx, converted_frame->data, converted_frame->nb_samples,
                                const_cast<const uint8_t**>(frame->data), frame->nb_samples);
                    printf("%d\n", converted_frame->nb_samples);
                    converted_frame->pts = frame->pts;
                    avcodec_send_frame(out_codec_ctx, converted_frame);
                    av_frame_free(&converted_frame);
                }
                avcodec_receive_packet(out_codec_ctx, &out_packet);
                av_interleaved_write_frame(out_fmt_ctx, &out_packet);
                av_packet_unref(&out_packet);
            }
        }
        av_packet_unref(&packet);
    }

    swr_free(&swr_ctx);

    av_frame_free(&frame);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&format_ctx);


    av_write_trailer(out_fmt_ctx);
    avcodec_free_context(&out_codec_ctx);
    avio_closep(&out_fmt_ctx->pb);
    avformat_free_context(out_fmt_ctx);
    return 0;
}
