/**
 * ============================================================================
 * 17. 视频转码 (音频直接拷贝)
 * ============================================================================
 *
 * 等价命令:
 *   ffmpeg -i input.mp4 -c:v libx264 -c:a copy output.mp4
 *
 * 这是前面所有知识的综合运用:
 *   视频流: demux -> decode -> encode -> mux
 *   音频流: demux -> copy -> mux (不编解码)
 *
 * 关键难点:
 *   1. 视频和音频走不同路径, 需要根据 stream_index 分流
 *   2. 编码后的 packet 时间戳需要正确设置
 *   3. 输出的 time_base 由编码器决定, 需要转换
 */

#include <cstdio>
#include <cstring>
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/opt.h"
#include "libavutil/log.h"
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("用法: %s <输入文件> <输出文件>\n", argv[0]);
        printf("示例: %s input.mp4 output.mp4\n", argv[0]);
        return -1;
    }

    AVFormatContext *in_ctx = nullptr, *out_ctx = nullptr;
    AVCodecContext *dec_ctx = nullptr, *enc_ctx = nullptr;
    int video_in_idx = -1, audio_in_idx = -1;
    int video_out_idx = -1, audio_out_idx = -1;
    int ret = 0;

    // === 打开输入 ===
    ret = avformat_open_input(&in_ctx, argv[1], nullptr, nullptr);
    if (ret < 0) { printf("打开输入失败: %s\n", av_err2str(ret)); return -1; }
    avformat_find_stream_info(in_ctx, nullptr);
    av_dump_format(in_ctx, 0, argv[1], 0);

    video_in_idx = av_find_best_stream(in_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    audio_in_idx = av_find_best_stream(in_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);

    // === 初始化视频解码器 ===
    if (video_in_idx >= 0) {
        dec_ctx = avcodec_alloc_context3(nullptr);
        avcodec_parameters_to_context(dec_ctx, in_ctx->streams[video_in_idx]->codecpar);
        const AVCodec *decoder = avcodec_find_decoder(dec_ctx->codec_id);
        avcodec_open2(dec_ctx, decoder, nullptr);
    }

    // === 初始化视频编码器 ===
    if (video_in_idx >= 0) {
        const AVCodec *encoder = avcodec_find_encoder_by_name("libx264");
        if (!encoder) encoder = avcodec_find_encoder(AV_CODEC_ID_H264);
        if (!encoder) { printf("找不到 H.264 编码器\n"); goto end; }

        enc_ctx = avcodec_alloc_context3(encoder);
        enc_ctx->width = dec_ctx->width;
        enc_ctx->height = dec_ctx->height;
        enc_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
        enc_ctx->time_base = in_ctx->streams[video_in_idx]->time_base;
        enc_ctx->bit_rate = 2000000;
        enc_ctx->gop_size = 50;
        enc_ctx->max_b_frames = 0;

        if (strcmp(encoder->name, "libx264") == 0)
            av_opt_set(enc_ctx->priv_data, "preset", "medium", 0);

        ret = avcodec_open2(enc_ctx, encoder, nullptr);
        if (ret < 0) { printf("打开编码器失败: %s\n", av_err2str(ret)); goto end; }
    }

    // === 创建输出 ===
    ret = avformat_alloc_output_context2(&out_ctx, nullptr, nullptr, argv[2]);
    if (ret < 0) { printf("创建输出失败\n"); goto end; }

    // 视频输出流: 用编码器参数
    if (video_in_idx >= 0) {
        AVStream *out_stream = avformat_new_stream(out_ctx, nullptr);
        avcodec_parameters_from_context(out_stream->codecpar, enc_ctx);
        out_stream->codecpar->codec_tag = 0;
        out_stream->time_base = enc_ctx->time_base;
        video_out_idx = out_stream->index;
    }

    // 音频输出流: 直接拷贝参数
    if (audio_in_idx >= 0) {
        AVStream *out_stream = avformat_new_stream(out_ctx, nullptr);
        avcodec_parameters_copy(out_stream->codecpar, in_ctx->streams[audio_in_idx]->codecpar);
        out_stream->codecpar->codec_tag = 0;
        out_stream->time_base = in_ctx->streams[audio_in_idx]->time_base;
        audio_out_idx = out_stream->index;
    }

    if (!(out_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&out_ctx->pb, argv[2], AVIO_FLAG_WRITE);
        if (ret < 0) { printf("打开输出文件失败\n"); goto end; }
    }
    avformat_write_header(out_ctx, nullptr);

    // === 主循环: 分流处理 ===
    {
    AVPacket pkt;
    AVFrame *frame = av_frame_alloc();
    int frame_count = 0;

    while (av_read_frame(in_ctx, &pkt) >= 0) {
        if (pkt.stream_index == video_in_idx) {
            // 视频: decode -> encode
            avcodec_send_packet(dec_ctx, &pkt);
            while (avcodec_receive_frame(dec_ctx, frame) == 0) {
                frame->pict_type = AV_PICTURE_TYPE_NONE; // 让编码器自己决定帧类型

                avcodec_send_frame(enc_ctx, frame);
                AVPacket enc_pkt;
                memset(&enc_pkt, 0, sizeof(enc_pkt));
                while (avcodec_receive_packet(enc_ctx, &enc_pkt) == 0) {
                    enc_pkt.stream_index = video_out_idx;
                    // 时间基转换: 编码器 -> 输出流
                    av_packet_rescale_ts(&enc_pkt, enc_ctx->time_base,
                                        out_ctx->streams[video_out_idx]->time_base);
                    av_interleaved_write_frame(out_ctx, &enc_pkt);
                    av_packet_unref(&enc_pkt);
                }
                frame_count++;
                if (frame_count % 100 == 0) printf("已转码 %d 帧...\n", frame_count);
            }
        } else if (pkt.stream_index == audio_in_idx) {
            // 音频: 直接拷贝, 只转换时间戳
            pkt.stream_index = audio_out_idx;
            av_packet_rescale_ts(&pkt,
                in_ctx->streams[audio_in_idx]->time_base,
                out_ctx->streams[audio_out_idx]->time_base);
            av_interleaved_write_frame(out_ctx, &pkt);
        }
        av_packet_unref(&pkt);
    }

    // flush 解码器 + 编码器
    if (dec_ctx && enc_ctx) {
        avcodec_send_packet(dec_ctx, nullptr);
        while (avcodec_receive_frame(dec_ctx, frame) == 0) {
            frame->pict_type = AV_PICTURE_TYPE_NONE;
            avcodec_send_frame(enc_ctx, frame);
            AVPacket enc_pkt;
            memset(&enc_pkt, 0, sizeof(enc_pkt));
            while (avcodec_receive_packet(enc_ctx, &enc_pkt) == 0) {
                enc_pkt.stream_index = video_out_idx;
                av_packet_rescale_ts(&enc_pkt, enc_ctx->time_base,
                                    out_ctx->streams[video_out_idx]->time_base);
                av_interleaved_write_frame(out_ctx, &enc_pkt);
                av_packet_unref(&enc_pkt);
            }
            frame_count++;
        }
        avcodec_send_frame(enc_ctx, nullptr);
        AVPacket enc_pkt;
        memset(&enc_pkt, 0, sizeof(enc_pkt));
        while (avcodec_receive_packet(enc_ctx, &enc_pkt) == 0) {
            enc_pkt.stream_index = video_out_idx;
            av_packet_rescale_ts(&enc_pkt, enc_ctx->time_base,
                                out_ctx->streams[video_out_idx]->time_base);
            av_interleaved_write_frame(out_ctx, &enc_pkt);
            av_packet_unref(&enc_pkt);
        }
    }

    printf("转码完成: %d 帧\n", frame_count);
    av_frame_free(&frame);
    }

    av_write_trailer(out_ctx);
    printf("输出: %s\n", argv[2]);

end:
    if (enc_ctx) avcodec_free_context(&enc_ctx);
    if (dec_ctx) avcodec_free_context(&dec_ctx);
    if (out_ctx && !(out_ctx->oformat->flags & AVFMT_NOFILE))
        avio_closep(&out_ctx->pb);
    if (out_ctx) avformat_free_context(out_ctx);
    if (in_ctx) avformat_close_input(&in_ctx);
    return 0;
}
