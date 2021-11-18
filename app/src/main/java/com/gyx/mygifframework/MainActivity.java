package com.gyx.mygifframework;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import android.app.Activity;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.gyx.mygifframework.databinding.ActivityMainBinding;

import java.io.File;

import static android.graphics.Bitmap.Config.ARGB_8888;

public class MainActivity extends AppCompatActivity {


	private ActivityMainBinding binding;
	Bitmap bitmap;
	GifHandler gifHandler;
	//ImageView imageView;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		binding = ActivityMainBinding.inflate(getLayoutInflater());
		setContentView(binding.getRoot());

		verifyStoragePermissions(this);

		//TextView tv = binding.sampleText;
		//tv.setText(stringFromJNI());
	}

	private void verifyStoragePermissions(Activity activity) {
		int REQUEST_EXTERNAL_STORAGE = 1;
		String[] PERMISSIONS_STORAGE = {
				"android.permission.READ_EXTERNAL_STORAGE",
				"android.permission.WRITE_EXTERNAL_STORAGE" };
		try {
			//检测是否有写的权限
			int permission = ActivityCompat.checkSelfPermission(activity,
					"android.permission.WRITE_EXTERNAL_STORAGE");
			if (permission != PackageManager.PERMISSION_GRANTED) {
				// 没有写的权限，去申请写的权限，会弹出对话框
				ActivityCompat.requestPermissions(activity, PERMISSIONS_STORAGE,REQUEST_EXTERNAL_STORAGE);
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	Handler myHandler = new Handler() {
		@Override
		public void handleMessage(@NonNull Message msg) {
			super.handleMessage(msg);
			int delay = gifHandler.updateFrame(bitmap);
			myHandler.sendEmptyMessageDelayed(1, delay);
			binding.image.setImageBitmap(bitmap);
		}
	};



	public void ndkLoadGif(View view) {
		File file = new File(Environment.getExternalStorageDirectory(), "demo2.gif");
		gifHandler = GifHandler.load(file.getAbsolutePath());
		int width = gifHandler.getWidth();
		int height = gifHandler.getHeight();
		Log.e("gyx", "宽" + width + "  高 " + height);
		bitmap = Bitmap.createBitmap(width, height, ARGB_8888);
		//通知 C 渲染完成
		int delay = gifHandler.updateFrame(bitmap);

		binding.image.setImageBitmap(bitmap);
		myHandler.sendEmptyMessageDelayed(1, delay);
	}
}