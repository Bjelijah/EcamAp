package com.howell.ecameraap;

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
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.DatePicker;
import android.widget.ImageButton;
import android.widget.TextView;

import com.example.com.howell.ecameraap.R;
import com.howell.ecameraap.MyListViewWithFoot.OnRefreshListener;
import com.howell.utils.Utils;

public class VedioList extends Activity implements OnItemClickListener,OnClickListener{
	private ImageButton back,search;
	private MyListViewWithFoot vedioList;
	//private ArrayList<String> mAdapterList;
	private VideoListAdapter adapter;
	private String ip;
	
	private ArrayList<ReplayFile> rf;
	private int replayCount ;
	
	private static final int STOPREFRESH = 1;
	private static final int REFRESHFAIL = 2;
	private int userHandle;
	private Pagination pagination;
	
	private ReplayFile queryReplay;
	private Dialog waitDialog;
	
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
		Intent intent = getIntent();
		ip = intent.getStringExtra("ip");
		back = (ImageButton)findViewById(R.id.ib_vediolist_back);
		back.setOnClickListener(listener);
		rf = new ArrayList<ReplayFile>();
		//mAdapterList = new ArrayList<String>();
		adapter = new VideoListAdapter(this);
		vedioList = (MyListViewWithFoot)findViewById(R.id.mylistview_vedio_list);
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
						return null;
					}

					@Override
					protected void onPostExecute(Void result) {
						if(pagination.page_count <  pagination.page_no + 1){
							Utils.postToast(VedioList.this,"没有更多视频录像",1000);
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
		vedioList.setOnItemClickListener(this);
		
		search = (ImageButton)findViewById(R.id.ib_vediolist_search);
		search.setOnClickListener(this);
	}
	
	private ReplayFile getEpoch2NowTime(){
		Calendar calendar = Calendar.getInstance();
		return new ReplayFile((short)1970, (short)1, (short)1, (short)0,(short) 0, (short)0
				, (short)calendar.get(Calendar.YEAR), (short)(calendar.get(Calendar.MONTH) + 1)
				, (short)calendar.get(Calendar.DAY_OF_MONTH), (short)calendar.get(Calendar.HOUR)
				, (short)calendar.get(Calendar.MINUTE), (short)calendar.get(Calendar.SECOND));
	}
	
	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
		closeFileList(HWCameraActivity.fileListHandle);
		vedioListLogout(userHandle);
		//HWCameraActivity.fileListHandle = -2;
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
				Utils.postFinishActivityAlerDialog(VedioList.this, "获取回放列表失败，请重新获取！");
				adapter.notifyDataSetChanged();
				vedioList.onRefreshComplete();
				break;
			default:
				break;
			}
		}
	};
	
	private OnClickListener listener = new OnClickListener() {
		
		@Override
		public void onClick(View view) {
			// TODO Auto-generated method stub
			switch (view.getId()) {
			case R.id.ib_vediolist_back:
				finish();
				break;

			default:
				break;
			}
		}
	};
	
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
        //private VODRecord record;
        //private ArrayList<VODRecord> mAdapterList;

        public VideoListAdapter(Context context) {
            mContext = context;
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
//        	Log.e("---------->>>>", "getView");
//        	System.out.println("position"+position);
			System.out.println("getCount:"+getCount());
			//record = (VODRecord) getItem(position);
			ReplayFile replayFile = (ReplayFile)getItem(position);
			LayoutInflater layoutInflater = LayoutInflater.from(mContext);
			convertView = layoutInflater.inflate(R.layout.video_item, null);
			TextView name = (TextView) convertView.findViewById(R.id.name);
			beg = new Date(replayFile.begYear - 1900, replayFile.begMonth - 1, replayFile.begDay, replayFile.begHour, replayFile.begMinute, replayFile.begSecond);
	 		end = new Date(replayFile.endYear - 1900, replayFile.endMonth - 1, replayFile.endDay, replayFile.endHour, replayFile.endMinute, replayFile.endSecond);
	        s_beg = Utils.dateToString(beg);
	        s_end = Utils.dateToString(end);
	        name.setText(s_beg + " -> " + s_end);
            return convertView;
        }
    }
    
	public native ReplayFile[] getReplayList(int file_list_handle,int fileCount);
	public native void closeFileList(int file_list_handle);
	public native int getListByPage(int user_handle,int slot,int stream,ReplayFile replay,int type,int order_by_time,Pagination page_info);
	public native int vedioListLogin(String ip);
	public native int vedioListLogout(int user_handle);
	
	@Override
	public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
		// TODO Auto-generated method stub
		HWCameraActivity.cameraLogin(ip);
		Intent intent = new Intent(VedioList.this,HWCameraActivity.class);
		intent.putExtra("playback", 1);//回放
		intent.putExtra("replayfile", rf.get((int)arg3));
		startActivity(intent);
	}
	
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

	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
		case R.id.ib_vediolist_search:
			Calendar calendar = Calendar.getInstance();
			new DatePickerDialog(VedioList.this, new DatePickerDialog.OnDateSetListener() {
	            @Override
	            public void onDateSet(DatePicker view, int year, int month, int day) {
	                // TODO Auto-generated method stub
	                //更新EditText控件日期 小于10加0
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

		default:
			break;
		}
	}
}
