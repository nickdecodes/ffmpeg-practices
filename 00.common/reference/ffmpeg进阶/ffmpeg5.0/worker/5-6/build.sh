#!/bin/bash

clang -g -o cut cut.c `pkg-config --libs --cflags libavutil libavformat libavcodec`
