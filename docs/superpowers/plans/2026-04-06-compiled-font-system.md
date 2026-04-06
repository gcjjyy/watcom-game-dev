# Compiled Font System Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add Korean+English font rendering with precompiled x86 glyph code and software fallback.

**Architecture:** `mkfont.ts` (Bun) scans `.CPP` source for used characters, reads 8x4x4-bul Korean + 8x16 English bitmap fonts, compiles each glyph to x86 MOV instructions (color via EAX register), outputs `FONT.BIN`. DOS side loads the binary, does binary search per codepoint; cache miss falls back to nibble-table software renderer using raw `.FNT` data.

**Tech Stack:** TypeScript/Bun (build tool), Watcom C++ 10.6 (DOS runtime), x86 32-bit flat model

---

### Task 1: Create `tools/mkfont.ts` — font compiler

**Files:**
- Create: `tools/mkfont.ts`

This is the largest task. It reads font files, scans source for characters, compiles x86 code, and writes FONT.BIN.

- [ ] **Step 1: Scaffold CLI and font loading**

```ts
#!/usr/bin/env bun
/**
 * mkfont.ts - Compile bitmap fonts to x86 glyph code for DOS VGA
 *
 * Reads 8x16 English font (4096B) and 16x16 Korean 8x4x4-bul font (11520B),
 * scans SRC/*.CPP for used characters, compiles to FONT.BIN.
 *
 * Usage:
 *   bun tools/mkfont.ts [--eng <path>] [--han <path>]
 */

import { readFileSync, writeFileSync, readdirSync, copyFileSync } from "fs";
import { parseArgs } from "util";

const { values } = parseArgs({
  args: Bun.argv.slice(2),
  options: {
    eng: { type: "string", default: "fonts/ENG_HGDIARY.FNT" },
    han: { type: "string", default: "fonts/HAN_GOTHIC.FNT" },
  },
  allowPositionals: false,
});

// ---- Load font files ----

const engFont = readFileSync(values.eng!);
if (engFont.length !== 4096) throw new Error(`English font must be 4096 bytes, got ${engFont.length}`);

const hanFont = readFileSync(values.han!);
if (hanFont.length !== 11520) throw new Error(`Korean font must be 11520 bytes, got ${hanFont.length}`);
```

- [ ] **Step 2: Add source scanner to extract unique characters from string literals**

```ts
// ---- Scan SRC/*.CPP for characters used in string literals ----

function scanSourceChars(): Set<number> {
  const chars = new Set<number>();
  const files = readdirSync("SRC").filter(f => f.endsWith(".CPP") || f.endsWith(".H"));

  for (const file of files) {
    const src = readFileSync(`SRC/${file}`, "utf-8");
    const re = /"((?:[^"\\]|\\.)*)"/g;
    let m: RegExpExecArray | null;
    while ((m = re.exec(src)) !== null) {
      const raw = m[1];
      let i = 0;
      while (i < raw.length) {
        if (raw[i] === "\\") { i += 2; continue; }
        const cp = raw.codePointAt(i)!;
        if (cp >= 0x20 && cp <= 0x7E) chars.add(cp);
        else if (cp >= 0xAC00 && cp <= 0xD7A3) chars.add(cp);
        i += cp > 0xFFFF ? 2 : 1;
      }
    }
  }

  // Always include ASCII printable range
  for (let c = 0x20; c <= 0x7E; c++) chars.add(c);
  return chars;
}

const usedChars = scanSourceChars();
console.error(`Found ${usedChars.size} unique characters (${[...usedChars].filter(c => c >= 0xAC00).length} Korean)`);
```

- [ ] **Step 3: Add Korean decomposition and bul selection**

```ts
// ---- Korean decomposition and bul selection ----

function decomposeHangul(cp: number): { cho: number; jung: number; jong: number } {
  const idx = cp - 0xAC00;
  return {
    cho: Math.floor(idx / 588),
    jung: Math.floor((idx % 588) / 28),
    jong: idx % 28,
  };
}

const CHO_BUL: number[][] = [
  [0,0,0,0,0,0,0,0, 1,3,3,3,1, 2,4,4,4,2, 1,3,0],  // no jong
  [5,5,5,5,5,5,5,5, 6,7,7,7,6, 6,7,7,7,6, 6,7,5],  // has jong
];
const JONG_BUL = [0,2,0,2,1,2,1,2, 3,0,2,1,3, 3,1,2,1,3, 3,1,1];

function getChoBul(jung: number, hasJong: boolean): number {
  return CHO_BUL[hasJong ? 1 : 0][jung];
}
function getJungBul(cho: number, hasJong: boolean): number {
  return ((cho === 0 || cho === 16) ? 0 : 1) + (hasJong ? 2 : 0);
}
function getJongBul(jung: number): number {
  return JONG_BUL[jung];
}
```

- [ ] **Step 4: Add Korean glyph composition**

```ts
// ---- Korean glyph composition ----

function composeHangulBitmap(cp: number): Uint8Array {
  const { cho, jung, jong } = decomposeHangul(cp);
  const hasJong = jong > 0;
  const choBul = getChoBul(jung, hasJong);
  const jungBul = getJungBul(cho, hasJong);
  const jongBul = hasJong ? getJongBul(jung) : 0;

  const choIdx  = choBul * 20 + (cho + 1);
  const jungIdx = 160 + jungBul * 22 + (jung + 1);
  const jongIdx = 248 + jongBul * 28 + jong;

  const result = new Uint8Array(32);
  for (let i = 0; i < 32; i++) {
    result[i] = hanFont[choIdx * 32 + i]
              | hanFont[jungIdx * 32 + i]
              | hanFont[jongIdx * 32 + i];
  }
  return result;
}
```

- [ ] **Step 5: Add x86 code generator (color-via-EAX)**

```ts
// ---- x86 compiled glyph code gen ----
// MOV [EDI+disp32], EAX  -> 89 87 xx xx xx xx       (4 pixels)
// MOV [EDI+disp32], AX   -> 66 89 87 xx xx xx xx    (2 pixels)
// MOV [EDI+disp32], AL   -> 88 87 xx xx xx xx       (1 pixel)
// RET                    -> C3

function compileFontGlyph(w: number, h: number, bitmap1bit: Uint8Array): Uint8Array {
  const code: number[] = [];
  const stride = 320;

  const pushLE32 = (v: number) => {
    code.push(v & 0xFF, (v >> 8) & 0xFF, (v >> 16) & 0xFF, (v >> 24) & 0xFF);
  };

  const bytesPerRow = Math.ceil(w / 8);

  for (let y = 0; y < h; y++) {
    let x = 0;
    while (x < w) {
      // Check if pixel is set
      const byteIdx = y * bytesPerRow + Math.floor(x / 8);
      const bitIdx = 7 - (x % 8);
      if (!((bitmap1bit[byteIdx] >> bitIdx) & 1)) { x++; continue; }

      // Find run of set pixels
      const runStart = x;
      while (x < w) {
        const bi = y * bytesPerRow + Math.floor(x / 8);
        const bt = 7 - (x % 8);
        if (!((bitmap1bit[bi] >> bt) & 1)) break;
        x++;
      }
      const runLen = x - runStart;

      const baseOff = y * stride + runStart;
      let pos = 0;
      while (pos < runLen) {
        const remaining = runLen - pos;
        const off = baseOff + pos;

        if (remaining >= 4) {
          code.push(0x89, 0x87); pushLE32(off); pos += 4;
        } else if (remaining >= 2) {
          code.push(0x66, 0x89, 0x87); pushLE32(off); pos += 2;
        } else {
          code.push(0x88, 0x87); pushLE32(off); pos += 1;
        }
      }
    }
  }

  code.push(0xC3); // RET
  return new Uint8Array(code);
}
```

- [ ] **Step 6: Add main build loop and FONT.BIN writer**

```ts
// ---- Build all glyphs ----

interface GlyphEntry { codepoint: number; code: Uint8Array; }
const glyphs: GlyphEntry[] = [];

for (const cp of [...usedChars].sort((a, b) => a - b)) {
  let code: Uint8Array;
  if (cp >= 0x20 && cp <= 0x7E) {
    const bitmap = engFont.subarray(cp * 16, cp * 16 + 16);
    code = compileFontGlyph(8, 16, bitmap);
  } else if (cp >= 0xAC00 && cp <= 0xD7A3) {
    const bitmap = composeHangulBitmap(cp);
    code = compileFontGlyph(16, 16, bitmap);
  } else {
    continue;
  }
  glyphs.push({ codepoint: cp, code });
}

// ---- Write FONT.BIN ----

const headerSize = 8;
const indexEntrySize = 10;
const indexSize = glyphs.length * indexEntrySize;
let totalCode = 0;
for (const g of glyphs) totalCode += g.code.length;

const bin = Buffer.alloc(headerSize + indexSize + totalCode);
let boff = 0;

// Header: "CFNT" + u16 num_glyphs + u8 eng_w + u8 han_w
bin.write("CFNT", 0, 4, "ascii"); boff += 4;
bin.writeUInt16LE(glyphs.length, boff); boff += 2;
bin[boff++] = 8;
bin[boff++] = 16;

// Index
let codeOff = 0;
for (const g of glyphs) {
  bin.writeUInt32LE(g.codepoint, boff); boff += 4;
  bin.writeUInt32LE(codeOff, boff); boff += 4;
  bin.writeUInt16LE(g.code.length, boff); boff += 2;
  codeOff += g.code.length;
}

// Code section
for (const g of glyphs) {
  Buffer.from(g.code).copy(bin, boff);
  boff += g.code.length;
}

writeFileSync("SRC/FONT.BIN", bin);
console.error(`Compiled ${glyphs.length} glyphs -> SRC/FONT.BIN (${bin.length} bytes)`);

// ---- Copy raw fonts for fallback ----
copyFileSync(values.eng!, "SRC/ENG.FNT");
copyFileSync(values.han!, "SRC/HAN.FNT");
console.error(`Copied ${values.eng} -> SRC/ENG.FNT, ${values.han} -> SRC/HAN.FNT`);
```

- [ ] **Step 7: Run mkfont.ts and verify output**

Run: `cd /Users/gcjjyy/lab/watcom-game-dev && bun tools/mkfont.ts`

Expected: `SRC/FONT.BIN`, `SRC/ENG.FNT`, `SRC/HAN.FNT` created.

Verify: `ls -la SRC/FONT.BIN SRC/ENG.FNT SRC/HAN.FNT`

- [ ] **Step 8: Commit**

```bash
git add tools/mkfont.ts fonts/
git commit -m "Add mkfont.ts font compiler and font file collection"
```

---

### Task 2: Create `SRC/FONT.H`

**Files:**
- Create: `SRC/FONT.H`

- [ ] **Step 1: Write FONT.H**

```c
#ifndef FONT_H_INCLUDED
#define FONT_H_INCLUDED

#define FONT_ENG_W    8
#define FONT_ENG_H   16
#define FONT_HAN_W   16
#define FONT_HAN_H   16

void font_init(void);
void font_close(void);

void font_puts(int x, int y, const char *str, unsigned char color);
int  font_text_width(const char *str);

#endif
```

- [ ] **Step 2: Commit**

```bash
git add SRC/FONT.H
git commit -m "Add FONT.H header"
```

---

### Task 3: Create `SRC/FONT.CPP`

**Files:**
- Create: `SRC/FONT.CPP`

- [ ] **Step 1: Write file loading, binary search, UTF-8 decoder, fallback renderer, and public API**

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FONT.H"
#include "GFX.H"

/* ---- compiled font binary data ---- */

static unsigned char *_bin_data = NULL;
static unsigned long  _bin_size = 0;
static int            _num_glyphs = 0;
static unsigned char *_bin_index = NULL;
static unsigned char *_bin_code  = NULL;

/* ---- raw font data for fallback ---- */

static unsigned char _eng_font[4096];
static unsigned char _han_font[11520];

/* ---- call compiled glyph: EDI=dst, ESI=code, EAX=color4 ---- */

void _draw_cfont(void *dst, const void *code, unsigned long color4);
#pragma aux _draw_cfont = \
    "call esi" \
    parm [edi] [esi] [eax] \
    modify [edi esi eax];

/* ---- file helpers ---- */

static int _load_file(const char *name, void *buf, unsigned long size) {
    FILE *f = fopen(name, "rb");
    if (!f) return -1;
    fread(buf, 1, size, f);
    fclose(f);
    return 0;
}

static unsigned char *_load_alloc(const char *name, unsigned long *out_size) {
    FILE *f;
    long sz;
    unsigned char *buf;
    f = fopen(name, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    buf = (unsigned char *)malloc(sz);
    if (!buf) { fclose(f); return NULL; }
    fread(buf, 1, sz, f);
    fclose(f);
    *out_size = (unsigned long)sz;
    return buf;
}

/* ---- init / close ---- */

void font_init(void) {
    unsigned long magic;

    _bin_data = _load_alloc("FONT.BIN", &_bin_size);
    if (_bin_data && _bin_size >= 8) {
        memcpy(&magic, _bin_data, 4);
        if (magic == 0x544E4643UL) {  /* "CFNT" */
            _num_glyphs = _bin_data[4] | (_bin_data[5] << 8);
            _bin_index = _bin_data + 8;
            _bin_code  = _bin_index + _num_glyphs * 10;
        }
    }

    _load_file("ENG.FNT", _eng_font, 4096);
    _load_file("HAN.FNT", _han_font, 11520);
}

void font_close(void) {
    if (_bin_data) { free(_bin_data); _bin_data = NULL; }
    _num_glyphs = 0;
}

/* ---- binary search ---- */

static unsigned char *_find_glyph(unsigned long cp) {
    int lo = 0, hi = _num_glyphs - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        unsigned char *entry = _bin_index + mid * 10;
        unsigned long ecp;
        memcpy(&ecp, entry, 4);
        if (ecp == cp) {
            unsigned long off;
            memcpy(&off, entry + 4, 4);
            return _bin_code + off;
        }
        if (ecp < cp) lo = mid + 1;
        else hi = mid - 1;
    }
    return NULL;
}

/* ---- UTF-8 decoder ---- */

static unsigned long _utf8_decode(const char **pp) {
    unsigned char c = (unsigned char)*(*pp)++;
    unsigned long cp;
    if (c < 0x80) return c;
    if ((c & 0xE0) == 0xC0) {
        cp = (unsigned long)(c & 0x1F) << 6;
        cp |= (unsigned long)(*(*pp)++ & 0x3F);
        return cp;
    }
    if ((c & 0xF0) == 0xE0) {
        cp = (unsigned long)(c & 0x0F) << 12;
        cp |= (unsigned long)(*(*pp)++ & 0x3F) << 6;
        cp |= (unsigned long)(*(*pp)++ & 0x3F);
        return cp;
    }
    if ((c & 0xF8) == 0xF0) (*pp) += 3;
    return '?';
}

/* ---- fallback renderer ---- */

static const unsigned char _cho_bul[2][21] = {
    { 0,0,0,0,0,0,0,0, 1,3,3,3,1, 2,4,4,4,2, 1,3,0 },
    { 5,5,5,5,5,5,5,5, 6,7,7,7,6, 6,7,7,7,6, 6,7,5 }
};
static const unsigned char _jong_bul[21] = {
    0,2,0,2,1,2,1,2, 3,0,2,1,3, 3,1,2,1,3, 3,1,1
};

static void _fb_eng(int x, int y, int ch, unsigned char color) {
    unsigned char *buf = gfx_buffer();
    int row, bit;
    unsigned char byte;
    for (row = 0; row < 16; row++) {
        byte = _eng_font[ch * 16 + row];
        if (!byte) continue;
        for (bit = 0; bit < 8; bit++) {
            if (byte & (0x80 >> bit))
                buf[(y + row) * GFX_W + x + bit] = color;
        }
    }
}

static void _fb_han(int x, int y, unsigned long cp, unsigned char color) {
    unsigned char *buf = gfx_buffer();
    unsigned long idx;
    int cho, jung, jong, has_jong;
    int cb, jb, kb, ci, ji, ki;
    int row, bit;
    unsigned char hi, lo;

    idx = cp - 0xAC00UL;
    cho  = (int)(idx / 588);
    jung = (int)((idx % 588) / 28);
    jong = (int)(idx % 28);
    has_jong = (jong > 0) ? 1 : 0;

    cb = _cho_bul[has_jong][jung];
    jb = ((cho == 0 || cho == 16) ? 0 : 1) + (has_jong ? 2 : 0);
    kb = has_jong ? _jong_bul[jung] : 0;

    ci = cb * 20 + (cho + 1);
    ji = 160 + jb * 22 + (jung + 1);
    ki = 248 + kb * 28 + jong;

    for (row = 0; row < 16; row++) {
        hi = _han_font[ci*32 + row*2]
           | _han_font[ji*32 + row*2]
           | _han_font[ki*32 + row*2];
        lo = _han_font[ci*32 + row*2 + 1]
           | _han_font[ji*32 + row*2 + 1]
           | _han_font[ki*32 + row*2 + 1];
        if (!hi && !lo) continue;
        for (bit = 0; bit < 8; bit++) {
            if (hi & (0x80 >> bit))
                buf[(y + row) * GFX_W + x + bit] = color;
            if (lo & (0x80 >> bit))
                buf[(y + row) * GFX_W + x + 8 + bit] = color;
        }
    }
}

/* ---- public API ---- */

void font_puts(int x, int y, const char *str, unsigned char color) {
    unsigned long cp, color4;
    unsigned char *code;

    color4 = (unsigned long)color;
    color4 |= color4 << 8;
    color4 |= color4 << 16;

    while (*str) {
        cp = _utf8_decode(&str);
        if (cp == 0) break;

        code = _find_glyph(cp);
        if (code) {
            _draw_cfont(gfx_buffer() + y * GFX_W + x, code, color4);
        } else if (cp >= 0xAC00UL && cp <= 0xD7A3UL) {
            _fb_han(x, y, cp, color);
        } else if (cp < 256) {
            _fb_eng(x, y, (int)cp, color);
        }

        if (cp >= 0xAC00UL && cp <= 0xD7A3UL)
            x += FONT_HAN_W;
        else
            x += FONT_ENG_W;
    }
}

int font_text_width(const char *str) {
    int w = 0;
    unsigned long cp;
    while (*str) {
        cp = _utf8_decode(&str);
        if (cp == 0) break;
        w += (cp >= 0xAC00UL && cp <= 0xD7A3UL) ? FONT_HAN_W : FONT_ENG_W;
    }
    return w;
}
```

- [ ] **Step 2: Commit**

```bash
git add SRC/FONT.CPP
git commit -m "Add FONT.CPP with compiled glyph renderer and software fallback"
```

---

### Task 4: Update `convert.sh` and `.gitignore`

**Files:**
- Modify: `convert.sh`
- Modify: `.gitignore`

- [ ] **Step 1: Add mkfont.ts call at end of convert.sh**

After the final `echo "Done:..."` line, append:

```bash

# 6. Compiled font
echo "Building compiled font..."
bun tools/mkfont.ts
```

- [ ] **Step 2: Add generated font files to .gitignore**

Append to `.gitignore`:

```
SRC/FONT.BIN
SRC/ENG.FNT
SRC/HAN.FNT
```

- [ ] **Step 3: Commit**

```bash
git add convert.sh .gitignore
git commit -m "Integrate mkfont.ts into asset pipeline"
```

---

### Task 5: Add test string to GAME.CPP, build and verify

**Files:**
- Modify: `SRC/GAME.CPP`

- [ ] **Step 1: Add font include**

Add `#include "FONT.H"` with the other includes at the top of `GAME.CPP`.

- [ ] **Step 2: Add font_init() call**

In `main()`, after the sound init block (after `snd_play()` closing brace), add:

```c
font_init();
```

- [ ] **Step 3: Add font_close() call**

In the shutdown section, add `font_close();` as the first cleanup call (before `snd_close()`), to maintain LIFO ordering.

- [ ] **Step 4: Add test text rendering**

In the render section, after `draw_ship();` and before `gfx_vsync();`, add:

```c
font_puts(4, 4, "Hello, World!", 15);
font_puts(4, 24, "한글 테스트", 15);
```

- [ ] **Step 5: Build**

Run: `./build.sh`

Check BUILD.LOG: all `.CPP` files compile with no errors, linker produces GAME.EXE.

- [ ] **Step 6: Run and visually verify**

Run: `./run.sh`

Expected: white English and Korean text rendered at top-left of screen over the game.

- [ ] **Step 7: Commit**

```bash
git add SRC/GAME.CPP
git commit -m "Add font rendering test: English and Korean text display"
```
