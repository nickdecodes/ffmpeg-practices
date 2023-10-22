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

    if (argc < 4) {
        av_log(nullptr, AV_LOG_ERROR,
               "Usage:%s <input file name> <output file name> <format name avcc or annexb> \n", argv[0]);
        return -1;
    }
    const char *in_filename = argv[1];
    const char *out_filename = argv[2];
    const char *fmt_name = argv[3];

    StreamMedia obj = StreamMedia();
    obj.set_in_filename(in_filename);
    obj.set_out_filename(out_filename);
    if (obj.demux_video_to_h264(fmt_name) != 0) {
        av_log(nullptr, AV_LOG_ERROR, "demux video to h264 failed\n");
        return -1;
    }

    return 0;
}
