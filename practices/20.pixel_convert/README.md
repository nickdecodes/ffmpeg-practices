# 20. 像素格式转换

## 等价命令
```bash
ffmpeg -i input.mp4 -pix_fmt rgb24 -vframes 10 output.rgb
```

## 这条命令做了什么
解码视频, 将 YUV420P 转换为 RGB24, 保存前 10 帧为 BMP 图片序列。

## 你将学到
- 像素格式的种类: YUV420P, NV12, RGB24, BGR24, RGBA 等
- sws_scale 做格式转换 (不缩放, 只转格式)
- 不同像素格式的内存布局差异

## 编译运行
```bash
./20_pixel_convert input.mp4 output_dir/ 10
# 会生成 output_dir/frame_000.bmp ~ frame_009.bmp
```
