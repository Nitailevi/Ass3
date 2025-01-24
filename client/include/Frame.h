#ifndef FRAME_H
#define FRAME_H

#include <string>
#include <unordered_map>

class Frame {
private:
    std::string command;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    std::map<std::string, std::map<std::string, summaryReport>> reports; //outer -channel name, inner user name
    std::mutex reportsMutex; // locks reports map
public:
    // Constructors
    Frame(); // Default constructor
    Frame(const std::string& rawFrame); // Parse a raw frame
    Frame(const std::string& command, const std::unordered_map<std::string, std::string>& headers, const std::string& body);
    int subscriptionId = 1; // Keep track of subscription IDs
    int receiptId = 1; // Keep track of receipt IDs
    std::map<int, Event> eventsStorage; // Store events for summary

    // Getters
    std::string getCommand() const;
    std::string getHeader(const std::string& key) const;
    std::string getBody() const;
    const std::map<int, Event>& geteventsStorage() const;


    // Frame operations
    void handleConnect(class ConnectionHandler& connectionHandler, const std::string& hostPort, const std::string& username, const std::string& password, bool& shouldTerminate);
    void handleSubscribe(class ConnectionHandler& connectionHandler, const std::string& channelName);
    void handleUnsubscribe(class ConnectionHandler& connectionHandler, const std::string& channelName);
    void handleReport(class ConnectionHandler& connectionHandler, const std::string& filePath);
    void handleDisconnect(class ConnectionHandler& connectionHandler, bool& shouldTerminate);
    void handleError(class ConnectionHandler& connectionHandler, const std::string& errorMessage);
    void handleSummary(const std::string& channelName, const std::string& user, const std::string& filePath);
    std::string epochToDate(const int time);

    // Convert to string (for sending frames)
    std::string toString() const;
};
struct summaryReport { //structure ment to keep track of events reported by User X for channel Y
    int totalReports = 0;                      // Total number of events reported
    int activeCount = 0;                       // Count of active events
    int forcesArrivalCount = 0;                // Count of events with forces arrival
    std::vector<Event> events;                 // List of all events for the user
};
#endif // FRAME_H
