# 11. 解码音频为 PCM

## 等价命令
```bash
ffmpeg -i input.mp4 -f f32le -acodec pcm_f32le output.pcm
```

## 这条命令做了什么
解码音频流为 PCM 原始采样数据。

## 相比视频解码的区别
- 音频帧有 planar 和 packed 两种格式
- planar: 左声道和右声道分开存储 (data[0]=左, data[1]=右)
- packed: 左右交替存储 (LRLRLR...)
- 需要根据 sample_fmt 判断是哪种

## 你将学到
- 音频 AVFrame 的结构: nb_samples, sample_fmt, channels
- planar vs packed 采样格式
- av_get_bytes_per_sample: 每个采样点占几个字节

## 编译运行
```bash
./11_decode_audio_pcm input.mp4 output.pcm
ffplay -f f32le -ar 44100 -ac 2 output.pcm  # 验证 (参数根据实际调整)
```
