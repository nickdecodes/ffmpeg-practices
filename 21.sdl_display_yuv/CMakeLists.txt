cmake_minimum_required(VERSION 3.2)
project(21.sdl_display_yuv)
link_directories(../00.common/lib)
include_directories(../00.common/include)

add_executable(21.sdl_display_yuv main.cpp)
target_link_libraries(21.sdl_display_yuv sdl2)
