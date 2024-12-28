#include <cassert>
#include <cerrno>
#include <fcntl.h>
#include <cstddef>
#include <cstdio>
#include <unistd.h>
#include <iostream>
#include <string>
#include <array>
#include <type_traits>
#include <iterator>

#pragma once

namespace Consts {
    constexpr size_t SIZE_HEADER_LEN = 4;
    constexpr size_t MAX_MSG_LEN = 4096;
    constexpr size_t MAX_EPOLL_EVENTS = 512;
}

inline int setnonblocking(int fd) {
    if (fd < 0) {
        std::cerr << "Invalid file descriptor: " << fd << "\n";
        return -1;
    }

    errno = 0;
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl error (F_GETFL)");
        return -1;
    }

    flags |= O_NONBLOCK;
    errno = 0;
    if (fcntl(fd, F_SETFL, flags) == -1) {
        perror("fcntl error (F_SETFL)");
        return -1;
    }

    return 0;
}

namespace Logger {
    enum LogLevel {
        DEBUG,
        INFO,
        ERROR,
    };

    constexpr std::array<const char*, 3> log_strings = {
      "DEBUG",
      "INFO",
      "ERROR"
    };

    inline void log(LogLevel lvl, const std::string& msg, bool newline = true) {
        bool print = (lvl != DEBUG);
#ifdef _DEBUG
        print = true;
#endif
        if (!print) return;

        std::cout << "[" << log_strings[lvl] << "] " << msg;
        if (newline) std::cout << "\n";
    }

    template<typename T>
    void log(LogLevel lvl, T begin, T end) {
        static_assert(std::is_same_v<typename std::iterator_traits<T>::iterator_category, std::input_iterator_tag>,
            "Logger::log requires input iterators.");

        bool print = (lvl != DEBUG);
#ifdef _DEBUG
        print = true;
#endif
        if (!print) return;

        std::cout << "[" << log_strings[lvl] << "] ";
        for (auto it = begin; it != end; ++it) {
            std::cout << *it;
            if (std::next(it) != end) std::cout << " ";
        }
        std::cout << "\n";
    }
}
