package com.howell.ecameraap;

import java.util.ArrayList;
import java.util.List;

import com.example.com.howell.ecameraap.R;
import com.howell.db.Camera;
import com.howell.db.DBManager;

import android.animation.ObjectAnimator;
import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.text.TextPaint;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

/**
 * @author 霍之昊 
 *
 * 类说明:滑动删除ListViewAdapter
 */
@SuppressLint("NewApi")
public class ListAdapter extends BaseAdapter {
    
    private Context context;
    //private List<String> list;
	private ArrayList<Camera> list;

    private View selectedView;
    private int selectedItem;
    
    private DBManager mDBManager; 
    
    public View getSelectedView(){
         return selectedView;
    }     
    public void delete(){
        //删除所选的项
    	new AlertDialog.Builder(context).
		setTitle(context.getResources().getString(R.string.delete)).   
	    setMessage(context.getResources().getString(R.string.wether_delete)).   
	    setIcon(R.drawable.expander_ic_minimized).   
	    setPositiveButton(context.getResources().getString(R.string.ok), new DialogInterface.OnClickListener() {
				
			@Override
			public void onClick(DialogInterface arg0, int arg1) {
				// TODO Auto-generated method stub
				//数据库删除
				mDBManager.deleteOldCamera(list.get(selectedItem).name);
				//内存删除
				list.remove(selectedItem);
		        notifyDataSetChanged();
		        selectedView=null;
		        
			}
		}).   
		setNegativeButton(context.getResources().getString(R.string.cancel), new DialogInterface.OnClickListener(){

			@Override
			public void onClick(DialogInterface arg0, int arg1) {
				// TODO Auto-generated method stub
				cancel();
			}
			
		}).
	    create().show();   
    }
    
    public void cancel(){
        //取消操作，还原被移动项的位置和透明度
        ObjectAnimator.ofFloat(selectedView, "x", 0f).start();
        ObjectAnimator.ofFloat(selectedView, "alpha", 1.0f).start();
        selectedView=null;
    }
    
	public void setList(ArrayList<Camera> list) {
		this.list = list;
	}
	
	public ListAdapter(Context context, ArrayList<Camera> list ,DBManager mDBManager) {
        super();
        this.context = context;
        this.list = list;
        this.mDBManager = mDBManager;
    }

    @Override
    public int getCount() {
        // TODO Auto-generated method stub
        return list == null ? 0 : list.size();
    }

    @Override
    public Object getItem(int position) {
        // TODO Auto-generated method stub
        return list == null ? null : list.get(position);
    }

    @Override
    public long getItemId(int position) {
        // TODO Auto-generated method stub
        return position;
    }

    @Override
    public View getView(final int position, View convertView, ViewGroup parent) {
        // TODO Auto-generated method stub
    	System.out.println("getview");
        View view = LayoutInflater.from(context).inflate(R.layout.camera_list_item,null);
        TextView tvName = (TextView) view.findViewById(R.id.tv_camera_name);
        TextView tvIp = (TextView) view.findViewById(R.id.tv_camera_ip);
        System.out.println("list "+position);
        tvName.setText(list.get(position).name);
        TextPaint tp = tvName.getPaint();
        tp.setFakeBoldText(true);
        tvIp.setText(list.get(position).ip);
        
        view.setOnTouchListener(new OnTouchListener() {
            
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                // TODO Auto-generated method stub
                switch(event.getAction()){
                case MotionEvent.ACTION_DOWN:
                    selectedView = v;
                    selectedItem = position;
                    break;
                }
                return false;
            }
        });
        
        return view;
    }
}