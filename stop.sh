#!/bin/bash
# This script stops all instances of the server, including old named ones.

OLD_SERVER_NAME="md_web_server"
SERVER_NAME="md_c_server"
PID_FILE="server.pid"

echo "Stopping all '$SERVER_NAME' and '$OLD_SERVER_NAME' processes..."

# Use pkill to find and kill all processes for both names.
# The -f flag matches against the full command line.
pkill -f "$SERVER_NAME"
pkill -f "$OLD_SERVER_NAME"

# Check if any process was killed. pkill returns 0 if one or more processes were matched.
if [ $? -eq 0 ]; then
    echo "Server process(es) terminated."
else
    echo "No running server process found."
fi

# Clean up the PID file if it exists
if [ -f "$PID_FILE" ]; then
    rm "$PID_FILE"
fi
