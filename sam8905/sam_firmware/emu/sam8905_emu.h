// Copyright 2026 Ambika Retro port.
// Based on MAME SAM8905 emulation by Draft.
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
// SAM8905 emulator - ported from MAME.
//
// This provides a software emulation of the Dream SAM8905 DSP chip.
// Used for host testing and potentially for running on RP2040.
//
// Direct Function Interface:
// The emulator implements the direct function interface from sam8905.h.
// Call sam8905_emu_set_instance() to set which emulator instance is active.
// Then use sam8905_write_dram(), sam8905_process_frame(), etc.

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "sam8905.h"

// -----------------------------------------------------------------------------
// Waveform Memory Callbacks (for external wave ROM/RAM)
// -----------------------------------------------------------------------------

typedef int16_t (*sam8905_waveform_read_fn)(void* ctx, uint32_t addr);
typedef void (*sam8905_waveform_write_fn)(void* ctx, uint32_t addr, int16_t data);

// -----------------------------------------------------------------------------
// Emulator State Structure
// -----------------------------------------------------------------------------

typedef struct Sam8905Emu {
    // RAM (directly accessible, no bus protocol needed)
    uint16_t aram[SAM8905_ARAM_SIZE];  // 256 x 15-bit micro-instructions
    uint32_t dram[SAM8905_DRAM_SIZE];  // 256 x 19-bit parameters

    // DSP registers (shared across all slots within a frame)
    uint32_t bus;           // Internal 19-bit bus
    uint32_t a, b;          // Adder registers (19-bit)
    uint32_t x, y;          // Multiplier inputs (12-bit)
    uint32_t phi;           // Phase register (12-bit)
    uint32_t wf;            // Waveform select register (9-bit)
    uint32_t mul_result;    // Multiplier output (19-bit)

    // Flags
    bool carry;             // Adder carry flag
    bool clear_rqst;        // Clear request flag (for conditional writes)
    bool int_mod;           // Interrupt modification flag

    // Output accumulators (24-bit)
    int32_t l_acc;
    int32_t r_acc;
    uint8_t mix_l;          // Left output attenuation (3-bit, 0=mute, 7=0dB)
    uint8_t mix_r;          // Right output attenuation (3-bit)

    // Control state
    bool idle;              // IDL bit: all slots idle when true
    bool ssr_22khz;         // SSR bit: false=44.1kHz, true=22.05kHz
    uint8_t interrupt_slot; // Last slot that triggered interrupt (for debugging)

    // External waveform memory callbacks
    sam8905_waveform_read_fn waveform_read;
    sam8905_waveform_write_fn waveform_write;
    void* waveform_ctx;

} Sam8905Emu;

// -----------------------------------------------------------------------------
// Emulator Instance Management
// -----------------------------------------------------------------------------

// Set the active emulator instance for direct function calls.
// After calling this, sam8905_write_dram() etc. will operate on this instance.
void sam8905_emu_set_instance(Sam8905Emu* emu);

// Get the current active emulator instance (may be NULL)
Sam8905Emu* sam8905_emu_get_instance(void);

// -----------------------------------------------------------------------------
// Emulator Functions
// -----------------------------------------------------------------------------

// Initialize emulator to power-on state
void sam8905_emu_init(Sam8905Emu* emu);

// Reset emulator (clears registers but keeps RAM contents)
void sam8905_emu_reset(Sam8905Emu* emu);

// -----------------------------------------------------------------------------
// Waveform Memory Setup
// -----------------------------------------------------------------------------

// Set external waveform read callback
void sam8905_emu_set_waveform_read(Sam8905Emu* emu, sam8905_waveform_read_fn fn, void* ctx);

// Set external waveform write callback
void sam8905_emu_set_waveform_write(Sam8905Emu* emu, sam8905_waveform_write_fn fn, void* ctx);

// -----------------------------------------------------------------------------
// Direct Access (for debugging/testing)
// -----------------------------------------------------------------------------

// Process a single slot for one frame (exposed for unit testing)
void sam8905_emu_process_slot(Sam8905Emu* emu, int slot_idx);

// Execute a single micro-instruction (exposed for unit testing)
void sam8905_emu_execute_cycle(Sam8905Emu* emu, int slot_idx, uint16_t inst);
