#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Connection : public std::enable_shared_from_this<Connection> {
public:
    explicit Connection(boost::asio::io_context& io_context)
        : socket(io_context) {
    }

    boost::asio::ip::tcp::socket socket;
    std::string command;
    std::vector<char> wbuf;
};

class RedisServer {
public:
    RedisServer(boost::asio::io_context& io_context, int port);
    void acceptConnection();
    void handleRead(std::shared_ptr<Connection> conn);
    void handleWrite(std::shared_ptr<Connection> conn, const std::string& response);
    std::string processCommand(const std::string& command);

private:
    boost::asio::ip::tcp::acceptor acceptor_;
    std::unordered_map<std::string, std::string> kvstore;
};

#endif

