/**
 * ============================================================================
 * 22. 自定义 IO: 从内存读取视频
 * ============================================================================
 *
 * 无直接等价命令, 这是 API 独有的能力。
 *
 * 正常流程: avformat_open_input 打开文件, FFmpeg 内部用 file:// 协议读取。
 * 自定义 IO: 我们提供一个 read_packet 回调函数, FFmpeg 通过它来读数据。
 *            数据可以来自内存、网络、加密文件等任何地方。
 *
 * 新增 API:
 *   avio_alloc_context() - 创建自定义 IO 上下文
 *     参数: 缓冲区, 缓冲区大小, 是否可写, 用户数据指针,
 *           read_packet 回调, write_packet 回调, seek 回调
 *
 * 本例: 先把文件整个读入内存, 然后通过自定义 IO 让 FFmpeg 从内存读。
 * 实际应用: 加密视频解密后在内存中播放, 不落盘。
 */

#include <cstdio>
#include <cstring>
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/log.h"
}

// 自定义 IO 的用户数据
struct BufferData {
    uint8_t *ptr;      // 当前读取位置
    size_t   size;     // 剩余可读大小
};

// read_packet 回调: FFmpeg 需要数据时调用这个函数
// 参数: opaque=用户数据, buf=FFmpeg提供的缓冲区, buf_size=期望读取的大小
// 返回: 实际读取的字节数, 或 AVERROR_EOF
static int read_packet(void *opaque, uint8_t *buf, int buf_size) {
    BufferData *bd = (BufferData *)opaque;
    int read_size = buf_size;

    if (bd->size <= 0) return AVERROR_EOF;
    if (read_size > (int)bd->size) read_size = (int)bd->size;

    memcpy(buf, bd->ptr, read_size);
    bd->ptr += read_size;
    bd->size -= read_size;

    return read_size;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("用法: %s <输入文件>\n", argv[0]);
        return -1;
    }

    // 1. 把文件整个读入内存
    FILE *fp = fopen(argv[1], "rb");
    if (!fp) { printf("打开文件失败\n"); return -1; }
    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    uint8_t *file_data = (uint8_t *)av_malloc(file_size);
    fread(file_data, 1, file_size, fp);
    fclose(fp);
    printf("文件已读入内存: %zu bytes\n", file_size);

    // 2. 准备自定义 IO
    BufferData bd = {file_data, file_size};

    // avio_alloc_context 的内部缓冲区 (FFmpeg 用来做预读)
    int avio_buf_size = 4096;
    uint8_t *avio_buf = (uint8_t *)av_malloc(avio_buf_size);

    // 创建自定义 AVIOContext
    // 参数: 缓冲区, 大小, 0=只读, 用户数据, read回调, write回调, seek回调
    AVIOContext *avio_ctx = avio_alloc_context(
        avio_buf, avio_buf_size, 0, &bd, read_packet, nullptr, nullptr);

    // 3. 用自定义 IO 打开
    AVFormatContext *fmt_ctx = avformat_alloc_context();
    fmt_ctx->pb = avio_ctx;  // 关键: 把自定义 IO 赋给 pb

    // avformat_open_input 的第二个参数传空字符串 (因为不是从文件打开)
    int ret = avformat_open_input(&fmt_ctx, "", nullptr, nullptr);
    if (ret < 0) {
        printf("打开失败: %s\n", av_err2str(ret));
        goto end;
    }

    avformat_find_stream_info(fmt_ctx, nullptr);

    // 4. 正常使用, 和从文件打开完全一样
    printf("\n从内存读取成功:\n");
    printf("格式: %s\n", fmt_ctx->iformat->name);
    printf("流数量: %d\n", fmt_ctx->nb_streams);
    if (fmt_ctx->duration != AV_NOPTS_VALUE)
        printf("时长: %.2f 秒\n", fmt_ctx->duration / (double)AV_TIME_BASE);

    for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++) {
        AVCodecParameters *par = fmt_ctx->streams[i]->codecpar;
        const char *type = par->codec_type == AVMEDIA_TYPE_VIDEO ? "视频" :
                           par->codec_type == AVMEDIA_TYPE_AUDIO ? "音频" : "其他";
        printf("  流 #%d: %s, %s\n", i, type, avcodec_get_name(par->codec_id));
    }

    // 也可以正常 av_read_frame 读包、解码等, 和文件方式完全一样
    {
    AVPacket pkt;
    int count = 0;
    while (av_read_frame(fmt_ctx, &pkt) >= 0 && count < 10) {
        printf("  包 #%d: stream=%d, size=%d, pts=%lld\n",
               count, pkt.stream_index, pkt.size, (long long)pkt.pts);
        av_packet_unref(&pkt);
        count++;
    }
    printf("  ... (只打印前 10 个包)\n");
    }

end:
    if (fmt_ctx) avformat_close_input(&fmt_ctx);
    if (avio_ctx) {
        av_freep(&avio_ctx->buffer);
        avio_context_free(&avio_ctx);
    }
    av_freep(&file_data);
    return 0;
}
