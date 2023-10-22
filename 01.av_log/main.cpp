/**
@Author  : zhengdongqi
@Email   : 
@Usage   :
@Filename: main.c
@DateTime: 2022/11/17 10:31
@Software: CLion
**/

//
// Created by 郑东琦 on 2022/11/17.
//

#include <istream>
extern "C" {
#include "libavutil/log.h"
}


int main(int argc, char **argv) {
    av_log_set_level(AV_LOG_ERROR); // 设置log级别
    av_log(nullptr, AV_LOG_ERROR, "this is error log level!\n");
    av_log(nullptr, AV_LOG_INFO, "this is info log level! %d\n", argc);
    av_log(nullptr, AV_LOG_WARNING, "this is warning log level, %s\n", argv[0]);
    av_log(nullptr, AV_LOG_DEBUG, "this is debug log level!\n");
    return 0;
}
