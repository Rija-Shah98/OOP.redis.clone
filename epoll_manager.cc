#include "epoll_manager.h"
#include "utils.h"

EpollManager::EpollManager() {
  epoll_fd = epoll_create1(0);
  if (epoll_fd == -1) {
    perror("[ERROR]: epoll_create error");
    exit(1);
  }
}

int EpollManager::register_fd(int fd, uint32_t flags) {
  struct epoll_event ev;
  ev.events = flags;
  ev.data.fd = fd;
  int err = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
  if (err == -1) {
    perror("[ERROR]: epoll_ctl");
  }
  return err;
}

int EpollManager::delete_fd(int fd, uint32_t flags) {
  struct epoll_event ev;
  ev.events = flags;
  ev.data.fd = fd;
  int err = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &ev);
  if (err < 0) {
    fprintf(stderr, "[ERROR]: epoll set deletion error: fd=%d\n", fd);
  }
  close(fd);
  return err;
}

int EpollManager::poll(std::vector<struct epoll_event>& events) {
  return epoll_wait(epoll_fd, events.data(), Consts::MAX_EPOLL_EVENTS, -1);
}