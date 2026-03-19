/**
 * ============================================================================
 * 13. 视频缩放
 * ============================================================================
 *
 * 等价命令:
 *   ffmpeg -i input.mp4 -vf scale=640:480 -pix_fmt yuv420p output.yuv
 *
 * 在 10(解码视频) 的基础上, 加入 sws_scale 做分辨率缩放。
 *
 * 新增 API:
 *   sws_getContext()  创建缩放上下文
 *   sws_scale()       执行缩放
 *
 * sws_getContext 参数:
 *   源宽高 + 源像素格式 -> 目标宽高 + 目标像素格式 + 缩放算法
 *
 * 常用缩放算法:
 *   SWS_FAST_BILINEAR - 快速双线性, 速度优先
 *   SWS_BILINEAR      - 双线性, 平衡
 *   SWS_BICUBIC       - 双三次, 质量优先
 *   SWS_LANCZOS       - Lanczos, 最高质量但最慢
 */

#include <cstdio>
#include <cstring>
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavutil/parseutils.h"
#include "libavutil/log.h"
}

int main(int argc, char **argv) {
    if (argc < 4) {
        printf("用法: %s <输入文件> <目标尺寸> <输出.yuv>\n", argv[0]);
        printf("示例: %s input.mp4 640x480 output.yuv\n", argv[0]);
        return -1;
    }

    int dst_w = 0, dst_h = 0;
    if (av_parse_video_size(&dst_w, &dst_h, argv[2]) < 0) {
        printf("无效尺寸: %s\n", argv[2]);
        return -1;
    }

    AVFormatContext *fmt_ctx = nullptr;
    AVCodecContext *dec_ctx = nullptr;
    struct SwsContext *sws_ctx = nullptr;
    FILE *fp = nullptr;
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

    printf("%dx%d -> %dx%d\n", dec_ctx->width, dec_ctx->height, dst_w, dst_h);

    // === 新内容: 创建缩放上下文 ===
    sws_ctx = sws_getContext(
        dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt,   // 源
        dst_w, dst_h, AV_PIX_FMT_YUV420P,                    // 目标
        SWS_BILINEAR, nullptr, nullptr, nullptr);             // 算法
    if (!sws_ctx) { printf("创建缩放上下文失败\n"); goto end; }

    {
    // 分配目标帧缓冲区
    AVFrame *dst_frame = av_frame_alloc();
    dst_frame->format = AV_PIX_FMT_YUV420P;
    dst_frame->width = dst_w;
    dst_frame->height = dst_h;
    av_frame_get_buffer(dst_frame, 0);

    fp = fopen(argv[3], "wb");
    if (!fp) { printf("无法创建 %s\n", argv[3]); goto end; }

    AVPacket pkt;
    memset(&pkt, 0, sizeof(pkt));
    AVFrame *frame = av_frame_alloc();
    int count = 0;

    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        if (pkt.stream_index == video_idx) {
            avcodec_send_packet(dec_ctx, &pkt);
            while (avcodec_receive_frame(dec_ctx, frame) == 0) {
                // sws_scale: 执行缩放
                // 参数: 源data, 源linesize, 起始行, 源高度, 目标data, 目标linesize
                sws_scale(sws_ctx,
                          (const uint8_t *const *)frame->data, frame->linesize,
                          0, dec_ctx->height,
                          dst_frame->data, dst_frame->linesize);

                // 写入缩放后的 YUV
                for (int y = 0; y < dst_h; y++)
                    fwrite(dst_frame->data[0] + y * dst_frame->linesize[0], 1, dst_w, fp);
                for (int y = 0; y < dst_h / 2; y++)
                    fwrite(dst_frame->data[1] + y * dst_frame->linesize[1], 1, dst_w / 2, fp);
                for (int y = 0; y < dst_h / 2; y++)
                    fwrite(dst_frame->data[2] + y * dst_frame->linesize[2], 1, dst_w / 2, fp);

                count++;
                if (count % 100 == 0) printf("已处理 %d 帧...\n", count);
            }
        }
        av_packet_unref(&pkt);
    }

    // flush
    avcodec_send_packet(dec_ctx, nullptr);
    while (avcodec_receive_frame(dec_ctx, frame) == 0) {
        sws_scale(sws_ctx, (const uint8_t *const *)frame->data, frame->linesize,
                  0, dec_ctx->height, dst_frame->data, dst_frame->linesize);
        for (int y = 0; y < dst_h; y++)
            fwrite(dst_frame->data[0] + y * dst_frame->linesize[0], 1, dst_w, fp);
        for (int y = 0; y < dst_h / 2; y++)
            fwrite(dst_frame->data[1] + y * dst_frame->linesize[1], 1, dst_w / 2, fp);
        for (int y = 0; y < dst_h / 2; y++)
            fwrite(dst_frame->data[2] + y * dst_frame->linesize[2], 1, dst_w / 2, fp);
        count++;
    }

    printf("完成: %d 帧 -> %s\n", count, argv[3]);
    printf("播放: ffplay -video_size %dx%d -pixel_format yuv420p %s\n", dst_w, dst_h, argv[3]);

    av_frame_free(&frame);
    av_frame_free(&dst_frame);
    }

end:
    if (fp) fclose(fp);
    if (sws_ctx) sws_freeContext(sws_ctx);
    if (dec_ctx) avcodec_free_context(&dec_ctx);
    if (fmt_ctx) avformat_close_input(&fmt_ctx);
    return 0;
}
