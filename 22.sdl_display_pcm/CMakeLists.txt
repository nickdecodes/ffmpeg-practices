cmake_minimum_required(VERSION 3.2)
project(22.sdl_display_pcm)
link_directories(../00.common/lib)
include_directories(../00.common/include)

add_executable(22.sdl_display_pcm main.cpp)
target_link_libraries(22.sdl_display_pcm sdl2)
