#!/bin/bash

clang -g -o encode_audio encode_audio.c `pkg-config --libs --cflags libavutil libavformat libavcodec`
