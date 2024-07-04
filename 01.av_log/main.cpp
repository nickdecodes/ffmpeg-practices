/**
@Author  : zhengdongqi
@Email   : 
@Usage   :
@Filename: main.c
@DateTime: 2022/11/17 10:31
@Software: CLion
**/

#include<iostream>
#include<map>
extern "C" {
#include "libavutil/log.h"
}

using namespace std;

// 日志回调函数
void custom_log_callback(void* ptr, int level, const char* fmt, va_list vl) {
    static int print_prefix = 1;
    static char line[1024];
    av_log_format_line(ptr, level, fmt, vl, line, sizeof(line), &print_prefix);
    fprintf(stderr, "%s", line);
}

int main(int argc, char **argv) {
    // 检查命令行参数
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <log_level>" << endl;
        cerr << "Log levels: quiet, panic, fatal, error, warning, info, verbose, debug, trace" << endl;
        return -1;
    }

    // Map of log level names to constants
    map<string, int> logLevels = {
        {"quiet", AV_LOG_QUIET},
        {"panic", AV_LOG_PANIC},
        {"fatal", AV_LOG_FATAL},
        {"error", AV_LOG_ERROR},
        {"warning", AV_LOG_WARNING},
        {"info", AV_LOG_INFO},
        {"verbose", AV_LOG_VERBOSE},
        {"debug", AV_LOG_DEBUG},
        {"trace", AV_LOG_TRACE}
    };

    const string logLevelStr = argv[1];
    int logLevel = AV_LOG_INFO; // Default level

    // 获取设置的日志级别
    if (logLevels.find(logLevelStr) != logLevels.end()) {
        logLevel = logLevels[logLevelStr];
    } else {
        cerr << "Invalid log level: " << logLevelStr << endl;
        return -1;
    }

    // 设置FFmpeg日志回调函数和级别
    av_log_set_callback(custom_log_callback);
    av_log_set_level(logLevel);

    // 打印不同级别的日志信息
    av_log(nullptr, AV_LOG_ERROR, "This is an ERROR log level message!\n");
    av_log(nullptr, AV_LOG_INFO, "This is an INFO log level message!\n");
    av_log(nullptr, AV_LOG_WARNING, "This is a WARNING log level message!\n");
    av_log(nullptr, AV_LOG_DEBUG, "This is a DEBUG log level message!\n");

    return 0;
}
