#!/bin/bash

clang -g -o extra_audio extra_audio.c `pkg-config --libs --cflags libavutil libavformat libavcodec`
