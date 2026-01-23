# Solton MS4 - Firmware Analysis

## Hardware

- **CPU**: 80C52 (I80C32 in MAME - ROM-less variant with Timer 2)
- **Crystal**: 16 MHz
- **Program ROM**: ms4_05_r1_0.bin (64KB) - CRC32: b02cd104
- **RAM**: 8KB SRAM (data space 0x0000-0x1FFF)
- **Sound**: SAM8905 at data space 0x8000 (A0-A2 decoded, A15 chip select)
- **MIDI**: Standard 8051 UART Mode 1, 31250 baud via Timer 2 (T2CON=0x34, RCAP2=0xFFF0)

## MAME Driver

- Source: `src/mame/hohner/ms4.cpp`
- Run: `./mamemuse ms4 -rompath /home/jeff/bastel/roms/hohner -oslog`

## Boot Sequence

1. Reset vector -> 0xDCBC
2. Init at 0xDA24: clears IE, writes 0x40 to SAM control (0x8004)
3. Clears all external RAM (port 2 cycling through pages)
4. Clears SAM D-RAM (all 256 addresses, ctrl=0x05)
5. Programs SAM A-RAM algorithms (ctrl=0x03, at PC=D874)
6. Enters main loop

## MIDI Implementation

### ISR Chain

- 0x0023: UART interrupt vector
- 0xDC73: ISR stub (PUSH/LCALL/POP)
- 0xB630: **ISR_UART_HANDLER** - receives bytes into circular buffer

### ISR_UART_HANDLER (0xB630)

- Checks RI flag (SFR 0x98 bit 0), reads SBUF (SFR 0x99)
- Special case: byte == 0xFE (Active Sensing) -> reset timeout at 0x1D9A to 0x32
- Filters: bytes >= 0xF8 (real-time messages) -> discarded
- Circular buffer: 255 bytes at 0x13D4, write pointer at 0x12CF, count at 0x12CE
- Buffer overflow: sets bit 0x0E flag
- TX side: dequeues from output buffer at 0x12D5, count at 0x12D1, read pointer at 0x12D3

### SERIAL_HANDLER (0xC635) - Main Loop Parser

Called from main loop. Reads bytes from circular buffer and processes MIDI messages.

#### Channel Configuration

- **Base channel**: stored at RAM 0x12C0
- **Accepted channels**: 4 consecutive channels (base, base+1, base+2, base+3)
- **OMNI mode**: bit 0x0C flag - when set, all channels treated as channel 0
- **Channel offset**: `(received_channel - base_channel) & 0x0F`, must be < 4

#### Running Status

- Running status stored at 0x12CC
- Properly handles 1-byte messages (0xC0-0xDF: Program Change, Channel Aftertouch)
- 2-byte messages (Note, CC, Pitch Bend) wait for second data byte via flag 0x0F

### Message Dispatch (Jump Table at 0xC869)

| Status | Message | Address | Handler | Notes |
|--------|---------|---------|---------|-------|
| 0x8n | Note Off | 0xC78A | Sets vel=0, falls through to Note On | |
| 0x9n | Note On | 0xC78F | LCALL 0xB75F | ch->0x08, note->0x09, vel->0x0A |
| 0xAn | Poly Aftertouch | 0xC7A1 | (ignored) | SJMP to end |
| 0xBn | Control Change | 0xC7A3 | LCALL 0xC1A0 | Dual-layer: also processes ch+4 (except CC 96/97) |
| 0xCn | Program Change | 0xC7D2 | LCALL 0xC45B | Looks up ROM table at 0x0040 |
| 0xDn | Channel Aftertouch | 0xC7E5 | (inline) | Stores (value-64) at 0x11C8[ch] and 0x11C4[ch] |
| 0xEn | Pitch Bend | 0xC801 | LCALL 0x9AB0 | Dual-layer: also processes ch+4 |
| 0xFn | System | 0xC821 | (loop back) | Handled by SysEx state machine or discarded |

### Dual-Layer Processing

Control Change and Pitch Bend are processed on **both** the received channel AND channel+4:
- Channel 0 -> processes on channels 0 and 4
- Channel 1 -> processes on channels 1 and 5
- Channel 2 -> processes on channels 2 and 6
- Channel 3 -> processes on channels 3 and 7

Exception: CC 96 (Data Increment) and CC 97 (Data Decrement) are only processed on the received channel.

This suggests an 8-voice architecture with two layers per MIDI channel.

### Recognized Control Changes

CC lookup table at code address 0xD405 (18 entries):

| Slot | CC# | Name | Category |
|------|-----|------|----------|
| 0 | 6 | Data Entry MSB | RPN |
| 1 | 96 | Data Increment | RPN |
| 2 | 97 | Data Decrement | RPN |
| 3 | 100 | RPN LSB | RPN |
| 4 | 101 | RPN MSB | RPN |
| 5 | 1 | Modulation Wheel | Performance |
| 6 | 5 | Portamento Time | Performance |
| 7 | 65 | Portamento On/Off | Performance |
| 8 | 7 | Volume | Performance |
| 9 | 10 | Pan | Performance |
| 10 | 64 | Sustain Pedal | Pedals |
| 11 | 66 | Sostenuto Pedal | Pedals |
| 12 | 67 | Soft Pedal | Pedals |
| 13 | 123 | All Notes Off | Channel Mode |
| 14 | 124 | Omni Off | Channel Mode |
| 15 | 125 | Omni On | Channel Mode |
| 16 | 126 | Mono On | Channel Mode |
| 17 | 127 | Poly On | Channel Mode |

**Slots 0-4** (RPN): Dispatched via jump table at 0xC824 to individual handlers at 0xC1CE, 0xC215, 0xC261, 0xC2AB, 0xC2B3.

**Slots 5-17** (Performance/Mode): Handled at 0xC2BA.

Unrecognized CC numbers are silently ignored (lookup returns 0xFF).

### SysEx Implementation

State machine (state variable at 0x12C3):

| State | Address | Action |
|-------|---------|--------|
| 0 | 0xC69D | Idle - fall through to channel msg processing |
| 1 | 0xC69F | Check manufacturer ID == 0x31 (Solton/Hohner). Match->state 2, else->state 4 |
| 2 | 0xC6B6 | Store data byte 1 at 0x1D18, state=3 |
| 3 | 0xC6C6 | Store data byte 2 at 0x1D19, state=4, call sysex_voice_enable (0xB59B) |
| 4 | 0xC6D9 | Ignore remaining bytes until next status byte |

**SysEx Format**: `F0 31 <mask1> <mask2> F7`

- Manufacturer ID: 0x31 (Solton/Hohner)
- mask1 and mask2: must have bits 6-7 clear (values 0-63)
- Bits 0-4 of each mask control enabling/disabling of 5 voices
- Enabled voices get 0xC0 written to tables at 0x1D1A[voice] and 0x1D20[voice]
- Disabled voices get 0x00

### Buffer Overflow Handler (0xBDBC)

Called `midi_panic_all_channels` - when the RX buffer overflows:
- Resets buffer pointers and count
- Loops through 7 channels calling 0xBAE1 for each (all-notes-off equivalent)

## Key Differences from Max1

| Feature | Max1 | MS4 |
|---------|------|-----|
| CPU | 80C32 | 80C52 (Timer 2) |
| UART | Bit-banged P3.0/P3.1 | Standard SCON/SBUF |
| Baud gen | N/A (bit-bang) | Timer 2, RCAP2=0xFFF0 |
| RAM | 32KB | 8KB |
| Display | 4x7-seg | None |
| Keys | 4 buttons | None |
| Waveform ROM | 128KB (IC14) | Not connected (minimal) |
| ISR status | Serial ISR patched out | All ISRs functional |

## Key RAM Addresses

| Address | Purpose |
|---------|---------|
| 0x12C0 | Configured MIDI base channel |
| 0x12C3 | SysEx parser state (0-4) |
| 0x12CC | Running status byte |
| 0x12CE | RX buffer byte count |
| 0x12CF | RX buffer write pointer |
| 0x12D0 | RX buffer read pointer |
| 0x12D1 | TX buffer byte count |
| 0x12D3 | TX buffer read pointer |
| 0x12D4 | Current byte being processed |
| 0x13D4 | RX circular buffer (255 bytes) |
| 0x12D5 | TX circular buffer |
| 0x11C4 | Channel Aftertouch storage (per channel) |
| 0x11C8 | Channel Aftertouch storage 2 (per channel) |
| 0x14D3 | Program data pointer (2 bytes) |
| 0x1D15-0x1D17 | Mode flags for CC extended handler |
| 0x1D18 | SysEx mask1 / voice enable pattern 1 |
| 0x1D19 | SysEx mask2 / voice enable pattern 2 |
| 0x1D1A | Voice enable table 1 (5 entries, 0xC0=on, 0x00=off) |
| 0x1D20 | Voice enable table 2 (5 entries) |
| 0x1D9A | Active Sensing timeout counter |

## Key Function Addresses

| Address | Name | Purpose |
|---------|------|---------|
| 0xB630 | ISR_UART_HANDLER | UART ISR: buffer RX, dequeue TX |
| 0xC635 | SERIAL_HANDLER | Main loop MIDI parser/dispatcher |
| 0xB75F | handle_note_on_off | Note On/Off processing |
| 0xC1A0 | handle_control_change | CC lookup and dispatch |
| 0xC45B | handle_program_change | Program Change with ROM table lookup |
| 0x9AB0 | handle_pitch_bend | Pitch Bend processing (dual-layer) |
| 0xB59B | sysex_voice_enable | SysEx voice mask processing |
| 0xBDBC | midi_panic_all_channels | Buffer overflow panic handler |
| 0xDC1C | table_lookup | Generic byte-search in code space |
| 0xDCB3 | add_a_to_dptr | Utility: DPTR += A (indexed array access) |

## Implementation Steps

- [x] Create `src/mame/hohner/ms4.cpp` (used I80C32 instead of I80C52 - ROM-less variant, still has Timer 2)
- [x] Add to `scripts/target/mame/muse.lua`
- [x] Add to `src/mame/muse.lst`
- [x] Build with `make -j8 SUBTARGET=muse REGENIE=1`
- [x] Test with `./mamemuse ms4 -rompath /home/jeff/bastel/roms/hohner -oslog`
- [x] Firmware boots, SAM init visible, no unmapped access errors
- [x] Document MIDI implementation via Ghidra analysis
