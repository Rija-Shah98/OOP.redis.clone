#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

const size_t k_max_msg = 4096;

static void msg(const char *msg) {
  fprintf(stderr, "%s\n", msg);
}

static void die(const char *msg) {
  int err = errno;
  fprintf(stderr, "[%d] %s\n", err, msg);
  abort();
}

static int32_t read_full(int fd, char *buf, size_t n) {
  while (n > 0) {
    ssize_t rv = read(fd, buf, n);
    if (rv <= 0) {
      return -1;  // error, or unexpected EOF
    }
    assert((size_t)rv <= n);
    n -= (size_t)rv;
    buf += rv;
  }
  return 0;
}

static int32_t write_all(int fd, const char *buf, size_t n) {
  while (n > 0) {
    ssize_t rv = write(fd, buf, n);
    if (rv <= 0) {
      return -1;  // error
    }
    assert((size_t)rv <= n);
    n -= (size_t)rv;
    buf += rv;
  }
  return 0;
}


static int32_t query(int fd, const char *text) {
  uint32_t len = (uint32_t)strlen(text);
  if (len > k_max_msg) {
    return -1;
  }

  char wbuf[4 + k_max_msg];
  memcpy(wbuf, &len, 4);  // assume little endian
  memcpy(&wbuf[4], text, len);
  if (int32_t err = write_all(fd, wbuf, 4 + len)) {
    return err;
  }

  // 4 bytes header
  char rbuf[4 + k_max_msg + 1];
  errno = 0;
  int32_t err = read_full(fd, rbuf, 4);
  if (err) {
    if (errno == 0) {
      msg("EOF");
    } else {
      msg("read() error");
    }
    return err;
  }

  memcpy(&len, rbuf, 4);  // assume little endian
  if (len > k_max_msg) {
    msg("too long");
    return -1;
  }

  // reply body
  err = read_full(fd, &rbuf[4], len);
  if (err) {
    msg("read() error");
    return err;
  }

  // echo back
  rbuf[4 + len] = '\0';
  printf("server says: %s\n", &rbuf[4]);
  return 0;
}

// send the same message 5x, but using only one write() syscall.
static int32_t multiQuery(int fd, const char *text) {
  uint32_t len = (uint32_t)strlen(text);
  if (len > k_max_msg) {
    return -1;
  }

  char wbuf[4 + k_max_msg];
  uint32_t offset = 0;
  for (int i = 0; i < 5; ++i) {
    memcpy(wbuf + offset, &len, 4);  // assume little endian
    memcpy(&wbuf[4] + offset, text, len);
    offset += (4 + len);
  }

  if (int32_t err = write_all(fd, wbuf, 5 * (4 + len))) {
    return err;
  }

  for (int i = 0; i < 5; ++i) {
    // 4 bytes header
    char rbuf[4 + k_max_msg + 1];
    errno = 0;
    int32_t err = read_full(fd, rbuf, 4);
    if (err) {
      if (errno == 0) {
        msg("EOF");
      } else {
        msg("read() error");
      }
      return err;
    }

    memcpy(&len, rbuf, 4);  // assume little endian
    if (len > k_max_msg) {
      msg("too long");
      return -1;
    }

    // reply body
    err = read_full(fd, &rbuf[4], len);
    if (err) {
      msg("read() error");
      return err;
    }

    // do something
    rbuf[4 + len] = '\0';
    printf("server says: %s\n", &rbuf[4]);
  }
  return 0;
}

int test_multiple_queries_multiple_write(int fd) {
  int32_t err = query(fd, "hello1");
  if (err) return err;
  err = query(fd, "hello2");
  if (err) return err;
  err = query(fd, "hello3");
  return err;
}

int test_multiple_queries_single_write(int fd) {
  int32_t err = multiQuery(fd, "heloooo");
  return err;
}

int test_two_max_queries_single_write(int fd) {
  constexpr size_t two_messages_len = 2 * (k_max_msg + 4);
  char wbuf[two_messages_len];
  memset(wbuf, 'A', two_messages_len);

  uint32_t len = (uint32_t)k_max_msg;
  // set the length correctly
  memcpy(wbuf, &len, 4);
  memcpy(wbuf + k_max_msg + 4, &len, 4);

  if (int32_t err = write_all(fd, wbuf, two_messages_len)) {
    return err;
  }

  char rbuf[two_messages_len];
  for (int i = 0; i < 2; ++i) {
    errno = 0;
    int32_t err = read_full(fd, rbuf, 4);
    if (err) {
      if (errno == 0) {
        msg("EOF");
      } else {
        msg("read() error");
      }
      return err;
    }

    memcpy(&len, rbuf, 4);  // assume little endian
    if (len > k_max_msg) {
      msg("too long");
      return -1;
    }

    // reply body
    err = read_full(fd, &rbuf[4], len);
    if (err) {
      msg("read() error");
      return err;
    }

    // echo back
    rbuf[4 + len] = '\0';
    printf("server says: %s\n", &rbuf[4]);
  }
  return 0;
}

int test_fragmented_queries1(int fd) {
  // total buffer size = 4 (size header) + 4096 (max_msg_len) = 4100
  // send 2 message at 2050 each -> last message will be truncated
  // received second message: 4100 - 2054 = 2046 (incl. header)
  char wbuf[2 * (4 + 2050)];
  memset(wbuf, 'A', 2054);
  memset(wbuf + 2054, 'B', 2054);

  uint32_t len = 2050;
  // set length
  memcpy(wbuf, &len, 4);
  memcpy(wbuf + 2050 + 4, &len, 4);

  if (int32_t err = write_all(fd, wbuf, sizeof(wbuf))) {
    return err;
  }

  char rbuf[2 * (4 + 2050)];
  for (int i = 0; i < 2; ++i) {
    errno = 0;
    int32_t err = read_full(fd, rbuf, 4);
    if (err) {
      if (errno == 0) {
        msg("EOF");
      } else {
        msg("read() error");
      }
      return err;
    }

    memcpy(&len, rbuf, 4);  // assume little endian
    if (len > k_max_msg) {
      msg("too long");
      return -1;
    }

    // reply body
    err = read_full(fd, &rbuf[4], len);
    if (err) {
      msg("read() error");
      return err;
    }

    // echo back
    rbuf[4 + len] = '\0';
    printf("server says: %s\n", &rbuf[4]);
  }
  return 0;
}

int test_fragmented_queries2(int fd) {
  char wbuf[4 + 100];
  memset(wbuf, 'C', 104);

  uint32_t len = 100;
  // set the length correctly
  memcpy(wbuf, &len, 4);

  // write half
  if (int32_t err = write_all(fd, wbuf, 50)) {
    return err;
  }
  sleep(3);
  // write the next half
  if (int32_t err = write_all(fd, wbuf+50, 54)) {
    return err;
  }

  char rbuf[4 + 100];
  errno = 0;
  int32_t err = read_full(fd, rbuf, 4);
  if (err) {
    if (errno == 0) {
      msg("EOF");
    } else {
      msg("read() error");
    }
    return err;
  }

  memcpy(&len, rbuf, 4);  // assume little endian
  if (len > k_max_msg) {
    msg("too long");
    return -1;
  }

  // reply body
  err = read_full(fd, &rbuf[4], len);
  if (err) {
    msg("read() error");
    return err;
  }

  // echo back
  rbuf[4 + len] = '\0';
  printf("server says: %s\n", &rbuf[4]);
  return 0;
}

int runTests(int fd) {
  int err;

  printf("===== Test: multiple queries, multiple write()s =====\n");
  err = test_multiple_queries_multiple_write(fd);
  if (err) return err;

  printf("===== Test: multiple queries, single write() =====\n");
  err = test_multiple_queries_single_write(fd);
  if (err) return err;

  printf("===== Test: two max queries, single write() =====\n");
  err = test_two_max_queries_single_write(fd);
  if (err) return err;

  printf("===== Test: fragmented queries 1 =====\n");
  err = test_fragmented_queries1(fd);
  if (err) return err;

  printf("===== Test: fragmented queries 2 =====\n");
  err = test_fragmented_queries2(fd);
  if (err) return err;

  return err;
}

int main() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    die("socket()");
  }

  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(6969);
  addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);  // 127.0.0.1
  int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
  if (rv) {
    die("can't connect to server");
  }

  int err = runTests(fd);

  close(fd);

  return err;
}