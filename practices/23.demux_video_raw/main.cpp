/**
 * ============================================================================
 * 23. 提取视频裸流 (不转格式, 对比 06)
 * ============================================================================
 *
 * 等价命令:
 *   ffmpeg -i input.mp4 -an -vcodec copy output.h264
 *
 * 和 06 的区别:
 *   06: 用 h264_mp4toannexb BSF 转换, 输出 Annex B 格式, ffplay 能播
 *   23: 直接提取, 输出 AVCC 格式, 大多数播放器不能播
 *
 * 对比两个输出文件的前几个字节:
 *   Annex B: 00 00 00 01 67 ...  (起始码)
 *   AVCC:    00 00 XX XX 67 ...  (长度前缀)
 *
 * 这个例子的目的是让你理解为什么 06 需要 BSF。
 */

#include <cstdio>
#include <cstring>
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("用法: %s <输入文件> <输出.h264>\n", argv[0]);
        return -1;
    }

    AVFormatContext *fmt_ctx = nullptr;
    avformat_open_input(&fmt_ctx, argv[1], nullptr, nullptr);
    avformat_find_stream_info(fmt_ctx, nullptr);

    int vi = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (vi < 0) { printf("没有视频流\n"); return -1; }

    printf("视频: %s, %dx%d\n",
           avcodec_get_name(fmt_ctx->streams[vi]->codecpar->codec_id),
           fmt_ctx->streams[vi]->codecpar->width,
           fmt_ctx->streams[vi]->codecpar->height);

    FILE *fp = fopen(argv[2], "wb");

    AVPacket pkt;
    memset(&pkt, 0, sizeof(pkt));
    int count = 0;

    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        if (pkt.stream_index == vi) {
            // 直接写, 不经过 BSF
            fwrite(pkt.data, 1, pkt.size, fp);
            // 打印前几个包的头部字节, 观察格式
            if (count < 5) {
                printf("包 #%d (%d bytes), 头部: %02x %02x %02x %02x %02x\n",
                       count, pkt.size,
                       pkt.data[0], pkt.data[1], pkt.data[2], pkt.data[3], pkt.data[4]);
            }
            count++;
        }
        av_packet_unref(&pkt);
    }

    printf("提取完成: %d 个包 -> %s\n", count, argv[2]);
    printf("注意: 这个文件是 AVCC 格式, ffplay 可能无法播放\n");
    printf("对比: 用 06 提取的 Annex B 格式可以播放\n");

    fclose(fp);
    avformat_close_input(&fmt_ctx);
    return 0;
}
