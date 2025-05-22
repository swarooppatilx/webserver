# Simple HTTP Server in C

This is a simple multithreaded HTTP server written in C using POSIX sockets and `pthread`. It listens on port `8080` and responds with basic HTML for `/`, `/about`, and returns a 404 for any other path.

## Features

- Handles multiple client connections using threads
- Supports GET requests for:
  - `/` → Home Page
  - `/about` → About Page
  - All other paths → 404 Page

## Build

```bash
gcc server.c -o server -lpthread
```

## Run
```bash
./server
```

Open localhost:8080 on your browser.
