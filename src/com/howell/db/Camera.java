package com.howell.db;

public class Camera {
	public int _id;
	public String name;
	public String ip;
	public int channel;
//	public SlideView slideView;
	public Camera(String name, String ip, int channel) {
		super();
		this.name = name;
		this.ip = ip;
		this.channel = channel;
	}
	public Camera() {
		super();
	}
	@Override
	public String toString() {
		return "Camera [_id=" + _id + ", name=" + name + ", ip=" + ip
				+ ", channel=" + channel + "]";
	}
	
}
