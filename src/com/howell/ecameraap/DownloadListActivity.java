package com.howell.ecameraap;

import java.util.ArrayList;
import java.util.Date;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.example.com.howell.ecameraap.R;
import com.howell.utils.Utils;

/**
 * @author 霍之昊 
 *
 * 类说明
 */
public class DownloadListActivity extends Activity {
	
	private ListView listview;
	private ArrayList<ReplayFile> downloadList;
	private DownloadAdapter adapter;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.donwload_list_activity);
		init();
	}
	
	private void init(){
		downloadList = new ArrayList<ReplayFile>();
		downloadList.add(new ReplayFile((short)2015,(short)1,(short)1,(short)10,(short)10,(short)10
				,(short)2015,(short)1,(short)1,(short)10,(short)10,(short)30));
		downloadList.add(new ReplayFile((short)2015,(short)1,(short)1,(short)10,(short)12,(short)10
				,(short)2015,(short)1,(short)1,(short)10,(short)12,(short)30));
		downloadList.add(new ReplayFile((short)2015,(short)1,(short)1,(short)10,(short)14,(short)10
				,(short)2015,(short)1,(short)1,(short)10,(short)14,(short)30));
		listview = (ListView)findViewById(R.id.download_listview);
		adapter = new DownloadAdapter(this);
		listview.setAdapter(adapter);
	}
	
	public class DownloadAdapter extends BaseAdapter{
		
		private Context mContext;
		private Date beg ,end;
        private String s_beg,s_end;
		
		public DownloadAdapter(Context mContext) {
			// TODO Auto-generated constructor stub
			this.mContext = mContext;
		}
		@Override
		public int getCount() {
			// TODO Auto-generated method stub
			return downloadList.size();
		}

		@Override
		public Object getItem(int arg0) {
			// TODO Auto-generated method stub
			return downloadList.get(arg0);
		}

		@Override
		public long getItemId(int arg0) {
			// TODO Auto-generated method stub
			return arg0;
		}

		@Override
		public View getView(int position, View convertView, ViewGroup arg2) {
			// TODO Auto-generated method stub
			ViewHolder holder = null;
	    	if (convertView == null) {
	    		LayoutInflater layoutInflater = LayoutInflater.from(mContext);
	    		convertView = layoutInflater.inflate(R.layout.download_list_item, null);
				holder = new ViewHolder();
				
				holder.date = (TextView)convertView.findViewById(R.id.download_date);
				holder.progressbar = (ProgressBar)convertView.findViewById(R.id.download_progress_bar);
				
				convertView.setTag(holder);
	    	}else{
	         	holder = (ViewHolder)convertView.getTag();
	        }
	    	
	    	ReplayFile rf = downloadList.get(position);
	    	beg = new Date(rf.begYear - 1900, rf.begMonth - 1, rf.begDay, rf.begHour, rf.begMinute, rf.begSecond);
	 		end = new Date(rf.endYear - 1900, rf.endMonth - 1, rf.endDay, rf.endHour, rf.endMinute, rf.endSecond);
	        s_beg = Utils.dateToString(beg);
	        s_end = Utils.dateToString(end);
	    	holder.date.setText(s_beg + " -> " + s_end);
			return convertView;
		}
		class ViewHolder {
			//group_item:videoGroupName,video_source_item:videoSourceLayout
		    public TextView date;
		    public ProgressBar progressbar;
		}
	}

}
