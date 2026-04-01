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
echo "  ship.png -> SRC/SPR_SHIP.H (grid 5x2)"
bun tools/mksprite.ts --grid 5x2 assets/ship.png SPR_SHIP > SRC/SPR_SHIP.H

# 4. Enemies
echo "  enemy-small.png -> SRC/SPR_ENSM.H (grid 2x1)"
bun tools/mksprite.ts --grid 2x1 assets/enemy-small.png SPR_ENSM > SRC/SPR_ENSM.H

echo "  enemy-medium.png -> SRC/SPR_ENMD.H (grid 2x1)"
bun tools/mksprite.ts --grid 2x1 assets/enemy-medium.png SPR_ENMD > SRC/SPR_ENMD.H

echo "  enemy-big.png -> SRC/SPR_ENLG.H (grid 2x1)"
bun tools/mksprite.ts --grid 2x1 assets/enemy-big.png SPR_ENLG > SRC/SPR_ENLG.H

# 5. Out-of-scope assets (converted for future use, not used in GAME.CPP)
echo "  laser-bolts.png -> SRC/SPR_LASR.H (grid 2x2)"
bun tools/mksprite.ts --grid 2x2 assets/laser-bolts.png SPR_LASR > SRC/SPR_LASR.H

echo "  explosion.png -> SRC/SPR_EXPL.H (grid 5x1)"
bun tools/mksprite.ts --grid 5x1 assets/explosion.png SPR_EXPL > SRC/SPR_EXPL.H

echo "  power-up.png -> SRC/SPR_PWUP.H (grid 2x2)"
bun tools/mksprite.ts --grid 2x2 assets/power-up.png SPR_PWUP > SRC/SPR_PWUP.H

echo "Done: 1 background + 7 sprite sheets."
