# watcom-game-dev

DOS scrolling shooter game built with Watcom C/C++ 10.6, targeting VGA Mode 13h (320x200, 256 colors). Developed on macOS, compiled inside DOSBox.

## Screenshot

The game features a vertically scrolling desert background, animated player ship, three enemy types, bullet firing with collision detection, explosions, and OPL2 VGM music playback.

## Features

- **Double-buffered VGA rendering** with vsync
- **Compiled sprites** — sprite frames are x86 machine code (MOV instructions), no pixel loops
- **Compiled font system** — Korean (8x4x4-bul 16x16) + English (8x16) bitmap fonts precompiled to x86 code, with software fallback for uncached characters
- **OPL2 VGM player** — plays .VGM music files through AdLib FM synthesis
- **1000 Hz timer** — PIT reprogrammed for millisecond-precision delta-time game loop
- **Custom keyboard handler** — INT 9 ISR with real-time key state + event buffer

## Requirements

- [DOSBox](https://www.dosbox.com/) (0.74+)
- [Bun](https://bun.sh/) (for asset pipeline)
- macOS (build scripts assume `/Applications/dosbox.app`)

## Build & Run

```bash
./build.sh      # Convert assets + compile in DOSBox → SRC/GAME.EXE
./run.sh         # Run GAME.EXE in DOSBox
```

## Project Structure

```
SRC/            Watcom C++ source code (8.3 UPPERCASE filenames)
  GAME.CPP      Main loop, game state, entity management
  GFX.CPP/H     VGA Mode 13h graphics, compiled sprite renderer
  INPUT.CPP/H   INT 9 keyboard handler
  TIMER.CPP/H   PIT 1000Hz timer via INT 8
  SOUND.CPP/H   OPL2 VGM music player
  SPRITE.CPP/H  Binary .SPR sprite loader
  FONT.CPP/H    Compiled font renderer with Korean/English support

tools/          Asset pipeline (TypeScript/Bun)
  mkpalette.ts  256-color palette generator
  mksprite.ts   PNG → compiled sprite converter
  mkfont.ts     Bitmap font → compiled x86 glyph converter

fonts/          Bitmap font files (multiple styles for easy swapping)
assets/         Source PNG sprite sheets
VGM/            VGM music files
WATCOM/         Watcom C/C++ 10.6 toolchain (headers + libs)
```

## Font System

The font system supports mixed Korean/English text rendering via UTF-8 strings:

```c
font_puts(x, y, "Hello 한글!", color);
```

- **Fast path**: Characters found in source code are precompiled to x86 at build time
- **Fallback**: Unknown characters are rendered at runtime using the 8x4x4-bul composition algorithm

To swap fonts, pass `--eng` and `--han` flags to mkfont.ts:

```bash
bun tools/mkfont.ts --eng fonts/ENG_MAX.FNT --han fonts/HAN_DEW_MYUNG.FNT
```

## License

[MIT](LICENSE)
