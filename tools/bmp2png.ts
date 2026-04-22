#!/usr/bin/env bun
/**
 * bmp2png.ts — convert 8-bit indexed BMPs produced by SRC/SCRNCAP.CPP
 * into PNGs under the given output directory.
 *
 * Usage:
 *   bun tools/bmp2png.ts IN1.BMP IN2.BMP ... OUT_DIR/
 *
 * Last argument must be a directory. All prior args are BMP files.
 */

import { readFileSync, writeFileSync } from "fs";
import { deflateSync } from "zlib";
import { basename, join, resolve } from "path";

function readBmp(path: string): { w: number; h: number; rgba: Uint8Array } {
  const buf = readFileSync(path);
  if (buf[0] !== 0x42 || buf[1] !== 0x4D)
    throw new Error(`${path}: not a BMP file`);
  const pixOff = buf.readUInt32LE(10);
  const w = buf.readInt32LE(18);
  const hRaw = buf.readInt32LE(22);
  const h = Math.abs(hRaw);
  const bpp = buf.readUInt16LE(28);
  if (bpp !== 8)
    throw new Error(`${path}: expected 8-bit BMP, got ${bpp}`);
  const compression = buf.readUInt32LE(30);
  if (compression !== 0)
    throw new Error(`${path}: compression ${compression} not supported`);
  const palOff = 14 + buf.readUInt32LE(14);
  const palEntries = buf.readUInt32LE(46) || 256;
  const pal: [number, number, number][] = [];
  for (let i = 0; i < palEntries; i++) {
    const b = buf[palOff + i * 4 + 0];
    const g = buf[palOff + i * 4 + 1];
    const r = buf[palOff + i * 4 + 2];
    pal.push([r, g, b]);
  }
  const rowBytes = (w + 3) & ~3; // 4-byte aligned
  const rgba = new Uint8Array(w * h * 4);
  const bottomUp = hRaw > 0;
  for (let y = 0; y < h; y++) {
    const srcY = bottomUp ? h - 1 - y : y;
    const srcRow = pixOff + srcY * rowBytes;
    for (let x = 0; x < w; x++) {
      const idx = buf[srcRow + x];
      const [r, g, b] = pal[idx] || [0, 0, 0];
      const di = (y * w + x) * 4;
      rgba[di + 0] = r;
      rgba[di + 1] = g;
      rgba[di + 2] = b;
      rgba[di + 3] = 255;
    }
  }
  return { w, h, rgba };
}

// Minimal PNG writer (RGBA, no filtering, single IDAT)
function writePng(path: string, w: number, h: number, rgba: Uint8Array) {
  const sig = Buffer.from([0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a]);

  function crc32(data: Uint8Array): number {
    let c = 0xffffffff;
    for (let i = 0; i < data.length; i++) {
      c = c ^ data[i];
      for (let k = 0; k < 8; k++)
        c = c & 1 ? 0xedb88320 ^ (c >>> 1) : c >>> 1;
    }
    return (c ^ 0xffffffff) >>> 0;
  }

  function chunk(type: string, data: Buffer): Buffer {
    const len = Buffer.alloc(4);
    len.writeUInt32BE(data.length, 0);
    const typeBuf = Buffer.from(type, "ascii");
    const crcBuf = Buffer.alloc(4);
    const crcData = Buffer.concat([typeBuf, data]);
    crcBuf.writeUInt32BE(crc32(new Uint8Array(crcData)), 0);
    return Buffer.concat([len, typeBuf, data, crcBuf]);
  }

  // IHDR
  const ihdr = Buffer.alloc(13);
  ihdr.writeUInt32BE(w, 0);
  ihdr.writeUInt32BE(h, 4);
  ihdr[8] = 8;  // bit depth
  ihdr[9] = 6;  // color type RGBA
  ihdr[10] = 0;
  ihdr[11] = 0;
  ihdr[12] = 0;

  // Raw with filter byte 0 per row
  const rawBytes = new Uint8Array(h * (1 + w * 4));
  for (let y = 0; y < h; y++) {
    rawBytes[y * (1 + w * 4)] = 0;
    rawBytes.set(rgba.subarray(y * w * 4, (y + 1) * w * 4), y * (1 + w * 4) + 1);
  }
  const idat = deflateSync(rawBytes);

  const png = Buffer.concat([
    sig,
    chunk("IHDR", ihdr),
    chunk("IDAT", idat),
    chunk("IEND", Buffer.alloc(0)),
  ]);
  writeFileSync(path, png);
}

// ---- main ----

const args = Bun.argv.slice(2);
if (args.length < 2) {
  console.error("Usage: bun tools/bmp2png.ts IN.BMP [IN2.BMP...] OUT_DIR/");
  process.exit(1);
}

const outDir = resolve(args[args.length - 1]);
const inputs = args.slice(0, -1);

for (const inPath of inputs) {
  const { w, h, rgba } = readBmp(inPath);
  const name = basename(inPath).replace(/\.bmp$/i, ".png").toLowerCase();
  const outPath = join(outDir, name);
  writePng(outPath, w, h, rgba);
  console.log(`  ${inPath} -> ${outPath} (${w}x${h})`);
}
