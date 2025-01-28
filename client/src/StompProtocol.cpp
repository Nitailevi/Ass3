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
      receiptMutex(),
      mapChannelID(),
      mapReceiptID(),
      subscriptionId(1),
      receiptUnsubscribe(1),
      receiptsubscribe(2),
      login(""){}

void StompProtocol:: sendLoginFrame(const std::string& hostPort, const std::string& login, const std::string& passcode){ 
    Frame frame(*this);
    this->login = login;
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

        std::cout << "recived MESSAGE frame1 " << std::endl;
        std::cout << response << std::endl;

    
        Event event(response); // Parse the event from the frame body

        std::cout << "recived MESSAGE frame2 " << std::endl;

        std::lock_guard<std::mutex> lock(reportsMutex); // Ensure thread-safe access

        std::cout << "recived MESSAGE frame3 " << std::endl;
       
        std::string channelName = event.get_channel_name();
        std::string eventOwnerUser = event.getEventOwnerUser();

        std::cout << "recived MESSAGE frame4 " << std::endl;
        std::cout <<"channelName "+ channelName << std::endl;
        std::cout <<"eventOwnerUser "+ eventOwnerUser << std::endl;
        
        reports[channelName];
        summaryReport& report = reports[channelName][eventOwnerUser]; // Get the report for the user

        std::cout << "recived MESSAGE frame5 " << std::endl;

        // Update statistics- based on gneeral info map
        report.totalReports++; // update count

        std::cout << "received MESSAGE frame6 " << std::endl;
        auto generalInfo1 = event.get_general_information();
        auto it1 = generalInfo1.find("active");
        if (it1 != generalInfo1.end() && it1->second == "true") {
            std::cout<<"active count "+report.activeCount<<std::endl;
            report.activeCount++; // Update count
        }

        std::cout << "received MESSAGE frame7 " << std::endl;
        auto generalInfo2 = event.get_general_information();
        auto it2 = generalInfo2.find("forces_arrival_at_scene");
        if (it2 != generalInfo2.end()) {
            std::cout << "no issue on general info " << std::endl;
        }
        if( it2->second == "true"){
            std::cout << "no issue on value " << std::endl;
        }
        std::cout << "generalInfo2 contents:" << std::endl;

        //check map need to delete
         for (const auto& pair : generalInfo2) {
            std::cout << "iterate on map" << pair.first << ": " << pair.second << std::endl;
        }

        if (it2 != generalInfo2.end() && it2->second == "true") {
            std::cout<<"forcesArrivalCount "+report.forcesArrivalCount<<std::endl;
            report.forcesArrivalCount++; // Update count
        }  

        std::cout << "recived MESSAGE frame8 " << std::endl;
        // Add the curr event  to the vector of events
        report.events.push_back(event);

    } else if (command == "RECEIPT") { // after join channel i sent recieptId. the server sends back a recieptId- need to check in map which channel - by odd and even
        std::string recieptId = frame.getHeader("receipt-id");

        std::unique_lock<std::mutex> lock(receiptMutex);

        auto it = mapReceiptID.find(std::stoi(recieptId));
        std::string channelName = (it != mapReceiptID.end()) ? it->second : "Unknown channel";
        if (std::stoi(recieptId) % 2 == 0 && channelName != "Unknown channel") {
            std::cout << "Joined channel "+channelName << "\n";
        } else if (std::stoi(recieptId) % 2 == 1 && channelName != "Unknown channel") {
            std::cout << "Exited channel " << channelName << std::endl;
        } else {
            std::cerr << "Unknown receipt ID: " +channelName << "\n";
        }
        lock.unlock();

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
  std::map<std::string, std::map<std::string, summaryReport>>& StompProtocol::getReports()  {
    return reports;
}

 std::unordered_map<std::string, int>& StompProtocol::getMapChannelID()  {
    return mapChannelID;
}

std::unordered_map<int, std::string>& StompProtocol::getMapReceiptID() {
    return mapReceiptID;
}
int StompProtocol::getandIncrementSubscriptionId() {
    subscriptionId++;
    return subscriptionId;
}

int StompProtocol::getandIncrementReceiptUnsubscribe(){
    receiptUnsubscribe=receiptUnsubscribe+2;
    return receiptUnsubscribe;
}

int StompProtocol::getandIncrementReceiptSubscribe(){
    receiptsubscribe=receiptsubscribe+2;
    return receiptsubscribe+2;
}

bool StompProtocol::getShouldTerminate() const {
    return shouldTerminate;
}

std::mutex& StompProtocol::getReportsMutex() {
    return reportsMutex;
}

std::mutex& StompProtocol::getReceiptMutex() {
    return receiptMutex;
}
//setters
void StompProtocol::setShouldTerminate(bool terminate) {
    shouldTerminate = terminate;
}

const std::string StompProtocol::getLogin() const{
    return login;
}



