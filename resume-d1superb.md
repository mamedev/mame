# Handoff: F-1 Super Battle (`f1superb`) — "FPU" coprocessor investigation

You are continuing work on MAME's Jaleco Mega System 32 driver, game **F-1 Super Battle** (Jaleco 1994).
Repo: this MAME checkout (branch `master`). ROMs: `roms/f1superb.zip` (MAME 0.270 split set, complete; only the motherboard PAL `91022-01.ic83` is NO_DUMP, which is normal).
Prebuilt binary: `./mame288` in the repo root (MAME 0.288, same f1superb driver as the source tree).

## Goal
Make f1superb playable. The main blocker is the two unemulated maths coprocessors that Jaleco calls "FPU". Immediate next step: **write a disassembler for its 20-bit instruction set**, then work out the opcode semantics, then a high-level implementation in the driver.

## Where the code is
- `src/mame/jaleco/ms32.cpp` — driver.
  - Header "Not Working Games" note, ~line 143.
  - `ms32_f1superbattle_state::f1superb_map`, ~line 724: link RAM `fd0c0000`, DSW2 `fd0d0000`, analog `fd0e0000`, COPRO 1 RAM `fd100000-fd105fff`, COPRO 2 RAM `fd140000-fd145fff`, road VRAM `fdc00000`, `fde00000` (scroll/line info?). The irq2/irq5 "guess" writers are commented out because they break the other MS32 games.
  - nuapete's reverse-engineering emails, ~lines 764-870 (read them).
  - Inputs `INPUT_PORTS_START( f1superb )` ~line 1452; `gfx_f1superb` uses `gfx5` as 2048x1 raw "tiles".
  - `init_f1superb` ~line 2714: an `#if 0` ROM hack (forces the sprite Y table copy); leave it disabled.
  - GAMEL line ~2752: `MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN`.
- `src/mame/jaleco/ms32.h` ~line 175 — `ms32_f1superbattle_state` (`// TODO: COPROs`).
- `src/mame/jaleco/ms32_v.cpp` — `get_ms32_extra_tile_info` / `video_start` create `m_extra_tilemap` (road, 1 x 0x400 rows, 2048 wide) but **it is never drawn** in `screen_update`.

## Known status (not working)
1. Coprocessors unemulated -> sprite X/Y buffers (`fee11000` X, `fee11100` Y, `fee11200` priority) never computed; the road is always straight.
2. Road layer never rendered; the `fde00000` table format is unknown.
3. Background pen 0 wrong for f1superb (grid text black on black), see TODO in `ms32_v.cpp` ~line 366.
4. Link (irq 11 -> "FLAM ERROR" in test mode) not emulated; sound-ack may kill sound.

## Findings from the previous session (verified)
### Interrupt names (strings in program ROM at ffe01522..)
`1MSEC`, `SOUND CPU`, `FPU 1-1`, `FPU 0-1`, `FPU 1-0`, `FPU 0-0`, `OPTION 2`, `OPTION 1`, `COMMUNICATION`, `32MSEC`, `16MSEC`.
So: **two FPU units, each with two IRQ lines**. nuapete's mapping: irq2 = fpu 1-1 (handler ffe00878), irq5 = fpu 0-0 (ffe008ac), irq7 identical to 5, irq 3/4/6 unused. The handlers do `or.w #6, FD1424C8[PC]` (likely ack). Also nearby: strings `LEFT  ERROR` / `RIGHT ERROR` at ffe01614 (possibly the two units' self-test result — unconfirmed).

### It is NOT a classic FPU — it is a programmable processor
- At boot the V70 uploads the **same program** into both units: `fd104000` and `fd144000` (0x800 u32 slots, 1996 words used).
- Source of that program in the program ROM: **`ffe16044`**, stored as u16 little-endian words (file offset 0x16044 in the interleaved program image).
- Each slot uses only 20 bits. Words come in pairs: **4-bit opcode word, 16-bit operand word** -> 998 instructions.
- Start of the program is a hardware self-test:
  ```
  fd104000: 000e 4130 0006 0000
  fd104010: 000f c939 0006 0001
  fd104020: 000f 8939 0006 0002   ; walking bit 0x0001..0x8000 into r6, then r7
  ...
  fd1048b0: 0004 0000 0000 ffff   ; r4=0000 r0=ffff
  fd1048c0: 0001 aaaa 0002 5555
  fd1048d0: 0003 1111 0005 0000
  ```
- Tentative opcode classes: `0-7` = load imm16 into r0-r7 (8 registers); `8-B` = ALU ops with packed register fields (e.g. `a fcf0`, `8 9df3`); `C` = memory access (`c 43b3`); `D` = shift/iteration with halving constants (`d b600, b500, b480, b440, b420, b410` and `d ea00, e900, e880 ...` — looks like divide/sqrt/CORDIC loops); `E` = control (`e 4080`, `e 4100`, `e 4130`); `F` = branches (`f a113`, `f c939`...). None of this is confirmed yet.
- Each unit's window: `+0x0000-0x1fff` 16-bit data RAM (fixed-point, no floats seen); `~+0x2400` control registers (non-zero at `+0x2430`, `+0x24c0`; ack at `+0x24c8`); `+0x4000-0x5fff` program.
- 20-bit instructions match **no common DSP** (TMS320C1x/C2x/C5x 16-bit, uPD7725/96050 / ADSP-21xx / DSP56k 24-bit, Fujitsu MB86233 TGP 32-bit). Best guess: Jaleco custom microprogrammed math unit (gate array, maybe plus a hardware multiplier). No PCB photo identified yet. Closest architectural analogue: Sega Model 1 (V60 host + programmable TGP via shared RAM).
- Jaleco's previous racers (Big Run, Cisco Heat, F-1 GP Star, Wild Pilot, `cischeat.cpp`) did the maths in software on multiple 68000s.
- nuapete: the host runs a sequence of **4 operations** on the unit per frame, using two static tables loaded at boot, two identical register sets and 4 banks of per-frame data from per-track arrays in ROM. Unused debug routine at `FFE47FBC` prints sprite coord/angle info (string at `FFE481FC`) — useful to learn the output format.

## Tools / recipes
Work in a scratch dir, not in the repo.

Build the interleaved V70 program image (`ROM_LOAD32_BYTE`: f1sb29 at +0, 28 at +1, 27 at +2, 26 at +3; mapped at `ffe00000`):
```python
import zipfile
z = zipfile.ZipFile('roms/f1superb.zip')
d = [z.read(f'f1sb{n}.bin') for n in (29, 28, 27, 26)]
out = bytearray()
for i in range(0x80000):
    for k in range(4): out.append(d[k][i])
open('prg.bin', 'wb').write(out)
```

Extract the coprocessor firmware (998 x (op4, arg16)):
```python
import struct
p = open('prg.bin', 'rb').read()
w = struct.unpack_from('<1996H', p, 0x16044)
prog = [((w[i] & 0xf) << 16) | w[i+1] for i in range(0, 1996, 2)]
```

Headless boot + memory dump (Lua, run from the scratch dir so files land there):
```lua
-- dump.lua
local sp = manager.machine.devices[":maincpu"].spaces["program"]
local frames = 0
local function dump(name, base, len)
  local f = io.open(name, "wb")
  for a = base, base + len - 1 do f:write(string.char(sp:read_u8(a))) end
  f:close()
end
emu.register_frame_done(function()
  frames = frames + 1
  if frames == 1800 then
    dump("c1.bin", 0xfd100000, 0x6000)
    dump("c2.bin", 0xfd140000, 0x6000)
    dump("fee.bin", 0xfee00000, 0x20000)
    manager.machine:exit()
  end
end)
```
```bash
/Users/andreabogazzi/develop/mame/mame288 f1superb -rompath /Users/andreabogazzi/develop/mame/roms -video none -sound none -nothrottle -skip_gameinfo -seconds_to_run 40 -autoboot_script dump.lua
```
(`timeout` is not installed on this Mac; use `-seconds_to_run`.) For interactive tracing use `-debug` and watchpoints, e.g. `wpset fd104000,2000,w` and `wpset fd1024c8,4,rw`.

## Suggested next steps
1. Disassembler for the 20-bit set (Python in scratch). Use the self-test to pin down load/compare/branch semantics: the walking-bit loop must load, compare and branch to an error path; find the branch target encoding (instruction index vs. byte offset).
2. Trace the V70 side: find the code around `ffe16044` that uploads the program, then the per-frame "4 operations" sequence (writes into `fd100000-fd103fff`, start/trigger register, IRQ wait). Log register writes with watchpoints.
3. Implement a device (e.g. `jaleco_f1sb_copro`) in the driver: 8 registers, program RAM, data RAM, two IRQ outputs; first as an interpreter, validate that the self-test passes (no `LEFT/RIGHT ERROR`) and that the sprite buffers at `fee11000/100/200` get sane values.
4. Then the road layer: draw `m_extra_tilemap` using `fde00000` line info.

## User preferences
- Minimal code comments; rationale goes in commit messages.
- Show evidence first (dumps, traces) before conclusions; don't send unsolicited media.
- Don't commit or push unless asked. Never cancel a build the user is watching.
- `roms/` and `f1sbwork/` are git-ignored by the `/*/` rule in `.gitignore`; never force-add them.


## API CRASHED WHAT WE DISCOVERD AND WHERE TO RESUME FROM

Confirmed structure so far
- f = conditional branch: target = low 10 bits, condition = top 6 bits, and bit 4 of the condition flips polarity (e.g. 7540/3540 are complementary branches to the same target).
- Error exits are 3-instruction stanzas at 0x136-0x14b: 0 <code> / a fcf0 / e 4080 — load error code 0-5/0x10 into r0, move it out, halt/signal. e 4080 = error trap, e 4130 (instr 0) = likely idle/wait-for-command, e 4100 = something else (only 3 uses).
- The self-test is textbook: walking-bit register test (r6, r7), pattern test (ffff/aaaa/5555/1111), an ALU flag test matrix at 0x5a-0xb0 (each a/8 op followed by 4 conditional branches checking all four flags — this will let us derive exact flag semantics per ALU op), then a data-RAM test at 0x115 using c store/load pairs, ending with 6 0/1 (pass/fail in r6) and jump to 0.
- The d ops (shift/iterate with halving constants — divide/sqrt-like) cluster in exactly two blocks: 0x168-0x187 and 0x38c-0x3ab, probably the same math kernel twice.

Next single pass: a full annotated disassembly dumped to a file (so we never re-extract), plus field-structure analysis of the 8/9/a/b ALU args to pin down the register-field layout. Then I'll report back before going further.

## Session 3 (instruction set decoded, validated)
All work lives in `f1sbwork/` (git-ignored, persists): `extract.py`, `copro_dis.py` -> `copro.dis`, `sim.py`, and **`f1sbwork/NOTES.md` (read this first)**.
- Decoded: arg layout `fn[15:10] A[9:6] m[5:4] B[3:0]`, 16 register selectors, loads 0-5 -> s3/s7/sb/sd/se/sf, counters r6/r7, class 8 add/sub/cmp with Z/N/C(borrow)/V, class a mov/inc, class c read/write [A], branches with delay-slot bit, sense bit, flag/counter codes, call/return, `e 4080` stop with result in s0.
- `sim.py` runs the embedded self-test end to end and reaches the pass code 0x10; injected RAM fault gives error 1; wrong semantics fail. This is the ground truth to extend.
- Next: decode classes 9/b/d and the remaining 8/a fn values from the real math routines (start with the d-kernels at 168-187 and 38c-3ab), then trace the V70 side (entry points, start/trigger, result read) with `-debug` watchpoints on fd100000-fd1025ff.

## Session 4
Routine map, mul/div/shift/adc/sbc decoded, host register layout (sN at +0x2400+N*4, PC at +0x24c0, control at +0x24c8) — see `f1sbwork/NOTES.md` "Session 4". A background web-search agent was looking for commercial chips with this instruction format (result reported in chat, not yet in notes).
Next: implement the coprocessor as a device in `ms32.cpp` (interpreter of the model), hook start via +0x24c0/+0x24c8 and the FPU IRQs, then log the host command stream to decode the remaining ops (d group, a fn 011111, mul variants, routines 20d/237/38b).

## Session 5-6 (MAME core + replay debugging)
Real CPU core in `src/devices/cpu/jalfpu/`, wired into `ms32.cpp` (uncommitted). Build: `make SUBTARGET=ms32 SOURCES=src/mame/jaleco/ms32.cpp -j14` -> `./ms32`. Offline replay tools: `f1sbwork/cap.lua` + `f1sbwork/sim2.py`. See `f1sbwork/NOTES.md` sessions 5 and 6 for the host interface, decoded ops and fixes. Next: validate memory modes 5-7 and 237 outputs (FEE11200), then check sprites/road against real-hardware videos.

## Session 7-15 summary (branch f1superb-fpu, head ea32786d5ac)
Commits: 5a9735f94a1 (coprocessor core + road layer), 758e6998723 (depth mixing), 1c0edbf8dde (two road planes, backdrop rows), ea32786d5ac (IRQ9/text-layer latch, minimap sprites). User rates the game "very good". Tools: f1sbwork/f1sb_dump.lua (in-game dumps + save states), f1sbwork/regress (14-set pixel regression vs f1sbwork/ms32_base), f1sbwork/NOTES.md sessions 7-15. Known open items: road segment cap 7 in mixer, cos=1.0 saturation (Z 126 vs 128), mode 5 addressing unverified, coprocessor clock unknown, link play not emulated, priority RAM output semantics not fully decoded.

## Session 16 (IN PROGRESS): real priority RAM lookup
User wants the inferred depth rule replaced by a real priority-RAM lookup (multi-game route, no hardware available). Status and next steps: f1sbwork/NOTES.md "Session 16". Priority tables of 14 MS32 sets already collected in f1sbwork/priram_study/. Branch f1superb-fpu at ea32786d5ac, clean.
