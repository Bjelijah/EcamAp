package com.howell.ecameraap.downloadfile;

import com.howell.ecameraap.ReplayFile;

/**
 * author:alexzhou 
 * email :zhoujiangbohai@163.com 
 * date :2013-1-27
 * 
 * 游戏列表中的app文件
 **/

public class AppFile {
	
	public int id;
	public String name,name2;
	// app的大小
	public int size;
	// 已下载大小
	public int downloadSize;
	// 下载状态:正常,正在下载，暂停，等待，已下载
	public int downloadState;
	
	public ReplayFile replay;
	
	@Override
	public String toString() {
		return "AppFile [id=" + id + ", name=" + name + ", size=" + size
				+ ", downloadSize=" + downloadSize + ", downloadState="
				+ downloadState + "]";
	}
	
	
}
