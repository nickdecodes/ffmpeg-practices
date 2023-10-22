cmake_minimum_required(VERSION 3.2)
project(19.sdl_display_bmp)
link_directories(../00.common/lib)
include_directories(../00.common/include)

add_executable(19.sdl_display_bmp main.cpp)
target_link_libraries(19.sdl_display_bmp sdl2)
