# 07. 转封装: MP4 转 FLV

## 等价命令
```bash
ffmpeg -i input.mp4 -c copy output.flv
```

## 这条命令做了什么
把音视频数据从 MP4 盒子搬到 FLV 盒子, 数据本身不变。
这就是为什么 `-c copy` 特别快: 不需要编解码。

## 新增 API (输出侧)
```
avformat_alloc_output_context2() - 创建输出格式上下文
avformat_new_stream()            - 创建输出流
avcodec_parameters_copy()        - 复制编码参数
avio_open()                      - 打开输出文件 IO
avformat_write_header()          - 写文件头
av_interleaved_write_frame()     - 写数据包
av_write_trailer()               - 写文件尾
av_rescale_q()                   - 时间基转换
```

## 你将学到
- 输出侧的完整流程
- 时间基转换: 为什么需要 av_rescale_q
- codec_tag 为什么要清零

## 编译运行
```bash
./07_remux input.mp4 output.flv
ffplay output.flv  # 验证
```
