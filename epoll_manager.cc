#ifndef EPOLL_MANAGER_H
#define EPOLL_MANAGER_H

#include <boost/asio.hpp>
#include <functional>
#include <memory>
#include <unordered_map>

class EpollManager {
public:
    EpollManager(); // Constructor to initialize Boost.Asio
    ~EpollManager() = default;

    void register_fd(int fd, std::function<void(const boost::system::error_code&)> read_handler);
    void mod_fd(int fd, std::function<void(const boost::system::error_code&)> read_handler);
    void delete_fd(int fd);
    void poll(); // Run the event loop to handle events

private:
    boost::asio::io_context io_context_; // The Boost.Asio event loop
    std::unordered_map<int, std::shared_ptr<boost::asio::ip::tcp::socket>> sockets_; // Map of file descriptors to sockets
};

#endif // EPOLL_MANAGER_H
