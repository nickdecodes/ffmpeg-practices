# 15. 音频编码: PCM 编码为 AAC

## 等价命令
```bash
ffmpeg -f f32le -ar 48000 -ac 2 -i input.pcm -c:a aac output.aac
```

## 新增知识
- 音频编码器的 frame_size 约束: 每次必须送入固定数量的采样点
- libfdk_aac vs 内置 aac 编码器的区别

## 编译运行
```bash
./15_encode_audio input.pcm output.aac 48000 2
ffplay output.aac
```
