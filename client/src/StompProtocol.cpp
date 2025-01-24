#include "StompProtocol.h"
#include <iostream>
#include <sstream>

StompProtocol::StompProtocol(ConnectionHandler& connectionHandler)
    : shouldTerminate(false), connectionHandler(connectionHandler), frameHandler() {}

void StompProtocol::processCommand(const std::string& command) {
    std::istringstream iss(command);
    std::string action;
    iss >> action;

    if (action == "login") {
        std::string hostPort, username, password;
        iss >> hostPort >> username >> password;
        frameHandler.handleConnect(connectionHandler, hostPort, username, password, shouldTerminate);
    } else if (action == "join") {
        std::string channelName;
        iss >> channelName;
        frameHandler.handleSubscribe(connectionHandler, channelName);
    } else if (action == "exit") {
        std::string channelName;
        iss >> channelName;
        frameHandler.handleUnsubscribe(connectionHandler, channelName);
    } else if (action == "report") {
        std::string filePath;
        iss >> filePath;
        frameHandler.handleReport(connectionHandler, filePath);
    } else if (action == "logout") {
        frameHandler.handleDisconnect(connectionHandler, shouldTerminate);
    }
    else if (action == "summary") {
    std::string channelName, user, filePath;
    iss >> channelName >> user >> filePath;
    frameHandler.handleSummary(channelName, user, filePath);
    }
    else {
        std::cout << "Unknown command: " << action << "\n";
    }
}

void StompProtocol::processServerResponse(const std::string& response) {
    Frame frame(response);
    std::string command = frame.getCommand();

    if (command == "MESSAGE") {
        std::cout << "Received message: " << frame.getBody() << "\n";
    } else if (command == "RECEIPT") {
        std::cout << "Operation acknowledged: " << frame.getHeader("receipt-id") << "\n";
    } else if (command == "ERROR") {
        std::cerr << "Error from server: " << frame.getBody() << "\n";
    } else {
        std::cerr << "Unknown server response: " << command << "\n";
    }
}

bool StompProtocol::shouldTerminateProtocol() const {
    return shouldTerminate;
}
