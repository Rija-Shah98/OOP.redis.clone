#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <iostream>
#include <unordered_map>
#include <memory>
#include <vector>
#include <sstream>
#include <iterator>

#include "server.h"

using boost::asio::ip::tcp;

struct Connection : public std::enable_shared_from_this<Connection> {
    explicit Connection(boost::asio::io_context& io_context)
        : socket(io_context) {
    }

    tcp::socket socket;
    std::string command;
    std::vector<char> wbuf;
};

RedisServer::RedisServer(boost::asio::io_context& io_context, int port)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
    acceptConnection();
    std::cout << "[INFO]: Server started on port " << port << "\n";
}

void RedisServer::acceptConnection() {
    auto conn = std::make_shared<Connection>(acceptor_.get_executor().context());
    acceptor_.async_accept(
        conn->socket,
        [this, conn](const boost::system::error_code& error) {
            if (!error) {
                std::cout << "[INFO]: New connection accepted\n";
                handleRead(conn);
            }
            else {
                std::cerr << "[ERROR]: Accept error: " << error.message() << "\n";
            }
            acceptConnection(); // Accept the next connection
        });
}

void RedisServer::handleRead(std::shared_ptr<Connection> conn) {
    boost::asio::async_read_until(
        conn->socket,
        boost::asio::dynamic_buffer(conn->command),
        '\n',
        [this, conn](const boost::system::error_code& error, std::size_t /*bytes_transferred*/) {
            if (!error) {
                std::string response = processCommand(conn->command);
                handleWrite(conn, response);
            }
            else {
                std::cerr << "[ERROR]: Read error: " << error.message() << "\n";
            }
        });
}

void RedisServer::handleWrite(std::shared_ptr<Connection> conn, const std::string& response) {
    conn->wbuf.assign(response.begin(), response.end());
    boost::asio::async_write(
        conn->socket,
        boost::asio::buffer(conn->wbuf),
        [this, conn](const boost::system::error_code& error, std::size_t /*bytes_transferred*/) {
            if (!error) {
                handleRead(conn); // Continue reading commands
            }
            else {
                std::cerr << "[ERROR]: Write error: " << error.message() << "\n";
            }
        });
}

std::string RedisServer::processCommand(const std::string& command) {
    std::istringstream iss(command);
    std::vector<std::string> args((std::istream_iterator<std::string>(iss)),
        std::istream_iterator<std::string>());

    if (args.empty()) {
        return "Invalid command\n";
    }

    const std::string& cmd_type = args[0];
    if (cmd_type == "set" && args.size() == 3) {
        kvstore[args[1]] = args[2];
        return "OK\n";
    }
    else if (cmd_type == "get" && args.size() == 2) {
        auto it = kvstore.find(args[1]);
        if (it != kvstore.end()) {
            return it->second + "\n";
        }
        else {
            return "Key not found\n";
        }
    }
    else if (cmd_type == "del" && args.size() == 2) {
        kvstore.erase(args[1]);
        return "OK\n";
    }
    else {
        return "Invalid command\n";
    }
}

int main() {
    try {
        boost::asio::io_context io_context;
        RedisServer server(io_context, 6969);
        io_context.run();
    }
    catch (std::exception& e) {
        std::cerr << "[ERROR]: Exception: " << e.what() << "\n";
    }
    return 0;
}

