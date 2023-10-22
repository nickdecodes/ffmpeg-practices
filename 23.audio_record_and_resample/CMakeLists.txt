cmake_minimum_required(VERSION 3.2)
project(23.audio_record_and_resample)
link_directories(../00.common/lib)
include_directories(../00.common/include)
include_directories(../00.common/StreamMedia)

add_executable(23.audio_record_and_resample main.cpp ../00.common/StreamMedia/StreamMedia.cpp)
target_link_libraries(23.audio_record_and_resample avutil avformat avcodec avdevice swscale swresample)
