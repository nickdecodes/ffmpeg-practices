cmake_minimum_required(VERSION 3.2)
project(26.capture_video_to_nv12_and_encode)
link_directories(../00.common/lib)
include_directories(../00.common/include)
include_directories(../00.common/StreamMedia)

add_executable(26.capture_video_to_nv12_and_encode main.cpp ../00.common/StreamMedia/StreamMedia.cpp)
target_link_libraries(26.capture_video_to_nv12_and_encode avutil avformat avcodec avdevice swscale swresample)
