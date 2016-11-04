package com.spindi;

import java.io.IOException;
import java.net.*;

public class UdpRacer {

	public org.slf4j.Logger logger = null;
	
	public UdpRacer() {
		logger = org.slf4j.LoggerFactory.getLogger(UdpRacer.class);
	}
	public byte[] createMessagePacket(String[] IPs) throws UnknownHostException {
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
		
		final java.nio.ByteBuffer bb = java.nio.ByteBuffer.wrap(new byte[8]);
		bb.putLong(0);
		
		for (String ip : IPs ) {
			InetAddress ip4 = Inet4Address.getByName(ip);
			System.arraycopy(ip4.getAddress(), 0, msg, pos, 4);
			pos += 4;
			
			System.arraycopy(bb.array(),0,msg,pos,8);
			pos += 8;
		}
		
		return msg;
	}
	public void UdpSendBytes(byte[] msg, String IP, int port) throws IOException {
		logger.info("sending packet to IP [{}] Port [{}]",IP,port);
		InetAddress addr = InetAddress.getByName(IP);
		DatagramPacket packet = new DatagramPacket(msg, msg.length,addr, port);
		DatagramSocket dsocket = new DatagramSocket();
		dsocket.send(packet);
		dsocket.close();
	}
	public void inject(String[] hosts, int port) throws UnknownHostException, IOException {
		UdpSendBytes(createMessagePacket(hosts), hosts[0], port);
	}
	public static void main(String[] args) {
		try {
			UdpRacer racer = new UdpRacer();
			
			if ( args.length > 0 && "-inject".equals(args[0]) ) {
				racer.logger.info("injecting packet");
				//String[] hosts =  new String[] { "10.24.107.74","10.1.1.3"};
				
				String[] hosts = new String[args.length-1]; 
				System.arraycopy(args, 1, hosts, 0, args.length-1);
				
				racer.inject(hosts,10000);
				racer.logger.info("injecting packet done");
			}
			else {
				new RacerServer(racer.logger).run(10000);
			}
			
		}
		catch (Exception ex) {
			System.err.println(ex);
		}
		
	}
}
