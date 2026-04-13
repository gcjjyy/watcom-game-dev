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

# 6. Convert SFX source OGGs to .SFX (22050 Hz 8-bit u8 mono PCM)
SFX_SRC_DIR="${SFX_SRC_DIR:-/Users/gcjjyy/Documents/게임개발/fx_sounds}"
echo "Converting SFX from $SFX_SRC_DIR ..."
bun tools/mksfx.ts "$SFX_SRC_DIR/weapon-sound1.ogg"   SRC/SFX_ATK.SFX
bun tools/mksfx.ts "$SFX_SRC_DIR/be-att-sound20.ogg"  SRC/SFX_HIT.SFX
bun tools/mksfx.ts "$SFX_SRC_DIR/ui-sound-13.ogg"     SRC/SFX_SEL.SFX
bun tools/mksfx.ts "$SFX_SRC_DIR/ui-sound3.ogg"       SRC/SFX_OK.SFX
bun tools/mksfx.ts "$SFX_SRC_DIR/ui-sound9.ogg"       SRC/SFX_CAN.SFX
bun tools/mksfx.ts "$SFX_SRC_DIR/etc-sound0014.ogg"   SRC/SFX_WIN.SFX
bun tools/mksfx.ts "$SFX_SRC_DIR/etc-sound0038.ogg"   SRC/SFX_LOSE.SFX
bun tools/mksfx.ts "$SFX_SRC_DIR/monster-sound8.ogg"  SRC/SFX_ENC.SFX
