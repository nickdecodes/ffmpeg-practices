## 时间基

- 时间基：是时间刻度，表示每个刻度多少秒，能更精确的度量时间，不同的封装格式下，时间基是不同的。

- ffmpeg内部的时间基

    ```c++
    #define AV_TIME_BASE // 1000000,其实是1微秒
    #define AV_TIME_BASE_Q (AVRational)(1, AV_TIME_BASE) // 其实是1/1000000秒
    
    eg: AVFormatContext中的int64_t duration, 单位为微秒
    ```

- tbr：time base of rate，表示帧率，eg：25tbr表示1/25秒

- tbn：time base of stream，表示视频流的时间基，eg：1k tbn表示1/1000秒

- tbc：time base of codec，表示视频解码的时间基，eg：50tbc表示1/50秒

## 时间戳

- 时间戳：表示占多少个时间刻度，它的单位不是秒，而是时间刻度，转成实际时间时，与对应的时间基才有意义。
- PTS：显示时间戳，在什么时候开始显示这一帧数据，转成时间：PTS * 时间基（视频流时间基）
- DTS：解码时间戳，在什么时候开始解码这一帧数据，转成时间：DTS * 时间基（视频流时间基）

1. ffmpeg内部时间戳与标准时间的转换

    ```c++
    timestamp(ffmpeg内部时间戳)=AV_TIME_BASE * time(秒);
    itme(秒) = AV_TIME_BASE_Q * timestamp(ffmpeg内部时间戳)
    ```

    

2. 根据PTS求出一帧数据在视频中对应的位置

    ```c++
    double ptsTime = packer.pts * av_q2d(inFmtCtx->streams[packet.stream_inddex]->time_base);
    ```

    

3. 输入PTS转成输出PTS

    ```c++
    packet.pts = av_rescale_q(packet.pts, inStream->time_base, outStream->time_base) ==> packet.pts = packet.pts * inStream->time_base / outStream->time_base
    ```

    