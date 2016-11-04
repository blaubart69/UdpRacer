package com.spindi;

import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.net.UnknownHostException;

import org.jboss.netty.buffer.AbstractChannelBuffer;
import org.jboss.netty.buffer.BigEndianHeapChannelBuffer;
import org.jboss.netty.channel.ChannelHandlerContext;
import org.jboss.netty.channel.ExceptionEvent;
import org.jboss.netty.channel.MessageEvent;
import org.jboss.netty.channel.SimpleChannelHandler;

public class RacerServerHandler extends SimpleChannelHandler  {

	org.slf4j.Logger log = null;
	BOStats stats;
	
	public RacerServerHandler(org.slf4j.Logger logger2use, BOStats stats2use) {
		log = logger2use;
		stats = stats2use;
	}
	
	@Override
	public void messageReceived(ChannelHandlerContext ctx, MessageEvent e)
			throws Exception {
		/*
		msg[pos++] = 1;						// 1. byte: version of the protocol
		msg[pos++] = (byte) IPs.length;		// 2. byte: number hosts
		msg[pos++] = 0;						// 3. byte: idx of the host receiving the packet
		*/
		BigEndianHeapChannelBuffer buf = (BigEndianHeapChannelBuffer)e.getMessage();
		byte idx = buf.getByte(2);
		
		stats.messageReceived();
		final long currNanos = System.nanoTime();
		final long lastNanos = getTimeStampByIdx(buf, idx, currNanos);
		stats.setLastRoundTrip( currNanos - lastNanos);
		
		byte numberHosts = buf.getByte(1);
		if ( idx+1 >= numberHosts ) {
			idx = 0;
		}
		else {
			idx++;
		}
		buf.setByte(2, idx);
		
		SocketAddress addr = getHostByIdx(buf, idx);
		e.getChannel().write(buf, addr);
		
	}
	public SocketAddress getHostByIdx(AbstractChannelBuffer buf, byte idx) throws UnknownHostException {
		int pos = 3 + (4+8) * idx;
		byte[] byteaddr = new byte[4];
		buf.getBytes(pos, byteaddr);
		return new InetSocketAddress(InetAddress.getByAddress(byteaddr), 10000);
	}
	public long getTimeStampByIdx(AbstractChannelBuffer buf, byte idx, long NewTimeStamp) {
		final int pos = 3 + (4+8) * idx + 4;
		long nanos = buf.getLong(pos);
		buf.setLong(pos, NewTimeStamp);
		return nanos;
	}
	@Override
	public void exceptionCaught(ChannelHandlerContext ctx, ExceptionEvent e)
			throws Exception {
		// TODO Auto-generated method stub
		super.exceptionCaught(ctx, e);
	}
	
}
