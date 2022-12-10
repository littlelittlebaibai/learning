//
// Created by bytedance on 9/15/22.
//参考：ffmpeg官方demo
//
#include "AudioEncode.h"
static int check_sample_fmt(const AVCodec *codec, enum AVSampleFormat sample_format){
    const enum AVSampleFormat *p = codec->sample_fmts;

    while (*p != AV_SAMPLE_FMT_NONE){
        if(*p == sample_format)
            return 1;
        p++;
    }
    return 0;
}

static int select_sampel_rate(const AVCodec *codec){
    const int *p;
    int best_samplerate =0;

    if(!codec->supported_samplerates)
        return 44100;
    p = codec->supported_samplerates;
    while (*p){
        if(!best_samplerate || abs(44100-*p) < abs(44100-best_samplerate))//选择最靠近44100的
            best_samplerate = *p;
        p++;

    }
    return best_samplerate;

}

static int select_channel_layout(const AVCodec *codec,AVChannelLayout *dst){
    //AVCahnnelLayout 多声道的布局和顺序
    const AVChannelLayout *p,*best_ch_layout,*src;
    //src = &(AVChannelLayout)AV_CHANNEL_LAYOUT_STEREO;
    int best_nb_channels = 0;
    if(!codec->ch_layouts)
        return av_channel_layout_copy(dst, src);
    p = codec->ch_layouts;
    while (p->nb_channels){
        int nb_channels = p->nb_channels;

        if(nb_channels > best_nb_channels){
            best_ch_layout = p;
            best_nb_channels = nb_channels;

        }
        p++;
    }
    return av_channel_layout_copy(dst,best_ch_layout);



}

AudioEncode::~AudioEncode() {

}

AudioEncode::AudioEncode() {
    tmp = fopen(tmpfilename, "wb");
    tmp1 = fopen(tmp1filename, "wb");

}

int AudioEncode::initEncoder() {
    codec = avcodec_find_encoder(AV_CODEC_ID_MP2);
    if(!codec){
        fprintf(stderr,"codec not found\n");
    }

    c = avcodec_alloc_context3(codec);
    if(!c){
        fprintf(stderr,"could not allocate audio codec context \n");
        return -1;
    }

    c->bit_rate = 64000;//体会/修改编码参数对实际编码的影响，重点看pts 和 dts


    c->sample_fmt = AV_SAMPLE_FMT_S16;
    if(!check_sample_fmt(codec,c->sample_fmt)){
        fprintf(stderr,"not support this format!");
        return -2;
    }

    c->codec = codec;
    c->sample_rate = 44100;//sampe rate ,channel ,format与输入数据对应
    c->channels = 2;
    c->channel_layout= AV_CH_LAYOUT_STEREO;//av_get_default_channel_layout(c->channels);
    c->codec_type = AVMEDIA_TYPE_AUDIO;

//    ret = select_channel_layout(codec,&c->ch_layout);
//    if(ret<0) return -3;

    //编码的时候avcodec_open2会设置frame_size
    int ret = avcodec_open2(c,codec,NULL);
    if(ret < 0){
        fprintf(stderr,"could not open codec \n");
        return -4;
    }

    f = fopen(filename,"wb");
    if(!f){
        fprintf(stderr,"could not open file");
        return -5;
    }

    pkt = av_packet_alloc();//AVPacket用于存储编码后的数据，视频中一个packet只有一帧压缩数据，而一个音频packet中有多帧
    if(!pkt){
        fprintf(stderr,"could not allocate the packet");
        return -6;
    }

    frame = av_frame_alloc();
    if(!frame){
        fprintf(stderr,"could not alloate the frame");
        return -7;
    }

    frame->nb_samples = c->frame_size;
    frame->format = c->sample_fmt;
    ret = av_channel_layout_copy(&frame->ch_layout,&c->ch_layout);
    if(ret<0){
        fprintf(stderr,"could not copy channel layout");
        return -8;
    }

    ret = av_frame_get_buffer(frame,0);
    if(ret<0){
        fprintf(stderr,"could not allocate auio data");
    }
    //至此，准备工作完毕，下面需要将采集进来的数据做编码
//    t = 0;
//    tincr = 2 * M_PI * 440.0 / c->sample_rate;
//    for (i = 0; i < 200; i++) {
//        /* make sure the frame is writable -- makes a copy if the encoder
//         * kept a reference internally */
//        ret = av_frame_make_writable(frame);
//        if (ret < 0)
//            exit(1);
//        samples = (uint16_t*)frame->data[0];
//
//        for (j = 0; j < c->frame_size; j++) {
//            samples[2*j] = (int)(sin(t) * 10000);
//
//            for (k = 1; k < c->ch_layout.nb_channels; k++)
//                samples[2*j + k] = samples[2*j];//声道copy
//            t += tincr;
//        }
//       // encodeFrames(c, frame, pkt, f);
//    }


    return 0;
}

int AudioEncode::sendSamples(uint16_t  * samples_) {
    int ret = av_frame_make_writable(frame);//frame没有初始化
   // fwrite(samples_,1,4096,tmp1);
    memcpy(frame->data[0],samples_,4096);
  //  fwrite(frame->data[0],1,4096,tmp);
    encodeFrames(c,frame,pkt,f);


    return 0;
}

int AudioEncode::encodeFrames(AVCodecContext *ctx,AVFrame *frame,AVPacket *pkt,FILE *output) {
    int ret;
    ret = avcodec_send_frame(ctx,frame);
    if(ret<0){
        fprintf(stderr,"wrong frame");
        return -1;
    }
    while (ret>=0){
        ret = avcodec_receive_packet(ctx,pkt);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return 0;
        else if (ret<0){
            fprintf(stderr,"wrong encode audio frame");
            return -2;
        }
        fwrite(pkt->data,1,pkt->size,output);//编码后的数据存在packet里，音质有问题！！！
       // pkt->dts;//解码时间：B帧需要在P帧解码后才能解码
        //pkt->pts;//显示时间：在时间上，B帧有可能在P帧前面
        av_packet_unref(pkt);
    }

    return 0;
}
