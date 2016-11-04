package com.spindi;

public class BOStats {

	public volatile long CountMessages = 0;
	public volatile long MessagesPerSecond = 0;
	public volatile long LastRoundtrip = 0;
	
	public void messageReceived() {
		CountMessages++;
		MessagesPerSecond++;
	}
	public void SecondTick() {
		MessagesPerSecond=0;
	}
	public void setLastRoundTrip(long nanos) {
		LastRoundtrip = nanos;
	}
}
