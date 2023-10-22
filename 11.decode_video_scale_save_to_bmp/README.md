## 更改视频格式流程

1. 分析视频像素大小`av_parse_video_size`
2. 获取上下文`sws_getContext`
3. 保存转换后的数据`av_frame_alloc`
4. 初始化buffer`av_image_get_buffer_size`
5. 申请空间`av_malloc`
6. 按照av_frame的内存空间进行分配`av_image_fill_arrays`
7. 进行处理`sws_scale`

## BMP文件格式

- 概念：BMP文件格式，又称Bitmap（位图）或是DIB（Device-Independent Device,设备无关位图），是Windows系统中国呢广泛使用的图像文件格式。由于它可以不做任何变换低保存图像或图像像素域的数据，因此成为我们取得RAW的数据的重要来源。

- 扫描方式：从左到右、从上到下

- 文件组成：
  1. 位图文件头（bmp file header）：提供文件的格式、大小等信息。
  2. 位图信息头（bitmap information）：提供图像数据的尺寸、位平面数、压缩方式、颜色索引等信息。
  3. 调色板（color palette）：可选，如使用索引来表示图像，调色板就是索引与其对应的颜色的映射表。
  4. 位图数据（bitmap data）：图像数据区
  
- 文件头结构体

    ```c++
    typedef struct tagBITMAPFILEHEADER {
        WORD btType; // 固定为0x4d42， 即'B'， 'M'
        DWORD bfSize; // 整个bmp文件大小
        WORD bfReserved1; // 保留字
        WORD bfReserved2; // 保留字
        DWORD bfOffBits; // 实际位图数据的偏移字节数
    } BITMAPFILEHEADER;
    ```

    

