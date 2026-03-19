/** FFmpeg API 知识库 */
export interface ApiInfo {
  name: string;
  signature: string;
  library: string;
  description: string;
  params: { name: string; desc: string }[];
  returnValue: string;
  tips: string[];
  sourceFile: string;
}

export const APIS: Record<string, ApiInfo> = {
  avformat_open_input: {
    name: 'avformat_open_input',
    signature: 'int avformat_open_input(AVFormatContext **ps, const char *url, const AVInputFormat *fmt, AVDictionary **options)',
    library: 'libavformat',
    description: '打开输入文件或 URL，探测封装格式，创建 AVFormatContext。这是使用 FFmpeg 的第一步。',
    params: [
      { name: 'ps', desc: '输出参数，函数内部分配 AVFormatContext' },
      { name: 'url', desc: '文件路径或网络地址（file://, http://, rtmp:// 等）' },
      { name: 'fmt', desc: '强制指定输入格式，nullptr 表示自动探测' },
      { name: 'options', desc: '额外选项（如超时时间），nullptr 使用默认值' },
    ],
    returnValue: '0 成功，负数失败（用 av_err2str 转为可读字符串）',
    tips: [
      '内部会调用 av_probe_input_format2 对文件头打分来探测格式',
      '支持的协议：file, http, https, rtmp, rtsp, udp, tcp 等',
      '用完必须调用 avformat_close_input 释放',
    ],
    sourceFile: 'libavformat/demux.c',
  },
  avformat_find_stream_info: {
    name: 'avformat_find_stream_info',
    signature: 'int avformat_find_stream_info(AVFormatContext *ic, AVDictionary **options)',
    library: 'libavformat',
    description: '读取一部分数据来分析流信息。有些格式（如 MPEG-TS）头部信息不完整，需要这一步。',
    params: [
      { name: 'ic', desc: '已打开的格式上下文' },
      { name: 'options', desc: '每路流的选项数组，通常传 nullptr' },
    ],
    returnValue: '>=0 成功',
    tips: [
      '内部会尝试解码几帧来确定编码参数',
      '对于网络流，这一步可能比较慢',
      '调用后 AVStream->codecpar 才有完整的编码参数',
    ],
    sourceFile: 'libavformat/demux.c',
  },
  av_read_frame: {
    name: 'av_read_frame',
    signature: 'int av_read_frame(AVFormatContext *s, AVPacket *pkt)',
    library: 'libavformat',
    description: '读取一个压缩数据包。可能是视频包也可能是音频包，通过 stream_index 区分。',
    params: [
      { name: 's', desc: '格式上下文' },
      { name: 'pkt', desc: '输出参数，读取到的数据包' },
    ],
    returnValue: '0 成功，负数表示结束或错误',
    tips: [
      '每次调用后必须 av_packet_unref 释放包数据',
      '返回的包可能是任意流的，需要检查 stream_index',
      '名字叫 read_frame 但实际读的是 packet（历史原因）',
    ],
    sourceFile: 'libavformat/demux.c',
  },
  avcodec_send_packet: {
    name: 'avcodec_send_packet',
    signature: 'int avcodec_send_packet(AVCodecContext *avctx, const AVPacket *avpkt)',
    library: 'libavcodec',
    description: '将压缩数据包送入解码器。采用异步 send/receive 模式。',
    params: [
      { name: 'avctx', desc: '解码器上下文' },
      { name: 'avpkt', desc: '压缩数据包，传 nullptr 表示 flush（取出缓存帧）' },
    ],
    returnValue: '0 成功，EAGAIN 表示需要先 receive',
    tips: [
      '送入后不一定能立即取出帧（B帧需要缓存）',
      '文件结束后发送 nullptr 来 flush 解码器',
      '替代了旧的 avcodec_decode_video2（FFmpeg 3.1+）',
    ],
    sourceFile: 'libavcodec/decode.c',
  },
  avcodec_receive_frame: {
    name: 'avcodec_receive_frame',
    signature: 'int avcodec_receive_frame(AVCodecContext *avctx, AVFrame *frame)',
    library: 'libavcodec',
    description: '从解码器取出一帧解码后的原始数据。',
    params: [
      { name: 'avctx', desc: '解码器上下文' },
      { name: 'frame', desc: '输出参数，解码后的帧' },
    ],
    returnValue: '0 成功，EAGAIN 需要更多输入，EOF 全部输出完毕',
    tips: [
      '一次 send 后可能需要多次 receive（编码器输出多帧）',
      '视频帧: data[0]=Y, data[1]=U, data[2]=V',
      '音频帧: planar 格式 data[0]=左声道, data[1]=右声道',
    ],
    sourceFile: 'libavcodec/decode.c',
  },
  avcodec_send_frame: {
    name: 'avcodec_send_frame',
    signature: 'int avcodec_send_frame(AVCodecContext *avctx, const AVFrame *frame)',
    library: 'libavcodec',
    description: '将原始帧送入编码器。编码是解码的镜像操作。',
    params: [
      { name: 'avctx', desc: '编码器上下文' },
      { name: 'frame', desc: '原始帧，传 nullptr 表示 flush' },
    ],
    returnValue: '0 成功',
    tips: [
      'frame->pts 必须正确设置且递增',
      '编码器可能缓存帧（特别是有 B 帧时）',
      '音频编码器要求每次送入 frame_size 个采样点',
    ],
    sourceFile: 'libavcodec/encode.c',
  },
  avcodec_receive_packet: {
    name: 'avcodec_receive_packet',
    signature: 'int avcodec_receive_packet(AVCodecContext *avctx, AVPacket *avpkt)',
    library: 'libavcodec',
    description: '从编码器取出一个压缩数据包。',
    params: [
      { name: 'avctx', desc: '编码器上下文' },
      { name: 'avpkt', desc: '输出参数，编码后的数据包' },
    ],
    returnValue: '0 成功，EAGAIN 需要更多输入，EOF 全部输出完毕',
    tips: [
      '取出后需要设置正确的 stream_index 和时间戳',
      '用 av_packet_rescale_ts 转换时间基',
    ],
    sourceFile: 'libavcodec/encode.c',
  },
  sws_getContext: {
    name: 'sws_getContext',
    signature: 'struct SwsContext *sws_getContext(int srcW, int srcH, enum AVPixelFormat srcFormat, int dstW, int dstH, enum AVPixelFormat dstFormat, int flags, ...)',
    library: 'libswscale',
    description: '创建像素缩放/格式转换上下文。',
    params: [
      { name: 'srcW/srcH', desc: '源宽高' },
      { name: 'srcFormat', desc: '源像素格式（如 YUV420P）' },
      { name: 'dstW/dstH', desc: '目标宽高' },
      { name: 'dstFormat', desc: '目标像素格式（如 RGB24）' },
      { name: 'flags', desc: '缩放算法' },
    ],
    returnValue: 'SwsContext 指针，nullptr 表示失败',
    tips: [
      'SWS_FAST_BILINEAR: 最快，质量一般',
      'SWS_BILINEAR: 平衡',
      'SWS_BICUBIC: 质量好，稍慢',
      'SWS_LANCZOS: 最高质量，最慢',
    ],
    sourceFile: 'libswscale/swscale.c',
  },
  sws_scale: {
    name: 'sws_scale',
    signature: 'int sws_scale(struct SwsContext *c, const uint8_t *const srcSlice[], const int srcStride[], int srcSliceY, int srcSliceH, uint8_t *const dst[], const int dstStride[])',
    library: 'libswscale',
    description: '执行像素缩放和/或格式转换。',
    params: [
      { name: 'srcSlice', desc: '源数据指针数组（frame->data）' },
      { name: 'srcStride', desc: '源行宽数组（frame->linesize）' },
      { name: 'srcSliceY', desc: '起始行，通常为 0' },
      { name: 'srcSliceH', desc: '源高度' },
      { name: 'dst/dstStride', desc: '目标数据和行宽' },
    ],
    returnValue: '输出的行数',
    tips: [
      '可以同时缩放和转换像素格式',
      '不缩放只转格式也用这个函数',
    ],
    sourceFile: 'libswscale/swscale.c',
  },
  swr_alloc_set_opts: {
    name: 'swr_alloc_set_opts',
    signature: 'struct SwrContext *swr_alloc_set_opts(struct SwrContext *s, int64_t out_ch_layout, enum AVSampleFormat out_sample_fmt, int out_sample_rate, int64_t in_ch_layout, enum AVSampleFormat in_sample_fmt, int in_sample_rate, ...)',
    library: 'libswresample',
    description: '创建音频重采样上下文，可以同时转换采样率、声道数、采样格式。',
    params: [
      { name: 'out_ch_layout', desc: '输出声道布局（如 AV_CH_LAYOUT_STEREO）' },
      { name: 'out_sample_fmt', desc: '输出采样格式（如 AV_SAMPLE_FMT_FLTP）' },
      { name: 'out_sample_rate', desc: '输出采样率（如 44100）' },
      { name: 'in_*', desc: '输入参数，同上' },
    ],
    returnValue: 'SwrContext 指针',
    tips: [
      '创建后需要调用 swr_init 初始化',
      '可以只转格式不改采样率，或只改采样率不转格式',
    ],
    sourceFile: 'libswresample/swresample.c',
  },
  swr_convert: {
    name: 'swr_convert',
    signature: 'int swr_convert(struct SwrContext *s, uint8_t **out, int out_count, const uint8_t **in, int in_count)',
    library: 'libswresample',
    description: '执行音频重采样。',
    params: [
      { name: 'out', desc: '输出缓冲区' },
      { name: 'out_count', desc: '输出缓冲区最大采样数' },
      { name: 'in', desc: '输入数据（传 nullptr 来 flush）' },
      { name: 'in_count', desc: '输入采样数' },
    ],
    returnValue: '实际输出的采样数',
    tips: [
      '内部有缓存，flush 时传 in=nullptr',
      '输出采样数可能和输入不同（采样率转换）',
      '用 swr_get_delay 获取内部缓存延迟',
    ],
    sourceFile: 'libswresample/swresample.c',
  },
  avformat_write_header: {
    name: 'avformat_write_header',
    signature: 'int avformat_write_header(AVFormatContext *s, AVDictionary **options)',
    library: 'libavformat',
    description: '写入容器的文件头。不同格式的文件头结构不同。',
    params: [
      { name: 's', desc: '输出格式上下文' },
      { name: 'options', desc: '额外选项' },
    ],
    returnValue: '0 成功',
    tips: [
      'MP4: 写入 ftyp box',
      'FLV: 写入 FLV Header (9字节)',
      '必须在 av_interleaved_write_frame 之前调用',
    ],
    sourceFile: 'libavformat/mux.c',
  },
  av_interleaved_write_frame: {
    name: 'av_interleaved_write_frame',
    signature: 'int av_interleaved_write_frame(AVFormatContext *s, AVPacket *pkt)',
    library: 'libavformat',
    description: '写入一个数据包，自动按 DTS 排序交织音视频。',
    params: [
      { name: 's', desc: '输出格式上下文' },
      { name: 'pkt', desc: '要写入的数据包' },
    ],
    returnValue: '0 成功',
    tips: [
      '会自动缓存并按 DTS 排序，确保音视频交替写入',
      '比 av_write_frame 更安全，推荐使用',
      '写入前需要正确设置 stream_index 和时间戳',
    ],
    sourceFile: 'libavformat/mux.c',
  },
  avfilter_graph_alloc: {
    name: 'avfilter_graph_alloc',
    signature: 'AVFilterGraph *avfilter_graph_alloc(void)',
    library: 'libavfilter',
    description: '创建滤镜图。滤镜图是一个有向图，数据从 buffersrc 流入，经过滤镜链处理，从 buffersink 流出。',
    params: [],
    returnValue: 'AVFilterGraph 指针',
    tips: [
      '滤镜图结构: [buffersrc] -> [filter1] -> [filter2] -> [buffersink]',
      '用 avfilter_graph_parse_ptr 解析滤镜描述字符串',
      '用 avfilter_graph_config 验证和初始化',
    ],
    sourceFile: 'libavfilter/avfiltergraph.c',
  },
  av_buffersrc_add_frame: {
    name: 'av_buffersrc_add_frame',
    signature: 'int av_buffersrc_add_frame(AVFilterContext *ctx, AVFrame *frame)',
    library: 'libavfilter',
    description: '向滤镜图的输入端送入一帧。',
    params: [
      { name: 'ctx', desc: 'buffersrc 滤镜上下文' },
      { name: 'frame', desc: '输入帧，nullptr 表示 EOF' },
    ],
    returnValue: '0 成功',
    tips: [
      '送入后用 av_buffersink_get_frame 从输出端取出处理后的帧',
      '一次送入可能产生 0 或多个输出帧',
    ],
    sourceFile: 'libavfilter/buffersrc.c',
  },
};

export function getApiInfo(name: string): ApiInfo | undefined {
  return APIS[name];
}
