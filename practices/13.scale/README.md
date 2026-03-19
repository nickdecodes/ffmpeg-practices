# 13. 视频缩放

## 等价命令
```bash
ffmpeg -i input.mp4 -vf scale=640:480 -pix_fmt yuv420p output.yuv
```

## 新增 API
```
sws_getContext() - 创建缩放/格式转换上下文
sws_scale()     - 执行缩放
sws_freeContext() - 释放
```

## 你将学到
- SwsContext 的创建和使用
- 缩放算法: SWS_BILINEAR, SWS_BICUBIC 等的区别
- 像素格式转换 (YUV420P -> RGB24 等)

## 编译运行
```bash
./13_scale input.mp4 640x480 output.yuv
ffplay -video_size 640x480 -pixel_format yuv420p output.yuv
```
