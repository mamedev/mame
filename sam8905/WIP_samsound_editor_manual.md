# DREAM S.A. samSOUND Editor Manual - Transcription

**Source**: DREAM S.A. samSOUND V1.0, May 1989
**Transcription date**: 2026-01-28
**Purpose**: Reference for identifying MS4 firmware voice modulation elements, supported parameter ranges, and mapping editor concepts to firmware functions.

## TODO

- [x] Transcribe Section 4-2 SCREEN 2 - Objects Definition (pages 13-15)
- [x] Transcribe Section 4-3 SCREEN 3 - Modulators and Envelope Generators (page 17)
- [x] Transcribe Section 4-4 SCREEN 4 - Keyboard Curves (page 19)
- [ ] Transcribe Figure 6 - Objects Definition screen layout
- [ ] Transcribe Figure 7 - General modulation structure diagram
- [ ] Transcribe Algorithm charts (referenced throughout)
- [ ] Complete firmware function cross-references for all parameters
- [ ] Map editor parameter ranges to D-RAM word bit fields
- [ ] Cross-reference with WIP_ms4_program_extraction.md handler formats

---

## 4-2 SCREEN 2 - Objects Definition (fig 6)

### 4-2-1 Principle

The samSOUND editor works for all types of algorithms, including algorithms you may wish to create yourself. An algorithm is a way of having data interact to produce a sound. The interaction can be typical mathematical formulae like sine waves, phase modulation, frequency modulation, feed-back modulation, amplitude modulation, etc. or combinations of different modulations.

The algorithm parameters are classified into objects:

- Frequency object
- Phase object
- Amplitude object
- Transfer object
- Special object

The purpose of this screen is to define how the objects are processed at key on/off and how they are modulated.

Fig 7 shows the general modulation structure of a sound: up to 5 frequency objects and 5 amplitude objects can be defined on screen 2. They can be modulated by up to 8 modulators defined screen 3. Several objects can be modulated by the same modulator. Part of the modulation is the envelope generation process (up to 5 envelopes for one sound). Several modulators can share the same envelope generator.

> **Firmware note**: The object count limits (5 freq, 5 amp, 8 modulators, 5 envelopes) correspond to the per-voice XRAM page layout documented in WIP_ms4_memory_init.md. Each voice page contains 7 LFO/envelope state blocks at 16 bytes each. The modulator/envelope assignments are stored in the program data parsed by `voice_init_copy_and_envelope` (CODE:AB73).

---

### 4-2-2 Frequency Object

Up to 5 frequency objects can be defined for one sound. The actual names of the frequency objects as defined in the algorithm appears on the screen (normally DPHIx). Please note that the number of frequency objects is defined by the current algorithm, i.e. it is not possible to add or delete a frequency object for a given algorithm from samSOUND.

> **Firmware note**: Frequency objects map to D-RAM DPHI words (18:0). The DPHI value directly controls pitch via `f = 0.084114 × DPHI` Hz. See sam8905_programmers_guide.md for the full DPHI format. Frequency setup is handled by the D-RAM command handler 0x08 (pitch/frequency) documented in WIP_ms4_program_extraction.md.

#### Frequency Object Parameters

| Parameter | Description | Range / Values |
|-----------|-------------|----------------|
| **Detune** | Fine tune | -127 to +127 (-1/2 tone to +1/2 tone) |
| **Detone** | Transposition in half tones | "." = middle A (440Hz), 12 = +1 octave, -24 = -2 octaves |
| **Fix Freq** | Fixed frequency (key-independent) | Hex value, max 3FFFF = 22.05 kHz. Blanks detune/detone fields |
| **Absol. Detune** | Detune type | Blank = relative (chorusing scales with freq), "Y" = absolute (constant offset) |
| **Pitch Bend** | Pitch bend sensitivity | "Y" = sensitive to pitch bend controller |
| **Modul Wheel** | Mod wheel sensitivity | "Y" = sensitive to modulation wheel |
| **EG amount** | Envelope generator depth on frequency | Signed value. Requires modulator + envelope assignment |
| **LFO amount** | LFO depth on frequency | Requires modulator + LFO assignment (SCREEN 3) |
| **Kbd Amount** | Keyboard curve depth on frequency | Requires kbd curve # + curve definition (SCREEN 4). Allows non-chromatic tuning |
| **Dyn amount** | Velocity sensitivity on pitch | Positive = pitch rises with velocity |
| **Touch amount** | After touch depth on frequency | |
| **Kbd curve** | Keyboard curve number | References curve defined on SCREEN 4 |
| **Modul. Nb** | Modulator number | References modulator defined on SCREEN 3 |

> **Firmware cross-references**:
> - **Detune/Detone**: Applied during `voice_note_init_dram` (CODE:A89B) when computing initial DPHI from MIDI note number. Pitch table at `pitch_table_init` (CODE:9B16).
> - **Fix Freq**: Bypasses note-to-DPHI lookup, writes hex value directly to D-RAM DPHI word.
> - **Pitch Bend**: Per-channel pitch bend table at XRAM 0x1194-0x11B3 (16 channels × 2 bytes). Applied by `dram_slot_apply_mod_depth` (CODE:A2E3).
> - **Modul Wheel**: Global mod LFO at XRAM 0x1180-0x1183, per-channel sensitivity at 0x1184-0x1193. Updated by `global_mod_lfo_update` (CODE:A314).
> - **EG/LFO amount**: Routed through modulator system. Modulator output scaled by amount, then applied to DPHI. See `modulation_write_dram` (CODE:9FCD).
> - **Dyn amount**: Velocity sensitivity for pitch in D-RAM handler 0x08 (WIP_ms4_program_extraction.md).
> - **Kbd Amount**: Keyboard curve lookup via `velocity_curve_lookup` (CODE:B3C5).

---

### 4-2-3 Phase Object

One phase object is normally associated with each frequency object.

| Parameter | Description | Values |
|-----------|-------------|--------|
| **Sync** | Phase sync at key-on | Blank = no sync, "I" = apply initial phase from "Phase" field |

> **Firmware note**: "I" sync forces initial phase (PHI) write to D-RAM at key-on. This controls the initial "click" in the sound. Phase is set during `voice_note_init_dram` (CODE:A89B) which writes initial D-RAM words including PHI. The phase accumulator format is PHI (18:0) as documented in sam8905_programmers_guide.md.

---

### 4-2-4 Amplitude Object

Up to 5 amplitude objects can be defined for one sound. The actual names of the amplitude objects as defined in the algorithm appears on the screen (normally DAx). Please note that the number of amplitude objects is defined by the current algorithm, i.e. it is not possible to add or delete an amplitude object for a given algorithm from samSOUND.

Clicking in the amplitude object name (i.e. DA0) allows to mute the corresponding amplitude.

> **Firmware note**: Amplitude objects map to D-RAM AMP words (12 bits, signed Q0.11). The amplitude update loop runs in `dram_slot_amplitude_update` (CODE:A18F) during `periodic_voice_update` (CODE:9BA7). Amplitude envelope processing is done by `envelope_tick_volume` (CODE:A403) with results written by `envelope_write_dram` (CODE:A471).

#### Amplitude Object Parameters

| Parameter | Description | Range / Values |
|-----------|-------------|----------------|
| **Mix L** | Left output level | 0-7 (0 = no output, 7 = full, each step = 6 dB) |
| **Mix R** | Right output level | 0-7 (same scale as Mix L) |
| **Direct** | Direct envelope application | "Y" = first envelope value applied directly at key-on. Blank = rise from 0 |
| **Ev. Int** | Envelope internal flag | "Y" = SAM micro-programmed envelope (no modulator assignable). Read-only |
| **EG Mod** | Envelope generator modulation | "Y" = EG applies to amplitude (requires modulator + EG on SCREEN 3) |
| **LFO Mod** | LFO modulation on amplitude | "Y" = LFO tremolo (requires modulator + LFO on SCREEN 3) |
| **Wheel** | Mod wheel sensitivity | "Y" = amplitude responds to mod wheel |
| **Amplit.** | Base amplitude | Mutable by clicking object name. Modified by modulations, kbd curves, velocity, after touch |
| **LFO Am** | LFO depth on amplitude | Requires modulator assignment + LFO parameters |
| **Kbd Am** | Keyboard curve depth on amplitude | Requires kbd curve # + curve definition (SCREEN 4) |
| **Dyn Am** | Velocity sensitivity on amplitude | |
| **Tch Am** | After touch sensitivity on amplitude | *<< under development >>* |
| **K. Curve** | Keyboard curve number | References SCREEN 4 |
| **Mod. Nb** | Modulator number | References SCREEN 3 |

> **Firmware cross-references**:
> - **Mix L/R**: 3-bit values (0-7) stored in D-RAM MIXL/MIXR fields. Each step = 6 dB attenuation. The SAM8905 hardware applies these as output attenuators. Mix values can be set at voice level or sound level (see Special Object). Applied during `voice_note_init_dram` (CODE:A89B) and modified by `dram_config_apply_velocity` (CODE:B1EC).
> - **Direct**: Controls whether envelope starts from 0 or jumps to initial value. Affects `voice_init_copy_and_envelope` (CODE:AB73).
> - **Ev. Int**: Indicates algorithm uses SAM's internal A-RAM micro-programmed envelope (WA WSP / WM WSP conditional write mechanism). These use the CARRY flag and waveform end detection. See sam8905_programmers_guide.md section on conditional writes.
> - **EG Mod / LFO Mod**: Routed through modulator system to `dram_slot_amplitude_update` (CODE:A18F).
> - **Amplit.**: Base amplitude value (0x00-0x7F). Processed by D-RAM handler 0x10 (amplitude/level). See WIP_ms4_program_extraction.md.
> - **Dyn Am**: Velocity scaling via `dram_config_apply_velocity` (CODE:B1EC) and `velocity_curve_lookup` (CODE:B3C5). D-RAM handler 0x10 flags: bit7=ENV_ENABLE, bit6=NO_VEL_SCALING.

---

### 4-2-5 Special Object

The special object allows to define parameters at the voice level. These parameters may or may not be used by the currently selected algorithm (see algorithm charts for details).

| Parameter | Description | Range / Values |
|-----------|-------------|----------------|
| **WF** | Waveform selector | sinus, ramps, constants |
| **MIXL** | Voice-level left mix | 0-7 (0 = no signal, 7 = 0dB). Steps are 6 dB |
| **MIXR** | Voice-level right mix | 0-7 (same). Recommended average: 3-5 |
| **INT.Mask** | Internal mask | For debugging purposes; do not change |

> **Firmware note**: The WF field maps to the D-RAM WAVE word format: R (ramp mode), I (inversion), SEL (2xPHI / constant / PHI / PHI/2), Z (zero). See sam8905_programmers_guide.md for the complete waveform encoding.
>
> Internal waveforms:
> - **Sinus**: 4096 samples, X = 0.71875 × sin((pi/2048)×PHI + pi/4096)
> - **Ramps**: PHI, 2×PHI, PHI/2
> - **Constants**: 16 values from MAD field (0.0004883 to 0.93799)
>
> MIXL/MIXR at voice level vs. amplitude object level: the SAM hardware has per-slot mix attenuators. "Mix makes sense only if the amplitude object actually generates a sound output, i.e. is not used to modulate another object."

---

### 4-2-6 Transfer Objects (XFER)

Some algorithms may need parameters which are constant values transferred to the SAM memory at key on. Examples are initial and final memory addresses for sampling, constants for filters, mixes for flutes, etc. These parameters are called transfer objects. If the current algorithm requires some of them, then the actual name used in the algorithm appears on the screen, as well as the transfer value defined in the algorithm. This value may be changed.

> **Firmware note**: Transfer objects are written as direct D-RAM word values during `voice_note_init_dram` (CODE:A89B). They correspond to D-RAM handler 0x18 (D-RAM write + velocity mod) documented in WIP_ms4_program_extraction.md. The handler writes a 19-bit word to a specific D-RAM address, with optional velocity-based MIXL/MIXR attenuation.

---

## 4-3 SCREEN 3 - Modulators and Envelope Generators (fig 8)

This screen defines the software modulators which can be applied to frequency and/or amplitude objects, as well as associated envelope generators.

### 4-3-1 Modulators

Up to 8 modulators can be defined for one sound. Modulators not used (i.e. with no EG amount or no LFO amount or not assigned in screen 2) will be automatically deleted in order to save memory space in the final product.

#### Modulator Parameters

| Parameter | Description | Range / Values |
|-----------|-------------|----------------|
| **EG number** | Envelope generator assignment | 0-5. One EG can be shared by several modulators |
| **No sustain** | Ignore EG sustain point | |
| **Free run** | No action at key-off on the EG | |
| **Reset** | Always restart EG from amplitude 0 at key-on | |
| **LFO wave** | LFO waveform shape | sine, up saw, down saw, square, S&H (random) |
| **LFO attack** | LFO onset time | Together with speed and delay, these 3 params define the LFO |
| **LFO speed** | LFO rate | |
| **LFO delay** | LFO onset delay | |
| **Kbd to EG rate** | Keyboard follow curve depth on EG rate | Requires kbd curve assignment. Used for piano-like sounds where lower notes fade slower than higher notes |
| **Kbd to LFO speed** | Keyboard follow curve depth on LFO speed | Requires kbd curve assignment |
| **Kbd to LFO attack** | Keyboard follow curve depth on LFO attack | Same as above for LFO attack |
| **LFO Kbd curve** | Keyboard follow curve for LFO | References curve on SCREEN 4 |
| **EG Kbd curve** | Keyboard follow curve for EG | References curve on SCREEN 5 |

> **Firmware cross-references**:
> - **Modulator system**: Up to 8 modulators stored in voice XRAM page. Each modulator has an LFO state + EG pointer. The `periodic_voice_update` (CODE:9BA7) iterates over active modulators each tick.
> - **LFO wave**: LFO waveform generation in `global_mod_lfo_update` (CODE:A314). The 5 waveforms (sine, up saw, down saw, square, S&H) are generated from the LFO phase accumulator. S&H uses a pseudo-random generator.
> - **LFO attack/speed/delay**: LFO state machine runs during `dram_slot_apply_mod_depth` (CODE:A2E3). Delay holds LFO at zero, attack ramps LFO amplitude from 0 to full, then speed controls the steady-state rate.
> - **EG number**: The EG assignment is parsed during `voice_init_copy_and_envelope` (CODE:AB73). EG sharing allows e.g. one EG to control both amplitude and pitch vibrato depth.
> - **No sustain / Free run / Reset**: These flags modify EG behavior. They are stored as bit flags in the modulator config bytes and checked by `envelope_tick_volume` (CODE:A403).
> - **Kbd to EG rate**: Keyboard curve lookup via `velocity_curve_lookup` (CODE:B3C5) applied as a scaling factor to the EG rate parameter. This makes higher notes decay faster (piano behavior).

---

### 4-3-2 Envelope Generators

Up to 6 envelope generators can be defined (EG0 to EG5), each envelope being up to 9 segments.

A segment is specified by:
- An **amplitude** to be reached (0 to 127)
- A **rate** to reach this amplitude (1 to 99)
- An **indicator** field (character just after the rate)

#### Envelope Segment Editing

| Action | Method |
|--------|--------|
| Insert segment | Click indicator field, type **I** |
| Delete segment | Click indicator field, type **D** |
| Step through segments | Use **>** and **<** keys |

#### Indicator Field Characters

| Indicator | Meaning |
|-----------|---------|
| **S** | Sustain point. Envelope holds at this level until key is released (unless a loop is defined) |
| **L** | Loop point (replaces sustain). Loop length set with + and - keys, shown with "<". Can also be in release portion |
| **E** | Envelope end. Typing E twice deletes all segments to the right |

To add a new envelope generator, click an indicator field below an existing envelope and type I.

> **Firmware cross-references**:
> - **Envelope generator**: The EG state machine is implemented in `envelope_tick_volume` (CODE:A403). Each tick, the current segment's rate determines how quickly the output moves toward the target amplitude. When the target is reached, the next segment begins.
> - **Segment storage**: Each EG's segments are stored in the program data region. The 9-segment limit with amplitude (0-127) and rate (1-99) matches the byte encoding found in the voice page XRAM layout. Amplitude 0-127 maps to 7 bits; rate 1-99 maps to 7 bits.
> - **Sustain (S)**: During sustain, `envelope_tick_volume` holds the output constant until a note-off event advances the EG past the sustain point. Sustain behavior is modified by the modulator's "No sustain" and "Free run" flags.
> - **Loop (L)**: Loop replaces sustain with a repeating segment sequence. The EG cycles between the loop start and current position, useful for sustained evolving timbres (e.g., slow tremolo or filter sweep).
> - **Envelope output**: Written to D-RAM via `envelope_write_dram` (CODE:A471). For amplitude EGs, the output directly scales the D-RAM AMP word. For frequency/pitch EGs, the output is scaled by the "EG amount" parameter before applying to DPHI via `modulation_write_dram` (CODE:9FCD).
> - **6 EGs × 9 segments**: The 8-byte envelope parameter block copied by `voice_init_slots` (CODE:9A2D) at page 0x11 offset 0xE6 contains the initial EG configuration.

---

## 4-4 SCREEN 4 - Keyboard Curves (fig 9)

This screen allows to define up to 8 keyboard curves.

Keyboard curves can be used to modify parameters according to an incoming key number: (de)tuning of a frequency object, amplitude of an amplitude object, rate of an envelope generator, speed and attack time of an LFO.

Up to 20 keyboard points can be defined with a keyboard curve, each point being specified by:
- A **key number** (MIDI number, i.e. 36 for the lowest C of a 5-octave keyboard)
- A **value** ranging from -63 to +63 (0 meaning no correction)

You can add points to a keyboard curve by clicking between Note and Val towards the bottom of the screen and remove points by clicking towards the top of the screen. The added points will follow the last increments (note and value) entered.

Once you have defined a keyboard curve, you should return to screen 2 or 3 to assign it and specify some amount.

#### Example

Suppose you want a given amplitude object to sound only in the lowest octave of a 5-octave keyboard, then the corresponding keyboard curve should look like:

```
Note  Val
 36     0
 48     0
 49   -63
```

> **Firmware cross-references**:
> - **Keyboard curve lookup**: Implemented by `velocity_curve_lookup` (CODE:B3C5). The function takes a MIDI note number and looks up the corresponding value from the curve's point table, interpolating between defined points.
> - **Curve storage**: Up to 8 curves × 20 points × 2 bytes (note + value) stored in the program data XRAM region. The -63 to +63 range (7-bit signed) is stored as a signed byte.
> - **Application points**: Keyboard curves are applied at key-on during `voice_note_init_dram` (CODE:A89B) and `voice_init_copy_and_envelope` (CODE:AB73). They affect:
>   - Frequency objects: pitch offset via "Kbd Amount" (non-chromatic tuning)
>   - Amplitude objects: level scaling via "Kbd Am"
>   - EG rate: decay time scaling via "Kbd to EG rate" (piano key follow)
>   - LFO speed/attack: rate scaling via "Kbd to LFO speed" / "Kbd to LFO attack"
> - **The example curve**: Notes 36-48 get value 0 (no modification), note 49+ gets -63 (maximum attenuation). This effectively mutes the amplitude object above the first octave, creating a keyboard split.

---

## Firmware Function Reference

Summary of all MS4 firmware functions referenced in this document:

| Function | Address | Role |
|----------|---------|------|
| `init_pitch_and_voices` | CODE:98AD | Initialize pitch tables and voice system |
| `voice_init_slots` | CODE:9A2D | Allocate SAM D-RAM slots for a voice |
| `pitch_table_init` | CODE:9B16 | Build note-to-DPHI lookup table |
| `periodic_voice_update` | CODE:9BA7 | Main timer-driven voice update loop |
| `modulation_write_dram` | CODE:9FCD | Write modulation result to D-RAM |
| `dram_slot_amplitude_update` | CODE:A18F | Update slot amplitude from envelope |
| `dram_slot_apply_mod_depth` | CODE:A2E3 | Apply mod wheel / LFO depth to pitch |
| `global_mod_lfo_update` | CODE:A314 | Update global modulation LFO |
| `dram_slot_portamento_update` | CODE:A33E | Smooth pitch glide update |
| `envelope_tick_volume` | CODE:A403 | Advance envelope generator one tick |
| `envelope_write_dram` | CODE:A471 | Write envelope output to D-RAM |
| `voice_deactivate` | CODE:A69C | Deactivate voice, clear slots |
| `voice_kill_channel` | CODE:A785 | Kill all voices on MIDI channel |
| `voice_trigger_note` | CODE:A834 | Handle MIDI note-on |
| `voice_note_init_dram` | CODE:A89B | Initialize D-RAM words for new note |
| `voice_init_next_slot` | CODE:AB40 | Set up next D-RAM slot in voice |
| `voice_init_copy_and_envelope` | CODE:AB73 | Copy params + init envelopes |
| `voice_assign_algorithm` | CODE:B4BF | Load algorithm to SAM A-RAM |
| `dram_config_apply_velocity` | CODE:B1EC | Apply velocity scaling to D-RAM config |
| `velocity_curve_lookup` | CODE:B3C5 | Velocity-to-level curve table |
| `voice_pages_clear` | CODE:B70B | Clear voice page XRAM regions |
| `find_active_voice` | CODE:D417 | Search for active voice in table |

