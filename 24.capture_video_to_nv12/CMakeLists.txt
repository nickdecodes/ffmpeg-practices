cmake_minimum_required(VERSION 3.2)
project(24.capture_video_to_nv12)
link_directories(../00.common/lib)
include_directories(../00.common/include)
include_directories(../00.common/StreamMedia)

add_executable(24.capture_video_to_nv12 main.cpp ../00.common/StreamMedia/StreamMedia.cpp)
target_link_libraries(24.capture_video_to_nv12 avutil avformat avcodec avdevice swscale swresample)
