# Keyfox10 Expander Mode Analysis and ROM Patch

## Overview

The Hohner Keyfox10 firmware operates in two modes:
- **Keyboard Mode** (mode=0): Program numbers are remapped through lookup tables for the built-in keyboard interface
- **Expander Mode** (mode=2): MIDI program numbers are used directly (1:1 mapping)

In keyboard mode, MIDI program 1 maps to "Upper Off" (no sound), making direct testing of internal synthesis algorithms difficult.

## Key Memory Locations

| Address | Size | Purpose |
|---------|------|---------|
| 0x1FFF | byte | Mode variable: 0=keyboard, 1=special, 2=expander |
| 0x1FFE | byte | Mode sub-state |
| 0x1FFD | byte | Mode sub-state |
| 0x1FFC | byte | Mode flag |
| bit 8.4 | bit | Expander mode active flag |

## MIDI Buffer Structure

| Address | Purpose |
|---------|---------|
| 0x1B20 | MIDI RX ring buffer base (256 bytes) |
| 0x1A1A | MIDI RX byte count |
| 0x1A1B | MIDI RX write pointer |
| 0x1A1C | MIDI RX read pointer |
| 0x1A21 | MIDI TX ring buffer base |
| 0x1A1D | MIDI TX byte count |
| 0x1A1E | MIDI TX write pointer |

## MIDI Processing Flow

1. **IRQ Handler** (0xC1E3): Receives MIDI bytes via UART, stores in RX buffer at 0x1B20
2. **Main Loop** calls FUN_CODE_d375 to process MIDI messages
3. **Channel Remapping** at 0xD44A:
   - Keyboard mode: Channel remapped via table at 0xF75A
   - Expander mode: Channel used directly

### Channel Remap Table (0xF75A) - Keyboard Mode Only
```
MIDI Ch: 0  1  2  3  4  5  6  7  8-15...
Maps to: 02 03 00 04 08 0C 01 05 02 02...
```

## Program Change Handler Analysis (0xD5F3)

```asm
D5F3: MOV DPTR, #0x1FFF    ; load mode address
D5F6: MOVX A, @DPTR        ; read mode
D5F7: JZ D602              ; if mode==0 (keyboard), skip expander logic
D5F9: MOV DPTR, #0x1C24    ; expander path: check channel
D5FC: MOVX A, @DPTR
D5FD: SETB C
D5FE: SUBB A, #7           ; check if channel < 8
D600: JC D604              ; if channel < 8, process it
D602: SJMP D61F            ; skip to keyboard handler

; Expander mode program load (channels 0-7):
D604: MOV DPTR, #0x1A0A    ; get program number
D607: MOVX A, @DPTR
D608: CLR C
D609: SUBB A, #0x80        ; check if prog < 128
D60B: JNC D61D             ; skip if prog >= 128
D60D: MOV DPTR, #0x1A09    ; get internal channel
D610: MOVX A, @DPTR
D611: MOV 0x78, A          ; param1 = channel
D613: MOV DPTR, #0x1A0A
D616: MOVX A, @DPTR        ; get program number
D617: INC A                ; prog + 1 (1-indexed)
D618: MOV 0x79, A          ; param2 = program
D61A: LCALL 0x8D73         ; call SAM_LOAD_SND_PROGRAM(channel, prog+1)
```

## Mode Toggle Function (0xA5B2)

This function toggles between keyboard and expander mode:
```c
void toggle_expander_mode() {
    if (bit_8_4 == 1) {
        // Exit expander mode
        mode = 0;
        mode_ffe = 0;
        mode_ffd = 0;
    } else {
        // Enter expander mode
        bit_8_4 = 1;
        if (check_condition()) {
            mode = 2;  // EXPANDER MODE
        }
    }
}
```

Called from button handler caseD_44 at 0xABF9.

## ROM Patch for Forcing Expander Mode

### Patch Location
- **File**: `kf10_ic27_v2.bin`
- **Offset**: 0xD5F7 (decimal 54775)
- **Original bytes**: `60 09` (JZ +9 - skip expander logic if mode==0)
- **Patched bytes**: `00 00` (NOP NOP - always fall through to expander logic)

### Creating the Patched ROM

```bash
# Create patched ROM
cp kf10_ic27_v2.bin kf10_ic27_v2_expander.bin
printf '\x00\x00' | dd of=kf10_ic27_v2_expander.bin bs=1 seek=54775 conv=notrunc

# Verify patch
xxd -s 54773 -l 16 kf10_ic27_v2_expander.bin
# Should show: d5f5: ffe0 0000 901c ... (00 00 instead of 60 09)
```

### Effect of Patch

With this patch, the Program Change handler always uses expander-mode logic regardless of the actual mode variable:
- MIDI program 0 loads sound program 1
- MIDI program 1 loads sound program 2
- etc.

This allows direct testing of all 128 sound programs via MIDI without keyboard mode's remapping.

## Testing

```bash
# Create test MIDI file for program 1 (sound program 2)
python3 -c "
from midiutil import MIDIFile
m = MIDIFile(1)
m.addProgramChange(0, 0, 0, 1)  # Program 1
m.addNote(0, 0, 60, 0, 1, 100)  # Note C4
with open('/tmp/test_prog1.mid', 'wb') as f:
    m.writeFile(f)
"

# Run with patched ROM
./mamemuse keyfox10 -rompath /path/to/roms -midiin /tmp/test_prog1.mid -seconds_to_run 3
```

## Files

- `kf10_ic27_v2_orig.bin` - Original firmware backup
- `kf10_ic27_v2_expander.bin` - Patched firmware with forced expander mode
- `kf10_ic27_v2.bin` - Active ROM (swap between orig/expander as needed)

## Restoring Original ROM

```bash
cd /path/to/roms/hohner/keyfox10
cp kf10_ic27_v2_orig.bin kf10_ic27_v2.bin
```
