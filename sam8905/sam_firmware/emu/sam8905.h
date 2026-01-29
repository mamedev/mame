// Copyright 2026 Ambika Retro port.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// -----------------------------------------------------------------------------
//
// SAM8905 high-level interface.
//
// Direct function interface for controlling a SAM8905 DSP chip.
// Platform provides implementations (hardware driver, emulator, etc.)

#pragma once

#include <stdbool.h>
#include <stdint.h>

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

#define SAM8905_NUM_SLOTS       16
#define SAM8905_ARAM_SIZE       256  // 256 x 15-bit micro-instructions
#define SAM8905_DRAM_SIZE       256  // 256 x 19-bit parameters
#define SAM8905_SLOT_DRAM_SIZE  16   // 16 words per slot in D-RAM
#define SAM8905_ALGO_SIZE_44K   32   // 32 instructions per algorithm at 44.1kHz
#define SAM8905_ALGO_SIZE_22K   64   // 64 instructions per algorithm at 22.05kHz
#define SAM8905_NUM_ALGOS_44K   8    // 8 algorithms at 44.1kHz
#define SAM8905_NUM_ALGOS_22K   4    // 4 algorithms at 22.05kHz

// D-RAM word 15 bit definitions
#define SAM8905_DRAM15_IDLE_BIT     11  // Idle bit (slot produces no sound)
#define SAM8905_DRAM15_ALG_SHIFT    8   // Algorithm select (bits 10:8 for 44k, 10:9 for 22k)
#define SAM8905_DRAM15_ALG_MASK_44K 0x7 // 3 bits for 44.1kHz
#define SAM8905_DRAM15_ALG_MASK_22K 0x3 // 2 bits for 22.05kHz (uses bits 10:9)
#define SAM8905_DRAM15_INTMASK_BIT  7   // Interrupt mask bit

// 19-bit mask for D-RAM values
#define SAM8905_MASK19  0x7FFFF

// -----------------------------------------------------------------------------
// Direct Function Interface
// -----------------------------------------------------------------------------
//
// These functions are implemented by the platform layer:
// - Hardware: sam8905_hw.c provides direct register access
// - Emulator: sam8905_emu.c provides software emulation
//
// For multiple SAM8905 chips, either:
// - Use separate compilation units with different function names
// - Add chip_id parameter (future extension)

// Write a 15-bit micro-instruction to A-RAM
// addr: 0-255 (8 algorithms x 32 instructions at 44.1kHz)
// inst: 15-bit instruction word
void sam8905_write_aram(uint8_t addr, uint16_t inst);

// Write a 19-bit parameter to D-RAM
// addr: 0-255 (16 slots x 16 words)
// param: 19-bit parameter value (masked to 0x7FFFF)
void sam8905_write_dram(uint8_t addr, uint32_t param);

// Read a 15-bit micro-instruction from A-RAM
// Note: Not supported on all hardware (may return 0)
uint16_t sam8905_read_aram(uint8_t addr);

// Read a 19-bit parameter from D-RAM
// Note: Not supported on all hardware (may return 0)
uint32_t sam8905_read_dram(uint8_t addr);

// Set idle state (IDL bit in control register)
// When idle=true, all 16 slots output silence
void sam8905_set_idle(bool idle);

// Set sample rate (SSR bit in control register)
// ssr_22khz=false: 44.1kHz (8 algos x 32 inst)
// ssr_22khz=true:  22.05kHz (4 algos x 64 inst)
void sam8905_set_sample_rate(bool ssr_22khz);

// Initialize SAM8905 to known state
// - Clears all slots to idle
// - Sets 44.1kHz sample rate
// - Starts processing
void sam8905_init(void);

// -----------------------------------------------------------------------------
// Audio Processing (emulator only)
// -----------------------------------------------------------------------------

// Process one audio frame and return stereo output
// For real hardware, this is a no-op (audio comes from DAC)
// out_l, out_r: pointers to receive 16-bit signed output
void sam8905_process_frame(int32_t* out_l, int32_t* out_r);

// -----------------------------------------------------------------------------
// Slot Helper Macros
// -----------------------------------------------------------------------------

// Calculate D-RAM address for a slot's parameter
// slot: 0-15, word: 0-15
#define SAM8905_DRAM_ADDR(slot, word)  (((slot) << 4) | (word))

// Calculate A-RAM address for an algorithm's instruction (44.1kHz mode)
// algo: 0-7, pc: 0-31
#define SAM8905_ARAM_ADDR_44K(algo, pc)  (((algo) << 5) | (pc))

// Calculate A-RAM address for an algorithm's instruction (22.05kHz mode)
// algo: 0-3, pc: 0-63
#define SAM8905_ARAM_ADDR_22K(algo, pc)  (((algo) << 6) | (pc))

// Set slot to idle (write to word 15 with IDLE bit set)
#define SAM8905_SLOT_IDLE  (1 << SAM8905_DRAM15_IDLE_BIT)

// Set slot active with algorithm selection (44.1kHz)
#define SAM8905_SLOT_ACTIVE_44K(algo)  ((algo) << SAM8905_DRAM15_ALG_SHIFT)

// Set slot active with algorithm selection (22.05kHz)
// Note: uses bits 10:9, so shift by 9 not 8
#define SAM8905_SLOT_ACTIVE_22K(algo)  ((algo) << 9)

// -----------------------------------------------------------------------------
// D-RAM Parameter Format Helpers
// -----------------------------------------------------------------------------

// Phase format: 19-bit unsigned, upper 12 bits are the actual phase
// DPHI (frequency): phase_inc = freq_hz * 4096 / sample_rate
// At 44.1kHz: phase_inc = freq_hz * 0.0928 (or freq_hz / 10.767)
// Stored left-shifted by 7 bits for fractional precision
#define SAM8905_PHASE(phase12)  ((uint32_t)(phase12) << 7)
#define SAM8905_DPHI(phase_inc) ((uint32_t)(phase_inc) << 7)

// Amplitude format: 12-bit signed in upper bits, mix in lower bits
// amp: -2048 to +2047 (Q0.11), mix_l/mix_r: 0-7
#define SAM8905_AMP_MIX(amp12, mix_l, mix_r) \
    (((uint32_t)((amp12) & 0xFFF) << 7) | (((mix_l) & 0x7) << 3) | ((mix_r) & 0x7))

// Wave format: WF register value
// Internal sinus: 0x100
// Internal ramp/constant: 0x100 | (R << 7) | (I << 6) | (SEL << 4) | (Z << 3)
// External memory: wave_bank (0-255)
#define SAM8905_WF_SINUS        0x100
#define SAM8905_WF_RAMP_2X      (0x100 | (1 << 6) | (0 << 4))  // 2x PHI ramp
#define SAM8905_WF_CONSTANT     (0x100 | (1 << 6) | (1 << 4))  // Constant from MAD
#define SAM8905_WF_RAMP_1X      (0x100 | (1 << 6) | (2 << 4))  // PHI ramp
#define SAM8905_WF_RAMP_HALF    (0x100 | (1 << 6) | (3 << 4))  // PHI/2 ramp
#define SAM8905_WF_ZERO         (0x100 | (1 << 3))             // Constant zero
#define SAM8905_WF_EXTERNAL(bank)  ((bank) & 0xFF)             // External memory bank
