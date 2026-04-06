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
if (engFont.length !== 4096)
  throw new Error(`English font must be 4096 bytes, got ${engFont.length}`);

const hanFont = readFileSync(values.han!);
if (hanFont.length !== 11520)
  throw new Error(`Korean font must be 11520 bytes, got ${hanFont.length}`);

// ---- Scan SRC/*.CPP for characters used in string literals ----

function scanSourceChars(): Set<number> {
  const chars = new Set<number>();
  const files = readdirSync("SRC").filter(
    (f) => f.endsWith(".CPP") || f.endsWith(".H"),
  );

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
        if (cp >= 0x20 && cp <= 0x7e) chars.add(cp);
        else if (cp >= 0xac00 && cp <= 0xd7a3) chars.add(cp);
        i += cp > 0xffff ? 2 : 1;
      }
    }
  }

  // Always include ASCII printable range
  for (let c = 0x20; c <= 0x7e; c++) chars.add(c);
  return chars;
}

const usedChars = scanSourceChars();
console.error(
  `Found ${usedChars.size} unique characters (${[...usedChars].filter((c) => c >= 0xac00).length} Korean)`,
);

// ---- Korean decomposition and bul selection ----

function decomposeHangul(cp: number): {
  cho: number;
  jung: number;
  jong: number;
} {
  const idx = cp - 0xac00;
  return {
    cho: Math.floor(idx / 588),
    jung: Math.floor((idx % 588) / 28),
    jong: idx % 28,
  };
}

const CHO_BUL: number[][] = [
  [0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 3, 1, 2, 4, 4, 4, 2, 1, 3, 0],
  [5, 5, 5, 5, 5, 5, 5, 5, 6, 7, 7, 7, 6, 6, 7, 7, 7, 6, 6, 7, 5],
];
const JONG_BUL = [
  0, 2, 0, 2, 1, 2, 1, 2, 3, 0, 2, 1, 3, 3, 1, 2, 1, 3, 3, 1, 1,
];

function getChoBul(jung: number, hasJong: boolean): number {
  return CHO_BUL[hasJong ? 1 : 0][jung];
}
function getJungBul(cho: number, hasJong: boolean): number {
  return (cho === 0 || cho === 16 ? 0 : 1) + (hasJong ? 2 : 0);
}
function getJongBul(jung: number): number {
  return JONG_BUL[jung];
}

// ---- Korean glyph composition ----

function composeHangulBitmap(cp: number): Uint8Array {
  const { cho, jung, jong } = decomposeHangul(cp);
  const hasJong = jong > 0;
  const choBul = getChoBul(jung, hasJong);
  const jungBul = getJungBul(cho, hasJong);
  const jongBul = hasJong ? getJongBul(jung) : 0;

  const choIdx = choBul * 20 + (cho + 1);
  const jungIdx = 160 + jungBul * 22 + (jung + 1);
  const jongIdx = 248 + jongBul * 28 + jong;

  const result = new Uint8Array(32);
  for (let i = 0; i < 32; i++) {
    result[i] =
      hanFont[choIdx * 32 + i] |
      hanFont[jungIdx * 32 + i] |
      hanFont[jongIdx * 32 + i];
  }
  return result;
}

// ---- x86 compiled glyph code gen ----
// MOV [EDI+disp32], EAX  -> 89 87 xx xx xx xx       (4 pixels)
// MOV [EDI+disp32], AX   -> 66 89 87 xx xx xx xx    (2 pixels)
// MOV [EDI+disp32], AL   -> 88 87 xx xx xx xx       (1 pixel)
// RET                    -> C3

function compileFontGlyph(
  w: number,
  h: number,
  bitmap1bit: Uint8Array,
): Uint8Array {
  const code: number[] = [];
  const stride = 320;

  const pushLE32 = (v: number) => {
    code.push(v & 0xff, (v >> 8) & 0xff, (v >> 16) & 0xff, (v >> 24) & 0xff);
  };

  const bytesPerRow = Math.ceil(w / 8);

  for (let y = 0; y < h; y++) {
    let x = 0;
    while (x < w) {
      const byteIdx = y * bytesPerRow + Math.floor(x / 8);
      const bitIdx = 7 - (x % 8);
      if (!((bitmap1bit[byteIdx] >> bitIdx) & 1)) {
        x++;
        continue;
      }

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
          code.push(0x89, 0x87);
          pushLE32(off);
          pos += 4;
        } else if (remaining >= 2) {
          code.push(0x66, 0x89, 0x87);
          pushLE32(off);
          pos += 2;
        } else {
          code.push(0x88, 0x87);
          pushLE32(off);
          pos += 1;
        }
      }
    }
  }

  code.push(0xc3); // RET
  return new Uint8Array(code);
}

// ---- Build all glyphs ----

interface GlyphEntry {
  codepoint: number;
  code: Uint8Array;
}
const glyphs: GlyphEntry[] = [];

for (const cp of [...usedChars].sort((a, b) => a - b)) {
  let code: Uint8Array;
  if (cp >= 0x20 && cp <= 0x7e) {
    const bitmap = engFont.subarray(cp * 16, cp * 16 + 16);
    code = compileFontGlyph(8, 16, bitmap);
  } else if (cp >= 0xac00 && cp <= 0xd7a3) {
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

bin.write("CFNT", 0, 4, "ascii");
boff += 4;
bin.writeUInt16LE(glyphs.length, boff);
boff += 2;
bin[boff++] = 8;
bin[boff++] = 16;

let codeOff = 0;
for (const g of glyphs) {
  bin.writeUInt32LE(g.codepoint, boff);
  boff += 4;
  bin.writeUInt32LE(codeOff, boff);
  boff += 4;
  bin.writeUInt16LE(g.code.length, boff);
  boff += 2;
  codeOff += g.code.length;
}

for (const g of glyphs) {
  Buffer.from(g.code).copy(bin, boff);
  boff += g.code.length;
}

writeFileSync("SRC/FONT.BIN", bin);
console.error(
  `Compiled ${glyphs.length} glyphs -> SRC/FONT.BIN (${bin.length} bytes)`,
);

// ---- Copy raw fonts for fallback ----
copyFileSync(values.eng!, "SRC/ENG.FNT");
copyFileSync(values.han!, "SRC/HAN.FNT");
console.error(
  `Copied ${values.eng} -> SRC/ENG.FNT, ${values.han} -> SRC/HAN.FNT`,
);
