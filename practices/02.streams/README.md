# 02. 查看文件里有哪些流

## 等价命令
```bash
ffprobe -show_streams input.mp4
```

## 这条命令做了什么
在 01 的基础上多一步: 分析文件里有几路流, 每路流的编码参数是什么。

## 新增 API
```
avformat_find_stream_info() - 读取部分数据, 分析每路流的编码参数
```

## 你将学到
- AVStream 结构体: 代表一路流 (视频/音频/字幕)
- AVCodecParameters: 编码参数 (编码器ID, 分辨率, 采样率等)
- time_base: 时间基的概念

## 编译运行
```bash
cd build && cmake .. && make
./02_streams input.mp4
```
