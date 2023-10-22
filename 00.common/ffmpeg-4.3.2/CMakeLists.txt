cmake_minimum_required(VERSION 3.2)
project(ffmpeg-4.3.2)

FILE(GLOB SRC_FILE "./libavformat/*.h" "./libavformat/*.c"
        "./libavdevice/*.h" "./libavdevice/*.c"
        "./libavcodec/*.h" "./libavcodec/*.c"
        "./libavutil/*.h" "./libavutil/*.c"
        "./libavfilter/*.h" "./libavfilter/*.c"
        "./libavresample/*.h" "./libswresample/*.c"
        "./libswscale/*.h" "./libswscale/*.c"
        "libswresample/*.h" "./libswresample/*.h")

add_executable(ffmpeg-4.3.2 ${SRC_FILE})
