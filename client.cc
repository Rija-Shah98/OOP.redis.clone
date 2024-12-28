#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#endif

const size_t k_max_msg = 4096;

static void msg(const char* msg) {
    fprintf(stderr, "%s\n", msg);
}

static void die(const char* msg) {
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

static int32_t read_full(int fd, char* buf, size_t n) {
    while (n > 0) {
#ifdef _WIN32
        ssize_t rv = recv(fd, buf, n, 0);
#else
        ssize_t rv = read(fd, buf, n);
#endif
        if (rv <= 0) {
            return -1;  // error, or unexpected EOF
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

static int32_t write_all(int fd, const char* buf, size_t n) {
    while (n > 0) {
#ifdef _WIN32
        ssize_t rv = send(fd, buf, n, 0);
#else
        ssize_t rv = write(fd, buf, n);
#endif
        if (rv <= 0) {
            return -1;  // error
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

int main() {
    /* disable printf buffering */
    setbuf(stdout, NULL);

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        die("WSAStartup() failed");
    }
#endif

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        die("socket()");
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(6969);
#ifdef _WIN32
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
#else
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);  // 127.0.0.1
#endif

    int rv = connect(fd, (const struct sockaddr*)&addr, sizeof(addr));
    if (rv) {
        die("can't connect to server");
    }

    char wbuf[k_max_msg];
    char rbuf[k_max_msg];

    while (true) {
        printf("Enter command: ");
        if (!fgets(wbuf, sizeof(wbuf), stdin)) {
            break;  // Exit on EOF
        }

        // Remove trailing newline from input
        size_t len = strlen(wbuf);
        if (wbuf[len - 1] == '\n') {
            wbuf[len - 1] = '\0';
            len--;
        }

        // Send command to server
        if (write_all(fd, wbuf, len) != 0) {
            perror("write");
            break;
        }

        // Receive reply from server
        ssize_t n = recv(fd, rbuf, sizeof(rbuf) - 1, 0);
        if (n <= 0) {
            perror("read");
            break;
        }
        rbuf[n] = '\0';
        printf("Server reply: %s\n", rbuf);
    }

#ifdef _WIN32
    closesocket(fd);
    WSACleanup();
#else
    close(fd);
#endif

    return 0;
}
