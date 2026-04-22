#!/usr/bin/env bun
/**
 * mkpalette.ts - Generate a 256-color game palette
 *
 * Layout:
 *   0:       Transparent (black)
 *   1-15:    Grayscale (15 levels)
 *   16-255:  15 color ramps x 16 shades
 *
 * Output:
 *   SRC/PALETTE.H     - C header with 6-bit VGA palette data
 *   tools/palette.json - 8-bit RGB values for sprite converter
 */

function hslToRgb(h: number, s: number, l: number): [number, number, number] {
  h = ((h % 360) + 360) % 360;
  const c = (1 - Math.abs(2 * l - 1)) * s;
  const x = c * (1 - Math.abs(((h / 60) % 2) - 1));
  const m = l - c / 2;
  let r = 0, g = 0, b = 0;
  if (h < 60)       { r = c; g = x; }
  else if (h < 120) { r = x; g = c; }
  else if (h < 180) { g = c; b = x; }
  else if (h < 240) { g = x; b = c; }
  else if (h < 300) { r = x; b = c; }
  else              { r = c; b = x; }
  return [
    Math.round((r + m) * 255),
    Math.round((g + m) * 255),
    Math.round((b + m) * 255),
  ];
}

const RAMPS = [
  { h: 0,   s: 0.85, name: "Red" },
  { h: 25,  s: 0.85, name: "Orange" },
  { h: 50,  s: 0.85, name: "Yellow" },
  { h: 80,  s: 0.80, name: "Lime" },
  { h: 120, s: 0.80, name: "Green" },
  { h: 160, s: 0.70, name: "Teal" },
  { h: 185, s: 0.75, name: "Cyan" },
  { h: 210, s: 0.75, name: "Sky" },
  { h: 240, s: 0.80, name: "Blue" },
  { h: 265, s: 0.70, name: "Indigo" },
  { h: 285, s: 0.70, name: "Purple" },
  { h: 320, s: 0.70, name: "Magenta" },
  { h: 30,  s: 0.50, name: "Brown" },
  { h: 20,  s: 0.40, name: "Skin" },
  { h: 100, s: 0.40, name: "Forest" },
];

// ---- build palette ----
const palette: [number, number, number][] = [];

// 0: transparent
palette.push([0, 0, 0]);

// 1-15: grayscale
for (let i = 1; i <= 15; i++) {
  const v = Math.round((i / 15) * 255);
  palette.push([v, v, v]);
}

// 16-255: 15 ramps x 16 shades
for (const ramp of RAMPS) {
  for (let shade = 0; shade < 16; shade++) {
    const l = 0.06 + (shade / 15) * 0.88;
    palette.push(hslToRgb(ramp.h, ramp.s, l));
  }
}

// ---- output C header (6-bit VGA) ----
const lines: string[] = [];
lines.push("#ifndef PALETTE_H_INCLUDED");
lines.push("#define PALETTE_H_INCLUDED");
lines.push("");
lines.push("unsigned char game_palette[768] = {");

for (let i = 0; i < 256; i++) {
  const [r, g, b] = palette[i];
  const r6 = Math.round((r * 63) / 255);
  const g6 = Math.round((g * 63) / 255);
  const b6 = Math.round((b * 63) / 255);

  let comment = "";
  if (i === 0) comment = " /* transparent */";
  else if (i <= 15) comment = ` /* gray ${i} */`;
  else if ((i - 16) % 16 === 0) comment = ` /* ${RAMPS[(i - 16) / 16].name} */`;

  const comma = i < 255 ? "," : "";
  lines.push(`    ${r6}, ${g6}, ${b6}${comma}${comment}`);
}

lines.push("};");
lines.push("");
lines.push("#endif");

await Bun.write("SRC/PALETTE.H", lines.join("\n") + "\n");
await Bun.write("tools/palette.json", JSON.stringify({ colors: palette }));

console.error("Generated palette: 256 colors (1 trans + 15 gray + 15 ramps x 16 shades)");
console.error("  -> SRC/PALETTE.H");
console.error("  -> tools/palette.json");
