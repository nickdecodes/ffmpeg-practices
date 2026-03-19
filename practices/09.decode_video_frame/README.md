# 09. 解码第一帧视频, 保存为 BMP

## 等价命令
```bash
ffmpeg -i input.mp4 -vframes 1 -f image2 frame.bmp
```

## 这条命令做了什么
解码视频的第一帧, 转换为 RGB, 保存为 BMP 图片。
这是第一个涉及解码的例子, 也是最直观的: 你能用图片查看器看到结果。

## 新增 API (解码三部曲)
```
avcodec_find_decoder()          - 查找解码器
avcodec_alloc_context3()        - 创建解码器上下文
avcodec_parameters_to_context() - 从流参数填充上下文
avcodec_open2()                 - 打开解码器
avcodec_send_packet()           - 送入压缩包
avcodec_receive_frame()         - 取出原始帧
```

## 你将学到
- AVCodecContext vs AVCodecParameters 的区别
- send/receive 异步模式
- AVFrame 的 data/linesize 内存布局

## 编译运行
```bash
./09_decode_video_frame input.mp4 frame.bmp
open frame.bmp  # macOS 直接打开看
```
