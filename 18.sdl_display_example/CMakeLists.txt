cmake_minimum_required(VERSION 3.2)
project(18.sdl_display_example)
link_directories(../00.common/lib)
include_directories(../00.common/include)

add_executable(18.sdl_display_example main.cpp)
target_link_libraries(18.sdl_display_example sdl2)
