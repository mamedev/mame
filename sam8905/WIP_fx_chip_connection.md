# SAM8905 FX Chip Connection Implementation Plan

## Overview

Connect the SAM8905 sound generation chip to the SAM8905 effects processor chip, enabling proper effects processing in the keyfox10 emulation.

## Status: COMPLETED ✓

All implementation tasks completed on 2026-01-10.

## Hardware Architecture

### Signal Flow (from schematic)
```
SAM_SND (Sound) ──DABD0/CLBD0──▶ 74HC595 (IC23) ──cascade──▶ 74HC595 (IC22)
                                      │                           │
                                      ▼                           ▼
                              8-bit parallel            8-bit parallel
                                      │                           │
                                      └──────────┬────────────────┘
                                                 ▼
                                      16-bit input to SAM_FX
                                      (via waveform data bus)
```

### Key Details
- **Stereo effect send**: L/R samples sent separately (multiplexed)
- **Parallel routing**: Sound SAM → speakers (dry) + FX SAM → speakers (wet)
- **Address control**: WA19, WA0, WA1 control HC595 output enable
  - When WA19=1 (WAVE[7]=1, i.e., WF ≥ 0x80), reads from shift registers
  - WA0/WA1 (PHI[0:1]) select L/R channel and byte

### Waveform Address Mapping
```
WA[19:0] = { WAVE[7:0], PHI[11:0] }
           └─WA19      └─WA1,WA0

When WA19=1: Read from shift register (input samples)
  - WA0=0: Left channel
  - WA0=1: Right channel
```

## Implementation Tasks

### 1. SAM8905 Device: Add Sample Output Callback

**File**: `src/devices/sound/sam8905.h`

- [x] Add sample output callback for inter-chip audio

```cpp
// Add callback for sample output (fires each sample period)
auto sample_output_callback() { return m_sample_output.bind(); }

private:
    devcb_write32 m_sample_output;  // Outputs L/R packed as (L << 16) | R
```

**File**: `src/devices/sound/sam8905.cpp`

- [x] Initialize callback in constructor initializer list
- [x] Call callback at end of each sample in `sound_stream_update()`

```cpp
// In constructor initializer list:
, m_sample_output(*this)

// In sound_stream_update(), after computing out_l/out_r:
if (!m_sample_output.isunset()) {
    // Pack L/R as 32-bit value: upper 16 = L, lower 16 = R
    uint32_t packed = ((out_l & 0xFFFF) << 16) | (out_r & 0xFFFF);
    m_sample_output(packed);
}
```

### 2. Keyfox10 Driver: Store Input Samples

**File**: `src/mame/wersi/keyfox10.cpp`

- [x] Add member variables for FX input samples
- [x] Add callback handler for sound SAM output
- [x] Add save_item calls for state saving

```cpp
// Add to keyfox10_state:
int16_t m_fx_input_l = 0;
int16_t m_fx_input_r = 0;

// Add callback handler:
void sam_snd_sample_out(uint32_t data) {
    m_fx_input_l = int16_t(data >> 16);
    m_fx_input_r = int16_t(data & 0xFFFF);
}
```

### 3. Keyfox10 Driver: Modify FX Waveform Callback

**File**: `src/mame/wersi/keyfox10.cpp`

- [x] Detect WA19=1 (bit 19 of address) in `sam_fx_waveform_r()`
- [x] Return input sample instead of ROM when WA19=1
- [x] Use WA0 (bit 0) to select L/R channel

```cpp
u16 keyfox10_state::sam_fx_waveform_r(offs_t offset) {
    // Check WA19 - if set, return input from sound SAM
    if (offset & 0x80000) {  // bit 19 set
        // WA0 selects channel: 0=Left, 1=Right
        int16_t sample = (offset & 1) ? m_fx_input_r : m_fx_input_l;
        return sample >> 4;  // Convert 16-bit to 12-bit
    }

    // Normal ROM access (existing code)
    // ... existing ROM read logic ...
}
```

### 4. Keyfox10 Driver: Configure Audio Routing

**File**: `src/mame/wersi/keyfox10.cpp`

- [x] Connect sound SAM sample output to callback
- [x] Keep existing speaker routing (both chips output to speakers)

```cpp
// SAM8905 SND - sound generation
SAM8905(config, m_sam_snd, 22'579'200);
m_sam_snd->waveform_read_callback().set(FUNC(keyfox10_state::sam_snd_waveform_r));
m_sam_snd->sample_output_callback().set(FUNC(keyfox10_state::sam_snd_sample_out));
m_sam_snd->add_route(0, "lspeaker", 1.0);      // Dry L
m_sam_snd->add_route(1, "rspeaker", 1.0);      // Dry R

// SAM8905 FX - effects processor
SAM8905(config, m_sam_fx, 22'579'200);
m_sam_fx->waveform_read_callback().set(FUNC(keyfox10_state::sam_fx_waveform_r));
m_sam_fx->add_route(0, "lspeaker", 1.0);       // Wet L
m_sam_fx->add_route(1, "rspeaker", 1.0);       // Wet R
```

### 5. Enable FX SAM Access (if not already)

**File**: `src/mame/wersi/keyfox10.cpp`

- [x] Removed `ENABLE_SAM_FX` conditional compilation
- [x] FX SAM register access at 0xE000-0xE007 now always enabled

## Audio Signal Flow (After Implementation)

```
                    ┌─────────────────────────────────┐
                    │         SAM_SND                 │
                    │    (Sound Generation)           │
                    └──────┬──────────┬───────────────┘
                           │          │
                     Dry L/R    Effect Send L/R
                           │          │
                           ▼          ▼
                      Speakers    ┌─────────────────┐
                        (+)◄──────│     SAM_FX      │
                                  │ (Effects Proc)  │
                                  └─────────────────┘
                                         │
                                    Wet L/R
                                         │
                                         ▼
                                    Speakers
```

## Files Modified

1. `src/devices/sound/sam8905.h` - Added sample output callback and waveform write callback
2. `src/devices/sound/sam8905.cpp` - Added callback initialization, stream_update output, and WWE (Write Waveform Enable) external write
3. `src/mame/wersi/keyfox10.cpp` - Added FX input storage, 32KB SRAM, read/write callbacks, removed ENABLE_SAM_FX guards

## Testing Results

1. ✓ Sound SAM still produces audio (dry path works)
   - Test: MIDI program 3 with note C4
   - Result: Audio produced with RMS 0.091, max amplitude 0.36

2. ✓ FX SAM SRAM read/write working
   - WWE (Write Waveform Enable) writes detected from slot 8
   - SRAM reads at address 0x0000 working
   - WWE triggered by RSP + clearB + WSP instruction combination

3. FX SAM input from sound SAM pending verification
   - Need FX microcode that uses WF >= 0x80 to read input samples

## Open Questions (Resolved)

- ✓ Stereo effect send (multiplexed via WA0)
- ✓ Parallel routing (dry + wet)
- ✓ Address control via WA19, WA0/WA1
- ✓ WWE (Write Waveform Enable) via RSP + clearB + WSP

## Notes

- The 74HC595 shift registers are not explicitly emulated - we directly pass samples
- WSBD0 timing is implicit (samples latched at frame boundaries)
- Both SAMs run at same clock, so 1:1 sample correspondence
- FX SRAM is 32KB (32768 x 16-bit words) for delay/reverb buffers
- Address mapping: WF[6:0] << 8 | PHI[7:0] = 15-bit SRAM address
