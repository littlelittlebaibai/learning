//
// Created by bytedance on 8/26/22.
//
#include <string>
#include "record_buffer.h"
#include "AudioEncode.h"


class OpenSLRecordPlay{

public:
    OpenSLRecordPlay(const SLObjectItf &engineObject);

    OpenSLRecordPlay();


    ~OpenSLRecordPlay();

    int startPlay(std::string path);

    int stopPlay();

    int startRecord(std::string path);

    int stopRecord();


private:
    //引擎
    SLObjectItf engineObject = 0;
    //引擎接口
    SLEngineItf engineInterface = 0;
    //混音器
    SLObjectItf outputMixObject = 0;
    //录制器
    SLObjectItf bqRecoderObject = 0;//object 录制对象，所有的对象都可抽象为SLObjectIft
    //播放器
    SLObjectItf bqPlayerObject = 0;

                        //录制对象，这个对象我们从里面获取了2个接口

public:

    SLRecordItf recorderRecorder = nullptr;                        //录制接口

    RecordBuffer *recordBuffer;

    FILE *pcmFile;

    bool finished;

    //编码部分
    AudioEncode *audioEncode;
    uint16_t  * encodeData;
};