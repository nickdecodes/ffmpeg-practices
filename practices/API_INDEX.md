# FFmpeg API 速查索引

按字母排序, 标注每个 API 在哪个模块首次出现和详细讲解。

## libavformat (封装/解封装)

| API | 说明 | 首次出现 |
|-----|------|---------|
| `avformat_open_input()` | 打开输入文件, 探测格式 | [01.open](01.open/) |
| `avformat_find_stream_info()` | 分析流信息 | [02.streams](02.streams/) |
| `avformat_close_input()` | 关闭输入, 释放资源 | [01.open](01.open/) |
| `av_read_frame()` | 读取一个压缩数据包 | [03.packets](03.packets/) |
| `av_find_best_stream()` | 查找最佳音频/视频流 | [05.demux_audio](05.demux_audio/) |
| `av_seek_frame()` | 跳转到指定时间位置 | [08.cut](08.cut/) |
| `av_dump_format()` | 打印格式摘要 | [02.streams](02.streams/) |
| `avformat_alloc_output_context2()` | 创建输出格式上下文 | [07.remux](07.remux/) |
| `avformat_new_stream()` | 在输出中创建新流 | [07.remux](07.remux/) |
| `avformat_write_header()` | 写文件头 | [07.remux](07.remux/) |
| `av_interleaved_write_frame()` | 写数据包 (自动交织) | [07.remux](07.remux/) |
| `av_write_trailer()` | 写文件尾 | [07.remux](07.remux/) |
| `avio_open()` | 打开输出文件 IO | [07.remux](07.remux/) |
| `avio_alloc_context()` | 创建自定义 IO 上下文 | [22.custom_io](22.custom_io/) |

## libavcodec (编解码)

| API | 说明 | 首次出现 |
|-----|------|---------|
| `avcodec_find_decoder()` | 按 codec_id 查找解码器 | [09.decode_video_frame](09.decode_video_frame/) |
| `avcodec_find_encoder()` | 按 codec_id 查找编码器 | [14.encode_video](14.encode_video/) |
| `avcodec_find_encoder_by_name()` | 按名称查找编码器 | [14.encode_video](14.encode_video/) |
| `avcodec_alloc_context3()` | 创建编解码器上下文 | [09.decode_video_frame](09.decode_video_frame/) |
| `avcodec_parameters_to_context()` | 流参数 -> 解码器上下文 | [09.decode_video_frame](09.decode_video_frame/) |
| `avcodec_parameters_from_context()` | 编码器上下文 -> 流参数 | [17.transcode_video](17.transcode_video/) |
| `avcodec_parameters_copy()` | 复制编码参数 | [07.remux](07.remux/) |
| `avcodec_open2()` | 打开编解码器 | [09.decode_video_frame](09.decode_video_frame/) |
| `avcodec_send_packet()` | 送入压缩包 (解码) | [09.decode_video_frame](09.decode_video_frame/) |
| `avcodec_receive_frame()` | 取出原始帧 (解码) | [09.decode_video_frame](09.decode_video_frame/) |
| `avcodec_send_frame()` | 送入原始帧 (编码) | [14.encode_video](14.encode_video/) |
| `avcodec_receive_packet()` | 取出压缩包 (编码) | [14.encode_video](14.encode_video/) |
| `avcodec_free_context()` | 释放编解码器上下文 | [09.decode_video_frame](09.decode_video_frame/) |
| `avcodec_get_name()` | 获取编解码器名称 | [02.streams](02.streams/) |
| `av_packet_unref()` | 释放包数据引用 | [03.packets](03.packets/) |
| `av_packet_rescale_ts()` | 包时间戳时间基转换 | [17.transcode_video](17.transcode_video/) |

## libavcodec - BitStreamFilter (码流过滤器)

| API | 说明 | 首次出现 |
|-----|------|---------|
| `av_bsf_get_by_name()` | 查找码流过滤器 | [06.demux_video_annexb](06.demux_video_annexb/) |
| `av_bsf_alloc()` | 分配过滤器上下文 | [06.demux_video_annexb](06.demux_video_annexb/) |
| `av_bsf_init()` | 初始化过滤器 | [06.demux_video_annexb](06.demux_video_annexb/) |
| `av_bsf_send_packet()` | 送入数据包 | [06.demux_video_annexb](06.demux_video_annexb/) |
| `av_bsf_receive_packet()` | 取出处理后的包 | [06.demux_video_annexb](06.demux_video_annexb/) |
| `av_bsf_free()` | 释放过滤器 | [06.demux_video_annexb](06.demux_video_annexb/) |

## libswscale (像素处理)

| API | 说明 | 首次出现 |
|-----|------|---------|
| `sws_getContext()` | 创建缩放/格式转换上下文 | [09.decode_video_frame](09.decode_video_frame/) |
| `sws_scale()` | 执行缩放/格式转换 | [09.decode_video_frame](09.decode_video_frame/) |
| `sws_freeContext()` | 释放上下文 | [09.decode_video_frame](09.decode_video_frame/) |

## libswresample (音频重采样)

| API | 说明 | 首次出现 |
|-----|------|---------|
| `swr_alloc_set_opts()` | 创建重采样上下文 | [16.resample_audio](16.resample_audio/) |
| `swr_init()` | 初始化 | [16.resample_audio](16.resample_audio/) |
| `swr_convert()` | 执行重采样 | [16.resample_audio](16.resample_audio/) |
| `swr_get_delay()` | 获取内部缓存延迟 | [16.resample_audio](16.resample_audio/) |
| `swr_free()` | 释放 | [16.resample_audio](16.resample_audio/) |

## libavfilter (滤镜)

| API | 说明 | 首次出现 |
|-----|------|---------|
| `avfilter_graph_alloc()` | 创建滤镜图 | [21.filter_graph](21.filter_graph/) |
| `avfilter_graph_create_filter()` | 创建滤镜实例 | [21.filter_graph](21.filter_graph/) |
| `avfilter_graph_parse_ptr()` | 解析滤镜描述字符串 | [21.filter_graph](21.filter_graph/) |
| `avfilter_graph_config()` | 配置滤镜图 | [21.filter_graph](21.filter_graph/) |
| `avfilter_graph_free()` | 释放滤镜图 | [21.filter_graph](21.filter_graph/) |
| `av_buffersrc_add_frame()` | 向滤镜图送入帧 | [21.filter_graph](21.filter_graph/) |
| `av_buffersink_get_frame()` | 从滤镜图取出帧 | [21.filter_graph](21.filter_graph/) |

## libavutil (工具)

| API | 说明 | 首次出现 |
|-----|------|---------|
| `av_log_set_level()` | 设置日志级别 | [01.open](01.open/) |
| `av_err2str()` | 错误码转可读字符串 | [01.open](01.open/) |
| `av_dict_get()` | 遍历字典键值对 | [04.metadata](04.metadata/) |
| `av_rescale_q()` | 时间基转换 | [07.remux](07.remux/) |
| `av_q2d()` | AVRational 转 double | [02.streams](02.streams/) |
| `av_frame_alloc()` | 分配 AVFrame | [09.decode_video_frame](09.decode_video_frame/) |
| `av_frame_free()` | 释放 AVFrame | [09.decode_video_frame](09.decode_video_frame/) |
| `av_frame_get_buffer()` | 为帧分配数据缓冲区 | [13.scale](13.scale/) |
| `av_image_get_buffer_size()` | 计算图像缓冲区大小 | [14.encode_video](14.encode_video/) |
| `av_image_fill_arrays()` | 填充图像数据指针 | [14.encode_video](14.encode_video/) |
| `av_parse_video_size()` | 解析 "640x480" 字符串 | [13.scale](13.scale/) |
| `av_get_bytes_per_sample()` | 每个采样点的字节数 | [11.decode_audio_pcm](11.decode_audio_pcm/) |
| `av_sample_fmt_is_planar()` | 判断是否 planar 格式 | [11.decode_audio_pcm](11.decode_audio_pcm/) |
| `av_get_sample_fmt_name()` | 采样格式名称 | [11.decode_audio_pcm](11.decode_audio_pcm/) |
| `av_get_pix_fmt_name()` | 像素格式名称 | [02.streams](02.streams/) |
| `av_opt_set()` | 设置编解码器私有选项 | [14.encode_video](14.encode_video/) |
| `av_malloc()` / `av_freep()` | 内存分配/释放 | [07.remux](07.remux/) |
| `av_gettime()` | 获取当前时间 (微秒) | [25.mini_player](25.mini_player/) |

## 核心数据结构

| 结构体 | 说明 | 首次出现 |
|--------|------|---------|
| `AVFormatContext` | 封装格式上下文 (最核心) | [01.open](01.open/) |
| `AVStream` | 一路流 (视频/音频/字幕) | [02.streams](02.streams/) |
| `AVCodecParameters` | 编码参数 (静态描述) | [02.streams](02.streams/) |
| `AVPacket` | 压缩数据包 | [03.packets](03.packets/) |
| `AVFrame` | 原始数据帧 (解码后) | [09.decode_video_frame](09.decode_video_frame/) |
| `AVCodecContext` | 编解码器运行时上下文 | [09.decode_video_frame](09.decode_video_frame/) |
| `AVCodec` | 编解码器描述 | [09.decode_video_frame](09.decode_video_frame/) |
| `AVDictionary` | 键值对容器 | [04.metadata](04.metadata/) |
| `AVRational` | 分数 (用于 time_base 等) | [02.streams](02.streams/) |
| `SwsContext` | 像素缩放/转换上下文 | [09.decode_video_frame](09.decode_video_frame/) |
| `SwrContext` | 音频重采样上下文 | [16.resample_audio](16.resample_audio/) |
| `AVFilterGraph` | 滤镜图 | [21.filter_graph](21.filter_graph/) |
| `AVIOContext` | IO 上下文 | [22.custom_io](22.custom_io/) |
| `AVBSFContext` | 码流过滤器上下文 | [06.demux_video_annexb](06.demux_video_annexb/) |


## libavdevice (设备采集)

| API | 说明 | 首次出现 |
|-----|------|---------|
| `avdevice_register_all()` | 注册所有输入/输出设备 | [26.screen_capture](26.screen_capture/) |
| `av_find_input_format()` | 查找输入设备格式 (avfoundation/x11grab) | [26.screen_capture](26.screen_capture/) |

## 硬件加速

| API | 说明 | 首次出现 |
|-----|------|---------|
| `av_hwdevice_ctx_create()` | 创建硬件设备上下文 | [28.hw_decode](28.hw_decode/) |
| `av_hwframe_transfer_data()` | GPU 帧数据传回 CPU 内存 | [28.hw_decode](28.hw_decode/) |
| `avcodec_get_hw_config()` | 查询解码器支持的硬件配置 | [28.hw_decode](28.hw_decode/) |
| `av_hwdevice_get_type_name()` | 获取硬件加速类型名称 | [28.hw_decode](28.hw_decode/) |
