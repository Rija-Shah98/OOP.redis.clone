#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio/io_context.hpp>
#include <iostream>
#include <unordered_map>
#include <memory>
#include <sstream>
#include <iterator>

using boost::asio::ip::tcp;

class RedisServer {
public:
    explicit RedisServer(boost::asio::io_context& io_context, int port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), socket_(io_context) {
        startAccept();
        std::cout << "[INFO]: Server started on port " << port << "\n";
    }

private:
    struct Connection {
        tcp::socket socket;
        std::vector<char> rbuf;  // Read buffer
        std::vector<char> wbuf;  // Write buffer
        std::string command;

        explicit Connection(boost::asio::io_context& io_context)
            : socket(io_context), rbuf(1024), wbuf(1024) {
        }
    };

    tcp::acceptor acceptor_;
    tcp::socket socket_;
    std::unordered_map<std::string, std::string> kvstore;

    void startAccept() {
        // Create a new connection object
        auto conn = std::make_shared<Connection>(acceptor_.get_executor().context());
        acceptor_.async_accept(
            conn->socket,
            boost::bind(&RedisServer::handleAccept, this, conn, boost::asio::placeholders::error));
    }

    void handleAccept(std::shared_ptr<Connection> conn, const boost::system::error_code& error) {
        if (!error) {
            std::cout << "[INFO]: New connection accepted\n";
            startRead(conn);
        }
        else {
            std::cerr << "[ERROR]: Accept error: " << error.message() << "\n";
        }
        // Accept the next connection
        startAccept();
    }

    void startRead(std::shared_ptr<Connection> conn) {
        boost::asio::async_read_until(
            conn->socket,
            boost::asio::dynamic_buffer(conn->command),
            '\n',
            boost::bind(&RedisServer::handleRead, this, conn, boost::asio::placeholders::error));
    }

    void handleRead(std::shared_ptr<Connection> conn, const boost::system::error_code& error) {
        if (!error) {
            // Process the received command
            std::string response = processCommand(conn->command);
            conn->command.clear();  // Clear the command buffer
            startWrite(conn, response);
        }
        else {
            std::cerr << "[ERROR]: Read error: " << error.message() << "\n";
        }
    }

    void startWrite(std::shared_ptr<Connection> conn, const std::string& response) {
        conn->wbuf.assign(response.begin(), response.end());
        boost::asio::async_write(
            conn->socket,
            boost::asio::buffer(conn->wbuf),
            boost::bind(&RedisServer::handleWrite, this, conn, boost::asio::placeholders::error));
    }

    void handleWrite(std::shared_ptr<Connection> conn, const boost::system::error_code& error) {
        if (!error) {
            // Continue reading commands from the same connection
            startRead(conn);
        }
        else {
            std::cerr << "[ERROR]: Write error: " << error.message() << "\n";
        }
    }

    std::string processCommand(const std::string& command) {
        // Parse the command
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
};

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

