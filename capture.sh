#!/bin/bash
# Graphics capture harness.
# 1. Places CAPTURE.FLG in SRC/ so GAME.EXE runs capture_run() on startup.
# 2. Launches DOSBox, which auto-exits after GAME.EXE returns.
# 3. Converts captured BMPs to PNGs under captures/ for agent review.

set -e

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$PROJECT_DIR"

DOSBOX="/Applications/dosbox.app/Contents/MacOS/DOSBox"
SRC_DIR="$PROJECT_DIR/SRC"
OUT_DIR="$PROJECT_DIR/captures"

mkdir -p "$OUT_DIR"
rm -f "$OUT_DIR"/*.png
rm -f "$SRC_DIR"/CAP*.BMP

# Trigger capture mode in GAME.EXE by creating a sentinel file.
touch "$SRC_DIR/CAPTURE.FLG"

# Run DOSBox with auto-exit. GAME.EXE in capture mode runs the harness
# then returns to DOS, at which point DOSBox processes EXIT.
"$DOSBOX" \
  -c "MOUNT C $PROJECT_DIR" \
  -c "C:" \
  -c "CALL AUTOEXEC.BAT" \
  -c "CD SRC" \
  -c "GAME.EXE" \
  -c "EXIT" 2>/dev/null

# Clean up the sentinel and stray BMPs the harness may have generated
# before exit.
rm -f "$SRC_DIR/CAPTURE.FLG"

shopt -s nullglob
bmps=("$SRC_DIR"/CAP*.BMP)
if [ ${#bmps[@]} -eq 0 ]; then
  echo "capture.sh: no BMPs produced — capture mode may have failed" >&2
  exit 1
fi

echo "Converting ${#bmps[@]} BMPs to PNG..."
bun "$PROJECT_DIR/tools/bmp2png.ts" "${bmps[@]}" "$OUT_DIR"

# Remove the BMPs from SRC/ so they don't leak into convert.sh or build.
rm -f "$SRC_DIR"/CAP*.BMP

echo "Captured PNGs under $OUT_DIR:"
ls "$OUT_DIR"
