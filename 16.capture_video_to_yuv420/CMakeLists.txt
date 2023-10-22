cmake_minimum_required(VERSION 3.2)
project(16.capture_video_to_yuv420)
link_directories(../00.common/lib)
include_directories(../00.common/include)
include_directories(../00.common/StreamMedia)

add_executable(16.capture_video_to_yuv420 main.cpp ../00.common/StreamMedia/StreamMedia.cpp)
target_link_libraries(16.capture_video_to_yuv420 avutil avformat avcodec avdevice swscale)
