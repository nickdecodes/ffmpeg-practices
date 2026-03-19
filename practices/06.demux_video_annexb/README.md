# 06. 提取视频流为 H.264 (Annex B 格式)

## 等价命令
```bash
ffmpeg -i input.mp4 -an -vcodec copy -bsf:v h264_mp4toannexb output.h264
```

## 这条命令做了什么
从 MP4 中提取 H.264 视频流, 并用码流过滤器转换为 Annex B 格式。

## 新增 API
```
av_bsf_get_by_name()  - 查找码流过滤器
av_bsf_alloc()         - 分配过滤器上下文
av_bsf_init()          - 初始化过滤器
av_bsf_send_packet()   - 送入数据包
av_bsf_receive_packet() - 取出处理后的数据包
```

## 你将学到
- AVCC vs Annex B: H.264 的两种打包格式
- BitStreamFilter (BSF): 不编解码, 只转换码流格式
- 为什么 MP4 里的 H.264 不能直接保存为 .h264 文件

## 编译运行
```bash
./06_demux_video_annexb input.mp4 output.h264
ffplay output.h264  # 验证
```
