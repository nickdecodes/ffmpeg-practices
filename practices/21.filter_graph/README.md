# 21. 滤镜系统

## 等价命令
```bash
ffmpeg -i input.mp4 -vf "scale=640:480,drawbox=x=10:y=10:w=100:h=100:color=red" -f rawvideo output.yuv
```

## 新增 API
```
avfilter_graph_alloc()          - 创建滤镜图
avfilter_graph_create_filter()  - 创建滤镜实例
avfilter_link()                 - 连接滤镜
avfilter_graph_config()         - 配置滤镜图
av_buffersrc_add_frame()        - 向滤镜图送入帧
av_buffersink_get_frame()       - 从滤镜图取出帧
```

## 你将学到
- AVFilterGraph: 滤镜图的概念
- buffersrc / buffersink: 滤镜图的输入和输出
- 滤镜链的字符串描述语法

## 编译运行
```bash
./21_filter_graph input.mp4 "scale=640:480" output.yuv
ffplay -video_size 640x480 -pixel_format yuv420p output.yuv
```
