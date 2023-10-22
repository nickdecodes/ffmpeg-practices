cmake_minimum_required(VERSION 3.2)
project(11.decode_video_scale_save_to_bmp)
link_directories(../00.common/lib)
include_directories(../00.common/include)
include_directories(../00.common/StreamMedia)

add_executable(11.decode_video_scale_save_to_bmp main.cpp ../00.common/StreamMedia/StreamMedia.cpp)
target_link_libraries(11.decode_video_scale_save_to_bmp avutil avformat avcodec swscale)
target_link_libraries(11.decode_video_scale_save_to_bmp avdevice)
