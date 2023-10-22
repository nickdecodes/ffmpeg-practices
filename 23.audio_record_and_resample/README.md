## 编码流程

1. `avdevice_register_all`
2. `avformat_alloc_format`
3. `av_dict_set`
4. `av_find_input_format`
5. `avformat_open_input`
6. `avformat_find_stream_info`
7. `av_find_best_stream`
8. `avcodec_alloc_context3`
9. `avcode_parameters_to_context`
10. `avcodec_find_decoder`
11. `avcodec_open2`
12. `av_read_frame`
13. `avcode_send_packet`
14. `avcodec_receive_frame`
15. `sws_getContext`
16. `av_frame_alloc`
17. `av_image_get_buffer_size`
18. `av_malloc`
19. `av_image_fill_arrays`
20. `sws_scale`

## 视频采集命令
- 查看设备列表`ffmpeg -hide_banner -devices`
- 查看avfoundation支持的参数`ffmpeg -h demuxer=avfoundatioin`
- 查看支持的采集设备列表`ffmpeg -f avfoundation -list_devices true -i ""`
- 采集摄像头画面`ffmpeg -f avfoundation -framerate 30 -video_size 640x480 -i 0 out.yuv`
- 播放摄像头采集画面`ffplay out.yuv -pixel_format uyvy422 -video_size 640x480`
