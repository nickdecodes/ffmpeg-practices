# 08. 裁剪视频片段

## 等价命令
```bash
ffmpeg -i input.mp4 -ss 10 -t 5 -c copy output.mp4
```

## 这条命令做了什么
从第 10 秒开始, 截取 5 秒的片段, 不重新编码。

## 新增 API
```
av_seek_frame() - 跳转到指定时间位置
```

## 你将学到
- av_seek_frame 的用法和 AVSEEK_FLAG_BACKWARD 标志
- 为什么 -ss 裁剪出来的视频开头可能有几帧花屏 (关键帧对齐问题)
- 时间戳偏移: 裁剪后需要把时间戳减去起始时间

## 编译运行
```bash
./08_cut input.mp4 10 5 output.mp4
ffplay output.mp4  # 验证
```
