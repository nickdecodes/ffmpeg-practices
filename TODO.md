# TODO

## practices (C++ 教程)

- [ ] 每个模块的 README 加上"API 速查卡"表格（参数、返回值一目了然）
- [ ] 补充每个模块的预期输出示例（让读者知道跑对了是什么样）
- [ ] 加一个 29.rtmp_push 模块（RTMP 推流，对应原项目的 27）
- [ ] 加一个 30.hls_output 模块（HLS 切片输出）
- [ ] 加一个 31.audio_mix 模块（多路音频混音）
- [ ] 加一个 32.concat 模块（视频拼接）
- [ ] 加一个 33.watermark 模块（图片水印叠加，用 overlay 滤镜）
- [ ] 00.common/build.sh 支持 Windows (MSYS2/MinGW)
- [ ] 加 GitHub Actions CI，自动编译验证所有模块
- [ ] 每个模块加单元测试（用测试素材自动验证输出）

## visualizer (Web 可视化)

### 命令解析器增强
- [ ] 支持 -filter_complex（复杂滤镜图，多输入多输出）
- [ ] 支持多个 -i 输入（如画中画、拼接场景）
- [ ] 支持 -map 参数（手动选择流映射）
- [ ] 支持 -ss/-t/-to 在流程图中显示 seek/cut 节点
- [ ] 支持 -b:v/-b:a/-crf/-preset 在节点详情中显示
- [ ] 支持 -hwaccel 显示硬件加速节点
- [ ] 支持 -f rawvideo/-f f32le 等原始格式输入（无 demux 节点）
- [ ] 未识别参数的处理优化（显示在流程图旁边而不是只有警告）

### 知识库扩展
- [ ] 补充更多编解码器：ProRes, DNxHD, FLAC, Vorbis, DTS, AC-3
- [ ] 补充更多封装格式：RTMP, HLS(m3u8), DASH, SRT
- [ ] 补充滤镜知识库：scale, crop, overlay, drawtext, fps, pad 等常用滤镜
- [ ] 补充像素格式知识库：YUV420P, NV12, RGB24, YUV422P 等的内存布局图
- [ ] 补充音频采样格式知识库：s16, s32, flt, fltp 等
- [ ] 每个 API 加上"常见错误"和"排查方法"

### UI/UX 优化
- [ ] 深色模式
- [ ] 流程图节点加数据流动粒子动画（用 framer-motion）
- [ ] 节点之间显示数据格式变化（如 "H.264 packets" → "YUV420P frames"）
- [ ] 移动端适配（详情面板改为底部弹出）
- [ ] 命令输入框加自动补全（常用参数提示）
- [ ] 命令历史记录（localStorage）
- [ ] 分享功能（URL 参数编码当前命令）
- [ ] 国际化（中英文切换）

### 学习模式
- [ ] 预设教学流程：逐步高亮节点，配合文字讲解
- [ ] 每个预设命令配一个"为什么这样写"的解释
- [ ] 交互式测验：给一个需求，让用户拼出正确的 ffmpeg 命令
- [ ] 对比模式：同时显示两条命令的管线差异（如 -c copy vs -c:v libx264）

### 部署
- [ ] 配置 GitHub Pages 自动部署
- [ ] 加 PWA 支持（离线可用）
- [ ] 自定义域名

## 长期规划

- [ ] 拖拽搭建模式：用户拖拽节点组装管线，反向生成 ffmpeg 命令
- [ ] 接入 WebAssembly 版 FFmpeg（ffmpeg.wasm），在浏览器中真正执行命令
- [ ] 社区贡献机制：用户可以提交新的命令示例和知识条目
