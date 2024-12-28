#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>

#include "command.h"
#include "utils.h"

class RedisServer {
public:
    RedisServer(boost::asio::io_context& io_context, int port);
    void runServer();

private:
    void acceptConnection();
    void handleRead(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void handleWrite(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const std::string& response);

    std::unordered_map<std::string, std::string> kvstore;

    boost::asio::ip::tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<boost::asio::ip::tcp::socket>> clients;
};
