#!/bin/bash

clang -g -o encode_video encode_video.c `pkg-config --libs --cflags libavutil libavformat libavcodec`
