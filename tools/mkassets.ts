#!/usr/bin/env bun
/**
 * mkassets.ts - Generate placeholder RPG tileset and character sprite PNGs
 *
 * Creates simple but functional 16x16 pixel art for:
 * - tileset.png: 8-tile tileset (grass, path, water, wall, building, tree, door, trigger)
 * - hero.png: 16x16 character (1x4 grid: down, left, right, up)
 * - npc.png: 16x16 NPC character (1x4 grid)
 * - enemy.png: 16x16 enemy character (1x4 grid)
 */

import { writeFileSync } from "fs";
import { deflateSync } from "zlib";

function writePNG(path: string, width: number, height: number, rgba: Uint8Array): void {
  const signature = Buffer.from([0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A]);

  function crc32(buf: Buffer): number {
    let c = 0xFFFFFFFF;
    for (let i = 0; i < buf.length; i++) {
      c ^= buf[i];
      for (let j = 0; j < 8; j++) c = (c >>> 1) ^ (c & 1 ? 0xEDB88320 : 0);
    }
    return (c ^ 0xFFFFFFFF) >>> 0;
  }

  function chunk(type: string, data: Buffer): Buffer {
    const len = Buffer.alloc(4);
    len.writeUInt32BE(data.length);
    const typeAndData = Buffer.concat([Buffer.from(type), data]);
    const crc = Buffer.alloc(4);
    crc.writeUInt32BE(crc32(typeAndData));
    return Buffer.concat([len, typeAndData, crc]);
  }

  const ihdr = Buffer.alloc(13);
  ihdr.writeUInt32BE(width, 0);
  ihdr.writeUInt32BE(height, 4);
  ihdr[8] = 8; ihdr[9] = 6; // 8-bit RGBA

  const rawRows: Buffer[] = [];
  for (let y = 0; y < height; y++) {
    const row = Buffer.alloc(1 + width * 4);
    row[0] = 0; // no filter
    rgba.subarray(y * width * 4, (y + 1) * width * 4).forEach((v, i) => row[i + 1] = v);
    rawRows.push(row);
  }
  const rawData = Buffer.concat(rawRows);
  const compressed = deflateSync(rawData);

  const iend = Buffer.alloc(0);
  writeFileSync(path, Buffer.concat([
    signature,
    chunk("IHDR", ihdr),
    chunk("IDAT", compressed),
    chunk("IEND", iend),
  ]));
}

function setPixel(rgba: Uint8Array, w: number, x: number, y: number, r: number, g: number, b: number, a = 255) {
  const i = (y * w + x) * 4;
  rgba[i] = r; rgba[i + 1] = g; rgba[i + 2] = b; rgba[i + 3] = a;
}

function fillRect(rgba: Uint8Array, w: number, x0: number, y0: number, rw: number, rh: number, r: number, g: number, b: number, a = 255) {
  for (let y = y0; y < y0 + rh; y++)
    for (let x = x0; x < x0 + rw; x++)
      setPixel(rgba, w, x, y, r, g, b, a);
}

/* ---- Tileset: 8 tiles in a row (128x16) ---- */
function makeTileset(): { w: number; h: number; rgba: Uint8Array } {
  const w = 128, h = 16;
  const rgba = new Uint8Array(w * h * 4);

  // Tile 0: Grass (green with variations)
  for (let y = 0; y < 16; y++)
    for (let x = 0; x < 16; x++) {
      const v = ((x * 7 + y * 13) % 3);
      setPixel(rgba, w, x, y, 30 + v * 10, 120 + v * 15, 30 + v * 5);
    }

  // Tile 1: Path (tan/brown)
  for (let y = 0; y < 16; y++)
    for (let x = 16; x < 32; x++) {
      const v = ((x * 5 + y * 11) % 3);
      setPixel(rgba, w, x, y, 160 + v * 10, 130 + v * 8, 80 + v * 5);
    }

  // Tile 2: Water (blue with wave pattern)
  for (let y = 0; y < 16; y++)
    for (let x = 32; x < 48; x++) {
      const wave = Math.sin(((x - 32) + y * 0.5) * 0.8) * 20;
      setPixel(rgba, w, x, y, 20, 60 + Math.floor(wave), 180 + Math.floor(wave * 0.5));
    }

  // Tile 3: Wall (gray brick pattern)
  for (let y = 0; y < 16; y++)
    for (let x = 48; x < 64; x++) {
      const bx = (x - 48), by = y;
      const mortar = (by % 8 === 0) || ((bx + (by < 8 ? 0 : 4)) % 8 === 0);
      if (mortar) setPixel(rgba, w, x, y, 100, 100, 100);
      else setPixel(rgba, w, x, y, 150, 140, 130);
    }

  // Tile 4: Building/roof (dark brown)
  for (let y = 0; y < 16; y++)
    for (let x = 64; x < 80; x++) {
      const v = ((x - 64 + y) % 4 < 2) ? 0 : 15;
      setPixel(rgba, w, x, y, 100 + v, 60 + v, 30 + v);
    }

  // Tile 5: Tree (trunk + canopy)
  for (let y = 0; y < 16; y++)
    for (let x = 80; x < 96; x++) {
      const lx = x - 80;
      if (y >= 12 && lx >= 6 && lx <= 9) // trunk
        setPixel(rgba, w, x, y, 100, 70, 30);
      else if (y < 13) { // canopy
        const cx = 8, cy = 6;
        const d = Math.abs(lx - cx) + Math.abs(y - cy);
        if (d <= 7) setPixel(rgba, w, x, y, 20, 100 + (d * 5), 20);
        else { // grass background
          const v = ((x * 7 + y * 13) % 3);
          setPixel(rgba, w, x, y, 30 + v * 10, 120 + v * 15, 30 + v * 5);
        }
      } else { // grass below trunk
        const v = ((x * 7 + y * 13) % 3);
        setPixel(rgba, w, x, y, 30 + v * 10, 120 + v * 15, 30 + v * 5);
      }
    }

  // Tile 6: Door (wooden door on wall)
  for (let y = 0; y < 16; y++)
    for (let x = 96; x < 112; x++) {
      const lx = x - 96;
      if (lx >= 3 && lx <= 12 && y >= 2 && y <= 15) {
        setPixel(rgba, w, x, y, 140, 90, 40);
        if (lx === 10 && y === 9) setPixel(rgba, w, x, y, 200, 180, 50); // knob
      } else {
        setPixel(rgba, w, x, y, 150, 140, 130); // wall around
      }
    }

  // Tile 7: Trigger (sparkle/special tile on path)
  for (let y = 0; y < 16; y++)
    for (let x = 112; x < 128; x++) {
      const lx = x - 112;
      const v = ((lx * 5 + y * 11) % 3);
      setPixel(rgba, w, x, y, 160 + v * 10, 130 + v * 8, 80 + v * 5); // path base
      // diamond sparkle
      const d = Math.abs(lx - 8) + Math.abs(y - 8);
      if (d <= 3 && d > 1 && (d + lx + y) % 2 === 0)
        setPixel(rgba, w, x, y, 255, 255, 100);
    }

  return { w, h, rgba };
}

/* ---- Character sprite (16x64: 4 frames in column) ---- */
function makeCharacter(
  bodyR: number, bodyG: number, bodyB: number,
  headR: number, headG: number, headB: number,
): { w: number; h: number; rgba: Uint8Array } {
  const w = 16, h = 64;
  const rgba = new Uint8Array(w * h * 4); // all transparent by default

  // 4 frames: down, left, right, up
  for (let frame = 0; frame < 4; frame++) {
    const oy = frame * 16;

    // Head (skin color, centered)
    for (let y = 1; y < 7; y++)
      for (let x = 5; x < 11; x++)
        setPixel(rgba, w, x, oy + y, headR, headG, headB);

    // Eyes (frame 0=down: visible, frame 3=up: no eyes)
    if (frame === 0) { // down-facing
      setPixel(rgba, w, 6, oy + 4, 20, 20, 40);
      setPixel(rgba, w, 9, oy + 4, 20, 20, 40);
    } else if (frame === 1) { // left-facing
      setPixel(rgba, w, 5, oy + 4, 20, 20, 40);
    } else if (frame === 2) { // right-facing
      setPixel(rgba, w, 10, oy + 4, 20, 20, 40);
    }
    // frame 3 (up): no eyes visible

    // Hair (slightly darker than head)
    for (let x = 4; x < 12; x++)
      setPixel(rgba, w, x, oy + 0, headR * 0.6, headG * 0.6, headB * 0.6);
    for (let x = 5; x < 11; x++)
      setPixel(rgba, w, x, oy + 1, headR * 0.6, headG * 0.6, headB * 0.6);

    // Body (colored shirt)
    for (let y = 7; y < 12; y++)
      for (let x = 4; x < 12; x++)
        setPixel(rgba, w, x, oy + y, bodyR, bodyG, bodyB);

    // Arms
    for (let y = 8; y < 11; y++) {
      setPixel(rgba, w, 3, oy + y, bodyR * 0.8, bodyG * 0.8, bodyB * 0.8);
      setPixel(rgba, w, 12, oy + y, bodyR * 0.8, bodyG * 0.8, bodyB * 0.8);
    }

    // Legs (darker)
    for (let y = 12; y < 15; y++) {
      for (let x = 5; x < 8; x++)
        setPixel(rgba, w, x, oy + y, bodyR * 0.4, bodyG * 0.4, bodyB * 0.4);
      for (let x = 8; x < 11; x++)
        setPixel(rgba, w, x, oy + y, bodyR * 0.4, bodyG * 0.4, bodyB * 0.4);
    }

    // Feet
    setPixel(rgba, w, 4, oy + 15, 80, 50, 20);
    setPixel(rgba, w, 5, oy + 15, 80, 50, 20);
    setPixel(rgba, w, 10, oy + 15, 80, 50, 20);
    setPixel(rgba, w, 11, oy + 15, 80, 50, 20);
  }

  return { w, h, rgba };
}

/* ---- Generate all assets ---- */

// Tileset
const tileset = makeTileset();
writePNG("assets/tileset.png", tileset.w, tileset.h, tileset.rgba);
console.error("Generated assets/tileset.png (128x16, 8 tiles)");

// Hero (blue body, peach skin)
const hero = makeCharacter(40, 80, 200, 220, 180, 140);
writePNG("assets/hero.png", hero.w, hero.h, hero.rgba);
console.error("Generated assets/hero.png (16x64, 4 frames)");

// NPC (green body, peach skin)
const npc = makeCharacter(40, 160, 60, 220, 180, 140);
writePNG("assets/npc.png", npc.w, npc.h, npc.rgba);
console.error("Generated assets/npc.png (16x64, 4 frames)");

// Enemy (red body, pale green skin - looks menacing)
const enemy = makeCharacter(180, 40, 40, 160, 200, 160);
writePNG("assets/enemy.png", enemy.w, enemy.h, enemy.rgba);
console.error("Generated assets/enemy.png (16x64, 4 frames)");
