# 18. 完整转码: 音视频都重新编码 + 缩放

## 等价命令
```bash
ffmpeg -i input.mp4 -c:v libx264 -c:a aac -s 1280x720 -ar 44100 output.mp4
```

## 这条命令做了什么
音视频都重新编码, 视频缩放到 1280x720, 音频重采样到 44100Hz。
这就是一个迷你 ffmpeg。

## 串联的所有知识
- demux (07)
- decode video (10) + decode audio (11)
- scale (13) + resample (16)
- encode video (14) + encode audio (15)
- mux (07)

## 编译运行
```bash
./18_transcode_full input.mp4 output.mp4 1280x720
ffplay output.mp4
```
