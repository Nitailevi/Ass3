#include "StompProtocol.h"
#include <iostream>
#include <sstream>
#include "Frame.h"

StompProtocol::StompProtocol(ConnectionHandler& connectionHandler)
    : shouldTerminate(false), connectionHandler(connectionHandler), frameHandler(frameHandler) {}

void StompProtocol::processCommand(const std::string& command) {
    std::istringstream iss(command); // allows going word by word
    std::string action;
    iss >> action; //first word
    
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
    Frame frame(response, *this);
    std::string command = frame.getCommand();

    if (command == "MESSAGE") {
           // Update the summary reports- keep track of the events reported by each user for each channel
    
        Event event(response); // Parse the event from the frame body
        std::lock_guard<std::mutex> lock(reportsMutex); // Ensure thread-safe access
       
        std::string eventOwnerUser = event.getEventOwnerUser();
        std::map<std::string, std::map<std::string, summaryReport>> reports = protocol.getReports(); // Get the reports map
        
        summaryReport& report = reports[channelName][eventOwnerUser]; // Get the report for the user

        // Update statistics- based on gneeral info map
        report.totalReports++; // update count
        if (event.get_general_information().at("active") == "true") {
            report.activeCount++; // update count
        }
        if (event.get_general_information().at("forces_arrival_at_scene") == "true") {
            report.forcesArrivalCount++; // update count
        }

        // Add the event
        report.events.push_back(event);

    




    } else if (command == "RECEIPT") { // after join channel i sent recieptId. the server sends back a recieptId- need to check in map which channel - by odd and even

        std::cout << "Operation acknowledged: " << frame.getHeader("receipt-id") << "\n";


    } else if (command == "ERROR") {
        std::cerr << "Error from server: " << frame.getBody() << "\n";
     } else if (command == "CONNECTED") {
        std::cout << "Login Successful "<< "\n";
    } else {
        std::cerr << "Unknown server response: " << command << "\n";
    }
}


// Getter functions implementation
const std::map<std::string, std::map<std::string, SummaryReport>>& StompProtocol::getReports() const {
    return reports;
}

const std::unordered_map<std::string, int>& StompProtocol::getMapChannelID() const {
    return mapChannelID;
}

const std::unordered_map<std::string, int>& StompProtocol::getMapRecieptID() const {
    return mapRecieptID;
}
int StompProtocol::getSubscriptionId() const {
    return subscriptionId;
}

int StompProtocol::getReceiptUnsubscribe() const {
    return receiptUnsubscribe;
}

int StompProtocol::getReceiptSubscribe() const {
    return receiptsubscribe;
}
bool StompProtocol::getShouldTerminate() const {
    return shouldTerminate;
}
std::mutex& StompProtocol::getReportsMutex() {
    return reportsMutex;
}

//setters
void StompProtocol::setShouldTerminate(bool terminate) {
    shouldTerminate = terminate;
}
void StompProtocol::setSubscriptionId(int id) {
    subscriptionId = id;
}
void StompProtocol::setReceiptUnsubscribe(int id) {
    receiptUnsubscribe = id;
}
void StompProtocol::setReceiptSubscribe(int id) {
    receiptsubscribe = id;
}


