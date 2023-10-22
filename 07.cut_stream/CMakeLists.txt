cmake_minimum_required(VERSION 3.2)
project(07.cut_stream)
link_directories(../00.common/lib)
include_directories(../00.common/include)
include_directories(../00.common/StreamMedia)

add_executable(07.cut_stream main.cpp ../00.common/StreamMedia/StreamMedia.cpp)
target_link_libraries(07.cut_stream avutil avformat)
target_link_libraries(07.cut_stream avcodec avdevice swscale)
