import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.DatagramChannel;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Iterator;

class UdpRaceServer implements Runnable {

    @Override
    public void run() {
        try {
            DatagramChannel dgmChannel = DatagramChannel.open();
            dgmChannel.socket().bind(new InetSocketAddress(4444));
            System.out.println("listening on port 4444");
            dgmChannel.configureBlocking(false);

            Selector selector = Selector.open();
            dgmChannel.register(selector, SelectionKey.OP_READ );

            ByteBuffer buffer = ByteBuffer.allocate(1024);
            buffer.order(ByteOrder.LITTLE_ENDIAN);
            byte[] ipBytes = new byte[4];


            for(;;) {
                if ( selector.select() == 0 ) {
                    continue;
                }

                Iterator iterator = selector.selectedKeys().iterator();
                while (iterator.hasNext()) {
                    SelectionKey key = (SelectionKey)iterator.next();
                    iterator.remove();

                    if (key.isReadable()) {
                        DatagramChannel kanal = (DatagramChannel)key.channel();
                        SocketAddress remote = kanal.receive(buffer);
                        System.out.printf("got %d bytes from %s\n", buffer.position(), remote.toString());
                        //  0 id
                        //  4 hops
                        //  8 next
                        // 12 number of Hosts
                        int magic = buffer.getInt(0);
                        int hops = buffer.getInt(4);
                        buffer.putInt(4, hops + 1);

                        int currIdx     = buffer.getInt(8);
                        int numberHosts = buffer.getInt(12);
                        int nextIPidx = ( currIdx  + 1 ) % numberHosts ;
                        buffer.putInt(8, nextIPidx);

                        buffer.get(16 + (nextIPidx * 4), ipBytes, 0, 4);
                        InetAddress IPtoSend = InetAddress.getByAddress(ipBytes);
                        InetSocketAddress endpoint = new InetSocketAddress(IPtoSend, 4444);
                        System.out.printf("hops(%d) nextIPidx(%d) nextHost(%s)\n", hops, nextIPidx, endpoint.toString());

                        buffer.flip();
                        kanal.send(buffer, endpoint);
                    }
                }
            }
        }
        catch (Exception ex) {
            System.err.println("X: " + ex.getMessage());
            ex.printStackTrace();
        }
    }
}
