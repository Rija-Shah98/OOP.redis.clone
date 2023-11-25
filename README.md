# Redis Clone
A redis clone, implemented using C++. This is my implementation of James Smith's [Build Your Own Redis with C/C++ book](https://build-your-own.org/redis).

## Usage
1. Run `make` to build both server and client and `make debug` to build in debug mode.
1. Run `./server` and `./client` to start the server and client respectively. Both will try to connect to port `6969`.
## Project Structure
```bash
$ tree
.
├── client.cc
├── command.cc
├── command.h
├── conn.h
├── epoll_manager.cc
├── epoll_manager.h
├── Makefile
├── README.md
├── server.cc
├── server.h
├── test.py
└── utils.h
```
| File              | Description                                             |
|-------------------|---------------------------------------------------------|
| `client.cc`       | Client program to test the functionality of the server  |
| `command.*`       | A struct that encapsulates a command sent by the client |
| `conn.h`          | A struct that encapsulates a network connection         |
| `epoll_manager.*` | A wrapper around the `epoll` system call                |
| `server.*`        | The main redis server                                   |
| `utils.h`         | Miscellaneous utility functions and consts              |
| `test.py`         | Test script to run multiple clients simultaneously      |

## Some implementation details
Here are some implementation details of the redis server:
1. Since we are using `epoll`, we must have a way to store the 'state' of each client. Inside the `Conn` struct, we maintain two buffers: one for reading and one for writing. We also maintain 4 indexes to denote which part of the buffer contains data that needs to be read/write from/to.
1. Reading data from the socket: Instead of calling the `read()` syscall multiple times (i.e. one to get the length, and another call to get the actual data, repeated for each arguments), we `read()` as much data as possible and then deal with the mess later. Some cases that must be handled:
    - Client sending multiple commands in a single `write()` call (command pipelining).
    - Amount of data read is less than the amount sent by the client (partial read).

## Testing
Currently, the tests mainly checks for the implementation of `RedisServer::handleRead` (pipelining, partial read). When tested on my machine, the server is able to handle 8000 clients concurrently.

## Potential improvements
1. Using circular buffers to avoid using `std::memmove()` when partial read is encountered.
1. Use better protocol (maybe using Redis' actual protocol) since the current protocol depends on the fact that both client and server has the same host byte order (little endian).