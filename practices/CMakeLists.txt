cmake_minimum_required(VERSION 3.2)
project(ffmpeg_practices)

set(CMAKE_CXX_STANDARD 11)

# ============================================================================
# 查找 FFmpeg 库
#
# 优先使用 00.common 中编译好的库 (通过 00.common/build.sh 编译)
# 也支持 pkg-config 自动查找系统安装的 FFmpeg
# 也支持手动指定: cmake .. -DFFMPEG_PREFIX=/path/to/ffmpeg
# ============================================================================

set(FFMPEG_FOUND FALSE)

if(DEFINED FFMPEG_PREFIX)
    # 方式 1: 手动指定路径
    message(STATUS "使用手动指定的 FFmpeg: ${FFMPEG_PREFIX}")
    include_directories(${FFMPEG_PREFIX}/include)
    link_directories(${FFMPEG_PREFIX}/lib)
    set(FFMPEG_FOUND TRUE)

elseif(EXISTS "${CMAKE_SOURCE_DIR}/00.common/lib" AND EXISTS "${CMAKE_SOURCE_DIR}/00.common/include")
    # 方式 2: 使用 00.common 中编译好的库 (推荐)
    message(STATUS "使用 00.common 中的 FFmpeg 库")
    include_directories(${CMAKE_SOURCE_DIR}/00.common/include)
    link_directories(${CMAKE_SOURCE_DIR}/00.common/lib)
    set(FFMPEG_FOUND TRUE)

else()
    # 方式 3: pkg-config 自动查找系统安装的
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(AVFORMAT libavformat)
        pkg_check_modules(AVCODEC libavcodec)
        pkg_check_modules(AVUTIL libavutil)
        pkg_check_modules(SWSCALE libswscale)
        pkg_check_modules(SWRESAMPLE libswresample)
        pkg_check_modules(AVFILTER libavfilter)

        if(AVFORMAT_FOUND AND AVCODEC_FOUND AND AVUTIL_FOUND)
            include_directories(${AVFORMAT_INCLUDE_DIRS} ${AVCODEC_INCLUDE_DIRS}
                                ${AVUTIL_INCLUDE_DIRS} ${SWSCALE_INCLUDE_DIRS}
                                ${SWRESAMPLE_INCLUDE_DIRS} ${AVFILTER_INCLUDE_DIRS})
            link_directories(${AVFORMAT_LIBRARY_DIRS} ${AVCODEC_LIBRARY_DIRS}
                             ${AVUTIL_LIBRARY_DIRS} ${SWSCALE_LIBRARY_DIRS}
                             ${SWRESAMPLE_LIBRARY_DIRS} ${AVFILTER_LIBRARY_DIRS})
            set(FFMPEG_FOUND TRUE)
            message(STATUS "使用系统安装的 FFmpeg (pkg-config)")
        endif()
    endif()
endif()

if(NOT FFMPEG_FOUND)
    message(FATAL_ERROR
        "找不到 FFmpeg 库!\n"
        "请先运行: cd 00.common && bash build.sh\n"
        "或安装系统包: brew install ffmpeg (macOS) / apt install libavformat-dev (Ubuntu)")
endif()

# ============================================================================
# 子模块
# ============================================================================
add_subdirectory(01.open)
add_subdirectory(02.streams)
add_subdirectory(03.packets)
add_subdirectory(04.metadata)
add_subdirectory(05.demux_audio)
add_subdirectory(06.demux_video_annexb)
add_subdirectory(07.remux)
add_subdirectory(08.cut)
add_subdirectory(09.decode_video_frame)
add_subdirectory(10.decode_video_yuv)
add_subdirectory(11.decode_audio_pcm)
add_subdirectory(12.timestamp_deep)
add_subdirectory(13.scale)
add_subdirectory(14.encode_video)
add_subdirectory(15.encode_audio)
add_subdirectory(16.resample_audio)
add_subdirectory(17.transcode_video)
add_subdirectory(18.transcode_full)
add_subdirectory(19.encode_params)
add_subdirectory(20.pixel_convert)
add_subdirectory(21.filter_graph)
add_subdirectory(22.custom_io)
add_subdirectory(23.demux_video_raw)
add_subdirectory(24.multi_output)
add_subdirectory(25.mini_player)
add_subdirectory(26.screen_capture)
add_subdirectory(27.custom_filter)
add_subdirectory(28.hw_decode)
