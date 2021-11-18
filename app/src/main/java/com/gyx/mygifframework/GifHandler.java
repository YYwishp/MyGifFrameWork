package com.gyx.mygifframework;

import android.graphics.Bitmap;

public class GifHandler {

	//这里就指代的是C 中的 GifFileType 指针
	long gifHander; //地址，指针类型，大小一样的,纽带



	static{

		//System.loadLibrary("native-lib");
		System.loadLibrary("mygifframework");

	}

	private GifHandler(long gifHander) {
		this.gifHander = gifHander;
	}

	public static GifHandler load(String path) {
		long gifHander = loadGif(path);
		GifHandler gifHandler = new GifHandler(gifHander);
		return gifHandler;
	}




//	开始 加载 gif 文件
//	jni 方法； Java+包名+类名+方法名  中间分隔用下划线
//参数   第一个 JNIEnv；  第二个：如果是static 传递class，如果是 对象 传递 object；第三个参数才是需要   传递的参数
	public static native long loadGif(String path);



	public int getWidth() {
		return getWidth(gifHander);
	}

	public int getHeight() {
		return getHeight(gifHander);
	}

	/**
	 * 下一帧返回的时间
	 *
	 * @param bitmap
	 * @return 下一帧返回的时间
	 */
	public int updateFrame(Bitmap bitmap) {
		return updateFrame(gifHander,bitmap);
	}


	//	宽
	public static native int getWidth(long gifHander);

	/**
	 *  高
	 *
	 * @param gifPoint 指针
	 * @return
	 */
	public static native int getHeight(long gifPoint);

	/**
	 * 渲染图片
	 *
	 * @param gifPoint 指针
	 * @param bitmap
	 * @return 下一帧返回的时间
	 */
	public static native int updateFrame(long gifPoint, Bitmap bitmap);

}


































