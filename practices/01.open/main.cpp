/**
 * ============================================================================
 * 01. 打开一个媒体文件
 * ============================================================================
 *
 * 等价命令:
 *   ffprobe -show_format input.mp4
 *
 * 本程序只做一件事: 打开文件, 打印封装格式信息。
 * 这是所有 FFmpeg 操作的第一步。
 *
 * 核心 API:
 *   avformat_open_input()  - 打开文件, 探测封装格式
 *   avformat_close_input() - 关闭文件, 释放资源
 */

#include <cstdio>
extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/log.h"
}

int main(int argc, char **argv) {
    av_log_set_level(AV_LOG_INFO);

    if (argc < 2) {
        printf("用法: %s <输入文件>\n", argv[0]);
        printf("示例: %s input.mp4\n", argv[0]);
        return -1;
    }

    // avformat_open_input 的第一个参数是 AVFormatContext 的二级指针
    // 传入 nullptr, 函数内部会自动分配
    AVFormatContext *fmt_ctx = nullptr;

    // 打开文件
    // 参数: &fmt_ctx  - 输出: 分配好的格式上下文
    //       argv[1]   - 文件路径 (也支持 rtmp://, http:// 等)
    //       nullptr    - 不强制指定格式, 让 FFmpeg 自动探测
    //       nullptr    - 不传额外选项
    // 返回: 0 成功, 负数失败
    int ret = avformat_open_input(&fmt_ctx, argv[1], nullptr, nullptr);
    if (ret < 0) {
        // av_err2str 把错误码转成可读字符串
        printf("打开失败: %s\n", av_err2str(ret));
        return -1;
    }

    // 打开成功后, fmt_ctx 里已经有了基本信息
    printf("文件: %s\n", argv[1]);
    printf("封装格式: %s\n", fmt_ctx->iformat->name);
    printf("格式全名: %s\n", fmt_ctx->iformat->long_name);

    // 用完必须关闭, 否则内存泄漏
    // 注意传的是二级指针, 函数内部会把 fmt_ctx 置为 nullptr
    avformat_close_input(&fmt_ctx);

    return 0;
}
