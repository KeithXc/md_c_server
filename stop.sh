#!/bin/bash
# This script stops the server.

PID_FILE="server.pid"

if [ -f "$PID_FILE" ]; then
    PID=$(cat "$PID_FILE")
    echo "Stopping server with PID: $PID"
    # Use kill to terminate the process
    if kill $PID > /dev/null 2>&1; then
        echo "Server stopped."
    else
        echo "Failed to stop server. It may not be running."
    fi
    # Clean up the PID file
    rm "$PID_FILE"
else
    echo "Server PID file not found. Is the server running?"
fi
