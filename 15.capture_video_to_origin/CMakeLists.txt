cmake_minimum_required(VERSION 3.2)
project(15.capture_video_to_origin)
link_directories(../00.common/lib)
include_directories(../00.common/include)
include_directories(../00.common/StreamMedia)

add_executable(15.capture_video_to_origin main.cpp ../00.common/StreamMedia/StreamMedia.cpp)
target_link_libraries(15.capture_video_to_origin avutil avformat avcodec avdevice)
target_link_libraries(15.capture_video_to_origin swscale) # option
