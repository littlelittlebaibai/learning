//
// Created by 朱天成 on 2023/1/9.
//
#include "VideoEncode.h"


VideoEncode::VideoEncode(int encodeHeight,int encodeWidth){
    this->encodeHeight = 640;//encodeHeight/2*2;
    this->encodeWidth = 480;//encodeWidth/2*2;
    f = fopen(filename,"wb");
    if(!f){
        fprintf(stderr,"could not open file");
    }

}

VideoEncode::~VideoEncode(){

}


int VideoEncode::initEncoder(){
    //1.查找编码器
   // codec = avcodec_find_decoder_by_name("ff_libx264_encoder");//
    codec = avcodec_find_encoder(AV_CODEC_ID_MPEG1VIDEO);
    if(!codec){
        return -1;
    }

    //2.分配编码context
    c = avcodec_alloc_context3(codec);
    if(!c){
        return -2;
    }

    //3.给packet分配空间
    pkt = av_packet_alloc();
    if(!pkt){
        return -3;
    }
    //4.设置参数
    c->bit_rate = 400000;//这个比特率参数是用来干啥的？？？可以理解为视频编码的采样率，码率不够的时候会导致原始的一帧图像像素数不够，应使用重采样解决，重采样又会导致画质下降，编解码器如何重采样？？？
    c->width = encodeWidth;
    c->height = encodeHeight;//和渲染帧保持一致
    c->time_base = (AVRational){1,25};//???
    c->framerate = (AVRational){25,1};//???
    c->gop_size = 10;
    c->max_b_frames =1;
    c->pix_fmt = AV_PIX_FMT_YUV420P;//???格式不匹配导致编码有问题？
    c->codec_type = AVMEDIA_TYPE_VIDEO;
    c->codec = codec;
    if(codec->id == AV_CODEC_ID_H264)//???
        av_opt_set(c->priv_data,"preset","slow",0);

    //5.打开编码器
    ret = avcodec_open2(c,codec,NULL);//-13
    if(ret<0){
        return -5;
    }
    //6.给frame分配空间
    frame = av_frame_alloc();
    if(!frame){
        return -4;
    }
    frame->format = c->pix_fmt;
    frame->width = c->width;
    frame->height = c->height;


    ret = av_frame_get_buffer(frame,0);
    if(ret<0){
        return -6;
    }
    //开启编码线程
   // pthread_create(&mEncodeThreadID,0,encode_frame,this);
    return 0;

}

int VideoEncode::sendFrames(uint8_t* frame_) {
    //===================

    c->bit_rate = 400000;//这个比特率参数是用来干啥的？？？可以理解为视频编码的采样率，码率不够的时候会导致原始的一帧图像像素数不够，应使用重采样解决，重采样又会导致画质下降，编解码器如何重采样？？？
    c->width = encodeWidth;
    c->height = encodeHeight;//和渲染帧保持一致
    c->time_base = (AVRational){1,25};//???
    c->framerate = (AVRational){25,1};//???
    c->gop_size = 10;
    c->max_b_frames =1;
    c->pix_fmt = AV_PIX_FMT_YUV420P;//???格式不匹配导致编码有问题？
    c->codec_type = AVMEDIA_TYPE_VIDEO;
    c->codec = codec;
    av_opt_set(c->priv_data, "tune", "zerolatency", 0);


    for(i=0;i<500;i++){
       // isGetNewFrame = false;
        fflush(f);
        ret = av_frame_make_writable(frame);
        if(ret<0)
            return -7;
        for(y=0;y<c->height;y++){
            for(x=0;x<c->width;x++){
                frame->data[0][y*frame->linesize[0]+x] = x+y+i*3;
            }
        }

        for(y=0;y<c->height/2;y++){
            for(x=0;x<c->height/2;x++){
                frame->data[1][y*frame->linesize[1]+x] = 128+y+i*2;
                frame->data[2][y*frame->linesize[2]+x] = 64+y+i*5;
            }
        }
        frame->pts = i*33;
        encodeFrames(c,frame,pkt);//c有问题
    }
    encodeFrames(c,NULL,pkt);
    if(codec->id ==AV_CODEC_ID_MPEG1VIDEO || codec->id == AV_CODEC_ID_MPEG2VIDEO){
        uint8_t encode[] = {0,0,1,0xb7};
        fwrite(encode,1,sizeof(encode),f);
    }
    fclose(f);
    avcodec_free_context(&c);
    av_frame_free(&frame);
    av_packet_free(&pkt);

    //===================
//    ret = av_frame_make_writable(frame);
//    if(ret<0)
//        return -7;
//    //从frame_中copy数据
////    for(int i = 0;i<3;i++){
////       // memcpy(frame->data[i],frame_+i*(encodeWidth*encodeHeight),encodeWidth*encodeHeight);
////        frame->data[i] = frame_+i*(encodeWidth*encodeHeight);
////    }
//    av_image_fill_arrays(frame->data,frame->linesize,frame_,c->pix_fmt,c->width,c->height,1);
//    isGetNewFrame = true;
//    FILE* fp = fopen("/sdcard/Android/data/com.example.ffmpegtest/files/dumpimageForEncode","wb+");
//    fwrite(frame->data[0],640 * 480 * 4,1,fp);
    return 0;


}
//单独开辟线程去编码，否则会卡住预览线程
void *VideoEncode::encode_frame(void *myself){
    VideoEncode* videoEncode = (VideoEncode*)myself;
    while (true){
        if(videoEncode->isGetNewFrame){
            videoEncode->encodeFrames(videoEncode->c,videoEncode->frame,videoEncode->pkt);
            videoEncode->isGetNewFrame = false;
        }
    }
}
int VideoEncode::encodeFrames(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt) {
    //7.给编码器送入原始数据
    //frame->pts = frameNum*30;
    frameNum++;
    ret = avcodec_send_frame(enc_ctx,frame);//-22
    if(ret<0){
        return -1;
    }
    while (ret>=0){
        ret = avcodec_receive_packet(enc_ctx,pkt);//一个packet不一定和一个frame的数据量完全一一对应
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return -2;
        else if (ret < 0) {
            fprintf(stderr, "Error during encoding\n");
            return -3;
        }
        fwrite(pkt->data,1,pkt->size,f);
        av_packet_unref(pkt);

    }
    return 0;
}
