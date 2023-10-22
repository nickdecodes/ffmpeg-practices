# FFmpeg开发环境搭建

## Linux环境

### 1. 前提准备

安装有ubuntu或者centos的主机或设备

### 2. 编译安装

```bash
# 创建安装目录
sudo mkdir -p /usr/local/ffmpeg/lib
# 下载源码
http://ffmpeg.org/download.html
# 解压源码
tar -jxf ffmpeg-4.3.2.tar.bz2
# 配置ffmepg
cd ffmpeg-4.3.2/
./configure --prefix="/usr/local/ffmpeg/" --enable-gpl --enable-nonfree --enable-ffplay --enable-libfdk-aac --enable-libmp3lame --enable-libx264 --enable-libx265 --enable-filter=delogo --enable-debug --disable-optimizations --enable-libspeex --enable-shared --enable-pthreads --enable-version3 --enable-hardcoded-tables --extra-ldflags=-L/usr/local/ffmpeg/lib
# 如果有错误提示，解决方式看下面
make -j8
sudo make install
安装完成后发现/usr/localffmpeg/bin目录下， 没有ffplay等应用程序，需要安装SDL
# 官网下载源码
http://www.libsdl.org/download-2.0.php, 编译安装
./configure --prefix=/usr/local/ffmpeg/ --enable-shared
make -j8
sudo make install
再重新运行ffmpeg配置命令，重新安装ffmpeg，会发现/usr/local/ffmpeg/bin/目录下已经有ffmpeg、ffplay、ffprobe等应用程序
但是在使用时发现命令找不到，可能是没有添加paath路径
# 1.修改系统环境变量
sudo vim /etc/profile 文件最后加上
export PATH="/usr/local/ffpeg/bin:$PATH"
# 2.sudo vim /etc/ld.so.conf 文件后加上
/usr/local/ffmpeg/lib
再执行 sudo ldconfig
```

错误提示1: nasm/yasm not found

```bash
# 包管理工具下载
在ubuntu下直接使用 sudo apt-get install yasm即可
# 手动下载安装包
官网下载源码http://www.tortall.net/projects/yasm/releases, 然后编译安装
tar zxvf yasm-1.3.0.tar.gz
cd yasm-1.3.0
./configure
make && make install
# 安装完yasm后， 再运行ffmpeg配置命令，
```

错误提示2: ERROR: libfdk_aac not found

```bash
# 官方下载fdk-aac源码 
https://jaist.dl.sourceforge.net/project/opencore-amr/fdk-aac/, 然后编译安装
./configure --prefix=/usr/local/ffmpeg/ --enable-shared
make -j8
sudo make install
修改PKG_CPNFIG_PATH=$PKG_CONFIG_PATH:/usr/local/ffmpeg/lib/pkgconfig/
# 运行ffmpeg配置命令
```

错误提示3: libmp3lame >= 3.98.3 not found

```bash
# 官网下载mp3lame源码 
https://sourceforge.net/projects/lame/, 然后编译安装
tar zxf lame-3.100.tar.gz
cd lame-3.100/
./configure --prefix=/usr/localffmpeg --enable-shared
make -j8
sudo make install
# 运行ffmpeg配置命令
```

错误提示4: ERROR speex not found using pkg-config

```bash
# 官网下载speex源码
https://www.speex.org/downloads/, 然后编译安装
./configure --prefix=/usr/local/ffmpeg
make -j8
sudo make install 
# 运行ffmpeg配置命令
```

错误提示5: ERROR libx264 not found

```bash
# 官网下载libx264源码
wget https://code.videolan.org/videolan/x264/-/archive/master/x264-master.tar.bz2, 然后编译安装
tar jxf x264-master.tar.bz2
./configure --prefix=/usr/local/ffmpeg/ --enable-shared --disable-asm
make -j8
sudo make install
# 运行ffmpeg配置命令
```

错误提示6: ERROR x265 not found using pkg-config

```bash
# 官网下载libx265源码
http://ftp.videolan.org/pub/videolan/x265/, 然后编译安装
cd x265_3.2/build/linux
./make_Makefiles.bash
# 又提示错误：./make_Makefiles.bash: line3:camke:command not found
解决方法， 安装cmake，在Ubuntu系统下使用命令 sudo apt-get install camke
在其它环境下，可使用源码进行安装，安装后运行
cd x265_3.2/build/linux
./make_Makefiles.bash
make -j8
sudo make install
即可完成x265的安装
# 运行ffmpeg配置命令
```

错误提示7: ffplay 不能使用

```bash
# 1.安装依赖包
sudo apt-get install libasound2-dev
sudo apt-get install libpulse-dev
sudo apt-get install libx11-dev
sudo apt-get install xorg-dev
# 2.重新编译安装SDL
./config --prefix=/usr/local/ffmpeg/ --enable-shared --enable-video-x11 --enable-x11-shared --enable-video-x11-vm
make j8
sudo make install
# 3.重新配置、编译、安装ffmpeg, 再运行ffplay命令，即可正常运行
```

## mac环境

### 1. 前提准备

- 下载源码 [ffmpeg-4.3.2.tar.xz](https://links.jianshu.com/go?to=https%3A%2F%2Fffmpeg.org%2Freleases%2Fffmpeg-4.3.2.tar.xz)

> 源码地址: [https://github.com/FFmpeg/FFmpeg](https://links.jianshu.com/go?to=https%3A%2F%2Fgithub.com%2FFFmpeg%2FFFmpeg)

### 2. 编译安装

依赖项

- **`brew install yasm`**
    - ffmpeg的编译过程依赖yasm
    - 若未安装yasm会出现错误：nasm/yasm not found or too old. Use --disable-x86asm for a crippled build.
- **`brew install sdl2`**
    - ffplay 依赖于 sdl2
    - 如果缺少sdl2，就无法编译出ffplay
- **`brew install fdk-aac`**
    - 不然会出现错误：ERROR: libfdk_aac not found
- **`brew install x264`**
    - 不然会出现错误：ERROR: libx264 not found
    - [x264 地址](https://links.jianshu.com/go?to=https%3A%2F%2Fdownload.videolan.org%2Fpub%2Fvideolan%2Fx264%2Fbinaries%2Fmacosx-x86-64%2F)
- **`brew install x265`**
    - 不然会出现错误：ERROR: libx265 not found

其实 `x264`、`x265`、`sdl2` 都在曾经执行 `brew install ffmpeg` 的时候安装过了

- **可以通过 `brew list` 的结果查看是否安装过**
    - `brew list | grep fdk`
    - `brew list | grep x26`
    - `brew list | grep -E 'fdk|x26'`
- **如果已经安装过，可以不用再执行 `brew install`**

```bash
./configure --prefix="/usr/local/ffmpeg/" --enable-gpl --enable-nonfree --enable-ffplay --enable-libfdk-aac --enable-libmp3lame --enable-libx264 --enable-libx265 --enable-filter=delogo --enable-debug --disable-optimizations --enable-libspeex --enable-shared --enable-pthreads --enable-version3 --enable-hardcoded-tables --extra-ldflags=-L/usr/local/ffmpeg/lib
```

- **`--prefix`**
    - 用以指定编译好的FFmpeg安装到哪个目录
    - 一般放到 `/usr/local/ffmpeg` 中即可
- **`--enable-shared`**
    - 生成动态库
- **`--disable-static`**
    - 不生成静态库
- **`--enable-libfdk-aac`**
    - 将fdk-aac内置到FFmpeg中
- **`--enable-libx264`**
    - 将x264内置到FFmpeg中
- **`--enable-libx265`**
    - 将 x265 内置到 FFmpeg 中
- **`--enable-gpl`**
    - x264、x265 要求开启 [GPL License](https://links.jianshu.com/go?to=https%3A%2F%2Fwww.gnu.org%2Flicenses%2Fgpl-3.0.html)
- **`--enable-nonfree`**
    - - [fdk-aac与GPL不兼容](https://links.jianshu.com/go?to=https%3A%2F%2Fgithub.com%2FFFmpeg%2FFFmpeg%2Fblob%2Fmaster%2FLICENSE.md)，需要通过开启nonfree进行配置

------

你可以通过 **`configure --help`** 命令查看每一个配置项的作用。

```bash
make -j8 # 接下来开始解析源代码目录中的 Makefile 文件，进行编译。-j8 表示允许同时执行8个编译任务。
make install # 之后就是配置PATH
```
