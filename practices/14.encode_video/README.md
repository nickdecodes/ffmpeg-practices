# 14. 视频编码: YUV 编码为 H.264

## 等价命令
```bash
ffmpeg -f rawvideo -pix_fmt yuv420p -s 640x480 -r 30 -i input.yuv -c:v libx264 output.h264
```

## 新增 API (编码)
```
avcodec_find_encoder_by_name() - 按名称查找编码器
avcodec_send_frame()           - 送入原始帧
avcodec_receive_packet()       - 取出压缩包
```

## 你将学到
- 编码器参数: bitrate, gop_size, preset
- send_frame/receive_packet (编码是解码的镜像操作)
- flush 编码器

## 准备测试数据
```bash
# 先用 10 生成 YUV 文件
./10_decode_video_yuv input.mp4 test.yuv
# 然后编码
./14_encode_video test.yuv output.h264 640x480
ffplay output.h264
```
