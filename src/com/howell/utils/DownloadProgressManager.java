package com.howell.utils;
/**
 * @author 霍之昊 
 *
 * 类说明
 */
public class DownloadProgressManager {
	private int dataLen;
	private int totalLen;

	public DownloadProgressManager(int totalLen) {
		super();
		this.dataLen = 0;
		this.totalLen = totalLen;
	}

	public int getDataLen() {
		return dataLen;
	}

	public void setDataLen(int dataLen) {
		this.dataLen = dataLen;
	}

	public int getTotalLen() {
		return totalLen;
	}

}
