/**
 * ============================================================================
 * 25. 迷你播放器 (SDL2 显示)
 * ============================================================================
 *
 * 等价命令:
 *   ffplay input.mp4
 *
 * 把前面学的所有东西串成一个可交互的应用:
 *   demux -> decode -> scale(to YUV420P) -> SDL2 纹理渲染
 *
 * SDL2 渲染流程:
 *   SDL_CreateWindow()     创建窗口
 *   SDL_CreateRenderer()   创建渲染器
 *   SDL_CreateTexture()    创建纹理 (YUV420P 格式)
 *   SDL_UpdateYUVTexture() 用解码帧更新纹理
 *   SDL_RenderCopy()       把纹理画到窗口
 *   SDL_RenderPresent()    显示
 *
 * 帧率控制:
 *   用 pts 计算每帧应该显示多长时间, 用 SDL_Delay 等待。
 */

#include <cstdio>
#include <cstring>
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavutil/time.h"
}
#include <SDL2/SDL.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("用法: %s <输入文件>\n", argv[0]);
        return -1;
    }

    AVFormatContext *fmt_ctx = nullptr;
    AVCodecContext *dec_ctx = nullptr;
    struct SwsContext *sws_ctx = nullptr;

    avformat_open_input(&fmt_ctx, argv[1], nullptr, nullptr);
    avformat_find_stream_info(fmt_ctx, nullptr);
    int vi = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (vi < 0) { printf("没有视频流\n"); return -1; }

    dec_ctx = avcodec_alloc_context3(nullptr);
    avcodec_parameters_to_context(dec_ctx, fmt_ctx->streams[vi]->codecpar);
    avcodec_open2(dec_ctx, avcodec_find_decoder(dec_ctx->codec_id), nullptr);

    int w = dec_ctx->width, h = dec_ctx->height;
    AVRational tb = fmt_ctx->streams[vi]->time_base;
    printf("视频: %dx%d, %s\n", w, h, avcodec_get_name(dec_ctx->codec_id));

    // 如果解码输出不是 YUV420P, 需要转换
    sws_ctx = sws_getContext(w, h, dec_ctx->pix_fmt,
                              w, h, AV_PIX_FMT_YUV420P,
                              SWS_BILINEAR, nullptr, nullptr, nullptr);

    // === SDL2 初始化 ===
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("ffmpeg-flow player",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    // YUV420P 纹理
    SDL_Texture *texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, w, h);

    AVFrame *frame = av_frame_alloc();
    AVFrame *yuv_frame = av_frame_alloc();
    yuv_frame->format = AV_PIX_FMT_YUV420P;
    yuv_frame->width = w;
    yuv_frame->height = h;
    av_frame_get_buffer(yuv_frame, 0);

    AVPacket pkt;
    memset(&pkt, 0, sizeof(pkt));
    int quit = 0;
    int64_t start_time = av_gettime();
    int frame_count = 0;

    while (!quit && av_read_frame(fmt_ctx, &pkt) >= 0) {
        // 处理 SDL 事件 (关闭窗口、按 ESC 退出)
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) quit = 1;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
        }

        if (pkt.stream_index == vi) {
            avcodec_send_packet(dec_ctx, &pkt);
            while (avcodec_receive_frame(dec_ctx, frame) == 0 && !quit) {
                // 转换为 YUV420P
                sws_scale(sws_ctx, (const uint8_t *const *)frame->data, frame->linesize,
                          0, h, yuv_frame->data, yuv_frame->linesize);

                // 更新 SDL 纹理
                SDL_UpdateYUVTexture(texture, nullptr,
                    yuv_frame->data[0], yuv_frame->linesize[0],
                    yuv_frame->data[1], yuv_frame->linesize[1],
                    yuv_frame->data[2], yuv_frame->linesize[2]);

                SDL_RenderClear(renderer);
                SDL_RenderCopy(renderer, texture, nullptr, nullptr);
                SDL_RenderPresent(renderer);

                // 帧率控制: 按 pts 等待
                if (frame->pts != AV_NOPTS_VALUE) {
                    double pts_sec = frame->pts * av_q2d(tb);
                    int64_t target_time = start_time + (int64_t)(pts_sec * 1000000);
                    int64_t now = av_gettime();
                    if (target_time > now) {
                        SDL_Delay((uint32_t)((target_time - now) / 1000));
                    }
                }

                frame_count++;
            }
        }
        av_packet_unref(&pkt);
    }

    printf("播放完成: %d 帧\n", frame_count);

    // 清理
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    av_frame_free(&frame);
    av_frame_free(&yuv_frame);
    sws_freeContext(sws_ctx);
    avcodec_free_context(&dec_ctx);
    avformat_close_input(&fmt_ctx);
    return 0;
}
