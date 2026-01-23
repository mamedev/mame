# Wersi Max1 Firmware Analysis (ic6_00.bin)

## Status: Work in Progress

## Hardware

- **CPU**: Labeled 80C32 on schematic, 16MHz crystal
- **Program ROM**: IC6 - 64KB
- **Sample ROM**: IC14 - 128KB (SAM8905 waveforms)
- **RAM**: 32KB SRAM (data space 0x0000-0x7FFF)
- **Sound**: One SAM8905 (data space 0x8000, only A0-A2 decoded)
- **Display**: 4x 7-segment + DP, segments on P1, cathodes via 74HC139
- **Keys**: 4 inputs on P3.3, scanned via 74HC139 (P3.4, P3.5)
- **Serial**: Manual says MIDI and ASCII options exist

## MCU: 8032AH Module (1991)

The chip is confirmed as an 8032AH module from 1991. However the firmware uses
SFR addresses not defined on a standard 80C32. The "module" likely includes
additional peripherals (external UART, timer logic) that appear in the SFR space
via custom decoding or are integrated into the module package.

### SFR Anomalies

1. **No standard timer init** - No writes to TMOD (0x89), TCON (0x88), TH0/TL0/TH1/TL1
2. **No standard interrupt enable** - No writes to IE (0xA8), no SETB EA/ET0/ET1/ES
3. **No standard UART access** - Zero access to SCON (0x98) or SBUF (0x99)
4. **SFR 0x91 is a serial data register** (see Serial section below)
5. Other non-standard SFR writes:
   - 0x84, 0x85: Written during init (timer/interrupt config for this module?)
   - 0xC0: Cleared in init
   - Various others (some may be instruction alignment false positives)

The Ghidra processor definition loaded has wrong SFR labels (shows USB/PCA names
from AT89C5131 or similar - these are NOT correct for this chip). Ignore Ghidra's
SFR names; use raw addresses and xref data only.

### Serial: Bit-Banged via P3.0 (RXD) and P3.1 (TXD)

Serial communication is entirely bit-banged on the GPIO pins. The standard 8051
UART (SCON/SBUF) is NOT used at all.

**Receive (RXD = P3.0, bit address 0xB0):**
- `JBC RXD, target` (opcode 10 B0) used at 18 locations
- `CLR RXD` (opcode C2 B0) at 3 locations (0x439E, 0x49F6, 0x4ABE)
- Main entry: FUN_CODE_8011 at 0x8011 - `JBC RXD, 0x7FF4`
- Called from main loop area (xref at FUN_CODE_f0e5:f154)

**Transmit (TXD = P3.1, bit address 0xB1):**
- `JBC TXD, target` (opcode 10 B1) used at **83 locations**!
- Concentrated in ranges: 0x76A1-0x8148, 0x9705-0x9760, 0xEAFD-0xED50
- No SETB TXD or CLR TXD found (unusual for output)
- JBC usage: tests pin state and clears if set, for timing/sync

**Serial state in external RAM:**
- 0x10B0-0x10B7: Serial state buffer (0x10B7 used as TX byte source)
- 0x5330: RX byte counter (incremented by patched-out ISR at 0xA522)

**Bit-rate timing:**
- Likely provided by Timer 0 ISR (vector at 0x000B -> 0xF744)
- Or by instruction-counted loops between JBC operations

### SFR 0x91: Purpose Unclear

Written at 11 locations per Ghidra xrefs. On standard 8032AH this is undefined.
Possible interpretations:
1. Module-specific output latch/register
2. State variable (writes are no-ops on real HW, used as code documentation)
3. External peripheral on the module package

**FUN_CODE_a471** adds parity to bit 7 and writes to SFR 0x91 - this might be
preparing a byte for bit-banged TX rather than a hardware UART write.

**Read at 0xA531** (`MOV [0x14], 0x91`) - reads SFR 0x91. If this is an undefined
register on real hardware, it would return 0xFF (port pull-ups). Purpose unclear.

## Memory Map

### Code Space (ROM)
- 0x0000-0xFFFF: 64KB program ROM

### Data Space (External)
- 0x0000-0x7FFF: 32KB SRAM
- 0x8000-0xFFFF: SAM8905 registers (A0-A2 only, A15 = chip select)

### External RAM Usage (partial)
- 0x10A1-0x10C4: Port state mirrors, counters
- 0x5326-0x5334: SAM control state
- 0x5541-0x5542: Command/counter pair (written by key handlers)
- 0x5543-0x5554: Initialization counters
- 0x5545: Loop counter (main loop)
- 0x5546-0x5547: Timer/config values (init: 0x93, 0x77)

## Interrupt Vectors

| Vector | Address | Target | Purpose |
|--------|---------|--------|---------|
| Reset  | 0x0000  | 0xF050 | Main entry (but init is at 0xEDE0?) |
| INT0   | 0x0003  | --     | No handler (utility code at this addr) |
| Timer0 | 0x000B  | 0xF744 | Short routine, unclear purpose |
| INT1   | 0x0013  | --     | No handler |
| Timer1 | 0x001B  | 0x01D2 | Display multiplexing |
| Serial | 0x0023  | 0xF767 | UART ISR wrapper (calls stub at 0xA521) |

**Mystery**: Timer and serial ISRs exist but NO timer/interrupt enable code found.
Interrupts may fire because the MCU variant has different reset defaults or
the non-standard SFRs (0x84, 0x85) configure timer/interrupt behavior.

## ISR Details

### Timer 1 ISR (0x01D2) - Display Multiplexing
- Writes P1 (segments a-g + DP) and P3 bits 4-5 (mux select via 74HC139)
- Called only via interrupt vector, never by LCALL
- Pattern matches keyfox10: multiplexed 7-segment scanning

### Timer 0 ISR (0xF744)
```
f744: 75 F7 E5    MOV 0xF7, #0xE5    ; Non-standard SFR!
f747: 83          MOVC A, @A+PC
f748: 95 74       SUBB A, 0x74
f74a: F6          MOV @R0, A
f74b: 22          RET                 ; Not RETI - suspicious
```
Very short, writes non-standard SFR 0xF7, ends with RET not RETI.
May not actually be a proper ISR - could be misidentified or the disassembly
alignment is off (the function is only reached via the LJMP at 0x000B).

### Serial ISR (0xF767)
```
f767: C0 E0       PUSH A
f769: C0 F0       PUSH B
f76b: C0 83       PUSH DPH
f76d: C0 82       PUSH DPL
f76f: C0 D0       PUSH PSW
f771: 75 D0 10    MOV PSW, #0x10      ; Select register bank 2
f774: 12 A5 21    LCALL 0xA521        ; Handler = just RET!
f777: D0 D0       POP PSW
f779: D0 82       POP DPL
f77b: D0 83       POP DPH
f77d: D0 F0       POP B
f77f: D0 E0       POP A
f781: 32          RETI
```
The actual handler at 0xA521 is a single RET instruction (0x22).
**Serial interrupt handler is a stub - does nothing.**

**ROM appears patched**: byte at 0xA521 was changed to 0x22 (RET) to disable
serial receive. Original byte was likely 0x00 (NOP) or the first byte of
another instruction. The code at 0xA522 decodes properly as:
```
A522: 90 53 30    MOV DPTR, #0x5330   ; RX byte counter
A525: E0          MOVX A, @DPTR       ; Read counter
A526: 04          INC A               ; Increment
A527: F0          MOVX @DPTR, A       ; Store back
A528: 22          RET
```
This increments the received-bytes counter at [0x5330], which the main loop
checks at 0xA4CB (SUBB A,#0; JNC = if counter > 0, process received data).
With the patch, [0x5330] never increments so serial RX is permanently dead.

## Main Loop (0xF050)

```
F050: LCALL 0xA142          ; Init/handler (SAM D-RAM writes)
F053: MOV DPTR, #0x5545     ; Loop counter
F056: MOVX A, @DPTR
F057: ADD A, #1
F059: MOVX @DPTR, A         ; Increment counter
F05A: JNC ...               ; Loop or continue
F05C: LCALL 0xA2E0          ; Unknown handler
F05F: [timer/countdown logic on extram 0x10C2]
F07D: LCALL 0x7517          ; Unknown
F080: MOV DPTR, #0x5541     ; Read command word
F083-F089: Read [0x5541]+[0x5542], OR together
F08A: JZ skip               ; Skip if no command pending
F08C: LCALL 0xF7E5          ; Process command (decrements [0x5542])
      ...check if command done...
F09E: CLR bit 0x04          ; Signal completion
F0A0: Store internal RAM 0x20,0x21 to [0x10C3],[0x10C4]
F0AA: SETB bit 0x11
F0AC: LCALL 0x90C3          ; Unknown handler
F0AF: [countdown on 0x5547, loop back to top]
```

## Key Handling (0x80B0 area)

Four key handlers at regular 0x20 byte offsets: 0x80B9, 0x80D9, 0x80F9, 0x8119.
Pattern for each:
```
CJNE [0x73], #3, skip      ; Check debounce count
CLR bit_N                   ; Clear key flag (bits 0x13-0x16)
MOV DPTR, #0x5541
MOV A, #2                   ; Command code = 2
MOVX @DPTR, A
INC DPTR
MOV A, #0xB4               ; Counter = 180
MOVX @DPTR, A
```

Each key writes command=2 with counter=0xB4 (180) to the command buffer,
then clears its respective flag bit. The main loop decrements the counter
each iteration via LCALL 0xF7E5.

Bits 0x13, 0x14, 0x15, 0x16 serve as key-pressed flags.
Set during init (0xEE58), cleared by respective key handler.

## SAM8905 Access

### Init (0xA142 -> 0x9561)
- Writes to address 0x8513 (SAM register 3: data byte)
- Loops 5 times with DAT_EXTMEM_5334 counter
- Sets internal RAM 0x54 = 2 each iteration
- Only observed output: D-RAM[0x00] = 0x50000 (slot 0, word 0)

### SAM Register Addresses Used
- 0x8510 (reg 0): Address register
- 0x8511 (reg 1): Data low byte
- 0x8512 (reg 2): Data mid byte
- 0x8513 (reg 3): Data high byte
- 0x8514 (reg 4): Control register

## Initialization Sequence (0xEDE0)

```
EDE0: LCALL 0xF051          ; Into main loop area (warmup?)
EDE3: LCALL 0xF051          ; Again
EDE6: Clear all ports and SFRs:
      P0=0, P1=0, P3=0, PSW=0, PCON=0
      SFR 0x84=0, 0x85=0, 0x91=0, 0xC0=0
      SP=0, DPH=0, DPL=0, B=0
EE08: Init loop: fill [0x5543] counting 0..0xF7
EE26: [0x5554] = 0xF7, [0x5553] = 0xF7
EE30: Set all ports to 0xF7 (all bits high except bit 3)
EE38: P3 = #0x10           ; Mux select = 01
EE3B: SFR 0x84 = #0x93     ; Non-standard config!
EE43: [0x5546] = 0x93
EE4A: [0x5547] = 0x77
EE4B: SFR 0x85 = #0xE5     ; Non-standard config!
EE51: SP = #0x11           ; Stack at internal RAM 0x11
EE54: P1 = #0x54           ; Display pattern (segments c,e,g?)
EE57: Set bits 0x13-0x16   ; All key flags = pressed
```

## Port Assignments

### P1 (0x90) - Display Segments
- Bits 0-6: Segments a-g (active high)
- Bit 7: Decimal point

### P3 (0xB0) - Control
- P3.0: RXD (MIDI input) - standard 8051 UART pin
- P3.1: TXD (MIDI output) - standard 8051 UART pin
- P3.2: INT0 - possibly SAM8905 interrupt (no INT0 handler found)
- P3.3: INT1 - Key input (active-low via 74HC139)
- P3.4: 74HC139 address A0 (display/key mux)
- P3.5: 74HC139 address A1 (display/key mux)
- P3.6: /WR strobe
- P3.7: /RD strobe

## P3 Access Locations

- 0x01F0: MOV P3, A (in display ISR - mux select)
- 0x98CB: MOV A, P3 (reads P3, stores to [0x10A2])
- 0x98DC: MOV P3, A (writes P3, stores to [0x10A3])
- 0x98E2: MOV A, P3; SWAP; AND (extracts mux bits)
- 0x98F9: MOV P3, A (writes P3, reads [0x10A1])
- 0xEDF4: MOV P3, A (init - clear)
- 0xEE30: MOV P3, A (init - set 0xF7)
- 0xEE38: MOV P3, #0x10 (init - final mux select)

## SFR 0xE6 Writes

Found at 3 locations:
- 0x17D6: `F5 E6 24 F2` - MOV 0xE6, A; ADD A, #0xF2
- 0xF6B8: `F5 E6 C4 F6` - MOV 0xE6, A; SWAP A; MOV @R0, A
- 0xF7A1: `F5 E6 F4 D0` - MOV 0xE6, A; CPL A; POP PSW

These are scattered across different code areas. Purpose unknown.

## Open Questions

- [x] Does the firmware use the standard 8051 UART? **No - uses SFR 0x91 instead**
- [x] What does SFR 0x91 do? **Serial data register (TX with parity, RX)**
- [x] How is serial baud rate generated? **Bit-banged timing, likely Timer 0 ISR**
- [ ] How are interrupts enabled? No IE writes found but ISRs fire
- [ ] What does SFR 0x84 = 0x93 do? Module timer/interrupt config?
- [ ] What does SFR 0x85 = 0xE5 do? Module config?
- [x] Why is the serial ISR handler stubbed out (RET at 0xA521)?
      **ROM was patched - single byte changed to 0x22 to disable serial RX.
      Original code at 0xA522 increments RX counter [0x5330].**
- [ ] What is the relationship between the 4 command buffer writes (keys)
      and the serial TX? Keys write command=2 to 0x5541 - does this trigger
      serial transmission?
- [ ] Are SFRs 0xC2-0xC5 related to SAM8905 control?
- [ ] Is the init at 0xEDE0 reached via some other path than reset vector?
- [ ] How does P3 access at 0x98CB-0x98F9 relate to key scanning vs mux control?
- [ ] What is the "MIDI" vs "ASCII" mode selection mechanism?
      Both use bit-banged P3.0/P3.1. MIDI = 31.25 kbaud 8N1.
      ASCII = 7-bit + parity (parity added at 0xA471).
      Mode likely selected by internal flag in RAM.

## MAME Driver Status

- [x] Created `src/mame/wersi/max1.cpp`
- [x] Created `src/mame/layout/max1.lay`
- [x] Added to `scripts/target/mame/muse.lua`
- [x] Added to `src/mame/muse.lst`
- [x] Boots and runs firmware
- [x] SAM8905 receives D-RAM write during init
- [ ] Display not fully working (Timer1 ISR may not fire without proper timer init)
- [ ] Serial is bit-banged on P3.0/P3.1 (not SCON/SBUF) - works in emulation
      but RX ISR is patched out (0xA521 = RET), so serial input is dead
- [ ] Sound output minimal (only 1 D-RAM write observed during init)
- [ ] Need to emulate 8032AH module's custom SFRs (0x84, 0x85, 0x91, etc.)

## ROM Checksums

```cpp
ROM_LOAD("ic6_00.bin", 0x0000, 0x10000, CRC(b8f3037f) SHA1(92e1d4dd434828e49014ff50a37c6b1d9315f540))
ROM_LOAD("ic14_00.bin", 0x0000, 0x20000, CRC(94832a29) SHA1(c151e3cab9f724825f6b0d12fcbe3a3a87c01915))
```
