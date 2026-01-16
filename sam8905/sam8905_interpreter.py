#!/usr/bin/env python3
"""
SAM8905 Interpreter for Jupyter Notebooks

A Python implementation of the SAM8905 DSP for debugging and visualization.
Ports the execution logic from src/devices/sound/sam8905.cpp.
"""

import math
from dataclasses import dataclass, field
from typing import Optional, Callable, List, Tuple, Dict, Any
import numpy as np

from sam8905_aram_decoder import decode_instruction, format_instruction

# Constants
MASK19 = 0x7FFFF  # 19-bit mask
MASK12 = 0xFFF    # 12-bit mask

# Multiplier constants (Q0.11 format, Appendix I)
CONSTANTS = [
    0x001, 0x081, 0x101, 0x181, 0x201, 0x281, 0x301, 0x381,
    0x401, 0x481, 0x501, 0x581, 0x601, 0x681, 0x701, 0x781
]

# Mix attenuation lookup (dB): 000=mute, 001=-36dB, ..., 111=0dB
MIX_ATTEN = [0, 16, 32, 64, 128, 256, 512, 1024]


def sign_extend_12(val: int) -> int:
    """Sign-extend 12-bit value to Python int."""
    if val & 0x800:
        return val | ~0xFFF
    return val


def sign_extend_19(val: int) -> int:
    """Sign-extend 19-bit value to Python int."""
    if val & 0x40000:
        return val | ~MASK19
    return val


@dataclass
class SAM8905State:
    """Holds all SAM8905 chip state."""
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

    # Control
    control_reg: int = 0  # Bit 3 = SSR (sample rate select)

    # Memory
    dram: List[int] = field(default_factory=lambda: [0] * 256)
    aram: List[int] = field(default_factory=lambda: [0] * 256)

    def copy(self) -> 'SAM8905State':
        """Create a deep copy of state."""
        new_state = SAM8905State(
            a=self.a, b=self.b, x=self.x, y=self.y,
            phi=self.phi, wf=self.wf, mul_result=self.mul_result,
            carry=self.carry, clear_rqst=self.clear_rqst, int_mod=self.int_mod,
            mix_l=self.mix_l, mix_r=self.mix_r,
            l_acc=self.l_acc, r_acc=self.r_acc,
            control_reg=self.control_reg,
            dram=self.dram.copy(),
            aram=self.aram.copy()
        )
        return new_state


class SAM8905Interpreter:
    """SAM8905 DSP interpreter for debugging and visualization."""

    def __init__(self):
        self.state = SAM8905State()
        self.history: List[Dict[str, Any]] = []
        self.trace_enabled = False
        self.trace_output: List[str] = []

        # External waveform callback: (address) -> 12-bit sample
        self.waveform_read: Optional[Callable[[int], int]] = None

        # External waveform write callback: (address, data) -> None
        # Used for SRAM writes via WWE (Waveform Write Enable)
        self.waveform_write: Optional[Callable[[int, int], None]] = None

    def reset(self):
        """Reset interpreter state."""
        self.state = SAM8905State()
        self.history = []
        self.trace_output = []

    def load_aram(self, aram: List[int], offset: int = 0):
        """Load A-RAM program.

        Args:
            aram: List of 15-bit instruction words
            offset: Starting address in A-RAM (0-255)
        """
        for i, inst in enumerate(aram):
            if offset + i < 256:
                self.state.aram[offset + i] = inst & 0x7FFF

    def load_dram(self, slot: int, words: List[int]):
        """Load D-RAM for a specific slot.

        Args:
            slot: Slot index (0-15)
            words: List of up to 16 19-bit words
        """
        base = slot * 16
        for i, word in enumerate(words):
            if i < 16:
                self.state.dram[base + i] = word & MASK19

    def get_constant(self, mad: int) -> int:
        """Get multiplier constant from MAD field."""
        return CONSTANTS[mad & 0xF]

    def get_waveform(self, wf: int, phi: int, mad: int) -> int:
        """Generate waveform sample.

        Args:
            wf: 9-bit waveform register
            phi: 12-bit phase
            mad: MAD field for constant selection

        Returns:
            12-bit signed waveform value
        """
        phi = phi & MASK12
        internal = (wf & 0x100) != 0

        if internal:
            # Bit 8=1: Internal waveform
            z_bit = wf & 0x1
            if z_bit:
                return 0

            ramp_mode = (wf & 0x40) != 0
            if not ramp_mode:
                # Sinus: X = 0.71875 * sin((PI/2048) * PHI + PI/4096)
                angle = (math.pi / 2048.0) * phi + (math.pi / 4096.0)
                return int(0.71875 * math.sin(angle) * 2048.0)
            else:
                # Ramps based on SEL bits
                sel = (wf >> 2) & 3
                if sel == 0:
                    # 2x PHI triangle
                    if phi < 1024:
                        return phi * 2
                    elif phi < 3072:
                        return (phi * 2) - 4096
                    else:
                        return (phi * 2) - 8192
                elif sel == 1:
                    # Constant from MAD
                    return self.get_constant(mad)
                elif sel == 2:
                    # PHI ramp
                    if phi < 2048:
                        return phi
                    else:
                        return phi - 4096
                elif sel == 3:
                    # PHI/2 ramp
                    if phi < 2048:
                        return phi // 2
                    else:
                        return (phi // 2) - 2048
        else:
            # External memory access
            if self.waveform_read is not None:
                # Build 20-bit address: WF[7:0] | PHI[11:0]
                addr = ((wf & 0xFF) << 12) | phi
                sample = self.waveform_read(addr)
                # Sign-extend 12-bit
                return sign_extend_12(sample & MASK12)
            return 0

        return 0

    def update_carry(self):
        """Update carry flag based on current A and B values."""
        b_neg = (self.state.b & 0x40000) != 0
        sum_val = self.state.a + self.state.b

        if not b_neg:
            # B positive: carry = 19-bit overflow
            self.state.carry = sum_val > MASK19
        else:
            # B negative: carry = result is positive (sign bit = 0)
            self.state.carry = ((sum_val & MASK19) & 0x40000) == 0

    def execute_instruction(self, slot_idx: int, inst: int, pc: int) -> Dict[str, Any]:
        """Execute a single instruction.

        Args:
            slot_idx: Current slot index (0-15)
            inst: 15-bit instruction word
            pc: Program counter (for tracing)

        Returns:
            Dict with state changes for tracing
        """
        changes = {}
        s = self.state

        # Decode instruction fields
        mad = (inst >> 11) & 0xF
        emitter_sel = (inst >> 9) & 0x3
        wsp = (inst & 0x100) != 0

        dram_addr = (slot_idx << 4) | mad

        # Emitter: determine bus value
        if emitter_sel == 0:  # RM
            bus = s.dram[dram_addr]
        elif emitter_sel == 1:  # RADD
            bus = (s.a + s.b) & MASK19
        elif emitter_sel == 2:  # RP
            bus = s.mul_result
        else:  # RSP
            bus = 0

        # Initial carry state
        self.update_carry()

        # Receivers (active low except WSP)

        # WA (bit 7)
        if not (inst & 0x80):
            wphi_active = not (inst & 0x10)

            if wsp and not wphi_active:
                # WA WSP special logic
                wave = (bus >> 9) & 0x1FF
                final_wave = bus & 0x1FF
                end_bit = (bus & 0x40000) != 0
                wf_match = (wave == final_wave)

                if not s.carry:
                    s.a = 0
                    s.clear_rqst = False
                    s.int_mod = True
                elif not wf_match:
                    s.a = 0x200
                    s.clear_rqst = False
                    s.int_mod = True
                else:
                    s.a = 0
                    s.int_mod = True
                    s.clear_rqst = end_bit
                changes['a'] = s.a
            else:
                # Normal WA
                s.a = bus
                s.clear_rqst = True
                s.int_mod = False
                changes['a'] = s.a

            self.update_carry()

        # WB (bit 6)
        if not (inst & 0x40):
            s.b = bus
            changes['b'] = s.b
            self.update_carry()

        # WM (bit 5)
        if not (inst & 0x20):
            write_enable = True
            if wsp:
                # WM WSP conditional
                if not s.clear_rqst:
                    write_enable = False
                elif s.carry:
                    write_enable = False

            if write_enable:
                old_val = s.dram[dram_addr]
                s.dram[dram_addr] = bus
                if old_val != bus:
                    changes[f'dram[{dram_addr}]'] = {'old': old_val, 'new': bus}

        # WPHI (bit 4)
        if not (inst & 0x10):
            s.phi = (bus >> 7) & MASK12
            changes['phi'] = s.phi
            if wsp:
                s.wf = 0x100  # Force internal sinus
                changes['wf'] = s.wf

        # WXY (bit 3)
        if not (inst & 0x08):
            s.y = (bus >> 7) & MASK12
            s.x = self.get_waveform(s.wf, s.phi, mad) & MASK12
            changes['x'] = s.x
            changes['y'] = s.y

            # Calculate multiplication (Q0.11 * Q0.11 -> Q0.18)
            x_signed = sign_extend_12(s.x)
            y_signed = sign_extend_12(s.y)
            product = x_signed * y_signed
            s.mul_result = ((product + 8) >> 4) & MASK19
            changes['mul'] = s.mul_result

            if wsp:
                s.mix_l = (bus >> 3) & 0x7
                s.mix_r = bus & 0x7
                changes['mix_l'] = s.mix_l
                changes['mix_r'] = s.mix_r

        # clearB (bit 2)
        if not (inst & 0x04):
            s.b = 0
            changes['b'] = 0
            self.update_carry()

            # WWE (Write Waveform Enable) - triggered by RSP + clearB + WSP
            # Per SAM8905 datasheet Section 9: Data to write is Y register
            if emitter_sel == 3 and wsp and self.waveform_write is not None:
                # Only write externally when WF indicates external memory (WF < 0x80)
                # WF >= 0x80 is input sample address space, WF >= 0x100 is internal
                if (s.wf & 0x1FF) < 0x80:
                    # Build 15-bit address: WF[6:0] << 8 | PHI[11:4]
                    ext_addr = ((s.wf & 0x7F) << 8) | ((s.phi >> 4) & 0xFF)
                    # Write data is Y register (12-bit signed)
                    ext_data = sign_extend_12(s.y)
                    self.waveform_write(ext_addr, ext_data)
                    changes['wwe'] = {'addr': ext_addr, 'data': ext_data}

        # WWF (bit 1)
        if not (inst & 0x02):
            s.wf = (bus >> 9) & 0x1FF
            changes['wf'] = s.wf

        # WACC (bit 0)
        if not (inst & 0x01):
            # Accumulate with dB attenuation
            signed_mul = sign_extend_19(s.mul_result)
            l_contrib = (signed_mul * MIX_ATTEN[s.mix_l]) >> 10
            r_contrib = (signed_mul * MIX_ATTEN[s.mix_r]) >> 10
            s.l_acc += l_contrib
            s.r_acc += r_contrib
            changes['l_acc'] = s.l_acc
            changes['r_acc'] = s.r_acc

        # Trace output
        if self.trace_enabled:
            decoded = decode_instruction(inst)
            line = f"S{slot_idx:02d} PC{pc:02d}: {format_instruction(decoded)}"
            if changes:
                change_str = ', '.join(f"{k}=0x{v:05X}" if isinstance(v, int) else f"{k}={v}"
                                       for k, v in changes.items()
                                       if not isinstance(v, dict))
                line += f"  -> {change_str}"
            self.trace_output.append(line)

        return changes

    def execute_frame(self, active_slots: Optional[List[int]] = None) -> Tuple[int, int]:
        """Execute one sample frame.

        Args:
            active_slots: Optional list of slot indices to run.
                         If None, runs all non-idle slots.

        Returns:
            (left_sample, right_sample) tuple
        """
        s = self.state

        # Reset accumulators
        s.l_acc = 0
        s.r_acc = 0

        # SSR mode (bit 3): 0 = 44.1kHz, 1 = 22.05kHz
        ssr_mode = (s.control_reg & 0x08) != 0

        slots_to_run = active_slots if active_slots is not None else range(16)

        for slot in slots_to_run:
            # Check idle bit in word 15
            param15 = s.dram[(slot << 4) | 15]
            if param15 & 0x800:  # Bit 11 = IDLE
                continue

            if ssr_mode:
                # 22.05kHz: 4 algorithms x 64 instructions
                alg = (param15 >> 9) & 0x3
                pc_start = alg << 6
                inst_count = 64
            else:
                # 44.1kHz: 8 algorithms x 32 instructions
                alg = (param15 >> 8) & 0x7
                pc_start = alg << 5
                inst_count = 32

            # Execute instructions (skip last 2 reserved)
            for pc in range(inst_count - 2):
                inst = s.aram[pc_start + pc]
                self.execute_instruction(slot, inst, pc)

        return s.l_acc, s.r_acc

    def run(self, num_frames: int, active_slots: Optional[List[int]] = None) -> np.ndarray:
        """Run for multiple frames and collect output.

        Args:
            num_frames: Number of sample frames to generate
            active_slots: Optional list of slots to run

        Returns:
            numpy array of shape (num_frames, 2) with L/R samples
        """
        samples = np.zeros((num_frames, 2), dtype=np.int32)

        for i in range(num_frames):
            l, r = self.execute_frame(active_slots)
            # Clamp to 16-bit signed range
            samples[i, 0] = max(-32768, min(32767, l))
            samples[i, 1] = max(-32768, min(32767, r))

            if self.trace_enabled and self.history is not None:
                self.history.append({
                    'frame': i,
                    'l': l, 'r': r,
                    'dram_snapshot': self.state.dram.copy()
                })

        return samples

    def get_state_snapshot(self) -> Dict[str, Any]:
        """Return current register state as dict."""
        s = self.state
        return {
            'a': s.a,
            'b': s.b,
            'x': s.x,
            'y': s.y,
            'phi': s.phi,
            'wf': s.wf,
            'mul_result': s.mul_result,
            'carry': s.carry,
            'clear_rqst': s.clear_rqst,
            'int_mod': s.int_mod,
            'mix_l': s.mix_l,
            'mix_r': s.mix_r,
            'l_acc': s.l_acc,
            'r_acc': s.r_acc,
        }


# ============================================================
# Visualization Functions
# ============================================================

def plot_waveform(samples: np.ndarray, sample_rate: int = 44100,
                  title: str = "SAM8905 Output", figsize: Tuple[int, int] = (12, 4)):
    """Plot stereo waveform.

    Args:
        samples: Array of shape (N, 2) with L/R samples
        sample_rate: Sample rate in Hz
        title: Plot title
        figsize: Figure size tuple
    """
    import matplotlib.pyplot as plt

    time_ms = np.arange(len(samples)) / sample_rate * 1000

    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=figsize, sharex=True)

    ax1.plot(time_ms, samples[:, 0], 'b-', linewidth=0.5)
    ax1.set_ylabel('Left')
    ax1.set_title(title)
    ax1.grid(True, alpha=0.3)
    ax1.set_ylim(-35000, 35000)

    ax2.plot(time_ms, samples[:, 1], 'r-', linewidth=0.5)
    ax2.set_ylabel('Right')
    ax2.set_xlabel('Time (ms)')
    ax2.grid(True, alpha=0.3)
    ax2.set_ylim(-35000, 35000)

    plt.tight_layout()
    return fig


def export_wav(samples: np.ndarray, filename: str, sample_rate: int = 44100):
    """Export samples to WAV file.

    Args:
        samples: Array of shape (N, 2) with L/R samples
        filename: Output filename
        sample_rate: Sample rate in Hz
    """
    from scipy.io import wavfile

    # Convert to 16-bit int
    samples_16 = samples.astype(np.int16)
    wavfile.write(filename, sample_rate, samples_16)
    print(f"Wrote {filename}: {len(samples)} samples at {sample_rate} Hz")


def print_state(state: SAM8905State, slot: Optional[int] = None):
    """Pretty-print register state.

    Args:
        state: SAM8905State object
        slot: Optional slot to show D-RAM for
    """
    print("=" * 50)
    print("SAM8905 Register State")
    print("=" * 50)
    print(f"  A = 0x{state.a:05X}  ({sign_extend_19(state.a):+d})")
    print(f"  B = 0x{state.b:05X}  ({sign_extend_19(state.b):+d})")
    print(f"  X = 0x{state.x:03X}  ({sign_extend_12(state.x):+d})")
    print(f"  Y = 0x{state.y:03X}  ({sign_extend_12(state.y):+d})")
    print(f"  PHI = 0x{state.phi:03X}  ({state.phi})")
    print(f"  WF = 0x{state.wf:03X}")
    print(f"  MUL = 0x{state.mul_result:05X}  ({sign_extend_19(state.mul_result):+d})")
    print()
    print(f"  CARRY = {state.carry}")
    print(f"  CLEAR_RQST = {state.clear_rqst}")
    print(f"  INT_MOD = {state.int_mod}")
    print()
    print(f"  MIX_L = {state.mix_l}  MIX_R = {state.mix_r}")
    print(f"  L_ACC = {state.l_acc:+d}  R_ACC = {state.r_acc:+d}")

    if slot is not None:
        print()
        print(f"D-RAM Slot {slot}:")
        base = slot * 16
        for i in range(16):
            val = state.dram[base + i]
            print(f"  D[{i:2d}] = 0x{val:05X}  ({sign_extend_19(val):+d})")


def print_dram_changes(history: List[Dict[str, Any]], slot: int):
    """Show D-RAM changes between frames.

    Args:
        history: List from interpreter history
        slot: Slot index to show
    """
    if len(history) < 2:
        print("Need at least 2 frames in history")
        return

    base = slot * 16
    print(f"D-RAM changes for slot {slot}:")

    for i in range(1, len(history)):
        prev = history[i - 1]['dram_snapshot']
        curr = history[i]['dram_snapshot']

        changes = []
        for j in range(16):
            addr = base + j
            if prev[addr] != curr[addr]:
                changes.append(f"D[{j}]: 0x{prev[addr]:05X} -> 0x{curr[addr]:05X}")

        if changes:
            print(f"  Frame {i}: {', '.join(changes)}")


# ============================================================
# Main / Self-test
# ============================================================

if __name__ == "__main__":
    # Quick self-test with internal sine wave
    # Example from programmer's guide:
    #
    # PHI=0    ; phase in D[0]
    # DPHI=1   ; phase increment in D[1]
    # AMP=2    ; amplitude and mix in D[2]
    #
    # RM   PHI,  <WA,WPHI,WSP>   ; A=PHI, PHI reg=D[0], WF=0x100 (sinus)
    # RM   DPHI, <WB>            ; B=D[1] (phase increment)
    # RM   AMP,  <WXY,WSP>       ; X=sin(PHI), Y=AMP, mix updated
    # RADD PHI,  <WM>            ; D[0]=A+B (PHI+DPHI)
    # RSP                        ; NOP (wait for multiplier)
    # RSP       ,<WACC>          ; accumulate AMP x sin(PHI)
    # FIN

    print("SAM8905 Interpreter Self-Test")
    print("=" * 50)
    print("Sinus Oscillator from Programmer's Guide")
    print()

    sam = SAM8905Interpreter()

    # Build A-RAM program for ALG 0
    aram = [
        0x016F,  # PC00: RM 0, <WA, WPHI, WSP>  - A=PHI, set internal sine
        0x08BF,  # PC01: RM 1, <WB>             - B=DPHI
        0x11F7,  # PC02: RM 2, <WXY, WSP>       - X=sin(PHI), Y=AMP, set mix
        0x02DF,  # PC03: RADD 0, <WM>           - D[0]=A+B (update phase)
        0x06FF,  # PC04: RSP                    - NOP (wait for multiplier)
        0x06FE,  # PC05: RSP, <WACC>            - accumulate result
    ] + [0x7FFF] * 26  # Fill rest with NOPs

    sam.load_aram(aram, offset=0)

    # D-RAM for slot 0:
    # D[0] = PHI (phase) - starts at 0
    # D[1] = DPHI (phase increment) - for 440Hz: 4096 * 440 / 44100 ≈ 41
    # D[2] = AMP with mix bits - amplitude in upper bits, mix in lower
    # D[15] = ALG=0, IDLE=0
    phase_inc = int(4096 * 440 / 44100)  # ~41 for 440Hz
    amplitude = 0x400  # Half amplitude (Q0.11)
    mix_l = 7  # Full volume
    mix_r = 7

    dram_slot0 = [0] * 16
    dram_slot0[0] = 0  # PHI starts at 0
    dram_slot0[1] = phase_inc << 7  # DPHI in upper bits (bus[18:7] -> Y)
    dram_slot0[2] = (amplitude << 7) | (mix_l << 3) | mix_r  # AMP + mix
    dram_slot0[15] = 0x00000  # ALG=0, IDLE=0

    sam.load_dram(0, dram_slot0)

    # Run 100 frames
    sam.trace_enabled = True
    samples = sam.run(100, active_slots=[0])

    print(f"\nGenerated {len(samples)} samples")
    print(f"L range: [{samples[:, 0].min()}, {samples[:, 0].max()}]")
    print(f"R range: [{samples[:, 1].min()}, {samples[:, 1].max()}]")

    print("\nFirst 5 trace lines:")
    for line in sam.trace_output[:5]:
        print(f"  {line}")

    print("\nFinal state:")
    print_state(sam.state, slot=0)
