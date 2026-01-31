# SAM8905 Firmware - Emulator Component

This documents the SAM8905 DSP emulator and Linux platform support files
copied from the Ambika Retro project.

**Related documentation:**
- `WIP_sam_firmware_port.md` - Main firmware port planning
- `WIP_solton_ms4.md` - MS4 firmware analysis (reference)

## Directory Structure

```
sam_firmware/emu/
├── sam8905.h          # SAM8905 high-level interface (constants, macros)
├── sam8905_emu.h      # Emulator state structure and API
├── sam8905_emu.c      # Emulator implementation (ported from MAME)
├── sam_hw_emu.c       # Platform implementation of sam_hw.h for emulator
├── audio_portaudio.h  # PortAudio audio output interface
├── audio_portaudio.c  # PortAudio implementation
├── midi_alsa.h        # ALSA MIDI input (parsed messages)
├── midi_alsa.c        # ALSA sequencer - parsed message mode
├── midi_alsa_raw.h    # ALSA MIDI input (raw bytes)
├── midi_alsa_raw.c    # ALSA sequencer - raw byte mode
├── main_emu.c         # Main loop (MS4-like structure)
├── Makefile           # Build for Linux
└── sam_emu            # Output binary
```

## Building and Running

```bash
cd sam_firmware/emu
make
./sam_emu
```

Then connect a MIDI device:
```bash
aconnect <source_port> <sam_emu_port>
```

## SAM8905 Emulator

### Overview

The emulator is ported from MAME's SAM8905 sound device implementation.
It provides a cycle-accurate software emulation of the Dream SAM8905 DSP chip.

### Architecture

The SAM8905 is a programmable DSP with:
- **A-RAM**: 256 × 15-bit micro-instructions (algorithm code)
- **D-RAM**: 256 × 19-bit parameters (16 slots × 16 words)
- **16 voice slots**: Each slot runs an algorithm from A-RAM
- **Dual sample rate**: 44.1kHz (8 algos × 32 inst) or 22.05kHz (4 algos × 64 inst)

### Emulator State (`Sam8905Emu` structure)

```c
typedef struct Sam8905Emu {
    // RAM
    uint16_t aram[256];    // Algorithm micro-instructions
    uint32_t dram[256];    // Parameter D-RAM (19-bit values)

    // DSP registers
    uint32_t bus;          // Internal 19-bit bus
    uint32_t a, b;         // Adder registers
    uint32_t x, y;         // Multiplier inputs (12-bit)
    uint32_t phi;          // Phase register (12-bit)
    uint32_t wf;           // Waveform select (9-bit)
    uint32_t mul_result;   // Multiplier output (19-bit)

    // Flags
    bool carry;            // Adder carry
    bool clear_rqst;       // Conditional write flag
    bool int_mod;          // Interrupt modification

    // Output
    int32_t l_acc, r_acc;  // 24-bit stereo accumulators
    uint8_t mix_l, mix_r;  // Output attenuation (3-bit, 0=mute, 7=0dB)

    // Control
    bool idle;             // Global idle
    bool ssr_22khz;        // Sample rate select (false=44.1k, true=22.05k)

    // External waveform memory callbacks
    sam8905_waveform_read_fn waveform_read;
    sam8905_waveform_write_fn waveform_write;
    void* waveform_ctx;
} Sam8905Emu;
```

### Direct Function Interface (`sam8905.h`)

The emulator implements a direct function interface for portability:

```c
// A-RAM access (algorithm micro-instructions)
void sam8905_write_aram(uint8_t addr, uint16_t inst);
uint16_t sam8905_read_aram(uint8_t addr);

// D-RAM access (voice parameters)
void sam8905_write_dram(uint8_t addr, uint32_t param);
uint32_t sam8905_read_dram(uint8_t addr);

// Control
void sam8905_set_idle(bool idle);
void sam8905_set_sample_rate(bool ssr_22khz);
void sam8905_init(void);

// Audio processing (emulator only)
void sam8905_process_frame(int32_t* out_l, int32_t* out_r);
```

### Instance Management

For the direct function interface, a global instance must be set:

```c
Sam8905Emu emu;
sam8905_emu_init(&emu);
sam8905_emu_set_instance(&emu);

// Now sam8905_write_dram() etc. operate on 'emu'
```

### D-RAM Slot Layout

Each of 16 slots has 16 words in D-RAM:

| Word | Address | Typical Use |
|------|---------|-------------|
| 0 | slot×16+0 | PHI (phase accumulator) |
| 1 | slot×16+1 | DPHI (phase increment) |
| 2 | slot×16+2 | AMP (amplitude + mix) |
| 3-14 | slot×16+3..14 | Algorithm-specific |
| 15 | slot×16+15 | Control (IDLE, ALG, INTMASK) |

### Word 15 Control Bits

```
Bit 11:    IDLE - slot produces no sound when set
Bits 10:8: ALG  - algorithm select (0-7 at 44.1kHz)
Bit 7:     INTMASK - interrupt mask
Bits 6:0:  MIX (optional override)
```

### Helper Macros

```c
// D-RAM addressing
SAM8905_DRAM_ADDR(slot, word)  // → (slot<<4) | word

// A-RAM addressing (44.1kHz)
SAM8905_ARAM_ADDR_44K(algo, pc)  // → (algo<<5) | pc

// Parameter formatting
SAM8905_PHASE(phase12)           // Phase value (12-bit → 19-bit)
SAM8905_DPHI(phase_inc)          // Phase increment
SAM8905_AMP_MIX(amp12, mix_l, mix_r)  // Amplitude + mix routing

// Slot control
SAM8905_SLOT_IDLE                // Word 15 value for idle
SAM8905_SLOT_ACTIVE_44K(algo)    // Word 15 value for active
```

### Waveform Generation

Internal waveforms (WF[8]=1):
- **Sinus**: WF = 0x100
- **Ramps**: WF = 0x100 | (R<<6) | (SEL<<4)
  - SEL=0: 2× PHI ramp
  - SEL=1: Constant from MAD field
  - SEL=2: PHI ramp
  - SEL=3: PHI/2 ramp
- **Zero**: WF = 0x108

External waveforms (WF[8]=0):
- Address = WF[7:0] | PHI[11:0]
- Requires waveform_read callback

## Linux Platform Support

### Audio Output (PortAudio)

Simple callback-based audio output:

```c
// Initialize with sample rate
audio_portaudio_init(44100);

// Set audio callback
int my_callback(int16_t* buffer, int frames, void* user_data) {
    for (int i = 0; i < frames; i++) {
        int32_t left, right;
        sam8905_process_frame(&left, &right);
        buffer[i] = (int16_t)left;  // Mono output
    }
    return frames;
}
audio_portaudio_set_callback(my_callback, NULL);

// Start/stop streaming
audio_portaudio_start();
// ... run ...
audio_portaudio_stop();
audio_portaudio_shutdown();
```

Configuration:
- Sample rate: 44100 Hz (or as specified)
- Channels: 1 (mono)
- Format: 16-bit signed integer
- Buffer size: 256 frames (low latency)

### MIDI Input (ALSA)

ALSA sequencer-based MIDI input:

```c
// Initialize with client name
midi_alsa_init("SAM8905 Synth");

// Set MIDI callback
void my_midi_callback(MidiMessage* msg, void* user_data) {
    if (msg->status == MIDI_NOTE_ON) {
        // Handle note on
    }
}
midi_alsa_set_callback(my_midi_callback, NULL);

// Poll in main loop (non-blocking)
while (running) {
    midi_alsa_poll();
    // ... other processing ...
}

midi_alsa_shutdown();
```

Connect MIDI devices with:
```bash
aconnect <source_client>:<source_port> <our_client>:<our_port>
```

MIDI message types supported:
- Note On/Off
- Control Change
- Program Change
- Pitch Bend
- Poly/Channel Pressure

### Dependencies

For Linux build:
```bash
# Debian/Ubuntu
sudo apt install libportaudio2 libportaudio-dev libasound2-dev

# Arch
sudo pacman -S portaudio alsa-lib
```

## Integration with Firmware Port

The emulator connects to the firmware port via the SAM hardware interface:

```c
// In sam_hw.c, replace dummy implementation with emulator:

#include "emu/sam8905_emu.h"

static Sam8905Emu g_sam_emu;

void sam_write_reg(uint8_t reg, uint8_t value) {
    // Map 8051 register writes to emulator API
    switch (reg) {
        case SAM_REG_ADDR_DATA: g_dram_addr = value; break;
        case SAM_REG_DATA1: g_dram_hi = value; break;
        case SAM_REG_DATA2: g_dram_lo = value; break;
        case SAM_REG_CTRL:
            if (value == SAM_CTRL_DRAM_WR) {
                sam8905_write_dram(g_dram_addr,
                    ((uint32_t)g_dram_hi << 8) | g_dram_lo);
            }
            break;
    }
}
```

## Task List

- [x] Copy SAM8905 emulator from Ambika Retro
- [x] Copy PortAudio audio output
- [x] Copy ALSA MIDI input (parsed messages)
- [x] Add ALSA MIDI raw byte interface (`midi_alsa_raw.c`)
- [x] Fix include paths
- [x] Create Makefile for emu build
- [x] Create main loop (`main_emu.c`) with MS4-like structure
- [x] Test basic sine wave generation with MIDI note on/off
- [x] Integrate firmware MIDI parser (`sam_midi.c`) with emulator
  - ALSA MIDI bytes → `midi_rx_isr()` → firmware RX buffer
  - Main loop calls `midi_process_byte()` for parsing
  - Weak stub handlers overridden in `main_emu.c` for simple sound generation
- [x] Integrate with sam_hw.c register interface
  - `sam_hw_emu.c` implements `sam_write_reg()`/`sam_read_reg()`
  - Translates register writes to `sam8905_write_dram()`/`sam8905_write_aram()`
  - `sam_init()` now works through the emulator
- [x] Add MIDI port auto-connect via command line (`-m <port>`)
- [x] Fix handler_18 to write to SAM D-RAM (not just voice page)
- [x] Fix handler_20 status byte preservation (bits 7:3 must be preserved)
- [ ] Implement envelope modulation (periodic amplitude updates)
- [ ] Add stereo output support to audio_portaudio.c
- [ ] Add MIDI output support (for Active Sense, etc.)
- [ ] Port firmware voice allocation

## Progress Log

### 2026-01-30: D-RAM Handler Fixes

#### Issue 1: Handler 0x18 Not Writing to SAM D-RAM

**Problem**: `dram_config_handler_18` was only writing to voice page memory, not to SAM D-RAM.

**Discovery**: Ghidra decompilation of MS4 firmware CODE:B222 showed the original firmware:
1. Reads dispatch, value_lo, value_hi, vel_sens from stream
2. Writes dispatch to voice page (for mod state tracking)
3. Calls `sam_write_dram(dram_address_counter)` to write to SAM D-RAM
4. Advances DRAM address counter

**Fix in `sam_dram_config.c`**: Added SAM register writes:
```c
/* Write to SAM D-RAM */
sam_write_reg(SAM_REG_ADDR_DATA, g_intmem.dram_address_counter);
sam_write_reg(SAM_REG_DATA1, dispatch);
sam_write_reg(SAM_REG_DATA2, value_lo);
sam_write_reg(SAM_REG_DATA3, value_hi & 0x07);
sam_write_reg(SAM_REG_CTRL, g_intmem.sam_ctrl_flags);
```

**Result**: D-RAM words D[2], D[3], D[4], D[6], D[10], D[11] now written correctly.

#### Issue 2: Handler 0x20 Clobbering Voice Status

**Problem**: `dram_config_handler_20` was overwriting `VOICE_PAGE_ROUTE3` (offset 0xFB) entirely, which cleared bit 5 (0x20) - the "processing active" flag set during voice allocation.

**Discovery**: Ghidra decompilation of CODE:B278 showed:
```c
DAT_EXTMEM_00fb = bVar2 & 7 | DAT_EXTMEM_00fb & 0xf8
```
The original firmware only modifies bits 2:0, preserving bits 7:3.

**Fix in `sam_dram_config.c`**:
```c
/* Route3/status (0xFB): only update bits 2:0, preserve bits 7:3 */
uint8_t current_status = voice_page_read(g_intmem.voice_page_num, VOICE_PAGE_ROUTE3);
uint8_t new_status = (route3 & 0x07) | (current_status & 0xF8);
voice_page_write(g_intmem.voice_page_num, VOICE_PAGE_ROUTE3, new_status);
```

**Result**: Voice status now goes from 0x20 → 0x23 (active + routing) instead of 0x20 → 0x03 (freed).

### Current Status

**Working**:
- MIDI input → voice allocation → program loading
- A-RAM algorithm loading from ROM
- D-RAM handler chain execution (0x00, 0x08, 0x10, 0x18, 0x20)
- Simple test ROM with sinus algorithm produces sound
- Handler 0x18 writes D-RAM values correctly
- Voice status preserved correctly through handler chain

**Not Working**:
- Complex MS4 algorithms (e.g., dpiano27/Algorithm 027A) produce 0 audio

### Root Cause: Complex Algorithms Need Envelope Modulation

Algorithm 027A (dpiano27) analysis shows it reads amplitude from D[5]:
```
Instruction 0x09F7: RM 1, WXY, WSP
  - MAD = 1 → DRAM offset 1 → voice_base + 1 → D[5] for slot 0
  - WXY = set X (waveform) and Y (amplitude)
  - Without D[5] set, Y=0, output=0
```

**Why D[5] is not initialized**:
- dpiano27's D-RAM stream has NO handler 0x10 (amplitude handler)
- The firmware relies on **envelope modulation** to dynamically update amplitude words
- Envelope modulation runs periodically (Timer 1 ISR) and scales amplitude based on ADSR state
- Without envelope modulation, D[5] stays at 0 → no audio

### What's Needed to Complete

**Option A: Implement Envelope Modulation**
- Port `voice_mod_update()` or equivalent from firmware
- Called from periodic timer (every ~10ms)
- Reads envelope parameters from voice page
- Calculates current amplitude based on ADSR state
- Writes scaled amplitude to SAM D-RAM

**Option B: Use Simpler Test Programs**
- Create test ROM with explicit amplitude setup (handler 0x10 in D-RAM stream)
- Algorithm uses D[1] for amplitude instead of requiring envelope
- gen_test_rom.py already does this with ALGORITHM_SINUS

**Option C: Patch MS4 Programs**
- Modify dpiano27's D-RAM stream to include amplitude handler
- Direct approach but requires understanding each algorithm's D-RAM layout

**Recommended Path**: Start with Option B (simpler test programs work), then implement Option A (envelope modulation) for full MS4 program compatibility.

### Algorithm D-RAM Layout Reference

**Simple Sinus (test ROM)**:
- D[0] = pitch (phase increment)
- D[1] = amplitude (direct, no envelope)
- D[2] = phase accumulator (internal)

**Algorithm 027A (dpiano27)**:
- D[0] = pitch
- D[2], D[5], D[11] = amplitude words (via WXY instructions)
- Requires envelope modulation to set D[5]
