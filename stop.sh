#!/bin/bash
# This script stops all instances of the md_web_server.

SERVER_NAME="md_c_server"
PID_FILE="server.pid"

echo "Stopping all '$SERVER_NAME' processes..."
# Use pkill to find and kill all processes with the server name.
# The -f flag matches against the full command line.
if pkill -f "$SERVER_NAME"; then
    echo "Server process(es) terminated."
else
    echo "No running server process found."
fi

# Clean up the PID file if it exists
if [ -f "$PID_FILE" ]; then
    rm "$PID_FILE"
fi