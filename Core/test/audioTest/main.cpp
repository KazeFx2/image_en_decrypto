//
// Created by Fx Kaze on 25-3-13.
//

#include "private/AudioEncrypto.h"
#include <iostream>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <audio_file>\n";
        return -1;
    }

    avformat_network_init(); // 初始化 FFmpeg 网络功能（如果需要读取网络流）

    AVFormatContext *formatCtx = nullptr;
    if (avformat_open_input(&formatCtx, argv[1], nullptr, nullptr) != 0) {
        std::cerr << "Failed to open audio file!\n";
        return -1;
    }

    if (avformat_find_stream_info(formatCtx, nullptr) < 0) {
        std::cerr << "Could not find stream info!\n";
        avformat_close_input(&formatCtx);
        return -1;
    }

    // 查找音频流
    int audioStreamIndex = -1;
    for (unsigned int i = 0; i < formatCtx->nb_streams; i++) {
        if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndex = i;
            break;
        }
    }

    if (audioStreamIndex == -1) {
        std::cerr << "No audio stream found!\n";
        avformat_close_input(&formatCtx);
        return -1;
    }

    AVCodecParameters *codecParams = formatCtx->streams[audioStreamIndex]->codecpar;
    const AVCodec *codec = avcodec_find_decoder(codecParams->codec_id);
    if (!codec) {
        std::cerr << "Unsupported codec!\n";
        avformat_close_input(&formatCtx);
        return -1;
    }

    AVCodecContext *codecCtx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codecCtx, codecParams);

    if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
        std::cerr << "Could not open codec!\n";
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        return -1;
    }

    codecCtx->pkt_timebase = formatCtx->streams[audioStreamIndex]->time_base;

    std::cout << "Audio Format: " << codec->name
            << "Sample Size: " << av_get_bytes_per_sample(codecCtx->sample_fmt)
    // must be 4
            << ", Sample Rate: " << codecCtx->sample_rate
            << ", Channels: " << codecCtx->ch_layout.nb_channels << "\n";

    AVPacket packet;
    av_init_packet(&packet);
    AVFrame *frame = av_frame_alloc();

    while (av_read_frame(formatCtx, &packet) >= 0) {
        if (packet.stream_index == audioStreamIndex) {
            if (avcodec_send_packet(codecCtx, &packet) == 0) {
                while (avcodec_receive_frame(codecCtx, frame) == 0) {
                    std::cout << "Decoded frame with " << frame->nb_samples << " samples.\n";
                }
            }
        }
        av_packet_unref(&packet);
    }

    av_frame_free(&frame);
    avcodec_free_context(&codecCtx);
    avformat_close_input(&formatCtx);
    return 0;
}
