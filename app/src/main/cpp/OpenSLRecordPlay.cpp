//
// Created by bytedance on 8/26/22.
//

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "OpenSLRecordPlay.h"
#include "record_buffer.h"
#include <stdio.h>
#include <android/log.h>
#define RECORDER_FRAMES (2048)
#define TAG "OpenSLRecorder"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__) // 定义LOGD类型
static unsigned recorderSize = RECORDER_FRAMES * 2;

static void RecordCallback(SLAndroidSimpleBufferQueueItf bufferQueue,void* context){
    OpenSLRecordPlay *recorder = (OpenSLRecordPlay *)context;

    if(recorder->recordBuffer!=NULL){
        //加入audio encode，调整时间戳看对编码的影响
        memcpy(recorder->encodeData,recorder->recordBuffer->getNowBuffer(),recorderSize);
        fwrite(recorder->encodeData,1,recorderSize,recorder->pcmFile);//getNowBuffer()的指针指向了录制进来的data
        recorder->audioEncode->sendSamples(recorder->encodeData);
    }
    //录制进来的数据是已经经过了采样 量化 编码（PCM）的

    if(recorder->finished){
        (*recorder->recorderRecorder)->SetRecordState(recorder->recorderRecorder,SL_RECORDSTATE_STOPPED);
        fclose(recorder->pcmFile);
        delete  recorder->recordBuffer;
        recorder->recordBuffer = NULL;
    } else{
        (*bufferQueue)->Enqueue(bufferQueue,recorder->recordBuffer->getRecordBuffer(),recorderSize);
    }




}


int OpenSLRecordPlay::startPlay(std::string path) {

    return 0;
}

int OpenSLRecordPlay::startRecord(std::string path) {
    int channels = 2;
    int sampleRate = 44100;
    int bitRate = 0;
    //打开输出文件
    pcmFile = fopen(path.c_str(), "wb");
    LOGD(TAG, "pcm path is %s",pcmFile);

    recordBuffer = new RecordBuffer(RECORDER_FRAMES*2);

    //1.调用全局方法创建一个引擎对象，唯一入口
    SLresult sLresult = slCreateEngine(&engineObject,0,nullptr,0, nullptr, nullptr);
    if(sLresult!=SL_RESULT_SUCCESS)
        return -1;

    //2 实例化引擎对象
    sLresult = (*engineObject)->Realize(engineObject,SL_BOOLEAN_FALSE);
    if(sLresult!=SL_RESULT_SUCCESS)
        return -2;

    sLresult = (*engineObject)->GetInterface(engineObject,SL_IID_ENGINE,&engineInterface);//从引擎对象中获取引擎接口
    if(sLresult != SL_RESULT_SUCCESS)
        return -3;

    SLDataLocator_IODevice ioDevice = {
            SL_DATALOCATOR_IODEVICE,         //类型，只能是该类型
            SL_IODEVICE_AUDIOINPUT,          //device类型 选择了音频输入类型（录音）
            SL_DEFAULTDEVICEID_AUDIOINPUT,   //deviceID
            NULL
    };

    SLDataSource dataSource = {
            &ioDevice,
            NULL
    };

    //设置输出buffer队列
    SLDataLocator_AndroidSimpleBufferQueue buffer_queue = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
            2
    };

    SLDataFormat_PCM pcmFormat = {
            SL_DATAFORMAT_PCM,
            (SLuint32) channels,
            SL_SAMPLINGRATE_44_1,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN

    };
    SLDataSink audioSink = {
            &buffer_queue,
            &pcmFormat,
    };
    //对象是抽象的，如引擎对象，录音器对象都是SLObjectIt，而接口是指定的，如引擎接口，录音器接口，buffer接口等

    SLAndroidSimpleBufferQueueItf recorderBufferQueue;

    const SLInterfaceID  id[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean  req[1] = {SL_BOOLEAN_TRUE};

    //从对象中获取接口，通过接口去实现特定能力，如利用引擎接口创建录音器对象，创建依据：和设备相关的audioSource和格式相关的audioSink
    sLresult = (*engineInterface)->CreateAudioRecorder(engineInterface,&bqRecoderObject,&dataSource,&audioSink,1,id,req);

    if(sLresult != SL_RESULT_SUCCESS ) return -3;

    sLresult = (*bqRecoderObject)->Realize(bqRecoderObject,SL_BOOLEAN_FALSE);//realize失败
    if(sLresult != SL_RESULT_SUCCESS)
        return -3;

    (*bqRecoderObject)->GetInterface(bqRecoderObject,SL_IID_RECORD,&recorderRecorder);//从录制对象中获取录音器接口
    (*bqRecoderObject)->GetInterface(bqRecoderObject,SL_IID_ANDROIDSIMPLEBUFFERQUEUE,&recorderBufferQueue);//从录制对象中回去buffer接口

    finished = false;
    sLresult = (*recorderBufferQueue)->Enqueue(recorderBufferQueue,recordBuffer->getRecordBuffer(),recorderSize);//recordBuffer->getRecordBuffer()传入空间，便于A区数据
    if(sLresult!= SL_RESULT_SUCCESS) return -4;

    sLresult = (*recorderBufferQueue)->RegisterCallback(recorderBufferQueue,RecordCallback,this);
    if(sLresult!= SL_RESULT_SUCCESS) return -5;
    (*recorderRecorder)->SetRecordState(recorderRecorder,SL_RECORDSTATE_RECORDING);



    return 0;
}

OpenSLRecordPlay::OpenSLRecordPlay() {
    audioEncode = new AudioEncode();
    int ret = audioEncode->initEncoder();
    if(ret<0){
        LOGD(TAG,"encode error fail code is %d",ret);
    }
    encodeData  = new uint16_t [RECORDER_FRAMES*2];

}
