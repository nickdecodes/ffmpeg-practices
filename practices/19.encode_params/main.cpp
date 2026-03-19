/**
 * ============================================================================
 * 19. 编码参数对比实验
 * ============================================================================
 *
 * 用同一个 YUV 源, 不同参数编码, 对比文件大小和耗时。
 *
 * 测试项:
 *   1. preset 对比: ultrafast vs medium vs veryslow
 *   2. bitrate 对比: 500k vs 2M vs 5M
 *   3. CRF 对比: 18 vs 23 vs 28 vs 35
 *   4. GOP 对比: 10 vs 50 vs 250
 */

#include <cstdio>
#include <cstring>
#include <ctime>
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/imgutils.h"
#include "libavutil/parseutils.h"
#include "libavutil/opt.h"
#include "libavutil/log.h"
}

struct EncodeResult {
    const char *label;
    int64_t file_size;
    double elapsed_sec;
};

// 编码一遍 YUV, 返回输出大小
static EncodeResult encode_once(const char *yuv_path, int w, int h,
                                 const char *label,
                                 const char *preset, int bitrate, int crf, int gop) {
    EncodeResult result = {label, 0, 0};
    clock_t start = clock();

    const AVCodec *enc = avcodec_find_encoder_by_name("libx264");
    if (!enc) enc = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!enc) return result;

    AVCodecContext *ctx = avcodec_alloc_context3(enc);
    ctx->width = w;
    ctx->height = h;
    ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    ctx->time_base = (AVRational){1, 30};
    ctx->gop_size = gop;
    ctx->max_b_frames = 0;

    if (bitrate > 0) {
        ctx->bit_rate = bitrate;
    }

    if (strcmp(enc->name, "libx264") == 0) {
        av_opt_set(ctx->priv_data, "preset", preset, 0);
        if (crf >= 0) {
            char crf_str[16];
            snprintf(crf_str, sizeof(crf_str), "%d", crf);
            av_opt_set(ctx->priv_data, "crf", crf_str, 0);
        }
    }

    if (avcodec_open2(ctx, enc, nullptr) < 0) {
        avcodec_free_context(&ctx);
        return result;
    }

    FILE *fp = fopen(yuv_path, "rb");
    if (!fp) { avcodec_free_context(&ctx); return result; }

    AVFrame *frame = av_frame_alloc();
    frame->format = AV_PIX_FMT_YUV420P;
    frame->width = w;
    frame->height = h;
    int buf_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, w, h, 1);
    uint8_t *buf = (uint8_t *)av_malloc(buf_size);
    av_image_fill_arrays(frame->data, frame->linesize, buf, AV_PIX_FMT_YUV420P, w, h, 1);

    AVPacket pkt;
    memset(&pkt, 0, sizeof(pkt));
    int yuv_size = w * h * 3 / 2;
    int count = 0;

    while (fread(buf, 1, yuv_size, fp) == (size_t)yuv_size) {
        frame->data[0] = buf;
        frame->data[1] = buf + w * h;
        frame->data[2] = buf + w * h + w * h / 4;
        frame->pts = count++;

        avcodec_send_frame(ctx, frame);
        while (avcodec_receive_packet(ctx, &pkt) == 0) {
            result.file_size += pkt.size;
            av_packet_unref(&pkt);
        }
    }

    avcodec_send_frame(ctx, nullptr);
    while (avcodec_receive_packet(ctx, &pkt) == 0) {
        result.file_size += pkt.size;
        av_packet_unref(&pkt);
    }

    result.elapsed_sec = (double)(clock() - start) / CLOCKS_PER_SEC;

    fclose(fp);
    av_freep(&buf);
    av_frame_free(&frame);
    avcodec_free_context(&ctx);
    return result;
}

static void print_result(EncodeResult r) {
    printf("  %-25s  %8lld bytes  (%6.1f KB)  %.2f秒\n",
           r.label, (long long)r.file_size, r.file_size / 1024.0, r.elapsed_sec);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("用法: %s <输入.yuv> <宽x高>\n", argv[0]);
        printf("示例: %s test.yuv 640x480\n", argv[0]);
        return -1;
    }

    int w = 0, h = 0;
    av_parse_video_size(&w, &h, argv[2]);
    if (w <= 0) { printf("无效尺寸\n"); return -1; }

    printf("源: %s (%dx%d)\n\n", argv[1], w, h);

    // === preset 对比 ===
    printf("=== Preset 对比 (CRF=23, GOP=50) ===\n");
    print_result(encode_once(argv[1], w, h, "ultrafast", "ultrafast", 0, 23, 50));
    print_result(encode_once(argv[1], w, h, "medium",    "medium",    0, 23, 50));
    print_result(encode_once(argv[1], w, h, "veryslow",  "veryslow",  0, 23, 50));

    // === CRF 对比 ===
    printf("\n=== CRF 对比 (preset=medium, GOP=50) ===\n");
    print_result(encode_once(argv[1], w, h, "crf=18 (高质量)", "medium", 0, 18, 50));
    print_result(encode_once(argv[1], w, h, "crf=23 (默认)",   "medium", 0, 23, 50));
    print_result(encode_once(argv[1], w, h, "crf=28 (低质量)", "medium", 0, 28, 50));
    print_result(encode_once(argv[1], w, h, "crf=35 (很低)",   "medium", 0, 35, 50));

    // === GOP 对比 ===
    printf("\n=== GOP 对比 (preset=medium, CRF=23) ===\n");
    print_result(encode_once(argv[1], w, h, "gop=10",  "medium", 0, 23, 10));
    print_result(encode_once(argv[1], w, h, "gop=50",  "medium", 0, 23, 50));
    print_result(encode_once(argv[1], w, h, "gop=250", "medium", 0, 23, 250));

    return 0;
}
