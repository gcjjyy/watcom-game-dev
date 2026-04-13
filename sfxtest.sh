#!/bin/bash
# Build + run the standalone SFXTEST tool for fast SFX iteration.
# Usage: ./sfxtest.sh [slot]   (slot = 0..7, defaults to 0 = SFX_ATK)
#
# Produces SFXTEST/SFXTEST.EXE and runs it inside DOSBox.
# stdout from SFXTEST is redirected to SFXTEST/OUT.TXT and printed here.

DOSBOX="/Applications/dosbox.app/Contents/MacOS/DOSBox"
PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
SLOT="${1:-0}"

# Wait for any running DOSBox
while pgrep -x DOSBox > /dev/null 2>&1; do
  echo "Another DOSBox instance is running. Waiting..."
  sleep 2
done

rm -f "$PROJECT_DIR/SFXTEST.LOG"
rm -f "$PROJECT_DIR/SFXTEST/OUT.TXT"

# Build phase
"$DOSBOX" \
  -c "MOUNT C $PROJECT_DIR" \
  -c "C:" \
  -c "SFXBUILD.BAT" 2>/dev/null

echo "=== BUILD LOG ==="
if [ -f "$PROJECT_DIR/SFXTEST.LOG" ]; then
  cat "$PROJECT_DIR/SFXTEST.LOG"
else
  echo "(no build log)"
fi

if [ ! -f "$PROJECT_DIR/SFXTEST/SFXTEST.EXE" ]; then
  echo "Build failed - SFXTEST.EXE not produced"
  exit 1
fi

echo ""
echo "=== RUN (slot=$SLOT) ==="

# Run phase - redirect stdout to OUT.TXT inside DOSBox so we can capture it
"$DOSBOX" \
  -c "MOUNT C $PROJECT_DIR" \
  -c "C:" \
  -c "CALL AUTOEXEC.BAT" \
  -c "CD SFXTEST" \
  -c "SFXTEST.EXE $SLOT > OUT.TXT" \
  -c "EXIT" 2>/dev/null &

DBPID=$!
# Give it up to 10 seconds; some SFX are ~5s
sleep 8
pkill -f dosbox 2>/dev/null
wait $DBPID 2>/dev/null

echo "=== SFXTEST OUTPUT ==="
if [ -f "$PROJECT_DIR/SFXTEST/OUT.TXT" ]; then
  cat "$PROJECT_DIR/SFXTEST/OUT.TXT"
else
  echo "(no output captured — SFXTEST may have crashed DOSBox before flushing)"
fi
