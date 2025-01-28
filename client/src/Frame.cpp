#include "../include/Frame.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include "../include/ConnectionHandler.h"
#include "../include/event.h"
#include "../include/StompProtocol.h"


// Default constructor
 Frame::Frame(StompProtocol& protocol) : command(""), headers(), body(""), protocol(protocol){}


// Parse a raw frame = first line of frame
Frame::Frame(const std::string& rawFrame, StompProtocol& protocol)
    : command(), headers(), body(),protocol(protocol) { 
    std::istringstream stream(rawFrame);

    // Parse command
    std::getline(stream, command); // First line is the command

    // Parse headers
    std::string line;
    while (std::getline(stream, line) && !line.empty()) {
        size_t delimiter = line.find(':');
        if (delimiter != std::string::npos) {
            std::string key = line.substr(0, delimiter);
            std::string value = line.substr(delimiter + 1);
            headers[key] = value;
        }
    }

    std::getline(stream, body, '\0'); // Read until the null character
}

// Construct a frame
Frame::Frame(const std::string& command, const std::unordered_map<std::string, std::string>& headers, const std::string& body, StompProtocol& protocol)
    : command(command), headers(headers), body(body), protocol(protocol) {}

// Getters
std::string Frame::getCommand() const {
    return command;
}

std::string Frame::getHeader(const std::string& key) const {
    try {
        return headers.at(key);
    } catch (const std::out_of_range&) {
        return "no such key";
    }
}

std::string Frame::getBody() const {
    return body;
}

// Convert the frame to a string
std::string Frame::toString() const {
    std::ostringstream frame;
    frame << command << "\n";
    for (const std::pair<const std::string, std::string>& header : headers) {
        frame << header.first << ":" << header.second << "\n";
    }
    frame << "\n" << body << "\0";
    return frame.str();
}
// Handle CONNECT frame
void Frame::handleConnect(ConnectionHandler& connectionHandler, const std::string& hostPort, const std::string& username, const std::string& password, bool& shouldTerminate) {
    // Build CONNECT frame
    std::unordered_map<std::string, std::string> headers = {
        {"accept-version", "1.2"},
        {"host", "stomp.cs.bgu.ac.il"},
        {"login", username},
        {"passcode", password}
    };

    Frame connectFrame("CONNECT", headers, "", protocol);

    // Send frame using ConnectionHandler
    std::string frameString = connectFrame.toString(); // build the frame with tostring
    
    if (!connectionHandler.sendLine(frameString)) { // send the frame and check if it was sent- maybe DELETE
        std::cerr << "Failed to send CONNECT frame.\n";
        shouldTerminate = true;
    }
    else {
        std::cout << "Connected to " << hostPort << std::endl;
        std::cout <<frameString<< std::endl;
    }
}

// Handle SUBSCRIBE frame
void Frame::handleSubscribe(ConnectionHandler& connectionHandler, const std::string& channelName) {
   
    // make sure no double-subs
    std::unordered_map<int, std::string>& mapRecieptID = protocol.getMapRecieptID();
    std::unordered_map<std::string, int>& mapChannelID = protocol.getMapChannelID();

      if (mapChannelID.find(channelName) != mapChannelID.end()) {
             std::cerr << "Channel " << channelName << " is already subscribed with ID: " << mapChannelID[channelName] << "\n";  //check what to print
             return;
        }

    // Add the channel and ID to the map
    
    int subscriptionId = protocol.getandIncrementSubscriptionId(); // Increment the subscription ID
    
    int recieptsubscribe = protocol.getandIncrementReceiptSubscribe();

    std::unordered_map<std::string, std::string> headers = {
        {"destination", channelName},
        {"id", std::to_string(subscriptionId)}, // maybe need to wait for ticket?
        {"receipt", std::to_string(recieptsubscribe)}
    };

// check 
     std::cerr << "channel id map size before insert: " << mapChannelID.size() << "\n";
    std:: cerr << "reciept id map size before insert: " << mapRecieptID.size() << "\n";

    mapChannelID[channelName]=subscriptionId;
    mapRecieptID[recieptsubscribe] = channelName;
    
    // check 
    std::cerr << "channel id map size after insert: " << mapChannelID.size() << "\n";
    std:: cerr << "reciept id map size after insert: " << mapRecieptID.size() << "\n";

    Frame subscribeFrame("SUBSCRIBE", headers, "", protocol);

    // Send frame
    std::string frameString = subscribeFrame.toString();
    if (!connectionHandler.sendLine(frameString)) {
        std::cerr << "Failed to send SUBSCRIBE frame"<< "\n";
    }

}

// Handle UNSUBSCRIBE frame
void Frame::handleUnsubscribe(ConnectionHandler& connectionHandler, const std::string& channelName) {
    std::unordered_map<std::string, int>& mapChannelID = protocol.getMapChannelID();
    std::unordered_map<int, std::string>& mapRecieptID = protocol.getMapRecieptID();

    // **VALIDATION: Check if the client is subscribed to the channel**
    if (mapChannelID.find(channelName) == mapChannelID.end()) {
        std::cerr << "Error: Not subscribed to the channel \"" << channelName << "\".\n";
        return;
    }

    // Get the subscription ID
    int subscriptionId = mapChannelID.at(channelName);
    int recieptUnsubscribe = protocol.getandIncrementReceiptUnsubscribe();

    std::unordered_map<std::string, std::string> headers = {
        {"id", std::to_string(subscriptionId)}, 
        {"receipt", std::to_string(recieptUnsubscribe)}
    };
// check 
    std::cerr << "channel id map size before insert: " << mapChannelID.size() << "\n";
    std:: cerr << "reciept id map size before insert: " << mapRecieptID.size() << "\n";
    
    mapRecieptID[recieptUnsubscribe] = channelName;
    Frame unsubscribeFrame("UNSUBSCRIBE", headers, "", protocol) ;

// check 
    std::cerr << "channel id map size after insert: " << mapChannelID.size() << "\n";
    std:: cerr << "reciept id map size after insert: " << mapRecieptID.size() << "\n";

    // Send frame
    std::string frameString = unsubscribeFrame.toString();
    if (!connectionHandler.sendLine(frameString)) {
        std::cerr << "Failed to send UNSUBSCRIBE frame.\n";
    }
    std::cout << "Exited channel " << channelName << std::endl;
}
    
    
    //names_and_events parseEventsFile(std::string json_path); //used in report

    // Handle REPORT (SEND frames for multiple events)
void Frame::handleReport(ConnectionHandler& connectionHandler,std::string json_path) {
    // Parse the events file using the provided parser
    names_and_events parsedData = parseEventsFile(json_path);
    // Extract the channel name
    std::string channelName = parsedData.channel_name;

    std::unordered_map<std::string, int>& mapChannelID = protocol.getMapChannelID();

    // **VALIDATION: Check if the client is subscribed to the channel**
    if (mapChannelID.find(channelName) == mapChannelID.end()) {
        std::cerr << "Error: Not subscribed to the channel \"" << channelName << "\". Cannot send messages.\n";
        return;
    }


    // Loop through each event and send it as a SEND frame
    for (const Event& event : parsedData.events) {
        // Build the SEND frame headers
        std::unordered_map<std::string, std::string> headers = {
            {"destination",  channelName},    // Use the parsed channel name
        };

        // Format the event body according to the specified report format
        std::string body = "user: " + event.getEventOwnerUser() + "\n" +
                           "city: " + event.get_city() + "\n" +
                           "event name: " + event.get_name() + "\n" +
                           "date time: " + std::to_string(event.get_date_time()) + "\n" +
                           "general information:\n";

        // Add general information to the body
        for (const auto& pair : event.get_general_information()) {
            body += pair.first + ": " + pair.second + "\n";
        }

        // Add the description to the body
        body += "description:\n" + event.get_description() + "\n";

        // Create and send the SEND frame
        Frame finishedFrame("SEND", headers, body, protocol);
        std::string frameString = finishedFrame.toString();
        if (!connectionHandler.sendLine(frameString)) {
            std::cerr << "Failed to send SEND frame for event: " << event.get_name() << "\n";
        }
    }
    std::cout << "Reported" << channelName << std::endl;
}

// Handle DISCONNECT frame
void Frame::handleDisconnect(ConnectionHandler& connectionHandler, bool& shouldTerminate) {

    int receiptId = protocol.getandIncrementReceiptUnsubscribe();

    std::unordered_map<std::string, std::string> headers = {
        {"receipt", std::to_string(receiptId)}
    };
    Frame disconnectFrame("DISCONNECT", headers, "", protocol);

    // Send frame
    std::string frameString = disconnectFrame.toString();
    if (!connectionHandler.sendLine(frameString)) {
        std::cerr << "Failed to send DISCONNECT frame.\n";
    }
     // Close the socket
    connectionHandler.close(); // might not be necessery- maybe better in main
    shouldTerminate = true; // Signal protocol termination
     std::cout << " Logged out" << std::endl;
}   


//handle  summary
void Frame::handleSummary(const std::string& channelName, const std::string& user, const std::string& filePath) {
    // Open or create the output file
    std::ofstream outputFile(filePath); // ofStream legit opens if not existant
    if (!outputFile) {
        std::cerr << "Failed to open file: " << filePath << " for writing.\n";
        return;
    }
    std::unordered_map<std::string, int>& mapChannelID = protocol.getMapChannelID();

     if (mapChannelID.find(channelName) != mapChannelID.end()) {
             std::cerr <<  "you are not subscribed to channel" + channelName << "\n";
             return;
     }
    // Lock the reports map while accessing it- thread safe
    std::lock_guard<std::mutex> lock(protocol.getReportsMutex() );

 std::map<std::string, std::map<std::string, summaryReport>>& reports = protocol.getReports(); // Get the reports map
    // Check if the user and channel exist in the map
    if (reports.find(channelName) == reports.end() || reports[channelName].find(user) == reports[channelName].end()) { //end means non existant
        std::cerr << "No reports found for channel: " << channelName << " and user: " << user << "\n";
        return;
    }

    summaryReport& report = reports[channelName][user]; // Get the report for the user + channel

   // Sort events by date_time and then by event_name
   // sort function is used to sort the vector in ascending order ,implementation of compartor by lambda function
            std::sort(report.events.begin(), report.events.end(), [](const Event& a, const Event& b) { 
            if (a.get_date_time() == b.get_date_time()) {
                return a.get_name() < b.get_name();
            }
            return a.get_date_time() < b.get_date_time();
        });
    //at this point- the events are sorted

    // Print the sorted events for verification - toDELETE
    std::cout << "Sorted events for channel: " << channelName << " and user: " << user << "\n";
    for (const Event& event : report.events) {
        std::cout << "Event name: " << event.get_name() << ", Date time: " << event.get_date_time() << "\n";
    }   

    // Write the header
    outputFile << "Channel: " << channelName << "\n";
    outputFile << "Stats:\n";
    outputFile << "Total: " << report.totalReports << "\n";
    outputFile << "Active: " << report.activeCount << "\n";
    outputFile << "Forces arrival at scene: " << report.forcesArrivalCount << "\n\n";

    // Write the sorted event details
    outputFile << "Event Reports:\n";
    int reportNumber = 1;

    for (const Event& event : report.events) {
        outputFile << "Report_" << reportNumber++ << ":\n";
        outputFile << "City: " << event.get_city() << "\n";
        outputFile << "Date time: " << epochToDate(event.get_date_time()) << "\n"; // Convert timestamp to readable date
        outputFile << "Event name: " << event.get_name() << "\n";
        
        // Truncate the description to 27 characters
        std::string description = event.get_description().substr(0, 27);
        if (event.get_description().length() > 27) {
            description += "...";
        }
        outputFile << "Summary: " << description << "\n\n"; // Write the truncated description
    }

    std::cout << "Summary written to file: " << filePath << "\n";
    outputFile.close();
}


std::string Frame::epochToDate(const int time) {
    std::time_t epochTime = static_cast<std::time_t>(time);
    std::tm* tm = std::localtime(&epochTime);
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%d/%m/%y %H:%M", tm);
    return std::string(buffer);
}

