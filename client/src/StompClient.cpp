#include <iostream>
#include <thread>
#include <sstream>
#include <string>
#include "ConnectionHandler.h"

class StompClient {
private:
    ConnectionHandler connectionHandler;
    bool isLoggedIn;

public:
    StompClient(const std::string& host, int port)
        : connectionHandler(host, port), isLoggedIn(false) {}

};
