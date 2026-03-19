/**
 * ============================================================================
 * 12. 深入理解时间戳
 * ============================================================================
 *
 * 等价命令:
 *   ffprobe -show_frames -select_streams v input.mp4
 *
 * 解码视频帧, 打印每帧的详细时间戳和帧类型。
 *
 * I/P/B 帧:
 *   I帧 (关键帧): 完整的图像, 不依赖其他帧, 可以独立解码
 *   P帧 (前向预测): 只存储与前一帧的差异, 依赖前面的 I 或 P 帧
 *   B帧 (双向预测): 存储与前后帧的差异, 依赖前后的帧
 *
 * 因为 B 帧依赖"后面"的帧, 所以:
 *   解码顺序 (dts): I0 P3 B1 B2 P6 B4 B5 ...
 *   显示顺序 (pts): I0 B1 B2 P3 B4 B5 P6 ...
 *   dts 和 pts 不一样!
 *
 * 如果没有 B 帧, dts == pts, 一切都简单。
 */

#include <cstdio>
#include <cstring>
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/log.h"
}

static const char *get_pict_type(enum AVPictureType type) {
    switch (type) {
        case AV_PICTURE_TYPE_I: return "I";
        case AV_PICTURE_TYPE_P: return "P";
        case AV_PICTURE_TYPE_B: return "B";
        default: return "?";
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("用法: %s <输入文件> [最大帧数]\n", argv[0]);
        return -1;
    }

    int max_frames = 30;
    if (argc >= 3) max_frames = atoi(argv[2]);

    AVFormatContext *fmt_ctx = nullptr;
    AVCodecContext *dec_ctx = nullptr;
    int ret = 0;

    ret = avformat_open_input(&fmt_ctx, argv[1], nullptr, nullptr);
    if (ret < 0) { printf("打开失败: %s\n", av_err2str(ret)); return -1; }
    avformat_find_stream_info(fmt_ctx, nullptr);

    int video_idx = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (video_idx < 0) { printf("没有视频流\n"); return -1; }

    dec_ctx = avcodec_alloc_context3(nullptr);
    avcodec_parameters_to_context(dec_ctx, fmt_ctx->streams[video_idx]->codecpar);
    const AVCodec *decoder = avcodec_find_decoder(dec_ctx->codec_id);
    avcodec_open2(dec_ctx, decoder, nullptr);

    AVRational tb = fmt_ctx->streams[video_idx]->time_base;
    printf("视频: %s, %dx%d, time_base=%d/%d\n\n",
           decoder->name, dec_ctx->width, dec_ctx->height, tb.num, tb.den);

    printf("%-6s %-6s %-12s %-12s %-12s %-12s %-8s\n",
           "帧号", "类型", "PTS", "PTS(秒)", "DTS", "DTS(秒)", "key");
    printf("----------------------------------------------------------------------\n");

    AVPacket pkt;
    memset(&pkt, 0, sizeof(pkt));
    AVFrame *frame = av_frame_alloc();
    int count = 0;

    while (av_read_frame(fmt_ctx, &pkt) >= 0 && count < max_frames) {
        if (pkt.stream_index == video_idx) {
            // 打印包级别的 dts (解码顺序)
            int64_t pkt_dts = pkt.dts;

            avcodec_send_packet(dec_ctx, &pkt);
            while (avcodec_receive_frame(dec_ctx, frame) == 0 && count < max_frames) {
                // frame->pts 是显示时间戳
                // frame->pict_type 是帧类型
                double pts_sec = (frame->pts != AV_NOPTS_VALUE) ?
                                  frame->pts * av_q2d(tb) : -1;
                double dts_sec = (pkt_dts != AV_NOPTS_VALUE) ?
                                  pkt_dts * av_q2d(tb) : -1;

                printf("%-6d %-6s %-12lld %-12.3f %-12lld %-12.3f %-8s\n",
                       count,
                       get_pict_type(frame->pict_type),
                       (long long)frame->pts, pts_sec,
                       (long long)pkt_dts, dts_sec,
                       frame->key_frame ? "KEY" : "");
                count++;
            }
        }
        av_packet_unref(&pkt);
    }

    printf("\n共 %d 帧\n", count);

    av_frame_free(&frame);
    avcodec_free_context(&dec_ctx);
    avformat_close_input(&fmt_ctx);
    return 0;
}
