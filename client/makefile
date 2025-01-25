CFLAGS := -c -Wall -Weffc++ -g -std=c++11 -Iinclude
LDFLAGS := -lboost_system -lpthread

# Default target
all: EchoClient StompWCIClient

# Build EchoClient
EchoClient: bin/ConnectionHandler.o bin/echoClient.o
	g++ -o bin/EchoClient bin/ConnectionHandler.o bin/echoClient.o $(LDFLAGS)

# Build StompWCIClient
StompWCIClient: bin/ConnectionHandler.o bin/StompClient.o bin/event.o
	g++ -o bin/StompWCIClient bin/ConnectionHandler.o bin/StompClient.o bin/event.o $(LDFLAGS)

# Object file rules
bin/ConnectionHandler.o: src/ConnectionHandler.cpp | bin
	g++ $(CFLAGS) -o bin/ConnectionHandler.o src/ConnectionHandler.cpp

bin/echoClient.o: src/echoClient.cpp | bin
	g++ $(CFLAGS) -o bin/echoClient.o src/echoClient.cpp

bin/event.o: src/event.cpp | bin
    $(CXX) $(CXXFLAGS) -c -o bin/event.o src/event.cpp

bin/StompClient.o: src/StompClient.cpp | bin
    $(CXX) $(CXXFLAGS) -c -o bin/StompClient.o src/StompClient.cpp

bin/StompProtocol.o: src/StompProtocol.cpp | bin
    $(CXX) $(CXXFLAGS) -c -o bin/StompProtocol.o src/StompProtocol.cpp

bin/Frame.o: src/Frame.cpp | bin
    $(CXX) $(CXXFLAGS) -c -o bin/Frame.o src/Frame.cpp


g++ -I/root/Ass3/client/include -o Frame /root/Ass3/client/src/Frame.cpp
g++ -I/root/Ass3/client/include -o StompProtocol /root/Ass3/client/src/StompProtocol.cpp
g++ -I/root/Ass3/client/include -o StompClient /root/Ass3/client/src/StompClient
g++ -I/root/Ass3/client/include -o ConnectionHandler /root/Ass3/client/src/ConnectionHandler
g++ -I/root/Ass3/client/include -o event /root/Ass3/client/src/event.cpp


# Ensure bin directory exists
bin:
	mkdir -p bin

# Clean target
.PHONY: clean
clean:
	rm -f bin/*.o bin/EchoClient bin/StompWCIClient
