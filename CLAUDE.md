# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

DOS real-mode game engine built with Watcom C/C++ 10.x, targeting VGA Mode 13h (320x200x256). Developed on macOS, compiled inside DOSBox using the Watcom toolchain. This repository is engine-only — `SRC/GAME.CPP` is a minimal smoke-test `main()` and is meant to be replaced by a game.

## Build & Run

```bash
./build.sh          # Converts assets, launches DOSBox, compiles all SRC/*.CPP → SRC/GAME.EXE
./run.sh            # Runs SRC/GAME.EXE in DOSBox
./run.sh OTHER.EXE  # Runs a different executable
./convert.sh        # Asset pipeline only (called automatically by build.sh)
```

Build output goes to `BUILD.LOG` in the project root. The build script waits if another DOSBox instance is running.

Inside DOSBox, `BUILD.BAT` runs: `WCL386 *.CPP -fe=GAME.EXE` — all `.CPP` files in `SRC/` are compiled and linked in one step.

## Architecture

### Engine Modules (SRC/)

All source is C++ compiled with Watcom's `WCL386`. Uses `#pragma aux` for inline x86 assembly (Watcom-specific syntax, not AT&T or NASM).

| Module | Responsibility |
|--------|---------------|
| `GAME.CPP` | Engine smoke test — `main()` that initializes and shuts down every subsystem (replace when writing a game) |
| `GFX.CPP/H` | VGA Mode 13h: double-buffered rendering, vsync, drawing primitives, compiled sprite execution, palette |
| `INPUT.CPP/H` | INT 9 keyboard handler with real-time key state array + ring buffer for event-based input |
| `TIMER.CPP/H` | PIT reprogrammed to 1000 Hz via INT 8; provides `timer_ms()` millisecond clock |
| `SOUND.CPP/H` | OPL2 (AdLib) VGM player hooked into the timer ISR chain; detection, loading, playback |
| `SFX.CPP/H` | Sound Blaster PCM software mixer (auto-init DMA + IRQ-driven). Coexists with OPL2 music |
| `SPRITE.CPP/H` | Binary `.SPR` file loader for compiled sprites (x86 machine code frames) |
| `FONT.CPP/H` | Hangul (16x16 composed) + English (8x16) bitmap font renderer |
| `IMG.CPP/H` | Raw indexed `.IMG` loader |
| `SCRNCAP.CPP/H` | Screen capture helper |
| `PALETTE.H` | Generated 256-color palette (index 0 = transparent, 1-15 = grayscale, 16-255 = 15 color ramps × 16 shades) |

### ISR Chain (critical ordering)

Init order: `timer_init()` → `input_init()` → `snd_init()`. Shutdown is LIFO: `snd_close()` → `input_close()` → `timer_close()`. Sound chains through INT 8 to the timer ISR — breaking this order will crash.

### Asset Pipeline (host-side, requires Bun)

- `tools/mkpalette.ts` — Generates `SRC/PALETTE.H` (6-bit VGA) and `tools/palette.json` (8-bit RGB for sprite converter)
- `tools/mksprite.ts` — Converts PNG assets to compiled sprites. Key modes:
  - `--bin --grid CxR` → binary `.SPR` file (sprite sheets split into C×R frames)
  - `--raw` → raw pixel header (for backgrounds)
  - Default → C header with embedded machine code
- `tools/mkimg.ts` — Converts PNG → raw indexed `.IMG`
- `tools/mkfont.ts` — Compiles a bitmap font to an x86-glyph renderer blob
- `tools/mksfx.ts` — Converts audio to the SB PCM `.SFX` format
- `tools/bmp2png.ts` — BMP → PNG helper

Sprites are "compiled" — each frame is x86 machine code (MOV instructions targeting a linear buffer) executed via `call`. Transparent pixels (index 0, alpha < 128) are simply not emitted.

### File Naming Convention

DOS 8.3 filenames. Source files are UPPERCASE. Sprite files use `SPR_xxxx.SPR` prefix. All generated files go into `SRC/`.

## Watcom C++ Constraints

- No C++ exceptions, no RTTI, no STL — use C standard library only
- Inline assembly uses `#pragma aux` syntax (not `__asm` blocks)
- `__interrupt __far` for ISR functions; `_dos_getvect`/`_dos_setvect` for vector management
- `_chain_intr()` to chain ISRs; `outp()`/`inp()` for port I/O
- All integers are 32-bit flat model (DOS4GW extender)
- Filenames in source must be 8.3 UPPERCASE
