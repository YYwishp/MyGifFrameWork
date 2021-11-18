#include <jni.h>
#include <string>


#include <android/bitmap.h>




extern "C"{
#include "gif_lib.h"

}

#include <android/log.h>
#define  LOG_TAG    "David"
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  dispose(ext) (((ext)->Bytes[0] & 0x1c) >> 2)
#define  trans_index(ext) ((ext)->Bytes[3])
#define  transparency(ext) ((ext)->Bytes[0] & 1)

struct GifBean{
    int current_frame;
    int total_frame;
    //用指针的方式 表示数组
    int *delays;
};

#define  argb(a,r,g,b) ( ((a) & 0xff) << 24 ) | ( ((b) & 0xff) << 16 ) | ( ((g) & 0xff) << 8 ) | ((r) & 0xff)


// extern "C" JNIEXPORT jstring JNICALL
// Java_com_gyx_mygifframework_MainActivity_stringFromJNI(
//         JNIEnv* env,
//         jobject /* this */) {
//     std::string hello = "Hello from C++";
//     return env->NewStringUTF(hello.c_str());
// }



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

/**
 * 绘制 兼容 87a 89a 版本
 * @param gif
 * @param info
 * @param pixels
 * @param force_dispose_1
 * @return
 */
int drawFrame(GifFileType *gif, AndroidBitmapInfo info, void *pixels, bool force_dispose_1) {
    GifColorType *bg;

    GifColorType *color;

    SavedImage *frame;

    ExtensionBlock *ext = 0;

    GifImageDesc *frameInfo;

    ColorMapObject *colorMap;

    int *line;

    int width, height, x, y, j, loc, n, inc, p;

    void *px;

    GifBean *gifBean = static_cast<GifBean *>(gif->UserData);

    width = gif->SWidth;

    height = gif->SHeight;
    frame = &(gif->SavedImages[gifBean->current_frame]);

    frameInfo = &(frame->ImageDesc);

    if (frameInfo->ColorMap) {

        colorMap = frameInfo->ColorMap;

    } else {

        colorMap = gif->SColorMap;

    }


    bg = &colorMap->Colors[gif->SBackGroundColor];


    for (j = 0; j < frame->ExtensionBlockCount; j++) {

        if (frame->ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE) {

            ext = &(frame->ExtensionBlocks[j]);

            break;

        }

    }
    // For dispose = 1, we assume its been drawn
    px = pixels;
    if (ext && dispose(ext) == 1 && force_dispose_1 && gifBean->current_frame > 0) {
        gifBean->current_frame = gifBean->current_frame - 1,
                drawFrame(gif, info, pixels, true);
    } else if (ext && dispose(ext) == 2 && bg) {

        for (y = 0; y < height; y++) {

            line = (int *) px;

            for (x = 0; x < width; x++) {

                line[x] = argb(255, bg->Red, bg->Green, bg->Blue);

            }

            px = (int *) ((char *) px + info.stride);

        }

    } else if (ext && dispose(ext) == 3 && gifBean->current_frame > 1) {
        gifBean->current_frame = gifBean->current_frame - 2,
                drawFrame(gif, info, pixels, true);

    }
    px = pixels;
    if (frameInfo->Interlace) {

        n = 0;

        inc = 8;

        p = 0;

        px = (int *) ((char *) px + info.stride * frameInfo->Top);

        for (y = frameInfo->Top; y < frameInfo->Top + frameInfo->Height; y++) {

            for (x = frameInfo->Left; x < frameInfo->Left + frameInfo->Width; x++) {

                loc = (y - frameInfo->Top) * frameInfo->Width + (x - frameInfo->Left);

                if (ext && frame->RasterBits[loc] == trans_index(ext) && transparency(ext)) {

                    continue;

                }


                color = (ext && frame->RasterBits[loc] == trans_index(ext)) ? bg : &colorMap->Colors[frame->RasterBits[loc]];

                if (color)

                    line[x] = argb(255, color->Red, color->Green, color->Blue);

            }

            px = (int *) ((char *) px + info.stride * inc);

            n += inc;

            if (n >= frameInfo->Height) {

                n = 0;

                switch (p) {

                    case 0:

                        px = (int *) ((char *) pixels + info.stride * (4 + frameInfo->Top));

                        inc = 8;

                        p++;

                        break;

                    case 1:

                        px = (int *) ((char *) pixels + info.stride * (2 + frameInfo->Top));

                        inc = 4;

                        p++;

                        break;

                    case 2:

                        px = (int *) ((char *) pixels + info.stride * (1 + frameInfo->Top));

                        inc = 2;

                        p++;

                }

            }

        }

    } else {

        px = (int *) ((char *) px + info.stride * frameInfo->Top);

        for (y = frameInfo->Top; y < frameInfo->Top + frameInfo->Height; y++) {

            line = (int *) px;

            for (x = frameInfo->Left; x < frameInfo->Left + frameInfo->Width; x++) {

                loc = (y - frameInfo->Top) * frameInfo->Width + (x - frameInfo->Left);

                if (ext && frame->RasterBits[loc] == trans_index(ext) && transparency(ext)) {

                    continue;

                }

                color = (ext && frame->RasterBits[loc] == trans_index(ext)) ? bg : &colorMap->Colors[frame->RasterBits[loc]];

                if (color)

                    line[x] = argb(255, color->Red, color->Green, color->Blue);

            }

            px = (int *) ((char *) px + info.stride);

        }
    }
    GraphicsControlBlock gcb;//获取控制信息
    DGifSavedExtensionToGCB(gif, gifBean->current_frame, &gcb);
    int delay = gcb.DelayTime * 10;
    LOGE("delay %d", delay);
    return delay;
}
/**
 * 绘制 有问题 不兼容
 *
 * @param gifFileType
 * @param info
 * @param pixels
 */
int drawFrame1(GifFileType *gifFileType, AndroidBitmapInfo info, void *pixels){

    GifBean *gifBean = static_cast<GifBean *>(gifFileType->UserData);
    // 拿到当前帧
    SavedImage savedImage = gifFileType->SavedImages[gifBean->current_frame];

    // 图像分成两部分  1，像素，2，描述信息
    //描述信息
    GifImageDesc frameInfo = savedImage.ImageDesc;
    //颜色表
    ColorMapObject *pColorMapObject = frameInfo.ColorMap;
    //像素
    // savedImage.RasterBits;



    //bitmap 像素
    int *px = (int *) pixels;

    //记录每一行的 首地址,临时索引
    int *line;
    // 索引
    int pointPixel;

    //压缩数据
    GifByteType gifByteType;
    //颜色 类型
    GifColorType gifColorType;


// 操作像素  解压
    //Y轴
    for (int y = frameInfo.Top; y < frameInfo.Top+frameInfo.Height; ++y) {
        //每次遍历行，首地址传给 line
        line = px;
        // x 轴
        for (int x = frameInfo.Left; x < frameInfo.Left + frameInfo.Width; ++x) {
            //定位像素
            pointPixel = (y - frameInfo.Top) * frameInfo.Width + (x - frameInfo.Left);
            //压缩数据，光栅位
            gifByteType = savedImage.RasterBits[pointPixel];
            // 颜色信息，RGB
            gifColorType = pColorMapObject->Colors[gifByteType];

            // line 进行复制，0~255，屏幕有颜色，就是bitmap 在底层的表现
            line[x] = argb(255, gifColorType.Red, gifColorType.Green, gifColorType.Blue);

        }
        // 遍历条件， 转到下一行   info.stride 表示一行的字节数 The number of byte per row. = 像素数*4
        px = (int *) ((char *) px + info.stride);

    }

    GraphicsControlBlock gcb;//获取控制信息
    DGifSavedExtensionToGCB(gifFileType, gifBean->current_frame, &gcb);
    int delay = gcb.DelayTime * 10;
    // LOGE();
    LOGE("delay %d", delay);
    return delay;
}






/**
 * 渲染
 */
extern "C"
JNIEXPORT jint JNICALL
Java_com_gyx_mygifframework_GifHandler_updateFrame(JNIEnv *env, jclass clazz, jlong gif_point, jobject bitmap) {

    //将 gif_point 强转成 GifFileType；
    //第一种获取宽高方法
    GifFileType *gifFileType = reinterpret_cast<GifFileType *>(gif_point);
    // int width = gifFileType->SWidth;
    // int height = gifFileType->SHeight;

    //另一种方式，根据bitmap 获取

    AndroidBitmapInfo info;

    AndroidBitmap_getInfo(env, bitmap, &info);


    int width = info.width;
    int height = info.height;

    //bitmap  数组

    void *pixels;

    //将 bitmap 转换为 像素二维数组，--->锁住当前 bitmap
    AndroidBitmap_lockPixels(env, bitmap, &pixels);

    // 绘制
    // drawFrame1(gifFileType, info, pixels);
    int delay = drawFrame(gifFileType, info, pixels, false);

    //释放
    AndroidBitmap_unlockPixels(env, bitmap);

    //当前帧++
    GifBean *gifBean = static_cast<GifBean *>(gifFileType->UserData);
    gifBean->current_frame++;
    // 当前帧数 大于 总帧数 ，重置为0
    if (gifBean->current_frame >= gifBean->total_frame - 1) {
        gifBean->current_frame = 0;
    }


    return delay;
}































