# 12. 深入理解时间戳

## 等价命令
```bash
ffprobe -show_frames -select_streams v -show_entries frame=pict_type,pts,pts_time input.mp4
```

## 这条命令做了什么
解码视频, 打印每帧的 pts/dts 和帧类型 (I/P/B), 直观展示时间戳的工作方式。

## 你将学到
- I/P/B 帧的区别和出现规律
- 为什么 pts 和 dts 不一样 (有 B 帧时)
- 解码顺序 (dts) vs 显示顺序 (pts)

## 编译运行
```bash
./12_timestamp_deep input.mp4 30   # 只看前 30 帧
```
