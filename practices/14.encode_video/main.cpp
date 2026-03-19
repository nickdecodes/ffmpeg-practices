/**
 * ============================================================================
 * 14. 视频编码: YUV 编码为 H.264
 * ============================================================================
 *
 * 等价命令:
 *   ffmpeg -f rawvideo -pix_fmt yuv420p -s 640x480 -r 30 -i input.yuv
 *          -c:v libx264 -preset medium output.h264
 *
 * 编码是解码的镜像:
 *   解码: send_packet -> receive_frame  (压缩 -> 原始)
 *   编码: send_frame  -> receive_packet (原始 -> 压缩)
 *
 * 编码器关键参数:
 *   bit_rate     - 目标比特率, 越高画质越好文件越大
 *   gop_size     - 关键帧间隔, 越大压缩率越高但 seek 越慢
 *   max_b_frames - B帧数量, 0=低延迟, 2-3=高压缩
 *   preset       - libx264 专用, ultrafast~veryslow 速度/质量权衡
 */

#include <cstdio>
#include <cstring>
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/imgutils.h"
#include "libavutil/parseutils.h"
#include "libavutil/opt.h"
#include "libavutil/log.h"
}

int main(int argc, char **argv) {
    if (argc < 4) {
        printf("用法: %s <输入.yuv> <输出.h264> <宽x高>\n", argv[0]);
        printf("示例: %s test.yuv output.h264 640x480\n", argv[0]);
        return -1;
    }

    int width = 0, height = 0;
    if (av_parse_video_size(&width, &height, argv[3]) < 0) {
        printf("无效尺寸: %s\n", argv[3]);
        return -1;
    }

    AVCodecContext *enc_ctx = nullptr;
    FILE *in_fp = nullptr, *out_fp = nullptr;
    AVFrame *frame = nullptr;
    uint8_t *frame_buf = nullptr;
    int ret = 0;

    // === 查找编码器 ===
    // avcodec_find_encoder_by_name: 按名称找, 如 "libx264"
    // avcodec_find_encoder: 按 codec_id 找, 如 AV_CODEC_ID_H264
    const AVCodec *encoder = avcodec_find_encoder_by_name("libx264");
    if (!encoder) {
        printf("libx264 不可用, 尝试默认 H.264 编码器\n");
        encoder = avcodec_find_encoder(AV_CODEC_ID_H264);
    }
    if (!encoder) { printf("找不到 H.264 编码器\n"); return -1; }
    printf("编码器: %s\n", encoder->name);

    // === 配置编码器 ===
    enc_ctx = avcodec_alloc_context3(encoder);
    enc_ctx->width = width;
    enc_ctx->height = height;
    enc_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    enc_ctx->time_base = (AVRational){1, 30};
    enc_ctx->bit_rate = 800000;
    enc_ctx->gop_size = 50;
    enc_ctx->max_b_frames = 0;

    if (strcmp(encoder->name, "libx264") == 0) {
        av_opt_set(enc_ctx->priv_data, "preset", "medium", 0);
    }

    ret = avcodec_open2(enc_ctx, encoder, nullptr);
    if (ret < 0) { printf("打开编码器失败: %s\n", av_err2str(ret)); goto end; }

    // === 分配帧缓冲区 ===
    frame = av_frame_alloc();
    frame->format = AV_PIX_FMT_YUV420P;
    frame->width = width;
    frame->height = height;
    {
    int buf_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, width, height, 1);
    frame_buf = (uint8_t *)av_malloc(buf_size);
    av_image_fill_arrays(frame->data, frame->linesize, frame_buf,
                         AV_PIX_FMT_YUV420P, width, height, 1);

    in_fp = fopen(argv[1], "rb");
    out_fp = fopen(argv[2], "wb");
    if (!in_fp || !out_fp) { printf("打开文件失败\n"); goto end; }

    // === 编码循环 ===
    AVPacket pkt;
    memset(&pkt, 0, sizeof(pkt));
    int yuv_size = width * height * 3 / 2;
    int count = 0;

    while (fread(frame_buf, 1, yuv_size, in_fp) == (size_t)yuv_size) {
        frame->data[0] = frame_buf;
        frame->data[1] = frame_buf + width * height;
        frame->data[2] = frame_buf + width * height + width * height / 4;
        frame->pts = count;

        // send_frame: 送入原始帧
        avcodec_send_frame(enc_ctx, frame);

        // receive_packet: 取出压缩包
        while (avcodec_receive_packet(enc_ctx, &pkt) == 0) {
            fwrite(pkt.data, 1, pkt.size, out_fp);
            av_packet_unref(&pkt);
        }

        count++;
        if (count % 100 == 0) printf("已编码 %d 帧...\n", count);
    }

    // flush
    avcodec_send_frame(enc_ctx, nullptr);
    while (avcodec_receive_packet(enc_ctx, &pkt) == 0) {
        fwrite(pkt.data, 1, pkt.size, out_fp);
        av_packet_unref(&pkt);
    }

    printf("完成: %d 帧 -> %s\n", count, argv[2]);
    printf("验证: ffplay %s\n", argv[2]);
    }

end:
    if (in_fp) fclose(in_fp);
    if (out_fp) fclose(out_fp);
    if (frame_buf) av_freep(&frame_buf);
    if (frame) av_frame_free(&frame);
    if (enc_ctx) avcodec_free_context(&enc_ctx);
    return 0;
}
