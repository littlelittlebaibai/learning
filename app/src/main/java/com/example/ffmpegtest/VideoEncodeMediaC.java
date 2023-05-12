package com.example.ffmpegtest;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.os.Build;
import android.os.Environment;

import androidx.annotation.RequiresApi;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

public class VideoEncodeMediaC {
    /**
     * 1.创建编码器
     * 2.设置参数
     * 3.开启编码器
     * 4.通过两个缓冲器队列进行数据编解码
     * 5.释放资源
     */
    private static final String TAG = "MediaCode";
    private static final String MIME_TYPE="video/avc";
    private static final int FRAME_RATE = 30;
    private static final int I_FRAME_INTERVAL = 1;
    private static final int TIME_OUT = 10000;

    private MediaCodec mMediaCodec;
    private BufferedOutputStream mOutputStream;

    public void initEncoder(int width,int height,int bitRate) throws IOException {
        MediaFormat format = MediaFormat.createVideoFormat(MIME_TYPE,width,height);
        format.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);
        format.setInteger(MediaFormat.KEY_BIT_RATE,bitRate);
        format.setInteger(MediaFormat.KEY_FRAME_RATE,FRAME_RATE);
        format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL,I_FRAME_INTERVAL);

        mMediaCodec = MediaCodec.createEncoderByType(MIME_TYPE);
        mMediaCodec.configure(format,null,null,MediaCodec.CONFIGURE_FLAG_ENCODE);
        mMediaCodec.start();

        File file = new File(Environment.getExternalStorageDirectory(),"video.mp4");
        mOutputStream = new BufferedOutputStream(new FileOutputStream(file));
    }

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    public void encodeFrame(ByteBuffer buffer) throws IOException {
        int inputBufferIndex = mMediaCodec.dequeueInputBuffer(TIME_OUT);
        if(inputBufferIndex>=0){
            ByteBuffer inputBuffer = mMediaCodec.getInputBuffer(inputBufferIndex);
            inputBuffer.clear();
            inputBuffer.put(buffer);
            mMediaCodec.queueInputBuffer(inputBufferIndex,0,buffer.limit(),0,0);
            MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();
            int outputBufferIndex = mMediaCodec.dequeueOutputBuffer(bufferInfo,TIME_OUT);
            while (outputBufferIndex>=0){
                ByteBuffer outputBuffer = mMediaCodec.getOutputBuffer(outputBufferIndex);
                if(mOutputStream!=null){
                    byte[] data = new byte[bufferInfo.size];
                    outputBuffer.get(data);
                    mOutputStream.write(data);
                }
                mMediaCodec.releaseOutputBuffer(outputBufferIndex,false);
                outputBufferIndex = mMediaCodec.dequeueOutputBuffer(bufferInfo,TIME_OUT);

            }

        }
    }

    public void releaseEncoder() throws IOException {
        if(mMediaCodec!=null){
            mMediaCodec.start();
            mMediaCodec.release();
            mMediaCodec = null;
        }
        if(mOutputStream != null){
            mOutputStream.flush();
            mOutputStream.close();
            mOutputStream = null;
        }

    }
}
