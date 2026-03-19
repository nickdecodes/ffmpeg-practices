# ffmpeg-practices

从一条 ffmpeg 命令出发，拆解成 API 调用，自己写一个 main.cpp 实现同样的功能。

## 项目结构

```
ffmpeg-practices/
  practices/
    00.common/        FFmpeg 及依赖库的编译脚本、头文件、库文件、源码
    01.open/          打开文件
    02.streams/       遍历流
    ...
    25.mini_player/   迷你播放器
    API_INDEX.md      API 速查索引
    Makefile          一键编译
  visualizer/         Web 可视化 (规划中)
```

## 快速开始

```bash
cd practices

# 第一步: 编译 FFmpeg 及所有依赖 (首次运行, 约 10-20 分钟)
# 自动检测系统, 下载源码, 编译 x264/x265/SDL2/FFmpeg 等
make deps

# 第二步: 编译所有练习模块
make

# 第三步: 生成测试素材
make test_media

# 开始学习
cd build && ./01_open ../test_media/test.mp4
```

编译完成后, FFmpeg 及依赖库的源码都在 `practices/00.common/source/` 目录下, 学习过程中随时翻看:

```
00.common/source/FFmpeg-*/libavformat/format.c   - 格式探测
00.common/source/FFmpeg-*/libavformat/demux.c    - 解封装
00.common/source/FFmpeg-*/libavcodec/decode.c    - 解码入口
00.common/source/FFmpeg-*/libavcodec/encode.c    - 编码入口
00.common/source/FFmpeg-*/libavcodec/h264dec.c   - H.264 解码器
00.common/source/FFmpeg-*/libswscale/swscale.c   - 像素缩放
00.common/source/FFmpeg-*/fftools/ffmpeg.c       - ffmpeg 命令行主程序
```

## 学习路线

详见 [practices/README.md](practices/README.md) | [API 速查索引](practices/API_INDEX.md)

从 01 到 25, 每个模块只比前一个多一个知识点:

- 01-04: 基础 — 打开文件、遍历流、读包、元数据
- 05-06: 解封装 — 提取音频/视频裸流
- 07-08: 转封装 — 换容器、裁剪
- 09-12: 解码 — 视频帧/YUV/PCM/时间戳
- 13: 像素处理 — 缩放
- 14-16: 编码与重采样 — 视频/音频编码、音频重采样
- 17-18: 完整管线 — 迷你 ffmpeg
- 19-23: 深入理解 — 编码参数、像素格式、滤镜、自定义IO、码流格式
- 24-28: 实战应用 — 多码率转码、迷你播放器、录屏、自定义滤镜、硬件加速

学完 28, 你就能理解任何 ffmpeg 命令背后的 API 调用流程。
