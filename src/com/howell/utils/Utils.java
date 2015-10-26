package com.howell.utils;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;

import com.example.com.howell.ecameraap.R;
import com.howell.ecameraap.HWCameraActivity;
import com.howell.ecameraap.VedioList;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.widget.Toast;

public class Utils {
	public static String getCharacterAndNumber() {
		String rel="";
		SimpleDateFormat formatter = new SimpleDateFormat("yyyyMMddHHmmss");
		Date curDate = new Date(System.currentTimeMillis());
		rel = formatter.format(curDate);
		return rel;
	}

	public static String getFileName() {
		// mu
		//String fileNameRandom = getCharacterAndNumber(8);
		String fileNameRandom = getCharacterAndNumber();
		return fileNameRandom;
	}
	
	public static void postToast(Context context,String message,int time){
//		Toast toast= Toast.makeText(context, message, 1000);
//		toast.setGravity(Gravity.CENTER, 0, 0);
//		toast.show();
		Toast.makeText(context, message, time).show();
	}
	
	public static void postAlerDialog(Context context,String message){
		new AlertDialog.Builder(context)   
//        .setTitle("�û�����������")   
        .setMessage(message)                 
        .setPositiveButton("确定", null)   
        .show();  
	}
	
	public static Dialog postNewUIDialog(Context context){
		final Dialog lDialog = new Dialog(context,android.R.style.Theme_Translucent_NoTitleBar_Fullscreen);
//       lDialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
		lDialog.setContentView(R.layout.wait_dialog);
//       ((TextView) lDialog.findViewById(R.id.dialog_title)).setText(pTitle);
		//lDialog.show();
		return lDialog;
	}
	
	public static void postFinishActivityAlerDialog(final Context context,String message){
		new AlertDialog.Builder(context)   
//        .setTitle("�û�����������")   
        .setMessage(message)                 
        .setPositiveButton("确定", new DialogInterface.OnClickListener() {
			
			@Override
			public void onClick(DialogInterface dialog, int which) {
				((Activity) context).finish();
			}
		})
        .show();  
	}
	
    public static Date StringToDate(String string){
      	 SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
      	 Date date = null;
   		 try {
   			date = sdf.parse(string);
   		 } catch (ParseException e) {
   			// TODO Auto-generated catch block
   			e.printStackTrace();
   		 }
      	 return date;
    }
       
    public static String dateToString(Date date){
      	 SimpleDateFormat foo = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
         String stringTime = foo.format(date);
         return stringTime;
    }
    
    public static String dateToString2(Date date){
    	SimpleDateFormat foo = new SimpleDateFormat("HH:mm:ss");
        String stringTime = foo.format(date);
        return stringTime;
   }
}
