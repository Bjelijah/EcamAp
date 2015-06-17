package com.howell.ecameraap;

import com.example.com.howell.ecameraap.R;

import android.app.TabActivity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.RadioGroup.OnCheckedChangeListener;
import android.widget.TabHost;

@SuppressWarnings("deprecation")
public class CamTabActivity extends TabActivity implements
        OnCheckedChangeListener {

    private TabHost mHost;
    private RadioGroup mGroup;
    private RadioButton mCameraList,mSettings/*,mNotices*/;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        setContentView(R.layout.cam_tab);
        Log.e("CamTabActivity", "onCreate");
        mGroup = (RadioGroup) findViewById(R.id.radio_group);
        mGroup.setOnCheckedChangeListener(this);
        mCameraList = (RadioButton)findViewById(R.id.rb_camera_list);
        mSettings = (RadioButton)findViewById(R.id.rb_settings);

        mHost = getTabHost();
        mHost.addTab(mHost
                .newTabSpec("cameralist")
                .setIndicator(getResources().getString(R.string.camera_list),
                        getResources().getDrawable(R.drawable.camera))
                .setContent(new Intent(this, CameraList.class)));

        mHost.addTab(mHost
                .newTabSpec("settings")
                .setIndicator(getResources().getString(R.string.download_list),
                        getResources().getDrawable(R.drawable.setting))
                .setContent(new Intent(this, DownloadListActivity.class)));
        mHost.setCurrentTab(0);  
        
        
    }
    
	@Override
    public void onCheckedChanged(RadioGroup group, int checkedId) {
        // TODO Auto-generated method stub
        switch (checkedId) {
        case R.id.rb_camera_list:
            mHost.setCurrentTabByTag("cameralist");
            mCameraList.setTextColor(getResources().getColor(R.color.blue));
            mSettings.setTextColor(getResources().getColor(R.color.light_gray));
            break;
        case R.id.rb_settings:
            mHost.setCurrentTabByTag("settings");
            mSettings.setTextColor(getResources().getColor(R.color.blue));
            mCameraList.setTextColor(getResources().getColor(R.color.light_gray));
            break;
        default:
            break;
        }
    }
}
