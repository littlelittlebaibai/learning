package com.example.ffmpegtest;

import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.view.Surface;
import android.view.SurfaceView;

import java.io.IOException;

public class CameraCapture {
    private Camera mCamera;
    private SurfaceTexture mSurfaceTexture;
    private float[] transformMatrix = new float[16];
   public void openCamera(){
//        mCamera = Camera.open(1);
//        Camera.Parameters parameters = mCamera.getParameters();
//        parameters.setPreviewFormat(ImageFormat.NV21);//set source image data
//        parameters.setPreviewSize(640, 480);
//        mCamera.setParameters(parameters);
   }

   public int[] getCameraID(){
       int ret[] = new int[2];
       int numberOfCameras = Camera.getNumberOfCameras();
       for (int i = 0; i < numberOfCameras; i++) {
           Camera.CameraInfo info = new Camera.CameraInfo();
           //获取相机信息
           Camera.getCameraInfo(i, info);
           //前置摄像头
           if (Camera.CameraInfo.CAMERA_FACING_FRONT == info.facing) {
               ret[0] = i;
           } else if (Camera.CameraInfo.CAMERA_FACING_BACK == info.facing) {
               ret[1] = i;
           }
       }
       return null;
   }

   public void setSurfaceFromNative(int id) throws IOException {
       //创建camera
       int cameraID[] = getCameraID();
       mCamera = Camera.open(0);
       Camera.Parameters parameters = mCamera.getParameters();
       parameters.setPreviewFormat(ImageFormat.NV21);//set source image data
       parameters.setPreviewSize(640, 480);

       mCamera.setParameters(parameters);

       mSurfaceTexture = new SurfaceTexture(id);
       mCamera.setPreviewTexture(mSurfaceTexture);//openGL绘制与camera获取数据关联起来
       mSurfaceTexture.setOnFrameAvailableListener(new SurfaceTexture.OnFrameAvailableListener() {
           @Override
           public void onFrameAvailable(SurfaceTexture surfaceTexture) {
               notifyFrameAvailable();
               //通知底层有数据了，可以开始绘制
           }
       });
       mCamera.startPreview();

   }

   public void upDateTexImageFromNative(){
       mSurfaceTexture.updateTexImage();
       mSurfaceTexture.getTransformMatrix(transformMatrix);//纹理转换矩阵，将该矩阵传递给native，和纹理坐标相乘，得到正确纹理坐标
   }
//   public void startPreview(){
//       //创建GL环境，创建好以后通知上册真正开启相机预览，为相机设置surface
//
//   }

   public native void notifyFrameAvailable();


}
