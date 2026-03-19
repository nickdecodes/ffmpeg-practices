# 23. 提取视频裸流 (不转格式)

## 等价命令
```bash
ffmpeg -i input.mp4 -an -vcodec copy output.h264
```

## 这条命令做了什么
直接提取视频包, 不经过 BSF 转换。
对比 06 (用 h264_mp4toannexb), 理解 AVCC 和 Annex B 的区别。

## 你将学到
- 直接提取的 H.264 是 AVCC 格式, 很多播放器不能播
- 对比 06 的输出, 理解为什么需要 BSF

## 编译运行
```bash
./23_demux_video_raw input.mp4 output_raw.h264
# 对比: 06 的输出可以 ffplay, 这个可能不行
```
