# C Markdown Blog

A simple, lightweight, and fast web server written in C to serve Markdown files as a blog.

## Core Technologies

-   **Language**: C
-   **Build System**: CMake
-   **Core Library**: `mongoose` for handling HTTP requests.

## Features

-   Scans a directory (`md/`) for Markdown files.
-   Displays a clickable, collapsible tree view of all `.md` files and subdirectories on the homepage.
-   Serves the raw content of Markdown files when a link is clicked.

## How to Build and Run

The project is built using CMake:

```bash
# 1. Create a build directory
mkdir -p build
cd build

# 2. Generate the Makefile
cmake ..

# 3. Compile the source code
make

# 4. Run the server
../bin/md_web_server
```

The server will then be accessible at `http://localhost:8000`.
