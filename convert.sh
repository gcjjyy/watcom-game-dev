#!/bin/bash
# Asset conversion pipeline
# Converts RPG tileset and character PNGs to binary formats

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$PROJECT_DIR"

# 1. Generate palette
echo "Generating palette..."
bun tools/mkpalette.ts

# 2. Generate RPG placeholder assets
echo "Generating RPG assets..."
bun tools/mkassets.ts

# 3. Convert tileset PNG to .TIL binary
echo "  tileset.png -> SRC/TILES.TIL"
bun tools/mktile.ts assets/tileset.png SRC/TILES.TIL --tw 16 --th 16

# 4. Convert character sprites to .SPR binary
echo "  hero.png -> SRC/SPR_HERO.SPR (grid 1x4)"
bun tools/mksprite.ts --bin --grid 1x4 assets/hero.png SPR_HERO

echo "  npc.png -> SRC/SPR_NPC1.SPR (grid 1x4)"
bun tools/mksprite.ts --bin --grid 1x4 assets/npc.png SPR_NPC1

echo "  enemy.png -> SRC/SPR_ENM1.SPR (grid 1x1, 32x32 battle sprite)"
bun tools/mksprite.ts --bin --grid 1x1 assets/enemy.png SPR_ENM1

echo "Done: 1 tileset + 3 character sprites."

# 5. Compiled font
echo "Building compiled font..."
bun tools/mkfont.ts
