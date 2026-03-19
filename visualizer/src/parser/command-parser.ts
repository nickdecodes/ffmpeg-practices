import { tokenize } from './tokenizer';
import type { Pipeline, StreamPipeline, PipelineNode } from './types';

/** 根据文件扩展名猜测格式 */
function guessFormat(filename: string): string | undefined {
  const ext = filename.split('.').pop()?.toLowerCase();
  const map: Record<string, string> = {
    mp4: 'MP4', mkv: 'Matroska', flv: 'FLV', avi: 'AVI',
    ts: 'MPEG-TS', mov: 'MOV', webm: 'WebM', mp3: 'MP3',
    aac: 'AAC', wav: 'WAV', ogg: 'OGG', h264: 'H.264 raw',
    h265: 'H.265 raw', hevc: 'H.265 raw', yuv: 'Raw YUV',
    pcm: 'Raw PCM', rgb: 'Raw RGB', bmp: 'BMP',
  };
  return ext ? map[ext] : undefined;
}

/** 猜测输入文件的编解码器 */
function guessCodecs(format?: string): { video?: string; audio?: string } {
  if (!format) return { video: 'h264', audio: 'aac' };
  const f = format.toLowerCase();
  if (f === 'mp4' || f === 'mov') return { video: 'h264', audio: 'aac' };
  if (f === 'flv') return { video: 'h264', audio: 'aac' };
  if (f === 'webm') return { video: 'vp9', audio: 'opus' };
  if (f === 'avi') return { video: 'mpeg4', audio: 'mp3' };
  if (f === 'matroska') return { video: 'h264', audio: 'aac' };
  return { video: 'h264', audio: 'aac' };
}

export function parseCommand(command: string): Pipeline {
  const tokens = tokenize(command);
  const warnings: string[] = [];

  // 解析结果
  const inputs: Pipeline['inputs'] = [];
  const outputs: Pipeline['outputs'] = [];
  let videoCodec = '';
  let audioCodec = '';
  let scaleSize = '';
  let videoFilter = '';
  let audioFilter = '';
  let noVideo = false;
  let noAudio = false;
  let audioRate = '';
  let audioChannels = '';
  const inputOptions: Record<string, string> = {};

  // 逐 token 解析
  let i = 0;
  let parsingInputOpts = true; // -i 之前的选项属于输入

  while (i < tokens.length) {
    const t = tokens[i];

    if (t === '-i' && i + 1 < tokens.length) {
      inputs.push({ file: tokens[i + 1], format: guessFormat(tokens[i + 1]), options: { ...inputOptions } });
      parsingInputOpts = false;
      i += 2;
    } else if (t === '-c:v' || t === '-vcodec') {
      videoCodec = tokens[++i] || '';
      i++;
    } else if (t === '-c:a' || t === '-acodec') {
      audioCodec = tokens[++i] || '';
      i++;
    } else if (t === '-c' && i + 1 < tokens.length) {
      videoCodec = audioCodec = tokens[++i] || '';
      i++;
    } else if (t === '-s' && i + 1 < tokens.length) {
      scaleSize = tokens[++i];
      i++;
    } else if (t === '-vf' && i + 1 < tokens.length) {
      videoFilter = tokens[++i];
      i++;
    } else if (t === '-af' && i + 1 < tokens.length) {
      audioFilter = tokens[++i];
      i++;
    } else if (t === '-vn') {
      noVideo = true;
      i++;
    } else if (t === '-an') {
      noAudio = true;
      i++;
    } else if (t === '-ar' && i + 1 < tokens.length) {
      audioRate = tokens[++i];
      i++;
    } else if (t === '-ac' && i + 1 < tokens.length) {
      audioChannels = tokens[++i];
      i++;
    } else if (t === '-f' && parsingInputOpts && i + 1 < tokens.length) {
      inputOptions['format'] = tokens[++i];
      i++;
    } else if (t === '-ss' || t === '-t' || t === '-to' || t === '-r' ||
               t === '-b:v' || t === '-b:a' || t === '-preset' || t === '-crf' ||
               t === '-pix_fmt' || t === '-f' || t === '-framerate' ||
               t === '-video_size' || t === '-hwaccel') {
      // 已知参数，跳过值
      i += 2;
    } else if (t.startsWith('-bsf:')) {
      i += 2;
    } else if (t.startsWith('-')) {
      // 未知参数
      if (i + 1 < tokens.length && !tokens[i + 1].startsWith('-')) {
        warnings.push(`未识别参数: ${t} ${tokens[i + 1]}`);
        i += 2;
      } else {
        warnings.push(`未识别参数: ${t}`);
        i++;
      }
    } else {
      // 非 - 开头的 token，视为输出文件
      outputs.push({ file: t, format: guessFormat(t) });
      i++;
    }
  }

  // 如果没有输入，给个提示
  if (inputs.length === 0) {
    warnings.push('未检测到输入文件 (-i)');
  }
  if (outputs.length === 0) {
    warnings.push('未检测到输出文件');
  }

  // 构建管线
  const streams: StreamPipeline[] = [];
  const inputFormat = inputs[0]?.format;
  const inputCodecs = guessCodecs(inputFormat);

  // 视频管线
  if (!noVideo) {
    const nodes: PipelineNode[] = [];

    nodes.push({
      type: 'demux',
      label: '解封装',
      description: `从 ${inputFormat || '容器'} 中提取视频包`,
      apis: ['avformat_open_input', 'av_read_frame'],
      practiceRef: '03',
    });

    const isCopy = videoCodec === 'copy';

    if (!isCopy) {
      nodes.push({
        type: 'decode',
        label: '解码',
        description: `${inputCodecs.video?.toUpperCase() || '视频'} → YUV 原始像素`,
        codec: inputCodecs.video,
        apis: ['avcodec_send_packet', 'avcodec_receive_frame'],
        practiceRef: '10',
      });

      if (scaleSize) {
        nodes.push({
          type: 'scale',
          label: `缩放 ${scaleSize}`,
          description: `分辨率缩放到 ${scaleSize}`,
          params: { size: scaleSize },
          apis: ['sws_getContext', 'sws_scale'],
          practiceRef: '13',
        });
      }

      if (videoFilter) {
        nodes.push({
          type: 'filter',
          label: `滤镜`,
          description: videoFilter,
          params: { filter: videoFilter },
          apis: ['avfilter_graph_alloc', 'av_buffersrc_add_frame'],
          practiceRef: '21',
        });
      }

      nodes.push({
        type: 'encode',
        label: '编码',
        description: `YUV → ${(videoCodec || 'libx264').toUpperCase()} 压缩`,
        codec: videoCodec || 'libx264',
        apis: ['avcodec_send_frame', 'avcodec_receive_packet'],
        practiceRef: '14',
      });
    } else {
      nodes.push({
        type: 'copy',
        label: '直接拷贝',
        description: '不编解码，直接传递压缩数据',
        practiceRef: '07',
      });
    }

    nodes.push({
      type: 'mux',
      label: '封装',
      description: `写入 ${outputs[0]?.format || '容器'}`,
      apis: ['avformat_write_header', 'av_interleaved_write_frame'],
      practiceRef: '07',
    });

    streams.push({ streamType: 'video', nodes });
  }

  // 音频管线
  if (!noAudio) {
    const nodes: PipelineNode[] = [];

    nodes.push({
      type: 'demux',
      label: '解封装',
      description: `从 ${inputFormat || '容器'} 中提取音频包`,
      apis: ['avformat_open_input', 'av_read_frame'],
      practiceRef: '03',
    });

    const isCopy = audioCodec === 'copy';

    if (!isCopy) {
      nodes.push({
        type: 'decode',
        label: '解码',
        description: `${inputCodecs.audio?.toUpperCase() || '音频'} → PCM 原始采样`,
        codec: inputCodecs.audio,
        apis: ['avcodec_send_packet', 'avcodec_receive_frame'],
        practiceRef: '11',
      });

      if (audioRate || audioChannels) {
        const desc = [audioRate && `${audioRate}Hz`, audioChannels && `${audioChannels}声道`].filter(Boolean).join(', ');
        nodes.push({
          type: 'resample',
          label: `重采样 ${desc}`,
          description: `音频重采样: ${desc}`,
          apis: ['swr_alloc_set_opts', 'swr_convert'],
          practiceRef: '16',
        });
      }

      if (audioFilter) {
        nodes.push({
          type: 'filter',
          label: '音频滤镜',
          description: audioFilter,
          practiceRef: '21',
        });
      }

      nodes.push({
        type: 'encode',
        label: '编码',
        description: `PCM → ${(audioCodec || 'aac').toUpperCase()} 压缩`,
        codec: audioCodec || 'aac',
        apis: ['avcodec_send_frame', 'avcodec_receive_packet'],
        practiceRef: '15',
      });
    } else {
      nodes.push({
        type: 'copy',
        label: '直接拷贝',
        description: '不编解码，直接传递压缩数据',
        practiceRef: '07',
      });
    }

    nodes.push({
      type: 'mux',
      label: '封装',
      description: `写入 ${outputs[0]?.format || '容器'}`,
      apis: ['avformat_write_header', 'av_interleaved_write_frame'],
      practiceRef: '07',
    });

    streams.push({ streamType: 'audio', nodes });
  }

  return { command, inputs, outputs, streams, warnings };
}
