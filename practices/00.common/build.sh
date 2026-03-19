#!/bin/bash
# Enable error handling options
# set -e
CURRENT_DIR=$(pwd)
INCLUDE_DIR="${CURRENT_DIR}/include"
SOURCE_DIR="${CURRENT_DIR}/source"
SHARE_DIR="${CURRENT_DIR}/share"
LIB_DIR="${CURRENT_DIR}/lib"
BIN_DIR="${CURRENT_DIR}/bin"
VAR_DIR="${CURRENT_DIR}/var"
ETC_DIR="${CURRENT_DIR}/etc"
declare -a dirs=("$SOURCE_DIR" "$INCLUDE_DIR" "$SHARE_DIR" "$LIB_DIR" "$BIN_DIR" "$VAR_DIR" "$ETC_DIR")

# Some package management tool definitions
declare -a package_managers
package_managers[debian]="apt"
package_managers[ubuntu]="apt"
package_managers[centos]="yum"
package_managers[fedora]="dnf"
package_managers[arch]="pacman"
package_managers[opensuse]="zypper"
package_managers[darwin]="brew"
package_managers[win32]="choco"
package_managers[msys]="pacman"

# Initialize the package management command variable
PACKAGE_MANAGER=""

# common basics dependencies
declare -a common_basics=(
    "gcc"
    "gcc-c++"
    "autoconf" 
    "automake" 
    "cmake" 
    "git" 
    "libtool" 
    "pkg-config" 
    "wget" 
    "curl" 
    "nasm" 
    "yasm" 
    "zlib"
)

# global commands
UPDATE_CMD=""
INSTALL_CMD=""
CHECK_CMD=""
BASICS=""
# NUM_CORES=$(nproc)
NUM_CORES=4

# Define any other dependencies to compile
declare -a dependencies=(
    # "nasm"
    # "yasm"
    # "freetype"
    # "fontconfig" 
    # "frei0r-plugins"
    # "mp3lame" #
    # "libass" #
    # "libsoxr" #
    # "libvidstab" #
    # "libvorbis" #
    # "libvpx"
    # "libwebp"
    # "libx264"
    # "libx265"
    # "opencore-amr"
    # "opus"
    # "speex"
    # "libaac"
    # "vo-amrwbenc"
    # "GnuTLS"
    "sdl2"
    "ffmpeg"
)

function get_call_stack() {
    local i=1
    local frame call_stack_str=""

    while read -a frame; do
        call_stack_str="${frame[1]}:${frame[0]}|$call_stack_str"
        ((i++))
    done < <(caller $i)
    echo "${call_stack_str%|}"
}

# Example usage
#   log "This is an info message"
#   log "This is an error message" "ERROR"
#   log "This is an error message" "ERROR" "true"
function log() {
    local message="$1"
    local level="${2:-INFO}"            # Default log level is INFO
    local log_to_console="${3:-true}"  # Default to not log to console
    local log_file="${4:-/var/log/launch.log}"  # Default log file
    local log_entry="[$(date +'%Y-%m-%d %H:%M:%S')][$level][$0] [$(get_call_stack)] $message"

    if [ "$log_to_console" = "true" ]; then
        # Write log entry to console
        echo -e "$log_entry" >&2
    else
        # Ensure log file directory exists
        mkdir -p "$(dirname "$log_file")"
        # Write log entry to log file
        echo -e "$log_entry" >> "$log_file"
    fi
}

# Example usage
#   execute_command "ls -l"
#   execute_command "aws s3 cp a b"
#   execute_command "sudo sh t.sh" "false" "true"
#   result=$(exec_command "ls -l" "true") if need output
function exec_command() {
    local command="$1"                    # The command to execute
    local return_value="${2:-false}"      # Whether to return value (true for return, false for not return), defaule is false
    local exit_on_failure="${3:-true}"    # Whether to exit on command failure (true for exit, false for continue), default is true
    
    if [[ -z "$command" ]]; then
        [ "$return_value" = "false" ] && log "Error: No command provided to exec_command function."
         return 1
    fi

    [ "$return_value" = "false" ] && log "Executing command: $command"
    command_output=$(eval "$command" 2>&1)
    local exit_status=$?

    if (( exit_status == 0 )); then
        [ "$return_value" = "false" ] && log "Command '$command' executed successfully. Output:\n$command_output"
        [[ -n "$command_output" ]] && echo "$command_output"
        return 0
    else
        [ "$return_value" = "false" ] && log "Error: Command '$command' failed with exit status $exit_status. Output:\n$command_output"
        if [[ "$exit_on_failure" == "true" ]]; then
            exit $exit_status
        else
            return $exit_status
        fi
    fi
}

# Function to check and install missing commands
function install_if_not_exists() {
    local cmd=$1
    local install_cmd=$2

    if ! command -v "$cmd" &> /dev/null; then
        log "$cmd not found, attempting to install..."
        if exec_command "$install_cmd"; then
            log "$cmd installed successfully using '$install_cmd'"
        else
            log "Failed to install $cmd using '$install_cmd'"
        fi
    else
        log "$cmd is already installed"
    fi
}

# Associate the os id with the package manager
function detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if [[ -f /etc/os-release ]]; then
            . /etc/os-release
            PACKAGE_MANAGER=${package_managers[$ID]}
        fi
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        PACKAGE_MANAGER="brew"
        install_if_not_exists brew '/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"'
    elif [[ "$OSTYPE" == "msys" ]]; then
        PACKAGE_MANAGER="pacman"
        install_if_not_exists pacman 'echo "Please refer to https://www.msys2.org/ install"'
    elif [[ "$OSTYPE" == "win32" ]]; then
        PACKAGE_MANAGER="choco"
        install_if_not_exists choco 'echo "Please refer to https://chocolatey.org/install and https://docs.chocolatey.org/en-us/choco/setup install"'
    fi
}

# Function to set package manager specific commands
function set_package_manager_commands() {
    case $PACKAGE_MANAGER in
        "apt")
            UPDATE_CMD="sudo apt update"
            INSTALL_CMD="sudo apt install -y"
            CHECK_CMD="dpkg -s"
            BASICS=("${common_basics[@]}" "build-essential" "git-core" "zlib1g-dev" "libass-dev" "libfreetype6-dev" \
            "libfribidi-dev" "libgme-dev" "libgnutls28-dev" "libgmp-dev" "libmp3lame-dev" "libopencore-amrnb-dev" \
            "libopencore-amrwb-dev" "libopus-dev" "librubberband-dev" "libsrt-openssl-dev" "libsoxr-dev" \
            "libspeex-dev" "libtheora-dev" "libvidstab-dev" "libvo-amrwbenc-dev" "libvorbis-dev" \
            "libvpx-dev" "libwebp-dev" "libx264-dev" "libx265-dev" "libxml2-dev" "libdav1d-dev" \
            "libxvidcore-dev" "libzvbi-dev" "libzimg-dev" "frei0r-plugins-dev")
            ;;
        "yum")
            UPDATE_CMD="sudo yum update"
            exec_command "sudo yum groupinstall -y 'Development Tools'"
            INSTALL_CMD="sudo yum install -y"
            CHECK_CMD="rpm -q"
            BASICS=("${common_basics[@]}" "freetype-devel" "mercurial" "zlib-devel" "aom-devel" "libass-devel" \
            "fribidi-devel" "gnutls-devel" "gmp-devel" "lame-devel" "opus-devel" "x264-devel" "x265-devel")
            ;;
        "dnf")
            UPDATE_CMD="sudo dnf update"
            exec_command "sudo dnf groupinstall -y 'Development Tools'"
            INSTALL_CMD="sudo dnf install -y"
            CHECK_CMD="rpm -q"
            BASICS=("${common_basics[@]}" "freetype-devel" "mercurial" "zlib-devel" "aom-devel" "libass-devel" \
            "fribidi-devel" "gnutls-devel" "gmp-devel" "lame-devel" "opus-devel" "x264-devel" "x265-devel")
            ;;
        "pacman")
            UPDATE_CMD="sudo pacman -Syu"
            INSTALL_CMD="sudo pacman -S --noconfirm"
            CHECK_CMD="pacman -Qs"
            BASICS=()
            ;;
        "zypper")
            UPDATE_CMD="sudo zypper refresh"
            exec_command "sudo zypper install -t pattern devel_basis"
            INSTALL_CMD="sudo zypper install -y"
            CHECK_CMD="rpm -q"
            BASICS=()
            ;;
        "brew")
            UPDATE_CMD="brew update"
            INSTALL_CMD="brew install"
            CHECK_CMD="brew list"
            BASICS=("libjpeg" "libpng" "libtiff" "gnutls" "fontconfig" "frei0r" "gmp" "libgme" "aom" "fribidi" \
                    "libass" "libvmaf" "freetype" "lame" "opencore-amr" "openjpeg" "rubberband" "libsoxr" \
                    "speex" "srt" "opus" "theora" "libvidstab" "libvo-aacenc" "libvpx" "webp" "x264" \
                    "x265" "libxml2" "dav1d" "xvid" "zimg")
            ;;
        "choco")
            UPDATE_CMD="choco upgrade chocolatey"
            INSTALL_CMD="choco install -y"
            CHECK_CMD="choco list --local-only"
            BASICS=()
            ;;
        *)
            log "Unsupported package manager: $PACKAGE_MANAGER" "ERROR"
            exit 1
            ;;
    esac
}

function install_package() {
    local package=$1

    if $CHECK_CMD "$package" > /dev/null 2>&1; then
        log "Package $package already installed."
    else
        log "Installing package: $package"
        exec_command "$INSTALL_CMD $package"
    fi
}

# install NASM（Netwide Assembler）是用于x86架构的汇编器。它是开发低级系统软件的常用选择，
# 通常与GNU编译器集合（GCC）结合使用，用于构建x86平台的应用程序。NASM使用类似于英特尔汇编语言的语法，
# 并提供强大的宏功能以实现代码的重用。它广泛用于操作系统、设备驱动程序和其他系统级软件的开发。
function download_and_build_nasm() {
    cd $SOURCE_DIR
    if [ ! -f nasm-2.16.01.tar.gz ]; then
        wget --no-check-certificate https://www.nasm.us/pub/nasm/releasebuilds/2.16.01/nasm-2.16.01.tar.gz
    fi

    if [ ! -d nasm-2.16.01 ]; then
        tar -xf nasm-2.16.01.tar.gz
    fi
    pushd nasm-2.16.01
    ./configure --prefix="$CURRENT_DIR"
    make -j"$NUM_CORES"
    sudo make install
    popd
    # rm -rf nasm-2.16.01*
    cd $CURRENT_DIR
}

# install Yasm 是 NASM（Netwide Assembler）的重写版本，旨在具有更模块化的架构，使得维护和扩展更加容易。
# Yasm 支持 x86 和 AMD64 指令集，以及其他几种体系结构。与 NASM 一样，它经常用于低级系统软件开发，并以其性能和灵活性而闻名。
# Yasm 的语法也类似于 NASM，使得熟悉 NASM 的开发人员相对容易地转换到 Yasm。
function download_and_build_yasm() {
    cd $SOURCE_DIR
    if [ ! -f yasm-1.3.0.tar.gz ]; then
        wget http://www.tortall.net/projects/yasm/releases/yasm-1.3.0.tar.gz
    fi

    if [ ! -d yasm-1.3.0 ]; then
        tar -xf yasm-1.3.0.tar.gz
    fi
    pushd yasm-1.3.0
    ./configure --prefix="$CURRENT_DIR" --enable-shared --disable-static
    make -j"$NUM_CORES"
    sudo make install
    popd
    # rm -rf yasm-1.3.0*
    cd $CURRENT_DIR
}

# install FreeType是一个用于渲染字体的开源软件库，它支持多种字体格式，包括TrueType、OpenType和PostScript。
# FreeType可以用于在各种平台上渲染字体，包括Linux、Windows和Mac OS等操作系统。它提供了对字体文件的解析和渲染功能，
# 允许开发人员在其应用程序中使用高质量的字体渲染。FreeType还支持字体轮廓的平滑缩放和抗锯齿处理，
# 以确保在各种分辨率和尺寸下都能获得清晰的字体显示效果。这使得FreeType成为许多图形应用程序和操作系统中常用的字体渲染引擎。
function download_and_build_freetype() {
    cd $SOURCE_DIR
    if [ ! -f freetype-2.13.2.tar.gz ]; then
        wget --no-check-certificate https://download.savannah.gnu.org/releases/freetype/freetype-2.13.2.tar.gz
    fi

    if [ ! -d freetype-2.13.2 ]; then
        tar -xf freetype-2.13.2.tar.gz
    fi
    pushd freetype-2.13.2
    ./configure --prefix="$CURRENT_DIR" --enable-shared --disable-static
    make -j"$NUM_CORES"
    sudo make install
    popd
    # rm -rf freetype-2.13.2*
    cd $CURRENT_DIR
}

# install Fontconfig是一个用于配置和自定义字体渲染的开源库，它允许用户在Linux和Unix系统上对字体进行管理和设置。
# Fontconfig提供了一个统一的接口，使用户可以轻松地管理系统上的字体，包括字体的搜索、缓存和渲染。它还支持字体配置文件，
# 允许用户对字体进行详细的设置和调整，以满足其特定的字体需求。Fontconfig通常与FreeType字体渲染引擎一起使用，以提供全面的字体管理和渲染功能。
function download_and_build_fontconfig() {
    cd $SOURCE_DIR
    install_package "gperf"
    if [ ! -f fontconfig-2.15.0.tar.gz ]; then
        wget --no-check-certificate https://www.freedesktop.org/software/fontconfig/release/fontconfig-2.15.0.tar.gz
    fi

    if [ ! -d fontconfig-2.15.0 ]; then
        tar -xf fontconfig-2.15.0.tar.gz
    fi
    pushd fontconfig-2.15.0
    ./configure --prefix="$CURRENT_DIR" \
               --enable-shared --disable-static \
               FREETYPE_CFLAGS="-I$INCLUDE_DIR" \
               FREETYPE_LIBS="-L$LIB_DIR -lfreetype" \
               CPPFLAGS="-I$INCLUDE_DIR -I$INCLUDE_DIR/freetype2" \
               LDFLAGS="-L$LIB_DIR"
    make -j"$NUM_CORES"
    sudo make install
    popd
    # rm -rf fontconfig-2.15.0*
    cd $CURRENT_DIR
}

# install frei0r-plugins是一组用于视频编辑和处理的开源插件集合。这些插件可以用于各种视频编辑软件和框架，
# 如Kdenlive、OpenShot、以及GStreamer等。frei0r-plugins 提供了各种效果和过滤器，包括颜色校正、模糊、扭曲、转场效果等，
# 使得用户可以在视频编辑过程中添加各种特效和滤镜。这些插件是基于自由软件和开放源代码的原则开发的，可以在许多不同的平台上使用，
# 为视频编辑和处理提供了更多的可能性和灵活性。
function download_and_build_frei0r-plugins() {
    cd $SOURCE_DIR
    if [ ! -f v2.3.1.tar.gz ]; then
        wget --no-check-certificate https://github.com/dyne/frei0r/archive/refs/tags/v2.3.1.tar.gz
    fi

    if [ ! -d frei0r-2.3.1 ]; then
        tar -xf v2.3.1.tar.gz
    fi
    pushd frei0r-2.3.1
    cmake -DCMAKE_INSTALL_PREFIX="$CURRENT_DIR" .
    make -j"$NUM_CORES"
    sudo make install
    popd
    # rm -rf frei0r-2.3.1 v2.3.1.tar.gz
    cd $CURRENT_DIR
}

# install lame LAME Ain't an MP3 Encoder）是一个开源的音频编码器，主要用于将音频文件编码为MP3格式。
# LAME具有高质量的音频编码能力，能够在保持音频质量的同时实现较小的文件大小。它是一个非常流行的MP3编码器，被广泛用于音乐制作、
# 音频编辑软件和音频转换工具中。LAME还提供了一些高级的音频编码选项，使用户可以根据需要进行定制和优化。由于其出色的性能和开源的特性，
# LAME已成为许多音频应用程序的首选MP3编码器。
function download_and_build_mp3lame() {
    cd $SOURCE_DIR
    if [ ! -f lame-3.100.tar.gz ]; then
        wget --no-check-certificate https://sourceforge.net/projects/lame/files/latest/download/lame-3.100.tar.gz
    fi

    if [ ! -d lame-3.100 ]; then
        tar -xf lame-3.100.tar.gz
    fi
    pushd lame-3.100
    ./configure --prefix="$CURRENT_DIR" --enable-shared --disable-static \
               CFLAGS="-arch arm64"
    make -j"$NUM_CORES"
    sudo make install
    popd
    # rm -rf lame-3.100*
    cd $CURRENT_DIR
}

# install libass 是一个开源的字幕渲染库，主要用于嵌入式字幕在视频播放中的渲染和显示。它支持多种字幕格式，
# 包括SSA/ASS（SubStation Alpha）、SRT（SubRip）、和VobSub等。libass提供了丰富的文本和样式处理功能，
# 可以渲染出高质量的字幕效果，包括各种字体、颜色、描边、阴影、动画效果等。许多视频播放器和转码工具都使用libass来实现字幕的渲染和显示，
# 使得用户可以在视频中添加并调整字幕效果。由于其稳定性和灵活性，libass已成为许多视频应用程序中不可或缺的字幕渲染引擎。
function download_and_build_libass() {
    cd $SOURCE_DIR
    if [ ! -f libass-0.17.1.tar.gz ]; then
        wget --no-check-certificate https://github.com/libass/libass/releases/download/0.17.1/libass-0.17.1.tar.gz
    fi

    if [ ! -d libass-0.17.1 ]; then
        tar -xf libass-0.17.1.tar.gz
    fi
    pushd libass-0.17.1
    ./configure --prefix="$CURRENT_DIR" \
               --enable-shared --disable-static \
               FREETYPE_CFLAGS="-I$INCLUDE_DIR" \
               FREETYPE_LIBS="-L$LIB_DIR -lfreetype" \
               CPPFLAGS="-I$INCLUDE_DIR -I$INCLUDE_DIR/freetype2" \
               LDFLAGS="-L$LIB_DIR"
    make -j"$NUM_CORES"
    sudo make install
    popd
    # rm -rf libass-0.17.1*
    cd $CURRENT_DIR
}

# install libsoxr是一个高质量的音频重采样库，用于在不同采样率和比特深度之间进行音频重采样。它提供了高效的算法和优化，
# 能够快速地对音频进行重采样，并且保持良好的音频质量。libsoxr支持多种重采样模式和滤波器类型，用户可以根据自己的需求选择适合的重采样设置。
# 许多音频应用程序和工具使用libsoxr来实现音频重采样功能，以确保音频在不同采样率下都能保持高质量的声音表现。由于其高性能和良好的音频质量，
# libsoxr已成为许多音频处理流程中重要的组件。
function download_and_build_libsoxr() {
    cd $SOURCE_DIR
    if [ ! -f soxr-0.1.3-Source.tar.xz ]; then
        wget --no-check-certificate https://sourceforge.net/projects/soxr/files/soxr-0.1.3-Source.tar.xz
    fi

    if [ ! -d soxr-0.1.3-Source ]; then
        tar -xf soxr-0.1.3-Source.tar.xz
    fi
    pushd soxr-0.1.3-Source
    cmake -DCMAKE_INSTALL_PREFIX="$CURRENT_DIR" .
    make -j"$NUM_CORES"
    sudo make install
    popd
    # rm -rf soxr-0.1.3-Source*
    cd $CURRENT_DIR
}

# install libvidstab是一个用于视频图像稳定的开源库，可以用于去除视频中的抖动和抖动效果。它提供了一些算法和工具，
# 可以分析视频图像中的运动，然后对视频进行稳定处理，以减少或消除由相机抖动或运动引起的不稳定性。
# libvidstab通常用于视频编辑软件或视频处理工具中，以提供视频稳定性改进的功能。这个库可以帮助用户改善其视频素材的质量，使其更加平稳和专业。
function download_and_build_libvidstab() {
    cd $SOURCE_DIR
    if [ ! -f release-0.98b.tar.gz ]; then
        wget --no-check-certificate https://github.com/georgmartius/vid.stab/archive/refs/tags/release-0.98b.tar.gz
    fi

    if [ ! -d vid.stab-release-0.98b ]; then
        tar -xf release-0.98b.tar.gz
    fi
    pushd vid.stab-release-0.98b
    cmake -DCMAKE_INSTALL_PREFIX="$CURRENT_DIR" .
    make -j"$NUM_CORES"
    sudo make install
    popd
    # rm -rf vid.stab-release-0.98b release-0.98b.tar.gz
    cd $CURRENT_DIR
}

# install libvorbis是一个开源的音频压缩库，用于将音频数据编码为Vorbis格式。Vorbis是一种无损音频编码格式，
# 旨在提供高质量的音频压缩，并且是免费、开放的格式。libvorbis库提供了用于对音频数据进行编码和解码的API，
# 使得开发人员可以在其应用程序中实现Vorbis格式的音频处理和播放功能。这个库是许多音频应用程序和工具中常用的组件，用于实现高质量的音频压缩和解压缩。
function download_and_build_libvorbis() {
    cd $SOURCE_DIR
    cd $SOURCE_DIR
    if [ ! -f vorbis-tools-1.4.2.tar.gz ]; then
        wget --no-check-certificate https://ftp.osuosl.org/pub/xiph/releases/vorbis/vorbis-tools-1.4.2.tar.gz
    fi

    if [ ! -d vorbis-tools-1.4.2 ]; then
        tar -xf vorbis-tools-1.4.2.tar.gz
    fi
    pushd vorbis-tools-1.4.2
    ./configure --prefix="$CURRENT_DIR" --enable-shared --disable-static
    make -j"$NUM_CORES"
    sudo make install
    popd
    # rm -rf vorbis-tools-1.4.2*
    cd $CURRENT_DIR
}

# install libvpx是一个开源的视频编解码库，主要用于实时的、低延迟的视频压缩和解压缩。它是由Google开发的，旨在提供高质量的视频压缩，
# 并且是免费、开放的格式。libvpx库提供了用于对视频数据进行编码和解码的API，使得开发人员可以在其应用程序中实现VP8和VP9格式的视频处理和播放功能。
# 这个库是许多视频应用程序和工具中常用的组件，用于实现高质量的视频压缩和解压缩，同时保持较小的文件大小和良好的视觉质量。
function download_and_build_libvpx() {
    cd $SOURCE_DIR
    if [ ! -f v1.14.0.tar.gz ]; then
        wget --no-check-certificate https://github.com/webmproject/libvpx/archive/refs/tags/v1.14.0.tar.gz
    fi

    if [ ! -d libvpx-1.14.0 ]; then
        tar -xf v1.14.0.tar.gz
    fi
    pushd libvpx-1.14.0
    ./configure --prefix="$CURRENT_DIR" --enable-shared --disable-static --disable-examples --disable-unit-tests \
    --enable-vp9-highbitdepth --as=yasm
    make -j"$NUM_CORES"
    sudo make install
    popd
    # rm -rf v1.14.0.tar.gz libvpx-1.14.0
    cd $CURRENT_DIR
}

# install libwebp是一个开源的图像格式和编解码库，由Google开发。它旨在提供高效的无损和有损图像压缩，同时保持良好的图像质量。
# libwebp库提供了用于对WebP格式图像进行编码和解码的API，使得开发人员可以在其应用程序中实现WebP格式的图像处理和显示功能。
# WebP格式通常用于在Web上显示图像，因为它可以提供更小的文件大小而不损失太多的图像质量。
# libwebp库是许多图像处理应用程序和工具中常用的组件，用于实现高效的图像压缩和解压缩。
function download_and_build_libwebp() {
    cd $SOURCE_DIR
    if [ ! -f libwebp-1.3.2.tar.gz ]; then
        wget --no-check-certificate https://storage.googleapis.com/downloads.webmproject.org/releases/webp/libwebp-1.3.2.tar.gz
    fi

    if [ ! -d libwebp-1.3.2 ]; then
        tar -xf libwebp-1.3.2.tar.gz
    fi
    pushd libwebp-1.3.2
    ./configure --prefix="$CURRENT_DIR" --enable-shared --disable-static
    make -j"$NUM_CORES"
    sudo make install
    popd
    # rm -rf libwebp-1.3.2*
    cd $CURRENT_DIR
}

# install libx264是一个开源的H.264/MPEG-4 AVC视频编码器库，它提供了对H.264视频编解码的实现。它是由VideoLAN团队开发的，
# 旨在提供高效的视频压缩和解压缩，同时保持良好的视频质量。libx264库提供了用于对H.264视频进行编码和解码的API，
# 使得开发人员可以在其应用程序中实现H.264格式的视频处理和播放功能。这个库是许多视频应用程序和工具中常用的组件，
# 用于实现高质量的视频压缩和解压缩，同时保持较小的文件大小和良好的视觉质量。
function download_and_build_libx264() {
    cd $SOURCE_DIR
    if [ ! -f x264-master.tar.gz ]; then
        wget --no-check-certificate https://code.videolan.org/videolan/x264/-/archive/master/x264-master.tar.gz
    fi

    if [ ! -d x264-master ]; then
        tar -xf x264-master.tar.gz
    fi
    pushd x264-master
    ./configure --prefix="$CURRENT_DIR" --enable-shared --disable-static
    make -j"$NUM_CORES"
    sudo make install
    popd
    # rm -rf x264-master*
    cd $CURRENT_DIR
}

# install libx265是一个开源的视频编码库，用于实现HEVC (High Efficiency Video Coding) 或称为H.265 标准的视频编码。
# libx265提供了对H.265视频编码的实现，旨在提供高效的视频压缩和解压缩，同时保持良好的视频质量。这个库是许多视频应用程序和工具中常用的组件，
# 用于实现高质量的视频压缩和解压缩，同时保持较小的文件大小和良好的视觉质量。
function download_and_build_libx265() {
    cd $SOURCE_DIR
    if [ ! -f 3.4.tar.gz ]; then
        wget --no-check-certificate https://github.com/videolan/x265/archive/refs/tags/3.4.tar.gz
    fi

    if [ ! -d x265-3.4 ]; then
        tar -xf 3.4.tar.gz
    fi
    cd x265-3.4/build/linux
    cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX="$CURRENT_DIR" -DENABLE_SHARED:bool=off ../../source
    make -j"$NUM_CORES"
    sudo make install
    cd $SOURCE_DIR
    # rm -rf 3.4.tar.gz x265-3.4
    cd $CURRENT_DIR
}

# opencore-amr-0.1.3.tar.xz opencore-amr 是一个开源的 AMR-NB（Adaptive Multi-Rate Narrowband）和
# AMR-WB（Adaptive Multi-Rate Wideband）音频编解码器库。AMR 是一种音频编解码格式，通常用于语音通信和语音存储应用中。
# opencore-amr 库提供了对 AMR 格式音频进行编码和解码的功能，使得开发人员可以在其应用程序中实现对 AMR 格式音频的处理和播放功能。
# 这个库是许多音频应用程序和工具中常用的组件，用于实现高效的音频编解码，以及在语音通信和存储中使用 AMR 格式的支持。
function download_and_build_opencore-amr() {
    cd $SOURCE_DIR
    if [ ! -f opencore-amr-0.1.6.tar.gz ]; then
        wget --no-check-certificate https://sourceforge.net/projects/opencore-amr/files/opencore-amr/opencore-amr-0.1.6.tar.gz
    fi

    if [ ! -d opencore-amr-0.1.6 ]; then
        tar -xf opencore-amr-0.1.6.tar.gz
    fi
    pushd opencore-amr-0.1.6
    ./configure --prefix="$CURRENT_DIR" --enable-shared --disable-static
    make -j"$NUM_CORES"
    sudo make install
    popd
    # rm -rf opencore-amr-0.1.6*
    cd $CURRENT_DIR
}

# install Opus 是一个开源的音频编解码器，旨在提供高质量的音频压缩和解压缩，同时保持低延迟。Opus 支持广泛的音频应用，
# 包括语音通信、音频流媒体和音频存储。Opus 可以在各种比特率下提供高质量的音频编码，同时具有较低的延迟和高度的容错性。
# Opus 的开源特性使得它成为许多音频应用程序和工具中的首选编解码器，用于实现高效的音频传输和存储。
# Opus 也是 IETF（Internet Engineering Task Force）的一个标准，被广泛用于互联网应用中。
function download_and_build_opus() {
    cd $SOURCE_DIR
    if [ ! -f opus-1.4.tar.gz ]; then
        wget --no-check-certificate https://downloads.xiph.org/releases/opus/opus-1.4.tar.gz
    fi

    if [ ! -d opus-1.4 ]; then
        tar -xf opus-1.4.tar.gz
    fi
    pushd opus-1.4
    ./configure --prefix="$CURRENT_DIR" --enable-shared --disable-static
    make -j"$NUM_CORES"
    sudo make install
    popd
    # rm -rf opus-1.4*
    cd $CURRENT_DIR
}

# install Speex是一个开源的音频编解码器库，旨在提供高效的语音压缩和解压缩，同时保持较低的延迟和高度的容错性。
# Speex通常用于语音通信应用中，例如VoIP（Voice over Internet Protocol）和实时语音聊天。
# Speex的设计重点是在低比特率下提供良好的语音质量，同时尽可能减小数据传输的带宽占用。Speex还包括噪音抑制和回声消除等功能，
# 以提高语音通信的质量。Speex在许多语音应用程序和工具中被广泛使用，用于实现高效的语音编解码和通信。
function download_and_build_speex() {
    cd $SOURCE_DIR
    if [ ! -f speex-1.2.1.tar.gz ]; then
        wget --no-check-certificate wget http://downloads.xiph.org/releases/speex/speex-1.2.1.tar.gz
    fi

    if [ ! -d speex-1.2.1 ]; then
        tar -xf speex-1.2.1.tar.gz
    fi
    pushd speex-1.2.1
    ./configure --prefix="$CURRENT_DIR" --enable-shared --disable-static
    make -j"$NUM_CORES"
    sudo make install
    popd
    # rm -rf speex-1.2.1*
    cd $CURRENT_DIR
}

# vo-aacenc-0.1.3.tar.xz vo-aacenc是VisualOn AAC编码器的一个开源实现。AAC（Advanced Audio Coding）是一种高级音频编码格式，
# 通常用于音频压缩和存储。vo-aacenc用于将音频数据编码为AAC格式，以实现高质量的音频压缩。它是许多音频应用程序和工具中常用的组件，
# 用于实现AAC格式的音频编码。通过vo-aacenc，开发人员可以在其应用程序中实现对音频数据的高效编码，以及在音频存储和传输中使用AAC格式的支持。
function download_and_build_libaac() {
    cd $SOURCE_DIR
    if [ ! -f vo-aacenc-0.1.2.tar.gz ]; then
        wget --no-check-certificate https://sourceforge.net/projects/opencore-amr/files/vo-aacenc/vo-aacenc-0.1.2.tar.gz
    fi

    if [ ! -d vo-aacenc-0.1.2 ]; then
        tar -xf vo-aacenc-0.1.2.tar.gz
    fi
    pushd vo-aacenc-0.1.2
    ./configure --prefix="$CURRENT_DIR" --enable-shared --disable-static
    make -j"$NUM_CORES"
    sudo make install
    popd
    # rm -rf vo-aacenc-0.1.2*
    cd $CURRENT_DIR
}

# vo-amrwbenc-0.1.3.tar.xz vo-amrwbenc是VisualOn AMR-WB编码器的开源实现。
# AMR-WB（Adaptive Multi-Rate Wideband）是一种音频编解码格式，通常用于语音通信和存储应用中。
# vo-amrwbenc用于将音频数据编码为AMR-WB格式，以实现高质量的音频压缩。这个编码器是许多音频应用程序和工具中常用的组件，
# 用于实现对AMR-WB格式音频的编码。开发人员可以在其应用程序中使用vo-amrwbenc来实现对音频数据的高效编码，
# 以及在语音通信和存储中使用AMR-WB格式的支持。
function download_and_build_vo-amrwbenc() {
    cd $SOURCE_DIR
    if [ ! -f vo-amrwbenc-0.1.3.tar.gz ]; then
        wget --no-check-certificate https://sourceforge.net/projects/opencore-amr/files/vo-amrwbenc/vo-amrwbenc-0.1.3.tar.gz
    fi

    if [ ! -d vo-amrwbenc-0.1.3 ]; then
        tar -xf vo-amrwbenc-0.1.3.tar.gz
    fi
    pushd vo-amrwbenc-0.1.3
    ./configure --prefix="$CURRENT_DIR" --enable-shared --disable-static
    make -j"$NUM_CORES"
    sudo make install
    popd
    # rm -rf vo-amrwbenc-0.1.3*
    cd $CURRENT_DIR
}

# GnuTLS 是一个开源的加密库，用于提供安全的传输层协议（TLS）和安全套接字层（SSL）功能。它提供了对加密、身份验证和安全通信的支持，可以用于开发安全的网络应用程序和通信协议。
# GnuTLS 提供了一系列的 API 和工具，使得开发人员能够轻松地集成加密和安全功能到他们的应用程序中。它支持多种加密算法和协议，包括 TLS 1.2/1.3、DTLS、RSA、DSA、ECC、AES、SHA 等。
# GnuTLS 被广泛应用于网络安全领域，包括 Web 服务器、电子邮件服务器、VPN、安全通信协议等。许多 Linux 发行版和开源项目都使用 GnuTLS 作为默认的加密库。
function download_and_build_GnuTLS() {
    cd $SOURCE_DIR
    if [ ! -f gnutls-3.7.10 ]; then
        wget --no-check-certificate https://www.gnupg.org/ftp/gcrypt/gnutls/v3.7/gnutls-3.7.10.tar.xz
    fi

    if [ ! -d gnutls-3.7.10 ]; then
        tar -xf gnutls-3.7.10.tar.xz
    fi
    pushd gnutls-3.7.10
    ./configure --prefix="$CURRENT_DIR" --enable-shared --disable-static --with-included-unistring
    make -j"$NUM_CORES"
    sudo make install
    popd
    # rm -rf gnutls-3.7.10*
    cd $CURRENT_DIR
}

# SDL2（Simple DirectMedia Layer 2）是一个非常流行的跨平台多媒体库，广泛用于游戏开发、应用程序和多媒体项目。
# SDL2 是开源的，你可以从其官方仓库下载并查看源代码。
function download_and_build_sdl2() {
    cd $SOURCE_DIR
    if [ ! -f SDL2-2.30.3.tar.gz ]; then
        wget --no-check-certificate https://github.com/libsdl-org/SDL/releases/download/release-2.30.3/SDL2-2.30.3.tar.gz
    fi

    if [ ! -d SDL2-2.30.3 ]; then
        tar -xf SDL2-2.30.3.tar.gz
    fi
    pushd SDL2-2.30.3
    ./configure --prefix="$CURRENT_DIR" --enable-shared --disable-static
    make -j"$NUM_CORES"
    sudo make install
    popd
    # rm -rf SDL2-2.30.3*
    cd $CURRENT_DIR
}

function download_and_build_ffmpeg() {
    cd $SOURCE_DIR
    if [ ! -f n6.1.1.tar.gz ]; then
        wget --no-check-certificate https://github.com/FFmpeg/FFmpeg/archive/refs/tags/n6.1.1.tar.gz
    fi

    if [ ! -d FFmpeg-n6.1.1 ]; then
        tar -xf n6.1.1.tar.gz
    fi
    pushd FFmpeg-n6.1.1/
    ./configure --prefix="$CURRENT_DIR" \
    --extra-cflags="-I$INCLUDE_DIR -I/opt/homebrew/include" \
    --extra-ldflags="-L$LIB_DIR -L/opt/homebrew/lib -ldl -lm -lpthread" \
    --extra-libs="-lpthread -lm" \
    --bindir="$BIN_DIR" \
    --enable-shared \
    --disable-static \
    --disable-optimizations \
    --enable-cross-compile \
    --enable-shared \
    --enable-nonfree \
    --enable-pthreads \
    --enable-ffplay \
    --enable-debug \
    --enable-gpl \
    --enable-version3 \
    --enable-libgme \
    --enable-gray \
    --enable-libaom \
    --enable-libfribidi \
    --enable-libvmaf \
    --enable-libmp3lame \
    --enable-libopencore-amrnb \
    --enable-libopencore-amrwb \
    --enable-libopenjpeg \
    --enable-librubberband \
    --enable-libsoxr \
    --enable-libspeex \
    --enable-libsrt \
    --enable-libvorbis \
    --enable-libopus \
    --enable-libtheora \
    --enable-libvidstab \
    --enable-libvpx \
    --enable-libwebp \
    --enable-libx264 \
    --enable-libx265 \
    --enable-libxml2 \
    --enable-libdav1d \
    --enable-libxvid \
    --enable-libzimg \
    --enable-libfdk-aac \
    --enable-filter=delogo \
    --enable-hardcoded-tables
    make -j"$NUM_CORES"
    sudo make install
    popd
    # rm -rf n6.1.1.tar.gz FFmpeg-n6.1.1
    cd $CURRENT_DIR
}

function main() {
    # Select build or clean operation based on input argument
    local operation=${1:-build}

    case "$operation" in
        build)
            # Detect current OS package manager
            detect_os

            # Set package manager commands
            set_package_manager_commands

            log "Updating package list..."
            exec_command "$UPDATE_CMD"

            # Call function to install basic tools
            for basic in "${BASICS[@]}"; do
                install_package "$basic"
            done

            # Ensure required directories exist
            for dir in "${dirs[@]}"; do
                [ ! -d "$dir" ] && exec_command "mkdir -p $dir"
            done

            # Iterate through dependencies and build
            for dep in "${dependencies[@]}"; do
                download_and_build_"$dep"
            done
            ;;
        clean)
            for dir in "${dirs[@]}"; do
                [ -d "$dir" ] && exec_command "sudo rm -rf $dir"
            done
            ;;
        *)
            log "Unsupported operation: $operation. Use 'build' or 'clean'." "ERROR"
            exit 1
            ;;
    esac
}

# Main execution entry
main "$@"
