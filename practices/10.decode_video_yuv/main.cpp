/**
 * ============================================================================
 * 10. 解码整个视频为 YUV
 * ============================================================================
 *
 * 等价命令:
 *   ffmpeg -i input.mp4 -pix_fmt yuv420p output.yuv
 *
 * 相比 09 (只解码第一帧), 这里解码所有帧, 并正确 flush 解码器。
 *
 * flush 机制:
 *   文件读完后, 解码器内部可能还缓存着几帧 (因为 B 帧需要参考后续帧)。
 *   发送一个 nullptr packet 通知解码器"没有更多输入了",
 *   然后继续 receive 直到返回 EOF。
 *
 * linesize 注意事项:
 *   frame->linesize[0] 可能大于 width (内存对齐, 如 1920 对齐到 1984)。
 *   写文件时要逐行写, 每行只取 width 个字节, 否则图像会错位。
 */

#include <cstdio>
#include <cstring>
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/log.h"
}

// 将一帧 YUV420P 写入文件
static void write_yuv_frame(FILE *fp, AVFrame *frame, int width, int height) {
    // Y 平面: 逐行写, 每行 width 字节
    for (int y = 0; y < height; y++)
        fwrite(frame->data[0] + y * frame->linesize[0], 1, width, fp);
    // U 平面: 宽高各减半
    for (int y = 0; y < height / 2; y++)
        fwrite(frame->data[1] + y * frame->linesize[1], 1, width / 2, fp);
    // V 平面
    for (int y = 0; y < height / 2; y++)
        fwrite(frame->data[2] + y * frame->linesize[2], 1, width / 2, fp);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("用法: %s <输入文件> <输出.yuv>\n", argv[0]);
        return -1;
    }

    AVFormatContext *fmt_ctx = nullptr;
    AVCodecContext *dec_ctx = nullptr;
    FILE *fp = nullptr;
    int ret = 0;

    ret = avformat_open_input(&fmt_ctx, argv[1], nullptr, nullptr);
    if (ret < 0) { printf("打开失败: %s\n", av_err2str(ret)); return -1; }
    avformat_find_stream_info(fmt_ctx, nullptr);

    int video_idx = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (video_idx < 0) { printf("没有视频流\n"); return -1; }

    // 初始化解码器 (09 学过的)
    dec_ctx = avcodec_alloc_context3(nullptr);
    avcodec_parameters_to_context(dec_ctx, fmt_ctx->streams[video_idx]->codecpar);
    const AVCodec *decoder = avcodec_find_decoder(dec_ctx->codec_id);
    if (!decoder) { printf("找不到解码器\n"); goto end; }
    ret = avcodec_open2(dec_ctx, decoder, nullptr);
    if (ret < 0) { printf("打开解码器失败: %s\n", av_err2str(ret)); goto end; }

    printf("视频: %dx%d, 解码器: %s\n", dec_ctx->width, dec_ctx->height, decoder->name);

    fp = fopen(argv[2], "wb");
    if (!fp) { printf("无法创建 %s\n", argv[2]); goto end; }

    {
    AVPacket pkt;
    memset(&pkt, 0, sizeof(pkt));
    AVFrame *frame = av_frame_alloc();
    int count = 0;

    // === 解码循环 ===
    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        if (pkt.stream_index == video_idx) {
            avcodec_send_packet(dec_ctx, &pkt);
            while (avcodec_receive_frame(dec_ctx, frame) == 0) {
                write_yuv_frame(fp, frame, dec_ctx->width, dec_ctx->height);
                count++;
                if (count % 100 == 0) printf("已解码 %d 帧...\n", count);
            }
        }
        av_packet_unref(&pkt);
    }

    // === flush: 取出解码器缓存的剩余帧 ===
    avcodec_send_packet(dec_ctx, nullptr);
    while (avcodec_receive_frame(dec_ctx, frame) == 0) {
        write_yuv_frame(fp, frame, dec_ctx->width, dec_ctx->height);
        count++;
    }

    printf("完成: %d 帧 -> %s\n", count, argv[2]);
    printf("播放: ffplay -video_size %dx%d -pixel_format yuv420p %s\n",
           dec_ctx->width, dec_ctx->height, argv[2]);

    av_frame_free(&frame);
    }

end:
    if (fp) fclose(fp);
    if (dec_ctx) avcodec_free_context(&dec_ctx);
    if (fmt_ctx) avformat_close_input(&fmt_ctx);
    return 0;
}
