#!/bin/bash

clang -g -o decode_video decode_video.c `pkg-config --libs --cflags libavutil libavformat libavcodec libswscale`
