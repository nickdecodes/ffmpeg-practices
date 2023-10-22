cmake_minimum_required(VERSION 3.2)
project(14.encode_audio_pcm_to_aac)
link_directories(../00.common/lib)
include_directories(../00.common/include)
include_directories(../00.common/StreamMedia)

add_executable(14.encode_audio_pcm_to_aac main.cpp ../00.common/StreamMedia/StreamMedia.cpp)
target_link_libraries(14.encode_audio_pcm_to_aac avutil avformat avcodec)
target_link_libraries(14.encode_audio_pcm_to_aac avdevice swscale)
