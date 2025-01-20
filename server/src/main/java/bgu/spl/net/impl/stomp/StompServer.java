package bgu.spl.net.impl.stomp;
import java.util.concurrent.atomic.AtomicInteger;

import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.ConnectionsImpl;
import bgu.spl.net.srv.Server;

public class StompServer {

    public static void main(String[] args) {
        if (args.length < 2) {
            System.out.println("Incorrect number of arguments");
            return;
        }

        int port;
        try {
            port = Integer.parseInt(args[0]);
        } catch (NumberFormatException e) {
            System.out.println("Invalid port number: " + args[0]);
            return;
        }

        String serverType = args[1].toLowerCase(); // Make sure the server type is case-insensitive
        Connections<String> connections = new ConnectionsImpl<>();
        AtomicInteger connectionIdGenerator = new AtomicInteger(1);


        if (serverType.equals("tpc")) {
            Server.<String>threadPerClient(
                port,
                () -> {
                    int connectionId = connectionIdGenerator.getAndIncrement();
                    StompMessagingProtocolImpl protocol = new StompMessagingProtocolImpl();
                    protocol.start(connectionId, connections);
                    return protocol;
                }, 
                StompEncoderDecoder::new // Correct Supplier for MessageEncoderDecoder
            ).serve();
        } else if (serverType.equals("reactor")) {
            Server.<String>reactor(
                Runtime.getRuntime().availableProcessors(), // Number of threads available
                port,
                () -> {
                    int connectionId = connectionIdGenerator.getAndIncrement();
                    StompMessagingProtocolImpl protocol = new StompMessagingProtocolImpl();
                    protocol.start(connectionId, connections);
                    return protocol;
                },
                StompEncoderDecoder::new // Correct Supplier for MessageEncoderDecoder
            ).serve();
        } else {
            System.out.println("Unknown server type: " + serverType);
            System.out.println("Valid options are 'reactor' or 'tpc'.");
        }
    }
}