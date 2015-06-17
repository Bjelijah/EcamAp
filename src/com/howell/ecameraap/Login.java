package com.howell.ecameraap;

import java.io.File;

import com.example.com.howell.ecameraap.R;
import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

public class Login extends Activity{
	private Button login;
	private EditText username,password;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.login);
		createFile();
		login = (Button)findViewById(R.id.btn_login);
		username = (EditText)findViewById(R.id.et_user_name);
		password = (EditText)findViewById(R.id.et_user_password);
		login.setOnClickListener(new OnClickListener() {
			//128.83
			@Override
			public void onClick(View arg0) {
				// TODO Auto-generated method stub
				if(username.getText().toString().equals("admin") && password.getText().toString().equals("admin")){
					Intent intent = new Intent(Login.this,CameraList.class);
					startActivity(intent);
					//finish();
				}else{
					
				}
			}
		});
	}
	
	private void createFile(){
		if(!existSDCard()){
			return;
		}
		File destDir = new File(Environment.getExternalStorageDirectory()+"/eCamera_AP");
		if (!destDir.exists()) {
			destDir.mkdirs();
		}
	}
	
	private boolean existSDCard() {  
    	if (android.os.Environment.getExternalStorageState().equals(  
    		android.os.Environment.MEDIA_MOUNTED)) {  
        	return true;  
        } else  
        	return false;  
    }  
	
	
	@Override
	protected void onStop() {
		// TODO Auto-generated method stub
		super.onStop();
		finish();
	}
}	
