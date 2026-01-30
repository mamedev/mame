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
// SAM8905 emulator implementation - ported from MAME.

#include "sam8905_emu.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

/* Debug logging */
#define DEBUG_SAM(fmt, ...) do { printf("SAM: " fmt "\n", ##__VA_ARGS__); fflush(stdout); } while(0)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// -----------------------------------------------------------------------------
// Global Instance (for direct function interface)
// -----------------------------------------------------------------------------

static Sam8905Emu* g_emu_instance = NULL;

void sam8905_emu_set_instance(Sam8905Emu* emu) {
    g_emu_instance = emu;
}

Sam8905Emu* sam8905_emu_get_instance(void) {
    return g_emu_instance;
}

// -----------------------------------------------------------------------------
// Helper Macros
// -----------------------------------------------------------------------------

#define BIT(value, bit)  (((value) >> (bit)) & 1)
#define MASK19           SAM8905_MASK19

// Clamp value to range
static inline int32_t clamp_i32(int32_t val, int32_t min_val, int32_t max_val) {
    if (val < min_val) return min_val;
    if (val > max_val) return max_val;
    return val;
}

// -----------------------------------------------------------------------------
// Internal: Constant Lookup
// -----------------------------------------------------------------------------

// Constants derived from MAD field (Appendix I of programmer's guide)
// X is fractional Q0.11 format
static uint32_t sam8905_get_constant(uint8_t mad) {
    static const uint32_t constants[16] = {
        0x001, 0x081, 0x101, 0x181, 0x201, 0x281, 0x301, 0x381,
        0x401, 0x481, 0x501, 0x581, 0x601, 0x681, 0x701, 0x781
    };
    return constants[mad & 0xF];
}

// -----------------------------------------------------------------------------
// Internal: Waveform Generation
// -----------------------------------------------------------------------------

// Get waveform sample based on WF register and phase
// Returns 12-bit signed value (-2048 to +2047)
static int32_t sam8905_get_waveform(Sam8905Emu* emu, uint32_t wf, uint32_t phi, uint8_t mad) {
    phi &= 0xFFF;  // 12-bit phase
    bool internal = (wf & 0x100) != 0;  // WF[8] = INT/EXT

    if (internal) {
        // WF[8]=1: Internal waveform
        // Bit layout (from MAME working version):
        // WF[7] = I (invert), WF[6] = R (ramp mode), WF[5:4] = SEL, WF[3] = Z
        bool z_bit = (wf & 0x08) != 0;      // WF[3] = Z (zero select)
        int sel = (wf >> 4) & 3;            // WF[5:4] = SEL (ramp type)
        bool invert = (wf & 0x80) != 0;     // WF[7] = I
        bool ramp_mode = (wf & 0x40) != 0;  // WF[6] = R

        if (z_bit) {
            return 0;  // Zero constant
        }

        int32_t result;
        if (!ramp_mode) {
            // R=0: Sinus wave
            // X = .71875 sin((PI/2048) * PHI + PI/4096)
            double angle = (M_PI / 2048.0) * (double)phi + (M_PI / 4096.0);
            result = (int32_t)(0.71875 * sin(angle) * 2048.0);
        } else {
            // Ramps based on SEL bits
            switch (sel) {
                case 0:  // 2x PHI ramp
                    result = (phi < 1024) ? (int32_t)phi * 2 :
                             (phi < 3072) ? (int32_t)phi * 2 - 4096 :
                                            (int32_t)phi * 2 - 8192;
                    break;
                case 1:  // Constant from micro-instruction MAD field
                    result = (int32_t)sam8905_get_constant(mad);
                    break;
                case 2:  // PHI ramp
                    result = (phi < 2048) ? (int32_t)phi : (int32_t)phi - 4096;
                    break;
                case 3:  // PHI/2 ramp
                    result = (phi < 2048) ? (int32_t)phi / 2 : (int32_t)phi / 2 - 2048;
                    break;
                default:
                    result = 0;
            }
        }

        // Apply invert (I bit)
        if (invert) {
            result = (-result) & 0xFFF;
            if (result & 0x800) result |= ~0xFFF;  // Sign extend
        }

        result &= 0xFFF;
        return result;
    } else {
        // External memory access
        if (emu->waveform_read) {
            // Build 20-bit address: WAVE[7:0] | PHI[11:0]
            uint32_t addr = ((wf & 0xFF) << 12) | phi;
            return emu->waveform_read(emu->waveform_ctx, addr);
        }
        return 0;
    }
}

// -----------------------------------------------------------------------------
// Core: Execute One Micro-Instruction
// -----------------------------------------------------------------------------

void sam8905_emu_execute_cycle(Sam8905Emu* emu, int slot_idx, uint16_t inst) {
    // Instruction fields
    uint8_t mad = (inst >> 11) & 0xF;           // D-RAM address within slot
    uint8_t emitter_sel = (inst >> 9) & 0x3;    // Emitter selection
    bool wsp = BIT(inst, 8);                    // WSP modifier

    uint32_t bus = emu->bus;
    uint32_t dram_addr = (slot_idx << 4) | mad;

    // Emitters: select what drives the bus
    switch (emitter_sel) {
        case 0:  // RM: Read from D-RAM
            bus = emu->dram[dram_addr];
            break;
        case 1:  // RADD: Adder result
            bus = (emu->a + emu->b) & MASK19;
            break;
        case 2:  // RP: Multiplier result
            bus = emu->mul_result;
            break;
        case 3:  // RSP: NOP (bus unchanged)
            break;
    }
    emu->bus = bus;

    // Receivers (active LOW except WSP)

    // WA (Write A) - bit 7
    if (!BIT(inst, 7)) {
        bool wphi_active = !BIT(inst, 4);

        // "WPHI WSP takes priority over WA WSP giving a normal WA" (Section 8-3)
        if (wsp && !wphi_active) {
            // WA WSP: conditional write based on carry and wave matching
            uint32_t wave = (bus >> 9) & 0x1FF;
            uint32_t final_wave = bus & 0x1FF;
            bool end_bit = BIT(bus, 18);
            bool wf_match = (wave == final_wave);

            if (!emu->carry) {
                emu->a = 0;
                emu->clear_rqst = false;
                emu->int_mod = true;
            } else if (!wf_match) {
                emu->a = 0x200;
                emu->clear_rqst = false;
                emu->int_mod = true;
            } else {
                emu->a = 0;
                emu->int_mod = true;
                emu->clear_rqst = end_bit;
            }
        } else {
            // Normal WA: load A from bus
            emu->a = bus;
            emu->clear_rqst = true;
            emu->int_mod = false;
        }
    }

    // WB (Write B) - bit 6
    if (!BIT(inst, 6)) {
        emu->b = bus;
    }

    // WM (Write Memory) - bit 5
    if (!BIT(inst, 5)) {
        bool write_enable = true;
        if (wsp) {
            // WM WSP: conditional write
            if (!emu->clear_rqst) {
                write_enable = false;
            } else if (emu->carry) {
                write_enable = false;
            }
            // Interrupt latch update
            bool irq_condition = emu->clear_rqst && (emu->int_mod || emu->carry);
            if (irq_condition) {
                emu->interrupt_slot = (slot_idx << 4) | mad;
            }
        }
        if (write_enable) {
            emu->dram[dram_addr] = bus;
        }
    }

    // WPHI (Write Phase) - bit 4
    if (!BIT(inst, 4)) {
        emu->phi = (bus >> 7) & 0xFFF;  // Upper 12 bits
        if (wsp) {
            emu->wf = 0x100;  // Force internal sinus
        }
    }

    // WXY (Write X and Y) - bit 3
    if (!BIT(inst, 3)) {
        emu->y = (bus >> 7) & 0xFFF;
        emu->x = sam8905_get_waveform(emu, emu->wf, emu->phi, mad) & 0xFFF;
        if (wsp) {
            emu->mix_l = (bus >> 3) & 0x7;
            emu->mix_r = bus & 0x7;
        }
    }

    // clearB - bit 2
    if (!BIT(inst, 2)) {
        emu->b = 0;

        // WWE (Write Waveform Enable) - RSP + clearB + WSP triggers external write
        if (emitter_sel == 3 && wsp && emu->waveform_write) {
            if ((emu->wf & 0x1FF) < 0x80) {
                uint32_t ext_addr = ((emu->wf & 0x7) << 12) | (emu->phi & 0xFFF);
                int16_t ext_data = emu->y & 0xFFF;
                emu->waveform_write(emu->waveform_ctx, ext_addr, ext_data);
            }
        }
    }

    // WWF (Write Waveform) - bit 1
    if (!BIT(inst, 1)) {
        emu->wf = (bus >> 9) & 0x1FF;
    }

    // WACC (Accumulate) - bit 0
    if (!BIT(inst, 0)) {
        // dB attenuation lookup: 000=mute, 001=-36dB, ... 111=0dB
        static const uint16_t mix_atten[8] = { 0, 16, 32, 64, 128, 256, 512, 1024 };

        // Sign-extend 19-bit mul_result
        int32_t signed_mul = (int32_t)(emu->mul_result & MASK19);
        if (signed_mul & 0x40000) signed_mul |= (int32_t)0xFFF80000;

        // Apply attenuation and accumulate
        int32_t l_contrib = (signed_mul * mix_atten[emu->mix_l]) >> 10;
        int32_t r_contrib = (signed_mul * mix_atten[emu->mix_r]) >> 10;

        emu->l_acc = clamp_i32(emu->l_acc + l_contrib, -(1 << 24), (1 << 24) - 1);
        emu->r_acc = clamp_i32(emu->r_acc + r_contrib, -(1 << 24), (1 << 24) - 1);
    }

    // Update carry for next cycle
    // Carry calculation per Section 8-1:
    // - B positive: carry = 19-bit overflow
    // - B negative: carry = result is positive
    {
        bool b_neg = BIT(emu->b, 18);
        uint32_t sum = emu->a + emu->b;
        if (!b_neg) {
            emu->carry = (sum > MASK19);
        } else {
            emu->carry = !BIT(sum & MASK19, 18);
        }
    }

    // Update multiplier result (combinational - always computing)
    {
        int32_t x_signed = (int32_t)((int16_t)(emu->x << 4) >> 4);
        int32_t y_signed = (int32_t)((int16_t)(emu->y << 4) >> 4);
        // Q0.11 * Q0.11 = Q0.22, round to Q0.18 (shift right by 4)
        int32_t product = x_signed * y_signed;
        emu->mul_result = (uint32_t)((product + 8) >> 4) & MASK19;
    }
}

// -----------------------------------------------------------------------------
// Core: Process One Slot
// -----------------------------------------------------------------------------

void sam8905_emu_process_slot(Sam8905Emu* emu, int slot_idx) {
    // Fetch slot configuration from word 15
    uint32_t param15 = emu->dram[(slot_idx << 4) | 15];

    // Check idle bit
    if (BIT(param15, SAM8905_DRAM15_IDLE_BIT)) {
        return;  // Slot is idle, produces no sound
    }

    // Get algorithm and instruction count based on sample rate
    uint16_t pc_start;
    int inst_count;

    if (emu->ssr_22khz) {
        // 22.05kHz: 4 algorithms x 64 instructions
        // ALG uses bits 10:9
        uint8_t alg = (param15 >> 9) & SAM8905_DRAM15_ALG_MASK_22K;
        pc_start = alg << 6;
        inst_count = SAM8905_ALGO_SIZE_22K;
    } else {
        // 44.1kHz: 8 algorithms x 32 instructions
        // ALG uses bits 10:8
        uint8_t alg = (param15 >> SAM8905_DRAM15_ALG_SHIFT) & SAM8905_DRAM15_ALG_MASK_44K;
        pc_start = alg << 5;
        inst_count = SAM8905_ALGO_SIZE_44K;
    }

    // Execute algorithm (skip last 2 reserved instructions)
    for (int pc = 0; pc < inst_count - 2; pc++) {
        sam8905_emu_execute_cycle(emu, slot_idx, emu->aram[pc_start + pc]);
    }
}

// -----------------------------------------------------------------------------
// Core: Process One Audio Frame (internal)
// -----------------------------------------------------------------------------

static void sam8905_emu_process_frame_impl(Sam8905Emu* emu, int32_t* out_l, int32_t* out_r) {
    // Clear accumulators at start of frame
    emu->l_acc = 0;
    emu->r_acc = 0;

    // Process all 16 slots (unless globally idle)
    if (!emu->idle) {
        for (int slot = 0; slot < SAM8905_NUM_SLOTS; slot++) {
            sam8905_emu_process_slot(emu, slot);
        }
    }

    // Output: shift 24-bit accumulator to 16-bit range
    // Using 9-bit shift as in MAME (DAC_SHIFT)
    *out_l = emu->l_acc >> 9;
    *out_r = emu->r_acc >> 9;
}

// -----------------------------------------------------------------------------
// Direct Function Interface (sam8905.h)
// These operate on the global emulator instance set by sam8905_emu_set_instance()
// -----------------------------------------------------------------------------

void sam8905_write_aram(uint8_t addr, uint16_t inst) {
    if (g_emu_instance) {
        g_emu_instance->aram[addr] = inst & 0x7FFF;  // 15-bit
    }
}

void sam8905_write_dram(uint8_t addr, uint32_t param) {
    if (g_emu_instance) {
        uint8_t slot = addr >> 4;
        uint8_t word = addr & 0x0F;
        param &= MASK19;
        DEBUG_SAM("DRAM[%d][%d] = 0x%05X (addr=0x%02X)", slot, word, param, addr);
        g_emu_instance->dram[addr] = param;

        /* Check if slot becomes active (word 15, bit 17 = idle flag) */
        if (word == 15) {
            int is_idle = (param & SAM8905_SLOT_IDLE) != 0;
            DEBUG_SAM("  Slot %d %s (alg=%d)", slot, is_idle ? "IDLE" : "ACTIVE", param & 0x07);
        }
    }
}

uint16_t sam8905_read_aram(uint8_t addr) {
    if (g_emu_instance) {
        return g_emu_instance->aram[addr];
    }
    return 0;
}

uint32_t sam8905_read_dram(uint8_t addr) {
    if (g_emu_instance) {
        return g_emu_instance->dram[addr];
    }
    return 0;
}

void sam8905_set_idle(bool idle) {
    if (g_emu_instance) {
        g_emu_instance->idle = idle;
    }
}

void sam8905_set_sample_rate(bool ssr_22khz) {
    if (g_emu_instance) {
        g_emu_instance->ssr_22khz = ssr_22khz;
    }
}

void sam8905_init(void) {
    if (g_emu_instance) {
        sam8905_emu_init(g_emu_instance);
    }
}

void sam8905_process_frame(int32_t* out_l, int32_t* out_r) {
    if (g_emu_instance) {
        sam8905_emu_process_frame_impl(g_emu_instance, out_l, out_r);
    } else {
        *out_l = 0;
        *out_r = 0;
    }
}

// -----------------------------------------------------------------------------
// Emulator API
// -----------------------------------------------------------------------------

void sam8905_emu_init(Sam8905Emu* emu) {
    memset(emu, 0, sizeof(Sam8905Emu));
    // All slots start idle
    for (int slot = 0; slot < SAM8905_NUM_SLOTS; slot++) {
        emu->dram[(slot << 4) | 15] = SAM8905_SLOT_IDLE;
    }
}

void sam8905_emu_reset(Sam8905Emu* emu) {
    // Reset registers but keep RAM contents
    emu->bus = 0;
    emu->a = emu->b = 0;
    emu->x = emu->y = 0;
    emu->phi = emu->wf = 0;
    emu->mul_result = 0;
    emu->carry = false;
    emu->clear_rqst = false;
    emu->int_mod = false;
    emu->l_acc = emu->r_acc = 0;
    emu->mix_l = emu->mix_r = 0;
    emu->idle = false;
    emu->ssr_22khz = false;
    emu->interrupt_slot = 0;
}

void sam8905_emu_set_waveform_read(Sam8905Emu* emu, sam8905_waveform_read_fn fn, void* ctx) {
    emu->waveform_read = fn;
    emu->waveform_ctx = ctx;
}

void sam8905_emu_set_waveform_write(Sam8905Emu* emu, sam8905_waveform_write_fn fn, void* ctx) {
    emu->waveform_write = fn;
    emu->waveform_ctx = ctx;
}
