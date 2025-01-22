package bgu.spl.net.impl.stomp;
import java.util.concurrent.atomic.AtomicBoolean;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;

public class StompMessagingProtocolImpl implements StompMessagingProtocol<String> {

    private boolean shouldTerminate=false;
    private int connectionId;
    private Connections<String> connections;
    private AtomicBoolean terminate= new AtomicBoolean(false);


    public void start(int connectionId, Connections<String> connections){
        this.connectionId=connectionId;
        this.connections=connections;
    }
    
    public void process(String message){
       Frame frame= new Frame(message, connections, connectionId, terminate);
       frame.handleFrame();
    }
	
    public boolean shouldTerminate(){
        return shouldTerminate;
    }


}
