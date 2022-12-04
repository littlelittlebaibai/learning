//
// Created by bytedance on 8/26/22.
//
#include <string>
#include "../../../../../../SDKandNDK/android-ndk-r21b/toolchains/llvm/prebuilt/darwin-x86_64/sysroot/usr/include/SLES/OpenSLES.h"
#include "record_buffer.h"


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
    SLObjectItf bqRecoderObject = 0;//object 录制对象
    //播放器
    SLObjectItf bqPlayerObject = 0;

                        //录制对象，这个对象我们从里面获取了2个接口

public:

    SLRecordItf recorderRecorder = nullptr;                        //录制接口

    RecordBuffer *recordBuffer;

    FILE *pcmFile;

    bool finished;
};