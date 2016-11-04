package com.spindi;

import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.TimerTask;


public class StatsTimer extends TimerTask {

	private BOStats stat;
	SimpleDateFormat sdfDate;
	
	public StatsTimer(BOStats stat2use) {
		stat = stat2use;
		sdfDate = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
	}
	@Override
	public void run() {
		
		String timestr = sdfDate.format(Calendar.getInstance().getTime());
		
		System.out.printf("%s: packets/s [%d] | roundtrip nanos [%d]\r",
				timestr, 
				stat.MessagesPerSecond,
				stat.LastRoundtrip);
		
		stat.SecondTick();
	}

}
