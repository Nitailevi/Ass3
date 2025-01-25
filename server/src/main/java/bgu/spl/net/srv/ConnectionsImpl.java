package bgu.spl.net.srv;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.function.Function;

public class ConnectionsImpl<T> implements Connections<T> {

    private final Map<Integer, ConnectionHandler<T>> activeClients = new ConcurrentHashMap<>(); // Maps each connectionId to its active client
    private final Map<String, Map<Integer, String>> channelSubscriptions = new ConcurrentHashMap<>(); // Maps each channel (String) to its subscribed clients connectionId (Integer) and their subscriberId (Integer)
    private final Map<String, String> users = new ConcurrentHashMap<>(); // Storage all users data

    //maybe we will change to different data structure withou save the connections id
    private final Map<String, Integer> loggedInUsers = new ConcurrentHashMap<>(); // Storage all loggedin users each login(String) and its own connectionId (Integer)

    public ConnectionsImpl() {}

    @Override
    public boolean send(int connectionId, T msg) {
        ConnectionHandler<T> handler = activeClients.get(connectionId); // Getting ConnectionHandler that matches the given connectionId in the activeClients map
        if (handler != null) {
            handler.send(msg);
            return true;
        }
        return false;
    }

    public void send(String channel, Function<String, T> messageGenerator) {
        Map<Integer, String> subscribers = channelSubscriptions.get(channel);
        if (subscribers != null) {
            for (Map.Entry<Integer, String> entry : subscribers.entrySet()) {  // Iterate the map to get connectionId and subscriberId
                Integer connectionId = entry.getKey();
                String subscriberId = entry.getValue();
                T message = messageGenerator.apply(subscriberId);
                send(connectionId, message); 
            }
        }
    }
    @Override
    public void disconnect(int connectionId) {
        for (Map<Integer, String> subscribers : channelSubscriptions.values()) {
            subscribers.remove(connectionId); // Remove the client from all subscribed channels
        }
        activeClients.remove(connectionId); // Remove the client from active connections
        loggedInUsers.entrySet().removeIf(entry -> entry.getValue() == connectionId); // Find and remove the user from loggedInUsers
                                            
        activeClients.remove(connectionId);
        // ConnectionHandler<T> handler = activeClients.remove(connectionId);
        // if (handler != null) {
        //     try {
        //         handler.close(); // Close the connection handler
        //     } catch (IOException e) {
        //         System.err.println("Error closing connection for ID " + connectionId + ": " + e.getMessage());
        //     }
        // }
    }

    public void subscribe(String channel, int connectionId, String subscriberId) {
        channelSubscriptions.computeIfAbsent(channel, k -> new ConcurrentHashMap<>()).put(connectionId, subscriberId); // Add the client to the channel's map
    }
    
    public void unsubscribe(String subscriberId, int connectionId) {
        for (Map.Entry<String, Map<Integer, String>> entry : channelSubscriptions.entrySet()) { // Iterate all channels
            Map<Integer, String> subscribers = entry.getValue();
            String channel= entry.getKey();
            Integer connectionIdToRemove = null;
            for (Map.Entry<Integer, String> subscriberEntry : subscribers.entrySet()) {
                if (connectionId== subscriberEntry.getKey() && subscriberId.equals(subscriberEntry.getValue())) { // Find the connectionId associated with the given subscriberId
                    connectionIdToRemove = subscriberEntry.getKey();
                    break;
                }
            }
    
            if (connectionIdToRemove != null) {
                subscribers.remove(connectionIdToRemove); // If found this connectionId remove it
               if (subscribers.isEmpty()) 
                  channelSubscriptions.remove(channel); // If the channel is now empty, remove it entirely
                break; // Subscriber ID for client is unique 
            }
        }
    }

    public String authenticate(String login, String passcode, int connectionId){
        System.out.println("Received authentication request for login: " + login);
        if(loggedInUsers.containsKey(login)){
            System.out.println("Client already logged in: " + login);

            return "The client already logged in, log out before trying again";
        } else {
            users.putIfAbsent(login, passcode);
            if (!users.get(login).equals(passcode)) {
                System.out.println("Wrong password for login: " + login);

                return "Wrong Password";
            }
            loggedInUsers.put(login, connectionId);
            
            System.out.println("Client authenticated and registered with connectionId: " + connectionId);
            return "no error";  
        }
    }
    public void addOrUpdateConnectionHandler(int connectionId, ConnectionHandler<T> handler) {
        activeClients.put(connectionId, handler);
    }
}
