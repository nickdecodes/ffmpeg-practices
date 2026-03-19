/**
 * ============================================================================
 * 08. 裁剪视频片段
 * ============================================================================
 *
 * 等价命令:
 *   ffmpeg -i input.mp4 -ss 10 -t 5 -c copy output.mp4
 *
 * 在 07(转封装) 的基础上加两个东西:
 *   1. av_seek_frame 跳转到起始时间
 *   2. 读包时判断是否超过结束时间
 *   3. 时间戳偏移: 裁剪后的第一帧 pts 应该从 0 开始
 *
 * 新增 API:
 *   av_seek_frame() - 跳转到指定时间位置
 *
 * 关键帧对齐问题:
 *   av_seek_frame 只能跳到关键帧 (I帧) 的位置。
 *   如果你要从第 10 秒开始, 但最近的关键帧在第 8 秒,
 *   那实际会从第 8 秒开始, 前 2 秒可能花屏。
 *   这就是为什么 -c copy 裁剪不精确, 要精确裁剪需要重新编码。
 */

#include <cstdio>
#include <cstring>
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/log.h"
}

int main(int argc, char **argv) {
    if (argc < 5) {
        printf("用法: %s <输入> <起始秒> <持续秒> <输出>\n", argv[0]);
        printf("示例: %s input.mp4 10 5 output.mp4\n", argv[0]);
        return -1;
    }

    const char *in_file = argv[1];
    double start_sec = atof(argv[2]);
    double duration_sec = atof(argv[3]);
    const char *out_file = argv[4];
    double end_sec = start_sec + duration_sec;

    printf("裁剪: %.1f秒 ~ %.1f秒\n", start_sec, end_sec);

    AVFormatContext *in_ctx = nullptr, *out_ctx = nullptr;
    int ret = 0;

    ret = avformat_open_input(&in_ctx, in_file, nullptr, nullptr);
    if (ret < 0) { printf("打开输入失败: %s\n", av_err2str(ret)); return -1; }
    avformat_find_stream_info(in_ctx, nullptr);

    ret = avformat_alloc_output_context2(&out_ctx, nullptr, nullptr, out_file);
    if (ret < 0) { printf("创建输出失败: %s\n", av_err2str(ret)); goto end; }

    // 创建输出流 (同 07)
    for (unsigned int i = 0; i < in_ctx->nb_streams; i++) {
        AVStream *out_stream = avformat_new_stream(out_ctx, nullptr);
        avcodec_parameters_copy(out_stream->codecpar, in_ctx->streams[i]->codecpar);
        out_stream->codecpar->codec_tag = 0;
    }

    if (!(out_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&out_ctx->pb, out_file, AVIO_FLAG_WRITE);
        if (ret < 0) { printf("打开输出失败: %s\n", av_err2str(ret)); goto end; }
    }
    avformat_write_header(out_ctx, nullptr);

    // === 新内容: seek 到起始位置 ===
    // av_seek_frame 参数:
    //   stream_index: -1 表示按文件级别的时间基 (AV_TIME_BASE) seek
    //   timestamp:    目标时间, 单位取决于 stream_index
    //   flags:        AVSEEK_FLAG_BACKWARD 表示跳到目标时间之前最近的关键帧
    //
    // 为什么用 AVSEEK_FLAG_BACKWARD?
    //   视频只能从关键帧开始解码。如果目标时间不是关键帧,
    //   必须跳到之前的关键帧, 否则解码会出错。
    ret = av_seek_frame(in_ctx, -1, (int64_t)(start_sec * AV_TIME_BASE), AVSEEK_FLAG_BACKWARD);
    if (ret < 0) {
        printf("seek 失败: %s\n", av_err2str(ret));
        goto end;
    }

    {
    AVPacket pkt;
    int count = 0;

    // 记录每路流的起始时间戳, 用于偏移
    // 裁剪后的视频, 第一帧的 pts 应该从 0 开始
    int64_t *start_pts = (int64_t *)av_calloc(in_ctx->nb_streams, sizeof(int64_t));
    int *got_first = (int *)av_calloc(in_ctx->nb_streams, sizeof(int));

    while (av_read_frame(in_ctx, &pkt) >= 0) {
        AVStream *in_stream = in_ctx->streams[pkt.stream_index];
        AVStream *out_stream = out_ctx->streams[pkt.stream_index];

        // 判断是否超过结束时间
        double pkt_time = pkt.pts * av_q2d(in_stream->time_base);
        if (pkt_time > end_sec) {
            av_packet_unref(&pkt);
            break;
        }

        // 记录每路流的第一个包的时间戳
        if (!got_first[pkt.stream_index]) {
            start_pts[pkt.stream_index] = pkt.pts;
            got_first[pkt.stream_index] = 1;
        }

        // 时间戳偏移: 减去起始时间, 再转换时间基
        pkt.pts = av_rescale_q(pkt.pts - start_pts[pkt.stream_index],
                               in_stream->time_base, out_stream->time_base);
        pkt.dts = av_rescale_q(pkt.dts - start_pts[pkt.stream_index],
                               in_stream->time_base, out_stream->time_base);
        if (pkt.pts < 0) pkt.pts = 0;
        if (pkt.dts < 0) pkt.dts = 0;
        if (pkt.pts < pkt.dts) { av_packet_unref(&pkt); continue; }

        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;

        av_interleaved_write_frame(out_ctx, &pkt);
        av_packet_unref(&pkt);
        count++;
    }

    av_freep(&start_pts);
    av_freep(&got_first);
    printf("写入 %d 个包\n", count);
    }

    av_write_trailer(out_ctx);
    printf("完成: %s (%.1f秒 ~ %.1f秒) -> %s\n", in_file, start_sec, end_sec, out_file);

end:
    if (out_ctx && !(out_ctx->oformat->flags & AVFMT_NOFILE))
        avio_closep(&out_ctx->pb);
    if (out_ctx) avformat_free_context(out_ctx);
    if (in_ctx) avformat_close_input(&in_ctx);
    return 0;
}
