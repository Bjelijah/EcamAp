package com.howell.ecameraap;

import java.io.Serializable;

public class ReplayFile implements Serializable{
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	public short begYear;
	public short begMonth;
	public short begDay;
	public short begHour;
	public short begMinute;
	public short begSecond;
	public short endYear;
	public short endMonth;
	public short endDay;
	public short endHour;
	public short endMinute;
	public short endSecond;
	@Override
	public String toString() {
		return "ReplayFile [begYear=" + begYear + ", begMonth=" + begMonth
				+ ", begDay=" + begDay + ", begHour=" + begHour
				+ ", begMinute=" + begMinute + ", begSecond=" + begSecond
				+ ", endYear=" + endYear + ", endMonth=" + endMonth
				+ ", endDay=" + endDay + ", endHour=" + endHour
				+ ", endMinute=" + endMinute + ", endSecond=" + endSecond + "]";
	}
	
}
