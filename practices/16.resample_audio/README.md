# 16. 音频重采样

## 等价命令
```bash
ffmpeg -i input.mp4 -ar 16000 -ac 1 -f f32le output.pcm
```

## 新增 API
```
swr_alloc_set_opts() - 创建重采样上下文
swr_init()           - 初始化
swr_convert()        - 执行重采样
swr_free()           - 释放
```

## 你将学到
- SwrContext: 音频重采样上下文 (类似视频的 SwsContext)
- 采样率转换: 44100Hz -> 16000Hz
- 声道转换: 立体声 -> 单声道
- 采样格式转换: fltp -> s16

## 编译运行
```bash
./16_resample_audio input.mp4 output.pcm 16000 1
ffplay -f f32le -ar 16000 -ac 1 output.pcm
```
