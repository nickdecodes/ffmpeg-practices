cmake_minimum_required(VERSION 3.2)
project(10.decode_video_scale_to_rgb)
link_directories(../00.common/lib)
include_directories(../00.common/include)
include_directories(../00.common/StreamMedia)

add_executable(10.decode_video_scale_to_rgb main.cpp ../00.common/StreamMedia/StreamMedia.cpp)
target_link_libraries(10.decode_video_scale_to_rgb avutil avformat avcodec swscale)
target_link_libraries(10.decode_video_scale_to_rgb avdevice)
