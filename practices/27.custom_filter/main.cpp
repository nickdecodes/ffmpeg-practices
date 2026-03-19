/**
 * ============================================================================
 * 27. 自定义滤镜: 不用内置滤镜, 自己处理每一帧
 * ============================================================================
 *
 * 两种方式实现自定义处理:
 *
 * 方式 1 (本例): 在 decode 和 encode 之间直接操作 AVFrame 的像素数据
 *   这是最简单直接的方式, 不需要注册 AVFilter, 适合简单的像素操作。
 *
 * 方式 2: 注册自定义 AVFilter 到 filter graph
 *   更复杂, 但可以和其他内置滤镜串联使用。
 *
 * 本例实现一个"反色"效果: 每个像素值取反 (255 - value)
 * 你可以替换 process_frame 函数来实现任何自定义效果:
 *   - 灰度化
 *   - 加水印
 *   - 边缘检测
 *   - 马赛克
 *   - ...
 */

#include <cstdio>
#include <cstring>
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/log.h"
#include "libavutil/imgutils.h"
}

/**
 * 自定义帧处理函数: 反色效果
 * 直接操作 AVFrame 的像素数据
 *
 * YUV420P 内存布局:
 *   data[0] = Y 平面 (亮度, 0=黑 255=白)
 *   data[1] = U 平面 (色度)
 *   data[2] = V 平面 (色度)
 *
 * 只反转 Y 平面就能得到"底片"效果
 * 如果同时反转 U/V, 颜色也会反转
 */
static void process_frame(AVFrame *frame) {
    // 反转 Y 平面 (亮度取反)
    for (int y = 0; y < frame->height; y++) {
        uint8_t *row = frame->data[0] + y * frame->linesize[0];
        for (int x = 0; x < frame->width; x++) {
            row[x] = 255 - row[x];
        }
    }

    // 也反转 U/V 平面 (颜色取反)
    for (int p = 1; p <= 2; p++) {
        for (int y = 0; y < frame->height / 2; y++) {
            uint8_t *row = frame->data[p] + y * frame->linesize[p];
            for (int x = 0; x < frame->width / 2; x++) {
                row[x] = 255 - row[x];
            }
        }
    }
}

// 写 YUV 帧到文件
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
        printf("示例: %s input.mp4 output.yuv\n", argv[0]);
        return -1;
    }

    AVFormatContext *fmt_ctx = nullptr;
    AVCodecContext *dec_ctx = nullptr;
    FILE *fp = nullptr;

    avformat_open_input(&fmt_ctx, argv[1], nullptr, nullptr);
    avformat_find_stream_info(fmt_ctx, nullptr);
    int vi = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (vi < 0) { printf("没有视频流\n"); return -1; }

    dec_ctx = avcodec_alloc_context3(nullptr);
    avcodec_parameters_to_context(dec_ctx, fmt_ctx->streams[vi]->codecpar);
    avcodec_open2(dec_ctx, avcodec_find_decoder(dec_ctx->codec_id), nullptr);

    int w = dec_ctx->width, h = dec_ctx->height;
    printf("视频: %dx%d, 应用反色滤镜\n", w, h);

    fp = fopen(argv[2], "wb");
    if (!fp) { printf("无法创建 %s\n", argv[2]); return -1; }

    AVPacket pkt;
    memset(&pkt, 0, sizeof(pkt));
    AVFrame *frame = av_frame_alloc();
    int count = 0;

    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        if (pkt.stream_index == vi) {
            avcodec_send_packet(dec_ctx, &pkt);
            while (avcodec_receive_frame(dec_ctx, frame) == 0) {
                // === 关键: 在这里对帧做自定义处理 ===
                // 你可以替换这个函数来实现任何效果
                process_frame(frame);

                write_yuv(fp, frame, w, h);
                count++;
                if (count % 100 == 0) printf("已处理 %d 帧...\n", count);
            }
        }
        av_packet_unref(&pkt);
    }

    // flush
    avcodec_send_packet(dec_ctx, nullptr);
    while (avcodec_receive_frame(dec_ctx, frame) == 0) {
        process_frame(frame);
        write_yuv(fp, frame, w, h);
        count++;
    }

    printf("完成: %d 帧 (反色) -> %s\n", count, argv[2]);
    printf("播放: ffplay -video_size %dx%d -pixel_format yuv420p %s\n", w, h, argv[2]);

    av_frame_free(&frame);
    fclose(fp);
    avcodec_free_context(&dec_ctx);
    avformat_close_input(&fmt_ctx);
    return 0;
}
