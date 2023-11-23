CCX = g++
CXXFLAGS = -pedantic -Wall -Wextra -Werror

all: server client

server: conn.h utils.h epoll_manager.h epoll_manager.cc server.h server.cc
	$(CCX) $(CXXFLAGS) -O3 -o server epoll_manager.cc server.cc

client: client.cc
	$(CCX) $(CXXFLAGS) -O3 -o client client.cc

debug: conn.h utils.h epoll_manager.h epoll_manager.cc server.h server.cc client.cc
	$(CCX) $(CXXFLAGS) -g -o server epoll_manager.cc server.cc
	$(CCX) $(CXXFLAGS) -g -o client epoll_manager.cc client.cc

clean:
	rm -f server client