#!/bin/bash

clang -g -o player simpleplayer.c `pkg-config --libs --cflags libavutil libavformat libavcodec sdl2`
