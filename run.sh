#!/bin/sh
# Usage: ./run.sh [POLICY]

unlink /tmp/pq.$USER

if make; then
    ./pq -p $1 -f /tmp/pq.$USER
else
    echo "Error in make"
fi
