
# Simple FSM-Based Socket Server

This project is a non-blocking TCP server written in C that handles multiple clients concurrently using the `poll()` system call. The server manages client connections with a simple Finite-State Machine (FSM) logic.

## Features

-   **Concurrent Clients:** Manages multiple clients simultaneously without using threads or forks.
-   **Non-blocking I/O:** Efficiently waits for events using `poll()`.
-   **Connection Management:** Accepts new connections, reads data from clients, and handles disconnections.
-   **State Management:** Tracks states for each client, such as `STATE_NEW`, `STATE_CONNECTED`, and `STATE_DISCONNECTED`.
-   **Fast Restart:** The `SO_REUSEADDR` socket option allows the server to be restarted on the same port immediately after being shut down.

## How to Use

### 1. Compilation

Use the following command to compile the project:

```bash
gcc server.c -o server
```

### 2. Running the Server

Start the compiled server:

```bash
./server
```

The server will start listening on port `8080` by default.

## Testing

To connect to and test the server, you can use tools like `netcat` (nc) or `telnet`. Open a new terminal and run one of the following commands:

```bash
nc localhost 8080
```

or

```bash
telnet localhost 8080
```

Anything you type into this terminal will be received by the server and displayed in the server's console.