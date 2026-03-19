import { useState } from 'react';

interface CommandInputProps {
  onSubmit: (command: string) => void;
  presets: { label: string; command: string }[];
}

export default function CommandInput({ onSubmit, presets }: CommandInputProps) {
  const [value, setValue] = useState('ffmpeg -i input.mp4 -c:v libx264 -c:a aac -s 1280x720 output.mp4');

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    if (value.trim()) onSubmit(value.trim());
  };

  return (
    <div style={{ padding: '16px 20px', borderBottom: '1px solid #e0e0e0', backgroundColor: '#fff' }}>
      <form onSubmit={handleSubmit} style={{ display: 'flex', gap: 8 }}>
        <input
          value={value}
          onChange={(e) => setValue(e.target.value)}
          placeholder="输入 ffmpeg 命令..."
          style={{
            flex: 1, padding: '10px 14px', fontSize: 14,
            border: '2px solid #e0e0e0', borderRadius: 8,
            fontFamily: 'monospace', outline: 'none',
            transition: 'border-color 0.2s',
          }}
          onFocus={(e) => (e.target.style.borderColor = '#1976d2')}
          onBlur={(e) => (e.target.style.borderColor = '#e0e0e0')}
        />
        <button
          type="submit"
          style={{
            padding: '10px 20px', fontSize: 14, fontWeight: 600,
            backgroundColor: '#1976d2', color: '#fff', border: 'none',
            borderRadius: 8, cursor: 'pointer',
          }}
        >
          解析
        </button>
      </form>

      <div style={{ marginTop: 10, display: 'flex', gap: 8, flexWrap: 'wrap' }}>
        <span style={{ fontSize: 12, color: '#999', lineHeight: '28px' }}>示例:</span>
        {presets.map((p) => (
          <button
            key={p.label}
            onClick={() => { setValue(p.command); onSubmit(p.command); }}
            style={{
              padding: '4px 10px', fontSize: 12,
              backgroundColor: '#f5f5f5', border: '1px solid #ddd',
              borderRadius: 14, cursor: 'pointer', color: '#555',
            }}
          >
            {p.label}
          </button>
        ))}
      </div>
    </div>
  );
}
