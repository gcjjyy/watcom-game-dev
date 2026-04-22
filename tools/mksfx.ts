#!/usr/bin/env bun
/**
 * mksfx.ts - Convert any audio file to a DOS-side .SFX asset
 *
 * Spawns ffmpeg to decode <input> to raw 8-bit unsigned mono PCM at 22050 Hz,
 * then writes a 12-byte CSFX header + raw PCM bytes.
 *
 * File format (little-endian):
 *   offset  size  field
 *   0       4     'C','S','F','X'        magic
 *   4       4     sample_rate (uint32)   always 22050
 *   8       4     length (uint32)        number of PCM bytes following
 *   12      N     PCM bytes              unsigned 8-bit, mono, 22050 Hz
 *
 * Usage:
 *   bun tools/mksfx.ts <input_audio> <SRC/SFX_NAME.SFX>
 */

import { existsSync, writeFileSync, mkdirSync } from "fs";
import { dirname } from "path";

const FFMPEG = process.env.FFMPEG || "/Users/gcjjyy/.local/bin/ffmpeg";
const SAMPLE_RATE = 22050;
const MAX_BYTES = 1024 * 1024; // 1 MB cap

function fail(msg: string): never {
  process.stderr.write("mksfx: " + msg + "\n");
  process.exit(1);
}

const args = process.argv.slice(2);
if (args.length !== 2) {
  fail("usage: mksfx.ts <input_audio> <output.SFX>");
}
const [inputPath, outputPath] = args;

if (!existsSync(inputPath)) {
  fail("input file not found: " + inputPath);
}

const outDir = dirname(outputPath);
if (!existsSync(outDir)) {
  mkdirSync(outDir, { recursive: true });
}

const proc = Bun.spawnSync([
  FFMPEG,
  "-y",
  "-i", inputPath,
  "-f", "u8",
  "-ar", String(SAMPLE_RATE),
  "-ac", "1",
  "-loglevel", "error",
  "pipe:1",
]);

if (proc.exitCode !== 0) {
  const stderr = proc.stderr ? new TextDecoder().decode(proc.stderr) : "";
  fail("ffmpeg failed (" + proc.exitCode + "): " + stderr);
}

const pcm = proc.stdout;
if (!pcm || pcm.length === 0) {
  fail("ffmpeg produced empty output for " + inputPath);
}
if (pcm.length >= MAX_BYTES) {
  fail("SFX too large (" + pcm.length + " bytes, max " + MAX_BYTES + "): " + inputPath);
}

const out = new Uint8Array(12 + pcm.length);
// Magic "CSFX"
out[0] = 0x43; out[1] = 0x53; out[2] = 0x46; out[3] = 0x58;
// sample_rate LE
const sr = SAMPLE_RATE >>> 0;
out[4] = sr & 0xFF;
out[5] = (sr >>> 8) & 0xFF;
out[6] = (sr >>> 16) & 0xFF;
out[7] = (sr >>> 24) & 0xFF;
// length LE
const ln = pcm.length >>> 0;
out[8]  = ln & 0xFF;
out[9]  = (ln >>> 8) & 0xFF;
out[10] = (ln >>> 16) & 0xFF;
out[11] = (ln >>> 24) & 0xFF;
// Payload
out.set(pcm, 12);

writeFileSync(outputPath, out);
process.stdout.write(
  "  " + inputPath + " -> " + outputPath +
  " (" + pcm.length + " bytes PCM)\n"
);
