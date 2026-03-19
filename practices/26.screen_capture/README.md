# 26. 录屏: 采集屏幕并编码保存

## 等价命令
```bash
# macOS
ffmpeg -f avfoundation -framerate 30 -i "1:none" -t 10 -c:v libx264 screen.mp4

# Linux
ffmpeg -f x11grab -framerate 30 -video_size 1920x1080 -i :0.0 -t 10 -c:v libx264 screen.mp4
```

## 新增 API
```
avdevice_register_all()  - 注册所有设备 (avfoundation/x11grab/dshow 等)
av_find_input_format()   - 查找输入设备格式
```

## 你将学到
- avdevice 设备采集: 屏幕、摄像头、麦克风
- avfoundation (macOS) / x11grab (Linux) / dshow (Windows) 的区别
- 采集 + 编码 + 封装的完整管线

## 编译运行
```bash
./26_screen_capture screen.mp4 10   # 录制 10 秒
ffplay screen.mp4
```
