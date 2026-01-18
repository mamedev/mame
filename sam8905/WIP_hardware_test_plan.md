# SAM8905 Hardware Test Plan

## Objective

Run minimal test firmware on original Keyfox10 hardware to verify SAM8905 waveform generation behavior, specifically:

1. **WF bit layout**: Is R at bit 7 and I at bit 6 (manual), or swapped (working emulation)?
2. **I bit semantics**: Does I=0 mean direct or apply transformation?
3. **Invert operation**: Two's complement (-x), one's complement (~x), or something else?

## Background

The emulator works with swapped R/I bits (R=bit6, I=bit7) but the programmer's guide says R=bit7, I=bit6. Testing on real hardware will resolve this ambiguity.

## Hardware Setup

- [ ] Keyfox10 with working SAM8905 FX chip
- [ ] EPROM programmer (for 27C256 or similar)
- [ ] Logic analyzer or oscilloscope for output capture
- [ ] Audio recording setup (line out → audio interface)

## Test Strategy

### Approach C: Minimal 8051 Firmware (RECOMMENDED)

Write a minimal C firmware using SDCC that:
1. Initializes SAM8905 with test programs
2. Outputs known waveforms for analysis
3. Can be tested in MAME emulator first
4. Then burned to EPROM for real hardware

**Advantages**:
- Full control over SAM programming
- Test in emulator before burning ROM
- Simple, self-contained code
- Can iterate quickly

**Disadvantages**:
- Need to understand Keyfox10 memory map
- May need minimal hardware init (clocks, etc.)

### Approach A: ROM Patch Method

Patch the Keyfox10 firmware to:
1. Initialize SAM with a known simple program
2. Output a test waveform we can analyze
3. Vary the WF bits systematically

Advantages:
- Uses existing hardware as-is
- Can compare against known-working factory presets

Disadvantages:
- Need to understand firmware well enough to patch
- Limited control over SAM programming

### Approach B: Standalone SAM Test Rig

Build minimal circuit to drive SAM8905 directly:
1. Microcontroller writes ARAM/DRAM
2. SAM generates output
3. Capture and analyze audio

Advantages:
- Full control over SAM programming
- Can test exact instruction sequences

Disadvantages:
- Need to build hardware
- SAM8905 requires external components (clock, waveform ROM?)

## SDCC Firmware Implementation

### Toolchain Setup

```bash
# Install SDCC (Arch Linux)
pacman -S sdcc

# Compile for 8051
sdcc --model-small -mmcs51 test_sam.c -o test_sam.ihx

# Convert Intel HEX to binary
objcopy -I ihex -O binary test_sam.ihx test_sam.bin

# Pad to EPROM size (32KB for 27C256)
dd if=/dev/zero bs=32768 count=1 | cat test_sam.bin - | head -c 32768 > test_sam_padded.bin
```

### Keyfox10 Memory Map (from keyfox10.cpp)

```
CPU: 80C32 (8051 family, no internal ROM)

Program Memory (PSEN):
  0x0000-0xFFFF  128KB ROM via banking

External Data Memory (MOVX):
  0x0000-0x1FFF  8KB RAM
  0x2000-0x7FFF  ROM data (always accessible)
  0x8000-0xDFFF  ROM data (T1=0) or SAM SND (T1=1)
  0xE000-0xFFFF  ROM data (T1=0) or SAM FX (T1=1)

SAM Access Control:
  T1 = P3.5 - must be HIGH (1) to access SAM chips
```

### Keyfox10 SAM8905 Registers

SAM8905 SND chip: 0x8000-0x8007 (when T1=1)
SAM8905 FX chip: 0xE000-0xE007 (when T1=1)

```c
// Register offsets (same for both chips)
#define SAM_ADDR     0  // Address register (W)
#define SAM_DATA_L   1  // Data LSB (R/W)
#define SAM_DATA_M   2  // Data MSB (A-RAM) or NSB (D-RAM) (R/W)
#define SAM_DATA_H   3  // Data MSB (D-RAM only) (R/W)
#define SAM_CTRL     4  // Control register (any of 4-7) (W)

// Control register bits
#define CTRL_WR      0x01  // Bit 0: Write mode (1=write, 0=read)
#define CTRL_SEL     0x02  // Bit 1: Memory select (0=D-RAM, 1=A-RAM)
#define CTRL_IDL     0x04  // Bit 2: Idle (stops processing?)
#define CTRL_SSR     0x08  // Bit 3: Sample rate select (0=44kHz, 1=22kHz)

// T1 control via P3.5
// Set P3 |= 0x20 to enable SAM access
// Set P3 &= ~0x20 to disable (ROM data mode)
```

### Minimal Test Firmware Skeleton

```c
// test_sam.c - Minimal SAM8905 test firmware for Keyfox10
// Compile: sdcc --model-small -mmcs51 test_sam.c

#include <8051.h>

// P3.5 = T1 - controls SAM chip access
__sfr __at(0xB0) P3;
#define T1_BIT  0x20

// SAM8905 SND chip registers (at 0x8000 when T1=1)
__xdata __at(0x8000) unsigned char SAM_SND_ADDR;
__xdata __at(0x8001) unsigned char SAM_SND_DATA_L;
__xdata __at(0x8002) unsigned char SAM_SND_DATA_M;
__xdata __at(0x8003) unsigned char SAM_SND_DATA_H;
__xdata __at(0x8004) unsigned char SAM_SND_CTRL;

// SAM8905 FX chip registers (at 0xE000 when T1=1)
__xdata __at(0xE000) unsigned char SAM_FX_ADDR;
__xdata __at(0xE001) unsigned char SAM_FX_DATA_L;
__xdata __at(0xE002) unsigned char SAM_FX_DATA_M;
__xdata __at(0xE003) unsigned char SAM_FX_DATA_H;
__xdata __at(0xE004) unsigned char SAM_FX_CTRL;

// Control register bits
#define CTRL_WR   0x01  // Write mode
#define CTRL_SEL  0x02  // 0=D-RAM, 1=A-RAM
#define CTRL_IDL  0x04  // Idle (stops processing)
#define CTRL_SSR  0x08  // Sample rate (0=44kHz, 1=22kHz)

// Enable SAM chip access
void sam_enable(void) {
    P3 |= T1_BIT;  // Set T1=1
}

// Disable SAM chip access (back to ROM mode)
void sam_disable(void) {
    P3 &= ~T1_BIT;  // Set T1=0
}

// Write 15-bit instruction to SND A-RAM
// CRITICAL: Sequence must be ADDR -> DATA -> CTRL (CTRL triggers the write!)
void sam_snd_write_aram(unsigned char addr, unsigned int inst) {
    SAM_SND_ADDR = addr;
    SAM_SND_DATA_L = inst & 0xFF;
    SAM_SND_DATA_M = (inst >> 8) & 0x7F;
    SAM_SND_CTRL = CTRL_SEL | CTRL_WR;  // Select A-RAM, write mode - triggers write
}

// Write 19-bit parameter to SND D-RAM
// CRITICAL: Sequence must be ADDR -> DATA -> CTRL (CTRL triggers the write!)
void sam_snd_write_dram(unsigned char addr, unsigned long param) {
    SAM_SND_ADDR = addr;
    SAM_SND_DATA_L = param & 0xFF;
    SAM_SND_DATA_M = (param >> 8) & 0xFF;
    SAM_SND_DATA_H = (param >> 16) & 0x07;
    SAM_SND_CTRL = CTRL_WR;  // Select D-RAM, write mode - triggers write
}

// Start SAM processing (clear IDL bit)
void sam_snd_start(void) {
    SAM_SND_CTRL = 0;  // 44kHz mode (SSR=0), IDL=0 (running)
    // NOTE: SND chip should use 44kHz mode (32 instructions/algorithm)
}

// Load test program: output internal sinus waveform
void load_test_sinus(unsigned int wf_value) {
    // Slot 0 D-RAM (addresses 0x00-0x0F)
    // D0: WF in bits 17:9, PHI in bits 18:7
    unsigned long d0 = ((unsigned long)wf_value << 9);
    sam_snd_write_dram(0x00, d0);

    // D15 (0x0F): Output mix config
    sam_snd_write_dram(0x0F, 0x50080);  // Enable output

    // Slot 0 A-RAM (addresses 0x00-0x01)
    sam_snd_write_aram(0x00, 0x4800);  // RM D0, <WWF, WPHI, WXY>
    sam_snd_write_aram(0x01, 0x200F);  // RM D15, <WACC>
}

// Test cases for WF bit interpretation
#define WF_SINUS_I0     0x100  // INT=1, R=0, I=0
#define WF_SINUS_I1     0x140  // INT=1, R=0, I=1 (if I=bit6)
#define WF_SINUS_I1_ALT 0x180  // INT=1, R=0, I=1 (if I=bit7)
#define WF_RAMP_SEL2    0x1A0  // INT=1, R=1, SEL=2 (PHI ramp)

void main(void) {
    // Enable SAM chip access
    sam_enable();

    // Load test program
    load_test_sinus(WF_SINUS_I0);

    // Start SAM processing
    sam_snd_start();

    // Loop forever - SAM generates audio
    while(1) {
        // TODO: Add button input to switch test cases
    }
}
```

### Build and Test in MAME

```bash
# Build firmware
cd sam8905/firmware
sdcc --model-small -mmcs51 test_sam.c -o test_sam.ihx
objcopy -I ihex -O binary test_sam.ihx test_sam.bin

# Pad to ROM size
dd if=test_sam.bin of=test_sam_32k.bin bs=32768 conv=sync

# Copy to ROM location (backup original first!)
cp test_sam_32k.bin /path/to/roms/hohner/keyfox10/kf10_ic27_v2.bin

# Run in MAME
./mamemuse keyfox10 -rompath /path/to/roms/hohner -oslog
```

### Tasks

- [x] Extract SAM8905 register addresses from keyfox10.cpp
- [x] Write and compile skeleton firmware (485 bytes, pads to 128KB)
- [x] Test in MAME emulator - D-RAM and A-RAM writes confirmed working
- [x] Verify audio output from test firmware - 440Hz sine confirmed
- [x] Create assembler for programmer's guide syntax
- [ ] Iterate on test cases (different WF values for R/I bit testing)
- [ ] Burn to EPROM and test on real hardware

### Files Created

- `sam8905/firmware/test_sam.c` - Test firmware source (working, produces audio)
- `sam8905/firmware/Makefile` - Build system
- `sam8905/firmware/test_sam_padded.bin` - 128KB ROM ready for EPROM
- `sam8905/sam8905_assembler.py` - Assembler for programmer's guide syntax

### Critical Findings from Emulator Testing (2026-01-18)

#### 1. Write Sequence is Critical

The SAM8905 CPU interface triggers the RAM write when the **CTRL register is written**.
The correct sequence is:

```
1. Write address to ADDR register (offset 0x00)
2. Write data to DATA_L/M/H registers (offsets 0x01-0x03)
3. Write CTRL register (offset 0x04) - THIS TRIGGERS THE WRITE
```

Writing CTRL first (before ADDR/DATA) will write garbage to whatever address
was previously in the address register.

#### 2. IDLE Bit Position

The IDLE bit in param15 (D-RAM word 15) is at **bit 11 (0x800)**, not bit 1.
To idle a slot: `sam_write_dram(slot * 16 + 15, 0x00800);`

#### 3. Unused Slots Must Be Idled

Unused slots contribute noise to the output if not explicitly idled.
The firmware must idle all 15 unused slots and fill their A-RAM with NOPs.

#### 4. Sample Rate Mode

- **44kHz mode (SSR=0)**: 8 algorithms × 32 instructions each (A-RAM 0-255)
- **22kHz mode (SSR=1)**: 4 algorithms × 64 instructions each (A-RAM 0-255)

The SND chip should use **44kHz mode** for the 6-instruction sinus oscillator.
Using 22kHz mode causes the chip to execute 64 instructions per algorithm,
reading uninitialized A-RAM locations which produce garbage output.

#### 5. Emulator Test Results

Using the corrected firmware with 44kHz mode and idled slots:

| Parameter       | Result          |
|-----------------|-----------------|
| Target freq     | 440 Hz          |
| Measured freq   | 439.45 Hz       |
| THD             | 0.04%           |
| Output          | Clean sine wave |

The 0.55 Hz error (~0.1%) is due to phase increment rounding (41 vs 40.87).

### SAM8905 Assembler

Created `sam8905_assembler.py` with:

1. **Syntax parsing** matching the Programmer's Guide:
   ```
   PHI=0                       ; Variable definition
   RM      PHI,    <WA,WPHI,WSP>   ; Full instruction
                   ,<WACC>         ; Continuation
   FIN                         ; Fill with NOPs
   ```

2. **D-RAM helper functions**:
   - `make_dram_word(wf, phi, value, mix_l, mix_r, alg, idle)` - Create D-RAM word
   - `make_amplitude_word(amplitude, mix_l, mix_r)` - For WXY amplitude/mix
   - `make_phase_increment(frequency, sample_rate)` - Calculate DPHI for freq

Example usage:
```python
from sam8905_assembler import SAM8905Assembler, make_phase_increment, make_amplitude_word

asm = SAM8905Assembler()
aram, errors = asm.assemble(source_code)

dram[1] = make_phase_increment(440, 44100) << 7  # 440Hz
dram[2] = make_amplitude_word(0x400, mix_l=7, mix_r=7)
```

## Test Cases

### Test 1: Identify R/I Bit Positions

**Goal**: Determine if R=bit7/I=bit6 (manual) or R=bit6/I=bit7 (working)

**Method**:
```
WF = 0x140  # bit7=0, bit6=1, bit5=0, bit4=0
WF = 0x180  # bit7=1, bit6=0, bit5=0, bit4=0
```

With PHI sweeping 0-4095:
- If R=bit7: 0x180 should produce RAMP, 0x140 should produce SINUS
- If R=bit6: 0x140 should produce RAMP, 0x180 should produce SINUS

**Expected output**:
- SINUS: smooth sine wave
- RAMP: sawtooth wave

### Test 2: I Bit Effect

**Goal**: Determine what I bit does

**Method**:
```
WF = 0x100  # INT=1, R=0, I=0 (sinus, no invert?)
WF = 0x140  # INT=1, R=0, I=1 (sinus, invert?)
```

Compare output phase/polarity.

### Test 3: Invert at Zero

**Goal**: Test what happens when waveform=0 is inverted

**Method**:
Use RAMP mode at PHI=0 (should produce X=0), then check if:
- I=0 gives 0 (direct)
- I=1 gives 0 (two's complement of 0)
- I=1 gives -1 (one's complement of 0)

This requires precise measurement at specific PHI values.

### Test 4: Constant Mode Verification

**Goal**: Verify SEL=1 uses MAD for constant selection

**Method**:
```
WF = 0x190  # INT=1, R=1, SEL=1 (constant mode)
```

Execute WXY with different MAD values, verify output matches constant table.

## Firmware Patch Points

### Candidate: Boot-time SAM initialization

Location in Keyfox10 firmware where SAM ARAM/DRAM is initialized:
- [ ] Find SAM initialization routine
- [ ] Identify ARAM write sequence
- [ ] Patch to load test program instead

### Candidate: MIDI program change handler

Hijack program change to trigger test mode:
- [ ] Program 127 → enter test mode
- [ ] Test mode loads minimal SAM program
- [ ] Outputs test waveform continuously

## Minimal SAM Test Program

```asm
; Slot 0 test program - output internal waveform directly
; D-RAM setup:
;   D0 = 0x00000  (bus source)
;   D1 = WF_TEST << 9  (waveform config, PHI in lower bits)
;   DF = 0x?????  (mix/output config)

; A-RAM (2 instructions per slot, 16 slots):
PC00: RM D1, <WWF, WPHI>      ; Load WF and PHI from D1
PC01: RM D1, <WXY, WACC>      ; Generate waveform, accumulate to output
```

## Data Collection

For each test:
1. Record audio output (44.1kHz, 24-bit)
2. Capture with logic analyzer if available
3. Document exact ARAM/DRAM values used
4. Note any unexpected behavior

## Analysis

Compare captured waveforms against emulator output:
- FFT analysis for frequency content
- Phase comparison for invert behavior
- Amplitude measurement for constant values

## Open Questions

- [ ] What clock frequency does Keyfox10 use for SAM8905?
- [ ] Is waveform ROM required for internal waveform generation?
- [ ] Can we access SAM registers directly via Keyfox10 CPU bus?
- [ ] Are there test points on the PCB for SAM outputs?

## Resources Needed

- Keyfox10 service manual / schematics
- SAM8905 programmer's guide (have)
- EPROM programmer compatible with Keyfox10 ROMs
- Audio analysis software (Audacity, Sonic Visualiser, Python/scipy)

## Timeline

- [ ] Phase 1: Analyze Keyfox10 firmware for patch points
- [ ] Phase 2: Design test program
- [ ] Phase 3: Create ROM patch
- [ ] Phase 4: Execute tests on hardware
- [ ] Phase 5: Analyze results, update emulator

## Notes

- 2026-01-18: Emulator testing complete. Firmware produces clean 440Hz sine wave with 0.04% THD.
  Key fixes: correct write sequence (ADDR→DATA→CTRL), IDLE bit at position 11, 44kHz mode.

---
*Document created: 2026-01-17*
*Updated: 2026-01-18*
*Status: Emulator testing complete, ready for hardware testing*
