# 22. 自定义 IO: 从内存读取视频

## 等价命令
无直接等价命令。这是 API 独有的能力。

## 这个模块做了什么
不从文件读取, 而是先把文件读入内存, 然后通过自定义 AVIOContext 让 FFmpeg 从内存缓冲区读取。

## 新增 API
```
avio_alloc_context() - 创建自定义 IO 上下文
                       需要提供 read_packet 回调函数
```

## 你将学到
- AVIOContext 的工作原理
- 自定义 read_packet 回调
- 应用场景: 加密视频、网络流、内存中的数据

## 编译运行
```bash
./22_custom_io input.mp4
```
