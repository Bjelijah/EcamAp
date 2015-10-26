package com.howell.ecameraap.downloadfile;

import java.util.ArrayList;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.util.SparseArray;

/**
author:alexzhou 
email :zhoujiangbohai@163.com
date  :2013-1-27

下载管理
 **/

public class DownloadManager {
	
	// 下载状态：正常，暂停，下载中，已下载，排队中
	public static final int DOWNLOAD_STATE_NORMAL = 0x00;
	public static final int DOWNLOAD_STATE_PAUSE = 0x01;
	public static final int DOWNLOAD_STATE_DOWNLOADING = 0x02;
	public static final int DOWNLOAD_STATE_FINISH = 0x03;
	public static final int DOWNLOAD_STATE_WAITING = 0x04;
	
	// SparseArray是android中替代Hashmap的类,可以提高效率
	private SparseArray<DownloadFile> downloadFiles = new SparseArray<DownloadFile>();
	// 用来管理所有下载任务
	private ArrayList<DownloadTask> taskList = new ArrayList<DownloadTask>();
	private Handler mHandler;
	private final static Object syncObj = new Object();
	private static DownloadManager instance;
	private ExecutorService executorService;
	
	private int dataLen;
	private int userHandle;
	
    public native int downloadInit(int userHandle,String fileName,int slot,short begYear,short begMonth,short begDay,short begHour
    		,short begMinute,short begSecond,short endYear,short endMonth,short endDay,short endHour,short endMinute
    		,short endSecond);
    public native void downloadDestory();
	
	private DownloadManager()
	{
		// 最多只能同时下载3个任务，其余的任务排队等待
		executorService = Executors.newFixedThreadPool(1);
		dataLen = 0;
	}
	
	public static DownloadManager getInstance()
	{
		if(null == instance)
		{
			synchronized(syncObj) {
				instance = new DownloadManager();
			}
			return instance;
		}
		return instance;
	}
	
	public void setHandler(Handler handler) {
		this.mHandler =  handler;
	}

	// 开始下载，创建一个下载线程
	public void startDownload(DownloadFile file) {
		downloadFiles.put(file.downloadID, file);
		DownloadTask task = new DownloadTask(file.downloadID);
		taskList.add(task);
		executorService.submit(task);
	}
	
	public void stopAllDownloadTask() {
		while(taskList.size() != 0)
		{
			DownloadTask task = taskList.remove(0);
			// 可以在这里做其他的处理
			task.stopTask();
		}
	}
	
	public void freeResource(){
		// 会停止正在进行的任务和拒绝接受新的任务
		executorService.shutdownNow();
	}
	
	public void setUserHandle(int userHandle){
		this.userHandle = userHandle;
	}
	
	public boolean isDownloadFilesEmpty(){
		return downloadFiles.size() > 0 ? false : true;
	}
	
	public void refreshDataLen(int len){
		dataLen += len;
		Log.e("refreshDataLen", "len:"+len);
	}
	
	private String removeMarks(String SSID){
		if(SSID.startsWith("\"") && SSID.endsWith("\"")){
			SSID = SSID.substring(1, SSID.length()-1);
		}
		return SSID;
	}

	// 下载任务
	class DownloadTask implements Runnable {

		private boolean isWorking = false;
		private int downloadId;

		public DownloadTask(int id)
		{
			this.isWorking = true;
			this.downloadId = id;
		}
		
		public void stopTask()
		{
			this.isWorking = false;
		}
		
		// 更新listview中对应的item
		public void update(DownloadFile downloadFile)
		{
			Message msg = mHandler.obtainMessage();
			if(downloadFile.totalSize != 0 && downloadFile.totalSize == downloadFile.downloadSize)
				downloadFile.downloadState = DOWNLOAD_STATE_FINISH;
			msg.obj = downloadFile;
			msg.sendToTarget();
			
		}
		
		public void run() {
			// 更新下载文件的状态
			DownloadFile downloadFile = downloadFiles.get(downloadId);
			downloadFile.downloadState = DOWNLOAD_STATE_DOWNLOADING;
			while(isWorking)
			{
				// 检测是否下载完成
				if(downloadFile.downloadState != DOWNLOAD_STATE_DOWNLOADING)
				{
					downloadFiles.remove(downloadFile.downloadID);
					taskList.remove(this);
					isWorking = false;
					break;
				}
				
				if(downloadFile.downloadSize <= downloadFile.totalSize)
				{
					this.update(downloadFile);
				}
				
				//状态改变成下载中
				Log.e("", "downloadSize="+downloadFile.downloadSize+"; size="+downloadFile.totalSize);
				if(downloadFile.totalSize == 0){
					//获取totalsize
					try{
					downloadFile.totalSize = downloadInit(userHandle,downloadFile.replayName,0,downloadFile.replay.begYear,downloadFile.replay.begMonth,downloadFile.replay.begDay
							,downloadFile.replay.begHour,downloadFile.replay.begMinute,downloadFile.replay.begSecond
							,downloadFile.replay.endYear,downloadFile.replay.endMonth,downloadFile.replay.endDay
							,downloadFile.replay.endHour,downloadFile.replay.endMinute,downloadFile.replay.endSecond);
				
					}catch(Exception e){
						dataLen = 0;
						downloadDestory();
					}
				}
				Log.e("", "downloadFile.totalSize:"+downloadFile.totalSize);
				if(downloadFile.totalSize == -1){
					//下载失败
					dataLen = 0;
					downloadDestory();
					return;
				}
				
				//Log.e("", "downloadSize="+downloadFile.downloadSize+"; size="+downloadFile.totalSize);
				// 这里只是模拟了下载，每一秒更新一次item的下载状态
				if(downloadFile.downloadSize < downloadFile.totalSize)
				{
					try {
						downloadFile.downloadSize = dataLen;
						Thread.sleep(500);
					} catch (InterruptedException e) {
						e.printStackTrace();
						downloadFile.downloadState = DOWNLOAD_STATE_PAUSE;
						this.update(downloadFile);
						downloadFiles.remove(downloadId);
						isWorking = false;
						dataLen = 0;
						downloadDestory();
						break;
					}
					
					//++ downloadFile.downloadSize;
				}
			}
			dataLen = 0;
			downloadDestory();
		}
	}

}

