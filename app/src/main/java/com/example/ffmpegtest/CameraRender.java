package com.example.ffmpegtest;

import android.content.Context;
import android.util.AttributeSet;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.io.IOException;

public class CameraRender extends SurfaceView implements SurfaceHolder.Callback {
    CameraCapture mCameraCapture;

    //Simple constructor to use when creating a view from code

    public CameraRender(Context context, AttributeSet attrs)  {
        super(context,attrs);
        SurfaceHolder surfaceHolder = getHolder();
        surfaceHolder.addCallback(this);
        mCameraCapture = new CameraCapture();

    }  //Constructor that is called when inflating a view from XML

   public CameraRender(Context context, AttributeSet attrs, int defStyle){
        super(context,attrs,defStyle);
       SurfaceHolder surfaceHolder = getHolder();
       surfaceHolder.addCallback(this);
       mCameraCapture = new CameraCapture();

    }     //Perform inflation from XML and apply a class-specific base style

    public CameraRender(Context context) {
        super(context);
        SurfaceHolder surfaceHolder = getHolder();
        surfaceHolder.addCallback(this);
        mCameraCapture = new CameraCapture();

    }
    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Surface surface = holder.getSurface();
        int width = getWidth();
        int height = getHeight();//把surface设置到native层的EGL，将OpenGL和设备连接起来，用于显示
        startPreview(surface,width,height);//把surface传下去

    }

    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {

    }

    public void startCameraPreview(){
        mCameraCapture.openCamera();
    }

//camera1
//mediaCodec 硬编码，可以接受surface作为输入
    public void setCameraSurface(int texID) throws IOException {
        mCameraCapture.setSurfaceFromNative(texID);
    }

    public void updateTexImage(){
        mCameraCapture.upDateTexImageFromNative();
    }
    public native int startPreview(Surface surface,int width, int height);

}
