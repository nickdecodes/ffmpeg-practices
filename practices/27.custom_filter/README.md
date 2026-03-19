# 27. 自定义滤镜: 写一个自己的 AVFilter

## 等价命令
无直接等价命令。这是 API 独有的能力。

## 这个模块做了什么
不使用 FFmpeg 内置滤镜, 而是自己写一个滤镜函数,
通过 AVFilter 框架注册并使用。

本例实现一个简单的"反色"滤镜: 把每个像素的值取反 (255 - value)。

## 你将学到
- 如何在不修改 FFmpeg 源码的情况下注册自定义滤镜
- AVFilter 的 init/uninit/filter_frame 回调
- 滤镜的输入输出 pad 定义

## 编译运行
```bash
./27_custom_filter input.mp4 output.yuv
ffplay -video_size 640x480 -pixel_format yuv420p output.yuv
```
