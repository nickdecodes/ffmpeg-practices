/** 封装格式知识库 */
export interface FormatInfo {
  name: string;
  fullName: string;
  description: string;
  supportedCodecs: { video: string[]; audio: string[] };
  features: string[];
  limitations: string[];
  useCase: string;
}

export const FORMATS: Record<string, FormatInfo> = {
  mp4: {
    name: 'MP4',
    fullName: 'MPEG-4 Part 14',
    description: '最通用的容器格式，几乎所有设备和浏览器都支持。',
    supportedCodecs: {
      video: ['H.264', 'H.265', 'VP9', 'AV1'],
      audio: ['AAC', 'MP3', 'Opus', 'AC-3'],
    },
    features: ['支持流式传输', '支持字幕', '支持章节标记', '支持元数据'],
    limitations: ['不支持 VP8', '不适合直播推流'],
    useCase: '点播、本地存储、Web 播放',
  },
  flv: {
    name: 'FLV',
    fullName: 'Flash Video',
    description: 'RTMP 直播推流的标准格式，结构简单，延迟低。',
    supportedCodecs: {
      video: ['H.264', 'VP6'],
      audio: ['AAC', 'MP3'],
    },
    features: ['结构简单', '适合流式传输', 'RTMP 标配'],
    limitations: ['不支持 H.265', '不支持字幕', 'Flash 已淘汰'],
    useCase: 'RTMP 直播推流',
  },
  mkv: {
    name: 'MKV',
    fullName: 'Matroska',
    description: '功能最强大的开源容器，几乎支持所有编码格式。',
    supportedCodecs: {
      video: ['H.264', 'H.265', 'VP8', 'VP9', 'AV1', 'MPEG-4'],
      audio: ['AAC', 'MP3', 'Opus', 'FLAC', 'Vorbis', 'AC-3', 'DTS'],
    },
    features: ['支持几乎所有编码', '多音轨', '多字幕', '章节', '附件'],
    limitations: ['浏览器支持有限', '不适合流式传输'],
    useCase: '本地存储、高清视频、多语言内容',
  },
  ts: {
    name: 'MPEG-TS',
    fullName: 'MPEG Transport Stream',
    description: '广播电视和 HLS 直播的标准格式，抗错误能力强。',
    supportedCodecs: {
      video: ['H.264', 'H.265', 'MPEG-2'],
      audio: ['AAC', 'MP3', 'AC-3'],
    },
    features: ['抗传输错误', '适合广播', 'HLS 分片格式', '无需 seek 即可播放'],
    limitations: ['文件头开销大', '不支持 VP9/AV1'],
    useCase: '广播电视、HLS 直播、IPTV',
  },
  webm: {
    name: 'WebM',
    fullName: 'WebM (Google)',
    description: 'Google 推动的开源 Web 视频格式。',
    supportedCodecs: {
      video: ['VP8', 'VP9', 'AV1'],
      audio: ['Vorbis', 'Opus'],
    },
    features: ['免专利费', '浏览器原生支持', '适合 Web'],
    limitations: ['不支持 H.264/H.265', '编码速度慢'],
    useCase: 'Web 视频、YouTube',
  },
  avi: {
    name: 'AVI',
    fullName: 'Audio Video Interleave (Microsoft)',
    description: '最古老的视频容器之一，简单但功能有限。',
    supportedCodecs: {
      video: ['MPEG-4', 'H.264', 'DivX', 'Xvid'],
      audio: ['MP3', 'PCM', 'AC-3'],
    },
    features: ['结构简单', '兼容性好'],
    limitations: ['不支持流式传输', '不支持可变帧率', '不支持现代编码'],
    useCase: '老设备兼容、简单存储',
  },
  mov: {
    name: 'MOV',
    fullName: 'QuickTime File Format (Apple)',
    description: 'Apple 的容器格式，MP4 就是基于 MOV 发展而来。',
    supportedCodecs: {
      video: ['H.264', 'H.265', 'ProRes', 'VP9'],
      audio: ['AAC', 'ALAC', 'PCM'],
    },
    features: ['支持 ProRes', '专业视频制作标配', '与 MP4 高度兼容'],
    limitations: ['非 Apple 平台支持有限'],
    useCase: 'Apple 生态、专业视频制作',
  },
  wav: {
    name: 'WAV',
    fullName: 'Waveform Audio File Format',
    description: '未压缩音频的标准容器，内容就是 PCM 数据。',
    supportedCodecs: { video: [], audio: ['PCM', 'ADPCM', 'GSM'] },
    features: ['无损', '简单', '广泛支持'],
    limitations: ['文件巨大', '不支持视频'],
    useCase: '音频编辑、录音、无损存储',
  },
};

export function getFormatInfo(format: string): FormatInfo | undefined {
  return FORMATS[format.toLowerCase()];
}
