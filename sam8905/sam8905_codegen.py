#!/usr/bin/env python3
"""
SAM8905 Algorithm Code Generator

Translates SAM8905 algorithms to executable code with an abstract backend
interface supporting multiple target languages (Python, C, etc.).

Usage:
    from sam8905_codegen import SAM8905CodeGenerator, PythonBackend, CBackend

    alg = [0x00F7, 0x607F, ...]  # A-RAM instructions
    dram_init = [0x00000, 0x50080, ...]  # Initial D-RAM values

    # Generate Python code
    gen = SAM8905CodeGenerator(PythonBackend())
    py_code = gen.generate(alg, class_name="Alg0Processor")

    # Generate C code
    gen = SAM8905CodeGenerator(CBackend())
    c_code = gen.generate(alg, class_name="alg0")
"""

from abc import ABC, abstractmethod
from dataclasses import dataclass
from typing import List, Dict

from sam8905_aram_decoder import decode_instruction, format_instruction, analyze_dram_usage


# ============================================================
# Constants (shared by all backends)
# ============================================================

MASK19 = 0x7FFFF
MASK12 = 0xFFF

CONSTANTS = [
    0x001, 0x081, 0x101, 0x181, 0x201, 0x281, 0x301, 0x381,
    0x401, 0x481, 0x501, 0x581, 0x601, 0x681, 0x701, 0x781
]

MIX_ATTEN = [0, 16, 32, 64, 128, 256, 512, 1024]


# ============================================================
# Code Generation Context
# ============================================================

@dataclass
class CodeGenContext:
    """Shared context during code generation."""
    slot_idx: int                          # Slot index for D-RAM addressing
    algorithm: List[int]                   # A-RAM instructions
    class_name: str                        # Name for generated class/struct
    dram_usage: Dict[int, Dict[str, int]]  # Which D-RAM words are read/written
    has_external_waveform: bool = False    # True if any WF < 0x100 used
    has_waveform_write: bool = False       # True if WWE sequence present
    instruction_count: int = 0             # Number of instructions to emit


# ============================================================
# Abstract Backend Interface
# ============================================================

class CodeGenBackend(ABC):
    """Abstract backend for code generation.

    Each backend implements the emit_* methods to generate code
    for a specific target language.
    """

    @abstractmethod
    def emit_header(self, ctx: CodeGenContext) -> str:
        """Emit file/function header (imports, declarations)."""

    @abstractmethod
    def emit_class_start(self, ctx: CodeGenContext) -> str:
        """Emit class/struct declaration start."""

    @abstractmethod
    def emit_class_end(self, ctx: CodeGenContext) -> str:
        """Emit class/struct declaration end."""

    @abstractmethod
    def emit_init_method(self, ctx: CodeGenContext) -> str:
        """Emit constructor/initializer method."""

    @abstractmethod
    def emit_init_dram_method(self, ctx: CodeGenContext) -> str:
        """Emit method to initialize D-RAM from list of words."""

    @abstractmethod
    def emit_execute_frame_start(self, ctx: CodeGenContext) -> str:
        """Emit start of execute_frame method (local var declarations)."""

    @abstractmethod
    def emit_execute_frame_end(self, ctx: CodeGenContext) -> str:
        """Emit end of execute_frame method (return statement)."""

    @abstractmethod
    def emit_helper_functions(self, ctx: CodeGenContext) -> str:
        """Emit helper functions (sign extension, waveform, etc.)."""

    @abstractmethod
    def emit_comment(self, text: str) -> str:
        """Emit a comment line."""

    @abstractmethod
    def emit_instruction_comment(self, pc: int, inst: int) -> str:
        """Emit comment for instruction with PC and decoded assembly."""

    # Bus operations
    @abstractmethod
    def emit_bus_from_dram(self, mad: int) -> str:
        """Emit: bus = dram[mad]"""

    @abstractmethod
    def emit_bus_from_adder(self) -> str:
        """Emit: bus = (a + b) & MASK19"""

    @abstractmethod
    def emit_bus_from_multiplier(self) -> str:
        """Emit: bus = mul_result"""

    @abstractmethod
    def emit_bus_zero(self) -> str:
        """Emit: bus = 0 (RSP)"""

    # Register writes
    @abstractmethod
    def emit_write_a(self, wsp: bool, wphi_active: bool) -> str:
        """Emit: a = bus (with WSP special logic if needed)"""

    @abstractmethod
    def emit_write_b(self) -> str:
        """Emit: b = bus"""

    @abstractmethod
    def emit_write_dram(self, mad: int, wsp: bool) -> str:
        """Emit: dram[mad] = bus (with WSP conditional if needed)"""

    @abstractmethod
    def emit_write_phi(self, wsp: bool) -> str:
        """Emit: phi = (bus >> 7) & 0xFFF (with WSP sets wf=0x100)"""

    @abstractmethod
    def emit_write_xy(self, mad: int, wsp: bool) -> str:
        """Emit: y = bus[18:7], x = waveform(wf, phi), compute mul"""

    @abstractmethod
    def emit_clear_b(self, emitter_sel: int, wsp: bool, pc: int) -> str:
        """Emit: b = 0 (and WWE if RSP+WSP)"""

    @abstractmethod
    def emit_write_wf(self) -> str:
        """Emit: wf = (bus >> 9) & 0x1FF"""

    @abstractmethod
    def emit_wacc(self) -> str:
        """Emit: accumulate mul_result to l_acc/r_acc"""

    # Carry flag update
    @abstractmethod
    def emit_update_carry(self) -> str:
        """Emit carry flag update logic."""


# ============================================================
# SAM8905 Code Generator
# ============================================================

class SAM8905CodeGenerator:
    """Translates SAM8905 algorithms to target code."""

    def __init__(self, backend: CodeGenBackend):
        self.backend = backend

    def generate(self, algorithm: List[int], slot_idx: int = 0,
                 class_name: str = "AlgProcessor") -> str:
        """Generate code for an algorithm.

        Args:
            algorithm: List of 15-bit A-RAM instructions
            slot_idx: Slot index for D-RAM addressing (usually 0 for standalone)
            class_name: Name for generated class/struct

        Returns:
            Generated source code as string
        """
        # Analyze algorithm
        dram_usage = analyze_dram_usage(algorithm)

        # Check for external waveform and WWE usage
        has_external_wf = False
        has_wwe = False

        for inst in algorithm:
            decoded = decode_instruction(inst)
            # External waveform: WXY active and WF might be < 0x100
            if decoded.wxy:
                has_external_wf = True  # Conservative: assume possible
            # WWE: RSP + clearB + WSP
            if decoded.emitter == 'RSP' and decoded.clear_b and decoded.wsp:
                has_wwe = True

        # Build context
        ctx = CodeGenContext(
            slot_idx=slot_idx,
            algorithm=algorithm,
            class_name=class_name,
            dram_usage=dram_usage,
            has_external_waveform=has_external_wf,
            has_waveform_write=has_wwe,
            instruction_count=len(algorithm)
        )

        # Generate code sections
        parts = []

        parts.append(self.backend.emit_header(ctx))
        parts.append(self.backend.emit_helper_functions(ctx))
        parts.append(self.backend.emit_class_start(ctx))
        parts.append(self.backend.emit_init_method(ctx))
        parts.append(self.backend.emit_init_dram_method(ctx))
        parts.append(self.backend.emit_execute_frame_start(ctx))

        # Emit instructions (skip last 2 as reserved NOPs per SAM8905 spec)
        effective_count = len(algorithm) - 2 if len(algorithm) >= 2 else len(algorithm)
        for pc in range(effective_count):
            inst = algorithm[pc]
            parts.append(self._emit_instruction(ctx, inst, pc))

        parts.append(self.backend.emit_execute_frame_end(ctx))
        parts.append(self.backend.emit_class_end(ctx))

        return '\n'.join(parts)

    def _emit_instruction(self, ctx: CodeGenContext, inst: int, pc: int) -> str:
        """Generate code for a single instruction.

        Args:
            ctx: Code generation context
            inst: 15-bit instruction word
            pc: Program counter

        Returns:
            Generated code for this instruction
        """
        lines = []
        b = self.backend

        # Decode fields
        decoded = decode_instruction(inst)
        mad = decoded.mad
        emitter_sel = (inst >> 9) & 0x3
        wsp = decoded.wsp

        # Add instruction comment
        lines.append("")
        lines.append(b.emit_instruction_comment(pc, inst))

        # Emitter: determine bus value
        if decoded.emitter == 'RM':
            lines.append(b.emit_bus_from_dram(mad))
        elif decoded.emitter == 'RADD':
            lines.append(b.emit_bus_from_adder())
        elif decoded.emitter == 'RP':
            lines.append(b.emit_bus_from_multiplier())
        else:  # RSP
            lines.append(b.emit_bus_zero())

        # WA (write A register)
        if decoded.wa:
            lines.append(b.emit_update_carry())
            lines.append(b.emit_write_a(wsp, decoded.wphi))

        # WB (write B register)
        if decoded.wb:
            lines.append(b.emit_write_b())

        # WM (write D-RAM)
        if decoded.wm:
            lines.append(b.emit_write_dram(mad, wsp))

        # WPHI (write phase register)
        if decoded.wphi:
            lines.append(b.emit_write_phi(wsp))

        # WXY (write X/Y, compute multiplication)
        if decoded.wxy:
            lines.append(b.emit_write_xy(mad, wsp))

        # clearB
        if decoded.clear_b:
            lines.append(b.emit_clear_b(emitter_sel, wsp, pc))

        # WWF (write waveform register)
        if decoded.wwf:
            lines.append(b.emit_write_wf())

        # WACC (accumulate)
        if decoded.wacc:
            lines.append(b.emit_wacc())

        return '\n'.join(lines)


# ============================================================
# Python Backend
# ============================================================

class PythonBackend(CodeGenBackend):
    """Generate Python code."""

    def __init__(self, indent: str = "    "):
        self.indent = indent
        self._i1 = indent          # 1 level indent
        self._i2 = indent * 2      # 2 level indent

    def emit_header(self, ctx: CodeGenContext) -> str:
        return f'''#!/usr/bin/env python3
"""
SAM8905 Algorithm Processor - Generated Code

Class: {ctx.class_name}
Instructions: {ctx.instruction_count}
D-RAM addresses used: {sorted(ctx.dram_usage.keys())}

Generated by sam8905_codegen.py
"""

import math
from typing import Optional, Callable

# Constants
MASK19 = 0x7FFFF  # 19-bit mask
MASK12 = 0xFFF    # 12-bit mask

# Multiplier constants (Q0.11 format)
CONSTANTS = [
    0x001, 0x081, 0x101, 0x181, 0x201, 0x281, 0x301, 0x381,
    0x401, 0x481, 0x501, 0x581, 0x601, 0x681, 0x701, 0x781
]

# Mix attenuation lookup (dB): 000=mute, 001=-36dB, ..., 111=0dB
MIX_ATTEN = [0, 16, 32, 64, 128, 256, 512, 1024]
'''

    def emit_helper_functions(self, ctx: CodeGenContext) -> str:
        return '''

def sign_extend_12(val: int) -> int:
    """Sign-extend 12-bit value to Python int."""
    return val | ~0xFFF if val & 0x800 else val


def sign_extend_19(val: int) -> int:
    """Sign-extend 19-bit value to Python int."""
    return val | ~MASK19 if val & 0x40000 else val
'''

    def emit_class_start(self, ctx: CodeGenContext) -> str:
        return f'''

class {ctx.class_name}:
    """SAM8905 algorithm processor with persistent D-RAM state."""
'''

    def emit_class_end(self, ctx: CodeGenContext) -> str:
        return ""

    def emit_init_method(self, ctx: CodeGenContext) -> str:
        wf_read = "waveform_read: Optional[Callable[[int], int]] = None"
        wf_write_param = ""
        wf_write_assign = ""
        wf_write_doc = ""

        if ctx.has_waveform_write:
            wf_write_param = ",\n                 waveform_write: Optional[Callable[[int, int, int, int], None]] = None"
            wf_write_assign = f"\n{self._i2}self.waveform_write = waveform_write"
            wf_write_doc = f"\n{self._i2}    waveform_write: Callback (address, data, phi, pc) for WWE writes"

        return f'''
{self._i1}def __init__(self,
{self._i1}             {wf_read}{wf_write_param}):
{self._i2}"""Initialize processor.

{self._i2}Args:
{self._i2}    waveform_read: Callback (address) -> 12-bit sample for external waveforms{wf_write_doc}
{self._i2}"""
{self._i2}# Persistent D-RAM (16 words per slot)
{self._i2}self.dram = [0] * 16

{self._i2}# External waveform callbacks
{self._i2}self.waveform_read = waveform_read{wf_write_assign}
'''

    def emit_init_dram_method(self, ctx: CodeGenContext) -> str:
        return f'''
{self._i1}def init_dram(self, words):
{self._i2}"""Initialize D-RAM from list of 16 19-bit words."""
{self._i2}for i, w in enumerate(words[:16]):
{self._i2}    self.dram[i] = w & MASK19
'''

    def emit_execute_frame_start(self, ctx: CodeGenContext) -> str:
        return f'''
{self._i1}def execute_frame(self):
{self._i2}"""Execute one frame, return (l_acc, r_acc)."""
{self._i2}# Per-frame state (reset each frame)
{self._i2}a = 0
{self._i2}b = 0
{self._i2}x = 0
{self._i2}y = 0
{self._i2}phi = 0
{self._i2}wf = 0
{self._i2}mul_result = 0
{self._i2}carry = False
{self._i2}clear_rqst = False
{self._i2}int_mod = False
{self._i2}mix_l = 0
{self._i2}mix_r = 0
{self._i2}l_acc = 0
{self._i2}r_acc = 0

{self._i2}# Alias for readability
{self._i2}dram = self.dram
'''

    def emit_execute_frame_end(self, ctx: CodeGenContext) -> str:
        return f'''
{self._i2}return l_acc, r_acc

{self._i1}def _get_waveform(self, wf: int, phi: int, mad: int) -> int:
{self._i2}"""Get waveform sample (internal or external)."""
{self._i2}phi = phi & MASK12
{self._i2}internal = (wf & 0x100) != 0

{self._i2}if internal:
{self._i2}    # Internal waveform generation
{self._i2}    z_bit = (wf & 0x08) != 0
{self._i2}    if z_bit:
{self._i2}        return 0

{self._i2}    sel = (wf >> 4) & 3
{self._i2}    ramp_mode = (wf & 0x40) != 0
{self._i2}    invert = (wf & 0x80) != 0

{self._i2}    if not ramp_mode:
{self._i2}        # Sinus wave
{self._i2}        angle = (math.pi / 2048.0) * phi + (math.pi / 4096.0)
{self._i2}        result = int(0.71875 * math.sin(angle) * 2048.0)
{self._i2}    else:
{self._i2}        # Ramps based on SEL bits
{self._i2}        if sel == 0:
{self._i2}            # 2x PHI triangle
{self._i2}            if phi < 1024:
{self._i2}                result = phi * 2
{self._i2}            elif phi < 3072:
{self._i2}                result = (phi * 2) - 4096
{self._i2}            else:
{self._i2}                result = (phi * 2) - 8192
{self._i2}        elif sel == 1:
{self._i2}            # Constant from MAD
{self._i2}            result = CONSTANTS[mad & 0xF]
{self._i2}        elif sel == 2:
{self._i2}            # PHI ramp
{self._i2}            result = phi if phi < 2048 else phi - 4096
{self._i2}        elif sel == 3:
{self._i2}            # PHI/2 ramp
{self._i2}            result = phi // 2 if phi < 2048 else (phi // 2) - 2048
{self._i2}        else:
{self._i2}            result = 0

{self._i2}    # Apply inversion (matches sam8905.cpp exactly)
{self._i2}    if invert:
{self._i2}        result = (-result) & 0xFFF
{self._i2}        if result & 0x800:
{self._i2}            result |= ~0xFFF  # Sign extend

{self._i2}    return result
{self._i2}else:
{self._i2}    # External memory access
{self._i2}    if self.waveform_read is not None:
{self._i2}        addr = ((wf & 0xFF) << 12) | phi
{self._i2}        sample = self.waveform_read(addr)
{self._i2}        return sign_extend_12(sample & MASK12)
{self._i2}    return 0
'''

    def emit_comment(self, text: str) -> str:
        return f"{self._i2}# {text}"

    def emit_instruction_comment(self, pc: int, inst: int) -> str:
        decoded = decode_instruction(inst)
        asm = format_instruction(decoded, show_hex=True)
        return f"{self._i2}# PC{pc:02d}: {asm}"

    def emit_bus_from_dram(self, mad: int) -> str:
        return f"{self._i2}bus = dram[{mad}]"

    def emit_bus_from_adder(self) -> str:
        return f"{self._i2}bus = (a + b) & MASK19"

    def emit_bus_from_multiplier(self) -> str:
        return f"{self._i2}bus = mul_result"

    def emit_bus_zero(self) -> str:
        return f"{self._i2}bus = 0"

    def emit_update_carry(self) -> str:
        return f"""{self._i2}# Update carry flag
{self._i2}b_neg = (b & 0x40000) != 0
{self._i2}sum_val = a + b
{self._i2}if not b_neg:
{self._i2}    carry = sum_val > MASK19
{self._i2}else:
{self._i2}    carry = ((sum_val & MASK19) & 0x40000) == 0"""

    def emit_write_a(self, wsp: bool, wphi_active: bool) -> str:
        if wsp and not wphi_active:
            # WA + WSP special logic
            return f"""{self._i2}# WA + WSP special logic
{self._i2}wave = (bus >> 9) & 0x1FF
{self._i2}final_wave = bus & 0x1FF
{self._i2}end_bit = (bus & 0x40000) != 0
{self._i2}wf_match = (wave == final_wave)
{self._i2}if not carry:
{self._i2}    a = 0
{self._i2}    clear_rqst = False
{self._i2}    int_mod = True
{self._i2}elif not wf_match:
{self._i2}    a = 0x200
{self._i2}    clear_rqst = False
{self._i2}    int_mod = True
{self._i2}else:
{self._i2}    a = 0
{self._i2}    int_mod = True
{self._i2}    clear_rqst = end_bit"""
        else:
            return f"""{self._i2}a = bus
{self._i2}clear_rqst = True
{self._i2}int_mod = False"""

    def emit_write_b(self) -> str:
        return f"{self._i2}b = bus"

    def emit_write_dram(self, mad: int, wsp: bool) -> str:
        if wsp:
            return f"""{self._i2}# WM + WSP conditional write
{self._i2}if clear_rqst and not carry:
{self._i2}    dram[{mad}] = bus"""
        else:
            return f"{self._i2}dram[{mad}] = bus"

    def emit_write_phi(self, wsp: bool) -> str:
        if wsp:
            return f"""{self._i2}phi = (bus >> 7) & MASK12
{self._i2}wf = 0x100  # WPHI + WSP: force internal sinus"""
        else:
            return f"{self._i2}phi = (bus >> 7) & MASK12"

    def emit_write_xy(self, mad: int, wsp: bool) -> str:
        base = f"""{self._i2}y = (bus >> 7) & MASK12
{self._i2}x = self._get_waveform(wf, phi, {mad}) & MASK12
{self._i2}# Multiplier: Q0.11 * Q0.11 -> Q0.18
{self._i2}x_signed = sign_extend_12(x)
{self._i2}y_signed = sign_extend_12(y)
{self._i2}product = x_signed * y_signed
{self._i2}mul_result = ((product + 8) >> 4) & MASK19"""

        if wsp:
            base += f"""
{self._i2}# WXY + WSP: update mix attenuators
{self._i2}mix_l = (bus >> 3) & 0x7
{self._i2}mix_r = bus & 0x7"""

        return base

    def emit_clear_b(self, emitter_sel: int, wsp: bool, pc: int) -> str:
        base = f"{self._i2}b = 0"

        # WWE: RSP (emitter_sel=3) + clearB + WSP
        if emitter_sel == 3 and wsp:
            base += f"""
{self._i2}# WWE: Write Waveform Enable (RSP + clearB + WSP)
{self._i2}if self.waveform_write is not None and (wf & 0x1FF) < 0x80:
{self._i2}    ext_addr = ((wf & 0x7) << 12) | (phi & 0xFFF)
{self._i2}    ext_data = sign_extend_12(y)
{self._i2}    self.waveform_write(ext_addr, ext_data, phi, {pc})"""

        return base

    def emit_write_wf(self) -> str:
        return f"{self._i2}wf = (bus >> 9) & 0x1FF"

    def emit_wacc(self) -> str:
        return f"""{self._i2}# Accumulate with dB attenuation
{self._i2}signed_mul = sign_extend_19(mul_result)
{self._i2}l_contrib = (signed_mul * MIX_ATTEN[mix_l]) >> 10
{self._i2}r_contrib = (signed_mul * MIX_ATTEN[mix_r]) >> 10
{self._i2}l_acc += l_contrib
{self._i2}r_acc += r_contrib"""


# ============================================================
# C Backend
# ============================================================

class CBackend(CodeGenBackend):
    """Generate C code."""

    def __init__(self, indent: str = "    "):
        self.indent = indent
        self._i1 = indent          # 1 level indent
        self._i2 = indent * 2      # 2 level indent

    def emit_header(self, ctx: CodeGenContext) -> str:
        guard_name = ctx.class_name.upper()
        return f'''/*
 * SAM8905 Algorithm Processor - Generated Code
 *
 * Struct: {ctx.class_name}_state_t
 * Instructions: {ctx.instruction_count}
 * D-RAM addresses used: {sorted(ctx.dram_usage.keys())}
 *
 * Generated by sam8905_codegen.py
 */

#ifndef {guard_name}_H
#define {guard_name}_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/* Constants */
#define MASK19 0x7FFFF
#define MASK12 0xFFF

/* Multiplier constants (Q0.11 format) */
static const int32_t CONSTANTS[16] = {{
    0x001, 0x081, 0x101, 0x181, 0x201, 0x281, 0x301, 0x381,
    0x401, 0x481, 0x501, 0x581, 0x601, 0x681, 0x701, 0x781
}};

/* Mix attenuation lookup */
static const int32_t MIX_ATTEN[8] = {{0, 16, 32, 64, 128, 256, 512, 1024}};

/* Callback types */
typedef int32_t (*waveform_read_fn)(uint32_t address);
typedef void (*waveform_write_fn)(uint32_t address, int32_t data, uint32_t phi, int pc);
'''

    def emit_helper_functions(self, ctx: CodeGenContext) -> str:
        return f'''
/* Sign extension */
static inline int32_t sign_extend_12(uint32_t val) {{
    return (val & 0x800) ? (int32_t)(val | ~0xFFF) : (int32_t)val;
}}

static inline int32_t sign_extend_19(uint32_t val) {{
    return (val & 0x40000) ? (int32_t)(val | ~MASK19) : (int32_t)val;
}}

/* Internal waveform generation */
static int32_t get_internal_waveform(uint32_t wf, uint32_t phi, int mad) {{
    phi = phi & MASK12;

    bool z_bit = (wf & 0x08) != 0;
    if (z_bit) return 0;

    int sel = (wf >> 4) & 3;
    bool ramp_mode = (wf & 0x40) != 0;
    bool invert = (wf & 0x80) != 0;
    int32_t result;

    if (!ramp_mode) {{
        /* Sinus wave */
        double angle = (M_PI / 2048.0) * phi + (M_PI / 4096.0);
        result = (int32_t)(0.71875 * sin(angle) * 2048.0);
    }} else {{
        /* Ramps based on SEL bits */
        switch (sel) {{
            case 0:  /* 2x PHI triangle */
                if (phi < 1024)
                    result = phi * 2;
                else if (phi < 3072)
                    result = (phi * 2) - 4096;
                else
                    result = (phi * 2) - 8192;
                break;
            case 1:  /* Constant from MAD */
                result = CONSTANTS[mad & 0xF];
                break;
            case 2:  /* PHI ramp */
                result = (phi < 2048) ? phi : phi - 4096;
                break;
            case 3:  /* PHI/2 ramp */
                result = (phi < 2048) ? phi / 2 : (phi / 2) - 2048;
                break;
            default:
                result = 0;
        }}
    }}

    /* Apply inversion (matches sam8905.cpp exactly) */
    if (invert) {{
        result = (-result) & 0xFFF;
        if (result & 0x800) result |= ~0xFFF;  /* Sign extend */
    }}

    return result;
}}
'''

    def emit_class_start(self, ctx: CodeGenContext) -> str:
        name = ctx.class_name
        return f'''
/* Processor state structure */
typedef struct {{
    uint32_t dram[16];         /* Persistent D-RAM */
    waveform_read_fn read_cb;  /* External waveform read callback */
    waveform_write_fn write_cb; /* External waveform write callback */
}} {name}_state_t;
'''

    def emit_class_end(self, ctx: CodeGenContext) -> str:
        return f'''
#endif /* {ctx.class_name.upper()}_H */
'''

    def emit_init_method(self, ctx: CodeGenContext) -> str:
        name = ctx.class_name
        return f'''
/* Initialize processor state */
void {name}_init({name}_state_t *state, waveform_read_fn read_cb, waveform_write_fn write_cb) {{
    for (int i = 0; i < 16; i++) {{
        state->dram[i] = 0;
    }}
    state->read_cb = read_cb;
    state->write_cb = write_cb;
}}
'''

    def emit_init_dram_method(self, ctx: CodeGenContext) -> str:
        name = ctx.class_name
        return f'''
/* Initialize D-RAM from array of 16 words */
void {name}_init_dram({name}_state_t *state, const uint32_t *words) {{
    for (int i = 0; i < 16; i++) {{
        state->dram[i] = words[i] & MASK19;
    }}
}}
'''

    def emit_execute_frame_start(self, ctx: CodeGenContext) -> str:
        name = ctx.class_name
        return f'''
/* Execute one frame, output via pointers */
void {name}_execute_frame({name}_state_t *state, int32_t *l_out, int32_t *r_out) {{
    /* Per-frame state (reset each frame) */
    uint32_t a = 0;
    uint32_t b = 0;
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t phi = 0;
    uint32_t wf = 0;
    uint32_t mul_result = 0;
    bool carry = false;
    bool clear_rqst = false;
    int mix_l = 0;
    int mix_r = 0;
    int32_t l_acc = 0;
    int32_t r_acc = 0;
    uint32_t bus;
    int32_t x_signed, y_signed;
    int32_t product;

    /* Alias for readability */
    uint32_t *dram = state->dram;
'''

    def emit_execute_frame_end(self, ctx: CodeGenContext) -> str:
        return f'''
{self._i1}*l_out = l_acc;
{self._i1}*r_out = r_acc;
}}

/* Get waveform sample (internal or external) */
static int32_t {ctx.class_name}_get_waveform({ctx.class_name}_state_t *state,
                                              uint32_t wf, uint32_t phi, int mad) {{
    phi = phi & MASK12;
    bool internal = (wf & 0x100) != 0;

    if (internal) {{
        return get_internal_waveform(wf, phi, mad);
    }} else {{
        /* External memory access */
        if (state->read_cb != NULL) {{
            uint32_t addr = ((wf & 0xFF) << 12) | phi;
            int32_t sample = state->read_cb(addr);
            return sign_extend_12(sample & MASK12);
        }}
        return 0;
    }}
}}
'''

    def emit_comment(self, text: str) -> str:
        return f"{self._i1}/* {text} */"

    def emit_instruction_comment(self, pc: int, inst: int) -> str:
        decoded = decode_instruction(inst)
        asm = format_instruction(decoded, show_hex=True)
        return f"{self._i1}/* PC{pc:02d}: {asm} */"

    def emit_bus_from_dram(self, mad: int) -> str:
        return f"{self._i1}bus = dram[{mad}];"

    def emit_bus_from_adder(self) -> str:
        return f"{self._i1}bus = (a + b) & MASK19;"

    def emit_bus_from_multiplier(self) -> str:
        return f"{self._i1}bus = mul_result;"

    def emit_bus_zero(self) -> str:
        return f"{self._i1}bus = 0;"

    def emit_update_carry(self) -> str:
        return f"""{self._i1}/* Update carry flag */
{self._i1}{{
{self._i2}bool b_neg = (b & 0x40000) != 0;
{self._i2}uint32_t sum_val = a + b;
{self._i2}if (!b_neg)
{self._i2}    carry = sum_val > MASK19;
{self._i2}else
{self._i2}    carry = ((sum_val & MASK19) & 0x40000) == 0;
{self._i1}}}"""

    def emit_write_a(self, wsp: bool, wphi_active: bool) -> str:
        if wsp and not wphi_active:
            return f"""{self._i1}/* WA + WSP special logic */
{self._i1}{{
{self._i2}uint32_t wave = (bus >> 9) & 0x1FF;
{self._i2}uint32_t final_wave = bus & 0x1FF;
{self._i2}bool end_bit = (bus & 0x40000) != 0;
{self._i2}bool wf_match = (wave == final_wave);
{self._i2}if (!carry) {{
{self._i2}    a = 0;
{self._i2}    clear_rqst = false;
{self._i2}}} else if (!wf_match) {{
{self._i2}    a = 0x200;
{self._i2}    clear_rqst = false;
{self._i2}}} else {{
{self._i2}    a = 0;
{self._i2}    clear_rqst = end_bit;
{self._i2}}}
{self._i1}}}"""
        else:
            return f"""{self._i1}a = bus;
{self._i1}clear_rqst = true;"""

    def emit_write_b(self) -> str:
        return f"{self._i1}b = bus;"

    def emit_write_dram(self, mad: int, wsp: bool) -> str:
        if wsp:
            return f"""{self._i1}/* WM + WSP conditional write */
{self._i1}if (clear_rqst && !carry) {{
{self._i2}dram[{mad}] = bus;
{self._i1}}}"""
        else:
            return f"{self._i1}dram[{mad}] = bus;"

    def emit_write_phi(self, wsp: bool) -> str:
        if wsp:
            return f"""{self._i1}phi = (bus >> 7) & MASK12;
{self._i1}wf = 0x100;  /* WPHI + WSP: force internal sinus */"""
        else:
            return f"{self._i1}phi = (bus >> 7) & MASK12;"

    def emit_write_xy(self, mad: int, wsp: bool) -> str:
        base = f"""{self._i1}y = (bus >> 7) & MASK12;
{self._i1}x = {mad != 0 and f'{CBackend.__name__[0].lower()}' or ''}state ?
{self._i1}    {mad}_get_waveform(state, wf, phi, {mad}) & MASK12 : 0;"""

        # Actually, let's simplify the waveform call
        base = f"""{self._i1}y = (bus >> 7) & MASK12;
{self._i1}x = get_internal_waveform(wf, phi, {mad}) & MASK12;
{self._i1}if (!(wf & 0x100) && state->read_cb) {{
{self._i2}uint32_t addr = ((wf & 0xFF) << 12) | phi;
{self._i2}x = sign_extend_12(state->read_cb(addr) & MASK12) & MASK12;
{self._i1}}}
{self._i1}/* Multiplier: Q0.11 * Q0.11 -> Q0.18 */
{self._i1}x_signed = sign_extend_12(x);
{self._i1}y_signed = sign_extend_12(y);
{self._i1}product = x_signed * y_signed;
{self._i1}mul_result = ((product + 8) >> 4) & MASK19;"""

        if wsp:
            base += f"""
{self._i1}/* WXY + WSP: update mix attenuators */
{self._i1}mix_l = (bus >> 3) & 0x7;
{self._i1}mix_r = bus & 0x7;"""

        return base

    def emit_clear_b(self, emitter_sel: int, wsp: bool, pc: int) -> str:
        base = f"{self._i1}b = 0;"

        # WWE: RSP (emitter_sel=3) + clearB + WSP
        if emitter_sel == 3 and wsp:
            base += f"""
{self._i1}/* WWE: Write Waveform Enable (RSP + clearB + WSP) */
{self._i1}if (state->write_cb && (wf & 0x1FF) < 0x80) {{
{self._i2}uint32_t ext_addr = ((wf & 0x7) << 12) | (phi & 0xFFF);
{self._i2}int32_t ext_data = sign_extend_12(y);
{self._i2}state->write_cb(ext_addr, ext_data, phi, {pc});
{self._i1}}}"""

        return base

    def emit_write_wf(self) -> str:
        return f"{self._i1}wf = (bus >> 9) & 0x1FF;"

    def emit_wacc(self) -> str:
        return f"""{self._i1}/* Accumulate with dB attenuation */
{self._i1}{{
{self._i2}int32_t signed_mul = sign_extend_19(mul_result);
{self._i2}int32_t l_contrib = (signed_mul * MIX_ATTEN[mix_l]) >> 10;
{self._i2}int32_t r_contrib = (signed_mul * MIX_ATTEN[mix_r]) >> 10;
{self._i2}l_acc += l_contrib;
{self._i2}r_acc += r_contrib;
{self._i1}}}"""


# ============================================================
# Convenience functions
# ============================================================

def generate_python(algorithm: List[int], class_name: str = "AlgProcessor",
                    slot_idx: int = 0) -> str:
    """Generate Python code for an algorithm.

    Args:
        algorithm: List of 15-bit A-RAM instructions
        class_name: Name for generated class
        slot_idx: Slot index (usually 0)

    Returns:
        Generated Python source code
    """
    gen = SAM8905CodeGenerator(PythonBackend())
    return gen.generate(algorithm, slot_idx=slot_idx, class_name=class_name)


def generate_c(algorithm: List[int], class_name: str = "alg",
               slot_idx: int = 0) -> str:
    """Generate C code for an algorithm.

    Args:
        algorithm: List of 15-bit A-RAM instructions
        class_name: Name for generated struct/functions
        slot_idx: Slot index (usually 0)

    Returns:
        Generated C source code
    """
    gen = SAM8905CodeGenerator(CBackend())
    return gen.generate(algorithm, slot_idx=slot_idx, class_name=class_name)


# ============================================================
# Main / Self-test
# ============================================================

if __name__ == "__main__":
    # Test with sinus oscillator algorithm
    print("SAM8905 Code Generator Self-Test")
    print("=" * 50)

    # Sinus oscillator algorithm
    aram = [
        0x016F,  # PC00: RM 0, <WA, WPHI, WSP>  - A=PHI, set internal sine
        0x08BF,  # PC01: RM 1, <WB>             - B=DPHI
        0x11F7,  # PC02: RM 2, <WXY, WSP>       - X=sin(PHI), Y=AMP, set mix
        0x02DF,  # PC03: RADD 0, <WM>           - D[0]=A+B (update phase)
        0x06FF,  # PC04: RSP                    - NOP (wait for multiplier)
        0x06FE,  # PC05: RSP, <WACC>            - accumulate result
    ] + [0x7FFF] * 26  # Fill rest with NOPs

    print("\nGenerating Python code...")
    py_code = generate_python(aram, class_name="SinusOscillator")
    print(py_code[:2000] + "..." if len(py_code) > 2000 else py_code)

    print("\n" + "=" * 50)
    print("\nGenerating C code...")
    c_code = generate_c(aram, class_name="sinus_osc")
    print(c_code[:2000] + "..." if len(c_code) > 2000 else c_code)
