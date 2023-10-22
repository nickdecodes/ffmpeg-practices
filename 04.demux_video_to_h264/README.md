## 提取h264数据流程

1. 打开媒体文件`avformat_open_input`
2. 获取码流信息`avformat_find_stream_info`
3. 获取音频流`av_find_best_stream`
4. 初始化packet`av_new_packet`
5. 读取packet数据`av_read_frame`
6. 释放packet资源`av_packet_unref`
7. 关闭媒体文件`av_format_close_input`

## h264封装格式介绍

h264封装分两种格式：AnnexB和AVCC

- AnnexB：startcode（0x000001或0x00000001）+ NALU数据，常用于实时流传输。

- AVCC：又称AVC1，NALU长度+NALU数据，适合存储，如MP4、MKV等

- 一个NALU（network abstract layer）= 一组对应于视频编码的NALU头部信息+一个原始字节序列负荷（RBSP， raw byte sequence payload）。

- SPS：sequence paramater set，序列参数集，包含了解码器配置和帧率等信息

- PPS：picture paramater set，图像参数集，包含了熵编码模式，slice groups， motion prediction和去块滤波器控制等信息。

- 为什么需要SPS和PPS？

    解码器需要在码流中间开始解码；

    编码器在编码过程中改变了码流的参数（如图像分辨率等）；

## h264_mp4 to annexb

`ffmpeg -i input.mp4 -codec copy -bsf:v h264_mp4toannexb output.h264`
`ffplay output.h264`

1. `av_bsf_get_by_name`
2. `av_bsf_alloc`
3. `avcodec_parameters_copy`
4. `av_bsf_init`
5. `av_bsf_receive_packet`
6. `av_bsf_free`

