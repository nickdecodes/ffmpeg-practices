/**
 * ============================================================================
 * 09. 解码第一帧视频, 保存为 BMP
 * ============================================================================
 *
 * 等价命令:
 *   ffmpeg -i input.mp4 -vframes 1 -f image2 frame.bmp
 *
 * 这是第一个解码例子。之前都是操作压缩数据 (AVPacket),
 * 从这里开始接触原始数据 (AVFrame)。
 *
 * 解码器初始化四步:
 *   1. avcodec_alloc_context3()        分配上下文
 *   2. avcodec_parameters_to_context() 填充参数
 *   3. avcodec_find_decoder()          查找解码器
 *   4. avcodec_open2()                 打开解码器
 *
 * 解码循环:
 *   avcodec_send_packet()   送入压缩包
 *   avcodec_receive_frame() 取出原始帧
 *
 * 为什么是 send/receive 而不是一个函数搞定?
 *   因为视频有 B 帧, 解码顺序和显示顺序不同。
 *   解码器内部需要缓存, 一次 send 不一定能立即 receive,
 *   也可能一次 send 后能 receive 多帧。
 */

#include <cstdio>
#include <cstdint>
#include <cstring>
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavutil/log.h"
}

// BMP 文件头 (跨平台定义)
#pragma pack(push, 1)
struct BMPFileHeader {
    uint16_t type;        // 'BM'
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
};
struct BMPInfoHeader {
    uint32_t size;
    int32_t  width;
    int32_t  height;
    uint16_t planes;
    uint16_t bit_count;
    uint32_t compression;
    uint32_t image_size;
    int32_t  x_ppm;
    int32_t  y_ppm;
    uint32_t clr_used;
    uint32_t clr_important;
};
#pragma pack(pop)

static void save_bmp(const char *path, uint8_t *rgb_data, int width, int height) {
    int data_size = width * height * 3;
    BMPFileHeader fh = {0x4D42, (uint32_t)(sizeof(fh) + sizeof(BMPInfoHeader) + data_size),
                        0, 0, (uint32_t)(sizeof(fh) + sizeof(BMPInfoHeader))};
    BMPInfoHeader ih = {sizeof(ih), width, -height, 1, 24, 0, 0, 0, 0, 0, 0};

    FILE *fp = fopen(path, "wb");
    fwrite(&fh, 1, sizeof(fh), fp);
    fwrite(&ih, 1, sizeof(ih), fp);
    fwrite(rgb_data, 1, data_size, fp);
    fclose(fp);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("用法: %s <输入文件> <输出.bmp>\n", argv[0]);
        return -1;
    }

    AVFormatContext *fmt_ctx = nullptr;
    AVCodecContext *dec_ctx = nullptr;
    struct SwsContext *sws_ctx = nullptr;
    int ret = 0;

    // 打开文件, 找视频流 (前面学过的)
    ret = avformat_open_input(&fmt_ctx, argv[1], nullptr, nullptr);
    if (ret < 0) { printf("打开失败: %s\n", av_err2str(ret)); return -1; }
    avformat_find_stream_info(fmt_ctx, nullptr);

    int video_idx = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (video_idx < 0) { printf("没有视频流\n"); return -1; }

    AVCodecParameters *par = fmt_ctx->streams[video_idx]->codecpar;

    // === 新内容: 初始化解码器 ===

    // 1. 分配解码器上下文 (传 nullptr 表示先分配空的, 后面填参数)
    dec_ctx = avcodec_alloc_context3(nullptr);

    // 2. 把流的编码参数复制到解码器上下文
    //    codecpar 是"静态描述", dec_ctx 是"运行时状态"
    avcodec_parameters_to_context(dec_ctx, par);

    // 3. 根据 codec_id 查找解码器实现
    const AVCodec *decoder = avcodec_find_decoder(dec_ctx->codec_id);
    if (!decoder) { printf("找不到解码器\n"); goto end; }
    printf("解码器: %s, 视频: %dx%d\n", decoder->name, par->width, par->height);

    // 4. 打开解码器 (初始化内部状态)
    ret = avcodec_open2(dec_ctx, decoder, nullptr);
    if (ret < 0) { printf("打开解码器失败: %s\n", av_err2str(ret)); goto end; }

    // 准备 YUV->RGB 转换 (解码出来是 YUV, BMP 需要 RGB)
    sws_ctx = sws_getContext(
        dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt,  // 源
        dec_ctx->width, dec_ctx->height, AV_PIX_FMT_BGR24,  // 目标 (BMP 用 BGR)
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    {
    // 分配 RGB 帧缓冲区
    int rgb_size = av_image_get_buffer_size(AV_PIX_FMT_BGR24, dec_ctx->width, dec_ctx->height, 1);
    uint8_t *rgb_buf = (uint8_t *)av_malloc(rgb_size);
    uint8_t *rgb_data[4] = {rgb_buf, nullptr, nullptr, nullptr};
    int rgb_linesize[4] = {dec_ctx->width * 3, 0, 0, 0};

    AVPacket pkt;
    memset(&pkt, 0, sizeof(pkt));
    AVFrame *frame = av_frame_alloc();
    int got_frame = 0;

    // 逐包读取, 直到解码出第一帧
    while (av_read_frame(fmt_ctx, &pkt) >= 0 && !got_frame) {
        if (pkt.stream_index == video_idx) {
            // send: 送入压缩包
            ret = avcodec_send_packet(dec_ctx, &pkt);
            if (ret < 0) { av_packet_unref(&pkt); continue; }

            // receive: 取出解码帧
            ret = avcodec_receive_frame(dec_ctx, frame);
            if (ret == 0) {
                // 解码成功, 转换为 RGB
                sws_scale(sws_ctx,
                          (const uint8_t *const *)frame->data, frame->linesize,
                          0, dec_ctx->height,
                          rgb_data, rgb_linesize);

                save_bmp(argv[2], rgb_buf, dec_ctx->width, dec_ctx->height);
                printf("已保存第一帧: %s (%dx%d)\n", argv[2], dec_ctx->width, dec_ctx->height);
                got_frame = 1;
            }
            // ret == AVERROR(EAGAIN) 表示需要更多数据, 继续读包
        }
        av_packet_unref(&pkt);
    }

    if (!got_frame) printf("未能解码出任何帧\n");

    av_frame_free(&frame);
    av_freep(&rgb_buf);
    }

end:
    if (sws_ctx) sws_freeContext(sws_ctx);
    if (dec_ctx) avcodec_free_context(&dec_ctx);
    if (fmt_ctx) avformat_close_input(&fmt_ctx);
    return 0;
}
