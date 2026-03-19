import { Handle, Position } from '@xyflow/react';
import type { NodeProps } from '@xyflow/react';

interface StreamNodeData {
  label: string;
  description: string;
  nodeType: string;
  color: { bg: string; border: string };
  codec?: string;
  apis?: string[];
  practiceRef?: string;
  [key: string]: unknown;
}

export default function StreamNode({ data }: NodeProps) {
  const d = data as unknown as StreamNodeData;

  return (
    <div
      style={{
        padding: '12px 16px',
        borderRadius: 8,
        border: `2px solid ${d.color.border}`,
        backgroundColor: d.color.bg,
        minWidth: 150,
        cursor: 'pointer',
        boxShadow: '0 2px 8px rgba(0,0,0,0.1)',
        transition: 'box-shadow 0.2s',
      }}
    >
      <Handle type="target" position={Position.Left} style={{ background: d.color.border }} />
      <div style={{ fontWeight: 600, fontSize: 14, marginBottom: 4 }}>{d.label}</div>
      <div style={{ fontSize: 12, color: '#666', lineHeight: 1.4 }}>{d.description}</div>
      {d.codec && (
        <div style={{ fontSize: 11, color: '#999', marginTop: 4 }}>
          编码器: {d.codec}
        </div>
      )}
      <Handle type="source" position={Position.Right} style={{ background: d.color.border }} />
    </div>
  );
}
