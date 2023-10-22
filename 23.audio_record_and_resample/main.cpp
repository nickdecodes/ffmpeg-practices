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
extern "C" {
#include "libswresample/swresample.h"
};


SwrContext *init_swr() {
    SwrContext *swr_ctx = nullptr;
    //channel, number/
    swr_ctx = swr_alloc_set_opts(nullptr,                         //ctx
                                 AV_CH_LAYOUT_STEREO, //输出channel布局
                                 AV_SAMPLE_FMT_S16,  //输出的采样格式
                                 44100,              //采样率
                                 AV_CH_LAYOUT_STEREO,   //输入channel布局
                                 AV_SAMPLE_FMT_FLT,    //输入的采样格式
                                 44100,               //输入的采样率
                                 0, nullptr);

    if (!swr_ctx) {

    }

    if (swr_init(swr_ctx) < 0) {

    }

    return swr_ctx;
}

void rec_audio() {

    int ret = 0;
    char errors[1024] = {0,};

    // 重采样缓冲区
    uint8_t **src_data = nullptr;
    int src_linesize = 0;

    uint8_t **dst_data = nullptr;
    int dst_linesize = 0;

    //ctx
    AVFormatContext *fmt_ctx = nullptr;
    AVDictionary *options = nullptr;

    //pakcet
    AVPacket pkt;

    //[[video device]:[audio device]]
    char *devicename = ":0";

    //set log level
    av_log_set_level(AV_LOG_DEBUG);

    //register audio device
    avdevice_register_all();

    //get format
    AVInputFormat *iformat = av_find_input_format("avfoundation");

    //open device
    if ((ret = avformat_open_input(&fmt_ctx, devicename, iformat, &options)) < 0) {
        av_strerror(ret, errors, 1024);
        fprintf(stderr, "Failed to open audio device, [%d]%s\n", ret, errors);
        return;
    }

    //create file
    char *out = "./audio.pcm";
    FILE *outfile = fopen(out, "wb+");

    SwrContext *swr_ctx = init_swr();

    //4096/4=1024/2=512
    //创建输入缓冲区
    av_samples_alloc_array_and_samples(&src_data,         //输出缓冲区地址
                                       &src_linesize,     //缓冲区的大小
                                       2,                 //通道个数
                                       512,               //单通道采样个数
                                       AV_SAMPLE_FMT_FLT, //采样格式
                                       0);

    //创建输出缓冲区
    av_samples_alloc_array_and_samples(&dst_data,         //输出缓冲区地址
                                       &dst_linesize,     //缓冲区的大小
                                       2,                 //通道个数
                                       512,               //单通道采样个数
                                       AV_SAMPLE_FMT_S16, //采样格式
                                       0);
    //read data from device
    while ((ret = av_read_frame(fmt_ctx, &pkt)) == 0) {
        av_log(nullptr, AV_LOG_INFO, "packet size is %d(%p)\n", pkt.size, pkt.data);

        // 进行内存拷贝，按字节拷贝的
        memcpy((void *) src_data[0], (void *) pkt.data, pkt.size);

        // 重采样
        swr_convert(swr_ctx,                    //重采样的上下文
                    dst_data,                   //输出结果缓冲区
                    512,                        //每个通道的采样数
                    (const uint8_t **) src_data, //输入缓冲区
                    512);                       //输入单个通道的采样数

        // write file
        // fwrite(pkt.data, 1, pkt.size, outfile);
        fwrite(dst_data[0], 1, dst_linesize, outfile);
        fflush(outfile);
        av_packet_unref(&pkt); //release pkt
    }

    // close file
    fclose(outfile);

    // 释放输入输出缓冲区
    if (src_data) {
        av_freep(&src_data[0]);
    }
    av_freep(src_data);

    if (dst_data) {
        av_freep(&dst_data[0]);
    }
    av_freep(dst_data);

    // 释放重采样的上下文
    swr_free(&swr_ctx);

    // close device and release ctx
    avformat_close_input(&fmt_ctx);

    av_log(nullptr, AV_LOG_DEBUG, "finish!\n");
}


int main(int argc, char *argv[]) {
    rec_audio();
    return 0;
}
