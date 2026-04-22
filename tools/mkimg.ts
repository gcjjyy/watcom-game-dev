#!/usr/bin/env bun
/**
 * mkimg.ts - Convert a PNG into a raw indexed .IMG file
 *
 * Two modes:
 *   --fs       Full-screen 320x200 raw indexed (no header, exactly 64000 bytes)
 *   default    Variable-size with 4-byte header { uint16 w; uint16 h; } + pixels
 *
 * Usage:
 *   bun tools/mkimg.ts --fs in.png SRC/OUT.IMG
 *   bun tools/mkimg.ts in.png SRC/OUT.IMG
 */

import { readFileSync, writeFileSync, existsSync } from "fs";
import { parseArgs } from "util";

/* ---- PNG reader (lifted from tools/mksprite.ts) ---- */
function paethPredictor(a: number, b: number, c: number): number {
  const p = a + b - c;
  const pa = Math.abs(p - a);
  const pb = Math.abs(p - b);
  const pc = Math.abs(p - c);
  if (pa <= pb && pa <= pc) return a;
  if (pb <= pc) return b;
  return c;
}

function unfilterRow(filter: number, row: Uint8Array, prior: Uint8Array | null, bpp: number): void {
  switch (filter) {
    case 0: break;
    case 1:
      for (let i = bpp; i < row.length; i++) row[i] = (row[i] + row[i - bpp]) & 0xFF;
      break;
    case 2:
      if (prior) for (let i = 0; i < row.length; i++) row[i] = (row[i] + prior[i]) & 0xFF;
      break;
    case 3:
      for (let i = 0; i < row.length; i++) {
        const a = i >= bpp ? row[i - bpp] : 0;
        const b = prior ? prior[i] : 0;
        row[i] = (row[i] + ((a + b) >> 1)) & 0xFF;
      }
      break;
    case 4:
      for (let i = 0; i < row.length; i++) {
        const a = i >= bpp ? row[i - bpp] : 0;
        const b = prior ? prior[i] : 0;
        const c = i >= bpp && prior ? prior[i - bpp] : 0;
        row[i] = (row[i] + paethPredictor(a, b, c)) & 0xFF;
      }
      break;
  }
}

function readPNG(path: string): { width: number; height: number; rgba: Uint8Array } {
  const buf = readFileSync(path);
  if (buf[0] !== 0x89 || buf[1] !== 0x50) throw new Error("Not a PNG");

  let offset = 8;
  let width = 0, height = 0, bitDepth = 0, colorType = 0;
  const idatChunks: Buffer[] = [];
  let plte: Uint8Array | null = null;
  let trns: Uint8Array | null = null;

  while (offset < buf.length) {
    const length = buf.readUInt32BE(offset);
    const type = buf.toString("ascii", offset + 4, offset + 8);
    const data = buf.subarray(offset + 8, offset + 8 + length);
    if (type === "IHDR") {
      width = data.readUInt32BE(0); height = data.readUInt32BE(4);
      bitDepth = data[8]; colorType = data[9];
    } else if (type === "PLTE") plte = new Uint8Array(data);
    else if (type === "tRNS") trns = new Uint8Array(data);
    else if (type === "IDAT") idatChunks.push(Buffer.from(data));
    else if (type === "IEND") break;
    offset += 12 + length;
  }

  if (bitDepth !== 8) throw new Error(`Only 8-bit PNG (got ${bitDepth})`);

  const bppMap: Record<number, number> = { 2: 3, 3: 1, 6: 4 };
  const bpp = bppMap[colorType];
  if (!bpp) throw new Error(`Unsupported color type ${colorType}`);

  const compressed = Buffer.concat(idatChunks);
  const raw = Buffer.from(Bun.inflateSync(compressed.subarray(2)));
  const rowBytes = width * bpp;
  const pixels = new Uint8Array(height * rowBytes);
  let srcPos = 0;
  for (let y = 0; y < height; y++) {
    const filter = raw[srcPos++];
    const row = pixels.subarray(y * rowBytes, (y + 1) * rowBytes);
    const prior = y > 0 ? pixels.subarray((y - 1) * rowBytes, y * rowBytes) : null;
    for (let i = 0; i < rowBytes; i++) row[i] = raw[srcPos++];
    unfilterRow(filter, row, prior, bpp);
  }

  const rgba = new Uint8Array(width * height * 4);
  if (colorType === 6) {
    rgba.set(pixels);
  } else if (colorType === 2) {
    for (let i = 0; i < width * height; i++) {
      rgba[i * 4] = pixels[i * 3];
      rgba[i * 4 + 1] = pixels[i * 3 + 1];
      rgba[i * 4 + 2] = pixels[i * 3 + 2];
      rgba[i * 4 + 3] = 255;
    }
  } else if (colorType === 3) {
    if (!plte) throw new Error("Indexed PNG missing PLTE");
    for (let i = 0; i < width * height; i++) {
      const idx = pixels[i];
      rgba[i * 4] = plte[idx * 3];
      rgba[i * 4 + 1] = plte[idx * 3 + 1];
      rgba[i * 4 + 2] = plte[idx * 3 + 2];
      rgba[i * 4 + 3] = trns && idx < trns.length ? trns[idx] : 255;
    }
  }
  return { width, height, rgba };
}

/* ---- Palette match ---- */
function loadPalette(): [number, number, number][] {
  if (!existsSync("tools/palette.json"))
    throw new Error("tools/palette.json not found. Run mkpalette first.");
  const data = JSON.parse(readFileSync("tools/palette.json", "utf-8"));
  return data.colors;
}

function nearestColor(r: number, g: number, b: number, palette: [number, number, number][]): number {
  let bestIdx = 1, bestDist = Infinity;
  for (let i = 1; i < palette.length; i++) {
    const dr = r - palette[i][0];
    const dg = g - palette[i][1];
    const db = b - palette[i][2];
    const d = dr * dr + dg * dg + db * db;
    if (d < bestDist) { bestDist = d; bestIdx = i; }
  }
  return bestIdx;
}

/* ---- Main ---- */
const { values, positionals } = parseArgs({
  args: Bun.argv.slice(2),
  options: { fs: { type: "boolean", default: false } },
  allowPositionals: true,
});

if (positionals.length < 2) {
  console.error("Usage: bun tools/mkimg.ts [--fs] in.png OUT.IMG");
  process.exit(1);
}

const [inPath, outPath] = positionals;
const palette = loadPalette();
const { width, height, rgba } = readPNG(inPath);

if (values.fs) {
  if (width !== 320 || height !== 200) {
    console.error(`mkimg --fs requires 320x200 input, got ${width}x${height}`);
    process.exit(1);
  }
  const out = Buffer.alloc(64000);
  for (let i = 0; i < 320 * 200; i++) {
    const a = rgba[i * 4 + 3];
    if (a < 128) out[i] = 0;
    else out[i] = nearestColor(rgba[i * 4], rgba[i * 4 + 1], rgba[i * 4 + 2], palette);
  }
  writeFileSync(outPath, out);
  console.error(`/* ${outPath}: 320x200, 64000 bytes (--fs) */`);
} else {
  const total = 4 + width * height;
  const out = Buffer.alloc(total);
  out.writeUInt16LE(width, 0);
  out.writeUInt16LE(height, 2);
  for (let i = 0; i < width * height; i++) {
    const a = rgba[i * 4 + 3];
    if (a < 128) out[4 + i] = 0;
    else out[4 + i] = nearestColor(rgba[i * 4], rgba[i * 4 + 1], rgba[i * 4 + 2], palette);
  }
  writeFileSync(outPath, out);
  console.error(`/* ${outPath}: ${width}x${height}, ${total} bytes */`);
}
