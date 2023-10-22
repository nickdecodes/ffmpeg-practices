## SDL简介

- SDL，全称Simple DriectMedia Layer，SDL是一个开源的跨平台的多媒体库，封装了复杂的音视频底层操作，提供了数种控制图像、声音、输出入的函数，支持跨多个平台开发，多用与开发游戏、模拟器、媒体播放器等多媒体应用领域。


- 各系统封装：
1. windows: 使用Direct3D做显示，DirectSound和XAudio2做声音处理
2. Mac OS X：使用OpenGL做显示，Core Audio 做声音处理
3. Linux：使用X11、OpenGL做显示，ALSA、OSS等做声音处理
4. ios：使用OpenGL ES2.0做显示、JNI audio 做声音处理

- 子系统分类
1. Timer(定时器)
2. Audio(声音)：声音控制
3. Video(图像)：图像控制以及线程和时间管理
4. Joystic(摇杆)：游戏摇杆控制
5. Haptix(触摸屏)：触摸事件响应
6. Gamecontroller(控制器)：控制器子系统
7. Event(事件)：事件驱动

- SDL库分类
1. SDL_iamge：支持时下流行的图像格式，如BMP、PPM、XPM、PCX、GIF、JPEG、PNG、TGA
2. SDL_mixer：更多的声音输出函数以及更多的声音格式支持
3. SDL_net：网络支持
4. SDL_ttf：TrueType字体渲染支持
5. SLD_rtf：简单的RTF渲染支持
6. 官方下载路径：http://www.libsdl.org/projects/

- SDL开发环境
1. 从下载的源码中拷贝SLD2目录的头文件
2. 将编译好的SDL依赖库拷贝到代码目录
3. makefile中增加-lsdl2的链接选项

## SDL显示流程
1. `SDL_Init`
2. `SDL_CreateWindow`
3. `SDL_DestroyWindow`
4. `SDL_Quit`

## SDL事件处理
1. SDL_WaitEvent: SDL等待事件
2. SDL_PollEvent: SDL轮训事件
3. SDL_PumpEvents：从输入设备中搜集事件，将这些事件写入事件对列
4. SDL_PeepEvents: 从事件对列中取出事件，依赖SDL_PumpEvents更新事件对列，不从对列中删除事件
5. SDL_PushEvent: 向对列中插入事件

## SDL图像显示关键结构体
1. SDL_Window: 表示逻辑窗口，创建后并不会被显示出来
2. SDL_Render：表示渲染器，将各种图形渲染到SDL_Surface或SDL_Texture中，并提供接口进行显示
3. SDL_surface：Render对象中的视频缓冲区，按照像素存放图像，如RGB24数据
4. SDL_Texture：表示纹理，存放图像的描述信息， 通过OpenGl、D3D、Metal等技术操作GPU，从而得到SDL_Surface一样的图形，且效率高

## SDL显示图片流程
1. `SDL_Init`
2. `SDL_CreateWindow`
3. `SDL_GetWindowSurface`
4. `SDL_LoadBMP`
5. `SDL_BlitSurface`
6. `SDL_UpdateWindowSurface`
7. `SDL_FreeSurface`
8. `SDL_DestroyWindow`
9. `SDL_Quit`

## SDL显示纹理数据流程
1. `SDL_Init`
2. `SDL_CreateWindow`
3. `SDL_CreateRender`
4. `SDL_CreateTexture`
5. `SDL_UpdateTexture`
6. `SDL_RenderClear`
7. `SDL_RenderCopy`
8. `SDL_RenderPressent`
9. `SDL_DestroyWindow`
10. `SDL_Quit`
