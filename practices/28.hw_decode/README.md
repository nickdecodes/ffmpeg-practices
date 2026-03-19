# 28. 硬件加速解码

## 等价命令
```bash
# macOS (VideoToolbox)
ffmpeg -hwaccel videotoolbox -i input.mp4 -pix_fmt yuv420p output.yuv

# Linux (VAAPI)
ffmpeg -hwaccel vaapi -hwaccel_device /dev/dri/renderD128 -i input.mp4 output.yuv
```

## 新增 API
```
av_hwdevice_ctx_create()     - 创建硬件设备上下文
av_hwframe_transfer_data()   - 将硬件帧数据传回 CPU 内存
```

## 你将学到
- 硬件加速的工作原理: GPU 解码 vs CPU 解码
- AVHWDeviceContext / AVHWFramesContext
- 硬件帧 (GPU 内存) 到软件帧 (CPU 内存) 的数据传输
- 不同平台的硬件加速: VideoToolbox (macOS) / VAAPI (Linux) / DXVA2 (Windows)

## 编译运行
```bash
./28_hw_decode input.mp4 output.yuv
```
