## 编码流程

1. `avformat_open_input`
2. `avformat_find_stream_info`
3. `av_find_best_stream`
4. `avcodec_alloc_context3`
5. `avcodec_patameters_to_context`
6. `avvodec_find_decoder`
7. `avcodec_open2`
8. `av_frame_alloc`
9. `av_samples_get_buffer_size`
10. `avcodec_fill_audio_frame`
11. `av_read_frame`
12. `avcodec_send_frame`
13. `avcodec_receive_frame`

    

