#!/bin/bash

# Ensure any previous instances are stopped before starting.
./stop.sh

# Build and run
echo "Building project..."
cd build
cmake .. > /dev/null
make
cd ..

echo "" # Add a newline for cleaner output
echo "Starting md_web_server..."
nohup ./bin/md_web_server > /dev/null 2>&1 &
PID=$!
echo $PID > server.pid
echo "Server started with PID: $PID"