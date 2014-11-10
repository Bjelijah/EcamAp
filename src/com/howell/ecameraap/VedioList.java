package com.howell.ecameraap;

import java.util.Date;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.ImageButton;
import android.widget.TextView;

import com.example.com.howell.ecameraap.R;
import com.howell.ecameraap.MyListViewWithFoot.OnRefreshListener;
import com.howell.utils.Utils;

public class VedioList extends Activity implements OnItemClickListener{
	private ImageButton back;
	private MyListViewWithFoot vedioList;
	//private ArrayList<String> mAdapterList;
	private VideoListAdapter adapter;
	private String ip;
	
	private ReplayFile[] rf;
	private int replayCount ;
	
	private static final int STOPREFRESH = 1;
	private static final int REFRESHFAIL = 2;
	
	static {
		//System.loadLibrary("hwplay");
        System.loadLibrary("player_jni");
    }
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.vedio_list);
		replayCount = 100;
		Intent intent = getIntent();
		ip = intent.getStringExtra("ip");
		back = (ImageButton)findViewById(R.id.ib_vediolist_back);
		back.setOnClickListener(listener);
		//mAdapterList = new ArrayList<String>();
		adapter = new VideoListAdapter(this);
		vedioList = (MyListViewWithFoot)findViewById(R.id.mylistview_vedio_list);
		vedioList.setAdapter(adapter);
		vedioList.setonRefreshListener(new OnRefreshListener() {
			
			@Override
			public void onRefresh() {
				// TODO Auto-generated method stub
				replayCount = 100;
				rf = getReplayList(HWCameraActivity.fileListHandle,replayCount);
//				sort(rf);
//				for(ReplayFile r: rf){
//					System.out.println(r.toString());
//				}
				handler.sendEmptyMessage(STOPREFRESH);
				/*HWCameraActivity.fileListHandle = -2;
				while(HWCameraActivity.fileListHandle == -2){
					try {
						Thread.sleep(100);
						if(HWCameraActivity.fileListHandle == -1){
							System.out.println("search vedio list fail");
							handler.sendEmptyMessage(REFRESHFAIL);
							return;
						}
						System.out.println("HWCameraActivity.fileListHandle:"+HWCameraActivity.fileListHandle);
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
				rf = getReplayList(HWCameraActivity.fileListHandle,100);
				for(ReplayFile r: rf){
					System.out.println(r.toString());
				}
				handler.sendEmptyMessage(STOPREFRESH);*/
			}
			int position = 0;
			@Override
			public void onFootRefresh() {
				// TODO Auto-generated method stub
				new AsyncTask<Void, Void, Void>() {
					protected Void doInBackground(Void... params) {
						position = replayCount;
						replayCount +=  100;
						rf = getReplayList(HWCameraActivity.fileListHandle,replayCount);
//						sort(rf);
//						for(ReplayFile r: rf){
//							System.out.println(r.toString());
//						}
						return null;
					}

					@Override
					protected void onPostExecute(Void result) {
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
				while(HWCameraActivity.fileListHandle == -2){
					try {
						Thread.sleep(100);
						if(HWCameraActivity.fileListHandle == -1){
							System.out.println("search vedio list fail");
							handler.sendEmptyMessage(REFRESHFAIL);
							return;
						}
						System.out.println("HWCameraActivity.fileListHandle:"+HWCameraActivity.fileListHandle);
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
				rf = getReplayList(HWCameraActivity.fileListHandle,replayCount);
//				sort(rf);
//				for(ReplayFile r: rf){
//					System.out.println(r.toString());
//				}
				handler.sendEmptyMessage(STOPREFRESH);
			}
		});
		vedioList.setOnItemClickListener(this);
	}
	
	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
		closeFileList(HWCameraActivity.fileListHandle);
		HWCameraActivity.fileListHandle = -2;
	}
	
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
            return rf == null ? 0 : rf.length;
        }

        @Override
        public Object getItem(int position) {
            // TODO Auto-generated method stub
            return rf == null ? null : rf[position];
        }

        @Override
        public long getItemId(int position) {
            // TODO Auto-generated method stub
            return position;
        }

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

	@Override
	public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
		// TODO Auto-generated method stub
		HWCameraActivity.cameraLogin(ip);
		Intent intent = new Intent(VedioList.this,HWCameraActivity.class);
		intent.putExtra("playback", 1);//回放
		intent.putExtra("replayfile", rf[(int)arg3]);
		startActivity(intent);
	}
}
