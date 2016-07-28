package com.howell.ecameraap;

import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import com.example.com.howell.ecameraap.R;

/**
 * @author 霍之昊 
 *
 * 类说明
 */
public class SettingActivity extends Activity {
	
	private EditText modifySsid,modifyKey,ip;
	private Button ok;
	private String ssid,key;
	
	private static final int KEY_LESS_THEN_8 = 1;
	private static final int SET_SUCCESS = 2;
	private static final int SET_FAIL = 3;
	
	private native int getWifi(String ip);
	private native int setWifi(String ip,String ssid,String password);
	
	static {
		System.loadLibrary("hwplay");
		System.loadLibrary("ffmpeg");
        System.loadLibrary("player_jni");
    }
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.setting);
		modifySsid = (EditText)findViewById(R.id.modify_ssid);
		modifyKey = (EditText)findViewById(R.id.modify_key);
		ip = (EditText)findViewById(R.id.ip);
		ok = (Button)findViewById(R.id.setting_btn_ok);
		ok.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				ssid = modifySsid.getText().toString();
				key = modifyKey.getText().toString();
				final String deviceIp = ip.getText().toString();
				
				new Thread(){
					public void run() {
						if(key.length() < 8){
							handler.sendEmptyMessage(KEY_LESS_THEN_8);
						}else{
							if(setWifi(deviceIp,ssid,key) == 1){
								//设置成功
								handler.sendEmptyMessage(SET_SUCCESS);
							}else{
								//设置失败
								handler.sendEmptyMessage(SET_FAIL);
							}
						}
					};
				}.start();
			}
		});
		
	}
	
	Handler handler = new Handler(){
		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			super.handleMessage(msg);
			switch (msg.what) {
			case KEY_LESS_THEN_8:
				Toast.makeText(SettingActivity.this, "密码必须大于8位", 2000).show();
				break;
			case SET_SUCCESS:
				Toast.makeText(SettingActivity.this, "设置成功，重启设备生效", 2000).show();
				break;
			case SET_FAIL:
				Toast.makeText(SettingActivity.this, "设置失败，请重新设置", 2000).show();
				break;
			default:
				break;
			}
		}
	};
}
