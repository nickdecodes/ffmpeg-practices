#!/bin/bash

clang -g -o genpic gen_pic.c `pkg-config --libs --cflags libavutil libavformat libavcodec libswscale`
