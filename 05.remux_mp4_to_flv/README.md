## 转封装-mp4转flv

1. 打开输入媒体文件`avformat_open_input`
2. 获取输入流信息`avformat_find_stream_info`
3. 创建输出流上下文`avformat_alloc_output_context2`
4. 创建输出流的AVStream`avformat_new_stream`
5. 拷贝编码参数`avcodec_parameters_copy`
6. 写入视频文件头`avformat_write_header`
7. 读取输入视频流`av_read_frame`
8. 计算pts/dts/duration`av_rescale_qrnd/av_rescale_q`
9. 写入视频流数据`av_interleaved_write_frame`
10. 写入视频文件末尾`av_write_trailer`

## I帧、P帧和B帧

- I帧：帧内编码帧（intra picture），I帧通常是一个GOP的第一帧，经过轻度地压缩，作为随机访问的参考点，可以当成静态图像，I帧压缩可去掉视频的空间冗余信息。
- P帧：前向预测编码帧（predictive-frame），通过将图像序列中前面已编码的时间冗余信息充分去除来压缩传输数据量的编码图像，也称为预测帧
- B帧：双向预测内插编码帧（bi-directional interpolated prediction frame），即考虑源图像序列前面的已编码帧，又顾及源图像序列后面的已编码帧之间的时间冗余信息，来压缩传输数据量的编码图像，也称为双向预测帧。

## PTS与DTS

DTS-Pressentation Time Stamp，显示时间戳

PTS-Decompress Time Stamp，解码时间戳