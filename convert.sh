#!/bin/bash
# Engine asset pipeline. Generates PALETTE.H and nothing else.
# Extend with mksprite/mkimg/mkfont/mksfx calls as assets are added.

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$PROJECT_DIR"

set -e

echo "Generating palette..."
bun tools/mkpalette.ts

echo "Done."
