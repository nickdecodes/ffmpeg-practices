cmake_minimum_required(VERSION 3.2)
project(02.print_metadata)
link_directories(../00.common/lib)
include_directories(../00.common/include)
include_directories(../00.common/StreamMedia)

add_executable(02.print_metadata main.cpp ../00.common/StreamMedia/StreamMedia.cpp)
target_link_libraries(02.print_metadata avutil avformat)
target_link_libraries(02.print_metadata swscale avdevice avcodec) # option
