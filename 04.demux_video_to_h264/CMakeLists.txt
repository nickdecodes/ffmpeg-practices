cmake_minimum_required(VERSION 3.2)
project(04.demux_video_to_h264)
link_directories(../00.common/lib)
include_directories(../00.common/include)
include_directories(../00.common/StreamMedia)

add_executable(04.demux_video_to_h264 main.cpp ../00.common/StreamMedia/StreamMedia.cpp)
target_link_libraries(04.demux_video_to_h264 avutil avformat avcodec)
target_link_libraries(04.demux_video_to_h264 avdevice swscale) # option
