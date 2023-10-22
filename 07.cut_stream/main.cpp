/**
@Author  : zhengdongqi
@Email   : 
@Usage   :
@Filename: main.cpp
@DateTime: 2022/11/20 11:59
@Software: CLion
**/

//
// Created by 郑东琦 on 2022/11/20.
//

#include "StreamMedia.h"


int main(int argc, char **argv) {
    av_log_set_level(AV_LOG_DEBUG); // 设置日志级别
    if (argc < 5) {
        av_log(nullptr, AV_LOG_ERROR,
               "Usage:%s <input file name> <start time> <end time> <output file name> \n", argv[0]);
        return -1;
    }
    const char *in_filename = argv[1];
    int start_time = atoi(argv[2]);
    int end_time = atoi(argv[3]);
    const char *out_filename = argv[4];

    StreamMedia obj = StreamMedia();
    obj.set_in_filename(in_filename);
    obj.set_out_filename(out_filename);

    if (obj.cut_stream(start_time, end_time) != 0) {
        av_log(nullptr, AV_LOG_ERROR, "cut stream failed\n");
        return -1;
    }

    return 0;
}
