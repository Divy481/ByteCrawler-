# ByteCache Web Crawler

A high-performance, custom-built web crawler written in C++ using POSIX system calls and standard libraries. It features concurrent network requests via a custom ThreadPool, support for both HTTP and HTTPS using OpenSSL, and efficient handling of HTML fetching and link extraction.

## Features

- **Concurrent Crawling**: Leverages a custom thread pool to crawl multiple URLs concurrently, greatly increasing throughput.
- **HTTP/HTTPS Support**: Uses standard POSIX sockets and OpenSSL for secure (TLS/SSL) and non-secure communication.
- **Custom HTTP Parser**: Employs a robust, lightweight HTTP parser specifically optimized for the crawler.
- **URL Resolution**: Correctly resolves relative links extracted from HTML pages against the parent URL.
- **Domain Constraint**: Option to limit crawling strictly to the seed URL's domain.
- **Depth Control**: Easily define the maximum depth to crawl to prevent infinite traversal.

## Single-Threaded vs. Multi-Threaded Architecture

This web crawler leverages a multi-threaded architecture utilizing a custom ThreadPool, which provides significant performance improvements over a traditional single-threaded design:

| Feature | Single-Threaded Crawler | Multi-Threaded Crawler (ByteCache) |
| --- | --- | --- |
| **Network I/O** | Blocks execution. Must wait for DNS, TCP, and TLS handshakes to complete sequentially before starting the next fetch. | Non-blocking. Multiple HTTP/HTTPS requests are dispatched and awaited concurrently. |
| **Throughput** | Low. Bottlenecked by network latency and slow servers. | High. Bandwidth and CPU resources are utilized efficiently. |
| **Concurrency** | Processes one URL at a time. | Fetches and processes multiple URLs in parallel across different threads. |
| **Synchronization** | None required. | Robust thread-safety using `std::mutex` and `std::condition_variable` to protect the visited URL set and file I/O operations. |

## Requirements

- C++17 or later
- OpenSSL (`libssl-dev`, `libcrypto`)
- POSIX-compliant OS (Linux/macOS)
- CMake or simply a modern GCC/Clang compiler

## Build Instructions

You can compile the application directly using `g++`:

```bash
g++ main.cpp src/*.cpp -o run -lssl -lcrypto -pthread -Wall -Wextra
```

## Usage

Run the compiled executable with a seed URL and an optional maximum depth:

```bash
./run <SeedUrl> [MaxDepth]
```

### Example:

```bash
./run https://example.com 2
```

This will start the crawler at `https://example.com`, following links up to a depth of 2. Downloaded HTML files will be saved in the current directory as `<depth>.html`.

## Project Structure

- `include/`: Contains all header files defining classes and interfaces (`webCrawler.hpp`, `http_parser.hpp`, `threadpool.hpp`, etc.).
- `src/`: Contains the implementation files for the classes.
- `tests/`: Contains test suites to ensure component functionality.
- `main.cpp`: Entry point for the application.

## Acknowledgements
Designed as an educational project to understand web crawling, network socket programming, and concurrent processing in C++.
