cmake_minimum_required(VERSION 3.2)
project(06.time_stamp)
link_directories(../00.common/lib)
include_directories(../00.common/include)
include_directories(../00.common/StreamMedia)

add_executable(06.time_stamp main.cpp ../00.common/StreamMedia/StreamMedia.cpp)
target_link_libraries(06.time_stamp avutil avformat)
target_link_libraries(06.time_stamp avcodec avdevice swscale)
