# Install script for directory: /Users/zhengdongqi/N.Nick/ffmpeg-practices

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/Library/Developer/CommandLineTools/usr/bin/objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/Users/zhengdongqi/N.Nick/ffmpeg-practices/cmake-build-debug/00.common/cmake_install.cmake")
  include("/Users/zhengdongqi/N.Nick/ffmpeg-practices/cmake-build-debug/01.av_log/cmake_install.cmake")
  include("/Users/zhengdongqi/N.Nick/ffmpeg-practices/cmake-build-debug/02.print_metadata/cmake_install.cmake")
  include("/Users/zhengdongqi/N.Nick/ffmpeg-practices/cmake-build-debug/03.demux_audio_to_aac/cmake_install.cmake")
  include("/Users/zhengdongqi/N.Nick/ffmpeg-practices/cmake-build-debug/04.demux_video_to_h264/cmake_install.cmake")
  include("/Users/zhengdongqi/N.Nick/ffmpeg-practices/cmake-build-debug/05.remux_mp4_to_flv/cmake_install.cmake")
  include("/Users/zhengdongqi/N.Nick/ffmpeg-practices/cmake-build-debug/06.time_stamp/cmake_install.cmake")
  include("/Users/zhengdongqi/N.Nick/ffmpeg-practices/cmake-build-debug/07.cut_stream/cmake_install.cmake")
  include("/Users/zhengdongqi/N.Nick/ffmpeg-practices/cmake-build-debug/08.decode_video_to_yuv/cmake_install.cmake")
  include("/Users/zhengdongqi/N.Nick/ffmpeg-practices/cmake-build-debug/09.decode_video_scale_to_yuv/cmake_install.cmake")
  include("/Users/zhengdongqi/N.Nick/ffmpeg-practices/cmake-build-debug/10.decode_video_scale_to_rgb/cmake_install.cmake")
  include("/Users/zhengdongqi/N.Nick/ffmpeg-practices/cmake-build-debug/11.decode_video_scale_save_to_bmp/cmake_install.cmake")
  include("/Users/zhengdongqi/N.Nick/ffmpeg-practices/cmake-build-debug/12.encode_video_yuv_to_h264/cmake_install.cmake")
  include("/Users/zhengdongqi/N.Nick/ffmpeg-practices/cmake-build-debug/13.decode_audio_aac_to_pcm/cmake_install.cmake")
  include("/Users/zhengdongqi/N.Nick/ffmpeg-practices/cmake-build-debug/14.encode_audio_pcm_to_aac/cmake_install.cmake")
  include("/Users/zhengdongqi/N.Nick/ffmpeg-practices/cmake-build-debug/15.capture_video_to_origin/cmake_install.cmake")
  include("/Users/zhengdongqi/N.Nick/ffmpeg-practices/cmake-build-debug/16.capture_video_to_yuv420/cmake_install.cmake")
  include("/Users/zhengdongqi/N.Nick/ffmpeg-practices/cmake-build-debug/17.capture_audio_to_pcm/cmake_install.cmake")
  include("/Users/zhengdongqi/N.Nick/ffmpeg-practices/cmake-build-debug/18.sdl_display_example/cmake_install.cmake")
  include("/Users/zhengdongqi/N.Nick/ffmpeg-practices/cmake-build-debug/19.sdl_display_bmp/cmake_install.cmake")
  include("/Users/zhengdongqi/N.Nick/ffmpeg-practices/cmake-build-debug/20.sdl_display_by_texture/cmake_install.cmake")
  include("/Users/zhengdongqi/N.Nick/ffmpeg-practices/cmake-build-debug/21.sdl_display_yuv/cmake_install.cmake")
  include("/Users/zhengdongqi/N.Nick/ffmpeg-practices/cmake-build-debug/22.sdl_display_pcm/cmake_install.cmake")
  include("/Users/zhengdongqi/N.Nick/ffmpeg-practices/cmake-build-debug/23.audio_record_and_resample/cmake_install.cmake")
  include("/Users/zhengdongqi/N.Nick/ffmpeg-practices/cmake-build-debug/24.capture_video_to_nv12/cmake_install.cmake")
  include("/Users/zhengdongqi/N.Nick/ffmpeg-practices/cmake-build-debug/25.capture_audio_to_aac_and_encode/cmake_install.cmake")
  include("/Users/zhengdongqi/N.Nick/ffmpeg-practices/cmake-build-debug/26.capture_video_to_nv12_and_encode/cmake_install.cmake")
  include("/Users/zhengdongqi/N.Nick/ffmpeg-practices/cmake-build-debug/27.rtmp_push_stream/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/Users/zhengdongqi/N.Nick/ffmpeg-practices/cmake-build-debug/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
