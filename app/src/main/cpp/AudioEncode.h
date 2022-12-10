//
// Created by bytedance on 9/15/22.
//

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
}
#include "memory.h"

#ifndef FFMPEGTEST_AUDIOENCODE_H
#define FFMPEGTEST_AUDIOENCODE_H

#endif //FFMPEGTEST_AUDIOENCODE_Hc
class AudioEncode{


public:
    AudioEncode();

public:
    virtual ~AudioEncode();

    int initEncoder();

    int sendSamples(uint16_t  * samples);

    int encodeFrames(AVCodecContext *ctx,AVFrame *frame,AVPacket *pkt,FILE *output);

private:
    const char *filename = "/sdcard/Android/data/com.example.ffmpegtest/files/afterencode.mp3";
    const char *tmpfilename = "/sdcard/Android/data/com.example.ffmpegtest/files/afterencode.pcm";
    const char *tmp1filename = "/sdcard/Android/data/com.example.ffmpegtest/files/afterencode1.pcm";
    const AVCodec *codec;
    AVCodecContext *c= NULL;
    AVFrame *frame;
    AVPacket *pkt;
    int i, j, k, ret;
    FILE *f;
    FILE *tmp;
    FILE *tmp1;
    uint16_t *samples;
    //FILE *encodeFile;
    float t, tincr;


};