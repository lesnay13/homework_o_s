# homework_o_s

Operating Systems homework: a **client/server system built on named pipes (FIFOs)** in C. Clients send numbered "system call" requests to a central server through a FIFO, and the server processes them (number/text conversion, store/recall) and responds.

## Overview

The project demonstrates inter-process communication (IPC) on Linux/Unix using:

- **Named pipes (`mkfifo`)** for communication between separate processes
- **A well-known server FIFO** that every client writes requests to
- **A per-client FIFO** (named after the client's PID) for the server's reply
- **A simple binary message protocol** made of fixed-size fields

## Repository Structure

| File | Description |
| --- | --- |
| `server.c` | Server program. Creates the request FIFO, reads client requests, and dispatches them by system call number. |
| `client.c` | Interactive client. Connects to the server, creates its own FIFO, and sends requests from a text menu. |
| `functions.c` | Standalone test harness for the helper functions (e.g. `num_to_text`), with the FIFO-dependent functions commented out. |
| `server`, `client` | Compiled binaries. |
| `.vscode/` | VS Code editor configuration. |
| `LICENSE` | MIT License. |

## Requirements

- Linux or macOS (anything with POSIX FIFOs)
- `gcc` (or another C compiler)

## Build

```bash
gcc -o server server.c
gcc -o client client.c
gcc -o functions functions.c   # optional: test the number-to-text logic
```

## Usage

1. Start the server in one terminal:

   ```bash
   ./server
   ```

2. Start one or more clients in other terminals:

   ```bash
   ./client
   ```

3. Choose an option from the client menu:

   ```
   What to do now?
   1. Send a request to server
   2. Exit
   3. Terminate
   ```

4. For option 1, enter a system call number, the number of parameters (0 or 1), and the parameter value if needed.

### System Calls

| # | Name | Parameters | Description |
| --- | --- | --- | --- |
| `1` | Connect | 0 | Opens the client's response FIFO on the server side. Sent automatically when a client starts. |
| `2` | Number to text | 1 | Converts a number (up to 4 digits) to English words, e.g. `533` becomes "five hundred thirty three". |
| `3` | Text to number | 1 | Converts a spelled-out digit (`zero` to `nine`) to its numeric value. Returns `-1` if not recognized. |
| `4` | Store | 1 | Stores an integer value on the server. |
| `5` | Recall | 0 | Returns the previously stored value. |
| `0` | Exit | 0 | This client is done. Its FIFO is closed and removed. |
| `-1` | Terminate | 0 | This client is done **and** the server shuts down. |

### Message Format

Each request is written to the server FIFO as a sequence of raw binary fields:

```
| client PID (pid_t) | system call (int) | num params (int) | param size (int) | param data (256 bytes) |
```

## How It Works

```
 ┌────────┐   request (PID, call #, params)    ┌────────┐
 │ client │ ─────────────────────────────────▶ │ server │
 │        │ ◀───────────────────────────────── │        │
 └────────┘      response via client FIFO      └────────┘
```

1. The server creates the well-known request FIFO and waits for input.
2. Each client creates its own FIFO named with its PID, then sends a connect request.
3. The server reads each request, switches on the system call number, and runs the matching function.
4. Results are written back to the client's own FIFO.

## Known Issues / Work in Progress

This is coursework in progress, and a few things are not finished yet:

- **FIFO name mismatch:** the server creates `request.fifo`, but the client opens `./server_fifo`. These need to use the same name before the two can talk to each other.
- **Response path:** the server's reply logic (`response_fifo_fd`, `params` buffer handling, `store`/`recall`) is incomplete and may not compile cleanly or behave correctly yet.
- **Number to text:** limited to numbers of 4 digits or fewer, and prints to the server's console instead of sending the result to the client.
- **Cleanup:** if the server exits uncleanly, the leftover FIFO file must be removed manually (`rm request.fifo`) before restarting, otherwise `mkfifo` fails.

## License

Released under the [MIT License](LICENSE). Copyright (c) 2023 Yansel F.
