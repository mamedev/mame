# WIP: MS4 Algorithm Analysis Notebooks

## Overview

Create tools to extract MS4 program data and analyze each algorithm in dedicated Jupyter notebooks.
This provides a foundation for implementing envelope/LFO modulators based on firmware analysis.

## MS4 Algorithm Summary

From `ms4_parse_programs.py` analysis:
- **Program pointer table**: 0x0040, 247 entries
- **Valid programs**: 229 (18 SILENCE entries at 0x022E skipped)
- **Algorithms**: 17 unique (at addresses 0x027A through 0x067A)

| Algorithm | ROM Address | Programs | Description |
|-----------|-------------|----------|-------------|
| ALG_027A  | 0x027A      | 57       | Main voice algorithm (piano, strings, brass, etc.) |
| ALG_02BA  | 0x02BA      | 124      | Percussion/plucked + many variants |
| ALG_02FA  | 0x02FA      | 2        | String/voice special (dstring3, dkvoi1) |
| ALG_033A  | 0x033A      | 7        | Electric piano variants (dxketron, epiano9, etc.) |
| ALG_037A  | 0x037A      | 1        | xt_pia1 |
| ALG_03BA  | 0x03BA      | 10       | Guitar/bass variants (medium, gibson, jazzy2, etc.) |
| ALG_03FA  | 0x03FA      | 7        | Organ/synth variants (solosint, church3, vibes, etc.) |
| ALG_043A  | 0x043A      | 2        | Brass (trombone, dopenbra) |
| ALG_047A  | 0x047A      | 3        | Sweet/mellow (dsweetb, dboweds, dsm100) |
| ALG_04BA  | 0x04BA      | 5        | Synth pads (xt_sybr3, space, whalead, etc.) |
| ALG_04FA  | 0x04FA      | 1        | mb_pad1 |
| ALG_053A  | 0x053A      | 1        | strings |
| ALG_057A  | 0x057A      | 1        | analog |
| ALG_05BA  | 0x05BA      | 3        | Organ (dorgan5, dsmith, d1684) |
| ALG_05FA  | 0x05FA      | 3        | Organ (dchurch2, drotoorg, dorgan2) |
| ALG_063A  | 0x063A      | 1        | whistle |
| ALG_067A  | 0x067A      | 1        | wind |

Algorithm addresses are spaced 0x40 bytes (64 bytes = 32 words) apart.
Note: 0x06BA is NOT a valid algorithm (garbage data after 0x067A).

## Implementation Steps

- [x] Analyze existing `ms4_parse_programs.py` to understand data structures
- [x] Add export function to generate `ms4_programs.py` with programs grouped by algorithm
- [x] Create `notebooks/ms4_alg_027a.ipynb` - main voice algorithm analysis
- [x] Create `notebooks/ms4_alg_02ba.ipynb` - percussion algorithm analysis
- [x] Create `notebooks/ms4_alg_02fa.ipynb` - special voice algorithm analysis
- [x] Test notebooks can load and run programs from extracted data
- [x] Add `decode_dram_init()` to generate per-program initial D-RAM tables
- [x] Export dram_init, pitch_params, amp_params, waveform_words per program
- [x] Update notebooks to use program dram_init values
- [ ] Implement envelope/LFO modulators to update D-RAM dynamically

## Testing Results

Algorithm 027A test with 440Hz sine:
- D-RAM setup requires: D[0]=phase_inc, D[4]/D[10]/D[15]=waveform_select
- Output range: [-778, 777] (three WACC points contribute)
- Verified phase accumulator updates correctly each sample

## Data Export Format

The `ms4_programs.py` file contains:

```python
# A-RAM algorithm data (32 x 15-bit words)
ALGORITHMS = {
    '027A': [...],  # 32 instruction words
    '02BA': [...],
    '02FA': [...],
}

# Programs grouped by algorithm
PROGRAMS_027A = [
    {
        'idx': 0,
        'name': 'dpiano27',
        'addr': 0x926A,
        'flags': 0x11,
        'slot_count': 1,
        'complex_init': False,
        'dram_entry0': {'word': 0xA401, 'addr_nibble': 8, 'mix_bits': 5},
        'voice_slots': [
            {'ptr': 0x18C8, 'data': [0x12, 0x09, 0x40, ...]},
        ],
        'dram_stream': [0x08, 0x00, ...],  # Raw stream bytes

        # Decoded D-RAM init values (new):
        'dram_init': [0x00000, ...],  # 16 x 19-bit D-RAM words
        'pitch_params': {'note_offset': 192, 'fine_tune': 0, 'vel_sens': 0},
        'amp_params': {'level': 0, 'amp': 0x7F, 'vel_sens': 0, 'env_ctrl': 0x00},
        'waveform_words': [4, 10, 15],  # D-RAM indices for waveform select
    },
    ...
]
```

### D-RAM Init Fields

- `dram_init`: 16-word array of initial D-RAM values (19-bit each)
  - Waveform words are set to internal sine (0x20000 = 0x100 << 9)
  - Phase increment (D[0]) is 0 - needs runtime calculation from MIDI note
  - Amplitude words contain (amp << 7) | mix format where available

- `pitch_params`: Runtime pitch calculation parameters
  - `note_offset`: Added to MIDI note for octave transposition
  - `fine_tune`: 16-bit fine tuning value
  - `vel_sens`: Velocity sensitivity for pitch (0 = disabled)

- `amp_params`: Runtime amplitude/envelope parameters
  - `level`: Base level value
  - `amp`: Amplitude value (0x00-0x7F)
  - `vel_sens`: Velocity sensitivity for amplitude
  - `env_ctrl`: Envelope control flags (bit7=ENV, bit6=NO_VEL_SCALE, etc.)

- `waveform_words`: List of D-RAM word indices that need waveform select
  - Algorithm-specific: 027A uses [4, 10, 15], 02BA uses [15], etc.

## Notebook Structure (per algorithm)

1. **Setup cell**: Import interpreter, load algorithm A-RAM
2. **Program selector**: Dropdown/list to select program by name
3. **D-RAM visualization**: Show initial D-RAM state for selected program
4. **Run cell**: Execute samples and plot waveform
5. **Analysis cells**: D-RAM usage, signal flow, etc.

## Future Work: Envelope/LFO Modulators

The extracted voice slot data contains envelope parameters:
- `env_tbl`: Pointer to envelope segment table (3-byte entries)
- `ctrl`: Envelope enable flags (bit 7 = ENV, bit 6 = LSIGN)
- `atk`, `depth`, `rate`: Envelope shape parameters

The D-RAM command stream handlers:
- `0x08` (pitch): Portamento, pitch bend, modulation
- `0x10` (amplitude): ADSR envelope, velocity scaling, level modulation

Implementation will require:
1. Decode envelope segment tables from ROM
2. Implement envelope generator (attack/decay/sustain/release)
3. Implement LFO for pitch/amplitude modulation
4. Update D-RAM words per sample based on modulator outputs

## Files

- `sam8905/ms4_parse_programs.py` - Parser with export function
- `sam8905/ms4_programs.py` - Exported program data (generated)
- `sam8905/notebooks/ms4_alg_027a.ipynb` - Main algorithm notebook
- `sam8905/notebooks/ms4_alg_02ba.ipynb` - Percussion algorithm notebook
- `sam8905/notebooks/ms4_alg_02fa.ipynb` - Special voice algorithm notebook
