## 编码流程

1. 查找编码器`avvodec_find_encoder_by_name`
2. 创建编码器上下文`avcodec_alloc_context3`
3. 设置编码参数`avcodec_open2`
4. 打开编码器`av_frame_alloc`
5. 读取yuv数据`av_image_get_buffer_size`
6. 开始编码`av_image_fill_arrays`
7. 写入编码数据`avcodec_send_frame`
8. `avcodec_receive_packet`

## 命令
```bash
ffmpeg -s 768*432 -pix_fmt yub420p -i input.yuv -vcodec libx264 -b:v 4096k -bf 0 -g 10 -r 30 output.h264
# -s 指定视频大小
# -pix_fmt 指定图像颜色空间
# -b:v 指定视频平均码率
# -bf 指定B帧数目
# -g 指定两个I帧之间的间隔
# -r 指定视频帧率
```

    

