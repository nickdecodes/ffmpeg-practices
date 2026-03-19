/**
 * ============================================================================
 * 26. 录屏: 采集屏幕并编码保存
 * ============================================================================
 *
 * 等价命令 (macOS):
 *   ffmpeg -f avfoundation -framerate 30 -i "1:none" -t 10 -c:v libx264 screen.mp4
 *
 * 新增 API:
 *   avdevice_register_all() - 注册设备输入格式
 *   av_find_input_format()  - 查找设备格式 (avfoundation/x11grab)
 *
 * 设备采集和文件读取的区别:
 *   文件: avformat_open_input(ctx, "file.mp4", nullptr, nullptr)
 *   设备: avformat_open_input(ctx, "1:none", avfoundation_fmt, &options)
 *         需要指定输入格式, 并通过 options 设置帧率/分辨率等
 *
 * 平台差异:
 *   macOS:   avfoundation, 设备ID "1:none" (1=屏幕, none=无音频)
 *   Linux:   x11grab, 设备ID ":0.0" (X11 display)
 *   Windows: dshow 或 gdigrab
 */

#include <cstdio>
#include <cstring>
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavdevice/avdevice.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "libavutil/time.h"
#include "libavutil/dict.h"
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("用法: %s <输出文件> <录制秒数>\n", argv[0]);
        printf("示例: %s screen.mp4 10\n", argv[0]);
        return -1;
    }

    int duration_sec = atoi(argv[2]);

    // === 注册设备 ===
    // 必须调用, 否则 av_find_input_format 找不到 avfoundation/x11grab
    avdevice_register_all();

    AVFormatContext *in_ctx = nullptr;
    AVFormatContext *out_ctx = nullptr;
    AVCodecContext *dec_ctx = nullptr, *enc_ctx = nullptr;
    struct SwsContext *sws_ctx = nullptr;
    int ret = 0;

    // === 打开屏幕采集设备 ===
    AVDictionary *options = nullptr;
    av_dict_set(&options, "framerate", "30", 0);
    av_dict_set(&options, "capture_cursor", "1", 0);

#ifdef __APPLE__
    // macOS: avfoundation
    // "1:none" = 屏幕设备1, 无音频
    // "0:none" = 摄像头
    AVInputFormat *input_fmt = av_find_input_format("avfoundation");
    const char *device_id = "1:none";
    av_dict_set(&options, "pixel_format", "uyvy422", 0);
#elif __linux__
    // Linux: x11grab
    AVInputFormat *input_fmt = av_find_input_format("x11grab");
    const char *device_id = ":0.0";
    av_dict_set(&options, "video_size", "1920x1080", 0);
#else
    printf("当前平台暂不支持, 请使用 macOS 或 Linux\n");
    return -1;
#endif

    if (!input_fmt) {
        printf("找不到屏幕采集设备格式\n");
        printf("macOS 需要 avfoundation, Linux 需要 x11grab\n");
        return -1;
    }

    ret = avformat_open_input(&in_ctx, device_id, input_fmt, &options);
    av_dict_free(&options);
    if (ret < 0) {
        printf("打开屏幕采集失败: %s\n", av_err2str(ret));
        return -1;
    }
    avformat_find_stream_info(in_ctx, nullptr);

    int vi = av_find_best_stream(in_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (vi < 0) { printf("没有视频流\n"); return -1; }

    AVCodecParameters *in_par = in_ctx->streams[vi]->codecpar;
    printf("屏幕采集: %dx%d, 像素格式: %d\n", in_par->width, in_par->height, in_par->format);

    // === 解码器 (采集设备输出的可能是 uyvy422 等格式, 需要解码) ===
    dec_ctx = avcodec_alloc_context3(nullptr);
    avcodec_parameters_to_context(dec_ctx, in_par);
    const AVCodec *decoder = avcodec_find_decoder(dec_ctx->codec_id);
    if (decoder) {
        avcodec_open2(dec_ctx, decoder, nullptr);
    }

    // === 编码器 ===
    int out_w = in_par->width, out_h = in_par->height;
    const AVCodec *encoder = avcodec_find_encoder_by_name("libx264");
    if (!encoder) encoder = avcodec_find_encoder(AV_CODEC_ID_H264);
    enc_ctx = avcodec_alloc_context3(encoder);
    enc_ctx->width = out_w;
    enc_ctx->height = out_h;
    enc_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    enc_ctx->time_base = (AVRational){1, 30};
    enc_ctx->bit_rate = 4000000;
    enc_ctx->gop_size = 30;
    enc_ctx->max_b_frames = 0;
    if (strcmp(encoder->name, "libx264") == 0)
        av_opt_set(enc_ctx->priv_data, "preset", "ultrafast", 0);
    avcodec_open2(enc_ctx, encoder, nullptr);

    // === 像素格式转换 (采集格式 -> YUV420P) ===
    sws_ctx = sws_getContext(out_w, out_h, (enum AVPixelFormat)in_par->format,
                              out_w, out_h, AV_PIX_FMT_YUV420P,
                              SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

    // === 输出文件 ===
    avformat_alloc_output_context2(&out_ctx, nullptr, nullptr, argv[1]);
    AVStream *out_stream = avformat_new_stream(out_ctx, nullptr);
    avcodec_parameters_from_context(out_stream->codecpar, enc_ctx);
    out_stream->codecpar->codec_tag = 0;
    out_stream->time_base = enc_ctx->time_base;

    if (!(out_ctx->oformat->flags & AVFMT_NOFILE))
        avio_open(&out_ctx->pb, argv[1], AVIO_FLAG_WRITE);
    avformat_write_header(out_ctx, nullptr);

    // === 采集循环 ===
    AVPacket pkt;
    AVFrame *frame = av_frame_alloc();
    AVFrame *yuv_frame = av_frame_alloc();
    yuv_frame->format = AV_PIX_FMT_YUV420P;
    yuv_frame->width = out_w;
    yuv_frame->height = out_h;
    av_frame_get_buffer(yuv_frame, 0);

    int64_t start_time = av_gettime();
    int64_t end_time = start_time + (int64_t)duration_sec * 1000000;
    int frame_count = 0;

    printf("开始录屏 %d 秒...\n", duration_sec);

    while (av_gettime() < end_time) {
        ret = av_read_frame(in_ctx, &pkt);
        if (ret < 0) break;

        if (pkt.stream_index == vi) {
            // 有些采集设备直接输出 rawvideo, 不需要解码
            // 有些需要解码 (如 avfoundation 输出 uyvy422)
            if (decoder) {
                avcodec_send_packet(dec_ctx, &pkt);
                while (avcodec_receive_frame(dec_ctx, frame) == 0) {
                    sws_scale(sws_ctx, (const uint8_t *const *)frame->data, frame->linesize,
                              0, out_h, yuv_frame->data, yuv_frame->linesize);
                    yuv_frame->pts = frame_count++;
                    yuv_frame->pict_type = AV_PICTURE_TYPE_NONE;

                    avcodec_send_frame(enc_ctx, yuv_frame);
                    AVPacket ep;
                    memset(&ep, 0, sizeof(ep));
                    while (avcodec_receive_packet(enc_ctx, &ep) == 0) {
                        ep.stream_index = 0;
                        av_packet_rescale_ts(&ep, enc_ctx->time_base, out_stream->time_base);
                        av_interleaved_write_frame(out_ctx, &ep);
                        av_packet_unref(&ep);
                    }
                }
            }
        }
        av_packet_unref(&pkt);

        if (frame_count % 30 == 0)
            printf("  已录制 %d 帧 (%.1f秒)...\n", frame_count, frame_count / 30.0);
    }

    // flush
    avcodec_send_frame(enc_ctx, nullptr);
    AVPacket ep;
    memset(&ep, 0, sizeof(ep));
    while (avcodec_receive_packet(enc_ctx, &ep) == 0) {
        ep.stream_index = 0;
        av_packet_rescale_ts(&ep, enc_ctx->time_base, out_stream->time_base);
        av_interleaved_write_frame(out_ctx, &ep);
        av_packet_unref(&ep);
    }

    av_write_trailer(out_ctx);
    printf("录屏完成: %d 帧 -> %s\n", frame_count, argv[1]);

    av_frame_free(&frame);
    av_frame_free(&yuv_frame);
    sws_freeContext(sws_ctx);
    avcodec_free_context(&enc_ctx);
    if (dec_ctx) avcodec_free_context(&dec_ctx);
    if (!(out_ctx->oformat->flags & AVFMT_NOFILE)) avio_closep(&out_ctx->pb);
    avformat_free_context(out_ctx);
    avformat_close_input(&in_ctx);
    return 0;
}
