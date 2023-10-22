cmake_minimum_required(VERSION 3.2)
project(27.rtmp_push_stream)
link_directories(../00.common/lib)
include_directories(../00.common/include)
include_directories(../00.common/StreamMedia)

add_executable(27.rtmp_push_stream main.cpp ../00.common/StreamMedia/StreamMedia.cpp)
target_link_libraries(27.rtmp_push_stream avutil avformat avcodec avdevice swscale swresample)
