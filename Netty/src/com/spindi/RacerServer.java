package com.spindi;

import java.net.InetSocketAddress;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.Timer;

import org.jboss.netty.bootstrap.ConnectionlessBootstrap;
import org.jboss.netty.channel.socket.nio.NioDatagramChannelFactory;



public class RacerServer {

	private org.slf4j.Logger log = null;
	private BOStats stats;
	
	public RacerServer(org.slf4j.Logger loggerToUse) {
		log = loggerToUse;
	}
	public void run(final int port) {

		stats = new BOStats();
		Timer t = new Timer();
		t.scheduleAtFixedRate(new StatsTimer(stats), 0, 1000);
		
		
		ConnectionlessBootstrap bs = 
				new ConnectionlessBootstrap(
						new NioDatagramChannelFactory(
								Executors.newCachedThreadPool()));
		
		bs.getPipeline().addLast("decoder", new RacerDecoder(log));
		bs.getPipeline().addLast("handler", new RacerServerHandler(log,stats));
		log.info("UdpRacer starting on port {}", port);
		bs.bind(new InetSocketAddress(port));
		log.info("UdpRacer started on port {}", port);
	}
}
