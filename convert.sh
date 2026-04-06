#!/bin/bash
# Asset conversion pipeline
# Converts museum PNG assets to compiled sprite and raw data headers

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$PROJECT_DIR"

# 1. Generate palette
echo "Generating palette..."
bun tools/mkpalette.ts

# 2. Background (raw pixel data, not compiled sprite)
echo "  desert.png -> SRC/BG_DSRT.H (raw)"
bun tools/mksprite.ts --raw assets/desert.png BG_DSRT > SRC/BG_DSRT.H

# 3. Player ship (5 columns x 2 rows = 10 frames of 16x24)
echo "  ship.png -> SRC/SPR_SHIP.SPR (grid 5x2)"
bun tools/mksprite.ts --bin --grid 5x2 assets/ship.png SPR_SHIP

# 4. Enemies
echo "  enemy-small.png -> SRC/SPR_ENSM.SPR (grid 2x1)"
bun tools/mksprite.ts --bin --grid 2x1 assets/enemy-small.png SPR_ENSM

echo "  enemy-medium.png -> SRC/SPR_ENMD.SPR (grid 2x1)"
bun tools/mksprite.ts --bin --grid 2x1 assets/enemy-medium.png SPR_ENMD

echo "  enemy-big.png -> SRC/SPR_ENLG.SPR (grid 2x1)"
bun tools/mksprite.ts --bin --grid 2x1 assets/enemy-big.png SPR_ENLG

# 5. Additional sprites
echo "  laser-bolts.png -> SRC/SPR_LASR.SPR (grid 2x2)"
bun tools/mksprite.ts --bin --grid 2x2 assets/laser-bolts.png SPR_LASR

echo "  explosion.png -> SRC/SPR_EXPL.SPR (grid 5x1)"
bun tools/mksprite.ts --bin --grid 5x1 assets/explosion.png SPR_EXPL

echo "  power-up.png -> SRC/SPR_PWUP.SPR (grid 2x2)"
bun tools/mksprite.ts --bin --grid 2x2 assets/power-up.png SPR_PWUP

echo "Done: 1 background + 7 sprite sheets (.SPR binary)."

# 6. Compiled font
echo "Building compiled font..."
bun tools/mkfont.ts
