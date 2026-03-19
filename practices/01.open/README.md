# 01. 打开一个媒体文件

## 等价命令
```bash
ffprobe -show_format input.mp4
```

## 这条命令做了什么
只做一件事：打开文件，告诉你这是什么格式。

## 对应的 API 调用
```
avformat_open_input()   - 打开文件，探测封装格式
avformat_close_input()  - 关闭文件，释放资源
```

## 你将学到
- AVFormatContext 是什么（FFmpeg 中最核心的结构体）
- avformat_open_input 的参数含义
- av_err2str 错误处理

## 编译运行
```bash
cd build && cmake .. && make
./01.open input.mp4
```
