#include <jni.h>
#include <string>

extern "C"{
#include "gif_lib.h"

}



struct GifBean{
    int current_frame;
    int total_frame;
    //用指针的方式 表示数组
    int *delays;
};



extern "C" JNIEXPORT jstring JNICALL
Java_com_gyx_mygifframework_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}



/**
 *
 * @param env    java 与 C之间的转化
 * @param clazz
 * @param path
 * @return
 */
extern "C"
JNIEXPORT jlong JNICALL
Java_com_gyx_mygifframework_GifHandler_loadGif(JNIEnv *env, jclass clazz, jstring path_) {
    // new 了一个数组，存放地址，0 表示不拷贝
    const char *path = env->GetStringUTFChars(path_,0);

    //打开 失败还是成功；
    int Error;
    //结构体
    GifFileType *gifFiletype = DGifOpenFileName(path, &Error);

    //初始化 缓冲区，数组 SaveImages
    DGifSlurp(gifFiletype);
    //实例化 gifBean
    GifBean *gifBean = static_cast<GifBean *>(malloc(sizeof(GifBean))); //static_cast<GifBean *>表示强转
    //清空数据 避免脏数据
    memset(gifBean, 0, sizeof(GifBean));

    gifFiletype->UserData = gifBean;
    gifBean->current_frame = 0;//当前帧
    gifBean->total_frame = gifFiletype->ImageCount;//总帧数



    //释放资源
    env->ReleaseStringUTFChars(path_, path);

    return (jlong) gifFiletype;//强转 返回结构体

}

/**
 * native 层，获取宽度
 */
extern "C"
JNIEXPORT jint JNICALL
Java_com_gyx_mygifframework_GifHandler_getWidth(JNIEnv *env, jclass clazz, jlong gif_Hander) {

    //将gif_Hander 强转成 GifFileType；
    GifFileType *gifFileType = reinterpret_cast<GifFileType *>(gif_Hander);

    return gifFileType->SWidth;

}
extern "C"
JNIEXPORT jint JNICALL
Java_com_gyx_mygifframework_GifHandler_getHeight(JNIEnv *env, jclass clazz, jlong gif_point) {


    //将 gif_point 强转成 GifFileType；
    GifFileType *gifFileType = reinterpret_cast<GifFileType *>(gif_point);

    return gifFileType->SHeight;



}
extern "C"
JNIEXPORT jint JNICALL
Java_com_gyx_mygifframework_GifHandler_updateFrame(JNIEnv *env, jclass clazz, jlong gif_point, jobject bitmap) {
    // TODO: implement updateFrame()
}































