//
// Created by bairong on 2022/12/10.
//
#include <cstring>
#include "CameraPreview.h"
#include <unistd.h>
//surfaceCreate时调用，考虑surface发生变化，EGL又初始化好了的情况
int CameraPreview::prepareEGLContext(ANativeWindow *window, JavaVM *g_jvm, jobject obj, int screenWidth,
                                 int screenHeight, int cameraFacingId) {
    this->g_jvm = g_jvm;
    this->obj = obj;
    this->screenHeight = screenHeight;//surface的宽高 2117
    this->screenWidth = screenWidth;//1080
    this->_window = window;
    pthread_create(&_threadID,0,render_stream,this);


    return 0;
}


void *CameraPreview::render_stream(void* mySelf){
    //init EGL,if first preview
    CameraPreview *cameraPreview = (CameraPreview*)mySelf;
    if(!cameraPreview->initialize){
        cameraPreview->initEGL(NULL);
        cameraPreview->initRender();
        cameraPreview->setCameraPreviewTexture();
        cameraPreview->initialize = true;
    }
    while (true){
        if(cameraPreview->frameAvailable){
            cameraPreview->updateTexImage();
            cameraPreview->drawFrame();
            cameraPreview->frameAvailable = false;
            if(cameraPreview->isRecord){
                if(!cameraPreview->isEncodeInit){
                    cameraPreview->mVideEncode = new VideoEncode(cameraPreview->screenHeight,cameraPreview->screenWidth);
                    cameraPreview->mVideEncode->initEncoder();
                    cameraPreview->isEncodeInit = true;
                    cameraPreview->mVideEncode->sendFrames(NULL);//
                }
                uint8_t *image = cameraPreview->copyImageFromImage();//耗时，是否会阻塞预览

            }
        }

    }
}
int CameraPreview::initEGL(EGLContext context_) {
    EGLint numConfigs;
    EGLint width;
    EGLint height;

    const EGLint attribs[] = { EGL_BUFFER_SIZE, 32, EGL_ALPHA_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                               EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_NONE };
    //1.创建与本地窗口的连接
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if(display == EGL_NO_DISPLAY){
        return -1;
    }
    //2.初始化EGL方法
    int ret = eglInitialize(display,0,0);
    if(ret<0){
        return -2;
    }

    //3.确定渲染表面配置信息
    ret = eglChooseConfig(display,attribs,&config,1,&numConfigs);
    if(ret<0){
        return -3;
    }

    //4.创建EGL context
    EGLint eglContextAttributes[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    context = eglCreateContext(display,config,NULL==context_?EGL_NO_CONTEXT : context,eglContextAttributes);

    //5.创建渲染表面
    EGLint format;
    ret = eglGetConfigAttrib(display,config,EGL_NATIVE_VISUAL_ID,&format);
    if(ret<0){
        return -4;
    }
    ANativeWindow_setBuffersGeometry(_window,0,0,format);//设置窗口buffer的格式和大小
    surface = eglCreateWindowSurface(display,config,_window,0);
    if(surface == NULL){
        return -5;
    }
    //6.绑定上下文
    ret = eglMakeCurrent(display,surface,surface,context);

    return 0;
}
int CameraPreview::initRender(){
    glGenTextures(1,&texId);//产生一个纹理
    //将纹理ID绑定到指定纹理，也是就声明纹理ID的类型
    glBindTexture(GL_TEXTURE_EXTERNAL_OES,texId);
    //设置纹理采样方式
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

    //链接着色器程序
    programID = loadProgram(GPU_FRAME_VERTEX_SHADER,GPU_FRAME_FRAGMENT_SHADER);

    //获取shader代码中的变量索引，绘图坐标
    mGLVertexCoords = glGetAttribLocation(programID,"vPosition");
    //纹理坐标
    mGLTextureCoords = glGetAttribLocation(programID, "vTexCords");
    //oes纹理
    mGLUniformTexture = glGetUniformLocation(programID, "yuvTexSampler");

    mUniformTexMatrix = glGetUniformLocation(programID, "texMatrix");

    mUniformTransforms = glGetUniformLocation(programID, "trans");
    return 0;
}
GLint CameraPreview::loadProgram(char *vertexSource, char *fragmentSource) {
    //两个着色器链接成一个program
    GLint  vertexShader = loadShader(GL_VERTEX_SHADER, reinterpret_cast<const char *>(vertexSource));
    GLint fragmentShader = loadShader(GL_FRAGMENT_SHADER,
                                      reinterpret_cast<const char *>(fragmentSource));
    GLint program = glCreateProgram();
    if(!program) return -1;
    glAttachShader(program,vertexShader);
    glAttachShader(program,fragmentShader);
    glLinkProgram(program);
    return program;

}
GLint CameraPreview::loadShader(GLenum shaderType, const char* pSource){
    GLint shader = glCreateShader(shaderType);
    if(!shader) return -1;
    glShaderSource(shader,1,&pSource,NULL);//替换着色器中的源代码
    glCompileShader(shader);
    GLint compiled = 0;
    glGetShaderiv(shader,GL_COMPILE_STATUS,&compiled);//检测着色器是否编译成功
    if(!compiled){
        return -1;
    }
    return shader;
}
void CameraPreview::setCameraPreviewTexture() {
    //把纹理ID传回java，产生camera的surface
    JNIEnv *env;//prepareEGLContext传下来的env是另一个线程的
    int ret = g_jvm->AttachCurrentThread(&env,NULL);
    if(ret!=JNI_OK || env == NULL) return;
    jclass jcls = env->GetObjectClass(obj);
    if(jcls == NULL) return;
    jmethodID setCameraSurface = env->GetMethodID(jcls,"setCameraSurface","(I)V");
    env->CallVoidMethod(obj,setCameraSurface,texId);
    //JNI使用！！！！


}
//java层收到frameonavailable通知底层去绘制，怎么保证同在GL线程？
void CameraPreview::notifyFrameAvailable(){
    frameAvailable = true;
    //process frame
    //draw
    //updateTexImage();
   // drawFrame();
}

void CameraPreview::updateTexImage(){
    //更新texture中的数据
    JNIEnv *env;
    g_jvm->AttachCurrentThread(&env,NULL);
    if(env == NULL) return;
    jclass jcls = env->GetObjectClass(obj);//该obj是cameraRender
    jmethodID updateTexImage = env->GetMethodID(jcls,"updateTexImage","()V");
    env->CallVoidMethod(obj,updateTexImage);


}
int CameraPreview::processFrame(){
   // updateTexImage();
//    //激活纹理单元
//   glActiveTexture(GL_TEXTURE0);
//   //告诉openGL选择texId作为当前处理的纹理，后续所有的操作都放在该纹理上，激活内部纹理后将内部纹理和纹理ID绑定
//   glBindTexture(GL_TEXTURE_EXTERNAL_OES,texId);
//    //告诉openGL使用的纹理是哪一个纹理
//   // 保证纹理采样器对应正确的纹理单元，第二个参数0就对应glActiveTexture(GLES20.GL_TEXTURE0)
//   glUniform1i(mGLUniformTexture,0);
   return 0;

}

int CameraPreview::drawFrame() {
    //glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    //决定视区所见区域(0,0)窗口左下角位置，screenWidth,screenHeight窗口的宽和高
    glViewport(0,0,screenWidth,screenHeight);
    //intall a program object as part of current render state
    glUseProgram(programID);

    //绘图坐标可认为是往surface的哪个地方绘制，默认surface的大小是1X1的
    //因此坐标的具体值需要根据surface的宽高和camera帧的宽高去计算
    //surface(1080X2117) camera(640X480) x=1,y=(1080/640*480)/1080
    //此外，要注意绘图坐标和纹理坐标的对应关系，将纹理坐标对应的点根据surface的坐标绘制到surface上去
    static const GLfloat _vertices[] = { 1.0f, 0.7f, 1.0f, -0.7f, -1.0f, 0.7f,
                                         -1.0f, -0.7f };
    //绘图坐标，给vPosition所代表的变量传值
    glVertexAttribPointer(mGLVertexCoords, 2, GL_FLOAT, 0, 0, _vertices);
    //enable or disable a verex attribute array
    glEnableVertexAttribArray(mGLVertexCoords);

    static const GLfloat texCoords[] = { 0.0f, 0.0, 1.0f, 0.0, 0.0f, 1.0f,
                                         1.0f, 1.0f };
    //纹理坐标
    glVertexAttribPointer(mGLTextureCoords, 2, GL_FLOAT, 0, 0, texCoords);
    glEnableVertexAttribArray(mGLTextureCoords);

    //绘图
    /* Binding the input texture */
    //激活纹理单元
    glActiveTexture (GL_TEXTURE0);
    //告诉openGL选择texId作为当前处理的纹理，后续所有的操作都放在该纹理上，激活内部纹理后将内部纹理和纹理ID绑定
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texId);
    //告诉openGL使用的纹理是哪一个纹理
    // 保证纹理采样器对应正确的纹理单元，第二个参数0就对应glActiveTexture(GLES20.GL_TEXTURE0)
    glUniform1i(mGLUniformTexture, 0);
    //ID绑定内部纹理，内部纹理和代码的纹理绑定


    float rotateMatrix[4 * 4];
    matrixSetIdentityM(rotateMatrix);
    glUniformMatrix4fv(mUniformTransforms, 1, GL_FALSE, (GLfloat *) rotateMatrix);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(mGLVertexCoords);
    glDisableVertexAttribArray(mGLTextureCoords);
    glBindTexture(GL_TEXTURE_2D, 0);


    eglSwapBuffers(display,surface);

    return 0;
}
void CameraPreview::matrixSetIdentityM(float *m)
{
    memset((void*)m, 0, 16*sizeof(float));//screenHeight,screenWidth
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

void CameraPreview::startRecord() {
    isRecord = true;
}

uint8_t* CameraPreview::copyImageFromImage() {
    const int dataLength = 640 * 480 * 4 ;
    char* pixelData = new char[dataLength];
    //uint8_t *image = new uint8_t [screenWidth*screenWidth*4];
    glReadPixels(0,0,480,640,GL_RGBA, GL_UNSIGNED_BYTE,pixelData);//宽高不对，read出来的数据不对，按照surface大小尝试
    //dump pixeldata
    FILE* fp = fopen("/sdcard/Android/data/com.example.ffmpegtest/files/dumpimage","wb+");
    if(fp == nullptr)
        return nullptr;
    fwrite(pixelData,640 * 480 * 4,1,fp);

    return reinterpret_cast<uint8_t *>(pixelData);

//    int pixelSize = screenWidth*screenWidth
//    byte *packetBuffer = new byte[pixelSize];
}



