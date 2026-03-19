import { useMemo, useCallback, useState, useEffect } from 'react';
import {
  ReactFlow,
  Background,
  Controls,
  BackgroundVariant,
  useNodesState,
  useEdgesState,
} from '@xyflow/react';
import type { Node, Edge } from '@xyflow/react';
import '@xyflow/react/dist/style.css';
import type { Pipeline, PipelineNode as PNode, StreamPipeline } from '../parser/types';
import StreamNode from './StreamNode';
import DetailPanel from './DetailPanel';

const nodeTypes = { streamNode: StreamNode };

/** 节点颜色 */
const COLORS = {
  video: { bg: '#e8f5e9', border: '#4caf50' },
  audio: { bg: '#e3f2fd', border: '#2196f3' },
  input: { bg: '#fff3e0', border: '#ff9800' },
  output: { bg: '#fce4ec', border: '#e91e63' },
};

function buildFlowElements(pipeline: Pipeline) {
  const nodes: Node[] = [];
  const edges: Edge[] = [];

  if (pipeline.inputs.length === 0 && pipeline.outputs.length === 0) {
    return { nodes, edges };
  }

  const X_START = 50;
  const X_STEP = 220;
  const Y_START = 80;
  const Y_STEP = 140;

  // 输入节点
  pipeline.inputs.forEach((input, idx) => {
    nodes.push({
      id: `input-${idx}`,
      type: 'streamNode',
      position: { x: X_START, y: Y_START + idx * Y_STEP * pipeline.streams.length },
      data: {
        label: input.file,
        description: input.format || '输入文件',
        nodeType: 'input',
        color: COLORS.input,
      },
    });
  });

  // 每条流的管线
  pipeline.streams.forEach((stream: StreamPipeline, sIdx: number) => {
    const y = Y_START + sIdx * Y_STEP;
    const color = stream.streamType === 'video' ? COLORS.video : COLORS.audio;
    const prefix = stream.streamType === 'video' ? '🎬' : '🔊';

    stream.nodes.forEach((node: PNode, nIdx: number) => {
      const nodeId = `${stream.streamType}-${nIdx}`;
      const x = X_START + X_STEP * (nIdx + 1);

      nodes.push({
        id: nodeId,
        type: 'streamNode',
        position: { x, y },
        data: {
          label: `${prefix} ${node.label}`,
          description: node.description,
          nodeType: node.type,
          color,
          codec: node.codec,
          apis: node.apis,
          practiceRef: node.practiceRef,
          params: node.params,
        },
      });

      // 连线
      const prevId = nIdx === 0 ? 'input-0' : `${stream.streamType}-${nIdx - 1}`;
      edges.push({
        id: `e-${prevId}-${nodeId}`,
        source: prevId,
        target: nodeId,
        animated: true,
        style: { stroke: color.border, strokeWidth: 2 },
      });
    });

    // 最后一个节点连到输出
    if (pipeline.outputs.length > 0 && stream.nodes.length > 0) {
      const lastNodeId = `${stream.streamType}-${stream.nodes.length - 1}`;
      edges.push({
        id: `e-${lastNodeId}-output-0`,
        source: lastNodeId,
        target: 'output-0',
        animated: true,
        style: { stroke: color.border, strokeWidth: 2 },
      });
    }
  });

  // 输出节点
  pipeline.outputs.forEach((output, idx) => {
    const maxNodes = Math.max(...pipeline.streams.map(s => s.nodes.length), 1);
    nodes.push({
      id: `output-${idx}`,
      type: 'streamNode',
      position: {
        x: X_START + X_STEP * (maxNodes + 1),
        y: Y_START + idx * Y_STEP + (pipeline.streams.length > 1 ? Y_STEP / 2 : 0),
      },
      data: {
        label: output.file,
        description: output.format || '输出文件',
        nodeType: 'output',
        color: COLORS.output,
      },
    });
  });

  return { nodes, edges };
}

interface FlowCanvasProps {
  pipeline: Pipeline;
}

export default function FlowCanvas({ pipeline }: FlowCanvasProps) {
  const { nodes: initialNodes, edges: initialEdges } = useMemo(
    () => buildFlowElements(pipeline),
    [pipeline]
  );

  const [nodes, setNodes, onNodesChange] = useNodesState(initialNodes);
  const [edges, setEdges, onEdgesChange] = useEdgesState(initialEdges);
  const [selectedNode, setSelectedNode] = useState<Node | null>(null);

  // pipeline 变化时更新节点和边
  useEffect(() => {
    const { nodes: newNodes, edges: newEdges } = buildFlowElements(pipeline);
    setNodes(newNodes);
    setEdges(newEdges);
    setSelectedNode(null);
  }, [pipeline, setNodes, setEdges]);

  const onNodeClick = useCallback((_: React.MouseEvent, node: Node) => {
    setSelectedNode(node);
  }, []);

  return (
    <div style={{ display: 'flex', height: '100%' }}>
      <div style={{ flex: 1 }}>
        <ReactFlow
          nodes={nodes}
          edges={edges}
          onNodesChange={onNodesChange}
          onEdgesChange={onEdgesChange}
          onNodeClick={onNodeClick}
          nodeTypes={nodeTypes}
          fitView
          minZoom={0.3}
          maxZoom={2}
        >
          <Background variant={BackgroundVariant.Dots} gap={20} size={1} />
          <Controls />
        </ReactFlow>
      </div>
      {selectedNode && (
        <DetailPanel node={selectedNode} onClose={() => setSelectedNode(null)} />
      )}
    </div>
  );
}
