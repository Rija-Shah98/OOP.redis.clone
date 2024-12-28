#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
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
    struct Connection {
        boost::asio::ip::tcp::socket socket;
        std::vector<char> rbuf; // Read buffer
        std::vector<char> wbuf; // Write buffer
        std::string command;

        explicit Connection(boost::asio::io_context& io_context)
            : socket(io_context), rbuf(1024), wbuf(1024) {
        }
    };

    void acceptConnection();
    void handleRead(std::shared_ptr<Connection> conn);
    void handleWrite(std::shared_ptr<Connection> conn, const std::string& response);

    std::unordered_map<std::string, std::string> kvstore;
    boost::asio::ip::tcp::acceptor acceptor_;
};
