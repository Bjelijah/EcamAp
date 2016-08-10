package com.howell.jni;

import com.howell.ecameraap.Pagination;
import com.howell.ecameraap.ReplayFile;

public class JniUtil {
	
	
	/*
	 * yuv
	 */
	public static native void nativeInit(Object callbackObj);
	public static native void nativeDeinit();
	public static native void nativeRenderY();
	public static native void nativeRenderU();
	public static native void nativeRenderV();
	public static native void nativeOnSurfaceCreated();
	
	/*
	 * audio
	 */
	public static native void nativeAudioInit(Object callbackObj);
	public static native void nativeAudioDeinit();
	public static native void nativeAudioStop();
	
	/*
	 * download
	 */
	
    public static native int downloadInitEx(Object callbakcObj,String fileName,int slot,short begYear,short begMonth,short begDay,short begHour
    		,short begMinute,short begSecond,short endYear,short endMonth,short endDay,short endHour,short endMinute
    		,short endSecond);
    public static native void downloadDestoryEx();
    
    public static native int downloadInit(Object callbackObj,int userHandle,String fileName,int slot,short begYear,short begMonth,short begDay,short begHour
    		,short begMinute,short begSecond,short endYear,short endMonth,short endDay,short endHour,short endMinute
    		,short endSecond);
    
    public static native void downloadDestory();
    
    
    /*
     * net play
     */
    public static native int cameraLogin(String ip);
    public static native int display(int slot,int isPlayBack,short begYear,short begMonth,short begDay,short begHour
	,short begMinute,short begSecond,short endYear,short endMonth,short endDay,short endHour,short endMinute
	,short endSecond);
    public static native void quit();
    
    
    public static native void changeToD1();
	public static native void changeTo720P();
	public static native int getReplayListCount();
	public static native int catchPicture(String path);
	public static native int setFlip();
	public static native int getStreamLen();
	public static native void playbackPause(boolean bPause);
	public static native void playBackPositionChange(short begYear,short begMonth,short begDay,short begHour
			,short begMinute,short begSecond,short endYear,short endMonth,short endDay,short endHour,short endMinute
			,short endSecond);
	
	public static native void ptzTurnLeft(int slot);
	public static native void ptzTurnRight(int slot);
	public static native void ptzTurnUp(int slot);
	public static native void ptzTurnDown(int slot);
	public static native void zoomTele(int slot);
	public static native void zoomWide(int slot);
    
	
	/*
	 * setting
	 */
	public static native int getWifi(String ip);
	public static native int setWifi(String ip,String ssid,String password);
	
	/*
	 * list
	 */
	public static native ReplayFile[] getReplayList(int file_list_handle,int fileCount);
	public static native void closeFileList(int file_list_handle);
	public static native int getListByPage(int user_handle,int slot,int stream,ReplayFile replay,int type,int order_by_time,Pagination page_info);
	public static native int vedioListLogin(String ip);
	public static native int vedioListLogout(int user_handle);
	
	/*
	 * local play
	 */
	public static native void displayLocalFile(String fileName);
	public static native void localFileQuit();
	
}
