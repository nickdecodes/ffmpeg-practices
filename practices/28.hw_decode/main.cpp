/**
 * ============================================================================
 * 28. 硬件加速解码
 * ============================================================================
 *
 * 等价命令 (macOS):
 *   ffmpeg -hwaccel videotoolbox -i input.mp4 -pix_fmt yuv420p output.yuv
 *
 * 软件解码 vs 硬件解码:
 *   软件: CPU 逐像素计算, 通用但慢
 *   硬件: GPU/专用芯片解码, 快但数据在 GPU 内存中
 *
 * 硬件解码的额外步骤:
 *   1. 创建硬件设备上下文 (av_hwdevice_ctx_create)
 *   2. 设置解码器使用硬件加速 (hw_device_ctx)
 *   3. 解码后帧数据在 GPU 内存, 需要传回 CPU (av_hwframe_transfer_data)
 *
 * 支持的硬件加速:
 *   macOS:   AV_HWDEVICE_TYPE_VIDEOTOOLBOX
 *   Linux:   AV_HWDEVICE_TYPE_VAAPI, AV_HWDEVICE_TYPE_CUDA
 *   Windows: AV_HWDEVICE_TYPE_DXVA2, AV_HWDEVICE_TYPE_D3D11VA
 */

#include <cstdio>
#include <cstring>
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/hwcontext.h"
#include "libavutil/log.h"
}

// 自动选择当前平台支持的硬件加速类型
static enum AVHWDeviceType find_hw_type() {
    enum AVHWDeviceType types[] = {
#ifdef __APPLE__
        AV_HWDEVICE_TYPE_VIDEOTOOLBOX,
#endif
#ifdef __linux__
        AV_HWDEVICE_TYPE_VAAPI,
        AV_HWDEVICE_TYPE_CUDA,
#endif
#ifdef _WIN32
        AV_HWDEVICE_TYPE_DXVA2,
        AV_HWDEVICE_TYPE_D3D11VA,
#endif
        AV_HWDEVICE_TYPE_NONE
    };

    for (int i = 0; types[i] != AV_HWDEVICE_TYPE_NONE; i++) {
        // 简单返回第一个, 实际应该检查是否可用
        return types[i];
    }
    return AV_HWDEVICE_TYPE_NONE;
}

// 查找解码器支持的硬件像素格式
static enum AVPixelFormat hw_pix_fmt = AV_PIX_FMT_NONE;

static enum AVPixelFormat get_hw_format(AVCodecContext *ctx,
                                         const enum AVPixelFormat *pix_fmts) {
    for (const enum AVPixelFormat *p = pix_fmts; *p != AV_PIX_FMT_NONE; p++) {
        if (*p == hw_pix_fmt)
            return *p;
    }
    printf("警告: 硬件像素格式不可用, 回退到软件解码\n");
    return AV_PIX_FMT_NONE;
}

static void write_yuv(FILE *fp, AVFrame *frame, int w, int h) {
    for (int y = 0; y < h; y++)
        fwrite(frame->data[0] + y * frame->linesize[0], 1, w, fp);
    for (int y = 0; y < h / 2; y++)
        fwrite(frame->data[1] + y * frame->linesize[1], 1, w / 2, fp);
    for (int y = 0; y < h / 2; y++)
        fwrite(frame->data[2] + y * frame->linesize[2], 1, w / 2, fp);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("用法: %s <输入文件> <输出.yuv>\n", argv[0]);
        return -1;
    }

    AVFormatContext *fmt_ctx = nullptr;
    AVCodecContext *dec_ctx = nullptr;
    AVBufferRef *hw_device_ctx = nullptr;
    FILE *fp = nullptr;
    int ret = 0;

    // === 选择硬件加速类型 ===
    enum AVHWDeviceType hw_type = find_hw_type();
    if (hw_type == AV_HWDEVICE_TYPE_NONE) {
        printf("当前平台没有可用的硬件加速, 将使用软件解码\n");
    } else {
        printf("硬件加速: %s\n", av_hwdevice_get_type_name(hw_type));
    }

    avformat_open_input(&fmt_ctx, argv[1], nullptr, nullptr);
    avformat_find_stream_info(fmt_ctx, nullptr);
    int vi = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (vi < 0) { printf("没有视频流\n"); return -1; }

    // 查找解码器
    const AVCodec *decoder = avcodec_find_decoder(fmt_ctx->streams[vi]->codecpar->codec_id);
    dec_ctx = avcodec_alloc_context3(decoder);
    avcodec_parameters_to_context(dec_ctx, fmt_ctx->streams[vi]->codecpar);

    // === 配置硬件加速 ===
    if (hw_type != AV_HWDEVICE_TYPE_NONE) {
        // 查找解码器支持的硬件配置
        for (int i = 0;; i++) {
            const AVCodecHWConfig *config = avcodec_get_hw_config(decoder, i);
            if (!config) break;
            if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
                config->device_type == hw_type) {
                hw_pix_fmt = config->pix_fmt;
                break;
            }
        }

        if (hw_pix_fmt != AV_PIX_FMT_NONE) {
            // 创建硬件设备上下文
            ret = av_hwdevice_ctx_create(&hw_device_ctx, hw_type, nullptr, nullptr, 0);
            if (ret >= 0) {
                dec_ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);
                dec_ctx->get_format = get_hw_format;
                printf("硬件加速已启用, 像素格式: %s\n",
                       av_get_pix_fmt_name(hw_pix_fmt));
            } else {
                printf("创建硬件设备失败: %s, 回退到软件解码\n", av_err2str(ret));
            }
        }
    }

    avcodec_open2(dec_ctx, decoder, nullptr);
    printf("视频: %dx%d, 解码器: %s\n", dec_ctx->width, dec_ctx->height, decoder->name);

    fp = fopen(argv[2], "wb");
    if (!fp) { printf("无法创建 %s\n", argv[2]); return -1; }

    AVPacket pkt;
    memset(&pkt, 0, sizeof(pkt));
    AVFrame *frame = av_frame_alloc();
    AVFrame *sw_frame = av_frame_alloc(); // 用于硬件帧转软件帧
    int count = 0;

    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        if (pkt.stream_index == vi) {
            avcodec_send_packet(dec_ctx, &pkt);
            while (avcodec_receive_frame(dec_ctx, frame) == 0) {
                AVFrame *output_frame = frame;

                // 如果是硬件帧, 需要传回 CPU 内存
                if (frame->format == hw_pix_fmt) {
                    // av_hwframe_transfer_data: GPU -> CPU
                    ret = av_hwframe_transfer_data(sw_frame, frame, 0);
                    if (ret < 0) {
                        printf("硬件帧传输失败: %s\n", av_err2str(ret));
                        continue;
                    }
                    output_frame = sw_frame;
                }

                // 写入 YUV (假设输出是 NV12 或 YUV420P)
                if (output_frame->format == AV_PIX_FMT_YUV420P) {
                    write_yuv(fp, output_frame, dec_ctx->width, dec_ctx->height);
                } else if (output_frame->format == AV_PIX_FMT_NV12) {
                    // NV12: Y 平面 + UV 交织平面
                    // 简单写入 (ffplay 用 -pixel_format nv12 播放)
                    for (int y = 0; y < dec_ctx->height; y++)
                        fwrite(output_frame->data[0] + y * output_frame->linesize[0],
                               1, dec_ctx->width, fp);
                    for (int y = 0; y < dec_ctx->height / 2; y++)
                        fwrite(output_frame->data[1] + y * output_frame->linesize[1],
                               1, dec_ctx->width, fp);
                }

                count++;
                if (count % 100 == 0) printf("已解码 %d 帧...\n", count);

                if (output_frame == sw_frame)
                    av_frame_unref(sw_frame);
            }
        }
        av_packet_unref(&pkt);
    }

    const char *pix_name = hw_device_ctx ? "nv12" : "yuv420p";
    printf("完成: %d 帧 (%s解码) -> %s\n", count,
           hw_device_ctx ? "硬件" : "软件", argv[2]);
    printf("播放: ffplay -video_size %dx%d -pixel_format %s %s\n",
           dec_ctx->width, dec_ctx->height, pix_name, argv[2]);

    av_frame_free(&frame);
    av_frame_free(&sw_frame);
    if (hw_device_ctx) av_buffer_unref(&hw_device_ctx);
    avcodec_free_context(&dec_ctx);
    avformat_close_input(&fmt_ctx);
    fclose(fp);
    return 0;
}
