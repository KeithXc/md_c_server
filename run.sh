#!/bin/bash

# Ensure any previous instances are stopped before starting.
./stop.sh

# Clean up cache and previous build artifacts
echo "Cleaning cache and build directories..."
if [ -d "cache" ]; then
    rm -rf cache/*
fi
# A clean build ensures consistency
cd build
make clean > /dev/null
cd ..

# Build and run
echo "Building project..."
cd build
cmake .. > /dev/null
make
cd ..

echo "" # Add a newline for cleaner output
echo "Starting md__web_server..."
nohup ./bin/md_web_server > server.log 2>&1 &
PID=$!
echo $PID > server.pid
echo "Server started with PID: $PID"
