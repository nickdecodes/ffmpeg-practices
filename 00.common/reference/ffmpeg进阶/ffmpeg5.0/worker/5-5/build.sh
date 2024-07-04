#!/bin/bash

clang -g -o remux remux.c `pkg-config --libs --cflags libavutil libavformat libavcodec`
