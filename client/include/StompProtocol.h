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

    std::map<std::string, std::map<std::string, summaryReport>> reports; //outer -channel name, inner user name
    std::mutex reportsMutex; // locks reports map
    std::unordered_map<std::string, int> mapChannelID; // Map channel names to IDs
    std::unordered_map<std::string, int> mapRecieptID; // Map   

    int subscriptionId = 1; // Keep track of subscription IDs
    int receiptUnsubscribe = 1; // odd
    int receiptsubscribe = 2; //even
public:
    StompProtocol(ConnectionHandler& connectionHandler); // Constructor

    void sendLoginFrame(const std::string& hostPort, const std::string& username, const std::string& password);

    void sendLogoutFrame();

    // Process user commands like `login`, `join`, `report`, etc.
    void processCommand(const std::string& command);

    // Process server responses like `MESSAGE`, `RECEIPT`, `ERROR`
    void processServerResponse(const std::string& response);

    // Check if the protocol should terminate
    bool shouldTerminateProtocol() const;

    //getters
    const std::map<std::string, std::map<std::string, summaryReport>>& getReports() const; ;
    const std::unordered_map<std::string, int>& getMapChannelID() const;
    const std::unordered_map<std::string, int>& getMapRecieptID() const;
    int getSubscriptionId() const;
    int getReceiptUnsubscribe() const;
    int getReceiptSubscribe() const;
    bool getShouldTerminate() const;
    std::mutex& getReportsMutex();

    //setters
    void setShouldTerminate(bool terminate);
    void setSubscriptionId(int id);
    void setReceiptUnsubscribe(int id);
    void setReceiptSubscribe(int id);
};
struct summaryReport { //structure ment to keep track of events reported by User X for channel Y
    int totalReports = 0;                      // Total number of events reported
    int activeCount = 0;                       // Count of active events
    int forcesArrivalCount = 0;                // Count of events with forces arrival
    std::vector<Event> events;                 // List of all events for the user
};
#endif // STOMPPROTOCOL_H
