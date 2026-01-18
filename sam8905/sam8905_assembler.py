#!/usr/bin/env python3
"""
SAM8905 Assembler

Assembles SAM8905 DSP programs from the syntax used in the Programmer's Guide.

Syntax:
    ; Comment
    NAME=N              ; Define D-RAM variable at address N
    EMITTER MAD, <RECEIVERS>    ; Full instruction
    EMITTER MAD                 ; Instruction without receivers
    EMITTER     , <RECEIVERS>   ; Instruction with implicit MAD
              , <RECEIVERS>     ; Continue previous emitter
    FIN                         ; Fill rest with NOPs

Emitters: RM, RADD, RP, RSP
Receivers: WA, WB, WM, WPHI, WXY, clearB, WWF, WACC, WSP

Example:
    PHI=0
    DPHI=1
    AMP=2

    RM      PHI,    <WA,WPHI,WSP>
    RM      DPHI,   <WB>
    RM      AMP,    <WXY,WSP>
    RADD    PHI,    <WM>
    RSP
                    ,<WACC>
    FIN
"""

import re
from dataclasses import dataclass, field
from typing import List, Dict, Optional, Tuple


@dataclass
class Instruction:
    """Assembled instruction."""
    pc: int
    word: int
    source: str
    emitter: str = ""
    mad: int = 0
    receivers: List[str] = field(default_factory=list)


@dataclass
class DRAMWord:
    """D-RAM word definition."""
    address: int
    value: int
    name: str = ""
    comment: str = ""


class SAM8905Assembler:
    """Assembler for SAM8905 DSP programs."""

    # Emitter encoding (bits 14:13)
    EMITTERS = {
        'RM': 0b00,    # Read Memory
        'RADD': 0b01,  # Read A+B
        'RP': 0b10,    # Read Product
        'RSP': 0b11,   # Read Special (NOP or with WSP)
    }

    # Receiver bit positions (active LOW except WSP)
    # Instruction format: [14:13]=emitter, [12]=WSP(active high), [11:4]=MAD, [7:0]=receivers
    # Actually from the decoder analysis:
    # Bits 3:0 seem to be receiver bits, not MAD
    # Let me re-analyze based on the sandbox make_inst function

    # From sandbox notebook make_inst:
    # inst = (mad & 0xF) << 11
    # inst |= emitter_map[emitter] << 9
    # inst |= 0x100 if wsp else 0
    # if not wa: inst |= 0x80
    # if not wb: inst |= 0x40
    # if not wm: inst |= 0x20
    # if not wphi: inst |= 0x10
    # if not wxy: inst |= 0x08
    # if not clear_b: inst |= 0x04
    # if not wwf: inst |= 0x02
    # if not wacc: inst |= 0x01

    # So the layout is:
    # [14:11] = MAD (4 bits)
    # [10:9]  = Emitter (2 bits)
    # [8]     = WSP (active HIGH)
    # [7]     = ~WA (active LOW)
    # [6]     = ~WB
    # [5]     = ~WM
    # [4]     = ~WPHI
    # [3]     = ~WXY
    # [2]     = ~clearB
    # [1]     = ~WWF
    # [0]     = ~WACC

    RECEIVERS_ACTIVE_LOW = {
        'WA': 7,
        'WB': 6,
        'WM': 5,
        'WPHI': 4,
        'WXY': 3,
        'CLEARB': 2,
        'WWF': 1,
        'WACC': 0,
    }

    # WSP is active HIGH at bit 8
    WSP_BIT = 8

    def __init__(self, algorithm_size: int = 32):
        """Initialize assembler.

        Args:
            algorithm_size: Number of instructions per algorithm (32 for 22kHz, 64 for 44kHz)
        """
        self.algorithm_size = algorithm_size
        self.variables: Dict[str, int] = {}
        self.instructions: List[Instruction] = []
        self.dram_words: List[DRAMWord] = []
        self.errors: List[str] = []
        self.pc = 0
        self.last_emitter = 'RSP'

    def reset(self):
        """Reset assembler state."""
        self.variables = {}
        self.instructions = []
        self.dram_words = []
        self.errors = []
        self.pc = 0
        self.last_emitter = 'RSP'

    def assemble(self, source: str) -> Tuple[List[int], List[str]]:
        """Assemble source code.

        Args:
            source: Assembly source code

        Returns:
            Tuple of (instruction_words, errors)
        """
        self.reset()
        lines = source.strip().split('\n')

        for line_num, line in enumerate(lines, 1):
            try:
                self._process_line(line, line_num)
            except Exception as e:
                self.errors.append(f"Line {line_num}: {e}")

        # Fill remaining with NOPs if FIN was used
        words = [inst.word for inst in self.instructions]

        return words, self.errors

    def _process_line(self, line: str, line_num: int):
        """Process a single line of source."""
        # Remove comments
        if ';' in line:
            line, _ = line.split(';', 1)

        line = line.strip()
        if not line:
            return

        # Check for variable definition: NAME=N
        match = re.match(r'^(\w+)\s*=\s*(\d+)\s*$', line)
        if match:
            name, addr = match.groups()
            self.variables[name.upper()] = int(addr)
            return

        # Check for FIN directive
        if line.upper() == 'FIN':
            self._fill_nops()
            return

        # Parse instruction
        self._parse_instruction(line, line_num)

    def _parse_instruction(self, line: str, line_num: int):
        """Parse an instruction line."""
        # Patterns:
        # "EMITTER MAD, <RECEIVERS>"
        # "EMITTER MAD"
        # "EMITTER , <RECEIVERS>"
        # ", <RECEIVERS>"

        # Extract receivers if present
        receivers = []
        if '<' in line and '>' in line:
            recv_match = re.search(r'<([^>]+)>', line)
            if recv_match:
                recv_str = recv_match.group(1)
                receivers = [r.strip().upper() for r in recv_str.split(',')]
                line = line[:line.index('<')].strip()
                if line.endswith(','):
                    line = line[:-1].strip()

        # Check for continuation (line starts with comma)
        if line.startswith(','):
            emitter = self.last_emitter
            mad = 0
            line = line[1:].strip()
        else:
            # Parse emitter and MAD
            parts = line.split(',')
            if len(parts) >= 1:
                emitter_mad = parts[0].strip().split()
                if len(emitter_mad) >= 1:
                    emitter = emitter_mad[0].upper()
                    if len(emitter_mad) >= 2:
                        mad_str = emitter_mad[1].strip()
                        # Resolve variable name to address
                        mad = self._resolve_mad(mad_str)
                    else:
                        mad = 0
                else:
                    emitter = self.last_emitter
                    mad = 0
            else:
                emitter = self.last_emitter
                mad = 0

        # Validate emitter
        if emitter not in self.EMITTERS:
            raise ValueError(f"Unknown emitter: {emitter}")

        self.last_emitter = emitter

        # Build instruction word
        word = self._build_instruction(emitter, mad, receivers)

        # Record instruction
        inst = Instruction(
            pc=self.pc,
            word=word,
            source=f"{emitter} {mad}, <{','.join(receivers)}>",
            emitter=emitter,
            mad=mad,
            receivers=receivers
        )
        self.instructions.append(inst)
        self.pc += 1

    def _resolve_mad(self, mad_str: str) -> int:
        """Resolve MAD value from string (variable name or number)."""
        mad_str = mad_str.upper()
        if mad_str in self.variables:
            return self.variables[mad_str]
        try:
            if mad_str.startswith('0X'):
                return int(mad_str, 16)
            return int(mad_str)
        except ValueError:
            raise ValueError(f"Unknown variable or invalid number: {mad_str}")

    def _build_instruction(self, emitter: str, mad: int, receivers: List[str]) -> int:
        """Build 15-bit instruction word."""
        # Start with all receivers disabled (bits set to 1 for active-low)
        word = 0xFF  # All receiver bits high (disabled)

        # Set MAD (bits 14:11)
        word |= (mad & 0xF) << 11

        # Set emitter (bits 10:9)
        word |= self.EMITTERS[emitter] << 9

        # Enable receivers (clear their bits for active-low)
        wsp = False
        for recv in receivers:
            recv = recv.upper()
            if recv == 'WSP':
                wsp = True
            elif recv in self.RECEIVERS_ACTIVE_LOW:
                bit = self.RECEIVERS_ACTIVE_LOW[recv]
                word &= ~(1 << bit)  # Clear bit to enable
            else:
                raise ValueError(f"Unknown receiver: {recv}")

        # Set WSP (bit 8, active HIGH)
        if wsp:
            word |= (1 << self.WSP_BIT)

        return word & 0x7FFF  # Mask to 15 bits

    def _fill_nops(self):
        """Fill remaining instructions with NOPs."""
        nop = self._build_instruction('RSP', 0, [])
        while self.pc < self.algorithm_size:
            inst = Instruction(
                pc=self.pc,
                word=nop,
                source="NOP (FIN fill)",
                emitter='RSP',
                mad=0,
                receivers=[]
            )
            self.instructions.append(inst)
            self.pc += 1

    def get_aram(self) -> List[int]:
        """Get assembled A-RAM words."""
        return [inst.word for inst in self.instructions]

    def disassemble(self) -> str:
        """Disassemble to human-readable format."""
        lines = []
        for inst in self.instructions:
            lines.append(f"PC{inst.pc:02d}: {inst.word:04X}  {inst.source}")
        return '\n'.join(lines)


def make_dram_word(wf: int = 0, phi: int = 0, value: int = 0,
                   mix_l: int = 0, mix_r: int = 0,
                   alg: int = 0, idle: bool = False) -> int:
    """Create a D-RAM word with proper bit packing.

    D-RAM words are 19 bits with various field layouts depending on usage:

    For waveform config (word loaded by WWF/WPHI):
        bits 18:9 = WF (9-bit waveform select)
        bits 18:7 = PHI (12-bit phase, overlaps with WF)

    For amplitude/mix (word loaded by WXY):
        bits 18:7 = Y value (amplitude)
        bits 5:3 = MIX_L (3-bit)
        bits 2:0 = MIX_R (3-bit)

    For param15 (slot control):
        bits 18:16 = MIX_L
        bits 6:4 = MIX_R
        bits 3:2 = ALG
        bit 1 = IDLE
        bit 0 = reserved

    Args:
        wf: 9-bit waveform select (for WWF)
        phi: 12-bit phase (for WPHI)
        value: Direct 19-bit value (if other params not used)
        mix_l: 3-bit left mix (0-7)
        mix_r: 3-bit right mix (0-7)
        alg: 2-bit algorithm select (for param15)
        idle: Idle flag (for param15)

    Returns:
        19-bit D-RAM word
    """
    if value != 0:
        return value & 0x7FFFF

    word = 0

    # WF in bits 17:9 (shifted left by 9)
    if wf:
        word |= (wf & 0x1FF) << 9

    # PHI in bits 18:7 (12-bit value shifted left by 7)
    if phi:
        word |= (phi & 0xFFF) << 7

    # Mix settings depend on word type
    if mix_l or mix_r:
        # For param15: MIX_L in bits 18:16, MIX_R in bits 6:4
        word |= (mix_l & 7) << 16
        word |= (mix_r & 7) << 4

    # Algorithm and idle for param15
    if alg:
        word |= (alg & 3) << 2
    if idle:
        word |= 0x02

    return word & 0x7FFFF


def make_amplitude_word(amplitude: int, mix_l: int = 7, mix_r: int = 7) -> int:
    """Create D-RAM word for amplitude/mix (used with WXY).

    Args:
        amplitude: 12-bit amplitude value (Q0.11, -2048 to +2047)
        mix_l: Left mix (0-7, 7=full volume)
        mix_r: Right mix (0-7, 7=full volume)

    Returns:
        19-bit D-RAM word
    """
    # Amplitude goes in bits 18:7 (where Y is read from)
    # Mix goes in bits 5:3 (mix_l) and 2:0 (mix_r)
    word = ((amplitude & 0xFFF) << 7) | ((mix_l & 7) << 3) | (mix_r & 7)
    return word & 0x7FFFF


def make_phase_increment(frequency: float, sample_rate: float = 44100.0) -> int:
    """Calculate phase increment for a given frequency.

    Args:
        frequency: Target frequency in Hz
        sample_rate: Sample rate in Hz (default 44100)

    Returns:
        Phase increment value (for D-RAM, needs to be shifted)
    """
    # PHI range is 0-4095 (12-bit)
    # One complete cycle = 4096 phase units
    # phase_inc = 4096 * freq / sample_rate
    phase_inc = int(4096 * frequency / sample_rate)
    return phase_inc


# Example usage and test
if __name__ == '__main__':
    # Example: Sinus oscillator from Programmer's Guide
    source = """
    ; Sinus Oscillator from SAM8905 Programmer's Guide
    PHI=0
    DPHI=1
    AMP=2

    RM      PHI,    <WA,WPHI,WSP>
    RM      DPHI,   <WB>
    RM      AMP,    <WXY,WSP>
    RADD    PHI,    <WM>
    RSP
                    ,<WACC>
    FIN
    """

    asm = SAM8905Assembler(algorithm_size=32)
    words, errors = asm.assemble(source)

    if errors:
        print("Errors:")
        for e in errors:
            print(f"  {e}")
    else:
        print("Assembled successfully!")
        print()
        print("Variables:")
        for name, addr in asm.variables.items():
            print(f"  {name} = {addr}")
        print()
        print("A-RAM:")
        print(asm.disassemble())
        print()
        print("Hex words:")
        print([f"0x{w:04X}" for w in words])

        # Compare with sandbox values
        sandbox_aram = [0x016F, 0x08BF, 0x11F7, 0x02DF, 0x06FF, 0x06FE]
        print()
        print("Comparison with sandbox:")
        for i, (ours, theirs) in enumerate(zip(words[:6], sandbox_aram)):
            match = "✓" if ours == theirs else "✗"
            print(f"  PC{i}: {ours:04X} vs {theirs:04X} {match}")
