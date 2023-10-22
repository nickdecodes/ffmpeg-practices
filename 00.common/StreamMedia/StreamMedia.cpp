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

#include "StreamMedia.h"

const int sample_frequency_table[] = {
        96000,  // 0x0
        88200,  // 0x1
        64000,  // 0x2
        48000,  // 0x3
        44100,  // 0x4
        32000,  // 0x5
        24000,  // 0x6
        22050,  // 0x7
        16000,  // 0x8
        12000,  // 0x9
        11025,  // 0xa
        8000,   // 0xb
        7350
};

StreamMedia::StreamMedia():
        _p_in_fp(nullptr), _p_in_filename(nullptr), _p_in_fmt_ctx(nullptr),
        _p_out_fp(nullptr), _p_out_filename(nullptr), _p_out_fmt_ctx(nullptr),
        _p_encoder(nullptr), _p_encoder_name(nullptr), _p_encoder_ctx(nullptr),
        _p_decoder(nullptr), _p_decoder_name(nullptr), _p_decoder_ctx(nullptr),
        _p_bsf(nullptr), _p_bsf_ctx(nullptr),
        _p_stream_mapping(nullptr),
        _p_out_frame(nullptr),
        _p_out_buffer(nullptr) {
}


StreamMedia::~StreamMedia() {
    /* 输入相关 */
    if (_p_in_fp) {
        fclose(_p_in_fp);
    }
    if (_p_in_filename) {
        _p_in_filename = nullptr;
    }
    if (_p_in_fmt_ctx) {
        avformat_close_input(&_p_in_fmt_ctx);
    }

    /* 输出相关 */
    if (_p_out_fp) {
        fclose(_p_out_fp);
    }
    if (_p_out_filename) {
        _p_out_filename = nullptr;
    }
    if (_p_out_fmt_ctx && !(_p_out_fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        avio_closep(&_p_out_fmt_ctx->pb);
    }
    if (_p_out_fmt_ctx) {
        avformat_free_context(_p_out_fmt_ctx);
    }

    /* 编码相关 */
    // if (_p_encoder) {
    //     _p_encoder = nullptr;
    // }
    if (_p_encoder_ctx) {
        avcodec_free_context(&_p_encoder_ctx);
    }

    /* 解码相关 */
    // if (_p_decoder) {
    //     _p_decoder = nullptr;
    // }
    if (_p_decoder_ctx) {
        avcodec_free_context(&_p_decoder_ctx);
    }

    /* 码流过滤器相关 */
    // if (_p_bsf) {
    //     _p_bsf = nullptr;
    // }
    if (_p_bsf_ctx) {
        av_bsf_free(&_p_bsf_ctx);
    }

    /* 处理多路流 */
    if (_p_stream_mapping) {
        av_freep(&_p_stream_mapping);
    }

    /* 其它 */
    if (_p_out_frame) {
        av_frame_free(&_p_out_frame);
    }
    if (_p_out_buffer) {
        av_freep(&_p_out_buffer);
    }
}

void StreamMedia::set_in_filename(const char *in_filename) { _p_in_filename = in_filename; }
void StreamMedia::set_out_filename(const char *out_filename) { _p_out_filename = out_filename; }
void StreamMedia::set_encoder_name(const char *encoder_name) { _p_encoder_name = encoder_name; }
void StreamMedia::set_decoder_name(const char *decoder_name) { _p_decoder_name = decoder_name; }

/*
 * 基础信息
 * */
int StreamMedia::print_metadata() {
    // 打开输入文件，获取格式上下文
    int ret = avformat_open_input(&_p_in_fmt_ctx, _p_in_filename, nullptr, nullptr);
    if (ret != 0) {
        av_log(nullptr, AV_LOG_ERROR, "open input file:%s failed:%s\n",
               _p_in_filename, av_err2str(ret));
        return -1;
    }

    // 输出文件流信息
    av_dump_format(_p_in_fmt_ctx, 0, _p_in_filename, 0);
    return 0;
}

void StreamMedia::show_avfoundation_devices() {
    AVFormatContext *p_fmt_ctx = avformat_alloc_context();
    if (p_fmt_ctx == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "avformat alloc failed!\n");
    }

    AVDictionary *options = nullptr;
    av_dict_set(&options, "list_devices", "true", 0);
    AVInputFormat *p_input_fmt = av_find_input_format("avfoundation");
    if (p_input_fmt == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "find avfoundation failed\n");
    }

    int ret = avformat_open_input(&p_fmt_ctx, "", p_input_fmt, &options);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "open input format failed: %s\n", av_err2str(ret));
    }
    if (p_fmt_ctx) {
        avformat_close_input(&p_fmt_ctx);
        avformat_free_context(p_fmt_ctx);
    }
}

void StreamMedia::log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag) {
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

    printf("%s: pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
           tag,
           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
           av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
           pkt->stream_index);
}

/*
 * audio相关
 * */
int StreamMedia::get_adts_header(char *adts_header, int packet_size, int profile, int sample_rate, int channels) {
    int sample_frequence_index = 3; // 默认使用48000hz
    int adts_length = packet_size + 7; // 不使用crc

    for (int i = 0; i < sizeof(sample_frequency_table) / sizeof(sample_frequency_table[0]); i++) {
        if(sample_rate == sample_frequency_table[i]) {
            sample_frequence_index = i;
            break;
        }
    }
    adts_header[0] = (0xff);		                            // syncword:0xfff 高8bits
    adts_header[1] = (0xf0);		                            // syncword:0xfff 低4bits
    adts_header[1] |= (0 << 3);	                                // MPEG Version:0 for MPEG-4,1 for MPEG-2  1bit
    adts_header[1] |= (0 << 1);	                                // Layer:0                                 2bits
    adts_header[1] |= 1;			                            // protection absent:1                     1bit

    adts_header[2] = (profile << 6);							// profile:profile               2bits
    adts_header[2] |= ((sample_frequence_index & 0x0f) << 2);	// sampling frequency index:sampling_frequency_index  4bits
    adts_header[2] |= (0 << 1);								    // private bit: 0                  1bit
    adts_header[2] |= (channels & 0x04) >> 2;				    // channel configuration:channels  高1bit

    adts_header[3] = (channels & 0x03) << 6;					// channel configuration:channels  低2bits
    adts_header[3] |= (0 << 5);								    // original:0			 1bit
    adts_header[3] |= (0 << 4);								    // home:0                1bit
    adts_header[3] |= (0 << 3);								    // copyright id bit:0    1bit
    adts_header[3] |= (0 << 2);								    // copyright id start:0	 1bit
    adts_header[3] |= ((adts_length & 0x1800) >> 11);			// frame length:value    高2bits
    adts_header[4] = (uint8_t)((adts_length & 0x7f8) >> 3);	    // frame length:value    中间8bits
    adts_header[5] = (uint8_t)((adts_length & 0x7) << 5);		// frame length:value    低3bits
    adts_header[5] |= 0x1f;									    // buffer fullness:0x7ff 高5bits
    adts_header[6] = 0xfc;									    // buffer fullness:0x7ff 低6bits
    // number_of_raw_data_blocks_in_frame：
    //    表示ADTS帧中有number_of_raw_data_blocks_in_frame + 1个AAC原始帧
    return 0;
}

int StreamMedia::encode_audio(AVPacket *packet) {
    int ret = avcodec_send_frame(_p_encoder_ctx, _p_out_frame);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "send frame to encoder failed: %s\n", av_err2str(ret));
        return -1;
    }
    while (ret >= 0) {
        ret = avcodec_receive_packet(_p_encoder_ctx, packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return 0;
        } else if(ret < 0) {
            av_log(nullptr, AV_LOG_ERROR, "encode frame failed: %s\n", av_err2str(ret));
            return -1;
        }
        fwrite(packet->data, 1, packet->size, _p_out_fp);
        av_packet_unref(packet);
    }
    return 0;
}

int StreamMedia::decode_audio(AVPacket *packet, const char *out_fmt) {
    int ret = avcodec_send_packet(_p_decoder_ctx, packet);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "send packet to decoder failed: %s\n", av_err2str(ret));
        return -1;
    }
    int i = 0, channel = 0;
    while (ret >= 0) {
        ret = avcodec_receive_frame(_p_decoder_ctx, _p_out_frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return 0;
        } else if (ret < 0) {
            av_log(nullptr, AV_LOG_ERROR, "decode packet failed: %s\n", av_err2str(ret));
        }

        if (strcmp(out_fmt, "f32le") == 0) {
            // f32le packed
            fwrite(_p_out_frame->data[0], 1, _p_out_frame->linesize[0], _p_out_fp);
        } else {
            int data_size = av_get_bytes_per_sample(_p_decoder_ctx->sample_fmt);
            if (data_size < 0) {
                av_log(nullptr, AV_LOG_ERROR, "get bytes failed!\n");
                return -1;
            }
            for (i = 0; i < _p_out_frame->nb_samples; i++) {
                for (channel = 0; channel < _p_decoder_ctx->channels; channel++) {
                    fwrite(_p_out_frame->data[channel] + data_size * i, 1, data_size, _p_out_fp);
                }
            }
        }
    }
    return 0;
}

/*
 * video相关
 * */
int StreamMedia::avcc(int video_index) {
    // 申请包结构
    AVPacket packet;
    av_new_packet(&packet, sizeof(AVPacket));

    while (av_read_frame(_p_in_fmt_ctx, &packet) == 0) {
        if (packet.stream_index == video_index) {
            int ret = fwrite(packet.data, 1, packet.size, _p_out_fp);
            if (ret != packet.size) {
                av_log(nullptr, AV_LOG_ERROR, "write file failed!\n");
                // 解引用
                av_packet_unref(&packet);
                return -1;
            }
        }
        // 解引用
        av_packet_unref(&packet);
    }
    return 0;
}

int StreamMedia::annexb(int video_index) {
    // 获取码流过滤器
    _p_bsf = av_bsf_get_by_name("h264_mp4toannexb");
    if (_p_bsf == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "get h264_mp4toannexb bsf failed\n");
        return -1;
    }
    // 申请码流过滤器上下文
    av_bsf_alloc(_p_bsf, &_p_bsf_ctx);
    avcodec_parameters_copy(_p_bsf_ctx->par_in, _p_in_fmt_ctx->streams[video_index]->codecpar);
    // 初始化码流过滤器上下文
    av_bsf_init(_p_bsf_ctx);

    // 申请包结构
    AVPacket packet;
    av_new_packet(&packet, sizeof(AVPacket));

    while (av_read_frame(_p_in_fmt_ctx, &packet) == 0) {
        if (packet.stream_index == video_index) {
            if (av_bsf_send_packet(_p_bsf_ctx, &packet) == 0) {
                while(av_bsf_receive_packet(_p_bsf_ctx, &packet) == 0) {
                    int ret = fwrite(packet.data, 1, size_t(packet.size), _p_out_fp);
                    if (ret != packet.size) {
                        av_log(nullptr, AV_LOG_ERROR, "write file failed!\n");
                        // 解引用
                        av_packet_unref(&packet);
                        return -1;
                    }
                }
            }
        }
        // 解引用
        av_packet_unref(&packet);
    }
    return 0;
}

int StreamMedia::encode_video(AVPacket *packet) {
    int write_packet_count = 0;
    int ret = avcodec_send_frame(_p_encoder_ctx, _p_out_frame);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "send frame to encoder failed: %s\n", av_err2str(ret));
        return -1;
    }
    while (ret >= 0) {
        ret = avcodec_receive_packet(_p_encoder_ctx, packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return 0;
        } else if (ret < 0) {
            av_log(nullptr, AV_LOG_ERROR, "encoder frame failed: %s\n", av_err2str(ret));
        }
        fwrite(packet->data, 1, packet->size, _p_out_fp);
        write_packet_count++;
        av_log(nullptr, AV_LOG_INFO, "write_packet_count: %d\n", write_packet_count);
        av_packet_unref(packet);
    }
    return 0;
}

int StreamMedia::decode_video(AVPacket *packet, SwsContext *sws_ctx, const char *dst_pix_fmt,
                              int out_width, int out_height) {
    int frame_count = 0;
    int ret = avcodec_send_packet(_p_decoder_ctx, packet);
    if (ret != 0) {
        av_log(nullptr, AV_LOG_ERROR, "send packet failed: %s\n", av_err2str(ret));
        return -1;
    }
    AVFrame *frame = av_frame_alloc();
    while (avcodec_receive_frame(_p_decoder_ctx, frame) == 0) {
        if (sws_ctx != nullptr) {
            sws_scale(sws_ctx, (const uint8_t * const*)frame->data, frame->linesize, 0,
                      _p_decoder_ctx->height, _p_out_frame->data, _p_out_frame->linesize);
        }
        if (strcmp(dst_pix_fmt, "yuv420") == 0) {
            fwrite(frame->data[0], 1, _p_decoder_ctx->width * _p_decoder_ctx->height, _p_out_fp);
            fwrite(frame->data[1], 1, _p_decoder_ctx->width * _p_decoder_ctx->height / 4, _p_out_fp);
            fwrite(frame->data[2], 1, _p_decoder_ctx->width * _p_decoder_ctx->height / 4, _p_out_fp);
            frame_count++;
            av_log(nullptr, AV_LOG_INFO, "frame_count: %d\n", frame_count);
            av_log(nullptr, AV_LOG_INFO,
                   "linesize[0] = %d, linesize[1] = %d, linesize[2] = %d, width = %d, heigth = %d\n",
                   frame->linesize[0], frame->linesize[1], frame->linesize[2], _p_decoder_ctx->width, _p_decoder_ctx->height);
        } else if (strcmp(dst_pix_fmt, "yuv422") == 0) {
            // yuv422
            fwrite(frame->data[0], 1, _p_decoder_ctx->width * _p_decoder_ctx->height * 2, _p_out_fp);
            frame_count++;
            av_log(nullptr, AV_LOG_INFO, "frame_count: %d\n", frame_count);
            av_log(nullptr, AV_LOG_INFO,
                   "linesize[0] = %d, linesize[1] = %d, linesize[2] = %d, width = %d, heigth = %d\n",
                   frame->linesize[0], frame->linesize[1], frame->linesize[2], _p_decoder_ctx->width, _p_decoder_ctx->height);
        } else if (strcmp(dst_pix_fmt, "rgb") == 0) {
            fwrite(_p_out_frame->data[0], 1, out_width * out_height * 3, _p_out_fp);
            frame_count++;
            av_log(nullptr, AV_LOG_INFO, "frame_count: %d\n", frame_count);
            av_log(nullptr, AV_LOG_INFO,
                   "linesize[0] = %d, linesize[1] = %d, linesize[2] = %d, width = %d, heigth = %d\n",
                   _p_out_frame->linesize[0], _p_out_frame->linesize[1], _p_out_frame->linesize[2], out_width, out_height);
        }

    }
    if (frame) {
        av_frame_free(&frame);
    }
    return 0;
}

/*
 * audio实战
 * */
int StreamMedia::demux_audio_to_aac() {
    int audio_index = 0;
    // 打开输入文件，获取格式上下文
    int ret = avformat_open_input(&_p_in_fmt_ctx, _p_in_filename, nullptr, nullptr);
    if (ret != 0) {
        av_log(nullptr, AV_LOG_ERROR, "open input file format failed:%s\n", av_err2str(ret));
        return -1;
    }

    // 查找流信息
    ret = avformat_find_stream_info(_p_in_fmt_ctx, nullptr);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "find stream info failed:%s\n", av_err2str(ret));
        return -1;
    }

    // 查找目标流索引
    audio_index = av_find_best_stream(_p_in_fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (audio_index < 0) {
        av_log(nullptr, AV_LOG_ERROR, "find best stream failed, index is %d\n", audio_index);
        return -1;
    }
    av_log(nullptr, AV_LOG_INFO, "audio index is %d\n", audio_index);

    // 申请包结构
    AVPacket packet;
    av_new_packet(&packet, sizeof(AVPacket));

    // 打开输出文件，拿到文件描述符
    _p_out_fp = fopen(_p_out_filename, "wb");
    if (_p_out_fp == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "open %s file failed\n", _p_out_filename);
        return -1;
    }

    // 读包
    while (av_read_frame(_p_in_fmt_ctx, &packet) == 0) {
        if (packet.stream_index == audio_index) {
            char adts_header[7] = {0};
            // 加载ADTS头
            get_adts_header(adts_header, packet.size,
                            _p_in_fmt_ctx->streams[audio_index]->codecpar->profile,
                            _p_in_fmt_ctx->streams[audio_index]->codecpar->sample_rate,
                            _p_in_fmt_ctx->streams[audio_index]->codecpar->channels);
            ret = fwrite(adts_header, 1, sizeof(adts_header), _p_out_fp);
            if (ret != sizeof(adts_header)) {
                av_log(nullptr, AV_LOG_ERROR, "write adts_header failed!\n");
                av_packet_unref(&packet);
                return -1;
            }
            ret = fwrite(packet.data, 1, packet.size, _p_out_fp);
            if (ret != packet.size) {
                av_log(nullptr, AV_LOG_ERROR, "write file failed!\n");
                av_packet_unref(&packet);
                return -1;
            }
        }
        // 解引用
        av_packet_unref(&packet);
    }
    return 0;
}

int StreamMedia::decode_audio_aac_to_pcm() {
    int audio_index = 0;
    int ret = avformat_open_input(&_p_in_fmt_ctx, _p_in_filename, nullptr, nullptr);
    if (ret != 0) {
        av_log(nullptr, AV_LOG_ERROR, "open input file failed: %s\n", av_err2str(ret));
        return -1;
    }

    ret = avformat_find_stream_info(_p_in_fmt_ctx, nullptr);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "find stream info failed: %s\n", av_err2str(ret));
        return -1;
    }

    ret = av_find_best_stream(_p_in_fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "find best stream failed: %s\n", av_err2str(ret));
        return -1;
    }
    audio_index = ret;

    _p_decoder_ctx = avcodec_alloc_context3(nullptr);
    if (_p_decoder_ctx == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "alloc decoder context failed: %s\n", av_err2str(ret));
        return -1;
    }
    avcodec_parameters_to_context(_p_decoder_ctx, _p_in_fmt_ctx->streams[audio_index]->codecpar);
    _p_decoder = avcodec_find_decoder(_p_decoder_ctx->codec_id);
    if (_p_decoder == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "find decoder %d failed!\n", _p_decoder_ctx->codec_id);
        return -1;
    }

    ret = avcodec_open2(_p_decoder_ctx, _p_decoder, nullptr);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "open decoder failed: %s\n", av_err2str(ret));
        return -1;
    }

    _p_out_frame = av_frame_alloc();
    int frame_size = av_samples_get_buffer_size(nullptr, _p_decoder_ctx->channels,
                                                _p_out_frame->nb_samples, _p_decoder_ctx->sample_fmt, 1);
    _p_out_buffer = (uint8_t *)av_malloc(frame_size);
    avcodec_fill_audio_frame(_p_out_frame, _p_decoder_ctx->channels, _p_decoder_ctx->sample_fmt,
                             _p_out_buffer, frame_size, 1);

    _p_out_fp = fopen(_p_out_filename, "wb+");
    if (_p_out_fp == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "open outfile %s failed!\n", _p_out_filename);
        return -1;
    }

    AVPacket packet;
    av_new_packet(&packet, sizeof(AVPacket));

    while (av_read_frame(_p_in_fmt_ctx, &packet) >= 0) {
        if (packet.stream_index == audio_index) {
            decode_audio(&packet);
        }
        av_packet_unref(&packet);
    }
    decode_audio(nullptr);
    return 0;
}

int StreamMedia::encode_audio_pcm_to_aac() {
    _p_out_frame = av_frame_alloc();
    if (_p_out_frame == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "alloc frame failed\n");
        return -1;
    }
    _p_out_frame->sample_rate = 48000;
    _p_out_frame->channels = 2;
    _p_out_frame->channel_layout = AV_CH_LAYOUT_STEREO;
    _p_out_frame->format = AV_SAMPLE_FMT_S16;
    _p_out_frame->nb_samples = 1024;
    av_frame_get_buffer(_p_out_frame, 0);


    _p_encoder = avcodec_find_encoder_by_name("libfdk_aac");
    if (_p_encoder == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "find encoder failed!\n");
        return -1;
    }

    _p_encoder_ctx = avcodec_alloc_context3(_p_encoder);
    if (_p_encoder_ctx == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "alloc encoder context failed!\n");
        return -1;
    }
    _p_encoder_ctx->sample_fmt = (AVSampleFormat)_p_out_frame->format;
    _p_encoder_ctx->sample_rate = _p_out_frame->sample_rate;
    _p_encoder_ctx->channels = _p_out_frame->channels;
    _p_encoder_ctx->channel_layout = _p_out_frame->channel_layout;

    int ret = avcodec_open2(_p_encoder_ctx, _p_encoder, nullptr);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "open encoder failed: %s\n", av_err2str(ret));
        return -1;
    }

    _p_in_fp = fopen(_p_in_filename, "rb");
    if (_p_in_fp == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "open infile: %s failed!\n", _p_in_filename);
        return -1;
    }

    _p_out_fp = fopen(_p_out_filename, "wb+");
    if (_p_out_fp == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "open outfile: %s failed!\n", _p_out_filename);
        return -1;
    }

    AVPacket packet;
    av_new_packet(&packet, sizeof(AVPacket));

    while (true) {
        // packed  L R L R L R L R
        // 2 * 2 * 1024 = 4096
        int readSize = fread(_p_out_frame->data[0], 1, _p_out_frame->linesize[0], _p_in_fp);
        if (readSize == 0) {
            av_log(nullptr, AV_LOG_INFO, "finish read infile!\n");
            break;
        }
        encode_audio(&packet);
    }
    encode_audio(&packet);
    return 0;
}

/*
 * video实战
 * */
void StreamMedia::save_bmp(const char *fileName, unsigned char *rgbData, int width, int height) {
    int bmpDataSize = width * height * 3;
    BITMAPFILEHEADER bmpFileHeader = {0};
    bmpFileHeader.bfType = 0x4d42;
    bmpFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bmpDataSize;
    bmpFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    BITMAPINFOHEADER bmpInfoHeader = {0};
    bmpInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpInfoHeader.biWidth = width;
    bmpInfoHeader.biHeight = height * (-1);
    bmpInfoHeader.biPlanes = 1;
    bmpInfoHeader.biBitCount = 24;
    bmpInfoHeader.biCompression = 0;
    bmpInfoHeader.biSizeImage = 0;
    bmpInfoHeader.biXPelsPerMeter = 0;
    bmpInfoHeader.biYPelsPerMeter = 0;
    bmpInfoHeader.biClrUsed = 0;
    bmpInfoHeader.biClrImportant = 0;

    FILE *fp = fopen(fileName, "wb");
    fwrite(&bmpFileHeader, 1, sizeof(BITMAPFILEHEADER), fp);
    fwrite(&bmpInfoHeader, 1, sizeof(BITMAPINFOHEADER), fp);
    fwrite(rgbData, 1, bmpDataSize, fp);
    fclose(fp);
}

int StreamMedia::demux_video_to_h264(const char *fmt_name) {
    int video_index = 0;
    // 打开输入文件，获取格式上下文
    int ret = avformat_open_input(&_p_in_fmt_ctx, _p_in_filename, nullptr, nullptr);
    if (ret != 0) {
        av_log(nullptr, AV_LOG_ERROR, "open input file format failed:%s\n", av_err2str(ret));
        return -1;
    }

    // 查找流信息
    ret = avformat_find_stream_info(_p_in_fmt_ctx, nullptr);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "find stream info failed:%s\n", av_err2str(ret));
        return -1;
    }

    // 查找目标流索引
    video_index = av_find_best_stream(_p_in_fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (video_index < 0) {
        av_log(nullptr, AV_LOG_ERROR, "find best stream failed, index is %d\n", video_index);
        return -1;
    }
    av_log(nullptr, AV_LOG_INFO, "video index is %d\n", video_index);

    // 打开输出文件，拿到文件描述符
    _p_out_fp = fopen(_p_out_filename, "wb+");
    if (_p_out_fp == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "open %s file failed\n", _p_out_filename);
        return -1;
    }

    if (strcmp(fmt_name, "avcc") == 0) {
        if (avcc(video_index) != 0) {
            av_log(nullptr, AV_LOG_ERROR, "avcc failed\n");
            return -1;
        }
    } else if (strcmp(fmt_name, "annexb") == 0) {
        if (annexb(video_index) != 0) {
            av_log(nullptr, AV_LOG_ERROR, "annexb failed\n");
            return -1;
        }
    }
    return 0;
}

int StreamMedia::decode_video_to_yuv() {
    int video_index = 0;
    // 打开输入文件，获取格式上下文
    int ret = avformat_open_input(&_p_in_fmt_ctx, _p_in_filename, nullptr, nullptr);
    if (ret != 0) {
        av_log(nullptr, AV_LOG_ERROR, "open input file format failed:%s\n", av_err2str(ret));
        return -1;
    }

    av_dump_format(_p_in_fmt_ctx, 0, _p_in_filename, 0);

    // 查找流信息
    ret = avformat_find_stream_info(_p_in_fmt_ctx, nullptr);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "find stream info failed:%s\n", av_err2str(ret));
        return -1;
    }

    video_index = av_find_best_stream(_p_in_fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (video_index < 0) {
        av_log(nullptr, AV_LOG_ERROR, "find input best stream index failed: %s\n", av_err2str(ret));
        return -1;
    }

    _p_decoder_ctx = avcodec_alloc_context3(nullptr);
    if (_p_decoder_ctx == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "alloc avcodec context failed!\n");
        return -1;
    }

    avcodec_parameters_to_context(_p_decoder_ctx, _p_in_fmt_ctx->streams[video_index]->codecpar);
    _p_decoder = avcodec_find_decoder(_p_decoder_ctx->codec_id);
    if (_p_decoder == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "find decoder failed, codec_id: %d\n", _p_decoder_ctx->codec_id);
        return -1;
    }

    ret = avcodec_open2(_p_decoder_ctx, _p_decoder, nullptr);
    if (ret != 0) {
        av_log(nullptr, AV_LOG_ERROR, "open decoder failed: %s\n", av_err2str(ret));
        return -1;
    }

    _p_out_fp = fopen(_p_out_filename, "wb+");
    if (_p_out_fp == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "open outfile: %s failed!\n", _p_out_filename);
        return -1;
    }

    AVPacket packet;
    av_new_packet(&packet, sizeof(AVPacket));
    while (av_read_frame(_p_in_fmt_ctx, &packet) >= 0) {
        if (packet.stream_index == video_index) {
            if (decode_video(&packet) == -1) {
                av_packet_unref(&packet);
                return -1;
            }
        }
        av_packet_unref(&packet);
    }

    // flush decoder
    decode_video(nullptr);
    return 0;
}

int StreamMedia::decode_video_scale_to_yuv(const char *out_scale) {
    int video_index = 0;
    int out_width = 0, out_height = 0;
    // 分析文件大小
    int ret = av_parse_video_size(&out_width, &out_height, out_scale);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "invalid video size: %s\n", out_scale);
        return -1;
    }
    av_log(nullptr, AV_LOG_INFO, "out_width: %d, out_height = %d\n", out_width, out_height);

    // 打开输入文件，获取格式上下文
    ret = avformat_open_input(&_p_in_fmt_ctx, _p_in_filename, nullptr, nullptr);
    if (ret != 0) {
        av_log(nullptr, AV_LOG_ERROR, "open input file format failed:%s\n", av_err2str(ret));
        return -1;
    }

    av_dump_format(_p_in_fmt_ctx, 0, _p_in_filename, 0);

    // 查找流信息
    ret = avformat_find_stream_info(_p_in_fmt_ctx, nullptr);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "find stream info failed:%s\n", av_err2str(ret));
        return -1;
    }

    video_index = av_find_best_stream(_p_in_fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (video_index < 0) {
        av_log(nullptr, AV_LOG_ERROR, "find input best stream index failed: %s\n", av_err2str(ret));
        return -1;
    }

    _p_decoder_ctx = avcodec_alloc_context3(nullptr);
    if (_p_decoder_ctx == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "alloc avcodec context failed!\n");
        return -1;
    }

    avcodec_parameters_to_context(_p_decoder_ctx, _p_in_fmt_ctx->streams[video_index]->codecpar);
    _p_decoder = avcodec_find_decoder(_p_decoder_ctx->codec_id);
    if (_p_decoder == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "find decoder failed, codec_id: %d\n", _p_decoder_ctx->codec_id);
        return -1;
    }

    ret = avcodec_open2(_p_decoder_ctx, _p_decoder, nullptr);
    if (ret != 0) {
        av_log(nullptr, AV_LOG_ERROR, "open decoder failed: %s\n", av_err2str(ret));
        return -1;
    }

    enum AVPixelFormat out_pix_fmt = _p_decoder_ctx->pix_fmt;
    struct SwsContext *sws_ctx = sws_getContext(_p_decoder_ctx->width, _p_decoder_ctx->height, _p_decoder_ctx->pix_fmt,
                                                out_width, out_height, out_pix_fmt, SWS_FAST_BILINEAR,
                                                nullptr, nullptr, nullptr);
    if (sws_ctx == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "get sws context failed!\n");
        ret = -1;
    }
    _p_out_frame = av_frame_alloc();
    _p_out_buffer = (uint8_t *)av_malloc(av_image_get_buffer_size(out_pix_fmt, out_width, out_height, 1));
    av_image_fill_arrays(_p_out_frame->data, _p_out_frame->linesize, _p_out_buffer, out_pix_fmt,
                         out_width, out_height, 1);

    _p_out_fp = fopen(_p_out_filename, "wb+");
    if (_p_out_fp == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "open outfile: %s failed!\n", _p_out_filename);
        return -1;
    }

    AVPacket packet;
    av_new_packet(&packet, sizeof(AVPacket));
    while (av_read_frame(_p_in_fmt_ctx, &packet) >= 0) {
        if (packet.stream_index == video_index) {
            if (decode_video(&packet, sws_ctx, "yuv420", out_width, out_height) == -1) {
                av_packet_unref(&packet);
                return -1;
            }
        }
        av_packet_unref(&packet);
    }

    // flush decoder
    decode_video(nullptr);
    return 0;
}

int StreamMedia::decode_video_scale_to_rgb(const char *out_scale) {
    int video_index = 0;
    int out_width = 0, out_height = 0;
    // 分析文件大小
    int ret = av_parse_video_size(&out_width, &out_height, out_scale);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "invalid video size: %s\n", out_scale);
        return -1;
    }
    av_log(nullptr, AV_LOG_INFO, "out_width: %d, out_height = %d\n", out_width, out_height);

    // 打开输入文件，获取格式上下文
    ret = avformat_open_input(&_p_in_fmt_ctx, _p_in_filename, nullptr, nullptr);
    if (ret != 0) {
        av_log(nullptr, AV_LOG_ERROR, "open input file format failed:%s\n", av_err2str(ret));
        return -1;
    }

    av_dump_format(_p_in_fmt_ctx, 0, _p_in_filename, 0);

    // 查找流信息
    ret = avformat_find_stream_info(_p_in_fmt_ctx, nullptr);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "find stream info failed:%s\n", av_err2str(ret));
        return -1;
    }

    video_index = av_find_best_stream(_p_in_fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (video_index < 0) {
        av_log(nullptr, AV_LOG_ERROR, "find input best stream index failed: %s\n", av_err2str(ret));
        return -1;
    }

    _p_decoder_ctx = avcodec_alloc_context3(nullptr);
    if (_p_decoder_ctx == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "alloc avcodec context failed!\n");
        return -1;
    }

    avcodec_parameters_to_context(_p_decoder_ctx, _p_in_fmt_ctx->streams[video_index]->codecpar);
    _p_decoder = avcodec_find_decoder(_p_decoder_ctx->codec_id);
    if (_p_decoder == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "find decoder failed, codec_id: %d\n", _p_decoder_ctx->codec_id);
        return -1;
    }

    ret = avcodec_open2(_p_decoder_ctx, _p_decoder, nullptr);
    if (ret != 0) {
        av_log(nullptr, AV_LOG_ERROR, "open decoder failed: %s\n", av_err2str(ret));
        return -1;
    }

    // enum AVPixelFormat out_pix_fmt = _p_decoder_ctx->pix_fmt;
    enum AVPixelFormat out_pix_fmt = AV_PIX_FMT_BGR24;
    struct SwsContext *sws_ctx = sws_getContext(_p_decoder_ctx->width, _p_decoder_ctx->height, _p_decoder_ctx->pix_fmt,
                                                out_width, out_height, out_pix_fmt, SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
    if (sws_ctx == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "get sws context failed!\n");
        ret = -1;
    }
    _p_out_frame = av_frame_alloc();
    _p_out_buffer = (uint8_t *)av_malloc(av_image_get_buffer_size(out_pix_fmt, out_width, out_height, 1));
    av_image_fill_arrays(_p_out_frame->data, _p_out_frame->linesize, _p_out_buffer, out_pix_fmt,
                         out_width, out_height, 1);

    _p_out_fp = fopen(_p_out_filename, "wb+");
    if (_p_out_fp == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "open outfile: %s failed!\n", _p_out_filename);
        return -1;
    }

    AVPacket packet;
    av_new_packet(&packet, sizeof(AVPacket));
    while (av_read_frame(_p_in_fmt_ctx, &packet) >= 0) {
        if (packet.stream_index == video_index) {
            if (decode_video(&packet, sws_ctx, "rgb", out_width, out_height) == -1) {
                av_packet_unref(&packet);
                return -1;
            }
        }
        av_packet_unref(&packet);
    }

    // flush decoder
    decode_video(nullptr);
    return 0;
}

int StreamMedia::decode_video_scale_save_to_bmp(const char *out_scale) {
    int video_index = 0;
    int out_width = 0, out_height = 0;
    // 分析文件大小
    int ret = av_parse_video_size(&out_width, &out_height, out_scale);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "invalid video size: %s\n", out_scale);
        return -1;
    }
    av_log(nullptr, AV_LOG_INFO, "out_width: %d, out_height = %d\n", out_width, out_height);

    // 打开输入文件，获取格式上下文
    ret = avformat_open_input(&_p_in_fmt_ctx, _p_in_filename, nullptr, nullptr);
    if (ret != 0) {
        av_log(nullptr, AV_LOG_ERROR, "open input file format failed:%s\n", av_err2str(ret));
        return -1;
    }

    av_dump_format(_p_in_fmt_ctx, 0, _p_in_filename, 0);

    // 查找流信息
    ret = avformat_find_stream_info(_p_in_fmt_ctx, nullptr);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "find stream info failed:%s\n", av_err2str(ret));
        return -1;
    }

    video_index = av_find_best_stream(_p_in_fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (video_index < 0) {
        av_log(nullptr, AV_LOG_ERROR, "find input best stream index failed: %s\n", av_err2str(ret));
        return -1;
    }

    _p_decoder_ctx = avcodec_alloc_context3(nullptr);
    if (_p_decoder_ctx == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "alloc avcodec context failed!\n");
        return -1;
    }

    avcodec_parameters_to_context(_p_decoder_ctx, _p_in_fmt_ctx->streams[video_index]->codecpar);
    _p_decoder = avcodec_find_decoder(_p_decoder_ctx->codec_id);
    if (_p_decoder == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "find decoder failed, codec_id: %d\n", _p_decoder_ctx->codec_id);
        return -1;
    }

    ret = avcodec_open2(_p_decoder_ctx, _p_decoder, nullptr);
    if (ret != 0) {
        av_log(nullptr, AV_LOG_ERROR, "open decoder failed: %s\n", av_err2str(ret));
        return -1;
    }

    // enum AVPixelFormat out_pix_fmt = _p_decoder_ctx->pix_fmt;
    enum AVPixelFormat out_pix_fmt = AV_PIX_FMT_BGR24;
    struct SwsContext *sws_ctx = sws_getContext(_p_decoder_ctx->width, _p_decoder_ctx->height, _p_decoder_ctx->pix_fmt,
                                                out_width, out_height, out_pix_fmt, SWS_FAST_BILINEAR,
                                                nullptr, nullptr, nullptr);
    if (sws_ctx == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "get sws context failed!\n");
        ret = -1;
    }
    _p_out_frame = av_frame_alloc();
    _p_out_buffer = (uint8_t *)av_malloc(av_image_get_buffer_size(out_pix_fmt, out_width, out_height, 1));
    av_image_fill_arrays(_p_out_frame->data, _p_out_frame->linesize, _p_out_buffer, out_pix_fmt,
                         out_width, out_height, 1);

    _p_out_fp = fopen(_p_out_filename, "wb+");
    if (_p_out_fp == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "open outfile: %s failed!\n", _p_out_filename);
        return -1;
    }

    AVPacket packet;
    av_new_packet(&packet, sizeof(AVPacket));
    while (av_read_frame(_p_in_fmt_ctx, &packet) >= 0) {
        if (packet.stream_index == video_index) {
            if (decode_video(&packet, sws_ctx, "rgb", out_width, out_height) == -1) {
                av_packet_unref(&packet);
                return -1;
            }
        }
        av_packet_unref(&packet);
    }

    // flush decoder
    decode_video(nullptr);
    return 0;
}

int StreamMedia::encode_video_yuv_to_h264(const char *out_scale) {
    int out_width = 0, out_height = 0;
    // 分析文件大小
    int ret = av_parse_video_size(&out_width, &out_height, out_scale);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "invalid video size: %s\n", out_scale);
        return -1;
    }
    av_log(nullptr, AV_LOG_INFO, "out_width: %d, out_height = %d\n", out_width, out_height);

    _p_encoder = avcodec_find_encoder_by_name(_p_encoder_name);
    if (_p_encoder == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "find encoder %s failed!\n", _p_encoder_name);
        return -1;
    }

    int fps = 30;
    enum AVPixelFormat pix_fmt = AV_PIX_FMT_YUV420P;
    _p_encoder_ctx = avcodec_alloc_context3(_p_decoder);
    if (_p_encoder_ctx == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "alloc encoder context!\n");
        return -1;
    } else {
        _p_encoder_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
        _p_encoder_ctx->pix_fmt = pix_fmt;
        _p_encoder_ctx->width = out_width;
        _p_encoder_ctx->height = out_height;
        _p_encoder_ctx->time_base = (AVRational){1, fps};
        _p_encoder_ctx->bit_rate = 652000;
        _p_encoder_ctx->max_b_frames = 0;
        _p_encoder_ctx->gop_size = 50;
    }

    ret = avcodec_open2(_p_decoder_ctx, _p_decoder, nullptr);
    if (ret != 0) {
        av_log(nullptr, AV_LOG_ERROR, "open decoder failed: %s\n", av_err2str(ret));
        return -1;
    }

    _p_in_fp = fopen(_p_in_filename, "rb");
    if (_p_in_fp == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "open infile %s failed!\n", _p_in_filename);
        return -1;
    }

    _p_out_fp = fopen(_p_out_filename, "wb+");
    if (_p_out_fp == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "open outfile %s failed!\n", _p_out_filename);
        return -1;
    }

    _p_out_frame = av_frame_alloc();
    int frame_size = av_image_get_buffer_size(pix_fmt, out_width, out_height, 1);
    _p_out_buffer = (uint8_t *)av_malloc(frame_size);
    av_image_fill_arrays(_p_out_frame->data, _p_out_frame->linesize, _p_out_buffer, pix_fmt, out_width, out_height, 1);

    _p_out_frame->format = pix_fmt;
    _p_out_frame->width = out_width;
    _p_out_frame->height = out_height;
    int picture_size = out_width * out_height;
    AVPacket packet;
    av_new_packet(&packet, sizeof(AVPacket));

    int read_frame_count = 0;
    while (fread(_p_out_buffer, 1, picture_size * 3 / 2, _p_in_fp) == picture_size * 3 / 2) {
        // Y 1 U 1/4 V 1/4
        _p_out_frame->data[0] = _p_out_buffer;
        _p_out_frame->data[1] = _p_out_buffer + picture_size;
        _p_out_frame->data[2] = _p_out_buffer + picture_size + picture_size / 4;
        _p_out_frame->pts = read_frame_count;
        read_frame_count++;
        av_log(nullptr, AV_LOG_INFO, "read_frame_count: %d\n", read_frame_count);
        encode_video(&packet);
        picture_size = out_width * out_height;
    }
    encode_video(&packet);
    return 0;
}

/*
 * 综合实战
 * */
int StreamMedia::cut_stream(int start_time, int end_time) {
    // 打开输入文件，获取格式上下文
    int ret = avformat_open_input(&_p_in_fmt_ctx, _p_in_filename, nullptr, nullptr);
    if (ret != 0) {
        av_log(nullptr, AV_LOG_ERROR, "open input file format failed:%s\n", av_err2str(ret));
        return -1;
    }

    av_dump_format(_p_in_fmt_ctx, 0, _p_in_filename, 0);

    // 查找流信息
    ret = avformat_find_stream_info(_p_in_fmt_ctx, nullptr);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "find stream info failed:%s\n", av_err2str(ret));
        return -1;
    }

    // 创建输出流上下文
    ret = avformat_alloc_output_context2(&_p_out_fmt_ctx, nullptr, nullptr, _p_out_filename);
    if (ret != 0) {
        av_log(nullptr, AV_LOG_ERROR, "alloc out format failed:%s\n", av_err2str(ret));
        return -1;
    }

    AVStream *in_stream = nullptr;
    AVStream *out_stream = nullptr;
    for (int i = 0; i < _p_in_fmt_ctx->nb_streams; i++) {
        in_stream = _p_in_fmt_ctx->streams[i];
        out_stream = avformat_new_stream(_p_out_fmt_ctx, nullptr);
        if (out_stream == nullptr) {
            av_log(nullptr, AV_LOG_ERROR, "new out stream failed\n");
            return -1;
        }

        ret = avcodec_parameters_copy(out_stream->codecpar, _p_in_fmt_ctx->streams[i]->codecpar);
        if (ret < 0) {
            av_log(nullptr, AV_LOG_ERROR, "copy in stream codecpar failed: %s\n", av_err2str(ret));
            return -1;
        }
        out_stream->codecpar->codec_tag = 0;
    }

    if (!(_p_out_fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&_p_out_fmt_ctx->pb, _p_out_filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            av_log(nullptr, AV_LOG_ERROR, "open %s file failed.\n", _p_out_filename);
            return -1;
        }
    }

    ret = avformat_write_header(_p_out_fmt_ctx, nullptr);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "writer header failed: %s\n", av_err2str(ret));
        return -1;
    }

    ret = av_seek_frame(_p_in_fmt_ctx, -1, start_time * AV_TIME_BASE, AVSEEK_FLAG_ANY);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "seek frame failed: %s\n", av_err2str(ret));
        return -1;
    }

    int64_t *dts_start_from = (int64_t *)av_calloc(_p_in_fmt_ctx->nb_streams, sizeof(int64_t));
    memset(dts_start_from, 0, _p_in_fmt_ctx->nb_streams * sizeof(int64_t));
    int64_t *pts_start_from = (int64_t *)av_calloc(_p_in_fmt_ctx->nb_streams, sizeof(int64_t));
    memset(pts_start_from, 0, _p_in_fmt_ctx->nb_streams * sizeof(int64_t));

    AVPacket packet;
    av_new_packet(&packet, sizeof(packet));
    while (av_read_frame(_p_in_fmt_ctx, &packet) == 0) {
        AVStream *inStream = _p_in_fmt_ctx->streams[packet.stream_index];
        AVStream *outStream = _p_out_fmt_ctx->streams[packet.stream_index];
        if (end_time < (double)packet.pts * av_q2d(inStream->time_base)) {
            av_packet_unref(&packet);
            break;
        }

        if (dts_start_from[packet.stream_index] == 0) {
            dts_start_from[packet.stream_index] = packet.pts;
            printf("dts_start_from: %s\n", av_ts2str(dts_start_from[packet.stream_index]));
        }
        if (pts_start_from[packet.stream_index] == 0) {
            pts_start_from[packet.stream_index] = packet.dts;
            printf("pts_start_from: %s\n", av_ts2str(pts_start_from[packet.stream_index]));
        }
        /* copy packet */
        packet.pts = av_rescale_q(packet.pts - dts_start_from[packet.stream_index], inStream->time_base, outStream->time_base);
        packet.dts = av_rescale_q(packet.dts - pts_start_from[packet.stream_index], inStream->time_base, outStream->time_base);
        if (packet.pts < 0) {
            packet.pts = 0;
        }
        if (packet.dts < 0) {
            packet.dts = 0;
        }
        packet.duration = av_rescale_q(packet.duration, inStream->time_base, outStream->time_base);
        packet.pos = -1;
        log_packet(_p_out_fmt_ctx, &packet, "out");

        if (packet.pts < packet.dts) {
            av_packet_unref(&packet);
            continue;
        }
        ret = av_interleaved_write_frame(_p_out_fmt_ctx, &packet);
        if (ret < 0) {
            av_log(nullptr, AV_LOG_ERROR, "write frame failed: %s\n", av_err2str(ret));
            av_packet_unref(&packet);
            break;
        }
        av_packet_unref(&packet);
    }
    av_freep(dts_start_from);
    av_freep(pts_start_from);

    ret = av_write_trailer(_p_out_fmt_ctx);
    if (ret != 0) {
        av_log(nullptr, AV_LOG_ERROR, "write trailer failed: %s\n", av_err2str(ret));
        return -1;
    }

    return 0;
}

int StreamMedia::remux_mp4_to_flv() {
    int stream_index = 0;
    // 打开输入文件，获取格式上下文
    int ret = avformat_open_input(&_p_in_fmt_ctx, _p_in_filename, nullptr, nullptr);
    if (ret != 0) {
        av_log(nullptr, AV_LOG_ERROR, "open input file format failed:%s\n", av_err2str(ret));
        return -1;
    }

    // 查找流信息
    ret = avformat_find_stream_info(_p_in_fmt_ctx, nullptr);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "find stream info failed:%s\n", av_err2str(ret));
        return -1;
    }

    av_dump_format(_p_in_fmt_ctx, 0, _p_in_filename, 0);

    // 创建输出流上下文
    ret = avformat_alloc_output_context2(&_p_out_fmt_ctx, nullptr, nullptr, _p_out_filename);
    if (ret != 0) {
        av_log(nullptr, AV_LOG_ERROR, "alloc out format failed:%s\n", av_err2str(ret));
        return -1;
    }

    int stream_mapping_size = _p_in_fmt_ctx->nb_streams;
    _p_stream_mapping = (int *)av_mallocz_array(stream_mapping_size, sizeof(*_p_stream_mapping));
    if (_p_stream_mapping == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "malloc stream array failed.\n");
        return -1;
    }

    AVStream *in_stream = nullptr;
    AVStream *out_stream = nullptr;
    for (int i = 0; i < _p_in_fmt_ctx->nb_streams; i++) {
        in_stream = _p_in_fmt_ctx->streams[i];
        if (in_stream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO
            && in_stream->codecpar->codec_type != AVMEDIA_TYPE_AUDIO
            && in_stream->codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
            _p_stream_mapping[i] = -1;
            continue;
        }

        _p_stream_mapping[i] = stream_index++;

        out_stream = avformat_new_stream(_p_out_fmt_ctx, nullptr);
        if (out_stream == nullptr) {
            av_log(nullptr, AV_LOG_ERROR, "new out stream failed\n");
            return -1;
        }

        ret = avcodec_parameters_copy(out_stream->codecpar, _p_in_fmt_ctx->streams[i]->codecpar);
        if (ret < 0) {
            av_log(nullptr, AV_LOG_ERROR, "copy in stream codecpar failed: %s\n", av_err2str(ret));
            return -1;
        }
        out_stream->codecpar->codec_tag = 0;
    }

    if (!(_p_out_fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&_p_out_fmt_ctx->pb, _p_out_filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            av_log(nullptr, AV_LOG_ERROR, "open %s file failed.\n", _p_out_filename);
            return -1;
        }
    }

    ret = avformat_write_header(_p_out_fmt_ctx, nullptr);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "writer header failed: %s\n", av_err2str(ret));
        return -1;
    }


    while (true) {
        AVPacket packet;
        ret = av_read_frame(_p_in_fmt_ctx, &packet);
        if (ret < 0) {
            break;
        }
        if (packet.stream_index >= stream_index || _p_stream_mapping[packet.stream_index] < 0) {
            av_packet_unref(&packet);
            continue;
        }
        AVStream *inStream = _p_in_fmt_ctx->streams[packet.stream_index];
        AVStream *outStream = _p_out_fmt_ctx->streams[packet.stream_index];
        log_packet(_p_in_fmt_ctx, &packet, "in");

        // 拷贝包
        packet.stream_index = _p_stream_mapping[packet.stream_index];
        packet.pts = av_rescale_q(packet.pts, inStream->time_base, outStream->time_base);
        packet.dts = av_rescale_q(packet.dts, inStream->time_base, outStream->time_base);
        packet.duration = av_rescale_q(packet.duration, inStream->time_base, outStream->time_base);
        packet.pos = -1;
        log_packet(_p_out_fmt_ctx, &packet, "out");

        ret = av_interleaved_write_frame(_p_out_fmt_ctx, &packet);
        if (ret != 0) {
            av_log(nullptr, AV_LOG_ERROR, "write frame failed: %s\n", av_err2str(ret));
            break;
        }
        // 解引用
        av_packet_unref(&packet);
    }

    ret = av_write_trailer(_p_out_fmt_ctx);
    if (ret != 0) {
        av_log(nullptr, AV_LOG_ERROR, "write trailer failed: %s\n", av_err2str(ret));
        return -1;
    }

    return 0;
}

int StreamMedia::capture_video_to_origin() {
    int video_index = 0;
    avdevice_register_all();
    show_avfoundation_devices();

    _p_in_fmt_ctx = avformat_alloc_context();
    if (_p_in_fmt_ctx == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "avformat alloc failed!\n");
    }

    AVDictionary *options = nullptr;
    av_dict_set(&options, "framerate", "30", 0);
    av_dict_set(&options, "video_size", "640x480", 0);
    AVInputFormat *p_input_format = av_find_input_format("avfoundation");
    if (p_input_format == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "find avfoundation failed\n");
    }

    int ret = avformat_open_input(&_p_in_fmt_ctx, "0", p_input_format, &options);
    if (ret != 0) {
        av_log(nullptr, AV_LOG_ERROR, "open input file failed: %s\n", av_err2str(ret));
        return -1;
    }

    ret = avformat_find_stream_info(_p_in_fmt_ctx, nullptr);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "find stream info failed: %s\n", av_err2str(ret));
        return -1;
    }

    ret = av_find_best_stream(_p_in_fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "find best stream failed: %s\n", av_err2str(ret));
        return -1;
    }
    video_index = ret;

    _p_decoder_ctx = avcodec_alloc_context3(nullptr);
    if (_p_decoder_ctx == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "alloc decoder context failed: %s\n", av_err2str(ret));
        return -1;
    }
    avcodec_parameters_to_context(_p_decoder_ctx, _p_in_fmt_ctx->streams[video_index]->codecpar);
    _p_decoder = avcodec_find_decoder(_p_decoder_ctx->codec_id);
    if (_p_decoder == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "find decoder %d failed!\n", _p_decoder_ctx->codec_id);
        return -1;
    }

    ret = avcodec_open2(_p_decoder_ctx, _p_decoder, nullptr);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "open decoder failed: %s\n", av_err2str(ret));
        return -1;
    }

    _p_out_fp = fopen(_p_out_filename, "wb+");
    if (_p_out_fp == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "open outfile %s failed!\n", _p_out_filename);
        return -1;
    }

    AVPacket packet;
    av_new_packet(&packet, sizeof(AVPacket));

    while (true) {
        if (av_read_frame(_p_in_fmt_ctx, &packet) == 0) {
            if (packet.stream_index == video_index) {
                ret = decode_video(&packet, nullptr, "yuv422");
                if (ret < 0) {
                    av_log(nullptr, AV_LOG_ERROR, "decoder_video failed!\n");
                    return -1;
                }
            }
            av_packet_unref(&packet);
        }
    }
    decode_video(nullptr, nullptr, "yuv422");
    return 0;
}

int StreamMedia::capture_video_to_yuv420() {
    int video_index = 0;
    avdevice_register_all();
    show_avfoundation_devices();

    enum AVPixelFormat out_pix_fmt = AV_PIX_FMT_YUV420P;
    _p_in_fmt_ctx = avformat_alloc_context();
    if (_p_in_fmt_ctx == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "avformat alloc failed!\n");
    }

    AVDictionary *options = nullptr;
    av_dict_set(&options, "framerate", "30", 0);
    av_dict_set(&options, "video_size", "640x480", 0);
    AVInputFormat *p_input_format = av_find_input_format("avfoundation");
    if (p_input_format == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "find avfoundation failed\n");
    }

    int ret = avformat_open_input(&_p_in_fmt_ctx, "0", p_input_format, &options);
    if (ret != 0) {
        av_log(nullptr, AV_LOG_ERROR, "open input file failed: %s\n", av_err2str(ret));
        return -1;
    }

    ret = avformat_find_stream_info(_p_in_fmt_ctx, nullptr);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "find stream info failed: %s\n", av_err2str(ret));
        return -1;
    }

    ret = av_find_best_stream(_p_in_fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "find best stream failed: %s\n", av_err2str(ret));
        return -1;
    }
    video_index = ret;

    _p_decoder_ctx = avcodec_alloc_context3(nullptr);
    if (_p_decoder_ctx == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "alloc decoder context failed: %s\n", av_err2str(ret));
        return -1;
    }
    avcodec_parameters_to_context(_p_decoder_ctx, _p_in_fmt_ctx->streams[video_index]->codecpar);
    _p_decoder = avcodec_find_decoder(_p_decoder_ctx->codec_id);
    if (_p_decoder == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "find decoder %d failed!\n", _p_decoder_ctx->codec_id);
        return -1;
    }

    _p_out_frame = av_frame_alloc();
    _p_out_buffer = (uint8_t *)av_malloc(av_image_get_buffer_size(out_pix_fmt, _p_decoder_ctx->width, _p_decoder_ctx->height, 1));
    av_image_fill_arrays(_p_out_frame->data, _p_out_frame->linesize, _p_out_buffer, out_pix_fmt, _p_decoder_ctx->width, _p_decoder_ctx->height, 1);
    struct SwsContext *sws_ctx = sws_getContext(_p_decoder_ctx->width, _p_decoder_ctx->height, _p_decoder_ctx->pix_fmt, _p_decoder_ctx->width, _p_decoder_ctx->height, out_pix_fmt, 0, nullptr, nullptr, nullptr);
    if (sws_ctx == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "sws_getContext failed\n");
        return -1;
    }


    ret = avcodec_open2(_p_decoder_ctx, _p_decoder, nullptr);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "open decoder failed: %s\n", av_err2str(ret));
        return -1;
    }

    _p_out_fp = fopen(_p_out_filename, "wb+");
    if (_p_out_fp == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "open outfile %s failed!\n", _p_out_filename);
        return -1;
    }

    AVPacket packet;
    av_new_packet(&packet, sizeof(AVPacket));

    while (true) {
        if (av_read_frame(_p_in_fmt_ctx, &packet) == 0) {
            if (packet.stream_index == video_index) {
                ret = decode_video(&packet, sws_ctx, "yuv420");
                if (ret < 0) {
                    av_log(nullptr, AV_LOG_ERROR, "decoder_video failed!\n");
                    return -1;
                }
            }
            av_packet_unref(&packet);
        }
    }
    decode_video(nullptr);
    return 0;
}

int StreamMedia::capture_audio_to_pcm() {
    int audio_index = 0;
    avdevice_register_all();
    show_avfoundation_devices();

    enum AVPixelFormat out_pix_fmt = AV_PIX_FMT_YUV420P;
    _p_in_fmt_ctx = avformat_alloc_context();
    if (_p_in_fmt_ctx == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "avformat alloc failed!\n");
    }

    AVDictionary *options = nullptr;
    // av_dict_set(&options, "framerate", "30", 0);
    // av_dict_set(&options, "video_size", "640x480", 0);
    AVInputFormat *p_input_format = av_find_input_format("avfoundation");
    if (p_input_format == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "find avfoundation failed\n");
    }

    int ret = avformat_open_input(&_p_in_fmt_ctx, ":0", p_input_format, &options);
    if (ret != 0) {
        av_log(nullptr, AV_LOG_ERROR, "open input file failed: %s\n", av_err2str(ret));
        return -1;
    }

    ret = avformat_find_stream_info(_p_in_fmt_ctx, nullptr);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "find stream info failed: %s\n", av_err2str(ret));
        return -1;
    }

    ret = av_find_best_stream(_p_in_fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "find best stream failed: %s\n", av_err2str(ret));
        return -1;
    }
    audio_index = ret;

    _p_decoder_ctx = avcodec_alloc_context3(nullptr);
    if (_p_decoder_ctx == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "alloc decoder context failed: %s\n", av_err2str(ret));
        return -1;
    }
    avcodec_parameters_to_context(_p_decoder_ctx, _p_in_fmt_ctx->streams[audio_index]->codecpar);
    _p_decoder = avcodec_find_decoder(_p_decoder_ctx->codec_id);
    if (_p_decoder == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "find decoder %d failed!\n", _p_decoder_ctx->codec_id);
        return -1;
    }

    ret = avcodec_open2(_p_decoder_ctx, _p_decoder, nullptr);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "open decoder failed: %s\n", av_err2str(ret));
        return -1;
    }

    _p_out_fp = fopen(_p_out_filename, "wb+");
    if (_p_out_fp == nullptr) {
        av_log(nullptr, AV_LOG_ERROR, "open outfile %s failed!\n", _p_out_filename);
        return -1;
    }

    AVPacket packet;
    av_new_packet(&packet, sizeof(AVPacket));

    while (true) {
        if (av_read_frame(_p_in_fmt_ctx, &packet) == 0) {
            if (packet.stream_index == audio_index) {
                ret = decode_audio(&packet, "f32le");
                if (ret < 0) {
                    av_log(nullptr, AV_LOG_ERROR, "decoder_video failed!\n");
                    return -1;
                }
            }
            av_packet_unref(&packet);
        }
    }
    decode_audio(nullptr);
    return 0;
}

