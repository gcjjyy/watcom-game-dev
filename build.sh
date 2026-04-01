#!/bin/bash
# DOSBox build script for Watcom C++ game project
# Usage: ./build.sh [source_file]
#   e.g. ./build.sh GAME.CPP

DOSBOX="/Applications/dosbox.app/Contents/MacOS/DOSBox"
PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
SRC_FILE="${1:-GAME.CPP}"

# Remove old build log
rm -f "$PROJECT_DIR/BUILD.LOG"

"$DOSBOX" \
  -c "MOUNT C $PROJECT_DIR" \
  -c "C:" \
  -c "CALL AUTOEXEC.BAT" \
  -c "CD SRC" \
  -c "WCL386 $SRC_FILE -fe=GAME.EXE > C:\BUILD.LOG 2>&1" \
  -c "EXIT"

echo "=== BUILD OUTPUT ==="
if [ -f "$PROJECT_DIR/BUILD.LOG" ]; then
  cat "$PROJECT_DIR/BUILD.LOG"
else
  echo "No BUILD.LOG found. DOSBox may have failed to start."
fi
