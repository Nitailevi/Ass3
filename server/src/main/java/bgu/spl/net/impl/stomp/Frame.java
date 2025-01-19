package bgu.spl.net.impl.stomp;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.atomic.AtomicInteger;

import bgu.spl.net.srv.Connections;

public class Frame {

    private final String command; 
    private final Map<String, String> headers;
    private final String body;
    private final Connections<String> connections;
    private final int connectionId;
    private static final AtomicInteger messageCounter = new AtomicInteger(1);

    public Frame(String message, Connections<String> connections, int connectionId){
        this.connections=connections;
        this.connectionId=connectionId;

        String[] parts = message.split("\n\n", 2); // Seperate between 2 parts command, headers [0] and body [1]
        String[] lines = parts[0].split("\n"); // Seperate between command [0] and headers [1]

        this.command=lines[0];
        this.body = parts.length > 1 ? parts[1] : "";

        headers=new HashMap<>();
        for(int i=1; i<lines.length; i++ ){
            String[] headerParts = lines[i].split(":", 2);
            if(headerParts.length==2)
                headers.put(headerParts[0].trim(), headerParts[1].trim());
        }
    }

    public String getCommand(){
        return command;
    }

    public void handleConnect() {
        String acceptVersion = headers.get("accept-version");
        String host = headers.get("host");
        String login = headers.get("login");
        String passcode = headers.get("passcode");

        if (acceptVersion == null || !acceptVersion.equals("1.2")) {
            handleError("Unsupported STOMP version. Only version 1.2 is supported");
            return;
        }
    
        if (host == null || !host.equals("stomp.cs.bgu.ac.il")) {
            handleError("Invalid host. Expected stomp.cs.bgu.ac.il");
            return;
        }
    
        if (login == null || passcode == null) {
            handleError("Missing login or passcode");
            return;
        }
    
        
        String message=connections.authenticate(login,passcode, connectionId);
        if (!(message.equals(""))) { // Check if user is already logged in
            handleError(message);
            return;
        }
    
        connections.send(connectionId, "CONNECTED\nversion:1.2\n\n\u0000");
    }

    public void handleSend() {
        String destination = headers.get("destination");
    
        if (destination == null) {
            handleError("Missing 'destination' header in SEND frame.");
            return;
        }
        String messageId = generateMessageId();
        connections.send(destination, subscriberId -> { // Broadcast to all subscribers of the destination
            return "MESSAGE\n" +
                   "subscription:" + subscriberId + "\n" +
                   "message-id:" + messageId + "\n" +
                   "destination:" + destination + "\n\n" +
                   body + "\u0000";
        });
    }


    public void handleSubscribe() {
        String destination = headers.get("destination");
        String subscriberId = headers.get("id");
    
        if (destination == null || subscriberId == null) {
            handleError("Missing 'destination' or 'id' header in SUBSCRIBE frame");
            return;
        }
    
        connections.subscribe(destination, connectionId, subscriberId);  // Subscribe the client to the channel
    
        String receipt = headers.get("receipt");
        if (receipt != null) {
            connections.send(connectionId, "RECEIPT\nreceipt-id:" + receipt + "\n\n\u0000");
        }
    }

    public void handleUnSubscribe() {
        String subscriberId = headers.get("id");
    
        if (subscriberId == null) {
            handleError("Missing 'id' header in SUBSCRIBE frame");
            return;
        }
        connections.unsubscribe(subscriberId, connectionId); // Unsubscribe the client to the channel
    
        String receipt = headers.get("receipt");
        if (receipt != null) {
            connections.send(connectionId, "RECEIPT\nreceipt-id:" + receipt + "\n\n\u0000");
        }
    }

    public void handleDisconnect(){
        connections.disconnect(connectionId);

        String receipt = headers.get("receipt");
        if (receipt != null) {
            connections.send(connectionId, "RECEIPT\nreceipt-id:" + receipt + "\n\n\u0000");
        }

    }
    
    private String generateMessageId() {
        return String.valueOf(messageCounter.getAndIncrement());
    }
    
    public void handleError(String errorMessage) {
        String errorFrame = "ERROR\nmessage:" + errorMessage + "\n\n\0";
        connections.send(connectionId, errorFrame);

        //need to add additional headears
        //need to close the socket afet send error frame
    //    connections.disconnect(connectionId); 
    }


}
