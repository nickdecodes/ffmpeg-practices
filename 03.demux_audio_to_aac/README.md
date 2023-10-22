## 提取aac数据流程

1. 打开媒体文件 `avformat_open_input`
2. 获取码流信息`avformat_find_stream_info`
3. 获取音频流`av_find_best_stream`
4. 初始化packet `av_new_packet`
5. 读取packet数据`av_read_frame`
6. 释放packet资源`av_packet_unref`
7. 关闭媒体文件`avformat_close_input`

## ffmpeg命令
```bash
ffmpeg -i input.mp4 -c:a copy -vn -sn -y output.aac
ffplay output.aac
```

## aac音频格式分析

- ADIF：（audio data interchange format）音频数据交换格式。这种格式的特征是可以确定的找到这个音频数据的开始，不需进行在音频数据中间开始的解码，即它的解码必须在明确定义的开始处进行。故这种格式常用在磁盘文件中
- ADTS：（audio data transport stream）音频数据传输流。这种数据格式的特征是它是一个有同步字的比特流，解码可以在这个流中任何位置开始。它的特征类似于mp3的数据流格式。
- 总而言之，ADTS可以在任意帧解码，也就是说它每一帧都有头信息。ADIF只有一个统一的头，所以必须得到所有的数据后解码。且这两种header的格式也是不同的，目前一般编码后的和抽取的都是ADTS格式的音频流，ADTS是帧序列，本身具备流特征，在音频流的传输与处理方面更加合适

## ADTS详解

adts fream：`adts header-aac es-adts header-aac es`

> ADTS header的长度可能为7字节或9字节，protection_absent=0时 9字节-protection——absent=1时 7字节

```c++
adts_fixed_header {
	syncword; // 12bits, 同步头 总是0xfff， all bits must be 1, 代表着一个ADTS帧的开始
    ID;	// 1bits，MPEG Version 0 for MPEG-4, 1 for MPEG-2
    layer; // 2bits always '00'
    protection_absent; // 1bits 表示是否误码校验， 1表示无CRC；2表示SSR；3保留
    profile; // 2bits 表示使用那个级别的AAc，有些芯片只支持AAC LC。在MPEG-2 AAC中定义了3种：0表示Main profile；1表示LC；2表示SSR；3保留
    sampling_frequency_Index; // 4bits 采样率
    private_bit; // 1bits
    channel_configuration; // 3bits 表示省道数，1表示1channel；2表示2channel
    original_copy; // 1bits
    home; // 1bits
}
```

```c++
atds_variable_header() {
    copyright_identification_bit; // 1bits
    copyright_identification_start; // 1bits
    aac_frame_length; // 13bits
    adts_buffer_fullness; // 11bits
    number_of_raw_data_blocks_in_frame; // 2bits
}
```

