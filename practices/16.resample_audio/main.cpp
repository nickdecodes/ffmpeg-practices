/**
 * ============================================================================
 * 16. 音频重采样
 * ============================================================================
 *
 * 等价命令:
 *   ffmpeg -i input.mp4 -ar 16000 -ac 1 -f f32le output.pcm
 *
 * 新增 API: SwrContext (libswresample)
 *   类似视频的 SwsContext, 但用于音频:
 *   - 采样率转换: 44100Hz -> 16000Hz
 *   - 声道转换: 立体声 -> 单声道
 *   - 采样格式转换: fltp -> s16le
 *
 * swr_convert 参数:
 *   输出缓冲区, 输出采样数上限, 输入缓冲区, 输入采样数
 *   返回实际输出的采样数
 */

#include <cstdio>
#include <cstring>
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
#include "libavutil/opt.h"
#include "libavutil/channel_layout.h"
#include "libavutil/log.h"
}

int main(int argc, char **argv) {
    if (argc < 5) {
        printf("用法: %s <输入文件> <输出.pcm> <目标采样率> <目标声道数>\n", argv[0]);
        printf("示例: %s input.mp4 output.pcm 16000 1\n", argv[0]);
        return -1;
    }

    int dst_rate = atoi(argv[3]);
    int dst_channels = atoi(argv[4]);
    int64_t dst_layout = av_get_default_channel_layout(dst_channels);
    enum AVSampleFormat dst_fmt = AV_SAMPLE_FMT_FLT; // packed float

    AVFormatContext *fmt_ctx = nullptr;
    AVCodecContext *dec_ctx = nullptr;
    struct SwrContext *swr_ctx = nullptr;
    FILE *fp = nullptr;
    int ret = 0;

    ret = avformat_open_input(&fmt_ctx, argv[1], nullptr, nullptr);
    if (ret < 0) { printf("打开失败: %s\n", av_err2str(ret)); return -1; }
    avformat_find_stream_info(fmt_ctx, nullptr);

    int audio_idx = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (audio_idx < 0) { printf("没有音频流\n"); return -1; }

    dec_ctx = avcodec_alloc_context3(nullptr);
    avcodec_parameters_to_context(dec_ctx, fmt_ctx->streams[audio_idx]->codecpar);
    const AVCodec *decoder = avcodec_find_decoder(dec_ctx->codec_id);
    avcodec_open2(dec_ctx, decoder, nullptr);

    printf("源: %dHz, %d声道, %s\n", dec_ctx->sample_rate, dec_ctx->channels,
           av_get_sample_fmt_name(dec_ctx->sample_fmt));
    printf("目标: %dHz, %d声道, %s\n", dst_rate, dst_channels,
           av_get_sample_fmt_name(dst_fmt));

    // === 新内容: 创建重采样上下文 ===
    swr_ctx = swr_alloc_set_opts(nullptr,
        dst_layout, dst_fmt, dst_rate,                                    // 目标
        dec_ctx->channel_layout, dec_ctx->sample_fmt, dec_ctx->sample_rate, // 源
        0, nullptr);
    if (!swr_ctx || swr_init(swr_ctx) < 0) {
        printf("创建重采样上下文失败\n");
        goto end;
    }

    fp = fopen(argv[2], "wb");
    if (!fp) { printf("无法创建 %s\n", argv[2]); goto end; }

    {
    AVPacket pkt;
    memset(&pkt, 0, sizeof(pkt));
    AVFrame *frame = av_frame_alloc();

    // 输出缓冲区
    uint8_t *out_buf = nullptr;
    int out_linesize = 0;
    int max_out_samples = 4096;
    av_samples_alloc(&out_buf, &out_linesize, dst_channels,
                     max_out_samples, dst_fmt, 0);

    int total_samples = 0;

    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        if (pkt.stream_index == audio_idx) {
            avcodec_send_packet(dec_ctx, &pkt);
            while (avcodec_receive_frame(dec_ctx, frame) == 0) {
                // 计算输出采样数
                int out_samples = av_rescale_rnd(
                    swr_get_delay(swr_ctx, dec_ctx->sample_rate) + frame->nb_samples,
                    dst_rate, dec_ctx->sample_rate, AV_ROUND_UP);

                if (out_samples > max_out_samples) {
                    av_freep(&out_buf);
                    max_out_samples = out_samples;
                    av_samples_alloc(&out_buf, &out_linesize, dst_channels,
                                     max_out_samples, dst_fmt, 0);
                }

                // swr_convert: 执行重采样
                int converted = swr_convert(swr_ctx,
                    &out_buf, out_samples,
                    (const uint8_t **)frame->data, frame->nb_samples);

                if (converted > 0) {
                    int data_size = converted * dst_channels * av_get_bytes_per_sample(dst_fmt);
                    fwrite(out_buf, 1, data_size, fp);
                    total_samples += converted;
                }
            }
        }
        av_packet_unref(&pkt);
    }

    // flush 解码器
    avcodec_send_packet(dec_ctx, nullptr);
    while (avcodec_receive_frame(dec_ctx, frame) == 0) {
        int out_samples = av_rescale_rnd(
            swr_get_delay(swr_ctx, dec_ctx->sample_rate) + frame->nb_samples,
            dst_rate, dec_ctx->sample_rate, AV_ROUND_UP);
        int converted = swr_convert(swr_ctx, &out_buf, out_samples,
            (const uint8_t **)frame->data, frame->nb_samples);
        if (converted > 0) {
            fwrite(out_buf, 1, converted * dst_channels * av_get_bytes_per_sample(dst_fmt), fp);
            total_samples += converted;
        }
    }

    // flush 重采样器 (内部可能还有缓存)
    while (1) {
        int converted = swr_convert(swr_ctx, &out_buf, max_out_samples, nullptr, 0);
        if (converted <= 0) break;
        fwrite(out_buf, 1, converted * dst_channels * av_get_bytes_per_sample(dst_fmt), fp);
        total_samples += converted;
    }

    printf("完成: %d 个采样点 -> %s\n", total_samples, argv[2]);
    printf("播放: ffplay -f %s -ar %d -ac %d %s\n",
           av_get_sample_fmt_name(dst_fmt), dst_rate, dst_channels, argv[2]);

    av_freep(&out_buf);
    av_frame_free(&frame);
    }

end:
    if (fp) fclose(fp);
    if (swr_ctx) swr_free(&swr_ctx);
    if (dec_ctx) avcodec_free_context(&dec_ctx);
    if (fmt_ctx) avformat_close_input(&fmt_ctx);
    return 0;
}
