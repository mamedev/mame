# Solton MS4 - Firmware Analysis

**Related documentation:**
- `WIP_ms4_memory_init.md` - Detailed INTMEM/EXTMEM memory maps, boot sequence, voice init data format
- `WIP_sam_firmware_port.md` - C port planning, struct definitions, porting task list

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
| 0x1E20+ch | All-notes-off flag per channel |
| 0x1E50+ch | Sustain pedal value per channel |
| 0x1D99 | Main loop periodic counter (reloads to 25) |
| 0x1D9A | Active Sensing timeout counter |

## Program Change -> SAM Programming Callgraph

### Overview

Program Change messages trigger loading of SAM8905 algorithms (A-RAM) and voice parameters (D-RAM).
Valid programs: **0-65** (66 total), validated via lookup table at code address 0xC8A1.
ROM pointer table at code address 0x0040 contains 2-byte big-endian pointers to program data.

### Program Data Format

Each program entry in code space starts with:

| Offset | Size | Description |
|--------|------|-------------|
| 0-7 | 8 | ASCII name (space-padded) |
| 8 | 1 | Null terminator (0x00) |
| 9 | 1 | Flags: bit 7 = complex init, bits 3:0 = SAM slot count |
| 10-11 | 2 | Algorithm reference |
| 12 | 1 | Unknown (always 0x01 in sampled programs) |
| 13-14 | 2 | Code pointer (algorithm/waveform data) |
| 15+ | var | Voice parameters, D-RAM init data |

### Callgraph

```
handle_program_change (0xC45B)
│
├── table_lookup (0xDC1C) - validate program number (table at 0xC8A1)
├── FUN_CODE_0026 - DPTR += 2*A (index into ROM pointer table at 0x0040)
│
├── [If program_data[9] bit 7 SET - complex init path:]
│   ├── voice_kill_channel (0xA785) - for ch
│   ├── voice_kill_channel (0xA785) - for ch+4
│   │   └── voice_deactivate (0xA69C) - SAM D-RAM writes to silence voice
│   ├── FUN_CODE_98ad - unknown setup
│   ├── FUN_CODE_b70b - unknown setup
│   ├── voice_assign_algorithm (0xB4BF) - allocate voice, load A-RAM
│   │   └── sam_write_aram (0xAD43) - write 32 A-RAM words (algorithm)
│   └── voice_init_slots (0x9A2D) - allocate and init D-RAM slots
│       └── sam_write_dram (0xA4BC) - write single D-RAM entry
│
└── [If program_data[9] bit 7 CLEAR - simple path:]
    ├── voice_kill_channel (0xA785) - for ch
    ├── voice_kill_channel (0xA785) - for ch+4
    └── voice_assign_algorithm (0xB4BF)
        └── sam_write_aram (0xAD43) - write 32 A-RAM words
```

### SAM Register Access Pattern

All SAM writes use P2=0x80 (Port 2 provides high address byte for MOVX, giving 0x8000+ range):

**A-RAM Write (sam_write_aram at 0xAD43):**
- Sets P2=0x80
- Loops 32 times (one full algorithm = 32 words):
  - reg 0 (0x8000): address (auto-increments)
  - reg 1 (0x8001): data low byte
  - reg 2 (0x8002): data high byte (7 bits)
  - reg 4 (0x8004): control with bit 1 SET (A-RAM select) + bit 0 (write enable)

**D-RAM Write (sam_write_dram at 0xA4BC):**
- Sets P2=0x80
- Single entry write:
  - reg 0 (0x8000): address
  - reg 1 (0x8001): data low byte
  - reg 2 (0x8002): data mid byte
  - reg 3 (0x8003): data high byte (3 bits)
  - reg 4 (0x8004): control with bit 1 CLEAR (D-RAM select) + bit 0 (write enable)

### Voice Management

- Voice pool at XRAM 0x11EE (8 entries, one per voice slot)
- Active voices form a linked list via *(voice_id + 0x7E) as next pointer
- voice_kill_channel walks the list and deactivates voices matching target channel
- voice_assign_algorithm allocates from pool, assigns algorithm slot via *(voice_id + 0x8E)
- voice_init_slots reads slot count from program_data[9] & 0x0F, allocates D-RAM slots via FUN_CODE_a9cf

### Program List (66 programs, ROM ms4_05_r1_0.bin)

| # | Name | ROM Addr | Description |
|---|------|----------|-------------|
| 0 | dpiano27 | 0x926A | Piano |
| 1 | dstringh | 0x462B | Strings |
| 2 | dcoros7 | 0x7415 | Chorus |
| 3 | dsaxs6 | 0x70A8 | Soprano Sax |
| 4 | dtrumph | 0x4672 | Trumpet |
| 5 | dpfluteh | 0x46B9 | Pan Flute |
| 6 | dbvios10 | 0x72F9 | Bass Violas |
| 7 | dclaguih | 0x4700 | Classical Guitar |
| 8 | dvibesh | 0x610E | Vibes |
| 9 | djorgh | 0x4747 | Jazz Organ |
| 10 | dchorg1h | 0x478E | Church Organ 1 |
| 11 | dmtrumph | 0x47D5 | Muted Trumpet |
| 12 | dbrassh | 0x481C | Brass |
| 13 | dmarimb | 0x4DC8 | Marimba |
| 14 | dsteelg | 0x4863 | Steel Guitar |
| 15 | dmand4 | 0x73CE | Mandolin |
| 16 | dsaxa4 | 0x48BF | Alto Sax |
| 17 | drsax4 | 0x4906 | Tenor Sax |
| 18 | dstrath | 0x8BEB | Stratocaster |
| 19 | djguit | 0x494D | Jazz Guitar |
| 20 | dharmh | 0x49A9 | Harmonica |
| 21 | dleadgh | 0x49F0 | Lead Guitar |
| 22 | dfzguith | 0x4A37 | Fuzz Guitar |
| 23 | dfhamh | 0x4A7E | Fender Ham(mond?) |
| 24 | dclikorh | 0x4AC5 | Click Organ |
| 25 | drockorh | 0x4B0C | Rock Organ |
| 26 | dstee | 0x8CA3 | Steel (variant) |
| 27 | dfguith | 0x4B53 | Finger Guitar |
| 28 | dbanjo | 0x4B9A | Banjo |
| 29 | dstratl | 0x8C47 | Stratocaster Low |
| 30 | delguirh | 0x4C50 | Electric Guitar |
| 31 | dstopg2 | 0x8CFF | Stopped Organ 2 |
| 32 | dchorg2h | 0x4C97 | Church Organ 2 |
| 33 | dstrin2h | 0x8B03 | Strings 2 |
| 34 | dcello9 | 0x7340 | Cello |
| 35 | doboe4 | 0x7387 | Oboe |
| 36 | dfluteh | 0x4CDE | Flute |
| 37 | dclarinh | 0x4D25 | Clarinet |
| 38 | dpizzah | 0x8D59 | Pizzicato |
| 39 | dstrih | 0x8BA4 | Strings (short) |
| 40 | dliptroh | 0x4E24 | Lip Trombone |
| 41 | dcrntle | 0x4E6B | Cornet (low) |
| 42 | dcrnthh | 0x6264 | Cornet (high) |
| 43 | dmuseth | 0x4F0C | Musette |
| 44 | daccordh | 0x4F53 | Accordion |
| 45 | dcassoth | 0x4F9A | Cassotto |
| 46 | ddiatonh | 0x8ABC | Diatonic |
| 47 | dzith2 | 0x8A06 | Zither |
| 48 | dfantash | 0x4FE1 | Fantasy |
| 49 | dnoisfl | 0x5028 | Noise Flute |
| 50 | dukule | 0x89AA | Ukulele |
| 51 | dcelesth | 0x506F | Celeste |
| 52 | dcor7 | 0x8963 | Chorus (variant) |
| 53 | dkalimb | 0x50B6 | Kalimba |
| 54 | dstring3 | 0x5112 | Strings 3 |
| 55 | dkvoi1 | 0x5164 | Key Voice 1 |
| 56 | dkvoi2 | 0x51B6 | Key Voice 2 |
| 57 | dlah | 0x520D | Lah (voice) |
| 58 | dbellsh | 0x5254 | Bells |
| 59 | dmagic | 0x8664 | Magic |
| 60 | dloopc2h | 0x52E2 | Loop C 2 |
| 61 | dloopchh | 0x529B | Loop Ch |
| 62 | dparadh | 0x5329 | Paradise |
| 63 | dfanh | 0x861D | Fanfare |
| 64 | dpiano7 | 0x745C | Piano 7 |
| 65 | dpiano37 | 0x9006 | Piano 37 |

Note: All names are prefixed with 'd' (likely "data" marker). Many end with 'h' (possibly "high" quality/resolution flag).

## Note On/Off -> SAM D-RAM Callgraph

### Overview

Note On/Off messages trigger voice allocation, D-RAM parameter writes, and sustain pedal management.
Voice status is tracked per-note in XRAM at 0x14D5 (128 bytes per channel, indexed by note number).

### Voice Status Byte (XRAM 0x14D5+ch*0x80+note)

| Bit | Meaning |
|-----|---------|
| 7 | Key physically held (Note On active) |
| 6 | Sustain-held (sustain pedal holding this voice) |
| 4 | Pending release (marked during sustain transitions) |
| 3:0 | Voice slot ID or other state |

### Callgraph

```
handle_note_on_off (0xB75F)
│
├── [Note On (velocity > 0):]
│   ├── Channel mask check (channels 0-3 only)
│   ├── Note range split: if note > threshold (table at 0xD383), use ch+4 (second layer)
│   ├── Voice table lookup (XRAM 0x14D5+ch*0x80+note)
│   ├── Set status byte = 0x80 | slot_id (key held)
│   └── voice_trigger_note (0xA834)
│       ├── Read 10 bytes from per-channel state (XRAM 0x1200+ch*0x0C)
│       ├── Allocate D-RAM slots via FUN_CODE_a9cf
│       └── dram_param_processor (0xAA9A) - for each slot
│           ├── Read 5 init bytes from program data
│           └── Loop: read parameter descriptors, dispatch via bits[5:3]:
│               ├── Type 0 (0xAD8F): frequency/pitch
│               ├── Type 1 (0xADBD): complex (596 bytes, envelope setup)
│               ├── Type 2 (0xB030): filter/amplitude
│               ├── Type 3 (0xB222): param type 3
│               ├── Type 4 (0xB278): param type 4
│               ├── Type 5 (0xB2D2): param type 5
│               └── Type 6 (0xB2CF): param type 6
│                   └── sam_write_dram (0xA4BC)
│
├── [Note Off (velocity = 0):]
│   └── handle_note_off_sustain (0xBC31)
│       ├── Check sustain pedal (XRAM 0x1E50[ch] bit 0x40)
│       ├── If sustain ON: set bit 0x10 (sustained hold), clear bit 0x80
│       └── If sustain OFF and (status & 0x50)==0:
│           └── voice_trigger_note (0xA834) - release parameters
│
├── [Sustain Pedal Off (CC#64 < 64):]
│   └── cc_sustain_pedal (0xC390)
│       └── handle_sustain_release (0xBDD5)
│           ├── Find voices with bit 0x10 set
│           ├── If bit 0x40 not set: voice_trigger_note (release)
│           └── Clear bit 0x10
│
└── [All Notes Off (CC#123):]
    └── handle_all_notes_off (0xBAE1)
        ├── Scan all 128 notes in voice table
        ├── For active voices: clear upper bits (mask 0x2F)
        ├── Call voice_trigger_note for each
        └── If 0x1E20 flag set: voice_kill_channel (0xA785)
```

### Per-Channel State (XRAM 0x1200 + ch*0x0C)

Each channel has 12 bytes of state used by voice_trigger_note:

| Offset | Purpose |
|--------|---------|
| 0x00 | Channel flags |
| 0x01 | Program/voice mode |
| 0x02 | Volume (CC#7 value, written by cc_volume_handler) |
| 0x07 | Note/velocity data pointer high |
| 0x08 | Note/velocity data pointer low |
| 0x0A | Volume copy (also written by cc_volume_handler) |

## Envelope and Modulation Update System

### Architecture

The MS4 uses a software envelope/LFO engine running on the 80C52, which periodically updates SAM D-RAM parameters. The system has three layers:

1. **Timer 1 ISR** (0xD440): Fires every ~5.5ms, increments two tick counters:
   - **INTMEM 0x17 (Bank2 R7)**: Fast tick - triggers periodic_voice_update
   - **INTMEM 0x16 (Bank2 R6)**: Slow tick - triggers Active Sense timeout
2. **Main Loop periodic call** (every 2 fast ticks ≈ 11ms): Calls `periodic_voice_update` (0x9BA7)
3. **CC#7 immediate update**: `envelope_tick_volume` (0xA403) re-evaluates envelopes on volume change

**Timer 1 Configuration:**
```
Mode:       Timer 1, Mode 1 (16-bit)
Reload:     TH1=0xE3, TL1=0x5B (0xE35B = 58203)
Period:     (65536 - 58203) × 12 / 16MHz = 5.5ms
ISR:        Switches to Bank 2, reloads timer, increments R6 and R7
```

### Main Loop Structure (MAINLOOP at 0xDA30)

```
Init:
  - Set up SFRs (T2CON=0x34, SCON=0x5C, IE=0x98, TMOD=0x11)
  - Enable Timer 1 & Serial interrupts
  - Call init functions (0xB70B, 0x98AD)

Loop:
  1. Call SERIAL_HANDLER (process one MIDI byte)
  2. Check INTMEM 0x17 (Bank2 R7, fast tick counter)
  3. If fast ticks >= 2:
     a. Call periodic_voice_update (0x9BA7) -- ENVELOPE/LFO TICK
     b. Decrement XRAM 0x1D99 (Active Sense TX counter)
     c. If counter reaches 0:
        - Reset counter to 0x19 (25)
        - Send MIDI Active Sense (0xFE)
     d. Subtract 2 from fast tick counter
  4. Check INTMEM 0x16 (Bank2 R6, slow tick counter)
  5. If slow ticks >= 2:
     a. Reset slow tick counter to 0
     b. Decrement active_sense_timer (MIDI RX timeout)
     c. If timeout reached: call midi_panic_all_channels (0xBDBC)
     d. Increment XRAM 0x1D96, when reaches 100: clear XRAM 0x12CD
  6. Loop back to step 1
```

### periodic_voice_update (0x9BA7) - The Core Update

Called every ~11ms. Structure:

**Outer loop**: Walks active voice linked list (head at INTMEM 0x54, next at voice+0x7E)

For each voice:
- Sets P2 = voice slot (provides high byte for XRAM access to per-voice state)
- Reads flags from voice XRAM 0xFB
- Gets channel from voice XRAM 0xFC upper nibble
- Computes mod wheel contribution: XRAM 0x1183 (mod wheel) * XRAM 0x1184+ch (sensitivity)

**Inner loop 1** (Envelope/LFO state machine, 7 blocks of 16 bytes at voice XRAM 0x00-0x6F):

Each 16-byte parameter state block:

| Offset | Purpose |
|--------|---------|
| 0x00-0x01 | Envelope segment pointer (code space) |
| 0x02 | Status: bit 7=LFO active, bit 6=envelope active, bits 2:0=waveform type |
| 0x03 | Modulation flags: bit 7=pitch bend, bit 6=mod wheel |
| 0x04 | LFO rate increment |
| 0x05 | LFO rate/phase step |
| 0x06 | LFO amplitude |
| 0x07 | LFO output value (computed result) |
| 0x08-0x09 | Envelope current position (16-bit) |
| 0x0A-0x0B | Phase accumulator (16-bit, for LFO) |
| 0x0C | Envelope flags: bit 7=active |
| 0x0D | Envelope target value |
| 0x0E | Envelope rate/mode: bits 2:0 = end action (0=next seg, 1=loop, 2=sustain, 3=stop, 4=off) |
| 0x0F | Envelope rate multiplier |

LFO Waveform types (switch on byte+2 & 7):
- 0, 5, 6, 7: Sine (64-entry table at code 0x9833)
- 1: Identity (ramp up)
- 2: Inverted (ramp down)
- 3: Square
- 4: Noise (LFSR: val*3 + 0x43)

**Inner loop 2** (Parameter output and SAM write):

Iterates parameter descriptor list (IRAM 0x70+, same as envelope_tick_volume):
- 0xFF = end of list -> move to next voice in linked list
- 0x0F = skip
- Otherwise: compute IRAM state address = (desc_low_nibble * 8) | 0x80
- Check status bits [5:3]:
  - 0x38 = done/idle -> skip
  - 0x10 = "volume envelope" type -> calls FUN_CODE_a18f (SAM D-RAM write)
  - Other = modulation type:
    - Reads LFO output from state block
    - Applies pitch bend (from XRAM 0x1194/0x1195 per channel)
    - Applies mod wheel (DAT_INTMEM_41/42)
    - Saturates to +/-127
    - Calls modulation_write_dram (0x9FCD) to write to SAM D-RAM

### CC#7 Volume -> Immediate Envelope Update

```
cc_handler_dual_layer (0xC7A3)
└── cc_dispatch (0xC1A0)
    └── cc_switch_handler (0xC2BA) [case index 8 = CC#7]
        └── cc_volume_handler (0xC34C)
            ├── Store value to XRAM 0x1202[ch] and 0x120A[ch]
            └── envelope_tick_volume (0xA403)
                ├── Walk voice linked list for channel
                ├── For each voice: iterate parameter descriptors
                ├── Check envelope state bits[5:3] == 0x10
                └── envelope_write_dram (0xA471)
                    ├── Compute parameter from rate * scale
                    └── sam_write_dram (0xA4BC)
```

### CC Dispatch Jump Table (0xC833, 16 entries after subtracting 5 from CC index)

| Entry | CC Index | CC# | Handler Address | Function |
|-------|----------|-----|-----------------|----------|
| 0 | 5 | 1 | 0xC2CA | Modulation Wheel |
| 1 | 6 | 5 | 0xC2D8 | Portamento Time |
| 2 | 7 | 65 | 0xC303 | Portamento On/Off |
| 3 | 8 | 7 | 0xC34C | Volume (triggers envelope update) |
| 4 | 9 | 10 | 0xC374 | Pan |
| 5 | 10 | 64 | 0xC390 | Sustain Pedal |
| 6 | 11 | 66 | 0xC3C4 | Sostenuto |
| 7 | 12 | 67 | 0xC3FE | Soft Pedal |
| 8-15 | 13-20 | 123-127+ | 0xC41F+ | Channel Mode messages |

### Key RAM Addresses (Envelope/Modulation)

See `WIP_ms4_memory_init.md` for comprehensive INTMEM/EXTMEM memory maps.

| Address | Purpose |
|---------|---------|
| INTMEM 0x16 | Timer 1 slow tick counter (Bank2 R6, Active Sense timeout) |
| INTMEM 0x17 | Timer 1 fast tick counter (Bank2 R7, voice update) |
| INTMEM 0x34 | Current channel being processed |
| INTMEM 0x38 | Current parameter descriptor |
| INTMEM 0x3A | Current voice slot ID |
| INTMEM 0x3B | Current IRAM state block address |
| INTMEM 0x41-42 | Mod wheel contribution (computed per voice) |
| INTMEM 0x4F | Parameter list iterator |
| INTMEM 0x54 | Active voice linked list head |
| XRAM 0x1183 | Mod wheel value (signed) |
| XRAM 0x1184+ch | Mod wheel sensitivity per channel |
| XRAM 0x1194/95+ch*2 | Pitch bend value per channel (16-bit) |
| XRAM 0x1E50+ch | Sustain pedal value per channel |
| XRAM voice+0x7E | Next voice pointer (linked list) |
| XRAM voice+0x00-0x6F | 7 parameter state blocks (16 bytes each) |
| XRAM voice+0xFB | Voice flags |
| XRAM voice+0xFC | Channel (upper nibble) |

## Key Function Addresses

| Address | Name | Purpose |
|---------|------|---------|
| 0xB630 | ISR_UART_HANDLER | UART ISR: buffer RX, dequeue TX |
| 0xC635 | SERIAL_HANDLER | Main loop MIDI parser/dispatcher |
| 0xB75F | handle_note_on_off | Note On/Off processing |
| 0xC1A0 | cc_dispatch | CC lookup and dispatch |
| 0xC45B | handle_program_change | Program Change with ROM table lookup |
| 0x9AB0 | handle_pitch_bend | Pitch Bend processing (dual-layer) |
| 0xB59B | sysex_voice_enable | SysEx voice mask processing |
| 0xBDBC | midi_panic_all_channels | Buffer overflow panic handler |
| 0xA785 | voice_kill_channel | Kill all active voices on a channel |
| 0xA69C | voice_deactivate | Deactivate single voice (SAM D-RAM silence) |
| 0xB4BF | voice_assign_algorithm | Allocate voice slot, load SAM A-RAM algorithm |
| 0xAD43 | sam_write_aram | Write 32-word algorithm to SAM A-RAM |
| 0x9A2D | voice_init_slots | Allocate and initialize SAM D-RAM voice slots |
| 0xA4BC | sam_write_dram | Write single D-RAM entry to SAM |
| 0x9BA7 | periodic_voice_update | Main loop periodic envelope/LFO/modulation update (~11ms) |
| 0x9FCD | modulation_write_dram | Write modulation parameter to SAM D-RAM |
| 0xA403 | envelope_tick_volume | CC#7 Volume immediate envelope update |
| 0xA471 | envelope_write_dram | Compute and write single envelope parameter to SAM D-RAM |
| 0xA834 | voice_trigger_note | Voice trigger (Note On/Off, sustain release) |
| 0xAA9A | dram_param_processor | D-RAM parameter processor (7-type switch on bits[5:3]) |
| 0xBAE1 | handle_all_notes_off | All Notes Off: scan + retrigger all active voices |
| 0xBC31 | handle_note_off_sustain | Note Off with sustain pedal check |
| 0xBDD5 | handle_sustain_release | Release voices held by sustain pedal |
| 0xBF18 | handle_sustain_toggle | Sustain pedal on/off transition handler |
| 0xC1A0 | cc_dispatch | CC lookup table (0xD405) and dispatch |
| 0xC2BA | cc_switch_handler | CC handler switch (16-entry jump table at 0xC833) |
| 0xC34C | cc_volume_handler | CC#7 handler: store + envelope update |
| 0xC390 | cc_sustain_pedal | CC#64 handler: sustain pedal on/off |
| 0xC7A3 | cc_handler_dual_layer | CC dual-layer wrapper (ch + ch+4) |
| 0xD417 | find_active_voice | Search voice table backward for active entry |
| 0xD440 | Timer1_ISR | Timer 1 ISR: reload + increment tick counter (INTMEM 0x17) |
| 0xDC1C | table_lookup | Generic byte-search in code space |
| 0xDC9C | load_dptr_from_xram | Load DPTR from 2-byte XRAM pointer |
| 0xDCB3 | add_a_to_dptr | Utility: DPTR += A (indexed array access) |
| 0x0026 | dptr_add_2a | Utility: DPTR += 2*A (word-indexed table access) |
| 0x0003 | indexed_array_access | Utility: indexed access to XRAM arrays |

## Implementation Steps

- [x] Create `src/mame/hohner/ms4.cpp` (used I80C32 instead of I80C52 - ROM-less variant, still has Timer 2)
- [x] Add to `scripts/target/mame/muse.lua`
- [x] Add to `src/mame/muse.lst`
- [x] Build with `make -j8 SUBTARGET=muse REGENIE=1`
- [x] Test with `./mamemuse ms4 -rompath /home/jeff/bastel/roms/hohner -oslog`
- [x] Firmware boots, SAM init visible, no unmapped access errors
- [x] Document MIDI implementation via Ghidra analysis
- [x] Analyze program range (0-65) and document callgraph from MIDI to SAM programming
- [x] Name and comment all SAM-related functions in Ghidra (voice_kill_channel, voice_deactivate, voice_assign_algorithm, sam_write_aram, voice_init_slots, sam_write_dram)
- [x] Trace handle_note_on_off callgraph to SAM D-RAM writes
- [x] Identify voice status byte semantics (bits 7,6,4 = key held, sustain, pending)
- [x] Trace sustain pedal handling (CC#64 -> handle_sustain_release/handle_sustain_toggle)
- [x] Identify periodic envelope/modulation update function (periodic_voice_update at 0x9BA7)
- [x] Document main loop structure (Timer 1 ticks -> periodic_voice_update every ~11ms)
- [x] Document LFO waveform types (sine, ramp, square, noise via LFSR)
- [x] Document per-voice parameter state block format (16 bytes: envelope + LFO state)
- [x] Document CC#7 Volume immediate envelope update path
- [x] Name and comment all note/envelope/CC functions in Ghidra (17 functions total)
