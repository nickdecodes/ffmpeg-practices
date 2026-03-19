/**
 * ============================================================================
 * 18. 完整转码: 迷你 ffmpeg
 * ============================================================================
 *
 * 等价命令:
 *   ffmpeg -i input.mp4 -c:v libx264 -c:a aac -s 1280x720 output.mp4
 *
 * 这是整个教程的终极目标: 把前面学的所有东西串成一条完整的管线。
 *
 *   视频: demux -> decode -> scale -> encode -> mux
 *   音频: demux -> decode -> resample -> encode -> mux
 *
 * 学完这个, 你就理解了 ffmpeg 命令行背后的完整工作流程。
 */

#include <cstdio>
#include <cstring>
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
#include "libavutil/channel_layout.h"
#include "libavutil/parseutils.h"
#include "libavutil/log.h"
}

int main(int argc, char **argv) {
    if (argc < 4) {
        printf("用法: %s <输入> <输出> <目标尺寸>\n", argv[0]);
        printf("示例: %s input.mp4 output.mp4 1280x720\n", argv[0]);
        return -1;
    }

    int dst_w = 0, dst_h = 0;
    av_parse_video_size(&dst_w, &dst_h, argv[3]);
    if (dst_w <= 0 || dst_h <= 0) { printf("无效尺寸: %s\n", argv[3]); return -1; }

    // 所有上下文
    AVFormatContext *in_ctx = nullptr, *out_ctx = nullptr;
    AVCodecContext *vdec_ctx = nullptr, *venc_ctx = nullptr;
    AVCodecContext *adec_ctx = nullptr, *aenc_ctx = nullptr;
    struct SwsContext *sws_ctx = nullptr;
    struct SwrContext *swr_ctx = nullptr;
    int v_in = -1, a_in = -1, v_out = -1, a_out = -1;
    int ret = 0;

    // === 输入 ===
    avformat_open_input(&in_ctx, argv[1], nullptr, nullptr);
    avformat_find_stream_info(in_ctx, nullptr);
    v_in = av_find_best_stream(in_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    a_in = av_find_best_stream(in_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);

    // === 视频解码器 ===
    if (v_in >= 0) {
        vdec_ctx = avcodec_alloc_context3(nullptr);
        avcodec_parameters_to_context(vdec_ctx, in_ctx->streams[v_in]->codecpar);
        avcodec_open2(vdec_ctx, avcodec_find_decoder(vdec_ctx->codec_id), nullptr);
        printf("视频解码: %s %dx%d -> %dx%d\n",
               avcodec_get_name(vdec_ctx->codec_id), vdec_ctx->width, vdec_ctx->height, dst_w, dst_h);
    }

    // === 音频解码器 ===
    if (a_in >= 0) {
        adec_ctx = avcodec_alloc_context3(nullptr);
        avcodec_parameters_to_context(adec_ctx, in_ctx->streams[a_in]->codecpar);
        avcodec_open2(adec_ctx, avcodec_find_decoder(adec_ctx->codec_id), nullptr);
        printf("音频解码: %s %dHz %dch\n",
               avcodec_get_name(adec_ctx->codec_id), adec_ctx->sample_rate, adec_ctx->channels);
    }

    // === 视频编码器 ===
    if (v_in >= 0) {
        const AVCodec *enc = avcodec_find_encoder_by_name("libx264");
        if (!enc) enc = avcodec_find_encoder(AV_CODEC_ID_H264);
        venc_ctx = avcodec_alloc_context3(enc);
        venc_ctx->width = dst_w;
        venc_ctx->height = dst_h;
        venc_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
        venc_ctx->time_base = in_ctx->streams[v_in]->time_base;
        venc_ctx->bit_rate = 2000000;
        venc_ctx->gop_size = 50;
        venc_ctx->max_b_frames = 0;
        if (strcmp(enc->name, "libx264") == 0)
            av_opt_set(venc_ctx->priv_data, "preset", "medium", 0);
        avcodec_open2(venc_ctx, enc, nullptr);

        // 缩放上下文
        sws_ctx = sws_getContext(vdec_ctx->width, vdec_ctx->height, vdec_ctx->pix_fmt,
                                 dst_w, dst_h, AV_PIX_FMT_YUV420P,
                                 SWS_BILINEAR, nullptr, nullptr, nullptr);
    }

    // === 音频编码器 ===
    if (a_in >= 0) {
        const AVCodec *enc = avcodec_find_encoder(AV_CODEC_ID_AAC);
        aenc_ctx = avcodec_alloc_context3(enc);
        aenc_ctx->sample_rate = 44100;
        aenc_ctx->channels = 2;
        aenc_ctx->channel_layout = AV_CH_LAYOUT_STEREO;
        aenc_ctx->sample_fmt = AV_SAMPLE_FMT_FLTP;
        aenc_ctx->bit_rate = 128000;
        avcodec_open2(aenc_ctx, enc, nullptr);

        // 重采样上下文
        swr_ctx = swr_alloc_set_opts(nullptr,
            aenc_ctx->channel_layout, aenc_ctx->sample_fmt, aenc_ctx->sample_rate,
            adec_ctx->channel_layout, adec_ctx->sample_fmt, adec_ctx->sample_rate,
            0, nullptr);
        swr_init(swr_ctx);
    }

    // === 输出 ===
    avformat_alloc_output_context2(&out_ctx, nullptr, nullptr, argv[2]);

    if (v_in >= 0) {
        AVStream *s = avformat_new_stream(out_ctx, nullptr);
        avcodec_parameters_from_context(s->codecpar, venc_ctx);
        s->codecpar->codec_tag = 0;
        s->time_base = venc_ctx->time_base;
        v_out = s->index;
    }
    if (a_in >= 0) {
        AVStream *s = avformat_new_stream(out_ctx, nullptr);
        avcodec_parameters_from_context(s->codecpar, aenc_ctx);
        s->codecpar->codec_tag = 0;
        s->time_base = (AVRational){1, aenc_ctx->sample_rate};
        a_out = s->index;
    }

    if (!(out_ctx->oformat->flags & AVFMT_NOFILE))
        avio_open(&out_ctx->pb, argv[2], AVIO_FLAG_WRITE);
    avformat_write_header(out_ctx, nullptr);

    // === 主循环 ===
    {
    AVPacket pkt;
    AVFrame *frame = av_frame_alloc();
    AVFrame *scaled = av_frame_alloc();
    if (v_in >= 0) {
        scaled->format = AV_PIX_FMT_YUV420P;
        scaled->width = dst_w;
        scaled->height = dst_h;
        av_frame_get_buffer(scaled, 0);
    }

    // 音频重采样输出帧
    AVFrame *resampled = nullptr;
    if (a_in >= 0) {
        resampled = av_frame_alloc();
        resampled->format = aenc_ctx->sample_fmt;
        resampled->nb_samples = aenc_ctx->frame_size;
        resampled->channels = aenc_ctx->channels;
        resampled->channel_layout = aenc_ctx->channel_layout;
        resampled->sample_rate = aenc_ctx->sample_rate;
        av_frame_get_buffer(resampled, 0);
    }

    int vcount = 0, acount = 0;
    int64_t audio_pts = 0;

    while (av_read_frame(in_ctx, &pkt) >= 0) {
        if (pkt.stream_index == v_in) {
            // 视频: decode -> scale -> encode
            avcodec_send_packet(vdec_ctx, &pkt);
            while (avcodec_receive_frame(vdec_ctx, frame) == 0) {
                sws_scale(sws_ctx, (const uint8_t *const *)frame->data, frame->linesize,
                          0, vdec_ctx->height, scaled->data, scaled->linesize);
                scaled->pts = frame->pts;
                scaled->pict_type = AV_PICTURE_TYPE_NONE;

                avcodec_send_frame(venc_ctx, scaled);
                AVPacket ep;
                memset(&ep, 0, sizeof(ep));
                while (avcodec_receive_packet(venc_ctx, &ep) == 0) {
                    ep.stream_index = v_out;
                    av_packet_rescale_ts(&ep, venc_ctx->time_base,
                                        out_ctx->streams[v_out]->time_base);
                    av_interleaved_write_frame(out_ctx, &ep);
                    av_packet_unref(&ep);
                }
                vcount++;
                if (vcount % 100 == 0) printf("视频: %d 帧...\n", vcount);
            }
        } else if (pkt.stream_index == a_in) {
            // 音频: decode -> resample -> encode
            avcodec_send_packet(adec_ctx, &pkt);
            while (avcodec_receive_frame(adec_ctx, frame) == 0) {
                // 重采样
                int out_samples = av_rescale_rnd(
                    swr_get_delay(swr_ctx, adec_ctx->sample_rate) + frame->nb_samples,
                    aenc_ctx->sample_rate, adec_ctx->sample_rate, AV_ROUND_UP);

                // 确保输出帧够大
                if (out_samples > resampled->nb_samples) {
                    av_frame_unref(resampled);
                    resampled->nb_samples = out_samples;
                    resampled->format = aenc_ctx->sample_fmt;
                    resampled->channels = aenc_ctx->channels;
                    resampled->channel_layout = aenc_ctx->channel_layout;
                    av_frame_get_buffer(resampled, 0);
                }

                int converted = swr_convert(swr_ctx,
                    resampled->data, out_samples,
                    (const uint8_t **)frame->data, frame->nb_samples);

                if (converted > 0) {
                    resampled->nb_samples = converted;
                    resampled->pts = audio_pts;
                    audio_pts += converted;

                    avcodec_send_frame(aenc_ctx, resampled);
                    AVPacket ep;
                    memset(&ep, 0, sizeof(ep));
                    while (avcodec_receive_packet(aenc_ctx, &ep) == 0) {
                        ep.stream_index = a_out;
                        av_packet_rescale_ts(&ep, aenc_ctx->time_base,
                                            out_ctx->streams[a_out]->time_base);
                        av_interleaved_write_frame(out_ctx, &ep);
                        av_packet_unref(&ep);
                    }
                }
                acount++;
            }
        }
        av_packet_unref(&pkt);
    }

    // flush 所有编解码器 (省略详细代码, 逻辑同 17)
    if (vdec_ctx) {
        avcodec_send_packet(vdec_ctx, nullptr);
        while (avcodec_receive_frame(vdec_ctx, frame) == 0) {
            sws_scale(sws_ctx, (const uint8_t *const *)frame->data, frame->linesize,
                      0, vdec_ctx->height, scaled->data, scaled->linesize);
            scaled->pts = frame->pts;
            scaled->pict_type = AV_PICTURE_TYPE_NONE;
            avcodec_send_frame(venc_ctx, scaled);
            AVPacket ep; memset(&ep, 0, sizeof(ep));
            while (avcodec_receive_packet(venc_ctx, &ep) == 0) {
                ep.stream_index = v_out;
                av_packet_rescale_ts(&ep, venc_ctx->time_base, out_ctx->streams[v_out]->time_base);
                av_interleaved_write_frame(out_ctx, &ep);
                av_packet_unref(&ep);
            }
        }
        avcodec_send_frame(venc_ctx, nullptr);
        AVPacket ep; memset(&ep, 0, sizeof(ep));
        while (avcodec_receive_packet(venc_ctx, &ep) == 0) {
            ep.stream_index = v_out;
            av_packet_rescale_ts(&ep, venc_ctx->time_base, out_ctx->streams[v_out]->time_base);
            av_interleaved_write_frame(out_ctx, &ep);
            av_packet_unref(&ep);
        }
    }
    if (adec_ctx) {
        avcodec_send_packet(adec_ctx, nullptr);
        while (avcodec_receive_frame(adec_ctx, frame) == 0) {
            int converted = swr_convert(swr_ctx, resampled->data, resampled->nb_samples,
                (const uint8_t **)frame->data, frame->nb_samples);
            if (converted > 0) {
                resampled->nb_samples = converted;
                resampled->pts = audio_pts; audio_pts += converted;
                avcodec_send_frame(aenc_ctx, resampled);
                AVPacket ep; memset(&ep, 0, sizeof(ep));
                while (avcodec_receive_packet(aenc_ctx, &ep) == 0) {
                    ep.stream_index = a_out;
                    av_packet_rescale_ts(&ep, aenc_ctx->time_base, out_ctx->streams[a_out]->time_base);
                    av_interleaved_write_frame(out_ctx, &ep);
                    av_packet_unref(&ep);
                }
            }
        }
        avcodec_send_frame(aenc_ctx, nullptr);
        AVPacket ep; memset(&ep, 0, sizeof(ep));
        while (avcodec_receive_packet(aenc_ctx, &ep) == 0) {
            ep.stream_index = a_out;
            av_packet_rescale_ts(&ep, aenc_ctx->time_base, out_ctx->streams[a_out]->time_base);
            av_interleaved_write_frame(out_ctx, &ep);
            av_packet_unref(&ep);
        }
    }

    printf("\n完成: 视频 %d 帧, 音频 %d 帧 -> %s\n", vcount, acount, argv[2]);
    printf("验证: ffplay %s\n", argv[2]);

    av_frame_free(&frame);
    av_frame_free(&scaled);
    if (resampled) av_frame_free(&resampled);
    }

    av_write_trailer(out_ctx);

    if (sws_ctx) sws_freeContext(sws_ctx);
    if (swr_ctx) swr_free(&swr_ctx);
    if (venc_ctx) avcodec_free_context(&venc_ctx);
    if (vdec_ctx) avcodec_free_context(&vdec_ctx);
    if (aenc_ctx) avcodec_free_context(&aenc_ctx);
    if (adec_ctx) avcodec_free_context(&adec_ctx);
    if (out_ctx && !(out_ctx->oformat->flags & AVFMT_NOFILE)) avio_closep(&out_ctx->pb);
    if (out_ctx) avformat_free_context(out_ctx);
    if (in_ctx) avformat_close_input(&in_ctx);
    return 0;
}
