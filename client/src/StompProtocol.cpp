#include <iostream>
#include <sstream>
#include "../include/StompProtocol.h"
#include "../include/event.h"
#include "../include/Frame.h"


StompProtocol::StompProtocol(ConnectionHandler& handler)
    : shouldTerminate(false),
      connectionHandler(handler),
      reports(),
      reportsMutex(),
      mapChannelID(),
      mapRecieptID(),
      subscriptionId(1),
      receiptUnsubscribe(1),
      receiptsubscribe(2){}

void StompProtocol:: sendLoginFrame(const std::string& hostPort, const std::string& login, const std::string& passcode){ 
    Frame frame(*this);
    frame.handleConnect(connectionHandler, hostPort, login, passcode, shouldTerminate);
}
   
   
void StompProtocol:: sendLogoutFrame(){
    Frame frame(*this);
    frame.handleDisconnect(connectionHandler,shouldTerminate);
}

void StompProtocol::processCommand(const std::string& command) {
    std::istringstream iss(command); // allows going word by word
    std::string action;
    iss >> action; //first word
    
    if (action == "join") {
        Frame frame(*this);
        std::string channelName;
        iss >> channelName;
        frame.handleSubscribe(connectionHandler, channelName);
    } else if (action == "exit") {
        Frame frame(*this);
        std::string channelName;
        iss >> channelName;
        frame.handleUnsubscribe(connectionHandler, channelName);
    } else if (action == "report") {
        Frame frame(*this);
        std::string filePath;
        iss >> filePath;
        frame.handleReport(connectionHandler, filePath);
    }
    else if (action == "summary") {
    Frame frame(*this);
    std::string channelName, user, filePath;
    iss >> channelName >> user >> filePath;
    frame.handleSummary(channelName, user, filePath);
    }
    else {
        std::cout << "Illegal command, please try a different one" << "\n"; 
    }
}

void StompProtocol::processServerResponse(const std::string& response) {
    Frame frame(response, *this);
    std::string command = frame.getCommand();

    if (command == "MESSAGE") {
           // Update the summary reports- keep track of the events reported by each user for each channel
    
        Event event(response); // Parse the event from the frame body
        std::lock_guard<std::mutex> lock(reportsMutex); // Ensure thread-safe access
       
        std::string channelName = event.get_channel_name();
        std::string eventOwnerUser = event.getEventOwnerUser();
        
        summaryReport& report = reports[channelName][eventOwnerUser]; // Get the report for the user

        // Update statistics- based on gneeral info map
        report.totalReports++; // update count
        if (event.get_general_information().at("active") == "true") {
            report.activeCount++; // update count
        }
        if (event.get_general_information().at("forces_arrival_at_scene") == "true") {
            report.forcesArrivalCount++; // update count
        }

        // Add the curr event  to the vector of events
        report.events.push_back(event);

    } else if (command == "RECEIPT") { // after join channel i sent recieptId. the server sends back a recieptId- need to check in map which channel - by odd and even

        if (std::stoi(frame.getHeader("receipt-id")) % 2 == 0) {
            std::cout << "Joined channel : " << frame.getHeader("receipt-id") << "\n";
            mapChannelID[frame.getHeader("receipt-id")] = subscriptionId;
            subscriptionId++;
        } else if (std::stoi(frame.getHeader("receipt-id")) % 2 == 1) {
            std::cout << "Exited channel : " << frame.getHeader("receipt-id") << "\n";
        } else {
            std::cerr << "Unknown receipt ID: " << frame.getHeader("receipt-id") << "\n";
        }


    } else if (command == "ERROR") {
        std::cerr << "ERROR FROM SERVER" << "\n" << "\n"<< "ERROR"<< "\n" << "message:" +frame.getBody() << "\n";
        shouldTerminate = true;
     } else if (command == "CONNECTED") {
        std::cout << "Login Successful "<< "\n";
    } else {
        std::cerr << "Unknown server response: " << command << "\n";
    }
}


// Getter functions implementation
 const std::map<std::string, std::map<std::string, summaryReport>>& StompProtocol::getReports() const {
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


