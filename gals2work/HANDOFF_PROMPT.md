# Gals Panic II — image decoding resolved (2026-10-01)

The earlier prompt below is retained as historical context. Its conclusion
that the data is raw/uncompressed, its width guesses, RGB555 ordering, and
its proposed missing per-row transform are superseded.

Read **IMAGE_FORMAT.md** first. `decode_bg15.py` successfully decodes all
**287 Asia type-0x0020 images**. The header is six bytes: type, height minus
one, width minus one, all big-endian. Packets use bit 7 for repeat versus
literal and the low seven bits for count minus one. Colors are GRB555.
There is no missing XOR/delta transform for this format.

The principal mistake was using the Asia `imlist` offsets while running
**galpani2j**. The handoff's raw dumps therefore start inside unrelated
Japanese image streams. Correct Asia addresses reveal the headers and
clean pictures immediately. `--index` now rejects mismatched ROM data.
The decoder also produces clean Japanese pictures with independently
verified offsets (example: 0x001ff17a); the Japanese index table remains
to be reconstructed.

`verify_native.lua` runs the actual Japanese 68000 B1C6 routine against a
correct Asia B5 payload copied into volatile emulated ROM memory. Its
65,536 output words match the standalone decoder exactly. This validates
the packet interpretation. The test requires a valid coroutine save area
and uses source = image offset + 6. Chunk yields remain runnable and do
not need repeated MCU reactivation.

Useful deliverables:
- `decode_bg15.py`: reproducible decoder and Asia table validation.
- `export_rom.lua`: bank-independent, endian-correct ROM export.
- `verify_native.lua`: native CPU comparison harness.
- `verified_bg15_manifest.json`: all 287 validated Asia images.
- `verified_japan_bg15_candidates.json`: 47 Japanese 256x256 streams.
- `decode_attempts/SOLVED_asia_b5.png`: clean illustration.
- `decode_attempts/SOLVED_japan_1ff17a.png`: clean Japanese picture.

Next work is MCU/driver integration and regional offset mapping. The
current driver retains the old forced job after ~30 seconds and a
hard-coded framebuffer page. No C++ driver changes were made here.

---

## Original handoff — historical, unverified conclusions

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

## Update: 8bpp at 320 bytes/row shows clearest silhouette

Image 0xB5 at **320 bytes per row, 8bpp** (one byte per pixel, palette-indexed)
shows a clear figure silhouette when rotated 90°. See `B5_8bpp_w320_rot90.png`.

Image 1 at **480 bytes/row, 16bpp** (240 pixels) also shows blue clothing
detail in the first ~30 rows when zoomed. See `ZOOM_w480_240px_rot90.png`.

254 unique byte values used — full 8-bit range. No palette found yet.
Trailing 60 bytes after pixel data don't form a valid palette.

The puzzle: WHERE is the 256-color palette that maps byte indices to RGB colors?
