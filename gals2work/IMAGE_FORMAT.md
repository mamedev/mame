# Gals Panic II — verified type-0x20 image decoding

Verified on 2026-10-01. This replaces the earlier raw-pixel, width-sweep,
RGB555, and per-row-transform hypotheses.

## Result and evidence

- All **287 type-0x0020 images** in the Asia driver's 794-entry `imlist`
  decode to the pixel count in their headers, within their ROM boundaries.
- The other 507 table entries have other types, principally 0x0030 and
  0x0050; this decoder does not claim to handle them.
- **47 Japanese 256x256 image headers** found at their actual ROM offsets
  also decode with the same algorithm. These are candidates with verified
  pixel streams, not a reconstructed Japanese image-index table.
- Running the actual Japanese 68000 **B1C6** routine on the correct Asia B5
  payload produces **65,536 words identical** to the standalone decoder.
  The harness patches only volatile emulated ROM memory, supplies a real
  coroutine save area, and exits before the driver's 30-second debug job.
- Output SHA-256 (131,072 bytes, big-endian words, bit 15 masked):
  `f4b6c7aaf66966aa6b87a95c4431caa94a561be30bcab5ba9761c7b37f901651`.

## Why the old investigation failed

The driver's hard-coded `imlist[794]` matches **galpani2 (Asia)** data.
The old `raw_image_01.bin` and `raw_image_b5.bin` were captured from
**galpani2j (Japan)** at the Asia offsets. Their initial bytes match the
Japanese ROM at those offsets, but those addresses land inside other
image streams, not at image headers.

At Asia offset 0x000eac7e, the actual header is `0020 00ef 013f`.
At the same Japanese offset, the bytes begin `197e 427f 4abf`, which are
image payload data. Trying to decode them as a fresh packet stream loses
synchronization. Viewing compressed literal payloads as raw pixels can
still show recognizable fragments, with drift from packet control bytes.

The dump command also interpreted length arguments as hexadecimal:
`raw_image_01.bin` is 0x143080 = 1,323,136 bytes, not 143,080 bytes;
`raw_image_b5.bin` is 0x120000 = 1,179,648 bytes, not 61,820 bytes.
The true Asia image lengths of 143,080 and 61,820 bytes came from adjacent
Asia offsets, not from those Japanese dumps.

## Header

Six bytes, big-endian:

| Offset | Meaning |
|---|---|
| +0 | Type word: 0x0020 |
| +2 | Height minus one |
| +4 | Width minus one |
| +6 | First compressed packet |

Examples:

| Asia index | Offset | Dimensions | Consumed including header | Next table entry |
|---|---|---|---|---|
| 0x00 | 0x000ccbf4 | 320x240 | 123,018 bytes | 0x000eac7e |
| 0x01 | 0x000eac7e | 320x240 | 143,079 bytes | 0x0010db66 |
| 0xb5 | 0x002cfb1e | 256x256 | 61,819 bytes | 0x002dec9a |
| 0xd3 | 0x0044250c | 256x256 | 57,461 bytes | 0x00450582 |

The first assets include abstract/title graphics; index 1 is not the blue
anime garment seen in the old Japanese raw dump. B5 is a clean illustration,
and D3 is a clean photograph.

## Packets and colors

Read one control byte. The pixel count is `(control & 0x7f) + 1`.

- Bit 7 clear: read that many individual big-endian 16-bit color words.
- Bit 7 set: read one big-endian 16-bit color word and repeat it.

Stop when the header's rectangle is filled. Some genuine final repeat
runs extend beyond that rectangle, just as the native routine permits.
Some entries also contain an unreachable trailing black repeat packet
and/or an alignment byte. Their details are retained in the manifest;
there is no end-marker scan or guessed width.

Colors are **GRB555**, matching `palette_device::GRB_555` in the driver:
G = bits 10..14, R = bits 5..9, B = bits 0..4. Native B1C6 sets bit 15 on
written framebuffer words; it is excluded from color conversion.

For standalone previews, the linear decoded words are rendered at the
header's width and height. A clockwise 90-degree rotation makes the B5
and verified Japanese examples upright. Hardware framebuffer addressing,
page selection, and image composition require separate integration work.

## Reproduce

Run from the repository root. The exporter reads the entire ROM region,
independent of the currently selected bank, and converts host-order 16-bit
region storage to logical big-endian bytes.

```sh
SDL_VIDEODRIVER=dummy ./mame galpani2 -video none -sound none -nothrottle \
  -skip_gameinfo -autoboot_delay 0 -autoboot_script gals2work/export_rom.lua \
  -seconds_to_run 2 -cfg_directory /tmp/gp2_codex_cfg \
  -nvram_directory /tmp/gp2_codex_nvram

python3 gals2work/decode_bg15.py gals2work/galpani2_subdata_logical.bin \
  --validate-table --manifest gals2work/verified_bg15_manifest.json

python3 gals2work/decode_bg15.py gals2work/galpani2_subdata_logical.bin \
  --index 0xb5 --rotate 90 --scale 2 \
  --output gals2work/decode_attempts/SOLVED_asia_b5.png \
  --words gals2work/decoded_b5_words.bin

SDL_VIDEODRIVER=dummy ./mame galpani2j -video none -sound none -nothrottle \
  -skip_gameinfo -autoboot_delay 0 -autoboot_script gals2work/verify_native.lua \
  -seconds_to_run 12 -cfg_directory /tmp/gp2_codex_cfg \
  -nvram_directory /tmp/gp2_codex_nvram

cmp gals2work/decoded_b5_words.bin gals2work/native_b5_words.bin
```

For Japan, first run `export_rom.lua` with `galpani2j`, then use a verified
header offset, e.g.:

```sh
python3 gals2work/decode_bg15.py gals2work/galpani2j_subdata_logical.bin \
  --offset 0x1ff17a --rotate 90 --scale 2 \
  --output gals2work/decode_attempts/SOLVED_japan_1ff17a.png
```

PNG creation uses Pillow. Binary decoding and table validation use only
Python's standard library. `--index` and `--validate-table` reject ROM data
that does not match the Asia table's first header.

## Remaining emulator work

The decoder succeeds independently; the game driver is still incomplete.
Use the existing disassembly to implement the MCU's image selection,
descriptor processing, task parameters, and completion handshake. Do not
assume the Japanese sub-CPU addresses apply to the Asia program.

For B1C6, the source must point **after the six-byte image header**.
The harness verified a raw framebuffer destination, +8/+a counters of
0x00ff, and +e = 0x00ff for a complete 256x256 image. A task entered directly
must also have a valid register save pointer at task-entry +0x0c.

The native code advances its line/column base by **0x400 bytes**. Do not
infer a 0x200-byte full stride just from a `lea ($200,A0),A0` that follows
0x200 bytes of post-increment writes. The actual raster mapping and
0x314000 page register are not established by this decoding result.

B1C6's chunk yields set the task to 0x40, leaving it runnable. It blocks
with 0xc0 at completion. Continually reactivating a completed task restarts
its image job; it is unnecessary for normal chunk progress.

The current C++ driver still contains the previous forced test at about
30 seconds, with a header included in its source and a mismatched regional
image offset. That experimental block and the hard-coded page-3 renderer
must be addressed during integration. No C++ driver changes were made in
this investigation.
