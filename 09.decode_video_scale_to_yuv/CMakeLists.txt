cmake_minimum_required(VERSION 3.2)
project(09.decode_video_scale_to_yuv)
link_directories(../00.common/lib)
include_directories(../00.common/include)
include_directories(../00.common/StreamMedia)

add_executable(09.decode_video_scale_to_yuv main.cpp ../00.common/StreamMedia/StreamMedia.cpp)
target_link_libraries(09.decode_video_scale_to_yuv avutil avformat avcodec swscale)
target_link_libraries(09.decode_video_scale_to_yuv avdevice)
