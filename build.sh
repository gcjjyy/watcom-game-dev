#!/bin/bash
# DOSBox build script for Watcom C++ game project
# Compiles all *.CPP files in SRC/ and links into GAME.EXE

DOSBOX="/Applications/dosbox.app/Contents/MacOS/DOSBox"
PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Convert assets first
"$PROJECT_DIR/convert.sh"

# Wait if another DOSBox instance is already running
while pgrep -x DOSBox > /dev/null 2>&1; do
  echo "Another DOSBox instance is running. Waiting..."
  sleep 5
done

rm -f "$PROJECT_DIR/BUILD.LOG"

"$DOSBOX" \
  -c "MOUNT C $PROJECT_DIR" \
  -c "C:" \
  -c "BUILD.BAT" 2>/dev/null

echo "=== BUILD OUTPUT ==="
if [ -f "$PROJECT_DIR/BUILD.LOG" ]; then
  cat "$PROJECT_DIR/BUILD.LOG"
else
  echo "No BUILD.LOG found. DOSBox may have failed to start."
fi
