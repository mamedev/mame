# Task: derive the true sprite/palette display pipeline of Taito Racing Beat (MAME `taito_z.cpp`)

## Goal

Determine, from first principles (68000 disassembly of the game code + MAME driver source), the exact
hardware behavior of the sprite double-buffer / display pipeline on the 1991 Taito Z board used by
`racingb` (Racing Beat) and `sci` (Chase H.Q. II / SCI), so the emulation stops being an empirically
tuned approximation. Deliverable: a precise statement of (a) what the spriteframe register write does
in hardware, (b) how many frames/periods separate a game write to sprite RAM from its scanout,
(c) when palette RAM writes take visual effect, and (d) a minimal MAME patch implementing it.
No real-hardware footage exists (Racing Beat has ~3 known PCBs, no home ports), so the game code is
the only ground truth: the game was written against the real chip, so every scheduling decision in
its code encodes the hardware's timing.

## Hardware / driver context

- Board: Taito Z (`src/mame/taito/taito_z.cpp`, `taito_z_v.cpp`, `taito_z.h`), twin 68000 @16MHz,
  screen 424x262 dots, visible lines 16-255, vblank lines 256-261+0-15.
- Sprites: 16x8 chunks composed into 64x64 objects via spritemap ROM
  (`sci_state::sci_draw_sprites_16x8`). Sprite chips TC0370MSO + TC0300FLA (+TC0380BSH on racingb),
  not emulated as devices — the driver hand-rolls drawing.
- Sprite RAM: 0x4000 bytes at 0xb00000 (racingb) / 0xc00000 (sci). Two list halves of 0x800 words:
  half A = +0x0000, half B = +0x1000 (byte offsets). Entry = 4 words:
  w0 = zoomy(14:9)|y(8:0), w1 = pri(15)|color(14:7)|zoomx(5:0), w2 = flips|x(8:0), w3 = tile(12:0).
- "Spriteframe" register: 0xb08000 (racingb) / 0xc08000 (sci), byte in the high half
  (`sci_spriteframe_w` uses `data >> 8`).
- Palette: 4096 xBGR555 entries, plain RAM at 0x700000 (racingb), written live by CPU A.

## Established facts (all verified this investigation; scripts and dumps exist)

1. racingb writes the spriteframe register once per 2-frame period, at vpos 258, alternating bit0.
   sci writes it EVERY frame at vpos 256-258 (same value twice per period, bit0 alternates per period).
2. racingb builds its sprite list into alternating halves, one half per period, with writes landing
   mid-visible-frame (beamy ~90-160) — i.e. it beam-races the buffer.
3. The build target ↔ register value pairing is OPPOSITE in the two games:
   - racingb (pointer-set table at CPU A ROM $7b84, four 42-byte blocks selected by counter&3):
     build half A (chip pointer 0xb00ff8) pairs with register value 1; half B (0xb01ff8) with 0.
   - sci: builds half A (0xc00000) while register = 0.
   Therefore the absolute register value cannot directly select the displayed half; only the bit-0
   EDGE (one per period in both games) can be universally meaningful — or each game's convention is
   absorbed by some other state.
4. Everything in racingb is scheduled off one counter in shared RAM at $11800a (A5=$118000 in the
   relevant code; the palette phase flag is `btst #0, ($b,a5)` = low byte of that counter).
   The counter is incremented by a main-loop routine at $7aa0 (int6-paced via a flag set by the
   int6 handler at $7a98); the same routine selects the pointer-set block (counter&3), updates the
   sprite build pointers, and writes the spriteframe register.
5. The game's palette script is double-buffered in software: routine at $7000 uploads one of two
   prepared blocks (A5-$6940 or A5-$6540, counts at A5-$6944/-$6942) to palette RAM $700400+
   (colour bank 0x20 onward) every vblank at vpos 259; the phase bit picks the block.
   Car body uses colour banks 0x20/0x21 (palette-cycled: 0x20 every period, 0x21 every 2 periods,
   with occasional longer holds — script-level), plus a duplicated shadow copy in bank 0x4a.
6. Measured art→palette schedule: when the car's tile set changes (steering state), the matching
   bank 0x21 palette lands 4 frames (2 periods) after the sprite-RAM art write, systematically
   (5+ instances). Under MAME's original code (draw live spriteram half selected by
   `0x800 - (reg&1)*0x800`) this shows as a 4-frame wrong-palette flash on the car.
7. Empirically best emulation found (user eye-verified, big flash gone, tiny residual remains):
   on each register bit-0 edge, latch (copy) the half that is ABOUT to be rebuilt — i.e. its stable
   one-period-old content — and draw from that latch for the whole period. Palette applied live
   (a vblank palette latch made things worse). In the working tree this is
   `sci_spriteframe_w`/`sci_draw_sprites_16x8` with env knobs; winning combo
   TZ_SPRMODE=1(edge), TZ_SPRSTAGES=1, TZ_SPRPHASE=1, TZ_PALLATCH=0.
8. Killed hypotheses (do not revisit without new evidence): register value as direct display-half
   selector (breaks one of the two games each way); pure write-triggered toggle (sci double-writes);
   wall-frame snapshot ring (mispairs when the script holds a step); vblank palette latch (worse);
   2-stage edge pipeline in any phase (worse per user).

## What to do

1. Disassemble racingb CPU A (ROMs c84-110.3/c84-111.5 + c84-104.2/c84-103.4, 16-bit interleave,
   see any capstone-based script in the project notes) around:
   - $7a98 (int6 handler), $7aa0-$7b93 (frame-advance routine), the $7b84 pointer table blocks,
   - the sprite list BUILD code (writes through the pointers loaded from the table: entries land
     via (a5-$7ed2..-$7eba) pointers; find where entries are emitted and which half),
   - $7000-$707e (palette upload) and the code that PREPARES the two palette blocks
     (writers of A5-$6940/A5-$6540 regions and counts at A5-$6944/-$6942),
   - whatever reads/writes the counter word $11800a and the int6 flag (A5-$7f24).
2. From the code, reconstruct the intended timeline in periods: for period N, which half is being
   built, what register value is written, which palette block is prepared vs uploaded, and which
   art the palette being uploaded belongs to. The 2-period art→palette offset (fact 6) must fall
   out of this reconstruction; when it does, the scanout delay of the sprite chip is pinned exactly.
3. Cross-check against sci's code (ROMs in sci.zip; its builder writes at beamy≈15, register write
   every frame) — the derived chip model must explain both games with one rule.
4. Express the result as a minimal patch to `sci_spriteframe_w` / `sci_draw_sprites_16x8`
   (and remove the TZ_* env scaffolding). Also state what the model implies for `dblaxle`
   (same sprite chips, single-buffered list, vestigial int6 — see notes) and for the remaining
   one-orientation residual flash.

## Materials available in the repo/workspace

- Working tree: `~/develop/mame`, branch `mame-tiato-double-axels`. Committed: tc0480scp row-zoom
  fix (65c9e379b08). Uncommitted: the experimental pipeline with TZ_* knobs in taito_z.{h,cpp}, taito_z_v.cpp.
- Binaries: `./taitoz-dev` (current), `./taitoz-family` (pre-fix reference), `./taitoz-start`.
- ROMs in `roms/` (racingb, sci, dblaxle, etc.). Interleaved CPU images and analysis scripts were
  built in /tmp/dblaxle_dis/ (racingb_cpua.bin etc.) — rebuild if gone.
- Frame-state dumps: `scenedump/rb_b*.bin` (+ .txt manifests) — per frame: u32 frame, u32 sprfrm,
  0x30 SCP ctrl, 0x4000 sprite RAM, 0x2000 palette (record size 0x6038). Older rb_long.bin uses
  record size 0x6008 (no ctrl block). Matching per-frame PNGs under snap/.
- Capture tooling: `scene_dump_racingb.lua` (/ = single, shift+/ = 120-frame burst with per-frame
  screenshots), Lua frame-exact snapshotting pattern, MAME debugger wpset with `addr:sub` syntax
  for CPU B (tag is "sub"), logerror-to-error.log via `-log`.
- Full investigation log: memory file `taitoz-road-investigation` (Claude Code memory) and this file.

## Constraints

- Do not trust the old driver comments about int6/spriteframe (partially wrong; see notes).
- User commits manually; never commit or add attribution.
- Any change to shared devices must be regression-checked (user does eye passes).
