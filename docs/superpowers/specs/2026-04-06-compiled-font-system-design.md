# Compiled Font System Design

## Overview

Korean+English bitmap font rendering for DOS (Watcom C++, VGA Mode 13h) using precompiled x86 glyph code with software fallback for uncached characters.

## Architecture

```
Build time (Bun):                     Runtime (DOS):
                                      
  SRC/*.CPP ──scan──┐                  FONT.BIN ──load──> index + code
  fonts/ENG.FNT ────┤                  ENG.FNT  ──load──> raw eng bitmap
  fonts/HAN.FNT ────┤                  HAN.FNT  ──load──> raw han bitmap
                    v                  
              mkfont.ts                font_puts("한글 test", color)
                    │                      │
                    v                      v
              FONT.BIN ───────────>  binary search index
              ENG.FNT  ──copy──>       ├─ HIT:  CALL compiled code (fast)
              HAN.FNT  ──copy──>       └─ MISS: software fallback  (slow)
```

## mkfont.ts

### Input
- `fonts/ENG_*.FNT` (4096B, 256 chars, 8x16 1-bit)
- `fonts/HAN_*.FNT` (11520B, 360 components, 16x16 1-bit)
- All `SRC/*.CPP` files (scanned for string literals)
- Config: which ENG/HAN pair to use (default: ENG_HGDIARY + HAN_GOTHIC)

### Character Extraction
1. Read each `.CPP` file as UTF-8
2. Extract bytes inside string literal quotes (handle escape sequences)
3. Decode UTF-8 sequences to Unicode code points
4. Collect unique set: ASCII 0x20-0x7E + Korean U+AC00-U+D7A3

### Korean Glyph Composition
For each Korean code point:
1. Decompose: `cho = (cp-0xAC00)/588`, `jung = ((cp-0xAC00)%588)/28`, `jong = (cp-0xAC00)%28`
2. Select bul (set variant) using tables below
3. Read 32-byte bitmaps from font data: cho, jung, jong components
4. OR the three 16x16 bitmaps together

### Bul Selection Tables (from bakyeono.net reference, verified against font data)

**Choseong 8 bul** — selected by (jungseong, has_jongseong):

Without jongseong:
- Bul 0: ㅏ(0) ㅐ(1) ㅑ(2) ㅒ(3) ㅓ(4) ㅔ(5) ㅕ(6) ㅖ(7) ㅣ(20)
- Bul 1: ㅗ(8) ㅛ(12) ㅡ(18)
- Bul 2: ㅜ(13) ㅠ(17)
- Bul 3: ㅘ(9) ㅙ(10) ㅚ(11) ㅢ(19)
- Bul 4: ㅝ(14) ㅞ(15) ㅟ(16)

With jongseong:
- Bul 5: ㅏ(0) ㅐ(1) ㅑ(2) ㅒ(3) ㅓ(4) ㅔ(5) ㅕ(6) ㅖ(7) ㅣ(20)
- Bul 6: ㅗ(8) ㅛ(12) ㅜ(13) ㅠ(17) ㅡ(18)
- Bul 7: ㅘ(9) ㅙ(10) ㅚ(11) ㅢ(19) ㅝ(14) ㅞ(15) ㅟ(16)

As lookup table:
```
cho_bul[2][21] = {
  { 0,0,0,0,0,0,0,0, 1,3,3,3,1, 2,4,4,4,2, 1,3,0 },  // no jong
  { 5,5,5,5,5,5,5,5, 6,7,7,7,6, 6,7,7,7,6, 6,7,5 },  // has jong
};
```

**Jungseong 4 bul** — selected by (choseong, has_jongseong):
- Narrow cho (ㄱ=0, ㅋ=16): base = 0
- Wide cho (all others): base = 1
- Has jongseong: + 2

```
jung_bul = (cho == 0 || cho == 16) ? 0 : 1) + (jong > 0 ? 2 : 0)
```

**Jongseong 4 bul** — selected by jungseong:
- Bul 0: ㅏ(0) ㅑ(2) ㅘ(9)
- Bul 1: ㅓ(4) ㅕ(6) ㅚ(11) ㅝ(14) ㅟ(16) ㅢ(19) ㅣ(20)
- Bul 2: ㅐ(1) ㅒ(3) ㅔ(5) ㅖ(7) ㅙ(10) ㅞ(15)
- Bul 3: ㅗ(8) ㅛ(12) ㅜ(13) ㅠ(17) ㅡ(18)

```
jong_bul[21] = { 0,2,0,2,1,2,1,2, 3,0,2,1,3, 3,1,2,1,3, 3,1,1 };
```

### Font Data Layout (11520B file)

```
Offset 0:    Choseong  — 8 bul × 20 jamo (fill + 19) × 32 bytes = 5120B
Offset 5120: Jungseong — 4 bul × 22 jamo (fill + 21) × 32 bytes = 2816B
Offset 7936: Jongseong — 4 bul × 28 jamo (fill + 27) × 32 bytes = 3584B
```

Component index calculation:
```
cho_idx  = cho_bul[has_jong][jung] * 20 + (cho + 1)
jung_idx = 160 + jung_bul * 22 + (jung + 1)
jong_idx = 248 + jong_bul * 28 + jong       // jong=0 → fill (blank)
```

### x86 Code Generation (color-via-register variant)

Unlike sprite compiled code which bakes color as immediate values, font code receives color in EAX register:

```asm
; Caller sets: EDI = buffer + y*320 + x
;              EAX = color | (color<<8) | (color<<16) | (color<<24)
; 4 contiguous pixels: MOV [EDI+offset], EAX    → 89 87 xx xx xx xx
; 2 contiguous pixels: MOV [EDI+offset], AX     → 66 89 87 xx xx xx xx
; 1 pixel:             MOV [EDI+offset], AL     → 88 87 xx xx xx xx
; End:                 RET                       → C3
```

Stride is 320 (VGA Mode 13h width).

### Output: FONT.BIN Format

```
Header (8 bytes):
  u8[4]  magic = "CFNT"
  u16    num_glyphs
  u8     eng_width  = 8
  u8     han_width  = 16

Index (num_glyphs × 10 bytes, sorted by codepoint):
  u32    codepoint
  u32    code_offset    (from start of code section)
  u16    code_size

Code section:
  [compiled x86 bytes for all glyphs, concatenated]
```

## FONT.CPP / FONT.H (DOS side)

### API

```c
void font_init(void);
void font_close(void);
void font_puts(int x, int y, const char *str, unsigned char color);
int  font_text_width(const char *str);
```

### Internals

**font_init()**:
1. Load `FONT.BIN` → malloc, parse header + index
2. Load `ENG.FNT` (4096B) → raw bitmap for fallback
3. Load `HAN.FNT` (11520B) → raw bitmap for fallback
4. Build nibble table for fallback renderer (default color white)

**font_puts()**:
1. Parse UTF-8 byte by byte
2. For each code point: binary search FONT.BIN index
3. HIT → call compiled code via `_draw_cfont(buf + y*320 + x, code_ptr, color4)`
4. MISS → fallback renderer
5. Advance x by 8 (English) or 16 (Korean)

**Fallback renderer**:
- English: read bitmap byte from ENG.FNT, expand via nibble table
- Korean: decompose codepoint → bul selection → OR three 32-byte components → nibble table

**_draw_cfont pragma aux**:
```c
void _draw_cfont(void *dst, const void *code, unsigned long color4);
#pragma aux _draw_cfont = \
    "call esi" \
    parm [edi] [esi] [eax] \
    modify [edi esi eax];
```

### UTF-8 Decoder

```c
static unsigned long _utf8_decode(const char **pp) {
    unsigned char c = *(*pp)++;
    if (c < 0x80) return c;
    if ((c & 0xE0) == 0xC0) {
        unsigned long cp = (c & 0x1F) << 6;
        return cp | (*(*pp)++ & 0x3F);
    }
    if ((c & 0xF0) == 0xE0) {
        unsigned long cp = (c & 0x0F) << 12;
        cp |= (*(*pp)++ & 0x3F) << 6;
        return cp | (*(*pp)++ & 0x3F);
    }
    return '?';
}
```

## Build Pipeline Integration

### convert.sh addition
```bash
# After sprite conversion
echo "Building compiled font..."
bun tools/mkfont.ts
```

### mkfont.ts copies to SRC/
- `SRC/FONT.BIN` — compiled glyphs
- `SRC/ENG.FNT` — English font for fallback
- `SRC/HAN.FNT` — Korean font for fallback

### Font selection
`mkfont.ts` accepts optional args or reads from a config:
```bash
bun tools/mkfont.ts --eng fonts/ENG_HGDIARY.FNT --han fonts/HAN_GOTHIC.FNT
```
Default: `ENG_HGDIARY.FNT` + `HAN_GOTHIC.FNT`

## Files to Create/Modify

| File | Action |
|------|--------|
| `tools/mkfont.ts` | Create — font compiler |
| `SRC/FONT.H` | Create — API header |
| `SRC/FONT.CPP` | Create — implementation |
| `convert.sh` | Modify — add mkfont.ts call |
| `fonts/` | Created — font originals (multiple choices) |
