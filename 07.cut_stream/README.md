## 截取封装文件处理流程

1. 打开输入媒体文件`avformat_open_input`
2. 获取输入流信息`avformat_find_stream_info`
3. 创建输出上下文`avformat_alloc_output_context2`
4. 创建输出码流的AVStream`avformat_new_stream`
5. 拷贝编码参数`avcodec_parameters_copy`
6. 写入视频文件头`avformat_write_header`
7. 读取输入视频流`av_read_frame`
8. 跳转指定时间戳`av_seek_frame`
9. 计算pts/dts/duration`av_rescale_q_rnd/av_rescale_q`
10. 写入视频流数据`av_interleaved_write_frame`
11. 写入视频文件末尾`av_write_trailer`