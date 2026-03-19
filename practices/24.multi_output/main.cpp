/**
 * ============================================================================
 * 24. 一个输入, 多个输出 (多分辨率转码)
 * ============================================================================
 *
 * 等价命令:
 *   ffmpeg -i input.mp4 -c:v libx264 -s 1280x720 hd.mp4 -c:v libx264 -s 640x480 sd.mp4
 *
 * 一次解码, 缩放到两个分辨率, 分别编码输出。
 * 这是 ABR (自适应码率) 转码的基础。
 */

#include <cstdio>
#include <cstring>
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavutil/parseutils.h"
#include "libavutil/opt.h"
}

struct OutputCtx {
    AVFormatContext *fmt_ctx;
    AVCodecContext *enc_ctx;
    struct SwsContext *sws_ctx;
    AVFrame *scaled_frame;
    int stream_idx;
    int width, height;
};

static int init_output(OutputCtx *o, const char *filename, int src_w, int src_h,
                        enum AVPixelFormat src_fmt, AVRational time_base) {
    av_parse_video_size(&o->width, &o->height, filename); // 不用这个
    const AVCodec *enc = avcodec_find_encoder_by_name("libx264");
    if (!enc) enc = avcodec_find_encoder(AV_CODEC_ID_H264);

    o->enc_ctx = avcodec_alloc_context3(enc);
    o->enc_ctx->width = o->width;
    o->enc_ctx->height = o->height;
    o->enc_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    o->enc_ctx->time_base = time_base;
    o->enc_ctx->bit_rate = (int64_t)o->width * o->height * 3; // 简单按分辨率算码率
    o->enc_ctx->gop_size = 50;
    o->enc_ctx->max_b_frames = 0;
    if (strcmp(enc->name, "libx264") == 0)
        av_opt_set(o->enc_ctx->priv_data, "preset", "fast", 0);
    avcodec_open2(o->enc_ctx, enc, nullptr);

    avformat_alloc_output_context2(&o->fmt_ctx, nullptr, nullptr, filename);
    AVStream *s = avformat_new_stream(o->fmt_ctx, nullptr);
    avcodec_parameters_from_context(s->codecpar, o->enc_ctx);
    s->codecpar->codec_tag = 0;
    s->time_base = o->enc_ctx->time_base;
    o->stream_idx = s->index;

    if (!(o->fmt_ctx->oformat->flags & AVFMT_NOFILE))
        avio_open(&o->fmt_ctx->pb, filename, AVIO_FLAG_WRITE);
    avformat_write_header(o->fmt_ctx, nullptr);

    o->sws_ctx = sws_getContext(src_w, src_h, src_fmt,
                                 o->width, o->height, AV_PIX_FMT_YUV420P,
                                 SWS_BILINEAR, nullptr, nullptr, nullptr);

    o->scaled_frame = av_frame_alloc();
    o->scaled_frame->format = AV_PIX_FMT_YUV420P;
    o->scaled_frame->width = o->width;
    o->scaled_frame->height = o->height;
    av_frame_get_buffer(o->scaled_frame, 0);

    return 0;
}

static void encode_and_write(OutputCtx *o, AVFrame *src_frame) {
    // 缩放
    sws_scale(o->sws_ctx, (const uint8_t *const *)src_frame->data, src_frame->linesize,
              0, src_frame->height, o->scaled_frame->data, o->scaled_frame->linesize);
    o->scaled_frame->pts = src_frame->pts;
    o->scaled_frame->pict_type = AV_PICTURE_TYPE_NONE;

    // 编码
    avcodec_send_frame(o->enc_ctx, o->scaled_frame);
    AVPacket pkt;
    memset(&pkt, 0, sizeof(pkt));
    while (avcodec_receive_packet(o->enc_ctx, &pkt) == 0) {
        pkt.stream_index = o->stream_idx;
        av_packet_rescale_ts(&pkt, o->enc_ctx->time_base,
                            o->fmt_ctx->streams[o->stream_idx]->time_base);
        av_interleaved_write_frame(o->fmt_ctx, &pkt);
        av_packet_unref(&pkt);
    }
}

static void flush_and_close(OutputCtx *o) {
    // flush 编码器
    avcodec_send_frame(o->enc_ctx, nullptr);
    AVPacket pkt;
    memset(&pkt, 0, sizeof(pkt));
    while (avcodec_receive_packet(o->enc_ctx, &pkt) == 0) {
        pkt.stream_index = o->stream_idx;
        av_packet_rescale_ts(&pkt, o->enc_ctx->time_base,
                            o->fmt_ctx->streams[o->stream_idx]->time_base);
        av_interleaved_write_frame(o->fmt_ctx, &pkt);
        av_packet_unref(&pkt);
    }
    av_write_trailer(o->fmt_ctx);

    av_frame_free(&o->scaled_frame);
    sws_freeContext(o->sws_ctx);
    avcodec_free_context(&o->enc_ctx);
    if (!(o->fmt_ctx->oformat->flags & AVFMT_NOFILE))
        avio_closep(&o->fmt_ctx->pb);
    avformat_free_context(o->fmt_ctx);
}

int main(int argc, char **argv) {
    if (argc < 6) {
        printf("用法: %s <输入> <输出1> <尺寸1> <输出2> <尺寸2>\n", argv[0]);
        printf("示例: %s input.mp4 hd.mp4 1280x720 sd.mp4 640x480\n", argv[0]);
        return -1;
    }

    AVFormatContext *in_ctx = nullptr;
    AVCodecContext *dec_ctx = nullptr;

    avformat_open_input(&in_ctx, argv[1], nullptr, nullptr);
    avformat_find_stream_info(in_ctx, nullptr);
    int vi = av_find_best_stream(in_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (vi < 0) { printf("没有视频流\n"); return -1; }

    dec_ctx = avcodec_alloc_context3(nullptr);
    avcodec_parameters_to_context(dec_ctx, in_ctx->streams[vi]->codecpar);
    avcodec_open2(dec_ctx, avcodec_find_decoder(dec_ctx->codec_id), nullptr);

    // 初始化两个输出
    OutputCtx out1 = {}, out2 = {};
    av_parse_video_size(&out1.width, &out1.height, argv[3]);
    av_parse_video_size(&out2.width, &out2.height, argv[5]);

    init_output(&out1, argv[2], dec_ctx->width, dec_ctx->height,
                dec_ctx->pix_fmt, in_ctx->streams[vi]->time_base);
    init_output(&out2, argv[4], dec_ctx->width, dec_ctx->height,
                dec_ctx->pix_fmt, in_ctx->streams[vi]->time_base);

    printf("输入: %dx%d\n", dec_ctx->width, dec_ctx->height);
    printf("输出1: %s (%dx%d)\n", argv[2], out1.width, out1.height);
    printf("输出2: %s (%dx%d)\n", argv[4], out2.width, out2.height);

    AVPacket pkt;
    AVFrame *frame = av_frame_alloc();
    int count = 0;

    while (av_read_frame(in_ctx, &pkt) >= 0) {
        if (pkt.stream_index == vi) {
            avcodec_send_packet(dec_ctx, &pkt);
            while (avcodec_receive_frame(dec_ctx, frame) == 0) {
                // 一帧解码, 两路输出
                encode_and_write(&out1, frame);
                encode_and_write(&out2, frame);
                count++;
                if (count % 100 == 0) printf("已处理 %d 帧...\n", count);
            }
        }
        av_packet_unref(&pkt);
    }

    flush_and_close(&out1);
    flush_and_close(&out2);

    printf("完成: %d 帧 -> %s + %s\n", count, argv[2], argv[4]);

    av_frame_free(&frame);
    avcodec_free_context(&dec_ctx);
    avformat_close_input(&in_ctx);
    return 0;
}
