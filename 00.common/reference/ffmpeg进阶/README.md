# player目录 
player目录中保存的是FFmpeg4.1时的播放器代码，最新的FFmpeg5.0的代码保存在FFmpeg5.0目录下，它们是以`8-`开头的。

# FFmpeg5.0 目录
FFmpeg5.0中保存的是本课程更新到FFmpeg5.0的例子代码。这次课程更新了三章的内容，其中以5开头的是第5章的内容；以6开头的是第6章的内容；8开头的是第8章的内容。

# build-ios-ffmpeg.sh
1. 使用该脚本有一个限制条件，就是该脚本要放在ffmpeg所在的目录，即与ffmpeg目录并列。
2. ffmpeg 源码目录名必须是ffmpeg，不能是ffmpeg...,例如：ffmpeg-4.0.2这样是不行的。
3. 如果你之前编译过ffmpeg，那么在 ffmpeg 目录下就会有config.h这个文件，在编译时会报`Out of tree builds are impossible with config.h in source dir.`错误，解决办法是将ffmpeg目录中用config.h文件删除掉。

