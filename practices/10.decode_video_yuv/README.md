# 10. 解码整个视频为 YUV

## 等价命令
```bash
ffmpeg -i input.mp4 -pix_fmt yuv420p output.yuv
```

## 这条命令做了什么
解码所有视频帧, 保存为 YUV420P 原始数据。

## 相比 09 多了什么
- 解码所有帧而不是只解第一帧
- flush 解码器: 文件读完后取出缓存的剩余帧
- 正确处理 linesize (内存对齐)

## 你将学到
- 完整的解码循环
- flush 机制: 为什么要发送 nullptr packet
- YUV420P 的内存布局: Y/U/V 三个平面

## 编译运行
```bash
./10_decode_video_yuv input.mp4 output.yuv
ffplay -video_size 1920x1080 -pixel_format yuv420p output.yuv  # 验证
```
