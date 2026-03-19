/**
 * ============================================================================
 * 05. 提取音频流为 AAC
 * ============================================================================
 *
 * 等价命令:
 *   ffmpeg -i input.mp4 -vn -acodec copy output.aac
 *
 * 新增 API:
 *   av_find_best_stream() - 自动查找指定类型的最佳流
 *
 * 关键知识 - ADTS 头:
 *   MP4 容器中的 AAC 数据是"裸"的, 没有帧同步信息。
 *   保存为独立的 .aac 文件时, 每个包前面要加 7 字节的 ADTS 头,
 *   告诉播放器: 这一帧有多大、采样率多少、几个声道。
 */

#include <cstdio>
#include <cstring>
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/log.h"
}

// ADTS 采样率索引表
static const int adts_sample_rates[] = {
    96000, 88200, 64000, 48000, 44100, 32000,
    24000, 22050, 16000, 12000, 11025, 8000, 7350
};

// 构造 7 字节 ADTS 头
static void build_adts_header(char *header, int data_size,
                               int profile, int sample_rate, int channels) {
    int freq_idx = 3; // 默认 48000
    for (int i = 0; i < 13; i++) {
        if (sample_rate == adts_sample_rates[i]) { freq_idx = i; break; }
    }
    int frame_len = data_size + 7;

    header[0] = (char)0xFF;
    header[1] = (char)0xF1;
    header[2] = (char)(((profile) << 6) | ((freq_idx & 0x0F) << 2) | ((channels & 0x04) >> 2));
    header[3] = (char)(((channels & 0x03) << 6) | ((frame_len >> 11) & 0x03));
    header[4] = (char)((frame_len >> 3) & 0xFF);
    header[5] = (char)(((frame_len & 0x07) << 5) | 0x1F);
    header[6] = (char)0xFC;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("用法: %s <输入文件> <输出.aac>\n", argv[0]);
        return -1;
    }

    AVFormatContext *fmt_ctx = nullptr;
    int ret = avformat_open_input(&fmt_ctx, argv[1], nullptr, nullptr);
    if (ret < 0) { printf("打开失败: %s\n", av_err2str(ret)); return -1; }
    avformat_find_stream_info(fmt_ctx, nullptr);

    // av_find_best_stream: 自动找到"最佳"的音频流
    // 参数: fmt_ctx, 流类型, 期望索引(-1=自动), 关联流(-1=无), 解码器输出, flags
    // 返回: 流索引 (>=0) 或错误码
    int audio_idx = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (audio_idx < 0) { printf("没有音频流\n"); return -1; }

    AVCodecParameters *par = fmt_ctx->streams[audio_idx]->codecpar;
    if (par->codec_id != AV_CODEC_ID_AAC) {
        printf("音频不是 AAC (是 %s), 本例仅支持 AAC\n", avcodec_get_name(par->codec_id));
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    printf("找到音频流 #%d: AAC, %dHz, %d声道\n", audio_idx, par->sample_rate, par->channels);

    FILE *fp = fopen(argv[2], "wb");
    if (!fp) { printf("无法创建 %s\n", argv[2]); return -1; }

    AVPacket pkt;
    memset(&pkt, 0, sizeof(pkt));
    int count = 0;

    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        if (pkt.stream_index == audio_idx) {
            char adts[7];
            build_adts_header(adts, pkt.size, par->profile, par->sample_rate, par->channels);
            fwrite(adts, 1, 7, fp);
            fwrite(pkt.data, 1, pkt.size, fp);
            count++;
        }
        av_packet_unref(&pkt);
    }

    printf("提取完成: %d 个音频包 -> %s\n", count, argv[2]);
    printf("验证: ffplay %s\n", argv[2]);

    fclose(fp);
    avformat_close_input(&fmt_ctx);
    return 0;
}
