#!/bin/bash

# Ensure any previous instances are stopped before starting.
./stop.sh

# Clean up cache (but keep build artifacts for incremental compilation)
echo "Cleaning cache..."
if [ -d "cache" ]; then
    rm -rf cache/*
fi

# Build and run
echo "Building project..."
mkdir -p build
cd build

# Run cmake if Makefile doesn't exist, otherwise let make handle re-configuration if needed
if [ ! -f "Makefile" ]; then
    cmake .. > /dev/null
fi

make
cd ..

echo "" # Add a newline for cleaner output
echo "Starting md_c_server..."
nohup ./bin/md_c_server > server.log 2>&1 &
PID=$!
echo $PID > server.pid
echo "Server started with PID: $PID"
