/**
 * ============================================================================
 * 20. 像素格式转换: 解码并保存为 BMP 图片序列
 * ============================================================================
 *
 * 等价命令:
 *   ffmpeg -i input.mp4 -pix_fmt rgb24 -vframes 10 frame_%03d.bmp
 *
 * 在 09 (解码第一帧) 的基础上, 解码多帧并保存为 BMP 序列。
 * 重点是理解不同像素格式的区别。
 *
 * 常见像素格式:
 *   YUV420P  - 最常见的视频格式, Y/U/V 三个平面, U/V 分辨率减半
 *   NV12     - Y 一个平面, UV 交织一个平面 (硬件解码常用)
 *   RGB24    - R/G/B 交织, 每像素 3 字节
 *   BGR24    - B/G/R 交织 (BMP 文件用这个)
 *   RGBA     - 带透明通道, 每像素 4 字节
 */

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <sys/stat.h>
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
}

#pragma pack(push, 1)
struct BMPFileHeader { uint16_t type; uint32_t size; uint16_t r1, r2; uint32_t offset; };
struct BMPInfoHeader { uint32_t size; int32_t w, h; uint16_t planes, bpp; uint32_t comp, img_size; int32_t xppm, yppm; uint32_t clr_used, clr_imp; };
#pragma pack(pop)

static void save_bmp(const char *path, uint8_t *data, int w, int h) {
    int ds = w * h * 3;
    BMPFileHeader fh = {0x4D42, (uint32_t)(sizeof(fh)+sizeof(BMPInfoHeader)+ds), 0, 0, (uint32_t)(sizeof(fh)+sizeof(BMPInfoHeader))};
    BMPInfoHeader ih = {sizeof(ih), w, -h, 1, 24, 0, 0, 0, 0, 0, 0};
    FILE *fp = fopen(path, "wb");
    fwrite(&fh, 1, sizeof(fh), fp);
    fwrite(&ih, 1, sizeof(ih), fp);
    fwrite(data, 1, ds, fp);
    fclose(fp);
}

int main(int argc, char **argv) {
    if (argc < 4) {
        printf("用法: %s <输入文件> <输出目录> <帧数>\n", argv[0]);
        printf("示例: %s input.mp4 frames/ 10\n", argv[0]);
        return -1;
    }

    int max_frames = atoi(argv[3]);
    mkdir(argv[2], 0755);

    AVFormatContext *fmt_ctx = nullptr;
    AVCodecContext *dec_ctx = nullptr;
    struct SwsContext *sws_ctx = nullptr;

    avformat_open_input(&fmt_ctx, argv[1], nullptr, nullptr);
    avformat_find_stream_info(fmt_ctx, nullptr);
    int vi = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (vi < 0) { printf("没有视频流\n"); return -1; }

    dec_ctx = avcodec_alloc_context3(nullptr);
    avcodec_parameters_to_context(dec_ctx, fmt_ctx->streams[vi]->codecpar);
    avcodec_open2(dec_ctx, avcodec_find_decoder(dec_ctx->codec_id), nullptr);

    int w = dec_ctx->width, h = dec_ctx->height;
    printf("视频: %dx%d, 像素格式: %s -> BGR24\n", w, h,
           av_get_pix_fmt_name(dec_ctx->pix_fmt));

    // YUV -> BGR24 (BMP 用 BGR)
    sws_ctx = sws_getContext(w, h, dec_ctx->pix_fmt,
                              w, h, AV_PIX_FMT_BGR24,
                              SWS_BILINEAR, nullptr, nullptr, nullptr);

    uint8_t *rgb_buf = (uint8_t *)av_malloc(w * h * 3);
    uint8_t *rgb_data[1] = {rgb_buf};
    int rgb_linesize[1] = {w * 3};

    AVPacket pkt;
    memset(&pkt, 0, sizeof(pkt));
    AVFrame *frame = av_frame_alloc();
    int count = 0;

    while (av_read_frame(fmt_ctx, &pkt) >= 0 && count < max_frames) {
        if (pkt.stream_index == vi) {
            avcodec_send_packet(dec_ctx, &pkt);
            while (avcodec_receive_frame(dec_ctx, frame) == 0 && count < max_frames) {
                sws_scale(sws_ctx, (const uint8_t *const *)frame->data, frame->linesize,
                          0, h, rgb_data, rgb_linesize);

                char path[512];
                snprintf(path, sizeof(path), "%s/frame_%03d.bmp", argv[2], count);
                save_bmp(path, rgb_buf, w, h);
                printf("保存: %s\n", path);
                count++;
            }
        }
        av_packet_unref(&pkt);
    }

    printf("完成: %d 帧 -> %s/\n", count, argv[2]);

    av_freep(&rgb_buf);
    av_frame_free(&frame);
    sws_freeContext(sws_ctx);
    avcodec_free_context(&dec_ctx);
    avformat_close_input(&fmt_ctx);
    return 0;
}
