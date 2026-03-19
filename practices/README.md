# ffmpeg-practices

从一条 ffmpeg 命令出发, 拆解成 API 调用, 自己写一个 main.cpp 实现同样的功能。

每个模块只比前一个多一个知识点, 像搭积木一样逐步叠加。

## 学习路线

### 第一部分: 基础 - 读懂一个文件

| # | 模块 | 等价命令 | 核心 API |
|---|------|---------|----------|
| 01 | [open](01.open/) | `ffprobe -show_format` | avformat_open_input |
| 02 | [streams](02.streams/) | `ffprobe -show_streams` | avformat_find_stream_info |
| 03 | [packets](03.packets/) | `ffprobe -show_packets` | av_read_frame |
| 04 | [metadata](04.metadata/) | `ffprobe -show_entries format_tags` | av_dict_get |

### 第二部分: 解封装 - 从容器中提取流

| # | 模块 | 等价命令 | 核心 API |
|---|------|---------|----------|
| 05 | [demux_audio](05.demux_audio/) | `ffmpeg -i in.mp4 -vn -c:a copy out.aac` | av_find_best_stream |
| 06 | [demux_video](06.demux_video_annexb/) | `ffmpeg -i in.mp4 -an -c:v copy -bsf:v h264_mp4toannexb out.h264` | AVBitStreamFilter |

### 第三部分: 转封装 - 换容器

| # | 模块 | 等价命令 | 核心 API |
|---|------|---------|----------|
| 07 | [remux](07.remux/) | `ffmpeg -i in.mp4 -c copy out.flv` | avformat_write_header, av_interleaved_write_frame |
| 08 | [cut](08.cut/) | `ffmpeg -i in.mp4 -ss 10 -t 5 -c copy out.mp4` | av_seek_frame |

### 第四部分: 解码 - 压缩数据变原始数据

| # | 模块 | 等价命令 | 核心 API |
|---|------|---------|----------|
| 09 | [decode_frame](09.decode_video_frame/) | `ffmpeg -i in.mp4 -vframes 1 frame.bmp` | avcodec_send_packet, avcodec_receive_frame |
| 10 | [decode_video](10.decode_video_yuv/) | `ffmpeg -i in.mp4 -pix_fmt yuv420p out.yuv` | 完整解码循环 + flush |
| 11 | [decode_audio](11.decode_audio_pcm/) | `ffmpeg -i in.mp4 -f f32le out.pcm` | planar vs packed |
| 12 | [timestamp](12.timestamp_deep/) | `ffprobe -show_frames` | I/P/B 帧, pts vs dts |

### 第五部分: 像素处理

| # | 模块 | 等价命令 | 核心 API |
|---|------|---------|----------|
| 13 | [scale](13.scale/) | `ffmpeg -i in.mp4 -vf scale=640:480 out.yuv` | sws_getContext, sws_scale |

### 第六部分: 编码 - 原始数据变压缩数据

| # | 模块 | 等价命令 | 核心 API |
|---|------|---------|----------|
| 14 | [encode_video](14.encode_video/) | `ffmpeg -f rawvideo -s 640x480 -i in.yuv -c:v libx264 out.h264` | avcodec_send_frame, avcodec_receive_packet |
| 15 | [encode_audio](15.encode_audio/) | `ffmpeg -f f32le -ar 48000 -ac 2 -i in.pcm -c:a aac out.aac` | 音频编码器 frame_size 约束 |

### 第七部分: 音频重采样

| # | 模块 | 等价命令 | 核心 API |
|---|------|---------|----------|
| 16 | [resample_audio](16.resample_audio/) | `ffmpeg -i in.mp4 -ar 16000 -ac 1 out.pcm` | swr_alloc_set_opts, swr_convert |

### 第八部分: 完整管线 - 把所有东西串起来

| # | 模块 | 等价命令 | 核心 API |
|---|------|---------|----------|
| 17 | [transcode_video](17.transcode_video/) | `ffmpeg -i in.mp4 -c:v libx264 -c:a copy out.mp4` | 完整管线: demux+decode+encode+mux |
| 18 | [transcode_full](18.transcode_full/) | `ffmpeg -i in.mp4 -c:v libx264 -c:a aac -s 1280x720 out.mp4` | 迷你 ffmpeg: 全部串联 |

### 第九部分: 深入理解

| # | 模块 | 等价命令 | 核心知识 |
|---|------|---------|----------|
| 19 | [encode_params](19.encode_params/) | 对比不同编码参数 | preset/CRF/bitrate/GOP 的影响 |
| 20 | [pixel_convert](20.pixel_convert/) | `ffmpeg -i in.mp4 -pix_fmt rgb24 frame_%03d.bmp` | 像素格式种类和转换 |
| 21 | [filter_graph](21.filter_graph/) | `ffmpeg -i in.mp4 -vf "scale=640:480" out.yuv` | AVFilterGraph 滤镜系统 |
| 22 | [custom_io](22.custom_io/) | 从内存读取视频 | AVIOContext 自定义 IO |
| 23 | [demux_video_raw](23.demux_video_raw/) | `ffmpeg -i in.mp4 -an -c:v copy out.h264` | AVCC vs Annex B 对比 |

### 第十部分: 实战应用

| # | 模块 | 等价命令 | 核心知识 |
|---|------|---------|----------|
| 24 | [multi_output](24.multi_output/) | `ffmpeg -i in.mp4 -s 1280x720 hd.mp4 -s 640x480 sd.mp4` | 多码率转码 (ABR) |
| 25 | [mini_player](25.mini_player/) | `ffplay input.mp4` | SDL2 视频播放器 |
| 26 | [screen_capture](26.screen_capture/) | `ffmpeg -f avfoundation -i "1:none" screen.mp4` | avdevice 屏幕采集 |
| 27 | [custom_filter](27.custom_filter/) | 自定义像素处理 | 手写滤镜 (反色/灰度/水印) |
| 28 | [hw_decode](28.hw_decode/) | `ffmpeg -hwaccel videotoolbox -i in.mp4 out.yuv` | 硬件加速解码 |

## 快速开始

```bash
cd practices

# 第一步: 编译 FFmpeg 及所有依赖 (首次运行, 约 10-20 分钟)
# 自动检测系统, 下载源码, 编译安装到 00.common/ 目录
make deps

# 第二步: 编译所有练习模块
make

# 第三步: 生成测试素材
make test_media

# 开始学习
cd build && ./01_open ../test_media/test.mp4
```

编译完成后, FFmpeg 源码在 `practices/00.common/source/` 目录下, 随时可以翻看:
```
00.common/source/FFmpeg-*/libavformat/format.c    - 格式探测 (对应 01)
00.common/source/FFmpeg-*/libavformat/demux.c     - 解封装 (对应 03-06)
00.common/source/FFmpeg-*/libavcodec/decode.c     - 解码入口 (对应 09-12)
00.common/source/FFmpeg-*/libavcodec/encode.c     - 编码入口 (对应 14-15)
00.common/source/FFmpeg-*/libswscale/swscale.c    - 像素缩放 (对应 13)
00.common/source/FFmpeg-*/fftools/ffmpeg.c        - ffmpeg 命令行主程序 (对应 18)
```

## 参考文档

- [API 速查索引](API_INDEX.md) - 所有 API 按字母排序, 标注在哪个模块讲解
