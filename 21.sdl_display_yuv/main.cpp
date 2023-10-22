/**
@Author  : zhengdongqi
@Email   : 
@Usage   :
@Filename: main.cpp
@DateTime: 2022/11/20 11:59
@Software: CLion
**/

//
// Created by 郑东琦 on 2022/11/20.
//

#include <iostream>
#include <cstdlib>
extern "C" {
#include "SDL2/SDL.h"
}
#define REFRESHVIDEO (SDL_USEREVENT + 1)

int video_exit = 0;
int window_width= 0, window_height = 0;
int refresh_video(void *data) {
    while (video_exit == 0) {
        SDL_Event event;
        event.type = REFRESHVIDEO;
        SDL_PushEvent(&event);
        SDL_Delay(40);
    }
    return 0;
}


int main(int argc, char **argv) {
    int ret = SDL_Init(SDL_INIT_VIDEO);
    if (ret != 0) {
        SDL_Log("init sdl video subsystem failed: %s\n", SDL_GetError());
        return ret;
    }

    SDL_Window *window = SDL_CreateWindow("test_window", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        SDL_Log("create window failed: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        SDL_Log("create renderer failed: %s\n", SDL_GetError());
        // goto end;
        return -1;
    }
    int yuv_width = 640, yuv_height = 480;
    SDL_Texture * texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STATIC, yuv_width, yuv_height);
    if (texture == nullptr) {
        //
        return -1;
    }

    FILE *infile_fp = fopen("./in.yuv", "rb");
    if (infile_fp == nullptr) {
        return -1;
    }

    SDL_Thread *video_thread = SDL_CreateThread(&refresh_video, "refresh_video", nullptr);
    uint8_t buffer[yuv_width * yuv_height * 3 / 2];
    SDL_memset(buffer, 0, sizeof(buffer));

    while (1) {
        SDL_Event event;
        SDL_WaitEvent(&event);
        // SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) {
            video_exit = 1;
            break;
        } else if (event.type == REFRESHVIDEO) {
            int read_size = fread(buffer, 1, sizeof(buffer), infile_fp);
            if (read_size != sizeof(buffer)) {
                fseek(infile_fp, 0, SEEK_SET);
                fread(buffer, 1, sizeof(buffer), infile_fp);
            }
            SDL_UpdateTexture(texture, nullptr, buffer, yuv_width);
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, nullptr, nullptr);
            SDL_RenderPresent(renderer);
        } else if (event.type == SDL_WINDOWEVENT) {
            SDL_GetWindowSize(window, &window_width, &window_height);
            // 窗口大小变化
        }
        // SDL_Log("event.type: %d\n", event.type);
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

