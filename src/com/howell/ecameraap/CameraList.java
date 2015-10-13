package com.howell.ecameraap;

import java.util.ArrayList;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ImageButton;
import android.widget.ListView;

import com.example.com.howell.ecameraap.R;
import com.howell.db.Camera;
import com.howell.db.DBManager;

public class CameraList extends Activity implements OnItemClickListener{
	private ImageButton addCam/*,back*/;
	private ListView cameraList;
	private ArrayList<Camera> cameras;
    private DBManager mDBManager; 
    private ListAdapter adapter;
    private boolean isMove = false;//限制滑动时触发OnItemClickListener的标志位
    
//	WifiManager.MulticastLock lock; 
    
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.camera_list);
//		WifiManager manager = (WifiManager)getSystemService(Context.WIFI_SERVICE);
//		lock = manager.createMulticastLock("UDPwifi");
//		lock.acquire();
		addCam = (ImageButton)findViewById(R.id.ib_add_cam);
		addCam.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View arg0) {
				// TODO Auto-generated method stub
				Intent intent = new Intent(CameraList.this,AddCamera.class);
				startActivity(intent);
			}
		});
/*		back = (ImageButton)findViewById(R.id.ib_back);
		back.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View arg0) {
				// TODO Auto-generated method stub
				finish();
			}
		});*/
		
		mDBManager = new DBManager(CameraList.this);
		cameras= mDBManager.query();
//		for(Camera c:cameras){
//			System.out.println(c.toString());
//		}
		
		adapter = new ListAdapter(this,cameras,mDBManager);
		cameraList = (ListView)findViewById(R.id.camera_list);
		cameraList.setAdapter(adapter);
		cameraList.setOnItemClickListener(this);
		
		cameraList.setOnTouchListener(new OnTouchListener() {
			
			private View selectedView;
			private float oldX;
			private float oldY;
			
			@Override
			public boolean onTouch(View v, MotionEvent event) {
				// TODO Auto-generated method stub
				if (adapter.getSelectedView() == null) {
					return false;
				}
				
				switch (event.getAction()) {
					case MotionEvent.ACTION_DOWN:
						// 按下事件，获取用户要操作的项，和事件坐标
						System.out.println("action down:"+isMove);
						selectedView = adapter.getSelectedView();
						oldX = event.getX();
						oldY = event.getY();
						
						break;
					case MotionEvent.ACTION_MOVE:
						/*
						* 移动事件，若y方向移动距离大于x方向移动距离，
						* 则认为用户是想上下滚动列表。
						* 否则，用户是想滑动列表的某一项。
						*/
						isMove = true;
						System.out.println("action move:"+isMove);
						
						float distanceX=event.getX()-oldX;
						float distanceY=event.getY()-oldY;
						if(Math.abs(distanceX)>Math.abs(distanceY)){
							selectedView.setX(selectedView.getX()+distanceX);
							selectedView.setAlpha(Math.max(0.1f,1-Math.abs(selectedView.getX()/200)));
						}
						oldX=event.getX();
						oldY=event.getY();
						break;
					case MotionEvent.ACTION_UP:
						/*
						* 弹起事件，根据x方向移动距离的大小，
						* 判断要进行什么操作
						*/
						
						if(Math.abs(selectedView.getX())>200){
						//移动距离大于200px，则删除该项
						//（直接删除会有点突兀，可增加确认对话框或删除动画等。）
							adapter.delete();
							//isMove = false;
						}else{
						//否则，将其还原
							adapter.cancel();
							isMove = false;
						}
						System.out.println("action up:"+isMove);
						break;
					
					}
					return false;
				}
			});
	}
	
/*	public class CameraListAdapter extends BaseAdapter {

	    private Context mContext;
//	    private LayoutInflater mInflater;

	    public CameraListAdapter(Context context) {
	        mContext = context;
//	        mInflater = getLayoutInflater();
	    }
	        
	    @Override
	    public int getCount() {
	        // TODO Auto-generated method stub
	        return cameras == null ? 0 : cameras.size();
	    }

	    @Override
	    public Object getItem(int position) {
	        // TODO Auto-generated method stub
	        return cameras == null ? null : cameras.get(position);
	    }

	    @Override
	    public long getItemId(int position) {
	        // TODO Auto-generated method stub
	        return position;
	    }

	    @Override
	    public View getView(int position, View convertView, ViewGroup parent) {
	        // TODO Auto-generated method stub
	        System.out.println("getView");
	        Camera c = (Camera)getItem(position);
	        ViewHolder holder;
            if (convertView == null) {
            	LayoutInflater layoutInflater = LayoutInflater.from(mContext);
            	convertView = layoutInflater.inflate(R.layout.camera_list_item, null);

                holder = new ViewHolder();
                holder.name = (TextView) convertView.findViewById(R.id.tv_camera_name);
                holder.ip = (TextView) convertView.findViewById(R.id.tv_camera_ip);
                convertView.setTag(holder);
            } else {
                holder = (ViewHolder) convertView.getTag();
            }
            
            holder.name.setText(c.name);
            TextPaint tp = holder.name.getPaint();
            tp.setFakeBoldText(true);
            holder.ip.setText(c.ip);
            
            return convertView;
	    }
	}
	
    public static class ViewHolder {
	    public TextView name,ip;
	    public int channel;
	}*/
	
	@Override  
    protected void onDestroy() {  
	    super.onDestroy();  
	    //Ӧ�õ����һ��Activity�ر�ʱӦ�ͷ�DB  
	    mDBManager.closeDB();  
//	    lock.release();
	}
	
	@Override
	protected void onRestart() {
		// TODO Auto-generated method stub
		super.onRestart();
		System.out.println("onRestart");
		cameras = mDBManager.query();
		adapter.setList(cameras);
		adapter.notifyDataSetChanged();
	}

	@Override
	public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
		// TODO Auto-generated method stub
		System.out.println("onItemClick");
		System.out.println("isMove:"+isMove);
		if(isMove){
			return;
		}
		
		if(HWCameraActivity.cameraLogin(cameras.get((int)arg3).ip) != -1){
			
			Intent intent = new Intent(CameraList.this,HWCameraActivity.class);
			//intent.putExtra("camera", cameras.get((int)arg3).ip);
			intent.putExtra("playback", 0);//预览
			intent.putExtra("ip", cameras.get((int)arg3).ip);
			intent.putExtra("slot", cameras.get((int)arg3).channel);
			startActivity(intent);
//			lock.release();
		}else{
			postAlerDialog(CameraList.this,getResources().getString(R.string.login_fail));
		}
	}
	
	public void postAlerDialog(Context context,String message){
		new AlertDialog.Builder(context)   
	//    .setTitle("�û�����������")   
	    .setMessage(message)                 
	    .setPositiveButton(getResources().getString(R.string.ok), null)   
	    .show();  
	}
}
