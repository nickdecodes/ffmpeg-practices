cmake_minimum_required(VERSION 3.2)
project(01.av_log)

# 设置C++标准版本
set(CMAKE_CXX_STANDARD 11)

# 配置包含目录和库目录
include_directories(${CMAKE_SOURCE_DIR}/../00.common/include)
link_directories(${CMAKE_SOURCE_DIR}/../00.common/lib)

# 添加可执行文件
add_executable(01.av_log main.cpp)

# 链接库
target_link_libraries(01.av_log avutil swscale)

# 打印一些信息，方便调试
message(STATUS "Include directories: ${CMAKE_SOURCE_DIR}/../00.common/include")
message(STATUS "Library directories: ${CMAKE_SOURCE_DIR}/../00.common/lib")

