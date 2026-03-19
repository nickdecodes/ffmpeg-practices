/**
 * ============================================================================
 * 15. 音频编码: PCM 编码为 AAC
 * ============================================================================
 *
 * 等价命令:
 *   ffmpeg -f s16le -ar 48000 -ac 2 -i input.pcm -c:a aac output.aac
 *
 * 和视频编码 (14) 的区别:
 *   视频编码每次送一帧 (一张图), 帧大小由分辨率决定。
 *   音频编码每次必须送 frame_size 个采样点, 这个值由编码器决定,
 *   不是你能随便设的。AAC 通常是 1024。
 *
 * 注意: 输出的 AAC 裸流需要 ADTS 头才能播放,
 *       这里用 avformat 的 muxer 来自动处理。
 */

#include <cstdio>
#include <cstring>
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/log.h"
#include "libavutil/opt.h"
#include "libavutil/channel_layout.h"
}

// ADTS 采样率索引表
static const int adts_rates[] = {
    96000, 88200, 64000, 48000, 44100, 32000,
    24000, 22050, 16000, 12000, 11025, 8000, 7350
};

static void write_adts_header(FILE *fp, int data_size, int profile,
                               int sample_rate, int channels) {
    int freq_idx = 3;
    for (int i = 0; i < 13; i++) {
        if (sample_rate == adts_rates[i]) { freq_idx = i; break; }
    }
    int frame_len = data_size + 7;
    char h[7];
    h[0] = (char)0xFF;
    h[1] = (char)0xF1;
    h[2] = (char)(((profile) << 6) | ((freq_idx & 0x0F) << 2) | ((channels & 0x04) >> 2));
    h[3] = (char)(((channels & 0x03) << 6) | ((frame_len >> 11) & 0x03));
    h[4] = (char)((frame_len >> 3) & 0xFF);
    h[5] = (char)(((frame_len & 0x07) << 5) | 0x1F);
    h[6] = (char)0xFC;
    fwrite(h, 1, 7, fp);
}

int main(int argc, char **argv) {
    if (argc < 5) {
        printf("用法: %s <输入.pcm> <输出.aac> <采样率> <声道数>\n", argv[0]);
        printf("示例: %s input.pcm output.aac 48000 2\n", argv[0]);
        printf("生成测试PCM: ./11_decode_audio_pcm input.mp4 input.pcm\n");
        return -1;
    }

    int sample_rate = atoi(argv[3]);
    int channels = atoi(argv[4]);

    AVCodecContext *enc_ctx = nullptr;
    FILE *in_fp = nullptr, *out_fp = nullptr;
    AVFrame *frame = nullptr;
    int ret = 0;

    // 查找 AAC 编码器
    const AVCodec *encoder = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!encoder) { printf("找不到 AAC 编码器\n"); return -1; }
    printf("编码器: %s\n", encoder->name);

    enc_ctx = avcodec_alloc_context3(encoder);
    enc_ctx->sample_rate = sample_rate;
    enc_ctx->channels = channels;
    enc_ctx->channel_layout = av_get_default_channel_layout(channels);
    // AAC 编码器通常要求 FLTP (float planar) 输入
    // 如果你的 PCM 是其他格式, 需要先用 swr_convert 转换
    enc_ctx->sample_fmt = AV_SAMPLE_FMT_FLTP;
    enc_ctx->bit_rate = 128000;

    ret = avcodec_open2(enc_ctx, encoder, nullptr);
    if (ret < 0) { printf("打开编码器失败: %s\n", av_err2str(ret)); goto end; }

    // frame_size: 编码器要求每次送入的采样点数, AAC 通常是 1024
    printf("frame_size: %d (每次必须送入这么多采样点)\n", enc_ctx->frame_size);

    frame = av_frame_alloc();
    frame->format = enc_ctx->sample_fmt;
    frame->nb_samples = enc_ctx->frame_size;
    frame->channels = channels;
    frame->channel_layout = enc_ctx->channel_layout;
    frame->sample_rate = sample_rate;
    av_frame_get_buffer(frame, 0);

    in_fp = fopen(argv[1], "rb");
    out_fp = fopen(argv[2], "wb");
    if (!in_fp || !out_fp) { printf("打开文件失败\n"); goto end; }

    {
    AVPacket pkt;
    memset(&pkt, 0, sizeof(pkt));
    int count = 0;

    // 输入 PCM 是 packed float (LRLRLR...), 需要转为 planar (LLL... RRR...)
    // 这里简单处理: 假设输入是 float planar 格式
    // 实际项目中应该用 swr_convert 做格式转换
    int frame_bytes = enc_ctx->frame_size * sizeof(float);

    while (1) {
        // 逐声道读取 (planar 格式)
        int got_data = 1;
        for (int ch = 0; ch < channels; ch++) {
            size_t rd = fread(frame->data[ch], 1, frame_bytes, in_fp);
            if (rd < (size_t)frame_bytes) {
                if (rd == 0 && ch == 0) { got_data = 0; break; }
                // 不足一帧, 补零
                memset(frame->data[ch] + rd, 0, frame_bytes - rd);
            }
        }
        if (!got_data) break;

        frame->pts = (int64_t)count * enc_ctx->frame_size;
        avcodec_send_frame(enc_ctx, frame);

        while (avcodec_receive_packet(enc_ctx, &pkt) == 0) {
            // 写 ADTS 头 + AAC 数据
            write_adts_header(out_fp, pkt.size, 1, sample_rate, channels);
            fwrite(pkt.data, 1, pkt.size, out_fp);
            av_packet_unref(&pkt);
        }
        count++;
    }

    // flush
    avcodec_send_frame(enc_ctx, nullptr);
    while (avcodec_receive_packet(enc_ctx, &pkt) == 0) {
        write_adts_header(out_fp, pkt.size, 1, sample_rate, channels);
        fwrite(pkt.data, 1, pkt.size, out_fp);
        av_packet_unref(&pkt);
    }

    printf("完成: %d 帧 -> %s\n", count, argv[2]);
    printf("验证: ffplay %s\n", argv[2]);
    }

end:
    if (in_fp) fclose(in_fp);
    if (out_fp) fclose(out_fp);
    if (frame) av_frame_free(&frame);
    if (enc_ctx) avcodec_free_context(&enc_ctx);
    return 0;
}
