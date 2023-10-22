cmake_minimum_required(VERSION 3.2)
project(13.decode_audio_aac_to_pcm)
link_directories(../00.common/lib)
include_directories(../00.common/include)
include_directories(../00.common/StreamMedia)

add_executable(13.decode_audio_aac_to_pcm main.cpp ../00.common/StreamMedia/StreamMedia.cpp)
target_link_libraries(13.decode_audio_aac_to_pcm avutil avformat avcodec)
target_link_libraries(13.decode_audio_aac_to_pcm avdevice swscale)
