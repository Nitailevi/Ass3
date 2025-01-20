package bgu.spl.net.impl.stomp;
import java.util.concurrent.atomic.AtomicBoolean;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;

public class StompMessagingProtocolImpl implements StompMessagingProtocol<String> {

    private boolean shouldTerminate=false;
    private int connectionId;
    private Connections<String> connections;
    private AtomicBoolean error;


    public void start(int connectionId, Connections<String> connections){
        this.connectionId=connectionId;
        this.connections=connections;
    }
    
    public void process(String message){
       Frame frame= new Frame(message, connections, connectionId);
       String command =frame.getCommand();

       switch (command) {
        case "CONNECT":
            frame.handleConnect(error);
            if(error != null)shouldTerminate();
            break;
        case "SEND":
            frame.handleSend(error);
            if(error != null)shouldTerminate();
            break;
        case "SUBSCRIBE":
            frame.handleSubscribe(error);
            if(error != null)shouldTerminate();
            break;
        case "UNSUBSCRIBE":
            frame.handleUnSubscribe(error);
            if(error != null)shouldTerminate();
            break;
        case "DISCONNECT":
            frame.handleDisconnect();
            shouldTerminate();
            break;
        default:
            frame.handleError("Unknown command");
            shouldTerminate();
        }
    }
	
    public boolean shouldTerminate(){
        return shouldTerminate;
    }


}
