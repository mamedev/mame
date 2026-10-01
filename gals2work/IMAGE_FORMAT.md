# Gals Panic 2 — Image Format Investigation

## Image Storage

Images are in the **subdata ROM** region (32MB, banked at sub CPU 0x800000).
The `imlist[794]` table in the MCU simulation maps image indices to ROM offsets.

Image sizes vary: 25KB to 140KB. No header at the start of image data — the
previous image's data runs right up to the next.

## Image Grid Table (Sub CPU ROM 0xA504)

9 columns × 10 rows. Each cell at byte offset `(col * 256 + row * 16)`:

| Offset | Size | Field | Notes |
|--------|------|-------|-------|
| +0 | word | image_index | Used for pointer table lookup |
| +2 | word | f1 | Y position/start (row pairs differ by ~37-47) |
| +4 | word | f2 | X column parameter (always == f3) |
| +6 | word | f3 | Same as f2 |
| +8 | word | f4 | Y end/total (sometimes == f1) |

Row pairs (0+1, 2+3, 4+5, 6+7) have consecutive image indices and matching
f2/f3 values, suggesting top/bottom halves of pictures.

## Raw Byte Analysis

Image 0xB5 (title screen, 61820 bytes) rendered as raw bytes at width **490**
shows a visible grayscale landscape. This proves the data is NOT RLE compressed.

| Image | Size | imlist offset | Notes |
|-------|------|--------------|-------|
| 0xB5 | 61820 bytes | 0x2CFB1E | Title screen, visible at w=490 |
| 0xD3 | 57462 bytes | 0x44250C | Gameplay image |
| 0x00 | 123018 bytes | 0x0CCBF4 | First image (largest) |

## What Failed

- B1C6 decompression (native 68000): produces noise
- B0E0 3-byte RLE format: structured but no picture
- GRAP2 byte-level RLE (from GP3): structured but no picture
- 3-plane split (R/G/B channels): doesn't resolve
- Raw 16-bit words at any width/endianness: noise
- Raw bytes at standard widths (256, 320, 512): noise

## What Works

- Raw bytes at width **490** show faint image structure
- 490 × 126 ≈ 61740 bytes ≈ full image 0xB5 size

## Open Questions

1. Why width 490? (not a power of 2 or standard resolution)
2. Where is the palette data? (if 8-bit indexed)
3. The grid table f1-f4 fields — how do they map to pixel positions?
4. Does the MCU apply a transform (XOR, bit rotation, palette lookup)?
