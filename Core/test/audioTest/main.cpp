//
// Created by Fx Kaze on 25-3-13.
//

#include "private/AudioEncrypto.h"
#include "ImageCrypto.h"
#include "ThreadPool.h"
#include <iostream>

int open_input_file(const char *filename, AVFormatContext **input_format_ctx, AVCodecContext **input_codec_ctx,
                    int *stream_idx) {
    int error = AVERROR_EXIT, i = 0;
    const AVStream *input_stream = nullptr;
    const AVCodec *input_codec = nullptr;
    *stream_idx = -1;

    // *input_format_ctx = avformat_alloc_context();

    error = avformat_open_input(input_format_ctx, filename, nullptr, nullptr);
    if (error < 0) {
        fprintf(stderr, "Could not open input file '%s'. (error: '%s')\n", filename, av_err2str(error));
        goto open_output_file_clean_up;
    }
    error = avformat_find_stream_info(*input_format_ctx, nullptr);
    if (error < 0) {
        fprintf(stderr, "Could not find stream info in input file '%s'. (error: '%s')\n", filename, av_err2str(error));
        goto open_output_file_clean_up;
    }
    for (i = 0; i < (*input_format_ctx)->nb_streams; i++) {
        if ((*input_format_ctx)->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            input_stream = (*input_format_ctx)->streams[i];
            *stream_idx = i;
            break;
        }
    }
    if (!input_stream) {
        fprintf(stderr, "Could not find an audio stream in input file '%s'.\n", filename);
        error = AVERROR_EXIT;
        goto open_output_file_clean_up;
    }
    input_codec = avcodec_find_decoder(input_stream->codecpar->codec_id);
    if (!input_codec) {
        fprintf(stderr, "Could not find an audio codec in input file '%s'.\n", filename);
        error = AVERROR_EXIT;
        goto open_output_file_clean_up;
    }
    *input_codec_ctx = avcodec_alloc_context3(input_codec);
    if (!*input_codec_ctx) {
        fprintf(stderr, "Could not allocate audio codec context.\n");
        error = AVERROR_EXIT;
        goto open_output_file_clean_up;
    }
    error = avcodec_parameters_to_context(*input_codec_ctx, input_stream->codecpar);
    if (error < 0) {
        fprintf(stderr, "Could not apply params to audio codec context.\n");
        goto open_output_file_clean_up;
    }
    error = avcodec_open2(*input_codec_ctx, input_codec, nullptr);
    if (error < 0) {
        fprintf(stderr, "Could not open audio codec context.\n");
        goto open_output_file_clean_up;
    }
    (*input_codec_ctx)->pkt_timebase = input_stream->time_base;
    return 0;
open_output_file_clean_up:
    if (*input_codec_ctx) {
        avcodec_free_context(input_codec_ctx);
        *input_codec_ctx = nullptr;
    }
    if (*input_format_ctx) {
        avformat_close_input(input_format_ctx);
        *input_format_ctx = nullptr;
    }
    return error;
}

int open_output_file(const char *filename, AVFormatContext **output_format_ctx, const AVCodecContext *input_codec_ctx,
                     AVCodecContext **output_codec_ctx) {
    int error = AVERROR_EXIT, n_format = 0;
    AVIOContext *output_io_ctx = nullptr;
    const AVCodec *output_codec = nullptr;
    AVStream *output_stream = nullptr;
    const AVSampleFormat *sample_formats = nullptr;
    error = avio_open(&output_io_ctx, filename, AVIO_FLAG_WRITE);
    if (error < 0) {
        fprintf(stderr, "Could not open output file '%s'. (error: '%s')\n", filename, av_err2str(error));
        goto open_output_file_clean_up;
    }
    *output_format_ctx = avformat_alloc_context();
    if (!*output_format_ctx) {
        fprintf(stderr, "Could not allocate output format context.\n");
        error = AVERROR_EXIT;
        goto open_output_file_clean_up;
    }
    (*output_format_ctx)->pb = output_io_ctx;
    (*output_format_ctx)->oformat = av_guess_format(nullptr, filename, nullptr);
    if (!(*output_format_ctx)->oformat) {
        fprintf(stderr, "Could not guess output format.\n");
        error = AVERROR_EXIT;
        goto open_output_file_clean_up;
    }
    (*output_format_ctx)->url = av_strdup(filename);
    output_codec = avcodec_find_encoder((*output_format_ctx)->oformat->audio_codec);
    if (!output_codec) {
        fprintf(stderr, "Could not find an audio codec in output file '%s'.\n", filename);
        error = AVERROR_EXIT;
        goto open_output_file_clean_up;
    }
    output_stream = avformat_new_stream(*output_format_ctx, nullptr);
    if (!output_stream) {
        fprintf(stderr, "Could not allocate an audio stream in output file '%s'.\n", filename);
        error = AVERROR_EXIT;
        goto open_output_file_clean_up;
    }
    *output_codec_ctx = avcodec_alloc_context3(output_codec);
    if (!*output_codec_ctx) {
        fprintf(stderr, "Could not allocate an output audio codec context.\n");
        error = AVERROR_EXIT;
        goto open_output_file_clean_up;
    }
    av_channel_layout_default(&(*output_codec_ctx)->ch_layout, input_codec_ctx->ch_layout.nb_channels);
    (*output_codec_ctx)->sample_rate = input_codec_ctx->sample_rate;
    error = avcodec_get_supported_config(*output_codec_ctx, output_codec, AV_CODEC_CONFIG_SAMPLE_FORMAT, 0,
                                         reinterpret_cast<const void **>(&sample_formats),
                                         &n_format);
    if (error < 0) {
        fprintf(stderr, "Could not find sample format. (error: '%s')\n", av_err2str(error));
        goto open_output_file_clean_up;
    }
    if (n_format < 1) {
        fprintf(stderr, "Could not find sample format.\n");
        error = AVERROR_EXIT;
        goto open_output_file_clean_up;
    }
    (*output_codec_ctx)->sample_fmt = sample_formats[0];
    (*output_codec_ctx)->bit_rate = input_codec_ctx->bit_rate;
    output_stream->time_base.den = input_codec_ctx->sample_rate;
    output_stream->time_base.num = 1;
    if ((*output_format_ctx)->oformat->flags & AVFMT_GLOBALHEADER) {
        (*output_codec_ctx)->flags2 |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    error = avcodec_open2(*output_codec_ctx, output_codec, nullptr);
    if ((*output_codec_ctx)->frame_size == 0)
        (*output_codec_ctx)->frame_size = input_codec_ctx->frame_size > 0 ? input_codec_ctx->frame_size : 1152;
    if (error < 0) {
        fprintf(stderr, "Could not open an audio codec. (error: '%s')\n", av_err2str(error));
        goto open_output_file_clean_up;
    }
    error = avcodec_parameters_from_context(output_stream->codecpar, *output_codec_ctx);
    if (error < 0) {
        fprintf(stderr, "Could not get params from context. (error: '%s')\n", av_err2str(error));
        goto open_output_file_clean_up;
    }
    return 0;
open_output_file_clean_up:
    if (output_io_ctx)
        avio_close(output_io_ctx);
    if (*output_format_ctx) {
        avformat_free_context(*output_format_ctx);
        *output_format_ctx = nullptr;
    }
    if (*output_codec_ctx) {
        avcodec_free_context(output_codec_ctx);
        *output_codec_ctx = nullptr;
    }
    return error;
}

int read_decode_cvt_store(AVAudioFifo *fifo, AVFormatContext *input_format_ctx, AVCodecContext *input_codec_ctx,
                          const AVCodecContext *output_codec_ctx, SwrContext *swr_ctx, int *finished,
                          const int stream_idx, const int cvt = 1) {
    int error = AVERROR_EXIT, n_out = 0;
    AVFrame *input_frame = nullptr;
    AVPacket *input_pkt = nullptr;
    u8 **converted_input_samples = nullptr;
    *finished = 0;
    input_frame = av_frame_alloc();
    if (!input_frame) {
        fprintf(stderr, "Could not allocate input frame.\n");
        error = AVERROR_EXIT;
        goto read_decode_cvt_store_clean_up;
    }
    input_pkt = av_packet_alloc();
    if (!input_pkt) {
        fprintf(stderr, "Could not allocate input packet.\n");
        error = AVERROR_EXIT;
        goto read_decode_cvt_store_clean_up;
    }
    error = av_read_frame(input_format_ctx, input_pkt);
    if (error < 0) {
        if (error == AVERROR_EOF) {
            *finished = 1;
            error = 0;
            goto read_decode_cvt_store_clean_up;
        }
        fprintf(stderr, "Could not read an audio frame. (error: '%s')\n", av_err2str(error));
        goto read_decode_cvt_store_clean_up;
    }
    if (input_pkt->stream_index != stream_idx) {
        error = 0;
        goto read_decode_cvt_store_clean_up;
    }
    error = avcodec_send_packet(input_codec_ctx, input_pkt);
    if (error < 0) {
        fprintf(stderr, "Could not send a packet to decoder. (error: '%s')\n", av_err2str(error));
        goto read_decode_cvt_store_clean_up;
    }
    error = avcodec_receive_frame(input_codec_ctx, input_frame);
    if (error < 0) {
        if (error == AVERROR(EAGAIN)) {
            error = 0;
            goto read_decode_cvt_store_clean_up;
        }
        av_assert0(error != AVERROR_EOF);
        // if (error == AVERROR_EOF) {
        //     // error = 0;
        //     *finished = 1;
        //     error = 0;
        //     goto read_decode_cvt_store_clean_up;
        // }
        fprintf(stderr, "Could not receive an audio frame from decoder. (error: '%s')\n", av_err2str(error));
        goto read_decode_cvt_store_clean_up;
    }
    if (cvt) {
        error = av_samples_alloc_array_and_samples(
            &converted_input_samples, nullptr,
            output_codec_ctx->ch_layout.nb_channels,
            input_frame->nb_samples,
            output_codec_ctx->sample_fmt, 0);
        if (error < 0) {
            fprintf(stderr, "Could not alloc input samples. (error: '%s')\n", av_err2str(error));
            goto read_decode_cvt_store_clean_up;
        }
        error = swr_convert(
            swr_ctx,
            converted_input_samples,
            input_frame->nb_samples,
            input_frame->data,
            input_frame->nb_samples
        );
        if (error < 0) {
            fprintf(stderr, "Could not convert input samples. (error: '%s')\n", av_err2str(error));
            goto read_decode_cvt_store_clean_up;
        }
        n_out = error;
    } else {
        converted_input_samples = input_frame->data;
        n_out = input_frame->nb_samples;
    }
    error = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + n_out);
    if (error != 0) {
        fprintf(stderr, "Could not realloc fifo with size '%d'. (error: '%s')\n",
                av_audio_fifo_size(fifo) + n_out, av_err2str(error));
        goto read_decode_cvt_store_clean_up;
    }
    if (av_audio_fifo_write(
            fifo,
            reinterpret_cast<void **>(converted_input_samples),
            n_out) < n_out) {
        fprintf(stderr, "Could not write to FIFO.\n");
        error = AVERROR_EXIT;
        goto read_decode_cvt_store_clean_up;
    }
    error = 0;
read_decode_cvt_store_clean_up:
    if (cvt && converted_input_samples) {
        av_freep(converted_input_samples);
    }
    if (input_frame) {
        av_frame_free(&input_frame);
    }
    if (input_pkt) {
        av_packet_free(&input_pkt);
    }
    return error;
}

int flush_audio_frame(AVFormatContext *output_format_ctx, AVCodecContext *output_codec_ctx,
                      int *data_writen) {
    int error = AVERROR_EXIT;
    AVPacket *out_pkt = nullptr;

    out_pkt = av_packet_alloc();
    *data_writen = 0;
    if (!out_pkt) {
        fprintf(stderr, "Could not allocate output packet.\n");
        error = AVERROR_EXIT;
        goto encode_audio_frame_clean_up;
    }
    error = avcodec_send_frame(output_codec_ctx, nullptr);
    if (error < 0) {
        if (error == AVERROR_EOF) {
            error = 0;
            goto encode_audio_frame_clean_up;
        }
        fprintf(stderr, "Could not send frame to output encoder. (error: '%s')\n", av_err2str(error));
        goto encode_audio_frame_clean_up;
    }
    // *data_exists = 0;
    do {
        error = avcodec_receive_packet(output_codec_ctx, out_pkt);
        if (error < 0) {
            if (error == AVERROR(EAGAIN)) {
                error = 0;
                goto encode_audio_frame_clean_up;
            }
            if (error == AVERROR_EOF) {
                error = 0;
                *data_writen = 1;
                goto encode_audio_frame_clean_up;
            }
            fprintf(stderr, "Could not encode frame. (error: '%s')\n", av_err2str(error));
            goto encode_audio_frame_clean_up;
        }
        error = av_write_frame(output_format_ctx, out_pkt);
        if (error < 0) {
            fprintf(stderr, "Could not write output packet. (error: '%s')\n", av_err2str(error));
            goto encode_audio_frame_clean_up;
        }
    } while (true);
encode_audio_frame_clean_up:
    if (out_pkt) {
        av_packet_free(&out_pkt);
    }
    return error;
}

int load_encode_write(AVAudioFifo *fifo, AVFormatContext *output_format_ctx,
                      AVCodecContext *output_codec_ctx, i64 *global_pts) {
    int error = AVERROR_EXIT, frame_size = 0;
    AVFrame *output_frame = nullptr;
    AVPacket *out_pkt = nullptr;

    frame_size = FFMIN(av_audio_fifo_size(fifo), output_codec_ctx->frame_size);
    output_frame = av_frame_alloc();
    if (!output_frame) {
        fprintf(stderr, "Could not allocate output frame.\n");
        error = AVERROR_EXIT;
        goto load_encode_and_write_clean_up;
    }
    output_frame->nb_samples = frame_size;
    output_frame->format = output_codec_ctx->sample_fmt;
    output_frame->sample_rate = output_codec_ctx->sample_rate;
    error = av_channel_layout_copy(&output_frame->ch_layout, &output_codec_ctx->ch_layout);
    if (error < 0) {
        fprintf(stderr, "Could not copy output channel layout. (error: '%s')\n", av_err2str(error));
        goto load_encode_and_write_clean_up;
    }
    error = av_frame_get_buffer(output_frame, 0);
    if (error < 0) {
        fprintf(stderr, "Could not get output frame buffer. (error: '%s')\n", av_err2str(error));
        goto load_encode_and_write_clean_up;
    }
    if (av_audio_fifo_read(fifo,
                           reinterpret_cast<void * const *>(output_frame->data),
                           frame_size) < frame_size) {
        printf("Could not read output frame from FIFO.\n");
        error = AVERROR_EXIT;
        goto load_encode_and_write_clean_up;
    }
    out_pkt = av_packet_alloc();
    if (!out_pkt) {
        fprintf(stderr, "Could not allocate output packet.\n");
        error = AVERROR_EXIT;
        goto load_encode_and_write_clean_up;
    }
    output_frame->pts = *global_pts;
    *global_pts += output_frame->nb_samples;
    error = avcodec_send_frame(output_codec_ctx, output_frame);
    if (error < 0) {
        fprintf(stderr, "Could not send packet for encoding. (error: '%s')\n", av_err2str(error));
        goto load_encode_and_write_clean_up;
    }
    error = avcodec_receive_packet(output_codec_ctx, out_pkt);
    if (error < 0) {
        if (error == AVERROR(EAGAIN)) {
            error = 0;
            goto load_encode_and_write_clean_up;
        }
        if (error == AVERROR_EOF) {
            error = 0;
            goto load_encode_and_write_clean_up;
        }
        fprintf(stderr, "Could not encode frame. (error: '%s')\n", av_err2str(error));
        goto load_encode_and_write_clean_up;
    }
    error = av_write_frame(output_format_ctx, out_pkt);
    if (error < 0) {
        fprintf(stderr, "Could not write output packet. (error: '%s')\n", av_err2str(error));
        goto load_encode_and_write_clean_up;
    }
    error = 0;
load_encode_and_write_clean_up:
    if (output_frame) {
        av_frame_free(&output_frame);
    }
    if (out_pkt) {
        av_packet_free(&out_pkt);
    }
    return error;
}

int load_crypto_store(AVAudioFifo *fifo_in, AVAudioFifo *fifo_out, SwrContext *swr_ctx, ImageCrypto *crypto,
                      bool encrypt, int crypto_block_size, const int size, const int sample_size,
                      AVSampleFormat out_fmt,
                      const int n_channels) {
    const int read_size = FFMIN(av_audio_fifo_size(fifo_in), crypto_block_size);
    const int byte_size_per_ch = crypto_block_size * sample_size;
    const int byte_size_total = byte_size_per_ch * n_channels;
    int i = 0, error = AVERROR_EXIT;

    u8 **crypto_ptrs = nullptr;
    u8 *crypto_data = nullptr, *crypto_data_out = nullptr;
    u8 **converted_input_samples = nullptr;
    crypto_ptrs = new u8 *[n_channels];
    if (!crypto_ptrs) {
        fprintf(stderr, "Could not allocate crypto_ptrs.\n");
        error = AVERROR_EXIT;
        goto load_crypto_store_clean_up;
    }
    crypto_data = new u8[byte_size_total];
    if (!crypto_data) {
        fprintf(stderr, "Could not allocate crypto_data.\n");
        error = AVERROR_EXIT;
        goto load_crypto_store_clean_up;
    }
    memset(crypto_data, 0, byte_size_total);
    for (i = 0; i < n_channels; i++) {
        crypto_ptrs[i] = &crypto_data[i * byte_size_per_ch];
    }
    if (av_audio_fifo_read(fifo_in, reinterpret_cast<void * const *>(crypto_ptrs), read_size) < read_size) {
        fprintf(stderr, "Could not read input data from fifo.\n");
        error = AVERROR_EXIT;
        goto load_crypto_store_clean_up;
    }
    crypto_data_out = static_cast<u8 *>(encrypt
                                            ? crypto->encrypt(crypto_data, size * sample_size, size, n_channels)
                                            : crypto->decrypt(crypto_data, size * sample_size, size, n_channels));
    if (!crypto_data_out) {
        fprintf(stderr, "Could not decrypt crypto_data.\n");
        error = AVERROR_EXIT;
        goto load_crypto_store_clean_up;
    }
    for (i = 0; i < n_channels; i++) {
        crypto_ptrs[i] = &crypto_data_out[i * byte_size_per_ch];
    }
    if (!encrypt) {
        error = av_samples_alloc_array_and_samples(
            &converted_input_samples, nullptr,
            n_channels,
            crypto_block_size,
            out_fmt, 0);
        if (error < 0) {
            fprintf(stderr, "Could not alloc output samples. (error: '%s')\n", av_err2str(error));
            goto load_crypto_store_clean_up;
        }
        error = swr_convert(
            swr_ctx,
            converted_input_samples,
            crypto_block_size,
            crypto_ptrs,
            crypto_block_size
        );
        if (error < 0) {
            fprintf(stderr, "Could not convert output samples. (error: '%s')\n", av_err2str(error));
            goto load_crypto_store_clean_up;
        }
        crypto_block_size = error;
    } else {
        converted_input_samples = crypto_ptrs;
    }
    error = av_audio_fifo_realloc(fifo_out, av_audio_fifo_size(fifo_out) + crypto_block_size);
    if (error != 0) {
        fprintf(stderr, "Could not re-allocate output fifo. (error: '%s')\n", av_err2str(error));
        goto load_crypto_store_clean_up;
    }
    if (av_audio_fifo_write(fifo_out, reinterpret_cast<void * const *>(converted_input_samples), crypto_block_size) <
        crypto_block_size) {
        fprintf(stderr, "Could not write output data to fifo.\n");
        error = AVERROR_EXIT;
        goto load_crypto_store_clean_up;
    }
    error = 0;
load_crypto_store_clean_up:
    if (crypto_ptrs) {
        delete[] crypto_ptrs;
    }
    if (crypto_data) {
        delete[] crypto_data;
    }
    if (crypto_data_out) {
        delete[] crypto_data_out;
    }
    if (!encrypt && converted_input_samples) {
        av_freep(converted_input_samples);
    }
    return error;
}

int audio_crypto(const char *input_filename, const char *output_filename, ImageCrypto *crypto, const int size,
                 const bool encrypt) {
    int error = AVERROR_EXIT, stream_idx = -1, finished = 0, output_size = 0, n_channels = 0,
            crypto_block_size = size * size;
    i64 global_pts = 0;
    AVFormatContext *input_format_ctx = nullptr, *output_format_ctx = nullptr;
    AVCodecContext *input_codec_ctx = nullptr, *output_codec_ctx = nullptr;
    SwrContext *swr_ctx = nullptr;
    AVAudioFifo *fifo_1 = nullptr;
    AVAudioFifo *fifo_2 = nullptr;

    if (open_input_file(input_filename, &input_format_ctx,
                        &input_codec_ctx, &stream_idx))
        goto cleanup;
    if (open_output_file(output_filename,
                         &output_format_ctx, input_codec_ctx, &output_codec_ctx))
        goto cleanup;
    // 分配重采样器（SwrContext）并初始化参数 <---3--->
    n_channels = input_codec_ctx->ch_layout.nb_channels;
    // 确认采样率要一样，否则处理方式不一样
    av_assert0(output_codec_ctx->sample_rate == input_codec_ctx->sample_rate);
    error = swr_alloc_set_opts2(
        &swr_ctx,
        &output_codec_ctx->ch_layout, output_codec_ctx->sample_fmt,
        output_codec_ctx->sample_rate,
        &input_codec_ctx->ch_layout, input_codec_ctx->sample_fmt,
        input_codec_ctx->sample_rate,
        0, nullptr);
    if (error < 0) {
        fprintf(stderr, "Could not set input to crypto swr options. (error: '%s')\n", av_err2str(error));
        goto cleanup;
    }
    //打开重采样器，调用 swr_init 之前必须用 swr_alloc_set_opts2 设置参数
    error = swr_init(swr_ctx);
    if (error < 0) {
        fprintf(stderr, "Could not init input to crypto swr context. (error: '%s')\n", av_err2str(error));
        goto cleanup;
    }
    // 根据指定的输出样本格式创建FIFO缓冲区 <---4--->
    output_size = output_codec_ctx->frame_size;
    if (encrypt) {
        fifo_1 = av_audio_fifo_alloc(output_codec_ctx->sample_fmt, n_channels,
                                     crypto_block_size);
        fifo_2 = av_audio_fifo_alloc(output_codec_ctx->sample_fmt, n_channels,
                                     output_size);
    } else {
        fifo_1 = av_audio_fifo_alloc(input_codec_ctx->sample_fmt, n_channels,
                                     crypto_block_size);
        fifo_2 = av_audio_fifo_alloc(output_codec_ctx->sample_fmt, n_channels,
                                     output_size);
    }
    if (!fifo_1) {
        fprintf(stderr, "Could not allocate input to crypto audio fifo.\n");
        goto cleanup;
    }
    if (!fifo_2) {
        fprintf(stderr, "Could not allocate crypto to output audio fifo.\n");
        goto cleanup;
    }
    // 写入输出文件头 <---5--->
    error = avformat_write_header(output_format_ctx, nullptr);
    if (error < 0) {
        fprintf(stderr, "Could not write output file header. (error: '%s')\n",
                av_err2str(error));
        goto cleanup;
    }
    // 只要有输入样本要读取或输出样本要写入，就会循环;一旦都没有，就中止
    while (true) {
        while (av_audio_fifo_size(fifo_1) < crypto_block_size) {
            // 解码一帧音频样本，将其转换为输出样本格式并将其放入FIFO缓冲区
            if (read_decode_cvt_store(fifo_1, input_format_ctx, input_codec_ctx,
                                      output_codec_ctx, swr_ctx, &finished, stream_idx, encrypt)) {
                goto cleanup;
            }
            if (finished)
                break;
        }
        if (av_audio_fifo_size(fifo_1) >= crypto_block_size || (
                finished && av_audio_fifo_size(fifo_1) > 0)) {
            if (load_crypto_store(fifo_1, fifo_2, swr_ctx, crypto, encrypt, crypto_block_size,
                                  size,
                                  av_get_bytes_per_sample(
                                      encrypt ? output_codec_ctx->sample_fmt : input_codec_ctx->sample_fmt),
                                  output_codec_ctx->sample_fmt,
                                  n_channels))
                goto cleanup;
        }
        while (av_audio_fifo_size(fifo_2) >= output_size ||
               (finished && av_audio_fifo_size(fifo_2) > 0)) {
            // 从FIFO缓冲区中获取一帧的音频样本，对其进行编码并将其写入输出文件
            if (load_encode_write(fifo_2, output_format_ctx, output_codec_ctx, &global_pts))
                goto cleanup;
        }
        /**
        * 如果我们在输入文件的末尾并已对所有剩余样本进行编码，则可以退出此循环并完成
        **/
        if (finished) {
            int data_written = 0;
            // 刷新编码器，因为它可能有延迟帧
            do {
                flush_audio_frame(output_format_ctx, output_codec_ctx, &data_written);
            } while (!data_written);
            break;
        }
    }
    // 写入输出文件容器的尾部 <---11--->
    error = av_write_trailer(output_format_ctx);
    if (error < 0)
        goto cleanup;
    error = 0;
cleanup:
    // 释放资源 <--- 12--->
    if (fifo_1)
        av_audio_fifo_free(fifo_1);
    if (fifo_2)
        av_audio_fifo_free(fifo_2);
    if (swr_ctx)
        swr_free(&swr_ctx);
    if (output_codec_ctx)
        avcodec_free_context(&output_codec_ctx);
    if (output_format_ctx) {
        avio_closep(&output_format_ctx->pb);
        avformat_free_context(output_format_ctx);
    }
    if (input_codec_ctx)
        avcodec_free_context(&input_codec_ctx);
    if (input_format_ctx)
        avformat_close_input(&input_format_ctx);
    return error;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <audio_file> <audio_out_file>\n";
        return -1;
    }

    ThreadPool threadPool;
    ImageCrypto crypto(threadPool,
                       {
                           1,
                           1,
                           1,
                           1,
                           1,
                           2,
                           false
                       },
                       {
                           0x0,
                           {0.1, 0.15},
                           {0.1, 0.15}
                       }
    );

    audio_crypto(
        argv[1], argv[2],
        &crypto,
        64,
        true
    );
    return audio_crypto(
        argv[2], argv[3],
        &crypto,
        64,
        false
    );

    // av_log_set_level(AV_LOG_DEBUG);

    int error = AVERROR_EXIT, stream_idx;
    i64 global_pts = 0;
    AVFormatContext *input_format_ctx = nullptr, *output_format_ctx = nullptr;
    AVCodecContext *input_codec_ctx = nullptr, *output_codec_ctx = nullptr;
    SwrContext *swr_ctx = nullptr;
    AVAudioFifo *fifo = nullptr;
    const char *input_filename = argv[1], *output_filename = argv[2];

    // 打开输入文件 <---1--->
    if (open_input_file(input_filename, &input_format_ctx,
                        &input_codec_ctx, &stream_idx))
        goto cleanup;
    // 打开输出文件 <---2--->
    if (open_output_file(output_filename,
                         &output_format_ctx, input_codec_ctx, &output_codec_ctx))
        goto cleanup;
    // 分配重采样器（SwrContext）并初始化参数 <---3--->
    error = swr_alloc_set_opts2(&swr_ctx,
                                &output_codec_ctx->ch_layout, output_codec_ctx->sample_fmt,
                                output_codec_ctx->sample_rate,
                                &input_codec_ctx->ch_layout, input_codec_ctx->sample_fmt,
                                input_codec_ctx->sample_rate,
                                0, nullptr);
    if (error < 0) {
        fprintf(stderr, "Could not set swr options. (error: '%s')\n", av_err2str(error));
        goto cleanup;
    }
    // 确认采样率要一样，否则处理方式不一样
    av_assert0(output_codec_ctx->sample_rate == input_codec_ctx->sample_rate);
    //打开重采样器，调用 swr_init 之前必须用 swr_alloc_set_opts2 设置参数
    error = swr_init(swr_ctx);
    if (error < 0) {
        fprintf(stderr, "Could not open resample context\n");
        goto cleanup;
    }
    // 根据指定的输出样本格式创建FIFO缓冲区 <---4--->
    fifo = av_audio_fifo_alloc(output_codec_ctx->sample_fmt, output_codec_ctx->ch_layout.nb_channels, 1);
    if (!fifo) {
        fprintf(stderr, "Could not allocate audio fifo.\n");
        goto cleanup;
    }
    // 写入输出文件头 <---5--->
    error = avformat_write_header(output_format_ctx, nullptr);
    if (error < 0) {
        fprintf(stderr, "Could not write output file header. (error: '%s')\n",
                av_err2str(error));
        goto cleanup;
    }
    // 只要有输入样本要读取或输出样本要写入，就会循环;一旦都没有，就中止
    while (true) {
        // 使用编码器所需的帧大小进行处理
        const int output_frame_size = output_codec_ctx->frame_size;
        int finished = 0;
        /**
        * 确保FIFO缓冲区中至少有一帧（Frame），以便编码器可以工作。
        * 由于解码器和编码器的帧大小(Frame Size)可能不同，我们需要 FIFO 缓冲区来存储尽可能多的输入帧，至少有一个输出帧可用于编码器。
        **/
        while (av_audio_fifo_size(fifo) < output_frame_size) {
            // 解码一帧音频样本，将其转换为输出样本格式并将其放入FIFO缓冲区
            if (read_decode_cvt_store(fifo, input_format_ctx, input_codec_ctx,
                                      output_codec_ctx, swr_ctx, &finished, stream_idx)) {
                goto cleanup;
            }
            if (finished)
                break;
        }
        /**
        * 如果对于编码器来说有足够的样本，就会对它们进行编码，在文件的末尾，将剩余的样本传递给编码器
        **/
        while (av_audio_fifo_size(fifo) >= output_frame_size ||
               (finished && av_audio_fifo_size(fifo) > 0)) {
            // 从FIFO缓冲区中获取一帧的音频样本，对其进行编码并将其写入输出文件
            if (load_encode_write(fifo, output_format_ctx, output_codec_ctx, &global_pts))
                goto cleanup;
        }
        /**
        * 如果我们在输入文件的末尾并已对所有剩余样本进行编码，则可以退出此循环并完成
        **/
        if (finished) {
            int data_written = 0;
            // 刷新编码器，因为它可能有延迟帧
            do {
                flush_audio_frame(output_format_ctx, output_codec_ctx, &data_written);
            } while (!data_written);
            break;
        }
    }
    // 写入输出文件容器的尾部 <---11--->
    error = av_write_trailer(output_format_ctx);
    if (error < 0)
        goto cleanup;
    error = 0;
cleanup:
    // 释放资源 <--- 12--->
    if (fifo)
        av_audio_fifo_free(fifo);
    if (swr_ctx)
        swr_free(&swr_ctx);
    if (output_codec_ctx)
        avcodec_free_context(&output_codec_ctx);
    if (output_format_ctx) {
        avio_closep(&output_format_ctx->pb);
        avformat_free_context(output_format_ctx);
    }
    if (input_codec_ctx)
        avcodec_free_context(&input_codec_ctx);
    if (input_format_ctx)
        avformat_close_input(&input_format_ctx);
    return error;
}
