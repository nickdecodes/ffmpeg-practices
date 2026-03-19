/**
 * ============================================================================
 * 03. 逐包读取数据
 * ============================================================================
 *
 * 等价命令:
 *   ffprobe -show_packets input.mp4
 *
 * 在 02 的基础上, 开始用 av_read_frame 逐包读取压缩数据。
 * 这是理解 FFmpeg 数据流转的关键一步。
 *
 * 新增 API:
 *   av_read_frame()   - 读取一个压缩数据包 (AVPacket)
 *   av_packet_unref() - 释放包数据的引用
 *
 * 重要概念 - AVPacket:
 *   data          - 压缩数据指针
 *   size          - 数据大小 (字节)
 *   stream_index  - 属于哪路流 (0=视频? 1=音频? 看具体文件)
 *   pts           - 显示时间戳 (Presentation Timestamp)
 *   dts           - 解码时间戳 (Decoding Timestamp)
 *   duration      - 持续时间
 *   flags         - 标志位, AV_PKT_FLAG_KEY 表示关键帧
 *
 * pts vs dts:
 *   大多数情况下 pts == dts。
 *   但如果视频有 B 帧, 解码顺序和显示顺序不同, 此时 dts != pts。
 *   dts 是送入解码器的顺序, pts 是显示给用户看的顺序。
 */

#include <cstdio>
#include <cstring>
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/log.h"
}

int main(int argc, char **argv) {
    av_log_set_level(AV_LOG_INFO);

    if (argc < 2) {
        printf("用法: %s <输入文件> [最大包数]\n", argv[0]);
        printf("示例: %s input.mp4 50\n", argv[0]);
        return -1;
    }

    int max_packets = 50; // 默认只打印前 50 个包, 避免刷屏
    if (argc >= 3) {
        max_packets = atoi(argv[2]);
    }

    // === 01, 02 学过的 ===
    AVFormatContext *fmt_ctx = nullptr;
    int ret = avformat_open_input(&fmt_ctx, argv[1], nullptr, nullptr);
    if (ret < 0) {
        printf("打开失败: %s\n", av_err2str(ret));
        return -1;
    }
    avformat_find_stream_info(fmt_ctx, nullptr);

    // 先打印流信息, 方便对照 stream_index
    for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++) {
        const char *type = "未知";
        switch (fmt_ctx->streams[i]->codecpar->codec_type) {
            case AVMEDIA_TYPE_VIDEO: type = "视频"; break;
            case AVMEDIA_TYPE_AUDIO: type = "音频"; break;
            case AVMEDIA_TYPE_SUBTITLE: type = "字幕"; break;
            default: break;
        }
        printf("流 #%d: %s (%s)\n", i, type,
               avcodec_get_name(fmt_ctx->streams[i]->codecpar->codec_id));
    }
    printf("\n");

    // === 新内容: 逐包读取 ===
    printf("%-6s %-6s %-10s %-12s %-12s %-10s %-5s\n",
           "序号", "流", "大小", "PTS", "DTS", "时间(秒)", "关键帧");
    printf("------------------------------------------------------------------\n");

    AVPacket pkt;
    memset(&pkt, 0, sizeof(pkt));
    int count = 0;

    // av_read_frame 每次返回一个包
    // 可能是视频包, 也可能是音频包, 通过 stream_index 区分
    // 返回 0 表示成功, 负数表示文件结束或错误
    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        AVStream *stream = fmt_ctx->streams[pkt.stream_index];

        // 把 pts 转换为秒: pts * time_base
        double time_sec = -1;
        if (pkt.pts != AV_NOPTS_VALUE) {
            time_sec = pkt.pts * av_q2d(stream->time_base);
        }

        // AV_PKT_FLAG_KEY: 关键帧标志
        int is_key = (pkt.flags & AV_PKT_FLAG_KEY) ? 1 : 0;

        const char *type = (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) ? "V" : "A";

        printf("%-6d %-2s#%-3d %-10d %-12lld %-12lld %-10.3f %-5s\n",
               count, type, pkt.stream_index, pkt.size,
               (long long)pkt.pts, (long long)pkt.dts,
               time_sec, is_key ? "是" : "");

        // 每次 av_read_frame 后必须 unref, 释放包内部的数据引用
        av_packet_unref(&pkt);

        count++;
        if (count >= max_packets) {
            printf("... (已打印 %d 个包, 使用第二个参数调整数量)\n", max_packets);
            break;
        }
    }

    printf("\n共读取 %d 个包\n", count);

    avformat_close_input(&fmt_ctx);
    return 0;
}
