package com.howell.ecameraap;

import com.example.com.howell.ecameraap.R;
import com.howell.play.YV12Renderer;

import android.app.Activity;
import android.content.Intent;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.KeyEvent;

/**
 * @author 霍之昊 
 *
 * 类说明
 */
public class LocalFilePlayer extends Activity {
	
	private GLSurfaceView surfaceview;
	
	private native void displayLocalFile(String fileName);
	private native void localFileQuit();
	
	static {
		System.loadLibrary("hwplay");
		System.loadLibrary("ffmpeg");
        System.loadLibrary("player_jni");
    }
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.local_file_player);
		
		surfaceview = (GLSurfaceView)findViewById(R.id.local_file_surface_view);
		surfaceview.setEGLContextClientVersion(2);
		surfaceview.setRenderer(new YV12Renderer(this,surfaceview));
//		surfaceview.getHolder().addCallback((Callback) this);
		surfaceview.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
		
		Intent intent = getIntent();
		String path = intent.getStringExtra("path");
		displayLocalFile(path);
	}
	
	@Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        super.onKeyDown(keyCode, event);
        if (keyCode == KeyEvent.KEYCODE_BACK && event.getRepeatCount() == 0) {
        	localFileQuit();
        	finish();
        }
        return false;
    }
	
}
