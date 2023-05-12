
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
}
#include <pthread.h>

class VideoEncode{
public:
    VideoEncode(int encodeHeight,int encodeWidth);
    virtual ~VideoEncode();
    int initEncoder();
    int sendFrames(uint8_t* frame);
    int encodeFrames(AVCodecContext *enc_ctx,AVFrame *frame,AVPacket *pkt);
    static  void *encode_frame(void *myself);
    //static void *render_stream(void *myself);
    const AVCodec *codec;
    const char *codec_name;
    AVCodecContext *c = NULL;
    AVFrame *frame;
    AVPacket *pkt;
    int i,x,y,ret;

    int encodeHeight;
    int encodeWidth;
    pthread_t mEncodeThreadID;
    bool isGetNewFrame = false;
    int frameNum = 0;

    //编码文件
    const char *filename = "/sdcard/Android/data/com.example.ffmpegtest/files/afterVideoencode.mp4";
    FILE *f;


};