#!/bin/bash
# This script compiles and runs the server.

set -e

# Navigate to build directory, create it if it doesn't exist
mkdir -p build
cd build

# Generate Makefile and compile
cmake ..
make

# Navigate back to the project root
cd ..

# Run the server in the background and save its PID
echo "Starting md_web_server..."
./bin/md_web_server &
echo $! > server.pid
echo "Server started with PID: $(cat server.pid)"
