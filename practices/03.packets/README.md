# 03. 逐包读取数据

## 等价命令
```bash
ffprobe -show_packets input.mp4
```

## 这条命令做了什么
逐个读取文件中的压缩数据包, 打印每个包的信息。

## 新增 API
```
av_read_frame()    - 读取一个压缩数据包
av_packet_unref()  - 释放包的数据引用
```

## 你将学到
- AVPacket: 压缩数据包, FFmpeg 中数据流转的基本单位
- pts/dts 的区别
- stream_index: 判断包属于哪路流

## 编译运行
```bash
cd build && cmake .. && make
./03_packets input.mp4
```
