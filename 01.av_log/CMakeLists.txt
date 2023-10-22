cmake_minimum_required(VERSION 3.2)
project(01.av_log)
link_directories(../00.common/lib)
include_directories(../00.common/include)

add_executable(01.av_log main.cpp)
target_link_libraries(01.av_log avutil)
target_link_libraries(01.av_log swscale) # option
