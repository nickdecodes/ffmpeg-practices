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


// ffplay t.yuv -pixel_format yuv420p -video_size 640x480
int main(int argc, char **argv) {
    av_log_set_level(AV_LOG_DEBUG);
    if (argc < 2) {
        av_log(nullptr, AV_LOG_ERROR, "Usage:%s <output file name>\n", argv[0]);
        return -1;
    }
    const char *out_filename = argv[1];

    StreamMedia obj = StreamMedia();
    obj.set_out_filename(out_filename);
    if (obj.capture_video_to_yuv420() != 0) {
        av_log(nullptr, AV_LOG_ERROR, "capture_video_to_yuv420 failed\n");
        return -1;
    }
    return 0;
}