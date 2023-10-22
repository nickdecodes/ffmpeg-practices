cmake_minimum_required(VERSION 3.2)
project(05.remux_mp4_to_flv)
link_directories(../00.common/lib)
include_directories(../00.common/include)
include_directories(../00.common/StreamMedia)

add_executable(05.remux_mp4_to_flv main.cpp ../00.common/StreamMedia/StreamMedia.cpp)
target_link_libraries(05.remux_mp4_to_flv avutil avformat)
target_link_libraries(05.remux_mp4_to_flv avcodec avdevice swscale) # option
