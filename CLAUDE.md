# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

DOS scrolling-shooter game built with Watcom C/C++ 10.x, targeting real-mode VGA (Mode 13h, 320x200x256). Developed on macOS, compiled inside DOSBox using the Watcom toolchain.

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

### Source Modules (SRC/)

All source is C++ compiled with Watcom's `WCL386`. Uses `#pragma aux` for inline x86 assembly (Watcom-specific syntax, not AT&T or NASM).

| Module | Responsibility |
|--------|---------------|
| `GAME.CPP` | Main loop, game state, entity management (player, enemies, bullets, explosions) |
| `GFX.CPP/H` | VGA Mode 13h: double-buffered rendering, vsync, drawing primitives, compiled sprite execution, palette |
| `INPUT.CPP/H` | INT 9 keyboard handler with real-time key state array + ring buffer for event-based input |
| `TIMER.CPP/H` | PIT reprogrammed to 1000 Hz via INT 8; provides `timer_ms()` millisecond clock |
| `SOUND.CPP/H` | OPL2 (AdLib) VGM player hooked into the timer ISR chain; detection, loading, playback |
| `SPRITE.CPP/H` | Binary `.SPR` file loader for compiled sprites (x86 machine code frames) |
| `PALETTE.H` | Generated 256-color palette (index 0 = transparent, 1-15 = grayscale, 16-255 = 15 color ramps × 16 shades) |
| `BG_DSRT.H` | Generated raw pixel data for the scrolling background |

### ISR Chain (critical ordering)

Init order: `timer_init()` → `input_init()` → `snd_init()`. Shutdown is LIFO: `snd_close()` → `input_close()` → `timer_close()`. Sound chains through INT 8 to the timer ISR — breaking this order will crash.

### Asset Pipeline (host-side, requires Bun)

- `tools/mkpalette.ts` — Generates `SRC/PALETTE.H` (6-bit VGA) and `tools/palette.json` (8-bit RGB for sprite converter)
- `tools/mksprite.ts` — Converts PNG assets to compiled sprites. Key modes:
  - `--bin --grid CxR` → binary `.SPR` file (sprite sheets split into C×R frames)
  - `--raw` → raw pixel header (for backgrounds)
  - Default → C header with embedded machine code

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

## Multi-Agent Harness

This project uses a custom multi-agent development harness (skills in `.claude/skills/`). Use `/harness` to run the full pipeline: Plan → Dev Alpha/Beta → Synthesize → Implement → Review → QA → Retrospective.
