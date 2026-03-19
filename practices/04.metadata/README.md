# 04. 读取文件元数据

## 等价命令
```bash
ffprobe -show_format_entry tags input.mp4
```

## 这条命令做了什么
读取文件和每路流的元数据标签 (标题、作者、创建时间等)。

## 新增 API
```
av_dict_get() - 从 AVDictionary 中读取键值对
```

## 你将学到
- AVDictionary: FFmpeg 的通用键值对容器
- 文件级元数据 vs 流级元数据

## 编译运行
```bash
cd build && cmake .. && make
./04_metadata input.mp4
```
