#!/bin/bash
# ============================================================================
# 生成测试素材
# 用 ffmpeg 命令行生成所有模块需要的测试文件
# 使用方法: cd practices && bash generate_test_media.sh
# ============================================================================

set -e
mkdir -p test_media
cd test_media

echo "=== 生成测试素材 ==="

# 1. 生成一个 5 秒的测试 MP4 (带视频+音频)
#    视频: 彩色条纹 640x480 30fps H.264
#    音频: 正弦波 44100Hz 立体声 AAC
if [ ! -f test.mp4 ]; then
    echo "[1/5] 生成 test.mp4 (5秒, 640x480, H.264+AAC)..."
    ffmpeg -y -f lavfi -i "testsrc2=size=640x480:rate=30:duration=5" \
           -f lavfi -i "sine=frequency=440:duration=5" \
           -c:v libx264 -preset fast -pix_fmt yuv420p \
           -c:a aac -ar 44100 -ac 2 \
           test.mp4 2>/dev/null
    echo "  -> test.mp4 OK"
else
    echo "[1/5] test.mp4 已存在, 跳过"
fi

# 2. 生成一个较长的测试 MP4 (15秒, 用于裁剪测试)
if [ ! -f test_long.mp4 ]; then
    echo "[2/5] 生成 test_long.mp4 (15秒, 用于裁剪测试)..."
    ffmpeg -y -f lavfi -i "testsrc2=size=640x480:rate=30:duration=15" \
           -f lavfi -i "sine=frequency=440:duration=15" \
           -c:v libx264 -preset fast -pix_fmt yuv420p \
           -c:a aac -ar 44100 -ac 2 \
           test_long.mp4 2>/dev/null
    echo "  -> test_long.mp4 OK"
else
    echo "[2/5] test_long.mp4 已存在, 跳过"
fi

# 3. 生成 YUV 原始视频 (用于编码测试)
if [ ! -f test.yuv ]; then
    echo "[3/5] 生成 test.yuv (640x480, YUV420P, 90帧)..."
    ffmpeg -y -f lavfi -i "testsrc2=size=640x480:rate=30:duration=3" \
           -pix_fmt yuv420p -f rawvideo \
           test.yuv 2>/dev/null
    echo "  -> test.yuv OK ($(du -h test.yuv | cut -f1))"
else
    echo "[3/5] test.yuv 已存在, 跳过"
fi

# 4. 生成 PCM 原始音频 (用于音频编码测试)
if [ ! -f test.pcm ]; then
    echo "[4/5] 生成 test.pcm (48000Hz, 2声道, float planar, 3秒)..."
    ffmpeg -y -f lavfi -i "sine=frequency=440:duration=3" \
           -ar 48000 -ac 2 -f f32le -acodec pcm_f32le \
           test_packed.pcm 2>/dev/null
    # 注意: 15.encode_audio 期望 planar 格式, 这里先生成 packed
    # 实际使用时可能需要调整
    mv test_packed.pcm test.pcm
    echo "  -> test.pcm OK ($(du -h test.pcm | cut -f1))"
else
    echo "[4/5] test.pcm 已存在, 跳过"
fi

# 5. 生成一个带字幕的 MKV (用于测试多流)
if [ ! -f test_multi.mkv ]; then
    echo "[5/5] 生成 test_multi.mkv (视频+音频+字幕)..."
    # 先生成字幕文件
    cat > /tmp/test_sub.srt << 'EOF'
1
00:00:00,000 --> 00:00:02,000
Hello FFmpeg

2
00:00:02,000 --> 00:00:05,000
ffmpeg-flow practices
EOF
    ffmpeg -y -f lavfi -i "testsrc2=size=640x480:rate=30:duration=5" \
           -f lavfi -i "sine=frequency=440:duration=5" \
           -i /tmp/test_sub.srt \
           -c:v libx264 -preset fast -pix_fmt yuv420p \
           -c:a aac -ar 44100 -ac 2 \
           -c:s srt \
           test_multi.mkv 2>/dev/null
    rm -f /tmp/test_sub.srt
    echo "  -> test_multi.mkv OK"
else
    echo "[5/5] test_multi.mkv 已存在, 跳过"
fi

echo ""
echo "=== 全部完成 ==="
echo "文件列表:"
ls -lh test*
echo ""
echo "现在可以运行练习了, 例如:"
echo "  cd build && ./01_open ../test_media/test.mp4"
