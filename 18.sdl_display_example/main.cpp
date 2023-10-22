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
    while (1) {
        SDL_Event event;
        SDL_WaitEvent(&event);
        // SDL_PollEvent(&event);
        // SDL_PumpEvents();
        // ret = SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
        // if (ret <= 0) {
        //     continue;
        // }
        if (event.type == SDL_QUIT) {
            break;
        }
        SDL_Log("event.type: %d\n", event.type);
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

