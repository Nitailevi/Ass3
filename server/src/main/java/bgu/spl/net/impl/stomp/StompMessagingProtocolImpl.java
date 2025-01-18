package bgu.spl.net.impl.stomp;
import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;

public class StompMessagingProtocolImpl implements StompMessagingProtocol<String> {

    private boolean shouldTerminate=false;
    private int connectionId;
    private Connections<String> connections;


    public void start(int connectionId, Connections<String> connections){
        this.connectionId=connectionId;
        this.connections=connections;
    }
    
    public void process(String message){
       Frame frame= new Frame(message, connections, connectionId);
       String command =frame.getCommand();

       switch (command) {
        case "CONNECT":
            frame.handleConnect();
            break;
        case "SEND":
            frame.handleSend();
            break;
        case "SUBSCRIBE":
            frame.handleSubscribe();
            break;
        case "UNSUBSCRIBE":
            frame.handleUnSubscribe();
            break;
        case "DISCONNECT":
            frame.handleDisconnect();
            break;
        default:
            frame.handleError("Unknown command");
        }
    }
	
    public boolean shouldTerminate(){
        return shouldTerminate;
    }


}
