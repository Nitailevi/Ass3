#include <iostream>
#include <thread>
#include <atomic>
#include "../include/ConnectionHandler.h"
#include "../include/StompProtocol.h"
#include "../include/Frame.h"

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

int main() {
    std::atomic<bool> shouldTerminate(false);
    bool loggedIn = false;
    std::thread serverThread;
    
    while (!shouldTerminate) {
        std::string command;
        std::getline(std::cin, command);
        std::istringstream iss(command); // allows going word by word
        std::string action;
        iss >> action; //first word

        if (action == "login") {
           
            if (loggedIn) {
                std::cout << "user already logedin" << std::endl;
                continue;
            }
            std::string hostPort, login, passcode;
            if (!(iss >> hostPort >> login >> passcode)) {
                std::cout << "login command needs 3 args: {host:port} {username} {password}"<< std::endl;
                continue;
            }
            
            size_t colonPos = hostPort.find(':');
            if (colonPos == std::string::npos) {
                std::cout << "host:port are illegal"<< std::endl;
                continue;
            }
            std::string host = hostPort.substr(0, colonPos);
            int port = std::stoi(hostPort.substr(colonPos + 1));

             std::cout << "Starting to connect to " << hostPort << std::endl;
            ConnectionHandler connectionHandler(host, port);
            if (!connectionHandler.connect()) { //connect to the server
                std::cout <<"Connection failed (Error: Invalid argument)\n" "Cannot connect to " << host << ":" << port << "please try to login again"<< std::endl;
                continue;
            }

            StompProtocol protocol(connectionHandler);
            protocol.sendLoginFrame(hostPort, login, passcode);

            std::thread serverThread (serverResponseHandler, std::ref(connectionHandler), std::ref(protocol), std::ref(shouldTerminate));
            loggedIn = true;

            while (!shouldTerminate) { 
                std::getline(std::cin, command);
                if (command == "logout") {
                    protocol.sendLogoutFrame(); 
                    shouldTerminate = true; 
                    loggedIn = false;
                } else {
                    protocol.processCommand(command);
                }
            }

            if (serverThread.joinable())
                serverThread.join();

        } else if (!loggedIn) {
            std::cout << "please login first" << std::endl;
            continue;
        }
        

       if (serverThread.joinable())
           serverThread.join();
    }
    return 0;
}