package com.howell.ecameraap;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Date;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.opengl.GLSurfaceView;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.GestureDetector;
import android.view.GestureDetector.OnGestureListener;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.view.WindowManager;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;

import com.example.com.howell.ecameraap.R;
import com.howell.play.YV12Renderer;
import com.howell.utils.Utils;

@SuppressLint("HandlerLeak")
public class HWCameraActivity extends Activity implements Callback, OnGestureListener, OnTouchListener{
    /** Called when the activity is first created. */
	private GLSurfaceView mGlView;
	private static HWCameraActivity mPlayer;
	
	private int backCount;
	private boolean mPausing;
	
	private static AudioTrack mAudioTrack;
	private byte[] mAudioData;
	private int mAudioDataLength;
	private int isPlayBack;
	private SeekBar replaySeekBar;
	private ImageButton vedioList,circle,quality,sound,catch_picture;
	private TextView streamLen;
	private LinearLayout surfaceControl;
	
	private boolean b_quality,isAudioOpen,bPause;
	
	public  AudioManager audiomanage;  
	private int maxVolume;
	private MyHandler handler;
	private static final int SHOWSTREAMLEN = 1;
	private static final int SEEKBARCHANGE = 2;
	
	public static int fileListHandle = -2;
	//public static boolean finishPlay;
	private ReplayFile replayfile;
	private String ip;
	private int replayTotalTime;
	
	private boolean changeReplayPosition;//标志位用于控制拖动seekbar时 防止seekbar再移动
    private boolean isFirstFrame;
	private long firstFrameTime,endFrameTime;
	private boolean isSufaceControlShown;
	
	private GestureDetector mGestureDetector;
	
	static {
		System.loadLibrary("hwplay");
        System.loadLibrary("player_jni");
    }
	
	public HWCameraActivity() {
		// TODO Auto-generated constructor stub
        mGestureDetector = new GestureDetector(this);   
    } 
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.glsurface);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        System.out.println("onCreate:"+this.toString());
        init();
		audioInit();
		audiomanage = (AudioManager)getSystemService(Context.AUDIO_SERVICE); 
	    maxVolume = audiomanage.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
	    int ret;
	    if(isPlayBack == 0){
	    	ret = display(isPlayBack,(short)0,(short)0,(short)0,(short)0,(short)0,(short)0,(short)0,(short)0,(short)0,(short)0,(short)0,(short)0);
	    }else{
	    	ret = display(isPlayBack, replayfile.begYear, replayfile.begMonth, replayfile.begDay, replayfile.begHour, replayfile.begMinute, replayfile.begSecond
	    			, replayfile.endYear, replayfile.endMonth, replayfile.endDay, replayfile.endHour, replayfile.endMinute, replayfile.endSecond);
	    }
	    if(ret < 0){//playhandle < 0
	    	displayError();
	    }
        handler.sendEmptyMessageDelayed(SHOWSTREAMLEN,5000);
    }
    
    private void init(){
    	mGlView = (GLSurfaceView)findViewById(R.id.glsurface_view);
 		mGlView.setEGLContextClientVersion(2);
 		mGlView.setRenderer(new YV12Renderer(this,mGlView));
 		mGlView.getHolder().addCallback((Callback) this);
 		mGlView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
 		
 		mGlView.setOnTouchListener(this);   
		mGlView.setFocusable(true);   
		mGlView.setClickable(true);   
		mGlView.setLongClickable(true);   
        mGestureDetector.setIsLongpressEnabled(true);  
 		
 		System.out.println("firstFrameTime:"+firstFrameTime+"endFrameTime:"+endFrameTime);
 		Intent intent = getIntent();
 		backCount = 0;
 		mPlayer = this;
 		mPausing = false;
 		isAudioOpen = true;
// 		finishPlay = false;
 		//fileListHandle = -2;
 		isFirstFrame = true;
 		isSufaceControlShown = true;
 		
 		isPlayBack = intent.getIntExtra("playback", 0);
 		replaySeekBar = (SeekBar)findViewById(R.id.replaySeekBar);
 		//pause = (ImageButton)findViewById(R.id.ib_pause);
 		vedioList = (ImageButton)findViewById(R.id.vedio_list);
 		circle = (ImageButton)findViewById(R.id.camera_circle);
 		quality = (ImageButton)findViewById(R.id.quality);
 		sound = (ImageButton)findViewById(R.id.sound);
 		catch_picture = (ImageButton)findViewById(R.id.catch_picture);
 		streamLen = (TextView)findViewById(R.id.tv_stream_len);
 		surfaceControl = (LinearLayout)findViewById(R.id.surface_icons);
 		handler = new MyHandler();
 		
 		if(isPlayBack == 0){
 			replaySeekBar.setVisibility(View.GONE);
 	 		//pause.setVisibility(View.GONE);
 	 		vedioList.setVisibility(View.VISIBLE);
 	 		ip = intent.getStringExtra("ip");
 		}else{
 			System.out.println("11111111111");
 			replaySeekBar.setVisibility(View.VISIBLE);
 	 		//pause.setVisibility(View.VISIBLE);
 	 		vedioList.setVisibility(View.GONE);
 	 		System.out.println("22222222222");
 	 		replayfile = (ReplayFile) intent.getSerializableExtra("replayfile");
 	 		final Date beg = new Date(replayfile.begYear - 1900, replayfile.begMonth - 1, replayfile.begDay, replayfile.begHour, replayfile.begMinute, replayfile.begSecond);
 			Date end = new Date(replayfile.endYear - 1900, replayfile.endMonth - 1, replayfile.endDay, replayfile.endHour, replayfile.endMinute, replayfile.endSecond);
 			replayTotalTime = (int)(end.getTime() - beg.getTime());
 	 		System.out.println("progress max:"+replayTotalTime+" beg.getTime():"+beg.getTime());
 	 		replaySeekBar.setMax(replayTotalTime);
 	 		handler.sendEmptyMessage(SEEKBARCHANGE);
 	 		replaySeekBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
				
				@Override
				public void onStopTrackingTouch(SeekBar arg0) {
					// TODO Auto-generated method stub
					int progress = replaySeekBar.getProgress();
					System.out.println("progress:"+progress+"beg:"+beg.getTime());
					//long startTime = beg.getTime();
					long replayStartTime = beg.getTime() + progress;
					//if(replayStartTime < startTime){
					//	replayStartTime = startTime;
					//}
					SimpleDateFormat sdf= new SimpleDateFormat("MM/dd/yyyy HH:mm:ss");
			    	java.util.Date dt = new Date(replayStartTime); 
			    	String sDateTime = sdf.format(dt);
			    	System.out.println(sDateTime);
			    	System.out.println("String Date: year:"+(dt.getYear()+1900)+"month:"+(dt.getMonth()+1) + "day:"+dt.getDate()+"hour:"+dt.getHours()+"min:"+dt.getMinutes()+"sec:"+dt.getSeconds());
//					long replayStartTime = correctedStartTime + (long)progress/1000;
			    	playBackPositionChange((short)(dt.getYear()+1900), (short)(dt.getMonth()+1), (short)dt.getDate(), (short)dt.getHours(), (short)dt.getMinutes(), (short)dt.getSeconds()
			    			, replayfile.endYear, replayfile.endMonth, replayfile.endDay, replayfile.endHour, replayfile.endMinute, replayfile.endSecond);
			    	changeReplayPosition = false;
			    	handler.sendEmptyMessage(SEEKBARCHANGE);
				}
				
				@Override
				public void onStartTrackingTouch(SeekBar arg0) {
					// TODO Auto-generated method stub
					int progress = replaySeekBar.getProgress();
					changeReplayPosition = true;
					System.out.println("progress start:"+progress);
				}
				
				@Override
				public void onProgressChanged(SeekBar arg0, int arg1, boolean arg2) {
					// TODO Auto-generated method stub
					
				}
			});
 	 		System.out.println("replayfile:"+replayfile.toString() + "replayTotalTime:" + replayTotalTime);
 		}
 		
 		quality.setOnClickListener(listener);
 		catch_picture.setOnClickListener(listener);
 		circle.setOnClickListener(listener);
 		sound.setOnClickListener(listener);
 		vedioList.setOnClickListener(listener);
 		//pause.setOnClickListener(listener);
    }
    
    @SuppressLint("SdCardPath")
	private OnClickListener listener = new OnClickListener() {
		
		@Override
		public void onClick(View view) {
			// TODO Auto-generated method stub
			switch (view.getId()) {
			case R.id.quality:
				if(!b_quality){
					quality.setImageDrawable(getResources().getDrawable(R.drawable.quality720p));
					b_quality = true;
					changeTo720P();
				}else{
					quality.setImageDrawable(getResources().getDrawable(R.drawable.gaoqing));
					b_quality = false;
					changeToD1();
				}
				break;
			case R.id.catch_picture:
				if(!existSDCard()){
					Utils.postToast(getApplicationContext(), getResources().getString(R.string.no_sdcard),2000);
					return;
				}
				File destDir = new File("/sdcard/eCamera_Ap");
				if (!destDir.exists()) {
					System.out.println("File not exists");
					destDir.mkdirs();
				}
				String path = "/sdcard/eCamera_Ap/"+Utils.getFileName()+".jpg";
				catchPicture(path);
				Utils.postToast(getApplicationContext(), getResources().getString(R.string.save_picture),2000);
				break;
			case R.id.camera_circle:
				setFlip();
				break;
			case R.id.sound:
				if(isAudioOpen){
					audioPause();
					
				}else {
					audioPlay();
//					mSound.setImageDrawable(getResources().getDrawable(R.drawable.sound));
				}
				break;
			case R.id.vedio_list:
				Intent intent = new Intent(HWCameraActivity.this,VedioList.class);
				intent.putExtra("ip", ip);
				startActivity(intent);
				QuitToVedioListTask task = new QuitToVedioListTask();
				task.execute();
				break;
//			case R.id.ib_pause:
//				if(!bPause){
//					pause.setImageResource(R.drawable.play);
//					playbackPause(bPause);
//					bPause = true;
//				}else{
//					pause.setImageResource(R.drawable.pause);
//					playbackPause(!bPause);
//					bPause = false;
//				}
//				break;
			default:
				break;
			}
		}
	};
	
	public class QuitToVedioListTask extends AsyncTask<Void, Integer, Void> {

		@Override
		protected Void doInBackground(Void... params) {
			// TODO Auto-generated method stub
			System.out.println("call doInBackground");
	        try{
	        	//HWCameraActivity.stopRecord();
//	        	finishPlay = true;
	        	audioStop();
	        	fileListHandle = getReplayListCount();
	        	handler.setHandlerWork(false);
		        quit();
		        audioRelease();
		        YV12Renderer.nativeDeinit();
		        finish();
	        }catch (Exception e) {
					// TODO: handle exception
			}
	        return null;
		}

	}
	
	class MyHandler extends Handler{
		private int oldStream;
		private boolean handlerWork;
		
		public MyHandler() {
			super();
			oldStream = 0;
			handlerWork = true;
		}
		public void setHandlerWork(boolean handlerWork) {
			this.handlerWork = handlerWork;
		}
		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			switch (msg.what) {
			case SHOWSTREAMLEN:
				if(!handlerWork){
					return;
				}
				int nowStream = (getStreamLen()/1024*8)/5;
				Log.e("SHOWSTREAMLEN", nowStream - oldStream+"");
				streamLen.setText(nowStream - oldStream + " Kibt/s");
				oldStream = nowStream;
				sendEmptyMessageDelayed(SHOWSTREAMLEN, 5000);
				break;
			case SEEKBARCHANGE:
				if(!handlerWork || changeReplayPosition){
					return;
				}
				if(YV12Renderer.time != 0 && isFirstFrame){
					firstFrameTime = YV12Renderer.time;
					isFirstFrame = false;
				}else if(YV12Renderer.time != 0 && !isFirstFrame){
					
					endFrameTime = YV12Renderer.time;
//					System.out.println("progress firstFrameTime:"+firstFrameTime+" endFrameTime:"+endFrameTime);
//					if(!handlerWork){
//						mPlayerHandler.sendEmptyMessage(HIDEPROGRESSBAR);
//						progressHasStop = true;
//					}
					replaySeekBar.setProgress((int)(endFrameTime - firstFrameTime));
//					System.out.println("progress:"+(int)(endFrameTime - firstFrameTime));
				}
				handler.sendEmptyMessageDelayed(SEEKBARCHANGE,100);
				break;

			default:
				break;
			}
		}
	}
	
	private void audioPause(){
//		mAudioTrack.flush();
//		mAudioTrack.pause();
		audiomanage.setStreamVolume(AudioManager.STREAM_MUSIC, 0 , 0);
		isAudioOpen = false;
		sound.setImageDrawable(getResources().getDrawable(R.drawable.no_sound));
	}
	
	private void audioPlay(){
		//mAudioTrack.play();
		audiomanage.setStreamVolume(AudioManager.STREAM_MUSIC, maxVolume/2 , 0);
		isAudioOpen = true;
		sound.setImageDrawable(getResources().getDrawable(R.drawable.sound));
	}
	
    private boolean existSDCard() {  
    	if (android.os.Environment.getExternalStorageState().equals(  
    		android.os.Environment.MEDIA_MOUNTED)) {  
        	return true;  
        } else  
        	return false;  
    } 
	
    public static Context getContext() {
        return mPlayer;
    }
    
    private native int display(int isPlayBack,short begYear,short begMonth,short begDay,short begHour
	,short begMinute,short begSecond,short endYear,short endMonth,short endDay,short endHour,short endMinute
	,short endSecond);
    public static native void quit();
	public native void nativeAudioInit();
	public static native int cameraLogin(String ip);
	public native void changeToD1();
	public native void changeTo720P();
	public native static int getReplayListCount();
	public native int catchPicture(String path);
	public native int setFlip();
	public native int getStreamLen();
	public native void playbackPause(boolean bPause);
	public native void playBackPositionChange(short begYear,short begMonth,short begDay,short begHour
			,short begMinute,short begSecond,short endYear,short endMonth,short endDay,short endHour,short endMinute
			,short endSecond);
	
	public native int ptzTurnLeft(int slot);
	public native int ptzTurnRight(int slot);
	public native int ptzTurnUp(int slot);
	public native int ptzTurnDown(int slot);
	
	private void displayError(){
		if(!isFinishing())
			Utils.postAlerDialog(this, "连接视频失败，请重新连接");
	}
    
    private void audioInit() {
		// TODO Auto-generated method stub
		int buffer_size = AudioTrack.getMinBufferSize(8000, AudioFormat.CHANNEL_OUT_MONO, AudioFormat.ENCODING_PCM_16BIT);
		mAudioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, 8000, AudioFormat.CHANNEL_OUT_MONO, AudioFormat.ENCODING_PCM_16BIT, buffer_size*8, AudioTrack.MODE_STREAM);
		mAudioData = new byte[buffer_size*8];
		
		nativeAudioInit();
		
		//Log.d("play","audio buffer size"+buffer_size);
		mAudioTrack.play();
	}
	
	public static void audioStop(){
		//mAudioTrack.flush();
		//mAudioTrack.pause();
		mAudioTrack.stop();
	}
	
	public static void audioRelease(){
		mAudioTrack.release();
	}

	public void audioWrite() {
//		Log.d("audio","audio data len: "+mAudioDataLength);
//		for (int i=0; i<10; i++) {
//			Log.d("audio","data "+i+" is "+mAudioData[i]);
//		}
		mAudioTrack.write(mAudioData,0,mAudioDataLength);
	}
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		super.onKeyDown(keyCode, event);
		if (keyCode == KeyEvent.KEYCODE_BACK) {
			Log.e("backCount", "backCount:"+backCount);
			if (backCount == 0) {
//				finishPlay = true;
				handler.setHandlerWork(false);
				MyTask mTask = new MyTask();
				mTask.execute();
			}
			System.out.println(backCount);
			backCount++;
		}
		return false;
	}
	
	@Override
	protected void onPause() {
		Log.e("", "onPause");
		mPausing = true;
		this.mGlView.onPause();
		super.onPause();
		
		//finish();
	}

	@Override
	protected void onDestroy() {
		Log.e("", "onDestroy");
		super.onDestroy();
		System.runFinalization();
	}

	@Override
	protected void onResume() {
		Log.e("", "onResume");
		mPausing = false;
		mGlView.onResume();
		super.onResume();
	}
    
	@Override
	protected void onRestart() {
		// TODO Auto-generated method stub
		super.onRestart();
		/*System.out.println("onRestart"+this.toString());
		backCount = 0;
 		mPlayer = this;
 		//mPausing = false;
 		isAudioOpen = true;
 		fileListHandle = -2;
 		isPlayBack = 0;
 		replaySeekBar.setVisibility(View.GONE);
	 	pause.setVisibility(View.GONE);
	 	vedioList.setVisibility(View.VISIBLE);
	 	System.out.println("ip:"+ip);
	 	
		audioInit();
		cameraLogin(ip);
		//audiomanage = (AudioManager)getSystemService(Context.AUDIO_SERVICE); 
	    //maxVolume = audiomanage.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
	    display(isPlayBack,(short)0,(short)0,(short)0,(short)0,(short)0,(short)0,(short)0,(short)0,(short)0,(short)0,(short)0,(short)0);

	    handler = new MyHandler();
        handler.sendEmptyMessageDelayed(SHOWSTREAMLEN,5000);*/
	}
	
	@Override
	 public void onConfigurationChanged(Configuration newConfig) {
		super.onConfigurationChanged(newConfig);
		Log.e("main","config change");
		if (this.getResources().getConfiguration().orientation == Configuration.ORIENTATION_LANDSCAPE){
			Log.i("info", "onConfigurationChanged landscape"); 
			surfaceControl.setVisibility(View.GONE);
			isSufaceControlShown = false;
		} else if(this.getResources().getConfiguration().orientation == Configuration.ORIENTATION_PORTRAIT){
			Log.i("info", "onConfigurationChanged PORTRAIT"); 
			surfaceControl.setVisibility(View.VISIBLE);
			isSufaceControlShown = true;
		}
	}
	
	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width,
			int height) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public boolean onDown(MotionEvent e) {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX,
			float velocityY) {
		// TODO Auto-generated method stub
		System.out.println("onFling");
		final int FLING_MIN_DISTANCE = 50, FLING_MIN_VELOCITY = 100;   
        if (e1.getX() - e2.getX() > FLING_MIN_DISTANCE && Math.abs(velocityX) > FLING_MIN_VELOCITY) {   
            // Fling left   
        	Log.e("MyGesture", "Fling left "+"x:"+Math.abs(e1.getX() - e2.getX())+"y:"+Math.abs(e1.getY() - e2.getY()));  
        	ptzTurnLeft(0);
        } else if (e2.getX() - e1.getX() > FLING_MIN_DISTANCE && Math.abs(velocityX) > FLING_MIN_VELOCITY) {   
            // Fling right   
        	ptzTurnRight(0);
        }  else if (e2.getY() - e1.getY() > FLING_MIN_DISTANCE && Math.abs(velocityY) > FLING_MIN_VELOCITY) {   
            // Fling Down   
        	Log.e("MyGesture", "Fling Down "+"y:"+Math.abs(e1.getY() - e2.getY())+"x:"+Math.abs(e1.getX() - e2.getX()));   
        	ptzTurnDown(0);
        }   else if (e1.getY() - e2.getY() > FLING_MIN_DISTANCE && Math.abs(velocityY) > FLING_MIN_VELOCITY) {   
            // Fling Up   
        	Log.e("MyGesture", "Fling Up "+"y:"+Math.abs(e1.getY() - e2.getY())+"x:"+Math.abs(e1.getX() - e2.getX()));   
        	ptzTurnUp(0);
        }   else{
        	return true;
        }
		return false;
	}

	@Override
	public void onLongPress(MotionEvent e) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX,
			float distanceY) {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public void onShowPress(MotionEvent e) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public boolean onSingleTapUp(MotionEvent e) {
		// TODO Auto-generated method stub
		if(PhoneConfig.getPhoneHeight(this) < PhoneConfig.getPhoneWidth(this)){
			if(isSufaceControlShown){
				surfaceControl.setVisibility(View.GONE);
				isSufaceControlShown = false;
			}else{
				surfaceControl.setVisibility(View.VISIBLE);
				isSufaceControlShown = true;
			}
		}
		return false;
	}

	@Override
	public boolean onTouch(View v, MotionEvent event) {
		// TODO Auto-generated method stub
		return mGestureDetector.onTouchEvent(event);  
	}
}