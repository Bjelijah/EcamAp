package com.howell.ecameraap;

import android.app.Activity;
import android.os.Bundle;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

import com.example.com.howell.ecameraap.R;
import com.howell.db.Camera;
import com.howell.db.DBManager;
import com.howell.utils.Utils;

public class AddCamera extends Activity {
	private EditText cameraName,Ip,channel;
	private Button ok,cancel;
    private DBManager mDBManager; 
    
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.add_cam);
		cameraName = (EditText)findViewById(R.id.et_camera_name);
		Ip = (EditText)findViewById(R.id.et_ip);
		channel = (EditText)findViewById(R.id.et_channel);
		ok = (Button)findViewById(R.id.btn_ok);
		cancel = (Button)findViewById(R.id.btn_cancel);
		mDBManager = new DBManager(AddCamera.this);
		
		ok.setOnClickListener(listener);
		cancel.setOnClickListener(listener);
	}
	
	private OnClickListener listener = new OnClickListener() {
		
		@Override
		public void onClick(View view) {
			// TODO Auto-generated method stub
			switch (view.getId()) {
			case R.id.btn_ok:
				String name = cameraName.getText().toString();
				String ip = Ip.getText().toString();
				String s_channel = channel.getText().toString();
				if(name.equals("")){
					Utils.postToast(AddCamera.this, "请输入设备名", 2000);
					break;
				}
				if(ip.equals("")){
					Utils.postToast(AddCamera.this, "请输入设备ip", 2000);
					break;
				}
				if(s_channel.equals("")){
					Utils.postToast(AddCamera.this, "请输入通道号", 2000);
					break;
				}
				mDBManager.add(new Camera(name,ip,Integer.valueOf(s_channel)));
				finish();
				break;
			case R.id.btn_cancel:
				finish();
				break;
			default:
				break;
			}
		}
	};
	
	@Override  
	    protected void onDestroy() {  
	    super.onDestroy();  
	    mDBManager.closeDB();  
	}  

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

}
