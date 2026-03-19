# 19. 编码参数对比实验

## 等价命令
```bash
ffmpeg -f rawvideo -s 640x480 -i in.yuv -c:v libx264 -preset ultrafast out1.h264
ffmpeg -f rawvideo -s 640x480 -i in.yuv -c:v libx264 -preset veryslow out2.h264
ffmpeg -f rawvideo -s 640x480 -i in.yuv -c:v libx264 -b:v 500k out3.h264
ffmpeg -f rawvideo -s 640x480 -i in.yuv -c:v libx264 -crf 28 out4.h264
```

## 这个模块做了什么
用同一个 YUV 源, 分别用不同的编码参数编码, 打印文件大小和耗时对比。
让你直观感受 preset/bitrate/crf/gop 对输出的影响。

## 你将学到
- preset: 编码速度和压缩率的权衡
- CRF: 恒定质量模式 (值越小质量越高)
- bitrate: 恒定码率模式
- gop_size: 关键帧间隔对文件大小的影响

## 编译运行
```bash
./19_encode_params input.yuv 640x480
```
