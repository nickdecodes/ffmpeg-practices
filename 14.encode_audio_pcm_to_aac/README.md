## 编码流程

1. `av_frame_alloc`
2. `av_frame_get_buffer`
3. `av_find_best_stream`
4. `avcodec_find_encoder_by_name`
5. `avcodec_alloc_context3`
6. `avcodec_open2`
7. `avcodec_send_frame`
8. `avcodec_receive_packet`

    

