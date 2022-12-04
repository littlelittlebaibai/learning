//
// Created by bytedance on 9/15/22.
//

#include <libavcodec/codec.h>
#include <libavcodec/avcodec.h>

#ifndef FFMPEGTEST_AUDIOENCODE_H
#define FFMPEGTEST_AUDIOENCODE_H

#endif //FFMPEGTEST_AUDIOENCODE_Hc
class AudioEncode{


public:
    AudioEncode();

public:
    virtual ~AudioEncode();

    int initEncoder();

    int sendSamples(short * samples);

    int encodeFrames(AVCodecContext *ctx,AVFrame *frame,AVPacket *pkt,FILE *output);

private:
    const char *filename;
    const AVCodec *codec;
    AVCodecContext *c= NULL;
    AVFrame *frame;
    AVPacket *pkt;
    int i, j, k, ret;
    FILE *f;
    uint16_t *samples;

};