#include <cstring>
#include <errno.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "conn.h"
#include "epoll_manager.h"
#include "utils.h"

#pragma once

class RedisServer {
public:
  RedisServer();
  void startTcpServer(int port);
  void runServer();
  int acceptConn();
  int handleRead(Conn& c);
  int handleWrite(Conn& c);

private:
  int server_fd;
  EpollManager epoll_man{};
  std::vector<Conn> clients;
};