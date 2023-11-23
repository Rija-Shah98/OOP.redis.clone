#include <cassert>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

#pragma once

namespace Consts {
  const size_t SIZE_HEADER_LEN = 4;
  const size_t MAX_MSG_LEN = 4096;
  const size_t MAX_EPOLL_EVENTS = 512;
}

inline int setnonblocking(int fd) {
  errno = 0;
  int flags = fcntl(fd, F_GETFL, 0);
  if (errno) {
    perror("fcntl error");
    return -1;
  }

  flags |= O_NONBLOCK;

  errno = 0;
  return fcntl(fd, F_SETFL, flags);
}