## 更改视频格式流程

1. 分析视频像素大小`av_parse_video_size`
2. 获取上下文`sws_getContext`
3. 保存转换后的数据`av_frame_alloc`
4. 初始化buffer`av_image_get_buffer_size`
5. 申请空间`av_malloc`
6. 按照av_frame的内存空间进行分配`av_image_fill_arrays`
7. 进行处理`sws_scale`

## FFmpeg命令
```bash
ffmpeg -i input.mp4 -an -sn -scale 1920x1080 -y output.yuv
ffplay output.yuv -pixel_format bgr24 -video_size 1920x1080
```