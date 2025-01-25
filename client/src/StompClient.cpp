#include <iostream>
#include <thread>
#include <atomic>
#include "ConnectionHandler.h"
#include "StompProtocol.h"
#include "Frame.h"

//rule of 5???
void keyboardInputHandler(StompProtocol& protocol, std::atomic<bool>& shouldTerminate) {
    std::string command;
    while (!shouldTerminate) {
        std::getline(std::cin, command); //get command
        protocol.processCommand(command);
        if (protocol.getShouldTerminate()) {
            shouldTerminate = true;
        }
    }
}

void serverResponseHandler(ConnectionHandler& connectionHandler, StompProtocol& protocol, std::atomic<bool>& shouldTerminate) {
    while (!shouldTerminate) { 
        std::string response;
        if (!connectionHandler.getLine(response)) { //check if the connection is still open
            std::cerr << "Disconnected from server\n";
            shouldTerminate = true;
            break;
        }
        protocol.processServerResponse(response);  //process the response
        if (protocol.getShouldTerminate()) {
            shouldTerminate = true;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <host> <port>\n";
        return 1;
    }

    std::string host = argv[1];
    short port = std::stoi(argv[2]);

    ConnectionHandler connectionHandler(host, port);
    if (!connectionHandler.connect()) { //connect to the server
        std::cerr << "Failed to connect to " << host << ":" << port << "\n";
        return 1;
    }

    StompProtocol protocol(connectionHandler);
    std::atomic<bool> shouldTerminate(false);
   
    // init threads
    std::thread keyboardThread(keyboardInputHandler, std::ref(protocol), std::ref(shouldTerminate));
    std::thread serverThread(serverResponseHandler, std::ref(connectionHandler), std::ref(protocol), std::ref(shouldTerminate));
    //synch with main
    keyboardThread.join();
    serverThread.join();

    return 0;
}