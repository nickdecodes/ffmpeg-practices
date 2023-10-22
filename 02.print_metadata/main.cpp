/**
@Author  : zhengdongqi
@Email   : 
@Usage   :
@Filename: main.py
@DateTime: 2022/11/17 22:45
@Software: CLion
**/

//
// Created by 郑东琦 on 2022/11/17.
//

#include <iostream>
#include <cstdlib>
#include "StreamMedia.h"


int main(int argc, char **argv) {
    av_log_set_level(AV_LOG_DEBUG); // 设置日志级别
    // av_log_set_level(AV_LOG_INFO); // 设置日志级别

    if (argc < 2) {
        av_log(nullptr, AV_LOG_ERROR, "Usage:%s <input file name>\n", argv[0]);
        return -1;
    }
    const char *in_filename = argv[1];
    StreamMedia obj = StreamMedia();
    obj.set_in_filename(in_filename);
    if (obj.print_metadata() != 0) {
        av_log(nullptr, AV_LOG_ERROR, "print metadata failed!\n");
        return -1;
    }
    return 0;
}