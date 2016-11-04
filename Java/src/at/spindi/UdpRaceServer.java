package at.spindi;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.channels.DatagramChannel;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Iterator;

public class UdpRaceServer implements Runnable {

	@Override
	public void run() {
		try {
			Selector selector = Selector.open();
			DatagramChannel dgmChannel = DatagramChannel.open();
			InetSocketAddress isa = new InetSocketAddress(11111);
			dgmChannel.socket().bind(isa);
			dgmChannel.configureBlocking(false);
			
			SelectionKey key = dgmChannel.register(selector, SelectionKey.OP_READ);
			//key.attach(arg0)
			while (true) {
				try {
					selector.select();
					Iterator<SelectionKey> selectedKeysIter = selector.selectedKeys().iterator();
					while ( selectedKeysIter.hasNext() ) {
						SelectionKey currKey = selectedKeysIter.next();
						selectedKeysIter.remove();
						
						if ( !currKey.isValid() )
							continue;
						
						 if (key.isReadable()) {
                             read(key);
                             key.interestOps(SelectionKey.OP_WRITE);
                         } else if (key.isWritable()) {
                             write(key);
                             key.interestOps(SelectionKey.OP_READ);
                         }						
					}
				}
				catch (IOException ex) {
					System.err.println("X: IO Exception: " + ex.getMessage());
				}

			}
		}
		catch (Exception ex) {
			System.err.println("X: " + ex.getMessage());
		}
	}
	private void read(SelectionKey key) throws IOException {
        DatagramChannel chan = (DatagramChannel)key.channel();
        Con con = (Con)key.attachment();
        con.sa = chan.receive(con.req);
        System.out.println(new String(con.req.array(), "UTF-8"));
        con.resp = Charset.forName( "UTF-8" ).newEncoder().encode(CharBuffer.wrap("send the same string"));
    }

    private void write(SelectionKey key) throws IOException {
        DatagramChannel chan = (DatagramChannel)key.channel();
        Con con = (Con)key.attachment();
        chan.send(con.resp, con.sa);
    }

}
