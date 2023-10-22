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
    av_log_set_level(AV_LOG_INFO); // 设置日志级别
    if (argc < 2) {
        av_log(nullptr, AV_LOG_ERROR, "Usage:%s infile\n", argv[0]);
        return -1;
    }

    const char *infile_name = argv[1];
    AVFormatContext *in_fmt_ctx = nullptr;

    avformat_open_input(&in_fmt_ctx, infile_name, nullptr, nullptr);
    avformat_find_stream_info(in_fmt_ctx, nullptr);
    av_dump_format(in_fmt_ctx, 0, infile_name, 0);
    av_log(nullptr, AV_LOG_INFO, "input file duration:%lld us, duration:%lf s\n",
           in_fmt_ctx->duration, (double)in_fmt_ctx->duration * av_q2d(AV_TIME_BASE_Q));

    AVRational video_timebase;
    AVRational audio_timebase;
    for (int i = 0; i < in_fmt_ctx->nb_streams; ++i) {
        AVStream *in_stream = in_fmt_ctx->streams[i];
        if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_timebase = in_stream->time_base;
            av_log(nullptr, AV_LOG_INFO, "video time base: num=%d, den=%d\n", video_timebase.num, video_timebase.den);
        }
        if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_timebase = in_stream->time_base;
            av_log(nullptr, AV_LOG_INFO, "audio time base: num=%d, den=%d\n", audio_timebase.num, audio_timebase.den);
        }
    }

    AVPacket packet;
    int ret = av_new_packet(&packet, sizeof(packet));
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "get new packet failed%s\n", av_err2str(ret));
        return -1;
    }
    while (av_read_frame(in_fmt_ctx, &packet) == 0) {
        AVStream *in_stream = in_fmt_ctx->streams[packet.stream_index];
        av_log(nullptr, AV_LOG_INFO, "stream_index=%d, pts=%lld, pts_time=%lf, dts=%lld, dts_time=%lf\n",
               packet.stream_index, packet.pts, (double)packet.pts * av_q2d(in_stream->time_base),
               packet.dts, (double)packet.dts * av_q2d(in_stream->time_base));
    }
    return 0;
}
