/*************************************************************************
 @Author  : zhengdongqi
 @Email   : 
 @Usage   :
 @FileName: StreamMeida.cpp
 @DateTime: 2023/2/23 17:15
 @SoftWare: CLion"))
************************************************************************/

//
// Created by 郑东琦 on 2023/2/23.
//

#ifndef STREAMING_MEDIA_PRACTICE_STREAMMEDIA_H
#define STREAMING_MEDIA_PRACTICE_STREAMMEDIA_H


#include <iostream>
#include <cstdlib>
extern "C" {
#include "libavutil/log.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavcodec/bsf.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/parseutils.h"
#include "libavutil/imgutils.h"
#include "libavdevice/avdevice.h"
#include <libavutil/timestamp.h>
}

typedef struct tagBITMAPFILEHEADER {
    uint16_t	bfType;
    uint32_t	bfSize;
    uint16_t	bfReserved1;
    uint16_t	bfReserved2;
    uint32_t	bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
    uint32_t	biSize;
    int32_t	    biWidth;
    int32_t	    biHeight;
    uint16_t	biPlanes;
    uint16_t	biBitCount;
    uint32_t	biCompression;
    uint32_t	biSizeImage;
    int32_t	    biXPelsPerMeter;
    int32_t	    biYPelsPerMeter;
    uint32_t	biClrUsed;
    uint32_t	biClrImportant;
} BITMAPINFOHEADER;

class StreamMedia {
public:
    explicit StreamMedia();
    ~StreamMedia();

public:
    void set_in_filename(const char *in_filename);
    void set_out_filename(const char *out_filename);
    void set_encoder_name(const char *encoder_name);
    void set_decoder_name(const char *decoder_name);

public:
    /* 基础信息 */
    int print_metadata();
    static void show_avfoundation_devices();
    static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag);
    /* audio相关 */
    static int get_adts_header(char *adts_header, int packet_size, int profile, int sample_rate, int channels);
    int encode_audio(AVPacket *packet);
    int decode_audio(AVPacket *packet, const char *out_fmt=nullptr);
    /* video相关 */
    int avcc(int video_index);
    int annexb(int video_index);
    int encode_video(AVPacket *packet);
    int decode_video(AVPacket *packet, struct SwsContext *sws_ctx=nullptr, const char *dst_pix_fmt=nullptr,
                     int out_width=0, int out_height=0);
    /* audio实战 */
    int demux_audio_to_aac();
    int decode_audio_aac_to_pcm();
    int encode_audio_pcm_to_aac();
    /* video实战 */
    void save_bmp(const char *fileName, unsigned char *rgbData, int width, int height);
    int demux_video_to_h264(const char *fmt_name);
    int decode_video_to_yuv();
    int decode_video_scale_to_yuv(const char *out_scale);
    int decode_video_scale_to_rgb(const char *out_scale);
    int decode_video_scale_save_to_bmp(const char *out_scale);
    int encode_video_yuv_to_h264(const char *out_scale);
    /* 综合实战 */
    int cut_stream(int start_time, int end_time);
    int remux_mp4_to_flv();
    int capture_video_to_origin();
    int capture_video_to_yuv420();
    int capture_audio_to_pcm();

private:
    FILE *_p_in_fp; // 输入文件描述符
    const char *_p_in_filename; // 输入文件名
    AVFormatContext *_p_in_fmt_ctx; // 输入上下文
    FILE *_p_out_fp; // 输出文件描述符
    const char *_p_out_filename; // 输出文件名
    AVFormatContext *_p_out_fmt_ctx; // 输出上下文
    /* 编码相关 */
    const AVCodec *_p_encoder;
    const char *_p_encoder_name;
    AVCodecContext *_p_encoder_ctx;
    /* 解码相关 */
    const AVCodec *_p_decoder;
    const char *_p_decoder_name;
    AVCodecContext *_p_decoder_ctx;
    /* 码流过滤器相关 */
    const AVBitStreamFilter *_p_bsf; // 码流过滤器
    AVBSFContext *_p_bsf_ctx; // 码流过滤器上下文
    int *_p_stream_mapping; // 处理多路流
    /* 其它 */
    AVFrame *_p_out_frame;
    uint8_t *_p_out_buffer;

};


#endif //STREAMING_MEDIA_PRACTICE_STREAMMEDIA_H
