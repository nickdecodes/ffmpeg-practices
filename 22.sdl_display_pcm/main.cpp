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


uint8_t *g_audio_data = nullptr;
int g_audio_data_size = 0;
void fill_audio_data(void *userdata, Uint8 *stream, int len) {
    if (g_audio_data_size == 0) {
        return ;
    }
    SDL_memset(stream, 0, len);
    len = (len > g_audio_data_size ? g_audio_data_size : len);
    SDL_MixAudio(stream, g_audio_data, len, SDL_MIX_MAXVOLUME);
    g_audio_data += len;
    g_audio_data_size -= len;
}

int main(int argc, char **argv) {
    int ret = SDL_Init(SDL_INIT_VIDEO);
    if (ret != 0) {
        SDL_Log("init sdl video subsystem failed: %s\n", SDL_GetError());
        return ret;
    }

    SDL_AudioSpec desired;
    SDL_zero(desired);
    desired.callback = fill_audio_data;
    desired.channels = 1;
    desired.format = AUDIO_F32SYS;
    desired.freq = 48000;
    desired.samples = 1024;
    desired.silence = 0;
    SDL_OpenAudio(&desired, nullptr);

    SDL_PauseAudio(0);

    FILE *infile_fp = fopen("./in.pcm", "rb");
    if (infile_fp == nullptr) {
        return -1;
    }

    uint8_t buffer[4096] = {0};
    while (true) {
        int read_size = fread(buffer, 1, sizeof(buffer), infile_fp);
        if (read_size != sizeof(buffer)) {
            fseek(infile_fp, 0, SEEK_SET);
            read_size = fread(buffer, 1, sizeof(buffer), infile_fp);
        }
        g_audio_data = buffer;
        g_audio_data_size = read_size;
        while (g_audio_data_size > 0) {
            SDL_Delay(1);
        }
    }

    SDL_CloseAudio();
    SDL_Quit();
    return 0;
}

