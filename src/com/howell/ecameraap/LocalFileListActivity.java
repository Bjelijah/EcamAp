package com.howell.ecameraap;

import java.io.File;
import java.util.ArrayList;
import java.util.Date;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.example.com.howell.ecameraap.R;
import com.howell.utils.Utils;

/**
 * @author 霍之昊 
 *
 * 类说明
 */
public class LocalFileListActivity extends Activity implements OnItemClickListener{
	
	private ListView listview;
	private LocalFileListAdapter adapter;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.local_file_list);
		listview = (ListView)findViewById(R.id.localfile_listview);
		adapter = new LocalFileListAdapter(this,getFileName(new File(Environment.getExternalStorageDirectory().getPath()+"/eCamera_Ap/")));
		listview.setAdapter(adapter);
		listview.setOnItemClickListener(this);
	}
	
	@Override
	protected void onResume() {
		// TODO Auto-generated method stub
		super.onResume();
		adapter.setmList(getFileName(new File(Environment.getExternalStorageDirectory().getPath()+"/eCamera_Ap/")));
	}
	
	public ArrayList<String> getFileName(File file){
		File[] fileArray = file.listFiles();
		ArrayList<String> mList = new ArrayList<String>();
		for (File f : fileArray) {
			if(f.isFile() && f.getPath().endsWith(".hwr")){
				System.out.println(f.getPath());
				mList.add(f.getPath());
			}
		}
		return mList;
	}
	
	class LocalFileListAdapter extends BaseAdapter{
		
		private ArrayList<String> mList;
		private Context mContext;
		
		public LocalFileListAdapter(Context mContext,ArrayList<String> mList) {
			super();
			this.mContext = mContext;
			this.mList = mList;
		}

		public void setmList(ArrayList<String> mList) {
			this.mList = mList;
			notifyDataSetChanged();
		}

		@Override
		public int getCount() {
			// TODO Auto-generated method stub
			return mList.size();
		}

		@Override
		public Object getItem(int arg0) {
			// TODO Auto-generated method stub
			return mList.get(arg0);
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
	    		convertView = layoutInflater.inflate(R.layout.local_file_item, null);
				holder = new ViewHolder();
				
				holder.name = (TextView)convertView.findViewById(R.id.tv_localfile_item_filename);
				
				convertView.setTag(holder);
	    	}else{
	         	holder = (ViewHolder)convertView.getTag();
	        }
	    	
	    	holder.name.setText(mList.get(position));
			return convertView;
		}
		
		class ViewHolder {
		    public TextView name;
		}
		
	}

	@Override
	public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
		// TODO Auto-generated method stub
		Intent intent = new Intent(this,LocalFilePlayer.class);
		intent.putExtra("path", adapter.mList.get((int)arg3));
		startActivity(intent);
	}
}
