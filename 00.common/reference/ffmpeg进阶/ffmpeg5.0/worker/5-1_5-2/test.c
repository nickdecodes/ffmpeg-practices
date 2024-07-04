#include <stdio.h>
#include <libavutil/log.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

int main(int argc, char* argv[]){
    //printf("hello, world!\n");
    printf("hello world\n");

    AVFormatContext *pFmtCtx = NULL;
    avformat_open_input(&pFmtCtx, argv[1], NULL, NULL);

    av_dump_format(pFmtCtx, 0, argv[1], 0);
    
    const AVCodec *pCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    AVCodecContext *pCodecCtx = avcodec_alloc_context3(pCodec);

    av_log_set_level(AV_LOG_DEBUG);
    av_log(pCodecCtx, AV_LOG_INFO, "Hello world!");
    
    return 0;
}