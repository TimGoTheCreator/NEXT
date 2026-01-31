#!/bin/sh

# Absolute path to the directory containing this script
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Project root = two levels up
ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# Run the executable from the root
"$ROOT/next" "$SCRIPT_DIR/two_bodies.txt"