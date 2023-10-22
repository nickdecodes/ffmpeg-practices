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
        goto end;
    }

    while (1) {
        SDL_Event event;
        // SDL_WaitEvent(&event);
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) {
            break;
        }
        SDL_Log("event.type: %d\n", event.type);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect rect;
        rect.x = rand() % 500;
        rect.y = rand() % 400;
        rect.w = 40;
        rect.h = 40;
        SDL_RenderDrawRect(renderer, &rect);
        SDL_RenderFillRect(renderer, &rect);
        SDL_RenderPresent(renderer);
    }
end:
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

