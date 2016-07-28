package com.howell.ecameraap.downloadfile;

import java.io.File;
import java.text.DecimalFormat;

import android.content.Context;
import android.content.Intent;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.util.SparseArray;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.example.com.howell.ecameraap.R;
import com.howell.ecameraap.HWCameraActivity;
import com.howell.utils.NetWorkUtils;

/**
author:alexzhou 
email :zhoujiangbohai@163.com
date  :2013-1-27

app列表的数据适配器
 **/
public class AppListAdapter extends BaseAdapter {

	private SparseArray<AppFile> dataList = null;
	private LayoutInflater inflater = null;
	private Context mContext;
	private DownloadManager downloadManager;
	private ListView listView;
	private String ip;
	private String ssid;

	public AppListAdapter(Context context, SparseArray<AppFile> dataList) {
		this.mContext = context;
		this.inflater = (LayoutInflater) context
				.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		this.dataList = dataList;
		this.mContext = context;
		this.downloadManager = DownloadManager.getInstance();
		this.downloadManager.setHandler(mHandler);
	}
	
	public AppListAdapter(Context context, SparseArray<AppFile> dataList,String ip) {
		this.mContext = context;
		this.inflater = (LayoutInflater) context
				.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		this.dataList = dataList;
		this.mContext = context;
		this.downloadManager = DownloadManager.getInstance();
		this.downloadManager.setHandler(mHandler);
		this.ip = ip;
		NetWorkUtils utils = new NetWorkUtils(mContext);
		this.ssid = utils.getSSID();
	}
	
	private String removeMarks(String SSID){
		if(SSID.startsWith("\"") && SSID.endsWith("\"")){
			SSID = SSID.substring(1, SSID.length()-1);
		}
		return SSID;
	}

	public void setListView(ListView view)
	{
		this.listView = view;
	}
	
	public void setUserHanle(int userHandle){
		this.downloadManager.setUserHandle(userHandle);
	}
	
	public void setData(SparseArray<AppFile> dataList){
		this.dataList = dataList;
	}
	
	@Override
	public int getCount() {
		return dataList.size();
	}

	@Override
	public Object getItem(int position) {
		return dataList.get(position);
	}

	@Override
	public long getItemId(int position) {
		return position;
	}
	
	// 改变下载按钮的样式
	private void changeBtnStyle(Button btn, boolean enable)
	{
		if(enable)
		{
			btn.setBackgroundResource(R.drawable.btn_download_norm);
		}
		else
		{
			btn.setBackgroundResource(R.drawable.btn_download_disable);
		}
		btn.setEnabled(enable);
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {

		final ViewHolder holder;
		if (null == convertView) {
			holder = new ViewHolder();
			convertView = inflater.inflate(R.layout.video_item, null);
			holder.layout = (LinearLayout) convertView
					.findViewById(R.id.video_item_name_layout);
			//holder.icon = (ImageView) convertView
			//		.findViewById(R.id.app_icon);
			holder.name = (TextView) convertView
					.findViewById(R.id.video_item_name);
			holder.name2 = (TextView) convertView
					.findViewById(R.id.video_item_name2);
			holder.size = (TextView) convertView
					.findViewById(R.id.video_item_download_progress);
			holder.btn = (Button) convertView
					.findViewById(R.id.video_item_download_icon);
			convertView.setTag(holder);
		} else {
			holder = (ViewHolder) convertView.getTag();
		}

		// 这里position和app.id的值是相等的
		final AppFile app = dataList.get(position);
		//Log.e("", "id="+app.id+", name="+app.name);
		holder.name.setText(app.name);
		holder.name2.setText(app.name2);
		if(app.size == 0){
			holder.size.setText("0%");
		}else{
			holder.size.setText(new DecimalFormat("0").format(app.downloadSize * 100 / app.size) + "%");
			//holder.size.setText((app.downloadSize * 100.0f / app.size) + "%");
		}
		//如果文件已存在则按钮状态为已下载
		String fileName = Environment.getExternalStorageDirectory()+"/eCamera_AP/"/*emoveMarks(ssid)+"-"*/+app.replay.begYear+app.replay.begMonth+app.replay.begDay+app.replay.begHour+app.replay.begMinute+app.replay.begSecond+".hwr";
		if(new File(fileName).exists()){
			app.downloadState = DownloadManager.DOWNLOAD_STATE_FINISH;
			holder.size.setText("100%");
		}
		
		switch(app.downloadState)
		{
		case DownloadManager.DOWNLOAD_STATE_NORMAL:
			holder.btn.setText("下载");
			this.changeBtnStyle(holder.btn, true);
			break;
		case DownloadManager.DOWNLOAD_STATE_DOWNLOADING:
			holder.btn.setText("下载中");
			this.changeBtnStyle(holder.btn, false);
			break;
		case DownloadManager.DOWNLOAD_STATE_FINISH:
			holder.btn.setText("已下载");
			this.changeBtnStyle(holder.btn, false);
			break;
		case DownloadManager.DOWNLOAD_STATE_WAITING:
			holder.btn.setText("排队中");
			this.changeBtnStyle(holder.btn, false);
			break;
		}
		holder.btn.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				DownloadFile downloadFile = new DownloadFile();
				downloadFile.downloadID = app.id;
				downloadFile.downloadState = DownloadManager.DOWNLOAD_STATE_WAITING;
				app.downloadState = DownloadManager.DOWNLOAD_STATE_WAITING;
				downloadFile.downloadSize = app.downloadSize;
				downloadFile.totalSize = app.size;
				downloadFile.replayName = Environment.getExternalStorageDirectory()+"/eCamera_AP/"+app.replay.begYear+app.replay.begMonth+app.replay.begDay+app.replay.begHour+app.replay.begMinute+app.replay.begSecond+".hwr";
				downloadFile.replay = app.replay;
				holder.btn.setText("排队中");
			    changeBtnStyle(holder.btn, false);
				downloadManager.startDownload(downloadFile);
			}
		});
		
		holder.layout.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View arg0) {
				// TODO Auto-generated method stub
				if(!downloadManager.isDownloadFilesEmpty()){
					Toast.makeText(mContext, "请耐心等待下载完成", 1000).show();
				}else{
					HWCameraActivity.cameraLogin(ip);
					Intent intent = new Intent(mContext,HWCameraActivity.class);
					intent.putExtra("playback", 1);//鍥炴斁
					intent.putExtra("replayfile", app.replay);
					mContext.startActivity(intent);
				}
			}
		});
		return convertView;
	}

	static class ViewHolder {
		LinearLayout layout;
		//ImageView icon;
		TextView name,name2;
		TextView size;
		Button btn;
	}
	
	
	private Handler mHandler = new Handler() {
		
		public void handleMessage(Message msg)
		{
			DownloadFile downloadFile = (DownloadFile)msg.obj;
			AppFile appFile = dataList.get(downloadFile.downloadID);
			appFile.downloadSize = downloadFile.downloadSize;
			appFile.downloadState = downloadFile.downloadState;
			appFile.size = downloadFile.totalSize;
			Log.e("", appFile.toString());
			// notifyDataSetChanged会执行getView函数，更新所有可视item的数据
			//notifyDataSetChanged();
			// 只更新指定item的数据，提高了性能
			updateView(appFile.id);
		}
	};
	
	// 更新指定item的数据
	private void updateView(int index)
	{
		int visiblePos = listView.getFirstVisiblePosition();
		int offset = index - visiblePos + 1;
		Log.e("", "index="+index+"visiblePos="+visiblePos+"offset="+offset);
		// 只有在可见区域才更新
		if(offset < 0) return;
		
		View view = listView.getChildAt(offset);
		final AppFile app = dataList.get(index);
		ViewHolder holder = (ViewHolder)view.getTag();
		//Log.e("", "id="+app.id+", name="+app.name);

		holder.name.setText(app.name);
		holder.name2.setText(app.name2);
		if(app.size == 0){
			holder.size.setText("0%");
		}else{
			holder.size.setText(new DecimalFormat("0").format(app.downloadSize * 100 / app.size) + "%");
			//holder.size.setText((app.downloadSize * 100.0f / app.size) + "%");
		}
		//Drawable drawable = mContext.getResources().getDrawable(R.drawable.app_icon);
		//holder.icon.setImageDrawable(drawable);

		switch(app.downloadState)
		{
		case DownloadManager.DOWNLOAD_STATE_DOWNLOADING:
			holder.btn.setText("下载中");
			this.changeBtnStyle(holder.btn, false);
			break;
		case DownloadManager.DOWNLOAD_STATE_FINISH:
			holder.btn.setText("已下载");
			this.changeBtnStyle(holder.btn, false);
			break;
		}

	}
}
