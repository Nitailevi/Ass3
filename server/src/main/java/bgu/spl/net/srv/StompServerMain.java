package main.java.bgu.spl.net.srv;

import bgu.spl.net.srv.Server;

public class StompServerMain {

    public static void main(String[] args) {
        if (args.length < 2) {
            System.out.println("incoorect number of arguments");
            return;
        }

        int port;
        try {
            port = Integer.parseInt(args[0]);
        } catch (NumberFormatException e) {
            System.out.println("Invalid port number: " + args[0]);
            return;
        }

        String serverType = args[1].toLowerCase(); //make sure the server type is case-insensitive

        if (serverType.equals("tpc")) {
            Server.threadPerClient(
                port,
                new StompProtocol(),
                new StompEncoderDecoder()
            ).serve(); //init
        } else if (serverType.equals("reactor")) {
            Server.reactor(
                Runtime.getRuntime().availableProcessors(), //number of threads available
                port, 
                new StompProtocol(),
                new StompEncoderDecoder()
            ).serve(); //init
        } else {
            System.out.println("Unknown server type: " + serverType);
            System.out.println("Valid options are 'reactor' or 'tpc'.");
        }
    }
}
