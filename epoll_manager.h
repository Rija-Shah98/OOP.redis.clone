#include <boost/asio.hpp>
#include <vector>
#include "utils.h"

#pragma once

class EpollManager {
public:
	EpollManager(boost::asio::io_context& io_context);
	void register_fd(boost::asio::ip::tcp::socket& socket);
	void delete_fd(boost::asio::ip::tcp::socket& socket);
	void poll();

private:
	boost::asio::io_context& io_context_;
	std::vector<std::shared_ptr<boost::asio::ip::tcp::socket>> sockets_;
};
