package com.howell.ecameraap;

import java.util.ArrayList;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.text.TextPaint;
import android.util.Log;
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
import com.howell.db.Camera;
import com.howell.db.DBManager;
import com.howell.ecameraap.SlideView.OnSlideListener;

public class CameraList extends Activity implements OnItemClickListener, OnClickListener,
	OnSlideListener {
	private ImageButton addCam,back;
	private ListViewCompat cameraList;
	private ArrayList<Camera> cameras;
    private DBManager mDBManager; 
    private CameraListAdapter adapter;
    private SlideView mLastSlideViewWithStatusOn;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.camera_list);
		addCam = (ImageButton)findViewById(R.id.ib_add_cam);
		addCam.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View arg0) {
				// TODO Auto-generated method stub
				Intent intent = new Intent(CameraList.this,AddCamera.class);
				startActivity(intent);
			}
		});
		back = (ImageButton)findViewById(R.id.ib_back);
		back.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View arg0) {
				// TODO Auto-generated method stub
				finish();
			}
		});
		
		mDBManager = new DBManager(CameraList.this);
		cameras= mDBManager.query();
//		for(Camera c:cameras){
//			System.out.println(c.toString());
//		}
		
		adapter = new CameraListAdapter(this);
		cameraList = (ListViewCompat)findViewById(R.id.camera_list);
		cameraList.setAdapter(adapter);
		cameraList.setOnItemClickListener(this);
	}
	
	public class CameraListAdapter extends BaseAdapter {

	    //private Context mContext;
	    private LayoutInflater mInflater;

	    public CameraListAdapter(Context context) {
	        //mContext = context;
	        mInflater = getLayoutInflater();
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
            SlideView slideView = (SlideView) convertView;
            if (slideView == null) {
                View itemView = mInflater.inflate(R.layout.camera_list_item, null);

                slideView = new SlideView(CameraList.this);
                slideView.setContentView(itemView);

                holder = new ViewHolder(slideView);
                slideView.setOnSlideListener(CameraList.this);
                slideView.setTag(holder);
            } else {
                holder = (ViewHolder) slideView.getTag();
            }
            c.slideView = slideView;
            c.slideView.shrink();

            holder.name.setText(c.name);
            TextPaint tp = holder.name.getPaint();
            tp.setFakeBoldText(true);
            //holder.name.setOnClickListener(CameraList.this);
            //holder.name.setTag(position);
            holder.ip.setText(c.ip);
            
            //holder.ip.setOnClickListener(CameraList.this);
            //holder.ip.setTag(position);
            holder.deleteHolder.setOnClickListener(CameraList.this);
            holder.deleteHolder.setTag(position);
            //holder.icon.setOnClickListener(CameraList.this);
            //holder.icon.setTag(position);

            return slideView;
	    }
	}
	
    public static class ViewHolder {
	    public TextView name,ip;
	    public int channel;
	    //public ImageView icon;
	    public ViewGroup deleteHolder;
	    ViewHolder(View view) {
	    	name = (TextView) view.findViewById(R.id.tv_camera_name);
            ip = (TextView) view.findViewById(R.id.tv_camera_ip);
            deleteHolder = (ViewGroup)view.findViewById(R.id.holder);
            //icon = (ImageView)view.findViewById(R.id.iv_icon);
	    }
	}
	
	@Override  
    protected void onDestroy() {  
	    super.onDestroy();  
	    //Ӧ�õ����һ��Activity�ر�ʱӦ�ͷ�DB  
	    mDBManager.closeDB();  
	}
	
	@Override
	protected void onRestart() {
		// TODO Auto-generated method stub
		super.onRestart();
		System.out.println("onRestart");
		cameras= mDBManager.query();
		adapter.notifyDataSetChanged();
	}

	@Override
	public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
		// TODO Auto-generated method stub
		System.out.println("onItemClick");
		if(HWCameraActivity.cameraLogin(cameras.get((int)arg3).ip) != -1){
			Intent intent = new Intent(CameraList.this,HWCameraActivity.class);
			//intent.putExtra("camera", cameras.get((int)arg3).ip);
			intent.putExtra("playback", 0);//预览
			intent.putExtra("ip", cameras.get((int)arg3).ip);
			startActivity(intent);
		}else{
			postAlerDialog(CameraList.this,"连接失败,请确认ip是否正确！");
		}
		
	}
	
	public void postAlerDialog(Context context,String message){
		new AlertDialog.Builder(context)   
	//    .setTitle("�û�����������")   
	    .setMessage(message)                 
	    .setPositiveButton("确定", null)   
	    .show();  
	}

    @Override
    public void onSlide(View view, int status) {
        if (mLastSlideViewWithStatusOn != null && mLastSlideViewWithStatusOn != view) {
            mLastSlideViewWithStatusOn.shrink();
        }

        if (status == SLIDE_STATUS_ON) {
            mLastSlideViewWithStatusOn = (SlideView) view;
        }
    }

    @Override
    public void onClick(View v) {
        if (v.getId() == R.id.holder) {
            Log.e("", "onClick v=" + v);
            int position = Integer.valueOf(v.getTag().toString());
            Camera c = cameras.get(position);
            cameras.remove(position);
            mDBManager.deleteOldCamera(c.name);
            adapter.notifyDataSetChanged();
        }/*else if (v.getId() == R.id.tv_camera_name || v.getId() == R.id.tv_camera_ip || v.getId() == R.id.iv_icon) {
            Log.e("", "onClick v=" + v);
            int position = Integer.valueOf(v.getTag().toString());
            if(HWCameraActivity.cameraLogin(cameras.get(position).ip) != -1){
    			Intent intent = new Intent(CameraList.this,HWCameraActivity.class);
    			//intent.putExtra("camera", cameras.get((int)arg3).ip);
    			startActivity(intent);
    		}else{
    			postAlerDialog(CameraList.this,"ip不存在");
    		}
        }*/
    }  
}
