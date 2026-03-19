/**
 * ============================================================================
 * 11. 解码音频为 PCM
 * ============================================================================
 *
 * 等价命令:
 *   ffmpeg -i input.mp4 -f f32le -acodec pcm_f32le output.pcm
 *
 * 音频解码流程和视频一样: send_packet / receive_frame。
 * 区别在于 AVFrame 的数据布局:
 *
 * 视频帧: data[0]=Y, data[1]=U, data[2]=V (按平面)
 * 音频帧: 取决于采样格式是 planar 还是 packed
 *
 *   packed (如 AV_SAMPLE_FMT_S16):
 *     data[0] = [L R L R L R ...]  所有声道交织在一起
 *
 *   planar (如 AV_SAMPLE_FMT_FLTP):
 *     data[0] = [L L L L ...]  左声道
 *     data[1] = [R R R R ...]  右声道
 *
 * 大多数解码器输出 planar 格式, 写文件时需要手动交织。
 */

#include <cstdio>
#include <cstring>
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/log.h"
#include "libavutil/samplefmt.h"
}

// 将一帧音频写入文件
// planar 格式需要手动交织: L0 R0 L1 R1 L2 R2 ...
static void write_audio_frame(FILE *fp, AVFrame *frame, AVCodecContext *dec_ctx) {
    int data_size = av_get_bytes_per_sample(dec_ctx->sample_fmt);

    if (av_sample_fmt_is_planar(dec_ctx->sample_fmt)) {
        // planar: 逐采样点交织各声道
        for (int i = 0; i < frame->nb_samples; i++) {
            for (int ch = 0; ch < dec_ctx->channels; ch++) {
                fwrite(frame->data[ch] + data_size * i, 1, data_size, fp);
            }
        }
    } else {
        // packed: 数据已经交织好了, 直接写
        fwrite(frame->data[0], 1, frame->nb_samples * dec_ctx->channels * data_size, fp);
    }
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("用法: %s <输入文件> <输出.pcm>\n", argv[0]);
        return -1;
    }

    AVFormatContext *fmt_ctx = nullptr;
    AVCodecContext *dec_ctx = nullptr;
    FILE *fp = nullptr;
    int ret = 0;

    ret = avformat_open_input(&fmt_ctx, argv[1], nullptr, nullptr);
    if (ret < 0) { printf("打开失败: %s\n", av_err2str(ret)); return -1; }
    avformat_find_stream_info(fmt_ctx, nullptr);

    int audio_idx = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (audio_idx < 0) { printf("没有音频流\n"); return -1; }

    // 初始化解码器 (同视频, 只是找的是音频解码器)
    dec_ctx = avcodec_alloc_context3(nullptr);
    avcodec_parameters_to_context(dec_ctx, fmt_ctx->streams[audio_idx]->codecpar);
    const AVCodec *decoder = avcodec_find_decoder(dec_ctx->codec_id);
    if (!decoder) { printf("找不到解码器\n"); goto end; }
    ret = avcodec_open2(dec_ctx, decoder, nullptr);
    if (ret < 0) { printf("打开解码器失败: %s\n", av_err2str(ret)); goto end; }

    printf("音频: %s, %dHz, %d声道, 格式: %s (%s)\n",
           decoder->name, dec_ctx->sample_rate, dec_ctx->channels,
           av_get_sample_fmt_name(dec_ctx->sample_fmt),
           av_sample_fmt_is_planar(dec_ctx->sample_fmt) ? "planar" : "packed");

    fp = fopen(argv[2], "wb");
    if (!fp) { printf("无法创建 %s\n", argv[2]); goto end; }

    {
    AVPacket pkt;
    memset(&pkt, 0, sizeof(pkt));
    AVFrame *frame = av_frame_alloc();
    int count = 0;

    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        if (pkt.stream_index == audio_idx) {
            avcodec_send_packet(dec_ctx, &pkt);
            while (avcodec_receive_frame(dec_ctx, frame) == 0) {
                write_audio_frame(fp, frame, dec_ctx);
                count++;
            }
        }
        av_packet_unref(&pkt);
    }

    // flush
    avcodec_send_packet(dec_ctx, nullptr);
    while (avcodec_receive_frame(dec_ctx, frame) == 0) {
        write_audio_frame(fp, frame, dec_ctx);
        count++;
    }

    // 计算 packed 后的采样格式名 (去掉 p 后缀)
    // fltp -> flt, s16p -> s16
    enum AVSampleFormat packed_fmt = av_get_packed_sample_fmt(dec_ctx->sample_fmt);

    printf("完成: %d 帧 -> %s\n", count, argv[2]);
    printf("播放: ffplay -f %s -ar %d -ac %d %s\n",
           av_get_sample_fmt_name(packed_fmt),
           dec_ctx->sample_rate, dec_ctx->channels, argv[2]);

    av_frame_free(&frame);
    }

end:
    if (fp) fclose(fp);
    if (dec_ctx) avcodec_free_context(&dec_ctx);
    if (fmt_ctx) avformat_close_input(&fmt_ctx);
    return 0;
}
