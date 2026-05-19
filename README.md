# Socket-Client-Code

A collection of C++ TCP socket client programs exploring progressively more complex networking and concurrency patterns, built with POSIX sockets and pthreads.

## Programs

| File | Description |
|------|-------------|
| `test_socket_client.cpp` | Minimal client — connects to `localhost:8080` and sends a single hardcoded message |
| `chat_socket_client.cpp` | Interactive chat client — takes an IP and port as arguments, loops sending/receiving messages with session stats |
| `multiSC_socket_client.cpp` | Chat client with chunked receive loop — handles multi-packet server responses correctly |
| `multithreadP_socket_client.cpp` | Multithreaded chat client — separate pthreads for sending and receiving; supports named users and Ctrl+C graceful exit |
| `toysim_pthread.cpp` | Toy pthread simulation — early prototype of the send/receive thread model |

## Build

Each file compiles independently. Use `-lpthread` for the threaded programs:

```bash
# Simple clients
g++ -o test_socket_client test_socket_client.cpp
g++ -o chat_socket_client chat_socket_client.cpp
g++ -o multiSC_socket_client multiSC_socket_client.cpp

# Threaded clients
g++ -o multithreadP_socket_client multithreadP_socket_client.cpp -lpthread
g++ -o toysim_pthread toysim_pthread.cpp -lpthread
```

## Usage

**test_socket_client** — no arguments, connects to `localhost:8080`:
```bash
./test_socket_client
```

**chat_socket_client** — pass IP address and port:
```bash
./chat_socket_client 127.0.0.1 8080
```

**multiSC_socket_client** — hardcoded to `127.0.0.1:8080`, no arguments:
```bash
./multiSC_socket_client
```

**multithreadP_socket_client** — pass port number (connects to `localhost`):
```bash
./multithreadP_socket_client 8080
```
Type `#exit` or press Ctrl+C to disconnect gracefully.

## Requirements

- A server program in this [repo](https://github.com/ProjectNerva/Socket-Server-Code) must be running
- Linux or macOS
- C++11 or later (`g++` or `clang++`)
- POSIX thread support (`-lpthread`)
