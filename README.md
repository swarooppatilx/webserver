# Simple HTTP Server in C

This is a simple multithreaded HTTP server written in C using POSIX sockets and `pthread`. It listens on port `8080` and serves static files from the `static/` directory.

## Features

- Handles multiple client connections using threads
- Supports GET requests for:
  - `/` → serves `static/index.html`
  - `/about.html` → serves `static/about.html`
  - `/tux.png` → serves `static/tux.png`
  - Any other path → returns 404 Not Found
- Automatically sets correct MIME types based on file extension
- Prevents directory traversal using `..` check
- Basic error handling for:
  - 403 Forbidden (path traversal attempt)
  - 404 Not Found (file not found)
  - 414 URI Too Long

## Build

```bash
gcc server.c -o server -lpthread
```

## Run
```bash
./server
```

open your browser and visit
```bash
http://localhost:8080/
http://localhost:8080/about.html
http://localhost:8080/tux.png
```
