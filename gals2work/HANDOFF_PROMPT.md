# Gals Panic 2 Image Decompression — Handoff for Continuation

## The Problem
MAME's Gals Panic 2 driver (`src/mame/kaneko/galpani2.cpp`) has unemulated MCU protection (NEC uPD78324 "PISCES" with undumped 32KB internal ROM). The game's bg15 layer should show photographic "gal" images but they never render.

## What Works Already
- Image address banking fix in cmd=0x0c handler: returns `0x800000 + (offset & 0x7FFFFF)` with bank select. Eliminated all unmapped reads.
- Title screen 3D bg8 backgrounds render correctly
- Attract mode gameplay runs with sprites, borders, scores
- Game is playable except bg15 picture layer is blank

## The Image Format Discovery (Where You Continue)

Images are stored in the **subdata ROM** (32MB, `ROM_REGION16_BE`). The `imlist[794]` table maps image indices to ROM byte offsets.

### What we know about the format:
1. **Width: ~322-324 bytes per row** (161-162 pixels at 16bpp). The autocorrelation peak is at lag=324. When rendering Image 1 (143080 bytes) at 320-324 bytes/row as 16-bit xRGB555, the first ~20 columns show **clean, recognizable anime art** (blue clothing/fabric with smooth shading). See `gals2work/decode_attempts/ZOOM_w324_first60.png`.

2. **The pixel data degrades across each row** — first columns are clean, later columns get noisy. This strongly suggests a **stateful per-row transform** (running counter, LFSR, XOR with incrementing key, delta encoding, or similar) that we're not applying. The MCU's internal code would apply this transform.

3. **The data is NOT RLE compressed** — raw bytes at the correct width show the image directly. B1C6 and B0E0 decompression routines from the sub CPU produce noise when applied to this data.

4. **Color format: xRGB555 big-endian** (5 bits each R, G, B with bit 15 unused). The visible blue clothing matches expected RGB values.

5. **Image sizes vary**: Image 0 = 123018 bytes, Image 1 = 143080 bytes, Image 0xB5 = 61820 bytes. At ~322 bytes/row: Image 1 = ~444 rows (taller than 320-pixel screen, includes scroll area).

6. **Some images have trailing bytes** after the pixel data: Image 0xB5 has 80 trailing bytes that look like count+color pairs.

### Key files:
- `gals2work/raw_image_01.bin` — Raw subdata ROM dump of Image 1 (143080 bytes)
- `gals2work/raw_image_b5.bin` — Raw subdata ROM dump of Image 0xB5 (61820 bytes)  
- `gals2work/decode_attempts/ZOOM_w324_first60.png` — **Best decode so far**: first 60 rows at width 324, zoomed 4×, rotated 90°. Shows clear anime art.
- `gals2work/decode_attempts/ZOOM_w320_first60.png` through `ZOOM_w328_first60.png` — Width comparison
- `gals2work/decode_attempts/IMG1_w320_rot90.png` — Full image at 320 bytes/row, noisy but structure visible

### The key question:
**What per-row stateful transform turns the noisy pixels into the clean image?**

Possibilities to try:
- Delta encoding (each pixel = previous pixel + encoded delta)
- XOR with previous pixel
- XOR with a per-row LFSR or counter  
- Predictive coding (each pixel predicted from neighbors, encoded value is difference)
- The first pixel of each row might be literal, subsequent pixels are deltas

The fact that the FIRST columns are clean and later columns degrade means the transform accumulates error when not applied — consistent with delta/predictive encoding.

### Hardware context:
- Game: Kaneko, 1993, arcade
- CPU: 2× 68000 + PISCES MCU (NEC uPD78324, 32KB internal ROM **NOT DUMPED**)
- Display: 320×240, ROT90 (stored as 240×320, displayed vertically)
- bg15 layer: 15-bit direct color framebuffer at sub CPU address 0x400000-0x5FFFFF
- Subdata ROM: 32MB at sub CPU 0x800000, banked in 8MB pages

### Image grid table at sub CPU ROM 0xA504:
9 cols × 10 rows. Each cell: `[image_index, y_start, x_col, x_col, y_end]` (5 words).
Row pairs share the same x_col value. The grid tiles compose the full-screen picture.

### Related working game:
Gals Panic 3 (`src/mame/kaneko/galpani3.cpp`) uses GRAP2 chips with byte-level RLE compression. GP3 is working. GP2 does NOT use GRAP2 format — the raw bytes show the image directly without RLE decompression.
