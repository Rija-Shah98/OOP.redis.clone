CCX = g++
CXXFLAGS = -pedantic -Wall -Wextra -Werror

all: server client

server: command.o epoll_manager.o server.o
	$(CCX) $(CXXFLAGS) -O3 -o server command.o epoll_manager.o server.o

command.o: command.h command.cc
	$(CCX) $(CXXFLAGS) -c -O3 -o command.o command.cc

epoll_manager.o: utils.h epoll_manager.h epoll_manager.cc 
	$(CCX) $(CXXFLAGS) -c -O3 -o epoll_manager.o epoll_manager.cc

server.o: command.h conn.h utils.h epoll_manager.h server.h server.cc
	$(CCX) $(CXXFLAGS) -c -O3 -o server.o server.cc

client: client.cc
	$(CCX) $(CXXFLAGS) -O3 -o client client.cc

debug: conn.h utils.h epoll_manager.h epoll_manager.cc server.h server.cc client.cc
	$(CCX) $(CXXFLAGS) -g -o server epoll_manager.cc server.cc
	$(CCX) $(CXXFLAGS) -g -o client epoll_manager.cc client.cc

clean:
	rm -f server client