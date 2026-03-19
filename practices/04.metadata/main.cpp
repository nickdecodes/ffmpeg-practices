/**
 * ============================================================================
 * 04. 读取文件元数据
 * ============================================================================
 *
 * 等价命令:
 *   ffprobe -show_entries format_tags input.mp4
 *
 * 新增 API:
 *   av_dict_get() - 遍历 AVDictionary 键值对
 *
 * AVDictionary 是 FFmpeg 中通用的键值对容器, 用于:
 *   - 文件元数据 (标题、作者、创建时间)
 *   - 传递选项参数 (如 avformat_open_input 的第四个参数)
 *   - 流级别的元数据 (如某路音频流的语言标签)
 */

#include <cstdio>
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/log.h"
#include "libavutil/dict.h"
}

int main(int argc, char **argv) {
    av_log_set_level(AV_LOG_INFO);

    if (argc < 2) {
        printf("用法: %s <输入文件>\n", argv[0]);
        return -1;
    }

    AVFormatContext *fmt_ctx = nullptr;
    int ret = avformat_open_input(&fmt_ctx, argv[1], nullptr, nullptr);
    if (ret < 0) {
        printf("打开失败: %s\n", av_err2str(ret));
        return -1;
    }
    avformat_find_stream_info(fmt_ctx, nullptr);

    // === 文件级元数据 ===
    // 存储在 fmt_ctx->metadata 中
    printf("=== 文件元数据 ===\n");
    if (fmt_ctx->metadata) {
        AVDictionaryEntry *tag = nullptr;
        // av_dict_get 遍历方式:
        //   第一个参数: AVDictionary
        //   第二个参数: "" 表示匹配所有 key
        //   第三个参数: 上一次返回的 entry, 首次传 nullptr
        //   第四个参数: AV_DICT_IGNORE_SUFFIX 表示前缀匹配
        // 每次调用返回下一个条目, 返回 nullptr 表示遍历结束
        while ((tag = av_dict_get(fmt_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
            printf("  %s = %s\n", tag->key, tag->value);
        }
    } else {
        printf("  (无)\n");
    }

    // === 流级元数据 ===
    // 每路流也可以有自己的元数据, 如音频流的语言标签
    for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++) {
        AVStream *stream = fmt_ctx->streams[i];
        const char *type = "未知";
        switch (stream->codecpar->codec_type) {
            case AVMEDIA_TYPE_VIDEO: type = "视频"; break;
            case AVMEDIA_TYPE_AUDIO: type = "音频"; break;
            case AVMEDIA_TYPE_SUBTITLE: type = "字幕"; break;
            default: break;
        }

        printf("\n=== 流 #%d (%s) 元数据 ===\n", i, type);
        if (stream->metadata) {
            AVDictionaryEntry *tag = nullptr;
            while ((tag = av_dict_get(stream->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
                printf("  %s = %s\n", tag->key, tag->value);
            }
        } else {
            printf("  (无)\n");
        }
    }

    avformat_close_input(&fmt_ctx);
    return 0;
}
