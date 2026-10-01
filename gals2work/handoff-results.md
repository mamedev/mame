# Handoff results for Opus — Gals Panic II image decoding

Verified 2026-10-01. Continue with MCU/driver integration, using these results
as the baseline. The previous handoff's image-format hypotheses are superseded.

## What is now established

**The ordinary type-0x0020 background image format is decoded successfully.**
There is no missing per-row XOR, LFSR, delta, or predictive transform for it.

The main failure in the previous investigation was applying the driver's
**Asia `imlist[794]` offsets to the Japanese ROM set**. The old raw files
match Japanese ROM bytes at those offsets, but start inside unrelated image
streams. Smooth literal color data still showed recognizable art when rendered
raw, which led to misleading width and transform guesses.

Correct Asia image starts have a **six-byte big-endian header**, followed by
ordinary 16-bit RLE packets. Colors are **GRB555**, as configured in the driver.
Start native decompression at **image offset + 6**, after the header.

The complete format, edge cases, exact offsets, and reproduction commands are
in [IMAGE_FORMAT.md](IMAGE_FORMAT.md). Read that file before further experiments.

## Evidence

- `decode_bg15.py --validate-table` passes **all 287 Asia type-0x0020 images**.
  Their output sizes match the header dimensions and fit within image boundaries.
  The other **507** table entries have other types and are outside this decoder's
  scope. See `verified_bg15_manifest.json`.
- **47 Japanese 256x256 streams** also decode at independently discovered
  header offsets. Example: **0x001ff17a**. See
  `verified_japan_bg15_candidates.json`. This does **not** establish the Japanese
  image-index mapping.
- The real Japanese 68000 **B1C6** routine was run against the correct Asia B5
  payload, copied into volatile emulated ROM memory for an isolated test.
  Its **65,536 output words match the standalone decoder byte for byte**.
  See `verify_native.lua`, `native_b5_words.bin`, and `decoded_b5_words.bin`.
- Native output SHA-256, 131,072 big-endian bytes with bit 15 masked:
  `f4b6c7aaf66966aa6b87a95c4431caa94a561be30bcab5ba9761c7b37f901651`.
- Clean previews:
  `decode_attempts/SOLVED_asia_b5.png` and
  `decode_attempts/SOLVED_japan_1ff17a.png`.

The native test initially failed because entering a task directly bypassed
the scheduler's normal assignment of its coroutine register save area.
Supplying task-entry **+0x0c** with a valid save buffer fixed the test. This is
now handled explicitly in the harness.

## Tools and data now available

All paths below are relative to `/Users/andreabogazzi/develop/mame/gals2work/`.

- `decode_bg15.py`: standard-library binary decoder, Asia table validation,
  optional Pillow PNG output. Use `--index` only for Asia, and `--offset` for
  a verified header in another set. It rejects mismatched Asia-table data.
- `export_rom.lua`: exports complete `:subdata` and `:sub` ROM regions in logical
  big-endian order, independently of bank selection. Handles host byte order.
- `galpani2_subdata_logical.bin` / `galpani2_sub_logical.bin`: fresh Asia exports.
- `galpani2j_subdata_logical.bin` / `galpani2j_sub_logical.bin`: fresh Japanese
  exports. These are already normalized; **do not swap their bytes again**.
- `verify_native.lua`: reproducible CPU comparison, using Japanese program
  addresses with a known Asia payload in volatile emulated memory.

The old dumps' sizes were also misread: debugger lengths are hexadecimal.
`raw_image_01.bin` is **1,323,136 bytes**, and `raw_image_b5.bin` is
**1,179,648 bytes**. They are not isolated 143,080-byte and 61,820-byte images.

## What remains and where to continue

1. **Integrate the proven decoder through the MCU simulation.** Use the existing
   descriptor and task-dispatch analysis to obtain real source, destination,
   dimensions, and completion parameters. The decoder itself is no longer the
   unknown for type 0x0020.
2. **Respect regional differences.** The hard-coded table is correct for Asia
   data. The saved B1C6/handler disassembly and test task addresses are Japanese.
   Do not assume Japanese program addresses apply to Asia. Reconstruct the
   Japanese image-index mapping before using MCU lookup commands there.
3. **Remove or replace the old forced C++ test when integrating.** The current
   `galpani2_mcu_nmi2()` still starts a forced decompression job after roughly
   30 seconds, using an unskipped header and mismatched regional data. It also
   repeatedly reactivates the task. This block is experimental debugging code.
4. **Implement the actual framebuffer page/layout selection.** The current
   renderer hard-codes page 3. B1C6 advances its base by **0x400 bytes**. A fill
   loop's `lea ($200,A0),A0` after 0x200 bytes of post-increment writes likewise
   gives a total 0x400-byte stride. Pixel decoding alone does not establish the
   final raster addressing, orientation, or `0x314000` register behavior.
5. **Handle other formats separately.** Type 0x0030 / 0x0050 assets were not
   decoded or integrated in this investigation.

The native B1C6 routine **does not need MCU reactivation every chunk**. Its
chunk yield clears bit 7 and sets bit 6 (0x40, runnable); it blocks with 0xc0
at completion. Reactivating a completed task restarts its job.

## Workspace state and operating notes

No C++ driver changes were made in this investigation. Documentation was
corrected in `IMAGE_FORMAT.md`, `HANDOFF_PROMPT.md`, `NOTES.md`, and
`GP2_MCU_ANALYSIS.md`; the historical notes are retained with correction notices.
New scripts, manifests, dumps, and previews exist on disk. The repository's
broad ignore rules hide new files under `gals2work`, so normal Git status may
not list them. No commit or staging was performed by this investigation.

Other experimental commits appeared while this work was running. Preserve
concurrent changes and inspect the current diff before editing the driver.

For GitHub work on this machine, use the **gh CLI exclusively**, as required
by the user's global instructions. Authentication was verified successfully.
Local headless MAME runs work with `SDL_VIDEODRIVER=dummy`; without that setting,
SDL fails to initialize displays in the execution environment. The native test
finishes around five emulated seconds, before the old driver's timed test.

## Suggested skills

- `handoff`: use when refreshing this continuation document.
- No additional installed skill is necessary for the immediate ROM decoding
  and MCU debugging work. The ZGB/Game Boy banking skills do not apply here.
