// SAM8905 FX Chip Test Harness
// Captures CPU writes and simulates the algorithm to verify behavior
//
// Build: g++ -O2 -std=c++17 -o fx_test_harness fx_test_harness.cpp
// Run: ./fx_test_harness

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cmath>

constexpr uint32_t MASK19 = 0x7FFFF;

// Simulated SAM8905 state
struct SAM8905 {
    uint16_t aram[256];  // 256 x 15-bit instructions
    uint32_t dram[256];  // 256 x 19-bit parameters

    // Per-slot state
    struct Slot {
        uint32_t a, b;
        uint32_t phi, wf;
        uint32_t x, y;
        uint32_t mul_result;
        uint32_t l_acc, r_acc;
        uint8_t mix_l, mix_r;
        bool clear_rqst, int_mod, carry;
    } slots[16];

    uint8_t control_reg;

    void reset() {
        memset(aram, 0, sizeof(aram));
        memset(dram, 0, sizeof(dram));
        memset(slots, 0, sizeof(slots));
        control_reg = 0;
    }

    // Decode and print instruction
    void decode_inst(uint16_t inst, int pc) {
        uint8_t mad = (inst >> 11) & 0xF;
        uint8_t emitter = (inst >> 9) & 0x3;
        bool wsp = (inst >> 8) & 1;
        bool wa = !((inst >> 7) & 1);
        bool wb = !((inst >> 6) & 1);
        bool wm = !((inst >> 5) & 1);
        bool wphi = !((inst >> 4) & 1);
        bool wxy = !((inst >> 3) & 1);
        bool clearb = !((inst >> 2) & 1);
        bool wwf = !((inst >> 1) & 1);
        bool wacc = !(inst & 1);

        const char* emitter_names[] = {"RM", "RADD", "RP", "RSP"};

        printf("  PC%02d: 0x%04X %s %d", pc, inst, emitter_names[emitter], mad);
        if (wsp) printf(" WSP");
        if (wa) printf(" WA");
        if (wb) printf(" WB");
        if (wm) printf(" WM");
        if (wphi) printf(" WPHI");
        if (wxy) printf(" WXY");
        if (clearb) printf(" clearB");
        if (wwf) printf(" WWF");
        if (wacc) printf(" WACC");

        // Flag WXY WSP specifically
        if (wxy && wsp) printf(" [MIX UPDATE]");

        printf("\n");
    }

    // Execute one instruction for a slot
    void execute_cycle(int slot_idx, uint16_t inst, int pc) {
        Slot &slot = slots[slot_idx];
        uint8_t mad = (inst >> 11) & 0xF;
        uint8_t emitter_sel = (inst >> 9) & 0x3;
        bool wsp = (inst >> 8) & 1;

        uint32_t bus = 0;
        uint32_t dram_addr = (slot_idx << 4) | mad;

        // Emitters
        switch (emitter_sel) {
            case 0: bus = dram[dram_addr]; break;  // RM
            case 1: bus = (slot.a + slot.b) & MASK19; break;  // RADD
            case 2: bus = slot.mul_result; break;  // RP
            case 3: bus = 0; break;  // RSP
        }

        // Carry calculation
        auto update_carry = [&slot]() {
            bool b_neg = (slot.b >> 18) & 1;
            uint32_t sum = slot.a + slot.b;
            if (!b_neg)
                slot.carry = (sum > MASK19);
            else
                slot.carry = !((sum & MASK19) >> 18);
        };

        // WA (bit 7 active low)
        if (!((inst >> 7) & 1)) {
            // WA WSP has special behavior
            if (wsp && !((inst >> 4) & 1)) {
                // WPHI WSP takes priority - normal WA
                slot.a = bus & MASK19;
                slot.clear_rqst = true;
                slot.int_mod = false;
            } else if (wsp) {
                // WA WSP wave management
                bool finalwf = ((bus >> 9) & 0xFF) == (bus & 0x1FF);
                bool end = (bus >> 18) & 1;
                if (!slot.carry) {
                    slot.a = 0;
                    slot.clear_rqst = false;
                    slot.int_mod = true;
                } else if (!finalwf) {
                    slot.a = 0x200;
                    slot.clear_rqst = false;
                    slot.int_mod = true;
                } else if (!end) {
                    slot.a = 0;
                    slot.clear_rqst = false;
                    slot.int_mod = true;
                } else {
                    slot.a = 0;
                    slot.clear_rqst = true;
                    slot.int_mod = true;
                }
            } else {
                // Normal WA
                slot.a = bus & MASK19;
                slot.clear_rqst = true;
                slot.int_mod = false;
            }
            update_carry();
        }

        // WB (bit 6 active low)
        if (!((inst >> 6) & 1)) {
            slot.b = bus & MASK19;
            update_carry();
        }

        // WM (bit 5 active low) - write to D-RAM
        bool write_enable = !((inst >> 5) & 1);
        if (wsp && write_enable) {
            // WM WSP - conditional write
            if (slot.clear_rqst && !slot.carry) {
                dram[dram_addr] = bus;
            }
            // Otherwise blocked
        } else if (write_enable) {
            dram[dram_addr] = bus;
        }

        // WPHI (bit 4 active low)
        if (!((inst >> 4) & 1)) {
            slot.phi = (bus >> 7) & 0xFFF;
            if (wsp) slot.wf = 0x100;  // Force internal sinus
        }

        // WXY (bit 3 active low)
        if (!((inst >> 3) & 1)) {
            slot.y = (bus >> 7) & 0xFFF;
            // X comes from waveform - simplified here
            slot.x = 0;  // Would need waveform lookup

            // WSP inside WXY updates MIX
            if (wsp) {
                slot.mix_l = (bus >> 3) & 0x7;
                slot.mix_r = bus & 0x7;
                printf("    [MIX UPDATE slot %d: bus=0x%05X mix_l=%d mix_r=%d]\n",
                       slot_idx, bus, slot.mix_l, slot.mix_r);
            }

            // Calculate product
            int32_t product = (int32_t)(int16_t)(slot.x << 4) >> 4;
            product *= (int32_t)(int16_t)(slot.y << 4) >> 4;
            slot.mul_result = (uint32_t)(product >> 5) & MASK19;
        }

        // clearB (bit 2 active low)
        if (!((inst >> 2) & 1)) {
            slot.b = 0;
            update_carry();
        }

        // WWF (bit 1 active low)
        if (!((inst >> 1) & 1)) {
            slot.wf = (bus >> 9) & 0x1FF;
        }

        // WACC (bit 0 active low)
        if (!(inst & 1)) {
            // Accumulate to L/R based on mix
            int32_t signed_mul = (int32_t)(int16_t)(slot.mul_result << 13) >> 13;
            // Simplified - just track that WACC happened
        }
    }

    // Process one frame for a slot
    void process_slot(int slot_idx, bool verbose = false) {
        uint32_t param15 = dram[(slot_idx << 4) | 15];
        bool idle = (param15 >> 11) & 1;
        uint8_t alg = (param15 >> 8) & 0x7;

        if (idle) {
            if (verbose) printf("Slot %d: IDLE\n", slot_idx);
            return;
        }

        // 22kHz mode: 4 algorithms x 64 instructions
        bool ssr_mode = (control_reg >> 3) & 1;
        int pc_start, inst_count;

        if (ssr_mode) {
            alg &= 0x3;  // Only 2 bits in 22kHz mode
            pc_start = alg << 6;
            inst_count = 64;
        } else {
            pc_start = alg << 5;
            inst_count = 32;
        }

        if (verbose) {
            printf("Slot %d: ALG=%d (p15=0x%05X) pc_start=0x%02X inst_count=%d\n",
                   slot_idx, alg, param15, pc_start, inst_count);
        }

        // Clear accumulators
        slots[slot_idx].l_acc = 0;
        slots[slot_idx].r_acc = 0;

        // Execute instructions (skip last 2)
        for (int pc = 0; pc < inst_count - 2; ++pc) {
            execute_cycle(slot_idx, aram[pc_start + pc], pc);
        }

        if (verbose) {
            printf("  mix_l=%d mix_r=%d\n", slots[slot_idx].mix_l, slots[slot_idx].mix_r);
        }
    }
};

// FX chip D-RAM initialization from Keyfox10 firmware
// Captured from actual MAME execution
void load_fx_dram(SAM8905 &sam) {
    // Slot 7 D-RAM (example)
    int slot = 7;
    sam.dram[(slot << 4) | 0] = 0x2B800;
    sam.dram[(slot << 4) | 1] = 0x7FE80;
    sam.dram[(slot << 4) | 2] = 0x00400;
    sam.dram[(slot << 4) | 3] = 0x40603;
    sam.dram[(slot << 4) | 4] = 0x00180;
    sam.dram[(slot << 4) | 5] = 0x6667F;  // MIX value supposedly
    sam.dram[(slot << 4) | 6] = 0x79999;
    sam.dram[(slot << 4) | 7] = 0x7F800;
    sam.dram[(slot << 4) | 8] = 0x7FF00;
    sam.dram[(slot << 4) | 9] = 0x00000;
    sam.dram[(slot << 4) | 10] = 0x40000;
    sam.dram[(slot << 4) | 11] = 0x00000;
    sam.dram[(slot << 4) | 12] = 0x7F880;
    sam.dram[(slot << 4) | 13] = 0x00000;
    sam.dram[(slot << 4) | 14] = 0x00000;
    sam.dram[(slot << 4) | 15] = 0x3C480;  // ALG=4, IDLE=0, M=1
}

// ALG=0 A-RAM (64 instructions for 22kHz mode)
// This is what runs for slots with ALG=0,4 in 22kHz mode
void load_alg0_aram(SAM8905 &sam) {
    // First 32 instructions of ALG=0
    uint16_t alg0[] = {
        0x00F7, 0x607F, 0x58BF, 0x5A5F, 0x30BF, 0x5DDF, 0x082D, 0x593F,
        0x5ADF, 0x58F7, 0x406F, 0x2CDF, 0x48BF, 0x58F7, 0x42DF, 0x749F,
        0x68F7, 0x38FD, 0x7FFB, 0x7FFB, 0x7EFB, 0x7EFB, 0x7FFF, 0x406F,
        0x50BF, 0x42DF, 0x683F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F,
        // Second 32 instructions (for 22kHz 64-instruction mode)
        0x7A3F, 0x58F7, 0x7AF7, 0x7FFB, 0x7FFB, 0x7EFB, 0x7EFB, 0x7FFF,
        0x58F7, 0x7AFB, 0x7FFB, 0x7EFB, 0x7EFB, 0x7FFF, 0x20F7, 0x7AFB,
        0x7FFB, 0x7EFB, 0x7EFB, 0x20F7, 0x7AFB, 0x7FFB, 0x7EFB, 0x7EFB,
        0x10F7, 0x7AFB, 0x7FFB, 0x7EFB, 0x7EFB, 0x7EFB, 0x7FFF, 0x7FFF
    };

    for (int i = 0; i < 64; i++) {
        sam.aram[i] = alg0[i];
    }
}

// ALG=2 A-RAM (base 0x80 in 22kHz mode, for slots with ALG=2 or ALG=6)
void load_alg2_aram(SAM8905 &sam) {
    // ALG=2 starts at address 0x80 (2 << 6 = 128)
    // These are the actual instructions from MAME dump
    uint16_t alg2[] = {
        0x5ADF, 0x68BF, 0x72DF, 0x10FD, 0x006F, 0x18BF, 0x02DF, 0x29B7,
        0x707F, 0x7CBF, 0x7A7E, 0x72DE, 0x30F7, 0x20BF, 0x6CDF, 0x006F,
        0x02DF, 0x087F, 0x28F7, 0x0ADF, 0x7CEF, 0x78FD, 0x40F7, 0x687F,
        0x7CBF, 0x6ADF, 0x38F7, 0x707F, 0x7CBF, 0x725E, 0x50BF, 0x797B,
        // Second 32 instructions
        0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F,
        0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F,
        0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F,
        0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7FFF, 0x7FFF
    };

    for (int i = 0; i < 64; i++) {
        sam.aram[0x80 + i] = alg2[i];
    }
}

void check_algorithm(SAM8905 &sam, int alg_base, const char* name) {
    printf("=== %s Instructions (base=0x%02X) ===\n", name, alg_base);
    printf("Looking for WXY WSP instructions (bit 3=0 AND bit 8=1):\n\n");

    int wxy_wsp_count = 0;
    for (int i = 0; i < 64; i++) {
        uint16_t inst = sam.aram[alg_base + i];
        bool wxy = !((inst >> 3) & 1);
        bool wsp = (inst >> 8) & 1;

        if (wxy && wsp) {
            printf("  ** PC%02d: 0x%04X HAS WXY+WSP (MIX update) **\n", i, inst);
            sam.decode_inst(inst, i);
            wxy_wsp_count++;
        } else if (wsp) {
            printf("     PC%02d: 0x%04X has WSP but NOT WXY (no MIX update)\n", i, inst);
        }
    }

    printf("\nTotal WXY WSP instructions: %d\n\n", wxy_wsp_count);
}

int main() {
    SAM8905 sam;
    sam.reset();

    // Set 22kHz mode
    sam.control_reg = 0x08;  // SSR=1

    // Load A-RAM and D-RAM
    load_alg0_aram(sam);
    load_alg2_aram(sam);
    load_fx_dram(sam);

    printf("=== SAM8905 FX Test Harness ===\n\n");

    // Check ALG=0 (base 0x00) - used by slots with ALG=0 or ALG=4
    check_algorithm(sam, 0x00, "ALG=0 (slots 4,7-11)");

    // Check ALG=2 (base 0x80) - used by slots with ALG=2 or ALG=6
    check_algorithm(sam, 0x80, "ALG=2 (slots 5,6)");

    // Process slot 7
    printf("=== Processing Slot 7 (ALG=0) ===\n");
    sam.process_slot(7, true);

    printf("\nFinal slot 7 state:\n");
    printf("  mix_l = %d\n", sam.slots[7].mix_l);
    printf("  mix_r = %d\n", sam.slots[7].mix_r);

    // Check D-RAM word 5 value interpretation
    uint32_t word5 = sam.dram[(7 << 4) | 5];
    printf("\nD-RAM word 5 = 0x%05X\n", word5);
    printf("  If used for MIX via WXY WSP:\n");
    printf("    AMP (bits 18:7) = 0x%03X\n", (word5 >> 7) & 0xFFF);
    printf("    MIXL (bits 5:3) = %d\n", (word5 >> 3) & 0x7);
    printf("    MIXR (bits 2:0) = %d\n", word5 & 0x7);

    return 0;
}
