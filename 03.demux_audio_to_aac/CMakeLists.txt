cmake_minimum_required(VERSION 3.2)
project(03.demux_audio_to_aac)
link_directories(../00.common/lib)
include_directories(../00.common/include)
include_directories(../00.common/StreamMedia)

add_executable(03.demux_audio_to_aac main.cpp ../00.common/StreamMedia/StreamMedia.cpp)
target_link_libraries(03.demux_audio_to_aac avutil avformat)
target_link_libraries(03.demux_audio_to_aac avcodec avdevice swscale) # option
