# SAM8905 Jupyter Interpreter - Implementation Plan

## Overview

Implement a Python-based SAM8905 interpreter that can run in Jupyter notebooks for debugging, visualization, and algorithm development.

## Requirements

1. **Input Formats**
   - Raw 15-bit instruction words (hex or integer arrays)
   - SAM assembler text format (decoded by existing sam8905_aram_decoder.py)

2. **Execution**
   - Execute a configurable number of sample frames
   - Support single-step execution for debugging
   - Support running a single slot or all 16 slots

3. **Output Visualization**
   - Waveform plot (L/R channels via matplotlib)
   - WAV file export

4. **State Inspection**
   - Print/log SAM register state (A, B, X, Y, PHI, WF, MUL, CARRY, etc.)
   - Track D-RAM changes between frames (for future visualization)
   - Optionally print instruction trace with register values

## Architecture

```
sam8905/
├── sam8905_aram_decoder.py    # [EXISTING] Instruction decoder
├── sam8905_interpreter.py     # [NEW] Core interpreter engine
└── notebooks/
    └── sam8905_sandbox.ipynb  # [NEW] Example notebook
```

## Implementation Details

### 1. SAM8905State Class (Data Container)

Holds all chip state - mirrors C++ implementation:

```python
@dataclass
class SAM8905State:
    # Registers
    a: int = 0           # 19-bit A register
    b: int = 0           # 19-bit B register
    x: int = 0           # 12-bit X (multiplier input from waveform)
    y: int = 0           # 12-bit Y (multiplier input from bus)
    phi: int = 0         # 12-bit phase register
    wf: int = 0          # 9-bit waveform select register
    mul_result: int = 0  # 19-bit multiplication result

    # Flags
    carry: bool = False
    clear_rqst: bool = False
    int_mod: bool = False

    # Output mix
    mix_l: int = 0       # 3-bit left attenuation select
    mix_r: int = 0       # 3-bit right attenuation select

    # Accumulators (reset each frame)
    l_acc: int = 0
    r_acc: int = 0

    # Memory (16 slots × 16 words = 256 words)
    dram: list = field(default_factory=lambda: [0] * 256)
    aram: list = field(default_factory=lambda: [0] * 256)
```

### 2. SAM8905Interpreter Class (Execution Engine)

```python
class SAM8905Interpreter:
    def __init__(self):
        self.state = SAM8905State()
        self.history = []  # List of (frame_num, dram_changes, registers)
        self.trace_enabled = False

    def load_aram(self, aram: list, offset: int = 0):
        """Load A-RAM program (up to 256 instructions)"""

    def load_dram(self, slot: int, words: list):
        """Load D-RAM for a specific slot (16 words)"""

    def execute_instruction(self, slot_idx: int, inst: int, pc: int) -> dict:
        """Execute single instruction, return state changes"""

    def execute_frame(self) -> tuple[int, int]:
        """Execute one sample frame (all active slots), return (L, R)"""

    def run(self, num_frames: int) -> np.ndarray:
        """Run for N frames, return waveform array [N, 2]"""

    def get_state_snapshot(self) -> dict:
        """Return current register state as dict for display"""
```

### 3. Core Execute Logic (Port from C++)

The `execute_instruction` method follows `src/devices/sound/sam8905.cpp:execute_cycle()`:

1. **Emitter Select** (bits 10-9):
   - `00 (RM)`: bus = dram[slot*16 + mad]
   - `01 (RADD)`: bus = (a + b) & 0x7FFFF
   - `10 (RP)`: bus = mul_result
   - `11 (RSP)`: bus = 0 (NOP for data path)

2. **Receivers** (active-low except WSP):
   - `WA`: a = bus, set clear_rqst (with WSP: special truth table)
   - `WB`: b = bus
   - `WM`: dram[addr] = bus (with WSP: conditional on carry/clear_rqst)
   - `WPHI`: phi = (bus >> 7) & 0xFFF (with WSP: wf = 0x100)
   - `WXY`: y = (bus >> 7) & 0xFFF, x = waveform(wf, phi), mul
   - `clearB`: b = 0 (with RSP+WSP: external write)
   - `WWF`: wf = (bus >> 9) & 0x1FF
   - `WACC`: accumulate mul_result * attenuation to l_acc/r_acc

3. **Carry Calculation** (Table 3 from programmer's guide):
   - B positive (bit 18=0): carry = 19-bit overflow
   - B negative (bit 18=1): carry = result sign bit = 0

### 4. Waveform Generation

Internal waveforms (when wf & 0x100):
- Sin mode: x = 0.71875 * sin(PI/2048 * phi + PI/4096) * 2048
- Ramp modes based on SEL bits (wf >> 2) & 3:
  - SEL=0: 2x PHI triangle
  - SEL=1: Constant from MAD
  - SEL=2: PHI ramp
  - SEL=3: PHI/2 ramp

External waveforms: callback for ROM lookup (stubbed or user-provided)

### 5. Visualization Functions

```python
def plot_waveform(samples: np.ndarray, sample_rate: int = 44100):
    """Plot stereo waveform with matplotlib"""

def export_wav(samples: np.ndarray, filename: str, sample_rate: int = 44100):
    """Export to WAV file"""

def print_state(state: SAM8905State, slot: int = None):
    """Pretty-print register state"""

def print_dram_changes(history: list):
    """Show D-RAM word changes between frames"""
```

### 6. Jupyter Integration Example

```python
from sam8905_interpreter import SAM8905Interpreter, plot_waveform

# Create interpreter
sam = SAM8905Interpreter()

# Load reverb algorithm (ALG 2 from Keyfox10)
aram_alg2 = [0x79F7, 0x207F, ...]  # 32 instructions
sam.load_aram(aram_alg2, offset=64)  # Load at ALG 2 position

# Initialize D-RAM for slot 4
dram_slot4 = [0x00000, 0x50080, ...]  # 16 words
sam.load_dram(slot=4, words=dram_slot4)

# Run 1000 samples
sam.trace_enabled = True
samples = sam.run(num_frames=1000)

# Visualize
plot_waveform(samples)

# Inspect state
print(sam.get_state_snapshot())
```

## Files to Create

| File | Purpose |
|------|---------|
| `sam8905/sam8905_interpreter.py` | Core interpreter (SAM8905State, SAM8905Interpreter classes) |
| `sam8905/notebooks/sam8905_sandbox.ipynb` | Example notebook with usage patterns |

## Files to Reuse

| File | What to Import |
|------|----------------|
| `sam8905/sam8905_aram_decoder.py` | `decode_instruction()`, `format_instruction()`, `DecodedInstruction` |

## Constants to Port from C++

```python
MASK19 = 0x7FFFF  # 19-bit mask
MASK12 = 0xFFF    # 12-bit mask

# Multiplier constants (Q0.11 format, Appendix I)
CONSTANTS = [
    0x001, 0x081, 0x101, 0x181, 0x201, 0x281, 0x301, 0x381,
    0x401, 0x481, 0x501, 0x581, 0x601, 0x681, 0x701, 0x781
]

# Mix attenuation (dB lookup)
MIX_ATTEN = [0, 16, 32, 64, 128, 256, 512, 1024]
```

## Verification Plan

1. **Unit tests**: Compare instruction decoding with sam8905_aram_decoder.py
2. **Register trace**: Compare A/B/X/Y/MUL values with MAME `-oslog` output
3. **Waveform**: Generate sine wave (internal waveform) and verify frequency/amplitude
4. **Reverb**: Load ALG 2 (reverb) program, compare output against MAME FX output

## Implementation Tasks

- [ ] Create `SAM8905State` dataclass with all registers
- [ ] Implement `execute_instruction()` - port execute_cycle from C++
- [ ] Implement internal waveform generation (sin, ramps)
- [ ] Implement `execute_frame()` - iterate slots, accumulate output
- [ ] Implement `run()` - generate sample array
- [ ] Add visualization: `plot_waveform()`, `export_wav()`
- [ ] Add state inspection: `print_state()`, `get_state_snapshot()`
- [ ] Add D-RAM change tracking
- [ ] Create example Jupyter notebook
- [ ] Test against MAME output traces

## Dependencies

```python
# Required
numpy        # Array operations, waveform storage
matplotlib   # Waveform plotting
scipy.io     # WAV file export (scipy.io.wavfile)

# From project
sam8905_aram_decoder  # Instruction decoding
```
