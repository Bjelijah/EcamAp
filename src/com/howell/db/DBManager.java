package com.howell.db;

import java.util.ArrayList;
import java.util.List;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;

public class DBManager {
	private DBHelper helper;
	private SQLiteDatabase db;
	
	public DBManager(Context context) {
		helper = new DBHelper(context);
		//��ΪgetWritableDatabase�ڲ�������mContext.openOrCreateDatabase(mName, 0, mFactory);
		//����Ҫȷ��context�ѳ�ʼ��,���ǿ��԰�ʵ��DBManager�Ĳ������Activity��onCreate��
		db = helper.getReadableDatabase();
	}
	
	/**
	 * add persons
	 * @param persons
	 */
	public void add(Camera camera) {
        db.beginTransaction();	//��ʼ����
        try {
        	//for (Person person : persons) {
        	db.execSQL("INSERT INTO camera VALUES(null, ?, ?, ?)", new Object[]{camera.name, camera.ip, camera.channel});
        	//}
        	db.setTransactionSuccessful();	//��������ɹ����
        } finally {
        	db.endTransaction();	//��������
        }
	}
	
	/**
	 * update person's age
	 * @param person
	 */
	public void updateName(String oldName,String newName) {
		ContentValues cv = new ContentValues();
		cv.put("name", oldName);
		db.update("camera", cv, "name = ?", new String[]{newName});
	}
	
	/**
	 * delete old person
	 * @param person
	 */
	public void deleteOldCamera(String name) {
		db.delete("camera", "name = ?", new String[]{name});
	}
	
	/**
	 * query all persons, return list
	 * @return List<Person>
	 */
	public ArrayList<Camera> query() {
		ArrayList<Camera> cameras = new ArrayList<Camera>();
		Cursor c = queryTheCursor();
        while (c.moveToNext()) {
        	Camera camera = new Camera();
        	camera._id = c.getInt(c.getColumnIndex("_id"));
        	camera.name = c.getString(c.getColumnIndex("name"));
        	camera.ip = c.getString(c.getColumnIndex("ip"));
        	camera.channel = c.getInt(c.getColumnIndex("channel"));
        	cameras.add(camera);
        }
        c.close();
        return cameras;
	}
	
	/**
	 * query all persons, return cursor
	 * @return	Cursor
	 */
	public Cursor queryTheCursor() {
        Cursor c = db.rawQuery("SELECT * FROM camera", null);
        return c;
	}
	
	/**
	 * close database
	 */
	public void closeDB() {
		db.close();
	}
}
