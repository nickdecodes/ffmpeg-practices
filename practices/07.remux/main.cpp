/**
 * ============================================================================
 * 07. 转封装: MP4 转 FLV (或任意格式互转)
 * ============================================================================
 *
 * 等价命令:
 *   ffmpeg -i input.mp4 -c copy output.flv
 *
 * 这是第一个同时涉及输入和输出的例子。
 * 之前的例子只有输入侧 (open/read), 这里加上了完整的输出侧 (write)。
 *
 * 输出侧流程:
 *   avformat_alloc_output_context2()  创建输出上下文
 *   avformat_new_stream()             为每路流创建输出流
 *   avcodec_parameters_copy()         复制编码参数
 *   avio_open()                       打开输出文件
 *   avformat_write_header()           写文件头
 *   av_interleaved_write_frame()      写数据包 (循环)
 *   av_write_trailer()                写文件尾
 */

#include <cstdio>
#include <cstring>
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/log.h"
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("用法: %s <输入文件> <输出文件>\n", argv[0]);
        printf("示例: %s input.mp4 output.flv\n", argv[0]);
        return -1;
    }

    AVFormatContext *in_ctx = nullptr;
    AVFormatContext *out_ctx = nullptr;
    int *stream_map = nullptr;
    int ret = 0;

    // === 输入侧: 前面学过的 ===
    ret = avformat_open_input(&in_ctx, argv[1], nullptr, nullptr);
    if (ret < 0) { printf("打开输入失败: %s\n", av_err2str(ret)); return -1; }
    avformat_find_stream_info(in_ctx, nullptr);
    av_dump_format(in_ctx, 0, argv[1], 0);

    // === 输出侧: 新内容 ===

    // 1. 创建输出格式上下文
    // FFmpeg 根据文件扩展名自动选择格式: .flv->FLV, .ts->MPEGTS, .mkv->Matroska
    ret = avformat_alloc_output_context2(&out_ctx, nullptr, nullptr, argv[2]);
    if (ret < 0) { printf("创建输出上下文失败: %s\n", av_err2str(ret)); goto end; }

    // 2. 为每路输入流创建对应的输出流
    // stream_map[i] 记录: 输入流 i -> 输出流 stream_map[i]
    // 值为 -1 表示跳过该流
    stream_map = (int *)av_mallocz_array(in_ctx->nb_streams, sizeof(int));
    {
    int out_idx = 0;
    for (unsigned int i = 0; i < in_ctx->nb_streams; i++) {
        AVCodecParameters *in_par = in_ctx->streams[i]->codecpar;

        // 只保留音频、视频、字幕
        if (in_par->codec_type != AVMEDIA_TYPE_VIDEO &&
            in_par->codec_type != AVMEDIA_TYPE_AUDIO &&
            in_par->codec_type != AVMEDIA_TYPE_SUBTITLE) {
            stream_map[i] = -1;
            continue;
        }
        stream_map[i] = out_idx++;

        // avformat_new_stream: 在输出上下文中创建一路新流
        AVStream *out_stream = avformat_new_stream(out_ctx, nullptr);
        if (!out_stream) { printf("创建输出流失败\n"); goto end; }

        // 复制编码参数: 输入流 -> 输出流
        avcodec_parameters_copy(out_stream->codecpar, in_par);

        // codec_tag 必须清零
        // 不同容器对同一编码器用不同的 tag
        // 例如 H.264 在 MP4 中 tag 是 avc1, 在 FLV 中不同
        // 清零后让 FFmpeg 自动选择正确的 tag
        out_stream->codecpar->codec_tag = 0;
    }
    }

    // 3. 打开输出文件
    if (!(out_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&out_ctx->pb, argv[2], AVIO_FLAG_WRITE);
        if (ret < 0) { printf("打开输出文件失败: %s\n", av_err2str(ret)); goto end; }
    }

    // 4. 写文件头
    ret = avformat_write_header(out_ctx, nullptr);
    if (ret < 0) { printf("写文件头失败: %s\n", av_err2str(ret)); goto end; }

    printf("\n");
    av_dump_format(out_ctx, 0, argv[2], 1);

    // 5. 逐包读取, 转换时间戳, 写入
    {
    AVPacket pkt;
    int count = 0;
    while (av_read_frame(in_ctx, &pkt) >= 0) {
        if (pkt.stream_index >= (int)in_ctx->nb_streams ||
            stream_map[pkt.stream_index] < 0) {
            av_packet_unref(&pkt);
            continue;
        }

        AVStream *in_stream = in_ctx->streams[pkt.stream_index];
        pkt.stream_index = stream_map[pkt.stream_index];
        AVStream *out_stream = out_ctx->streams[pkt.stream_index];

        // 时间基转换: av_rescale_q(值, 源时间基, 目标时间基)
        // 例如: 源 time_base=1/12800, 目标 time_base=1/1000
        //       pts=12800 -> 转换后 pts=1000 (都表示1秒)
        pkt.pts = av_rescale_q(pkt.pts, in_stream->time_base, out_stream->time_base);
        pkt.dts = av_rescale_q(pkt.dts, in_stream->time_base, out_stream->time_base);
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;

        // av_interleaved_write_frame: 写入并自动按 DTS 排序
        // 确保音视频包交替写入, 大多数容器格式要求这样
        ret = av_interleaved_write_frame(out_ctx, &pkt);
        if (ret < 0) { printf("写包失败: %s\n", av_err2str(ret)); break; }

        av_packet_unref(&pkt);
        count++;
    }
    printf("\n共写入 %d 个包\n", count);
    }

    // 6. 写文件尾 (必须调用, 否则文件可能损坏)
    av_write_trailer(out_ctx);
    printf("完成: %s -> %s\n", argv[1], argv[2]);

end:
    if (stream_map) av_freep(&stream_map);
    if (out_ctx && !(out_ctx->oformat->flags & AVFMT_NOFILE))
        avio_closep(&out_ctx->pb);
    if (out_ctx) avformat_free_context(out_ctx);
    if (in_ctx) avformat_close_input(&in_ctx);
    return 0;
}
