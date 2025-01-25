#ifndef STOMPPROTOCOL_H
#define STOMPPROTOCOL_H

#include <string>
#include "ConnectionHandler.h"
#include "Frame.h"

class StompProtocol {
private:
    bool shouldTerminate;                // Indicates if the protocol should stop
    ConnectionHandler& connectionHandler; // Reference to the ConnectionHandler
    Frame frameHandler;                  // Handles STOMP frame operations

public:
    StompProtocol(ConnectionHandler& connectionHandler); // Constructor

    // Process user commands like `login`, `join`, `report`, etc.
    void processCommand(const std::string& command);

    // Process server responses like `MESSAGE`, `RECEIPT`, `ERROR`
    void processServerResponse(const std::string& response);

    // Check if the protocol should terminate
    bool shouldTerminateProtocol() const;
};

#endif // STOMPPROTOCOL_H
