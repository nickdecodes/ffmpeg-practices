#!/bin/bash

clang -g -o extra_video extra_video.c `pkg-config --libs --cflags libavutil libavformat libavcodec`
