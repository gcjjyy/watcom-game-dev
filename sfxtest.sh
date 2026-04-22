#!/bin/bash
# Build + run the standalone SFXTEST tool for fast SFX / VGM iteration.
# Usage: ./sfxtest.sh [scenario] [sfx_slot]
#   scenario: 1=VGM-then-SFX 2=SFX-then-VGM 3=S2+restart 4=S2+opl2_clear
#   sfx_slot: 0..7 (default 0 = SFX_ATK)
#
# Writes SFXTEST/SNDLOG.TXT and prints it at the end.

DOSBOX="/Applications/dosbox.app/Contents/MacOS/DOSBox"
PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
SCENARIO="${1:-1}"
SLOT="${2:-0}"

# Wait for any running DOSBox
while pgrep -x DOSBox > /dev/null 2>&1; do
  echo "Another DOSBox instance is running. Waiting..."
  sleep 2
done

rm -f "$PROJECT_DIR/SFXTEST.LOG"
rm -f "$PROJECT_DIR/SFXTEST/SNDLOG.TXT"

# Build phase
"$DOSBOX" \
  -c "MOUNT C $PROJECT_DIR" \
  -c "C:" \
  -c "SFXBUILD.BAT" 2>/dev/null

echo "=== BUILD LOG ==="
if [ -f "$PROJECT_DIR/SFXTEST.LOG" ]; then
  tail -30 "$PROJECT_DIR/SFXTEST.LOG"
else
  echo "(no build log)"
fi

if [ ! -f "$PROJECT_DIR/SFXTEST/SFXTEST.EXE" ]; then
  echo "Build failed - SFXTEST.EXE not produced"
  exit 1
fi

echo ""
echo "=== RUN (scenario=$SCENARIO slot=$SLOT) ==="
echo "    — listen for VGM music + SFX — DOSBox will close after ~10 seconds"

# Run phase - DOSBox closes via -exit after autoexec commands finish
"$DOSBOX" \
  -c "MOUNT C $PROJECT_DIR" \
  -c "C:" \
  -c "CALL AUTOEXEC.BAT" \
  -c "CD SFXTEST" \
  -c "SFXTEST.EXE $SCENARIO $SLOT" \
  -exit 2>/dev/null

echo ""
echo "=== SNDLOG.TXT ==="
if [ -f "$PROJECT_DIR/SFXTEST/SNDLOG.TXT" ]; then
  cat "$PROJECT_DIR/SFXTEST/SNDLOG.TXT"
else
  echo "(no log captured)"
fi
