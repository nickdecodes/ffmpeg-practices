cmake_minimum_required(VERSION 3.2)
project(20.sdl_display_by_texture)
link_directories(../00.common/lib)
include_directories(../00.common/include)

add_executable(20.sdl_display_by_texture main.cpp)
target_link_libraries(20.sdl_display_by_texture sdl2)
