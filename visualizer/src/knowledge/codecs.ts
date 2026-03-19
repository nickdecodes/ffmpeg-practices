/** 编解码器知识库 */
export interface CodecInfo {
  name: string;
  fullName: string;
  type: 'video' | 'audio';
  description: string;
  pros: string[];
  cons: string[];
  commonParams: { name: string; desc: string; example: string }[];
  sourceFile?: string;
}

export const CODECS: Record<string, CodecInfo> = {
  h264: {
    name: 'H.264',
    fullName: 'H.264 / AVC (Advanced Video Coding)',
    type: 'video',
    description: '目前最广泛使用的视频编码标准，几乎所有设备和平台都支持。',
    pros: ['兼容性最好', '硬件解码支持广泛', '成熟稳定'],
    cons: ['压缩率不如 H.265', '专利费用（但实际使用中基本免费）'],
    commonParams: [
      { name: 'preset', desc: '编码速度/质量权衡', example: 'ultrafast, fast, medium, slow, veryslow' },
      { name: 'crf', desc: '恒定质量因子，越小质量越高', example: '18(高质量) 23(默认) 28(低质量)' },
      { name: 'profile', desc: '编码档次', example: 'baseline, main, high' },
      { name: 'tune', desc: '针对特定内容优化', example: 'film, animation, zerolatency' },
    ],
    sourceFile: 'libavcodec/libx264.c',
  },
  libx264: {
    name: 'libx264',
    fullName: 'x264 H.264 编码器',
    type: 'video',
    description: '最常用的 H.264 软件编码器，质量和速度的平衡最好。',
    pros: ['编码质量优秀', '参数丰富可调', '开源免费'],
    cons: ['纯 CPU 编码，速度不如硬件编码器'],
    commonParams: [
      { name: 'preset', desc: '速度/质量权衡', example: 'ultrafast ~ veryslow' },
      { name: 'crf', desc: '恒定质量', example: '0(无损) ~ 51(最差)，默认23' },
      { name: 'bitrate', desc: '目标码率', example: '-b:v 2M' },
    ],
    sourceFile: 'libavcodec/libx264.c',
  },
  h265: {
    name: 'H.265',
    fullName: 'H.265 / HEVC (High Efficiency Video Coding)',
    type: 'video',
    description: 'H.264 的继任者，同等质量下码率减少约 50%。',
    pros: ['压缩率比 H.264 高 40-50%', '支持 8K 分辨率'],
    cons: ['编码速度慢', '硬件支持不如 H.264 广泛', '专利问题复杂'],
    commonParams: [
      { name: 'preset', desc: '速度/质量权衡', example: 'ultrafast ~ veryslow' },
      { name: 'crf', desc: '恒定质量', example: '默认28，比 H.264 的23视觉质量相当' },
    ],
    sourceFile: 'libavcodec/libx265.c',
  },
  vp9: {
    name: 'VP9',
    fullName: 'VP9 (Google)',
    type: 'video',
    description: 'Google 开发的开源视频编码格式，YouTube 大量使用。',
    pros: ['免专利费', '压缩率接近 H.265', 'WebM 容器的标配'],
    cons: ['编码速度很慢', '硬件支持有限'],
    commonParams: [
      { name: 'crf', desc: '质量', example: '0-63，默认31' },
      { name: 'speed', desc: '编码速度', example: '0(最慢最好) ~ 8(最快)' },
    ],
    sourceFile: 'libavcodec/libvpxenc.c',
  },
  av1: {
    name: 'AV1',
    fullName: 'AV1 (Alliance for Open Media)',
    type: 'video',
    description: '最新一代开源视频编码格式，压缩率最高，但编码极慢。',
    pros: ['免专利费', '压缩率最高', '未来趋势'],
    cons: ['编码速度极慢', '硬件支持刚起步'],
    commonParams: [
      { name: 'crf', desc: '质量', example: '0-63' },
      { name: 'cpu-used', desc: '速度', example: '0-8' },
    ],
    sourceFile: 'libavcodec/libaomenc.c',
  },
  aac: {
    name: 'AAC',
    fullName: 'AAC (Advanced Audio Coding)',
    type: 'audio',
    description: '最常用的有损音频编码格式，MP4 容器的标配音频格式。',
    pros: ['音质好', '兼容性广', '低码率表现优秀'],
    cons: ['有损压缩'],
    commonParams: [
      { name: 'bitrate', desc: '目标码率', example: '-b:a 128k, 192k, 256k' },
      { name: 'profile', desc: 'AAC 档次', example: 'aac_low, aac_he, aac_he_v2' },
    ],
    sourceFile: 'libavcodec/aacenc.c',
  },
  mp3: {
    name: 'MP3',
    fullName: 'MP3 (MPEG-1 Audio Layer III)',
    type: 'audio',
    description: '最经典的音频压缩格式，虽然老但兼容性无敌。',
    pros: ['兼容性最好', '所有设备都支持'],
    cons: ['同码率下音质不如 AAC', '格式较老'],
    commonParams: [
      { name: 'bitrate', desc: '目标码率', example: '-b:a 128k, 192k, 320k' },
      { name: 'quality', desc: 'VBR 质量', example: '-q:a 0(最好) ~ 9(最差)' },
    ],
    sourceFile: 'libavcodec/libmp3lame.c',
  },
  opus: {
    name: 'Opus',
    fullName: 'Opus (IETF)',
    type: 'audio',
    description: '最先进的开源音频编码格式，低延迟，WebRTC 标配。',
    pros: ['低延迟', '低码率音质极好', '免专利费', '适合语音和音乐'],
    cons: ['容器支持有限（主要用 WebM/OGG）'],
    commonParams: [
      { name: 'bitrate', desc: '目标码率', example: '-b:a 64k(语音) 128k(音乐)' },
      { name: 'application', desc: '应用场景', example: 'voip, audio, lowdelay' },
    ],
    sourceFile: 'libavcodec/libopusenc.c',
  },
  pcm: {
    name: 'PCM',
    fullName: 'PCM (Pulse Code Modulation)',
    type: 'audio',
    description: '未压缩的原始音频数据，WAV 文件的内容就是 PCM。',
    pros: ['无损', '无延迟', '最简单'],
    cons: ['文件巨大（1分钟立体声44.1kHz约10MB）'],
    commonParams: [
      { name: 'sample_fmt', desc: '采样格式', example: 's16le, s32le, f32le' },
      { name: 'sample_rate', desc: '采样率', example: '44100, 48000, 16000' },
    ],
  },
};

export function getCodecInfo(codec: string): CodecInfo | undefined {
  const key = codec.toLowerCase().replace('lib', '');
  return CODECS[codec.toLowerCase()] || CODECS[key];
}
