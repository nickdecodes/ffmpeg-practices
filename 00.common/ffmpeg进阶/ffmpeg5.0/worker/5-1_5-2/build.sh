#!/bin/bash

clang -g -o test test.c `pkg-config --libs --cflags libavutil libavformat libavcodec`
