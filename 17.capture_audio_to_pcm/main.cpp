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


// ffplay t.pcm -ar 44100 -ac 2 -f f32le
int main(int argc, char **argv) {
    av_log_set_level(AV_LOG_DEBUG);
    if (argc < 2) {
        av_log(nullptr, AV_LOG_ERROR, "Usage:%s <output file name>\n", argv[0]);
        return -1;
    }
    const char *out_filename = argv[1];

    StreamMedia obj = StreamMedia();
    obj.set_out_filename(out_filename);
    if (obj.capture_audio_to_pcm() != 0) {
        av_log(nullptr, AV_LOG_ERROR, "capture audio to pcm failed\n");
        return -1;
    }
    return 0;
}
