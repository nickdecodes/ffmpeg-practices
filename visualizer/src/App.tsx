import { useState } from 'react';
import { parseCommand } from './parser/command-parser';
import type { Pipeline } from './parser/types';
import CommandInput from './components/CommandInput';
import FlowCanvas from './components/FlowCanvas';

const PRESETS = [
  { label: '转封装', command: 'ffmpeg -i input.mp4 -c copy output.flv' },
  { label: '视频转码', command: 'ffmpeg -i input.mp4 -c:v libx264 -c:a copy output.mp4' },
  { label: '完整转码', command: 'ffmpeg -i input.mp4 -c:v libx264 -c:a aac -s 1280x720 output.mp4' },
  { label: '提取音频', command: 'ffmpeg -i input.mp4 -vn -acodec copy output.aac' },
  { label: '提取视频', command: 'ffmpeg -i input.mp4 -an -vcodec copy output.h264' },
  { label: '缩放+滤镜', command: 'ffmpeg -i input.mp4 -vf "scale=640:480" -c:v libx264 output.mp4' },
  { label: '音频重采样', command: 'ffmpeg -i input.mp4 -c:v copy -ar 16000 -ac 1 -c:a aac output.mp4' },
];

const defaultPipeline = parseCommand(PRESETS[2].command);

export default function App() {
  const [pipeline, setPipeline] = useState<Pipeline>(defaultPipeline);

  const handleCommand = (cmd: string) => {
    setPipeline(parseCommand(cmd));
  };

  return (
    <div style={{ height: '100vh', display: 'flex', flexDirection: 'column' }}>
      <div style={{ padding: '12px 20px', backgroundColor: '#1a1a2e', color: '#fff' }}>
        <h1 style={{ margin: 0, fontSize: 20, fontWeight: 600 }}>
          ffmpeg-practices visualizer
        </h1>
        <p style={{ margin: '4px 0 0', fontSize: 13, color: '#aaa' }}>
          输入 ffmpeg 命令，查看数据流动过程。点击节点查看 API 详情。
        </p>
      </div>
      <CommandInput onSubmit={handleCommand} presets={PRESETS} />
      {pipeline.warnings.length > 0 && (
        <div style={{ padding: '8px 20px', backgroundColor: '#fff8e1', fontSize: 12, color: '#f57f17' }}>
          {pipeline.warnings.map((w, i) => <span key={i}>⚠️ {w} &nbsp;</span>)}
        </div>
      )}
      <div style={{ flex: 1 }}>
        <FlowCanvas pipeline={pipeline} />
      </div>
    </div>
  );
}
