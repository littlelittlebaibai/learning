//
// Created by 朱天成 on 2022/12/10.
//

#ifndef FFMPEGTEST_CAMERAPREVIEW_H
#define FFMPEGTEST_CAMERAPREVIEW_H

#endif //FFMPEGTEST_CAMERAPREVIEW_H
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <pthread.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "VideoEncode.h"

static char* GPU_FRAME_VERTEX_SHADER =
        "attribute vec4 vPosition;\n"
        "attribute vec4 vTexCords;\n"
        "varying vec2 yuvTexCoords;\n"
        "uniform highp mat4 trans; \n"
        "void main() {\n"
        "  yuvTexCoords = vTexCords.xy;\n"
        "  gl_Position = trans * vPosition;\n"
        "}\n";



static char* GPU_FRAME_FRAGMENT_SHADER =
        "#extension GL_OES_EGL_image_external : require\n"
        "precision mediump float;\n"
        "uniform samplerExternalOES yuvTexSampler;\n"
        "varying vec2 yuvTexCoords;\n"
        "void main() {\n"
        " vec4 rgba = texture2D(yuvTexSampler, yuvTexCoords);\n"
        "float gray = (0.30 * rgba.r   + 0.59 * rgba.g + 0.11* rgba.b);\n"
        "gl_FragColor = vec4(rgba.r, rgba.g, rgba.b, 1.0);\n"
        //textture2D:纹理采样器，获取纹理上指定位置的颜色值，会读入纹理上特定坐标处的颜色值，读给谁？
        //yuvTexSampler:oes纹理
        //yuvTexCoords:纹理顶点数据，纹理坐标
        "}\n";
//vec4 rgba =texture2D(vTexture, aCoord);
//    float gray = (0.30 * rgba.r   + 0.59 * rgba.g + 0.11* rgba.b);
//    gl_FragColor = vec4(gray, gray, gray, 1.0);

class CameraPreview{
public:
    int prepareEGLContext(ANativeWindow *window, JavaVM *g_jvm,
                          jobject obj, int screenWidth, int screenHeight,
                          int cameraFacingId);

    bool  initialize = false;

    /** 启动预览线程的五重要个参数 **/
    ANativeWindow *_window;
    JavaVM *g_jvm;
    jobject obj;
    int screenWidth;
    int screenHeight;
    bool isInSwitchingCamera;

    pthread_t _threadID;
    static void *render_stream(void *myself);


    //EGL
    int initEGL(EGLContext context);
    EGLDisplay display;
    EGLConfig config;
    EGLContext context;
    EGLSurface surface = NULL;

    //render
    int initRender();
    GLint loadProgram(char* vertexSource, char* fragmentSource);
    int processFrame();
    int drawFrame();
    void updateTexImage();

    GLint loadShader(GLenum shaderType, const char* pSource);
    int textureWidth = 0;
    int textureHeight = 0;
    int cameraWidth  = 0;
    int cameraHeight = 0;
    int degree = 0;
    GLuint texId;
    GLuint programID;
    GLint mGLUniformTexture;
    GLuint mGLVertexCoords;
    GLuint mGLTextureCoords;

    GLint mUniformTexMatrix;
    GLint mUniformTransforms;

    //camera
    void setCameraPreviewTexture();
    void notifyFrameAvailable();
    bool frameAvailable = false;


    void matrixSetIdentityM(float *m);

    //record
    bool isRecord = false;
    bool isEncodeInit = false;
    VideoEncode *mVideEncode = nullptr;
    uint8_t* copyImageFromImage();
    void startRecord();
};