## 视频解码yuv

1. 打开输入媒体文件`avformat_open_input`
2. 获取输入流信息`avformat_find_stream_info`
3. 获取音频流`av_find_best_stream`
4. 申请上下文`avcodec_alloc_context3`
5. 创建参数上下文`avcodec_parameters_to_context`
6. 选择解码器`avcodec_find_decoder`
7. 使用解码器`avcodec_open2`
8. 解包`av_read_frame`
9. 解码`avcodec_send_packet`
10. 封装`avcodec_receive_frame`

## FFmpeg命令
```bash
ffmpeg -i input.mp4 -an -sn -y output.yuv
ffplay output.yuv -pixel_format yuv420p -video_size 1920x1080
```