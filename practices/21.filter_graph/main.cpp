/**
 * ============================================================================
 * 21. 滤镜系统 (AVFilter)
 * ============================================================================
 *
 * 等价命令:
 *   ffmpeg -i input.mp4 -vf "scale=640:480" -f rawvideo -pix_fmt yuv420p output.yuv
 *
 * FFmpeg 的滤镜系统是一个有向图 (AVFilterGraph):
 *
 *   [buffersrc] -> [scale] -> [drawbox] -> [buffersink]
 *       ^                                       |
 *       |                                       v
 *   送入解码帧                              取出处理后的帧
 *
 * buffersrc:  滤镜图的入口, 接收 AVFrame
 * buffersink: 滤镜图的出口, 输出 AVFrame
 * 中间可以串联任意数量的滤镜
 *
 * 滤镜描述字符串语法:
 *   "scale=640:480"                    单个滤镜
 *   "scale=640:480,drawbox=x=10:y=10"  多个滤镜用逗号连接
 */

#include <cstdio>
#include <cstring>
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersrc.h"
#include "libavfilter/buffersink.h"
#include "libavutil/opt.h"
#include "libavutil/log.h"
}

int main(int argc, char **argv) {
    if (argc < 4) {
        printf("用法: %s <输入文件> <滤镜描述> <输出.yuv>\n", argv[0]);
        printf("示例: %s input.mp4 \"scale=640:480\" output.yuv\n", argv[0]);
        return -1;
    }

    const char *filter_descr = argv[2];

    AVFormatContext *fmt_ctx = nullptr;
    AVCodecContext *dec_ctx = nullptr;
    AVFilterGraph *graph = nullptr;
    AVFilterContext *src_ctx = nullptr, *sink_ctx = nullptr;
    FILE *fp = nullptr;
    int ret = 0;

    avformat_open_input(&fmt_ctx, argv[1], nullptr, nullptr);
    avformat_find_stream_info(fmt_ctx, nullptr);
    int vi = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (vi < 0) { printf("没有视频流\n"); return -1; }

    dec_ctx = avcodec_alloc_context3(nullptr);
    avcodec_parameters_to_context(dec_ctx, fmt_ctx->streams[vi]->codecpar);
    avcodec_open2(dec_ctx, avcodec_find_decoder(dec_ctx->codec_id), nullptr);

    // === 新内容: 构建滤镜图 ===

    // 1. 创建滤镜图
    graph = avfilter_graph_alloc();

    // 2. 创建 buffersrc (输入端)
    // 需要告诉它输入帧的参数: 宽高、像素格式、时间基、宽高比
    {
    char args[512];
    snprintf(args, sizeof(args),
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
             dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt,
             fmt_ctx->streams[vi]->time_base.num, fmt_ctx->streams[vi]->time_base.den,
             dec_ctx->sample_aspect_ratio.num, dec_ctx->sample_aspect_ratio.den);

    const AVFilter *buffersrc = avfilter_get_by_name("buffer");
    ret = avfilter_graph_create_filter(&src_ctx, buffersrc, "in", args, nullptr, graph);
    if (ret < 0) { printf("创建 buffersrc 失败\n"); goto end; }

    // 3. 创建 buffersink (输出端)
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");
    ret = avfilter_graph_create_filter(&sink_ctx, buffersink, "out", nullptr, nullptr, graph);
    if (ret < 0) { printf("创建 buffersink 失败\n"); goto end; }

    // 4. 解析滤镜描述字符串, 连接 src -> 滤镜链 -> sink
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs = avfilter_inout_alloc();
    outputs->name = av_strdup("in");
    outputs->filter_ctx = src_ctx;
    outputs->pad_idx = 0;
    outputs->next = nullptr;
    inputs->name = av_strdup("out");
    inputs->filter_ctx = sink_ctx;
    inputs->pad_idx = 0;
    inputs->next = nullptr;

    ret = avfilter_graph_parse_ptr(graph, filter_descr, &inputs, &outputs, nullptr);
    if (ret < 0) { printf("解析滤镜描述失败: %s\n", filter_descr); goto end; }

    // 5. 配置滤镜图 (验证连接、协商格式)
    ret = avfilter_graph_config(graph, nullptr);
    if (ret < 0) { printf("配置滤镜图失败\n"); goto end; }

    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
    }

    printf("滤镜: %s\n", filter_descr);

    fp = fopen(argv[3], "wb");
    if (!fp) { printf("无法创建 %s\n", argv[3]); goto end; }

    {
    AVPacket pkt;
    memset(&pkt, 0, sizeof(pkt));
    AVFrame *frame = av_frame_alloc();
    AVFrame *filt_frame = av_frame_alloc();
    int count = 0;

    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        if (pkt.stream_index == vi) {
            avcodec_send_packet(dec_ctx, &pkt);
            while (avcodec_receive_frame(dec_ctx, frame) == 0) {
                // 送入滤镜图
                av_buffersrc_add_frame(src_ctx, frame);

                // 从滤镜图取出处理后的帧
                while (av_buffersink_get_frame(sink_ctx, filt_frame) >= 0) {
                    // 写 YUV
                    int w = filt_frame->width, h = filt_frame->height;
                    for (int y = 0; y < h; y++)
                        fwrite(filt_frame->data[0] + y * filt_frame->linesize[0], 1, w, fp);
                    for (int y = 0; y < h / 2; y++)
                        fwrite(filt_frame->data[1] + y * filt_frame->linesize[1], 1, w / 2, fp);
                    for (int y = 0; y < h / 2; y++)
                        fwrite(filt_frame->data[2] + y * filt_frame->linesize[2], 1, w / 2, fp);

                    count++;
                    if (count % 100 == 0) printf("已处理 %d 帧...\n", count);
                    av_frame_unref(filt_frame);
                }
            }
        }
        av_packet_unref(&pkt);
    }

    // 获取输出尺寸 (可能被滤镜改变了)
    int out_w = av_buffersink_get_w(sink_ctx);
    int out_h = av_buffersink_get_h(sink_ctx);

    printf("完成: %d 帧 -> %s\n", count, argv[3]);
    printf("播放: ffplay -video_size %dx%d -pixel_format yuv420p %s\n", out_w, out_h, argv[3]);

    av_frame_free(&frame);
    av_frame_free(&filt_frame);
    }

end:
    if (fp) fclose(fp);
    if (graph) avfilter_graph_free(&graph);
    if (dec_ctx) avcodec_free_context(&dec_ctx);
    if (fmt_ctx) avformat_close_input(&fmt_ctx);
    return 0;
}
