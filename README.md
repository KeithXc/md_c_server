# C Markdown Blog

A simple, lightweight, and fast web server written in C to serve Markdown files as a blog.

## Live Demo

A live version of this server (when running) can be accessed at:
[http://keithmo.com.cpolar.top](http://keithmo.com.cpolar.top)

## Core Technologies

-   **Language**: C
-   **Build System**: CMake
-   **Core Library**: `mongoose` for handling HTTP requests.

## Features

-   Scans a directory (`md/`) for Markdown files.
-   Displays a clickable, collapsible tree view of all `.md` files and subdirectories on the homepage.
-   Serves the raw content of Markdown files when a link is clicked.

## How to Build and Run

The easiest way to build and run the server is by using the provided scripts:

```bash
# To compile and start the server:
./run.sh

# To stop the server:
./stop.sh
```

The server will then be accessible at `http://localhost:8000`.

For manual compilation, you can follow these steps:
```bash
# 1. Create a build directory
mkdir -p build
cd build

# 2. Generate the Makefile
cmake ..

# 3. Compile the source code
make

# 4. Run the server
../bin/md_c_server
```