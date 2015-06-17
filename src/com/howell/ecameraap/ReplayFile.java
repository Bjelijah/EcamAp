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
	
	
	public ReplayFile() {
		super();
	}

	public ReplayFile(short begYear, short begMonth, short begDay,
			short begHour, short begMinute, short begSecond, short endYear,
			short endMonth, short endDay, short endHour, short endMinute,
			short endSecond) {
		super();
		this.begYear = begYear;
		this.begMonth = begMonth;
		this.begDay = begDay;
		this.begHour = begHour;
		this.begMinute = begMinute;
		this.begSecond = begSecond;
		this.endYear = endYear;
		this.endMonth = endMonth;
		this.endDay = endDay;
		this.endHour = endHour;
		this.endMinute = endMinute;
		this.endSecond = endSecond;
	}


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
