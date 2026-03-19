/**
 * ============================================================================
 * 06. 提取视频流为 H.264 (Annex B 格式)
 * ============================================================================
 *
 * 等价命令:
 *   ffmpeg -i input.mp4 -an -vcodec copy -bsf:v h264_mp4toannexb output.h264
 *
 * 新增 API: AVBitStreamFilter 系列
 *
 * AVCC vs Annex B:
 *   MP4 中的 H.264 用 AVCC 格式打包:
 *     [4字节长度][NALU数据][4字节长度][NALU数据]...
 *     SPS/PPS 存在 extradata 里, 不在数据包中
 *
 *   .h264 裸流文件用 Annex B 格式:
 *     [00 00 00 01][NALU数据][00 00 00 01][NALU数据]...
 *     SPS/PPS 作为普通 NALU 出现在流中
 *
 *   h264_mp4toannexb 过滤器做的就是这个转换。
 */

#include <cstdio>
#include <cstring>
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavcodec/bsf.h"
#include "libavutil/log.h"
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("用法: %s <输入文件> <输出.h264>\n", argv[0]);
        return -1;
    }

    AVFormatContext *fmt_ctx = nullptr;
    int ret = avformat_open_input(&fmt_ctx, argv[1], nullptr, nullptr);
    if (ret < 0) { printf("打开失败: %s\n", av_err2str(ret)); return -1; }
    avformat_find_stream_info(fmt_ctx, nullptr);

    int video_idx = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (video_idx < 0) { printf("没有视频流\n"); return -1; }

    AVCodecParameters *par = fmt_ctx->streams[video_idx]->codecpar;
    printf("找到视频流 #%d: %s, %dx%d\n", video_idx,
           avcodec_get_name(par->codec_id), par->width, par->height);

    // === 新内容: 初始化码流过滤器 ===

    // 1. 按名称查找过滤器
    const AVBitStreamFilter *bsf = av_bsf_get_by_name("h264_mp4toannexb");
    if (!bsf) { printf("找不到 h264_mp4toannexb 过滤器\n"); return -1; }

    // 2. 分配过滤器上下文
    AVBSFContext *bsf_ctx = nullptr;
    av_bsf_alloc(bsf, &bsf_ctx);

    // 3. 把输入流的编码参数复制给过滤器
    //    过滤器需要知道 extradata (SPS/PPS) 才能正确转换
    avcodec_parameters_copy(bsf_ctx->par_in, par);

    // 4. 初始化
    av_bsf_init(bsf_ctx);

    FILE *fp = fopen(argv[2], "wb");
    if (!fp) { printf("无法创建 %s\n", argv[2]); return -1; }

    AVPacket pkt;
    memset(&pkt, 0, sizeof(pkt));
    int count = 0;

    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        if (pkt.stream_index == video_idx) {
            // 5. 送入过滤器
            if (av_bsf_send_packet(bsf_ctx, &pkt) == 0) {
                // 6. 取出转换后的包
                // 一个输入包可能产生多个输出包 (如第一个包会额外输出 SPS/PPS)
                while (av_bsf_receive_packet(bsf_ctx, &pkt) == 0) {
                    fwrite(pkt.data, 1, pkt.size, fp);
                    count++;
                    av_packet_unref(&pkt);
                }
            }
        }
        av_packet_unref(&pkt);
    }

    printf("提取完成: %d 个视频包 -> %s\n", count, argv[2]);
    printf("验证: ffplay %s\n", argv[2]);

    av_bsf_free(&bsf_ctx);
    fclose(fp);
    avformat_close_input(&fmt_ctx);
    return 0;
}
