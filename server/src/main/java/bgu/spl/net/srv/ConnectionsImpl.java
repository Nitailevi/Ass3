package bgu.spl.net.srv;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class ConnectionsImpl<T> implements Connections<T> {

    private Map<Integer, ConnectionHandler<T>> activeClients = new ConcurrentHashMap<>(); // Maps each connectionId to its active client
    private final Map<String, Map<Integer, Integer>> channelSubscriptions = new ConcurrentHashMap<>(); // Maps each channel (String) to its subscribed clients (connectionId) and their subscriberId

    public ConnectionsImpl() {
    }

    public boolean send(int connectionId, T msg) {
        ConnectionHandler<T> handler = activeClients.get(connectionId); // Getting ConnectionHandler that matches the given connectionId in the activeClients map
        if (handler != null) {
            handler.send(msg);
            return true;
        }
        return false;
    }

    public void send(String channel, T msg) {
        Map<Integer, Integer> subscribers = channelSubscriptions.get(channel);
        if (subscribers != null) {
            for (Integer connectionId : subscribers.keySet()) { // Iterate through each client that subscri
                send(connectionId, msg);
            }
        }
    }
    

    public void disconnect(int connectionId) {
        for (Map<Integer, Integer> subscribers : channelSubscriptions.values()) { 
            subscribers.remove(connectionId); // Remove the client from all subscribed channels
        }
        activeClients.remove(connectionId); // Remove the client from active connections
    }
}
