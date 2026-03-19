/** 流类型 */
export type StreamType = 'video' | 'audio' | 'subtitle';

/** 管线节点类型 */
export type NodeType =
  | 'input'
  | 'demux'
  | 'decode'
  | 'filter'
  | 'scale'
  | 'resample'
  | 'encode'
  | 'mux'
  | 'output'
  | 'copy';

/** 管线中的一个处理节点 */
export interface PipelineNode {
  type: NodeType;
  label: string;
  description: string;
  codec?: string;
  params?: Record<string, string>;
  /** 对应 practices 中的模块编号 */
  practiceRef?: string;
  /** 对应的核心 FFmpeg API */
  apis?: string[];
}

/** 一条流的处理管线 */
export interface StreamPipeline {
  streamType: StreamType;
  nodes: PipelineNode[];
}

/** 解析后的完整管线 */
export interface Pipeline {
  command: string;
  inputs: { file: string; format?: string; options: Record<string, string> }[];
  outputs: { file: string; format?: string }[];
  streams: StreamPipeline[];
  warnings: string[];
}
