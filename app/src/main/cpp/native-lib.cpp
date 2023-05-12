#include <jni.h>
#include <string>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "OpenSLRecordPlay.h"
#include "CameraPreview.h"
#include <android/native_window.h>
#include <android/native_window_jni.h>
extern "C" {
#include <libavcodec/avcodec.h>

}
static jobject g_obj = 0;
CameraPreview *cameraPreview;
void decode(AVCodecContext *pContext, AVPacket *pPacket, AVFrame *pFrame, FILE *pFile);


extern "C" JNIEXPORT jstring JNICALL
Java_com_example_ffmpegtest_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_example_ffmpegtest_MainActivity_decode(JNIEnv *env, jobject thiz) {
    // TODO: implement decode()
    const char *outfiltname, *filename;
    const AVCodec *codec;
    AVCodecContext *c= NULL;
    AVCodecParserContext *parser = NULL;//解析裸码流参数，比packet更小的处理单位
    int len,ret;
    FILE *f,*outfile;
    uint8_t inbuf[20480+4096];
    uint8_t *data;
    size_t data_size;
    AVPacket *pkt;//解复用后，解码器前的数据
    AVFrame *decode_frame = NULL;
    enum AVSampleFormat sfmt;
    int n_channels = 0;
    const char *fmt;
    pkt = av_packet_alloc();//如何知道申请的大小，固定值？

    codec = avcodec_find_decoder(AV_CODEC_ID_MP2);

    parser = av_parser_init(codec->id);

    c = avcodec_alloc_context3(codec);

    if(avcodec_open2(c,codec,NULL)<0) return -1;

    f = fopen(filename,"rb");

    outfile = fopen(outfiltname,"wb");

    data = inbuf;
    data_size = fread(inbuf,1,20480,f);//将码流文件按长度读入输入缓存区

    while (data_size>0){
        if(!decode_frame){
            if(!(decode_frame = av_frame_alloc())){
                return -2;
            }
        }
        //该函数用于从码流中读取完整的一帧数据，然后去解码，将缓存区中的数据解析为avpacket格式，可以理解为parser去承接原始的纯编码数据，可以将原始的纯编码数据包装成packet格式，用于解码
        //由此可以理解解码的时候需要的原始数据格式为packet格式
        ret = av_parser_parse2(parser,c,&pkt->data,&pkt->size,data,data_size,AV_NOPTS_VALUE,AV_NOPTS_VALUE,0);

        data +=ret;
        data_size -= ret;

        if(pkt->size)
            decode(c,pkt,decode_frame,outfile);

        //考虑数据不够长的情况
        //根据accodeccontext的设置，解析AVPacket中的码流，输出到AVFrame avcodec_decode_video2


    }
    //flush decoder

    //free resource
    avcodec_free_context(&c);
    av_parser_close(parser);
    av_frame_free(&decode_frame);
    av_packet_free(&pkt);





}

void decode(AVCodecContext *dec_ctx,AVPacket *pkt,AVFrame *frame,FILE *outflie) {
    int i,ch;
    int ret,data_size;
    ret = avcodec_send_packet(dec_ctx,pkt);
    while (ret>=0){
        ret = avcodec_receive_frame(dec_ctx,frame);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        data_size = av_get_bytes_per_sample(dec_ctx->sample_fmt);
        for(int i=0;i<frame->nb_samples;i++)
            for(ch = 0;ch<dec_ctx->ch_layout.nb_channels;ch++)
                fwrite(frame->data[ch]+data_size*i,1,data_size,outflie);

    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegtest_MainActivity_startRecord(JNIEnv *env, jobject thiz, jstring path) {
    const char *pcmPath = path == nullptr ? nullptr : env->GetStringUTFChars(path, nullptr);
    OpenSLRecordPlay *openSlRecordPlay = new OpenSLRecordPlay();
    openSlRecordPlay->startRecord(pcmPath);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_example_ffmpegtest_CameraRender_startPreview(JNIEnv *env, jobject thiz, jobject surface,
                                                      jint width, jint height) {
    cameraPreview = new CameraPreview();
    JavaVM *g_jvm = NULL;
    env->GetJavaVM(&g_jvm);
    g_obj = env->NewGlobalRef(thiz);
    if(surface != 0 && cameraPreview != NULL ){
        ANativeWindow *window = ANativeWindow_fromSurface(env,surface);
        cameraPreview->prepareEGLContext(window,g_jvm,g_obj,width,height,0);

    }
    return 0;
    // TODO: implement startPreview()
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegtest_CameraCapture_notifyFrameAvailable(JNIEnv *env, jobject thiz) {
    cameraPreview->notifyFrameAvailable();
    // TODO: implement notifyFrameAvailable()
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_ffmpegtest_CameraRender_startRecord(JNIEnv *env, jobject thiz) {
    // TODO: implement startRecord()
    cameraPreview->startRecord();
}