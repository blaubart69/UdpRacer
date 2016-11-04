package com.spindi;

import org.jboss.netty.buffer.ChannelBuffer;
import org.jboss.netty.channel.Channel;
import org.jboss.netty.channel.ChannelHandlerContext;
import org.jboss.netty.handler.codec.frame.FrameDecoder;

public class RacerDecoder extends FrameDecoder {
	
	org.slf4j.Logger log = null;
	
	public RacerDecoder(org.slf4j.Logger logger2use) {
		this.log = logger2use;
	}
	
	@Override
	protected Object decode(ChannelHandlerContext ctx, Channel chan, ChannelBuffer buf) throws Exception {
		
		/*
		final byte[] msg = new byte[3 					// header bytes
		                            + 
		                            (4					// 4 bytes IP address 
		                            + 
		                            8) 					// 4 bytes nanoseconds
		                            * IPs.length];
		
		int pos = 0;
		
		msg[pos++] = 1;						// 1. byte: version of the protocol
		msg[pos++] = (byte) IPs.length;		// 2. byte: number hosts
		msg[pos++] = 0;						// 3. byte: idx of the host receiving the packet
		*/
		
		final int allBytes = buf.readableBytes(); 
		if ( allBytes >= 3 ) {
			byte version = buf.getByte(0);
			byte numberHosts = buf.getByte(1);
			byte idx = buf.getByte(2);
			
			if ( version == 1) {
				final int shouldSize = 3 + (4 + 8) * numberHosts;
				if ( shouldSize == allBytes ) {
					return buf.readBytes(shouldSize);
				}
			}
			else {
				log.error("version [{}] is not supported yet", version);
			}
		}
		
		return null;
	}
}
