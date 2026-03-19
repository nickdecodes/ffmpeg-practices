import { useState, useRef, useEffect } from 'react';
import type { Node } from '@xyflow/react';
import { getCodecInfo } from '../knowledge/codecs';
import { getApiInfo } from '../knowledge/apis';

interface DetailPanelProps {
  node: Node;
  onClose: () => void;
}

interface NodeData {
  label?: string;
  description?: string;
  nodeType?: string;
  apis?: string[];
  practiceRef?: string;
  codec?: string;
  params?: Record<string, string>;
}

const PRACTICE_NAMES: Record<string, string> = {
  '01': '01.open', '02': '02.streams', '03': '03.packets',
  '04': '04.metadata', '05': '05.demux_audio',
  '06': '06.demux_video_annexb', '07': '07.remux', '08': '08.cut',
  '09': '09.decode_video_frame', '10': '10.decode_video_yuv',
  '11': '11.decode_audio_pcm', '12': '12.timestamp_deep',
  '13': '13.scale', '14': '14.encode_video',
  '15': '15.encode_audio', '16': '16.resample_audio',
  '17': '17.transcode_video', '18': '18.transcode_full',
  '19': '19.encode_params', '20': '20.pixel_convert',
  '21': '21.filter_graph', '22': '22.custom_io',
};

/** 可折叠的 API 卡片 */
function ApiCard({ apiName }: { apiName: string }) {
  const [expanded, setExpanded] = useState(false);
  const info = getApiInfo(apiName);

  if (!info) {
    return (
      <div style={{
        background: '#e8e8e8', padding: '4px 8px', borderRadius: 4,
        fontSize: 12, fontFamily: 'monospace', marginBottom: 4,
      }}>
        {apiName}()
      </div>
    );
  }

  return (
    <div style={{
      background: '#fff', border: '1px solid #e0e0e0', borderRadius: 6,
      marginBottom: 8, overflow: 'hidden',
      transition: 'box-shadow 0.2s',
      boxShadow: expanded ? '0 2px 8px rgba(0,0,0,0.08)' : 'none',
    }}>
      {/* 可点击的标题栏 */}
      <div
        onClick={() => setExpanded(!expanded)}
        style={{
          padding: '8px 10px', cursor: 'pointer',
          display: 'flex', justifyContent: 'space-between', alignItems: 'center',
          borderBottom: expanded ? '1px solid #f0f0f0' : 'none',
        }}
      >
        <div style={{ fontFamily: 'monospace', fontSize: 12, fontWeight: 600, color: '#1976d2' }}>
          {info.name}()
        </div>
        <span style={{ fontSize: 10, color: '#bbb', transition: 'transform 0.2s',
          transform: expanded ? 'rotate(180deg)' : 'rotate(0deg)' }}>
          ▼
        </span>
      </div>

      {/* 简要描述（始终显示） */}
      <div style={{ padding: '4px 10px 8px', fontSize: 12, color: '#666' }}>
        {info.description}
      </div>

      {/* 展开的详细内容 */}
      {expanded && (
        <div style={{ padding: '0 10px 10px' }}>
          {/* 函数签名 */}
          <div style={{
            background: '#f5f5f5', padding: 8, borderRadius: 4, marginBottom: 8,
            fontSize: 11, fontFamily: 'monospace', color: '#333',
            overflowX: 'auto', whiteSpace: 'pre-wrap', wordBreak: 'break-all',
          }}>
            {info.signature}
          </div>

          {/* 参数 */}
          {info.params.length > 0 && (
            <div style={{ marginBottom: 8 }}>
              <div style={{ fontSize: 11, fontWeight: 600, color: '#555', marginBottom: 4 }}>参数</div>
              {info.params.map(p => (
                <div key={p.name} style={{ fontSize: 11, marginBottom: 3, paddingLeft: 6 }}>
                  <code style={{ color: '#e65100', fontWeight: 600 }}>{p.name}</code>
                  <span style={{ color: '#888' }}> — {p.desc}</span>
                </div>
              ))}
            </div>
          )}

          {/* 返回值 */}
          <div style={{ fontSize: 11, color: '#888', marginBottom: 8 }}>
            <span style={{ fontWeight: 600, color: '#555' }}>返回:</span> {info.returnValue}
          </div>

          {/* 提示 */}
          <div style={{
            background: '#fffde7', borderRadius: 4, padding: 8,
            borderLeft: '3px solid #ffc107', marginBottom: 6,
          }}>
            {info.tips.map((tip, i) => (
              <div key={i} style={{ fontSize: 11, color: '#555', lineHeight: 1.7 }}>
                💡 {tip}
              </div>
            ))}
          </div>

          <div style={{ fontSize: 10, color: '#bbb', textAlign: 'right' as const }}>
            {info.library} · {info.sourceFile}
          </div>
        </div>
      )}
    </div>
  );
}

export default function DetailPanel({ node, onClose }: DetailPanelProps) {
  const d = (node.data || {}) as NodeData;
  const codecInfo = d.codec ? getCodecInfo(d.codec) : undefined;
  const scrollRef = useRef<HTMLDivElement>(null);

  // 切换节点时滚回顶部
  useEffect(() => {
    scrollRef.current?.scrollTo({ top: 0, behavior: 'smooth' });
  }, [node.id]);

  return (
    <div style={{
      width: 360, display: 'flex', flexDirection: 'column',
      borderLeft: '1px solid #e0e0e0', backgroundColor: '#fafafa',
    }}>
      {/* 固定标题栏 */}
      <div style={{
        padding: '14px 16px', borderBottom: '1px solid #e0e0e0',
        backgroundColor: '#fff', flexShrink: 0,
        display: 'flex', justifyContent: 'space-between', alignItems: 'center',
      }}>
        <h3 style={{ margin: 0, fontSize: 15, fontWeight: 600 }}>{d.label}</h3>
        <button onClick={onClose} style={{
          border: 'none', background: '#f5f5f5', width: 28, height: 28,
          borderRadius: 14, fontSize: 16, cursor: 'pointer', color: '#999',
          display: 'flex', alignItems: 'center', justifyContent: 'center',
        }}>×</button>
      </div>

      {/* 可滚动内容区 */}
      <div ref={scrollRef} style={{
        flex: 1, overflowY: 'auto', padding: 16, fontSize: 13,
      }}>
        <p style={{ color: '#666', lineHeight: 1.6, marginBottom: 16 }}>{d.description}</p>

        {/* 编解码器详情 */}
        {codecInfo && (
          <div style={{ marginBottom: 16 }}>
            <h4 style={{ fontSize: 13, color: '#333', marginBottom: 6, fontWeight: 600 }}>
              📦 {codecInfo.fullName}
            </h4>
            <p style={{ fontSize: 12, color: '#666', marginBottom: 8 }}>{codecInfo.description}</p>

            <div style={{
              display: 'flex', gap: 8, marginBottom: 8, flexWrap: 'wrap',
            }}>
              {codecInfo.pros.map(p => (
                <span key={p} style={{
                  fontSize: 11, padding: '2px 8px', borderRadius: 10,
                  background: '#e8f5e9', color: '#2e7d32',
                }}>✓ {p}</span>
              ))}
              {codecInfo.cons.map(c => (
                <span key={c} style={{
                  fontSize: 11, padding: '2px 8px', borderRadius: 10,
                  background: '#fce4ec', color: '#c62828',
                }}>✗ {c}</span>
              ))}
            </div>

            {codecInfo.commonParams.length > 0 && (
              <div style={{
                background: '#fff', border: '1px solid #e0e0e0', borderRadius: 6, padding: 10,
              }}>
                <div style={{ fontSize: 11, fontWeight: 600, color: '#555', marginBottom: 6 }}>常用参数</div>
                {codecInfo.commonParams.map(p => (
                  <div key={p.name} style={{ marginBottom: 6 }}>
                    <code style={{
                      background: '#e3f2fd', color: '#1565c0', padding: '1px 6px',
                      borderRadius: 3, fontSize: 12, fontFamily: 'monospace',
                    }}>{p.name}</code>
                    <span style={{ fontSize: 12, color: '#666', marginLeft: 6 }}>{p.desc}</span>
                    <div style={{ fontSize: 11, color: '#999', marginTop: 2, paddingLeft: 4 }}>{p.example}</div>
                  </div>
                ))}
              </div>
            )}

            {codecInfo.sourceFile && (
              <div style={{ fontSize: 11, color: '#999', marginTop: 8 }}>
                📄 源码位置: <code>{codecInfo.sourceFile}</code>
              </div>
            )}
          </div>
        )}

        {/* API 详情 (可折叠卡片) */}
        {d.apis && d.apis.length > 0 && (
          <div style={{ marginBottom: 16 }}>
            <h4 style={{ fontSize: 13, color: '#333', marginBottom: 8, fontWeight: 600 }}>
              🔧 核心 API <span style={{ fontSize: 11, color: '#999', fontWeight: 400 }}>（点击展开详情）</span>
            </h4>
            {d.apis.map(apiName => (
              <ApiCard key={apiName} apiName={apiName} />
            ))}
          </div>
        )}

        {/* 参数 */}
        {d.params && Object.keys(d.params).length > 0 && (
          <div style={{ marginBottom: 16 }}>
            <h4 style={{ fontSize: 13, color: '#333', marginBottom: 6, fontWeight: 600 }}>⚙️ 参数</h4>
            {Object.entries(d.params).map(([k, v]) => (
              <div key={k} style={{ fontSize: 12, marginBottom: 4 }}>
                <code style={{
                  background: '#e8e8e8', padding: '2px 8px', borderRadius: 4,
                  fontSize: 12, fontFamily: 'monospace',
                }}>{k}</code>
                <span style={{ marginLeft: 8, color: '#555' }}>{v}</span>
              </div>
            ))}
          </div>
        )}

        {/* 对应练习 */}
        {d.practiceRef && PRACTICE_NAMES[d.practiceRef] && (
          <div style={{
            background: '#e3f2fd', borderRadius: 6, padding: 12,
            borderLeft: '3px solid #1976d2',
          }}>
            <div style={{ fontSize: 12, fontWeight: 600, color: '#1565c0', marginBottom: 4 }}>
              📂 对应练习代码
            </div>
            <a
              href={`https://github.com/user/ffmpeg-practices/tree/main/practices/${PRACTICE_NAMES[d.practiceRef]}`}
              target="_blank"
              rel="noopener noreferrer"
              style={{ fontSize: 13, color: '#1976d2', textDecoration: 'none' }}
            >
              practices/{PRACTICE_NAMES[d.practiceRef]}/main.cpp →
            </a>
          </div>
        )}
      </div>
    </div>
  );
}
