# 17. 视频转码 (音频直接拷贝)

## 等价命令
```bash
ffmpeg -i input.mp4 -c:v libx264 -c:a copy output.mp4
```

## 这条命令做了什么
视频重新编码 (可以改分辨率/码率/编码器), 音频直接拷贝不动。
这是最常见的转码场景。

## 串联的知识点
demux -> decode(video) -> encode(video) -> mux
demux -> copy(audio) -> mux

## 你将学到
- 把前面学的 demux + decode + encode + mux 串成完整管线
- 视频流和音频流走不同的处理路径
- 编码后的时间戳处理

## 编译运行
```bash
./17_transcode_video input.mp4 output.mp4
ffplay output.mp4
```
