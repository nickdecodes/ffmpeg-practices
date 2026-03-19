/**
 * ============================================================================
 * 02. 查看文件里有哪些流
 * ============================================================================
 *
 * 等价命令:
 *   ffprobe -show_streams input.mp4
 *
 * 在 01 的基础上, 多了 avformat_find_stream_info 这一步。
 * 为什么需要? 因为 avformat_open_input 只读了文件头,
 * 有些格式 (如 MPEG-TS) 头部信息不完整, 需要实际读一些数据才能确定参数。
 *
 * 新增 API:
 *   avformat_find_stream_info() - 分析流信息
 */

#include <cstdio>
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/log.h"
}

int main(int argc, char **argv) {
    av_log_set_level(AV_LOG_INFO);

    if (argc < 2) {
        printf("用法: %s <输入文件>\n", argv[0]);
        return -1;
    }

    // === 01 学过的: 打开文件 ===
    AVFormatContext *fmt_ctx = nullptr;
    int ret = avformat_open_input(&fmt_ctx, argv[1], nullptr, nullptr);
    if (ret < 0) {
        printf("打开失败: %s\n", av_err2str(ret));
        return -1;
    }

    // === 新内容: 分析流信息 ===
    // 内部会读取一部分数据包, 尝试解码, 来确定每路流的编码参数
    ret = avformat_find_stream_info(fmt_ctx, nullptr);
    if (ret < 0) {
        printf("分析流信息失败: %s\n", av_err2str(ret));
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    // 现在可以遍历所有流了
    // fmt_ctx->nb_streams: 流的数量
    // fmt_ctx->streams[i]: 第 i 路流 (AVStream*)
    printf("文件: %s\n", argv[1]);
    printf("封装格式: %s\n", fmt_ctx->iformat->name);
    printf("流数量: %d\n", fmt_ctx->nb_streams);

    if (fmt_ctx->duration != AV_NOPTS_VALUE) {
        // duration 单位是 AV_TIME_BASE (微秒, 即 1000000)
        printf("时长: %.2f 秒\n", fmt_ctx->duration / (double)AV_TIME_BASE);
    }

    printf("\n");

    for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++) {
        AVStream *stream = fmt_ctx->streams[i];
        // AVCodecParameters: 存储编码参数, 不涉及编解码器的运行时状态
        AVCodecParameters *par = stream->codecpar;

        printf("--- 流 #%d ---\n", i);

        // time_base: 时间基, 是一个分数
        // pts * time_base = 实际秒数
        // 例如 time_base = 1/90000, pts = 90000, 则时间 = 1秒
        printf("  time_base: %d/%d\n", stream->time_base.num, stream->time_base.den);

        switch (par->codec_type) {
            case AVMEDIA_TYPE_VIDEO:
                printf("  类型: 视频\n");
                printf("  编码: %s\n", avcodec_get_name(par->codec_id));
                printf("  分辨率: %dx%d\n", par->width, par->height);
                if (stream->avg_frame_rate.den != 0) {
                    printf("  帧率: %.2f fps\n",
                           (double)stream->avg_frame_rate.num / stream->avg_frame_rate.den);
                }
                break;
            case AVMEDIA_TYPE_AUDIO:
                printf("  类型: 音频\n");
                printf("  编码: %s\n", avcodec_get_name(par->codec_id));
                printf("  采样率: %d Hz\n", par->sample_rate);
                printf("  声道数: %d\n", par->channels);
                break;
            case AVMEDIA_TYPE_SUBTITLE:
                printf("  类型: 字幕\n");
                printf("  编码: %s\n", avcodec_get_name(par->codec_id));
                break;
            default:
                printf("  类型: 其他 (%d)\n", par->codec_type);
                break;
        }
        printf("\n");
    }

    avformat_close_input(&fmt_ctx);
    return 0;
}
