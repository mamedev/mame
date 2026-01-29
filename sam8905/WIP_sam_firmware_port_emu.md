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
- [ ] Add stereo output support to audio_portaudio.c
- [ ] Add MIDI output support (for Active Sense, etc.)
- [ ] Port firmware voice allocation
