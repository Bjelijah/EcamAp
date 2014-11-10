package com.howell.ecameraap;

import com.howell.play.YV12Renderer;

import android.app.Activity;
import android.os.AsyncTask;

public class MyTask extends AsyncTask<Void, Integer, Void> {

	@Override
	protected Void doInBackground(Void... params) {
		// TODO Auto-generated method stub
		System.out.println("call doInBackground");
        try{
        	//HWCameraActivity.stopRecord();
        	HWCameraActivity.audioStop();
	        HWCameraActivity.quit();
	        HWCameraActivity.audioRelease();
	        YV12Renderer.nativeDeinit();
	        ((Activity) HWCameraActivity.getContext()).finish();
        }catch (Exception e) {
				// TODO: handle exception
		}
        return null;
	}

}
