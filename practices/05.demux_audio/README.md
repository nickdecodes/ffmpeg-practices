# 05. 提取音频流为 AAC

## 等价命令
```bash
ffmpeg -i input.mp4 -vn -acodec copy output.aac
```

## 这条命令做了什么
从容器中把音频流单独拿出来, 不重新编码。
因为 MP4 中的 AAC 是裸数据, 保存为 .aac 文件需要给每个包加 ADTS 头。

## 新增 API
```
av_find_best_stream() - 自动查找最佳音频/视频流
```

## 你将学到
- av_find_best_stream 的用法
- ADTS 头的结构和作用
- 为什么从 MP4 提取 AAC 需要加头, 而提取 MP3 不需要

## 编译运行
```bash
./05_demux_audio input.mp4 output.aac
ffplay output.aac  # 验证
```
