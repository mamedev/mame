# Gals Panic 2 Protection Emulation — Working Notes

## What's Done

### Fix 1: Image address banking (COMMITTED)
- File: `src/mame/kaneko/galpani2.cpp`, cmd=0x0c handler in `galpani2_mcu_nmi2()`
- Changed image lookup to return banked addresses: `0x800000 + (offset & 0x7FFFFF)`
- Also sets bank register via `sspace.write_word(0x7c0000, bank)`
- **Result**: 2.4M unmapped reads → 19. Title screens with 3D backgrounds work.

### Fix 2: Removed debug_break on unknown commands
- All unhandled MCU commands now log and continue instead of halting

### Understanding: MCU command protocol
- Master (CPU1→MCU at 0x100020): task count + 4-byte slots [cmd, addr_hi, addr_lo, pad]
- Slave (CPU2→MCU at 0x101012): same format
- Commands 0x0a/0x02: copy between CPUs (already implemented)
- Commands 0x0c: image address lookup (fixed with banking)
- Commands 0x10, 0x6b, etc.: acknowledgment signals from CPU1 perspective

### Understanding: Sub CPU task scheduler
- **Task table** at address stored in `$1094A8` (typically `0x10121A`)
  - 16 bytes per entry, up to 32 entries (0x200 bytes)
  - Offset 0: flags byte (bit 6=active, bit 7=blocked, bit 4=timer)
  - Offset 2: task ID (word)
  - Offset 4: code pointer (longword, 0=unresolved)
  - Offset 8: param block pointer (longword)
  - Offset C: save area pointer (longword)
- **Task queue** at address stored in `$1094A4` (typically `0x10110A`)
  - 8 bytes per entry: flags(w), ID(w), param(l)
  - Circular buffer with write/read pointers
- **Save area base** at `$1094AC` (typically `0x10141A`)
- **Dispatch table** at sub CPU ROM `0x0400`
  - Maps task ID → initial code address: `table[2 + task_id]` = longword
- **Dispatcher** at `0x18A6-0x191E`
  - Scans table, skips if bit 6 clear or bit 7 set or bit 4 set
  - If code pointer = 0: resolves from dispatch table, sets save area
  - Jumps to code pointer via `jsr (A0)`
- **Task kill**: trap #13 with task ID in D1

### Understanding: 12 blocked decompression tasks
- Created during init at `0x3D40-0x3EE8` with flags=0xC000 and param=0
- Task IDs: 0x5a, 0x60, 0x66, 0x6c, 0x72, 0x78, 0x7e, 0x84, 0x8a, 0x90, 0x96, 0x9c
- Dispatch table mappings:
  - 0x5a-0x6c → B1C6 (decompression WITH ROM banking via B456)
  - 0x72-0x84 → B47C (another decompression variant)
  - 0x8a-0x9c → B69E (another variant)
- They self-block after each chunk: `ori.b #$C0, (A6,D0)` at B2EA
- MCU must continuously re-activate them (clear bit 7) on every frame

### Understanding: B1C6 decompression
- Reads param block from `($8,A6)` = A4
  - `(A4+0)`: source ROM offset (raw, converted by B456 to banked addr)
  - `(A4+4)`: destination address (bg15 framebuffer)
  - `(A4+8)`: width in columns
  - `(A4+A)`: height in rows
  - `(A4+E)`: skip rows
- Column stride in decompression: `lea ($400,A1), A1` = 0x400 bytes = 0x200 words
- Current renderer uses stride 0x800 words — **MISMATCH, needs investigation**
- B456 bank setup: bank=(offset>>23)&3, A0=0x800000+(offset&0x7FFFFF)
- Self-blocks at B2EA after each 4095-iteration chunk, resumes at B432

### Understanding: bg15 handler (task 0x001E)
- Entered via command mailbox at `$10B5DF` (values 0x80+ from MCU 0x0a copies)
- Sub-dispatch table at `0x53D2`: value-0x80 indexes into code addresses
  - 0x81 → 0x656E (bg15 image handler containing 0x6612)
- At 0x6612: reads b5d4 data (image set, col, row)
- Image lookup: table at 0xA4F4[img_set] → table at 0xA504 → image index
- Image data: table at 0x46C0 (→0x481E) → relative offsets → sub CPU ROM address
- Kills tasks 0x6C and 0x9C, waits for completion flag at `$10B6E2`

## What's NOT Working Yet

### bg15 "gal" picture decompression
- The param block values (source, dest, dimensions) are WRONG
- Our guessed params produce garbled/noisy output
- The image data from the pointer table at 0x481E gives sub CPU ROM addresses
- But the B1C6 decompression adds 0x800000 via B456 (reads from subdata ROM)
- **Mismatch**: pointer table gives ROM addr ~0x4EF8, B456 converts to 0x804EF8 (wrong memory)

### Renderer stride
- Decompression uses 0x400 byte column stride
- Renderer uses 0x800 word (0x1000 byte) column stride
- Neither matches the boot test pattern correctly
- The 0x314000 register (currently nopw) likely controls the page/offset

## Next Steps (in priority order)

1. **Trace the sub CPU bg15 handler end-to-end** to get the EXACT param values
   - Set breakpoints at 0x66C8 (image data write) and 0x670E (task kill)
   - Capture the actual values of A0 (source pointer) and what goes into the save area
   - This tells us what the decompression source should really be

2. **Determine correct renderer stride**
   - The boot test pattern at 0x1B02-0x1B62 uses known column layout
   - Cross-reference with the hardware video at https://www.youtube.com/watch?v=2b2SLFtC0uA
   - The 0x314000 register controls page flip — capture its writes

3. **Fix param block generation**
   - The source might need to come from the IMAGE LOOKUP (cmd=0x0c imlist[]) not the ROM table
   - Each of the 12 tasks needs DIFFERENT params (different tiles/sections)
   - The dimensions depend on the image format header

## Key Addresses (Sub CPU)

| Address | What |
|---------|------|
| $1094A4 | Task queue pointer → 0x10110A |
| $1094A8 | Task table pointer → 0x10121A |
| $1094AC | Save area base → 0x10141A |
| $1094B0 | Active task count |
| $1094B2 | Current task slot offset |
| $1094B8 | Current ROM bank number |
| $10B5D3 | Flip screen flag (bit 5) |
| $10B5D4 | Image set index (byte, from MCU copy) |
| $10B5D6 | Image column (byte, 0-8) |
| $10B5D7 | Image row (byte, 0-9) |
| $10B5DF | Command mailbox (byte, 0x80+ triggers action) |
| $10B6D0 | Decompression completion flags array |
| $10B6E2 | Completion flag for task 0x6C |
| $10E000 | Our param block (test area) |

## Key Addresses (Main CPU)

| Address | What |
|---------|------|
| $100020 | MCU command count byte |
| $100021+ | MCU command slots (4 bytes each) |
| $10010C | CPU1 task table pointer |
| $100108 | CPU1 task queue pointer |
| $1008D0 | bg15 page selector value (→ written to 0x314000) |

## Files in this folder

- `sub_cpu_disasm.asm` — full sub CPU ROM disassembly (0x0000-0x10000)
- `main_cpu_disasm.asm` — full main CPU ROM disassembly (0x0000-0x40000)
- `gp2_sub_*.asm` — individual routine disassemblies
- `gp2_cpu1_*.asm` — CPU1 routine disassemblies
- `gp2_bg15_full.bin` — 2MB bg15 framebuffer dump (from C++ at ~70s)
- `gp2_sub_dispatch_full.bin` — sub CPU task dispatch table dump
- `GP2_MCU_ANALYSIS.md` — earlier analysis document

## LATEST FINDING: Slave command 0x02 and bg15 stride

### Slave command 0x02 ("HELP")
At sub CPU code 0x680E (the "else" branch when b5d4 validation fails):
- The sub CPU writes **slave command 0x02** to the slave command area at $101012
- The data is a "HELP" buffer (literal "HELP" = 0x48454C50) at save_area+0x180
- The buffer contains: word A2 addr, longword 0x100, word 4
- This is the sub CPU REQUESTING the MCU to process image data
- **Currently unhandled** in the MCU NMI2 simulation!

### Correct bg15 stride: 0x200 bytes (0x100 words) per column
At 0x6978: `lea ($200,A0), A0` — column advance in the bg15 fill
- 128 longwords per column = 256 words = 512 bytes = 0x200 bytes
- 256 columns at 0x200 = 0x20000 bytes per page half
- Renderer should use: `ram[(xx * 0x100) + yy]`
- NOT 0x800 (the original) or 0x200 (my first attempt)

### Page 3 fill and page select
- The fill at 0x685A writes to page 3 (0x4C0000)
- The page select register 0x314000 (currently nopw) switches display pages
- The renderer hardcodes page 1 offset — should be dynamic based on 0x314000

## CRITICAL FINDING: Image Descriptor Script Format

The image data pointer at 0x4EF8 (stored by handler at 0x66C8) is NOT
raw pixel data — it's an **image descriptor script**:

```
FFFC 00D3 0000 0039 FFFF FFFF   ← place image 0xD3 at Y=0x39
FFFC 00D3 0000 0056 FFFF FFFF   ← place image 0xD3 at Y=0x56
00D4 FFFF FFFF                   ← image 0xD4, end
FFFC ...                         ← more entries
```

Format per entry:
- 0xFFFC = marker for "place image"
- word = image index (into imlist[] / cmd=0x0c lookup)
- 0x0000 = separator?
- word = Y position in bg15
- 0xFFFF 0xFFFF = end of row/group

The MCU needs to:
1. Read the descriptor script from the sub CPU program ROM
2. For each entry, resolve the image index via imlist[] to get subdata ROM offset
3. Set up the decompression task's param block with:
   - source = subdata ROM offset
   - destination = bg15 page address + position
   - dimensions from the image header in subdata ROM
4. Activate the task and continuously re-activate it (cooperative handshake)

This is the final missing piece for bg15 picture rendering.
