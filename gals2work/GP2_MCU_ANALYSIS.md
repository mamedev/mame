# Gals Panic 2 — MCU Protection Analysis

> Historical analysis. See `IMAGE_FORMAT.md` for the verified image decoding
> results. The hard-coded `imlist` matches the Asia background ROM, not the
> Japanese one. Address banking alone cannot correct this regional mismatch.

## Architecture

```
CPU1 (maincpu, 68000)          MCU (PISCES, uPD78324)         CPU2 (sub, 68000)
  main game + sound      ←→    mediator, 32KB ROM (UNDUMPED)  ←→   backgrounds
  RAM: 0x100000-0x10FFFF        passes messages between CPUs       RAM: 0x100000-0x13FFFF
                                                                    bg15 fb: 0x400000-0x5FFFFF
                                                                    banked ROM: 0x800000-0xFFFFFF
```

## MCU Command Protocol

### Master side (CPU1 → MCU, via NMI1 at 0x680001)
- Task list header: byte at `0x100020` = number of task bytes
- Each slot = 4 bytes at `0x100021 + slot`: `[cmd_byte, addr_hi, addr_lo, pad]`
- Parameters at `0x100000 | addr_word`
- Done: MCU writes `0xFFFF` to word at param_address

### Slave side (CPU2 → MCU, via NMI2 at 0x780001)
- Task list header: byte at `0x101012`
- Same slot format at `0x101013 + slot`

## Commands Observed (10 min attract mode, galpani2j)

### Handled Commands

| Cmd  | Side   | Count | Description |
|------|--------|-------|-------------|
| 0x0a | Master | 98    | Copy CPU1 RAM → CPU2 RAM. params: w[1]=src, w[3]=dst, w[4]=size |
| 0x02 | Master | 0     | Copy CPU2 RAM → CPU1 RAM (gp2se only) |
| 0x0c | Slave  | 89    | Image address lookup: w[0]=index → dword at w[1:2] = ROM offset |

### Unhandled Commands

| Cmd  | Side   | Count | Slot | Address  | Description |
|------|--------|-------|------|----------|-------------|
| 0x10 | Master | 8     | 00   | 0x102148 | "Clear/display gal" — background image ops |
| 0x6b | Master | 8     | 08   | 0x102158 | "Display changed monster" — monster/sprite ops |

Always appear **in pairs** (0x6b then 0x10).

### cmd=0x10 parameter blocks

```
#  w0    w1    w2    w3    w4    w5    w6    w7
1  0000  0000  0000  0000  0000  0000  0000  0000   ← init/clear
2  0000  0402  00ff  0000  0000  0000  0000  0000   ← early attract
4  0000  0400  0030  1400  0010  9414  0003  d1d6   ← different variant
5+ 0000  0402  00ff  0000  0010  9414  0003  d1d6   ← steady state
```

### cmd=0x6b parameter blocks

```
#  w0    w1    w2    w3    w4    w5    w6    w7
1  0000  0000  0000  0000  0000  0000  0000  0000
2  0000  2148  0000  b5d4  0007  0000  0000  0000   ← looks like copy params!
4  0000  0420  0030  1420  0010  9434  0000  0000
5+ 0000  2148  0000  b5d4  0007  9434  0000  0000
```

## Sub CPU Image Decompression

### Bank setup routine at `0xB456`
Converts raw ROM offset → banked CPU address:
```asm
B456: move.l D4, D3        ; D3 = ROM offset
B458: swap D3              ; upper 16 bits
B45A: lsr.w #7, D3         ; bank = bits[23:22]
B45C: andi.w #3, D3        ; 4 banks (0-3)
B460: move.w D3, $7C0000   ; select ROM bank register
B46C: andi.l #$7FFFFF, D4  ; offset within 8MB bank
B472: movea.l D4, A0
B474: adda.l #$800000, A0  ; A0 = 0x800000 + offset_in_bank
```

### Address translation at `0xB186`
```asm
B186: btst #31, D0         ; if bit 31 set → bank calc for bg15 framebuffer dest
B18A: bne  B192            ; 
B18E: movea.l D0, A0       ; bit 31 clear → use raw address (for RAM sources)
B190: rts
```

### Two decompression types

| Routine | Bank handling | Source addr setup | Used for |
|---------|--------------|-------------------|----------|
| B0E0    | NO           | A0 = (A4) raw     | Small data already in CPU address space |
| B994    | NO           | A0 = (A4) raw     | Similar |
| BC4A    | NO           | A0 = (A4) raw     | Similar |
| B1CE    | YES (B456)   | A0 = banked addr   | ROM image decompression with bank crossing |

## ROOT CAUSE: Why No Backgrounds

1. **Image lookup (0x0c) returns raw ROM offsets** (e.g., `0x001A615C` for image 0x33)
2. **Sub CPU decompression at B0E0/B994/BC4A** uses these offsets as direct CPU addresses
3. **Sub CPU memory map gap**: `0x140000–0x3FFFFF` is UNMAPPED
4. **Result**: millions of reads from unmapped addresses, decompression produces garbage

The hot unmapped reads correlate exactly with image lookup offsets:
- Image 0x33 → ROM `0x001A615C` → reads at `0x1A6162+`
- Image 0x90 → ROM `0x001EDBB8` → reads at `0x1EE474+`
- Image 0xB4 → ROM `0x002C6816` → reads at `0x2C681C+`

## What Commands 0x10 and 0x6b Likely Do

The MCU should:
1. Read image parameters from CPU1 shared RAM
2. **Convert ROM offsets to banked CPU2 addresses** (bank select + `0x800000 + offset`)
3. Set up decompression task parameters in CPU2's task scheduler table at `$1094A8`
4. Trigger CPU2 to start the decompression task

Without handling these commands, the sub CPU either:
- Runs decompression with stale/wrong parameters → unmapped reads
- Never starts the background rendering tasks → no backgrounds

## Progress Made

### Fix 1: Image address banking (WORKING)
Changed cmd=0x0c to return banked addresses (`0x800000 + offset%0x800000`) with bank
selection, instead of raw ROM offsets.
- **Result**: Eliminated 2.4M unmapped reads → 19 (just startup probes)
- Title screen 3D backgrounds (bg8 layers) now render correctly
- Attract mode runs: title screens, demo gameplay, copyright screens all visible

### Fix 2: cmd=0x6b as copy (WORKING)
Implemented cmd=0x6b as CPU1→CPU2 memory copy (same format as cmd=0x0a).
- Confirmed: first instance is all-zeros (init), later instances copy display params

### Fix 3: cmd=0x10 task queue (INCOMPLETE)
Attempted to queue tasks on sub CPU's task scheduler at $1094A4.
- Task WAS queued but format was wrong (flags 0x0402 doesn't set active bits 6-7)
- bg15 framebuffer remains COMPLETELY EMPTY

## Current State

| What works | What doesn't |
|-----------|-------------|
| Title screen backgrounds (bg8) | bg15 "gal" picture (always empty/grey) |
| Attract mode gameplay demo runs | The revealed picture during gameplay is solid color |
| Sprites, borders, scores | gp2se has unhandled cmd=0x17 |
| Image lookups with proper banking | |
| No unmapped memory reads | |

## Open Questions

1. **How does the bg15 decompression task get created?** The sub CPU code at
   0x6612 that processes b5d4 image data is inside a task that's never instantiated.
   The task scheduler uses a queue at $1094A4 and table at $1094A8, but the exact
   entry format for creating NEW tasks (initial code pointer) is unclear.

2. **What does cmd=0x10 actually do?** It's not a simple copy or task queue.
   Parameters: w[1]=0x0402, w[2]=0x00ff. Might be a trigger to the sub CPU's
   own task creation mechanism.

3. **Sub CPU task scheduler architecture**: cooperative/coroutine-based.
   Tasks yield with `bra $55E0` (clears entry and returns). New tasks created
   by queue entries with flags word (high byte bit 6 = active), task ID, params.

## Next Steps

1. **Interactive debugging**: Use MAME debugger with breakpoints at sub CPU
   task creation code to trace how tasks are normally created during init
2. **Check trap instructions**: Sub CPU uses `trap #13` at 0x64F6 — might be
   the mechanism for creating tasks
3. **Try different ROM set**: gp2quiz is noted as "has demo" — compare behavior
4. **Look at what the KANEKO check counter ($100006) triggers** — the sub CPU
   increments this counter, and it might gate initialization phases
