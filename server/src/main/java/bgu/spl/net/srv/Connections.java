package bgu.spl.net.srv;

import java.io.IOException;
import java.util.Map;
import java.util.function.Function;

public interface Connections<T> {

    boolean send(int connectionId, T msg);

    void send(String channel, Function<String, T> messageGenerator);

    void disconnect(int connectionId);


    void subscribe(String channel, int connectionId, String subscriberId);
    void unsubscribe(String subscriberId, int connectionId);
    String authenticate(String login, String passcode, int connectionId);
    void addOrUpdateConnectionHandler(int connectionId, ConnectionHandler<T> handler);

}
