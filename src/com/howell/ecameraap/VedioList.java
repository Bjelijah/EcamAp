package com.howell.ecameraap;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Date;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.DatePickerDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.util.SparseArray;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.LinearInterpolator;
import android.view.animation.RotateAnimation;
import android.widget.BaseAdapter;
import android.widget.DatePicker;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.TextView;

import com.example.com.howell.ecameraap.R;
import com.howell.ecameraap.MyListViewWithFoot.OnRefreshListener;
import com.howell.ecameraap.downloadfile.AppFile;
import com.howell.ecameraap.downloadfile.AppListAdapter;
import com.howell.ecameraap.downloadfile.DownloadManager;
import com.howell.utils.NetWorkUtils;
import com.howell.utils.Utils;

public class VedioList extends Activity implements OnClickListener{
	private ImageButton back,search;
	private MyListViewWithFoot vedioList;
	//private ArrayList<String> mAdapterList;
	private AppListAdapter adapter;
	private String ip;
	
	private ArrayList<ReplayFile> rf;
	private int replayCount ;
	
	private static final int STOPREFRESH = 1;
	private static final int REFRESHFAIL = 2;
	private int userHandle;
	private Pagination pagination;
	
	private ReplayFile queryReplay;
	private Dialog waitDialog;
	
	private boolean isDownload;
	
	static {
		//System.loadLibrary("hwplay");
        System.loadLibrary("player_jni");
    }
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.vedio_list);
		replayCount = 20;
		queryReplay = getEpoch2NowTime();
		//downloadList = new ArrayList<Integer>();
		isDownload = false;
		Intent intent = getIntent();
		ip = intent.getStringExtra("ip");
		back = (ImageButton)findViewById(R.id.ib_vediolist_back);
		back.setOnClickListener(this);
		rf = new ArrayList<ReplayFile>();
		//mAdapterList = new ArrayList<String>();
		adapter = new AppListAdapter(this,new SparseArray<AppFile>(),ip);
		vedioList = (MyListViewWithFoot)findViewById(R.id.mylistview_vedio_list);
		adapter.setListView(vedioList);
		vedioList.setAdapter(adapter);
		vedioList.setonRefreshListener(new OnRefreshListener() {
			
			@Override
			public void onRefresh() {
				// TODO Auto-generated method stub
				rf.clear();
				queryReplay = getEpoch2NowTime();
				pagination = new Pagination(replayCount,0);
				HWCameraActivity.fileListHandle = getListByPage(userHandle,0,1,queryReplay,0,1,pagination);
				Log.e("onFirstRefresh", pagination.toString());
				if(HWCameraActivity.fileListHandle == -1){
					handler.sendEmptyMessage(REFRESHFAIL);
					return;
				}
				
				rf.addAll(Arrays.asList(getReplayList(HWCameraActivity.fileListHandle,replayCount)));
				adapter.setData(initData(rf));
				handler.sendEmptyMessage(STOPREFRESH);
			}
			
			int position = 0;
			@Override
			public void onFootRefresh() {
				// TODO Auto-generated method stub
				new AsyncTask<Void, Void, Void>() {
					protected Void doInBackground(Void... params) {
						position = rf.size();
						if(pagination.page_count <  pagination.page_no + 1){
							return null;
						}
						pagination = new Pagination(replayCount,pagination.page_no + 1);
						HWCameraActivity.fileListHandle = getListByPage(userHandle,0,1,queryReplay,0,1,pagination);
						Log.e("onFootRefresh", pagination.toString());
						rf.addAll(Arrays.asList(getReplayList(HWCameraActivity.fileListHandle,replayCount)));
						adapter.setData(initData(rf));
						return null;
					}

					@Override
					protected void onPostExecute(Void result) {
						if(pagination.page_count <  pagination.page_no + 1){
							Utils.postToast(VedioList.this,"没有更多数据",1000);
						}
						adapter.notifyDataSetChanged();
						vedioList.onFootRefreshComplete();
						vedioList.setSelection(position);
					}

				}.execute();
				
			}
			
			@Override
			public void onFirstRefresh() {
				// TODO Auto-generated method stub
				//HWCameraActivity.fileListHandle = -2;
				userHandle = vedioListLogin(ip);
				if(userHandle != -1){
					pagination = new Pagination(replayCount,0);
					HWCameraActivity.fileListHandle = getListByPage(userHandle,0,1,queryReplay,0,1,pagination);
					Log.e("onFirstRefresh", pagination.toString());
					if(HWCameraActivity.fileListHandle == -1){
						handler.sendEmptyMessage(REFRESHFAIL);
						return;
					}
					
					rf.addAll(Arrays.asList(getReplayList(HWCameraActivity.fileListHandle,replayCount)));
					adapter.setData(initData(rf));
					adapter.setUserHanle(userHandle);
					//					sort(rf);
//					for(ReplayFile r: rf){
//						System.out.println(r.toString());
//					}
					handler.sendEmptyMessage(STOPREFRESH);
				}else{
					handler.sendEmptyMessage(REFRESHFAIL);
				}
			}
		});
		//vedioList.setOnItemClickListener(this);
		
		search = (ImageButton)findViewById(R.id.ib_vediolist_search);
		search.setOnClickListener(this);
	}
	
	@SuppressWarnings("deprecation")
	private SparseArray<AppFile> initData(ArrayList<ReplayFile> rf){
		Date beg,end;
		String s_beg,s_end;
		SparseArray<AppFile> appList = new SparseArray<AppFile>();
		for(int i = 0; i < rf.size(); i++)
		{
			AppFile app = new AppFile();
			beg = new Date(rf.get(i).begYear - 1900, rf.get(i).begMonth - 1, rf.get(i).begDay, rf.get(i).begHour, rf.get(i).begMinute, rf.get(i).begSecond);
			end = new Date(rf.get(i).endYear - 1900, rf.get(i).endMonth - 1, rf.get(i).endDay, rf.get(i).endHour, rf.get(i).endMinute, rf.get(i).endSecond);
	        s_beg = Utils.dateToString(beg);
	        s_end = Utils.dateToString(end);
	        app.replay = rf.get(i);
	        app.name = s_beg + " -> ";
			app.name2 =  s_end;
			app.size = 0;
			app.id = i;
			app.downloadState = DownloadManager.DOWNLOAD_STATE_NORMAL;
			app.downloadSize = 0;
			appList.put(app.id, app);
		}
		return appList;
	}
	
	private ReplayFile getEpoch2NowTime(){
		Calendar calendar = Calendar.getInstance();
		
		ReplayFile r = new ReplayFile((short)1970, (short)1, (short)1, (short)0,(short) 0, (short)0
				, (short)calendar.get(Calendar.YEAR), (short)(calendar.get(Calendar.MONTH) + 1)
				, (short)calendar.get(Calendar.DAY_OF_MONTH), (short)calendar.get(Calendar.HOUR_OF_DAY)
				, (short)calendar.get(Calendar.MINUTE), (short)calendar.get(Calendar.SECOND));
		Log.e("Calendar", r.toString());
		return r;
		
	}
	
	@Override
	protected void onDestroy() {
		super.onDestroy();
		closeFileList(HWCameraActivity.fileListHandle);
		vedioListLogout(userHandle);
		DownloadManager.getInstance().stopAllDownloadTask();
	}
	
	@SuppressLint("HandlerLeak")
	private Handler handler = new Handler(){
		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			switch (msg.what) {
			case STOPREFRESH:
				adapter.notifyDataSetChanged();
				vedioList.onRefreshComplete();
				break;
			case REFRESHFAIL:
				Utils.postFinishActivityAlerDialog(VedioList.this, "连接失败，请重新连接");
				adapter.notifyDataSetChanged();
				vedioList.onRefreshComplete();
				break;
			default:
				break;
			}
		}
	};
	
//	private OnClickListener listener = new OnClickListener() {
//		
//		@Override
//		public void onClick(View view) {
//			// TODO Auto-generated method stub
//			switch (view.getId()) {
//			case R.id.ib_vediolist_back:
//				finish();
//				break;
//
//			default:
//				break;
//			}
//		}
//	};
	
//	private void sort(ReplayFile[] arr){
//		for(int i = 0; i < arr.length / 2; i++) {
//			ReplayFile temp = arr[i];
//		    arr[i] = arr[arr.length - 1 - i];
//		    arr[arr.length - 1 - i] = temp;
//		}
//	}
	
    public class VideoListAdapter extends BaseAdapter {

        private Context mContext;
        private Date beg ,end;
        private String s_beg,s_end;
        private String ssid;
        //private VODRecord record;
        //private ArrayList<VODRecord> mAdapterList;

        public VideoListAdapter(Context context) {
            mContext = context;
            NetWorkUtils utils = new NetWorkUtils(VedioList.this);
			ssid = utils.getSSID();
        }

        @Override
        public int getCount() {
            // TODO Auto-generated method stub
            return rf == null ? 0 : rf.size();
        }

        @Override
        public Object getItem(int position) {
            // TODO Auto-generated method stub
            return rf == null ? null : rf.get(position);
        }

        @Override
        public long getItemId(int position) {
            // TODO Auto-generated method stub
            return position;
        }

		@SuppressWarnings("deprecation")
		@Override
        public View getView(int position, View convertView, ViewGroup parent) {
            // TODO Auto-generated method stub
			ViewHolder holder = null;
	    	if (convertView == null) { 
	    		LayoutInflater layoutInflater = LayoutInflater.from(mContext);
				convertView = layoutInflater.inflate(R.layout.video_item, null);
	    		
	    		holder = new ViewHolder();
	    		holder.name = (TextView) convertView.findViewById(R.id.video_item_name);
	    		holder.progress = (TextView) convertView.findViewById(R.id.video_item_download_progress);
	    		holder.downloadIcon = (ImageView) convertView.findViewById(R.id.video_item_download_icon);
	            convertView.setTag(holder);
	    	}else { 
	    		holder = (ViewHolder)convertView.getTag();
	        } 
	    	
			ReplayFile replayFile = (ReplayFile)getItem(position);
			
			beg = new Date(replayFile.begYear - 1900, replayFile.begMonth - 1, replayFile.begDay, replayFile.begHour, replayFile.begMinute, replayFile.begSecond);
	 		end = new Date(replayFile.endYear - 1900, replayFile.endMonth - 1, replayFile.endDay, replayFile.endHour, replayFile.endMinute, replayFile.endSecond);
	        s_beg = Utils.dateToString(beg);
	        s_end = Utils.dateToString(end);
	        holder.name.setText(s_beg + " --> " + s_end);
	        holder.name.setTag(position);
	        holder.name.setOnClickListener(VedioList.this);
	        holder.downloadIcon.setOnClickListener(VedioList.this);
	        holder.downloadIcon.setTag(position);
	        String fileName = Environment.getExternalStorageDirectory()+"/eCamera_AP/"+removeMarks(ssid)+"-"+replayFile.begYear+replayFile.begMonth+replayFile.begDay+replayFile.begHour+replayFile.begMinute+replayFile.begSecond+".hwr";
//	        if(new File(fileName).exists()){
//	        	holder.progress.setText("瀹屾垚");
//	 	        holder.downloadIcon.setImageDrawable(getResources().getDrawable(R.drawable.delete));
//	        }else{
//	        	Log.e("", "position:"+position);
//		        holder.progress.setText("");
//		        holder.downloadIcon.setImageDrawable(getResources().getDrawable(R.drawable.dowload));
//		        for(Integer index : downloadList){
//		        	Log.e("", "downloadindex:"+index+",position:"+position);
//		        	if(position == index){
//		        		holder.downloadIcon.setImageDrawable(getResources().getDrawable(R.drawable.progress_blue));
//		        		holder.progress.setText("0%");
//		        		break;
//		        	}
//		        }
//	        }
            return convertView;
        }
		
		class ViewHolder{
			public TextView name;
			public TextView progress;
			public ImageView downloadIcon;
		}
    }
    
	private void startRotateAnimation(View v){
		RotateAnimation animation = new RotateAnimation(0f,360f,Animation.RELATIVE_TO_SELF,0.5f,Animation.RELATIVE_TO_SELF,0.5f);
		LinearInterpolator lir = new LinearInterpolator();
		animation.setInterpolator(lir);
		animation.setDuration(1000);
		animation.setFillAfter(true);
		animation.setRepeatCount(-1);
		v.startAnimation(animation);
	}
	
	private void finishRotateAnimation(View v){
		v.clearAnimation();
	}

	private String removeMarks(String SSID){
		if(SSID.startsWith("\"") && SSID.endsWith("\"")){
			SSID = SSID.substring(1, SSID.length()-1);
		}
		return SSID;
	}
	
	//int dataLen;
//	public void refreshDataLen(int len){
//		//dataLen += len;
////		System.out.println("a-7 dataLen:"+dataLen);
//	}
    
//    public native int downloadInit(String fileName,int slot,short begYear,short begMonth,short begDay,short begHour
//    		,short begMinute,short begSecond,short endYear,short endMonth,short endDay,short endHour,short endMinute
//    		,short endSecond);
//    public native void downloadDestory();
	public native ReplayFile[] getReplayList(int file_list_handle,int fileCount);
	public native void closeFileList(int file_list_handle);
	public native int getListByPage(int user_handle,int slot,int stream,ReplayFile replay,int type,int order_by_time,Pagination page_info);
	public native int vedioListLogin(String ip);
	public native int vedioListLogout(int user_handle);
	
	/*@Override
	public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
		// TODO Auto-generated method stub
		HWCameraActivity.cameraLogin(ip);
		Intent intent = new Intent(VedioList.this,HWCameraActivity.class);
		intent.putExtra("playback", 1);//鍥炴斁
		intent.putExtra("replayfile", rf.get((int)arg3));
		startActivity(intent);
	}*/
	
	public class RefreshDateTask extends AsyncTask<Void, Integer, Void> {
        @Override
        protected Void doInBackground(Void... params) {
            // TODO Auto-generated method stub
        	rf.clear();
        	pagination = new Pagination(replayCount,0);
			HWCameraActivity.fileListHandle = getListByPage(userHandle,0,1,queryReplay,0,1,pagination);
			if(HWCameraActivity.fileListHandle == -1){
				handler.sendEmptyMessage(REFRESHFAIL);
				return null;
			}
			rf.addAll(Arrays.asList(getReplayList(HWCameraActivity.fileListHandle,replayCount)));
			handler.sendEmptyMessage(STOPREFRESH);
            return null;
        }
        
        @Override
        protected void onPostExecute(Void result) {
        	// TODO Auto-generated method stub
        	super.onPostExecute(result);
        	waitDialog.dismiss();
        }
    }
	
	/*class DownloadTask extends AsyncTask<Void, Integer, Void>{
		
		private DownloadProgressManager dm;
		private TextView downloadProgress;
		private ImageView downloadIcon;

		public DownloadTask(TextView downloadProgress,
				ImageView downloadIcon) {
			super();
			this.downloadProgress = downloadProgress;
			this.downloadIcon = downloadIcon;
		}
		
		@Override
		protected Void doInBackground(Void... arg0) {
			// TODO Auto-generated method stub
			try{
				dm = new DownloadProgressManager(100);
				Thread.sleep(1000);
				publishProgress(dm.getTotalLen() * 100);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			return null;
		}
		
		@Override
		protected void onProgressUpdate(Integer... progress) {//鍦ㄨ皟鐢╬ublishProgress涔嬪悗琚皟鐢紝鍦╱i绾跨▼鎵ц  

			downloadProgress.setText(progress[0]/dm.getTotalLen()+"%");
        }  
		
		protected void onPostExecute(Void result) {
			Log.e("", "download finish");
			downloadList.remove(0);
			finishRotateAnimation(downloadIcon);
			downloadIcon.setImageDrawable(getResources().getDrawable(R.drawable.dowload));
			if(downloadList.size() > 0){
				int index = downloadList.get(0);
				View view = getItemView(index);
				if(view != null && view.isShown()){
			        ViewHolder holder = (ViewHolder) view.getTag();  
			        holder.progress = (TextView) view.findViewById(R.id.video_item_download_progress);
			    	holder.downloadIcon = (ImageView) view.findViewById(R.id.video_item_download_icon); 
			    	DownloadTask task = new DownloadTask(holder.progress,holder.downloadIcon);
			    	task.execute();
				}
			}else{
				isDownload = false;
			}
		};
		
	}*/
	
	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
		case R.id.ib_vediolist_back:
			finish();
			break;
		case R.id.ib_vediolist_search:
			Calendar calendar = Calendar.getInstance();
			new DatePickerDialog(VedioList.this, new DatePickerDialog.OnDateSetListener() {
	            @Override
	            public void onDateSet(DatePicker view, int year, int month, int day) {
	                // TODO Auto-generated method stub
	                //鏇存柊EditText鎺т欢鏃ユ湡 灏忎簬10鍔�
	            	System.out.println(year+"-"+(month + 1)+"-"+day);
	            	if(year == queryReplay.endYear && (month + 1) == queryReplay.endMonth && day == queryReplay.endDay){
	            		return;
	            	}
	            	queryReplay = new ReplayFile((short)1970,(short)1,(short)1,(short)0,(short)0,(short)0,(short)year,(short)(month+1),(short)day,(short)23,(short)59,(short)59);
	            	waitDialog = Utils.postNewUIDialog(VedioList.this);
	    			waitDialog.show();
	    			
	    			RefreshDateTask task = new RefreshDateTask();
	    			task.execute();
	            }
	        }, calendar.get(Calendar.YEAR), calendar.get(Calendar.MONTH),
	        calendar.get(Calendar.DAY_OF_MONTH) ).show();
			break;
//		case R.id.video_item_name:
//			HWCameraActivity.cameraLogin(ip);
//			Intent intent = new Intent(VedioList.this,HWCameraActivity.class);
//			intent.putExtra("playback", 1);//鍥炴斁
//			intent.putExtra("replayfile", rf.get(Integer.valueOf(v.getTag().toString())));
//			startActivity(intent);
//			break;
		/*case R.id.video_item_download_icon:			
			int index = Integer.valueOf(v.getTag().toString());
			View view = getItemView(index);
			if(view != null){
		        ViewHolder holder = (ViewHolder) view.getTag();  
		        holder.progress = (TextView) view.findViewById(R.id.video_item_download_progress);
		        holder.progress.setText("0%");
		    	holder.downloadIcon = (ImageView) view.findViewById(R.id.video_item_download_icon); 
		    	holder.downloadIcon.setImageDrawable(getResources().getDrawable(R.drawable.progress_blue));
		    	startRotateAnimation(holder.downloadIcon);
		    	
		    	downloadList.add(index);
		    	if(!isDownload){
		    		if(downloadList.size() > 0){
			    		isDownload = true;
			    		Log.e("", "downloadList size:"+downloadList.size());
			    		DownloadTask task = new DownloadTask(holder.progress,holder.downloadIcon);
			    	    task.execute();	
					}
		    	}	    
			}
			break;*/
		default:
			break;
		}
	}
}
