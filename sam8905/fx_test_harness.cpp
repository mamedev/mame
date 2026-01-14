// SAM8905 FX Chip Test Harness
// Tests FX reverb algorithm with simulated SND input
//
// Build: g++ -O2 -std=c++17 -o fx_test_harness fx_test_harness.cpp
// Run: ./fx_test_harness

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <vector>

constexpr uint32_t MASK19 = 0x7FFFF;
constexpr size_t FX_SRAM_SIZE = 32768;

// dB attenuation lookup (same as sam8905.cpp)
static constexpr uint16_t MIX_ATTEN[8] = { 0, 16, 32, 64, 128, 256, 512, 1024 };

// Simulated SAM8905 FX state
struct SAM8905_FX {
    uint16_t aram[256];     // 256 x 15-bit instructions
    uint32_t dram[256];     // 256 x 19-bit parameters
    int8_t sram[FX_SRAM_SIZE];  // 32KB SRAM (8-bit per location)

    // Per-slot state
    struct Slot {
        uint32_t a, b;
        uint32_t phi, wf;
        int16_t x, y;       // 12-bit signed
        uint32_t mul_result;
        int32_t l_acc, r_acc;
        uint8_t mix_l, mix_r;
        bool clear_rqst, int_mod, carry;
    } slots[16];

    uint8_t control_reg;

    // Input samples from SND chip
    int16_t input_l, input_r;

    // Global output
    int32_t out_l, out_r;

    // Debug
    bool verbose;
    int frame_count;

    void reset() {
        memset(aram, 0, sizeof(aram));
        memset(dram, 0, sizeof(dram));
        memset(sram, 0, sizeof(sram));
        memset(slots, 0, sizeof(slots));
        control_reg = 0;
        input_l = input_r = 0;
        out_l = out_r = 0;
        verbose = false;
        frame_count = 0;
    }

    // Get waveform sample - includes SRAM and input sample access
    int16_t get_waveform(uint16_t wf, uint16_t phi, int slot_idx) {
        // Build 20-bit address: WA[19:0] = { WAVE[7:0], PHI[11:0] }
        uint32_t wa = ((wf & 0xFF) << 12) | (phi & 0xFFF);

        // WA19 = 1: Read input sample from SND
        if (wa & 0x80000) {
            bool is_right = (wa & 1);
            int16_t sample = is_right ? input_r : input_l;

            // Convert 16-bit to 12-bit (take upper 8 bits, shift to bits 10:3)
            int8_t sample_8bit = sample >> 8;
            int16_t result = ((int16_t)sample_8bit) << 3;

            // Sign extend bit 10 to bit 11
            if (result & 0x400)
                result |= 0x800;

            if (verbose && sample != 0) {
                printf("  [waveform: INPUT ch=%c raw=%d 8bit=%d result=%d]\n",
                       is_right ? 'R' : 'L', sample, sample_8bit, result);
            }
            return result & 0xFFF;
        }

        // SRAM access: WF[6:0] < 0x80
        if ((wf & 0x7F) < 0x80 && (wf & 0xFF) < 0x80) {
            uint32_t sram_addr = ((wf & 0x7F) << 8) | ((phi >> 4) & 0xFF);
            if (sram_addr < FX_SRAM_SIZE) {
                int8_t sram_8bit = sram[sram_addr];
                int16_t result = ((int16_t)sram_8bit) << 3;

                // WAH0 = PHI[4]
                bool wah0 = (phi >> 4) & 1;
                if (!wah0) {
                    // Sign extend when WAH0=0
                    if (result & 0x400)
                        result |= 0x800;
                } else {
                    result &= 0x7FF;
                }

                if (verbose && sram_8bit != 0 && frame_count < 5) {
                    printf("  [waveform: SRAM[%04X]=%d -> %d wah0=%d]\n",
                           sram_addr, sram_8bit, result, wah0);
                }
                return result & 0xFFF;
            }
        }

        // Internal waveform (WF >= 0x100): sine table
        if (wf >= 0x100) {
            // Simplified sine lookup
            double phase = (phi & 0x3FF) / 1024.0 * 2.0 * M_PI;
            int16_t sine = (int16_t)(sin(phase) * 2047);
            return sine & 0xFFF;
        }

        return 0;
    }

    // Write to external SRAM
    void write_sram(uint16_t wf, uint16_t phi, int16_t data) {
        if ((wf & 0x1FF) < 0x80) {
            uint32_t sram_addr = ((wf & 0x7F) << 8) | ((phi >> 4) & 0xFF);
            if (sram_addr < FX_SRAM_SIZE) {
                // Store only 8 bits (bits 10:3 of 12-bit data)
                int8_t data_8bit = (data >> 3) & 0xFF;
                sram[sram_addr] = data_8bit;

                if (verbose && frame_count < 5) {
                    printf("  [SRAM WRITE: addr=%04X data12=%d data8=%d]\n",
                           sram_addr, data, data_8bit);
                }
            }
        }
    }

    // Execute one instruction for a slot
    void execute_cycle(int slot_idx, uint16_t inst, int pc_start, int pc) {
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
            if (wsp && !((inst >> 4) & 1)) {
                slot.a = bus & MASK19;
                slot.clear_rqst = true;
                slot.int_mod = false;
            } else if (wsp) {
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
            if (slot.clear_rqst && !slot.carry) {
                dram[dram_addr] = bus;
            }
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
            // Sign extend Y to 12-bit signed
            if (slot.y & 0x800) slot.y |= 0xF000;

            // X comes from waveform lookup
            slot.x = get_waveform(slot.wf, slot.phi, slot_idx);
            // Sign extend X
            if (slot.x & 0x800) slot.x |= 0xF000;

            // WSP inside WXY updates MIX
            if (wsp) {
                slot.mix_l = (bus >> 3) & 0x7;
                slot.mix_r = bus & 0x7;
                if (verbose && frame_count < 3) {
                    printf("  [PC%02d MIX UPDATE: bus=0x%05X mix_l=%d mix_r=%d]\n",
                           pc, bus, slot.mix_l, slot.mix_r);
                }
            }

            // Calculate product (12x12 fractional)
            int32_t product = (int32_t)slot.x * (int32_t)slot.y;
            slot.mul_result = (uint32_t)(product >> 5) & MASK19;

            if (verbose && slot.x != 0 && frame_count < 5) {
                printf("  [PC%02d WXY: wf=0x%03X phi=0x%03X x=%d y=%d mul=%d]\n",
                       pc, slot.wf, slot.phi, slot.x, slot.y,
                       (int32_t)(slot.mul_result << 13) >> 13);
            }
        }

        // clearB (bit 2 active low)
        if (!((inst >> 2) & 1)) {
            slot.b = 0;
            update_carry();

            // WWE (Write Waveform Enable) - triggered by RSP + clearB + WSP
            // Per SAM8905 datasheet Section 9: WWE writes Y register, not A
            if (emitter_sel == 3 && wsp) {
                if ((slot.wf & 0x1FF) < 0x80) {
                    // Write Y register (12-bit) to SRAM
                    int16_t write_data = slot.y & 0xFFF;
                    if (write_data & 0x800) write_data |= 0xF000;
                    write_sram(slot.wf, slot.phi, write_data);
                }
            }
        }

        // WWF (bit 1 active low)
        if (!((inst >> 1) & 1)) {
            slot.wf = (bus >> 9) & 0x1FF;
        }

        // WACC (bit 0 active low)
        if (!(inst & 1)) {
            // Sign-extend 19-bit mul_result
            int32_t signed_mul = (slot.mul_result & MASK19);
            if (signed_mul & 0x40000) signed_mul |= 0xFFF80000;

            // Apply dB attenuation
            int32_t l_contrib = (signed_mul * MIX_ATTEN[slot.mix_l]) >> 10;
            int32_t r_contrib = (signed_mul * MIX_ATTEN[slot.mix_r]) >> 10;
            slot.l_acc += l_contrib;
            slot.r_acc += r_contrib;

            if (verbose && (l_contrib != 0 || r_contrib != 0) && frame_count < 10) {
                printf("  [PC%02d WACC: mul=%d mix_l=%d mix_r=%d l_c=%d r_c=%d]\n",
                       pc, signed_mul, slot.mix_l, slot.mix_r, l_contrib, r_contrib);
            }
        }
    }

    // Process one frame for a slot
    void process_slot(int slot_idx) {
        uint32_t param15 = dram[(slot_idx << 4) | 15];
        bool idle = (param15 >> 11) & 1;

        if (idle) return;

        // 22kHz mode: 4 algorithms x 64 instructions
        bool ssr_mode = (control_reg >> 3) & 1;
        int pc_start, inst_count;
        uint8_t alg;

        if (ssr_mode) {
            // Per Table 6: Use AL2,AL1 (bits 10-9), not AL1,AL0 (bits 9-8)
            alg = (param15 >> 9) & 0x3;
            pc_start = alg << 6;
            inst_count = 64;
        } else {
            alg = (param15 >> 8) & 0x7;
            pc_start = alg << 5;
            inst_count = 32;
        }

        // Clear accumulators
        slots[slot_idx].l_acc = 0;
        slots[slot_idx].r_acc = 0;

        // Execute instructions (skip last 2)
        for (int pc = 0; pc < inst_count - 2; ++pc) {
            execute_cycle(slot_idx, aram[pc_start + pc], pc_start, pc);
        }
    }

    // Process all 16 slots
    void process_frame() {
        out_l = 0;
        out_r = 0;

        for (int s = 0; s < 16; ++s) {
            process_slot(s);
            out_l += slots[s].l_acc;
            out_r += slots[s].r_acc;
        }

        frame_count++;
    }
};

// Load FX chip D-RAM from actual firmware values (complete configuration)
void load_fx_dram(SAM8905_FX &sam) {
    // Mark all slots as IDLE first
    for (int slot = 0; slot < 16; slot++) {
        sam.dram[(slot << 4) | 15] = 0x00800;  // IDLE=1
    }

    // Slot 4: ALG=0 - Input processing
    {
        int slot = 4;
        uint32_t dram[] = {
            0x00000,  // word 0: PHI
            0x50080,  // word 1: DPHI/config - WF=0x80 (input sample access!)
            0x00400,  // word 2: WWF config
            0x40000,  // word 3: Config
            0x00080,  // word 4: Config
            0x00000,  // word 5: Accumulator
            0x7FFFF,  // word 6: Max amplitude
            0x40402,  // word 7: WWF/address
            0x00100,  // word 8: PHI increment
            0x00080,  // word 9: Config
            0x00180,  // word 10: Config
            0x0007C,  // word 11: Feedback coef
            0x00000,  // word 12
            0x00000,  // word 13
            0x00000,  // word 14
            0x34080,  // word 15: IDLE=0, ALG=0
        };
        for (int i = 0; i < 16; i++)
            sam.dram[(slot << 4) | i] = dram[i];
    }

    // Slot 5: ALG=2 - Diffusion (22kHz ALG=1)
    {
        int slot = 5;
        uint32_t dram[] = {
            0x00080,  // word 0: PHI offset
            0x00180,  // word 1
            0x1003F,  // word 2: WWF config
            0x10000,  // word 3: Amplitude
            0x00100,  // word 4
            0x00100,  // word 5
            0x3FF00,  // word 6: Large delay address
            0x6DF00,  // word 7: Delay address
            0x5A100,  // word 8: Delay address
            0x40402,  // word 9: WWF/SRAM address
            0x40402,  // word 10: WWF/SRAM address
            0x40402,  // word 11: WWF/SRAM address
            0x40402,  // word 12: WWF/SRAM address
            0x00000,  // word 13
            0x40000,  // word 14
            0x3C280,  // word 15: IDLE=0, ALG=2 (22kHz ALG=1)
        };
        for (int i = 0; i < 16; i++)
            sam.dram[(slot << 4) | i] = dram[i];
    }

    // Slot 6: ALG=6 - All-pass filter (22kHz ALG=3)
    {
        int slot = 6;
        uint32_t dram[] = {
            0x5F000,  // word 0: Large SRAM address
            0x7FC80,  // word 1: Near-max address
            0x00600,  // word 2
            0x00080,  // word 3
            0x00380,  // word 4
            0x00000,  // word 5: MIX=0 (MUTED!)
            0x00000,  // word 6
            0x52C00,  // word 7: SRAM address
            0x2D430,  // word 8: SRAM address (comb delay)
            0x00400,  // word 9
            0x7FC00,  // word 10
            0x00000,  // word 11
            0x34000,  // word 12: WWF config
            0x00000,  // word 13
            0x00000,  // word 14
            0x00680,  // word 15: IDLE=0, ALG=6 (22kHz ALG=3)
        };
        for (int i = 0; i < 16; i++)
            sam.dram[(slot << 4) | i] = dram[i];
    }

    // Slots 7-11: ALG=4 - Delay taps (22kHz ALG=2)
    // Different delay addresses for each slot
    uint32_t slot7_11_word0[] = {0x2B800, 0x23500, 0x1AE00, 0x0F700, 0x08C00};
    uint32_t slot7_11_word5[] = {0x6667F, 0x66A7F, 0x66EBF, 0x674BF, 0x677FF};
    uint32_t slot7_11_word6[] = {0x79999, 0x79A9F, 0x79BA5, 0x79D2F, 0x79DF3};

    for (int i = 0; i < 5; i++) {
        int slot = 7 + i;
        uint32_t dram[] = {
            slot7_11_word0[i],  // word 0: SRAM delay address
            0x7FE80,            // word 1
            0x00400,            // word 2: WWF config (WF=2, SRAM bank)
            0x40603,            // word 3
            0x00180,            // word 4
            slot7_11_word5[i],  // word 5: MIX value (mix_l=7, mix_r=7 + amplitude)
            slot7_11_word6[i],  // word 6
            0x7F800,            // word 7
            0x7FF00,            // word 8
            0x00000,            // word 9
            0x40000,            // word 10
            0x00000,            // word 11
            0x7F880,            // word 12
            0x00000,            // word 13
            0x00000,            // word 14
            0x3C480,            // word 15: IDLE=0, ALG=4 (22kHz ALG=2)
        };
        for (int j = 0; j < 16; j++)
            sam.dram[(slot << 4) | j] = dram[j];
    }
}

// ALG=0 A-RAM (64 instructions for 22kHz mode) - actual dump from MAME
void load_alg0_aram(SAM8905_FX &sam) {
    uint16_t alg0[] = {
        0x00F7, 0x607F, 0x58BF, 0x5A5F, 0x30BF, 0x5DDF, 0x082D, 0x593F,
        0x5ADF, 0x58F7, 0x406F, 0x2CDF, 0x48BF, 0x58F7, 0x42DF, 0x749F,
        0x68F7, 0x38FD, 0x7FFB, 0x7FFB, 0x7EFB, 0x7EFB, 0x7FFF, 0x406F,
        0x50BF, 0x42DF, 0x683F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F,
        0x7A3F, 0x7A3F, 0x7AF7, 0x7FFB, 0x7FFB, 0x7EFB, 0x7EFB, 0x7FFF,
        0x78FD, 0x18EF, 0x58F7, 0x7FFF, 0x50EF, 0x08FD, 0x24DF, 0x7FFF,
        0x20F7, 0x287F, 0x00EF, 0x7CBF, 0x2ADF, 0x20F7, 0x707F, 0x7CBF,
        0x28EF, 0x78FD, 0x10F7, 0x7A7F, 0x7CBF, 0x6A5B, 0x7FFF, 0x7FFF,
    };

    for (int i = 0; i < 64; i++) {
        sam.aram[i] = alg0[i];
    }
}

// ALG=2 A-RAM (base 0x80) - actual dump from MAME
void load_alg2_aram(SAM8905_FX &sam) {
    uint16_t alg2[] = {
        0x5ADF, 0x68BF, 0x72DF, 0x10FD, 0x006F, 0x18BF, 0x02DF, 0x29B7,
        0x707F, 0x7CBF, 0x7A7E, 0x72DE, 0x30F7, 0x20BF, 0x6CDF, 0x006F,
        0x02DF, 0x087F, 0x28F7, 0x0ADF, 0x7CEF, 0x78FD, 0x40F7, 0x687F,
        0x7CBF, 0x6ADF, 0x38F7, 0x707F, 0x7CBF, 0x725E, 0x50BF, 0x797B,
        0x7AEF, 0x78FD, 0x60F7, 0x70BF, 0x7C7F, 0x72D7, 0x10FD, 0x086F,
        0x7FFB, 0x7FFB, 0x7EFB, 0x7EFB, 0x7FFF, 0x18BF, 0x0ACF, 0x703F,
        0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7AF7,
        0x7FFB, 0x7FFB, 0x7EFB, 0x7EFB, 0x7FFF, 0x587B, 0x7FFF, 0x7FFF,
    };

    for (int i = 0; i < 64; i++) {
        sam.aram[0x80 + i] = alg2[i];
    }
}

void test_with_sine_input(SAM8905_FX &sam, int num_frames, int freq_hz) {
    printf("\n=== Testing with %d Hz sine input, %d frames ===\n", freq_hz, num_frames);

    double sample_rate = 22050.0;
    double phase_inc = 2.0 * M_PI * freq_hz / sample_rate;
    double phase = 0;

    int32_t max_out = 0, min_out = 0;

    for (int f = 0; f < num_frames; f++) {
        // Generate sine input (16-bit)
        int16_t sine_sample = (int16_t)(sin(phase) * 16000);
        phase += phase_inc;

        // SND outputs L=0, R=audio (like real keyfox10)
        sam.input_l = 0;
        sam.input_r = sine_sample;

        sam.process_frame();

        if (sam.out_l > max_out) max_out = sam.out_l;
        if (sam.out_l < min_out) min_out = sam.out_l;

        if (f < 10 || f == num_frames - 1) {
            printf("Frame %3d: in_r=%6d -> out_l=%8d out_r=%8d\n",
                   f, sine_sample, sam.out_l, sam.out_r);
        } else if (f == 10) {
            printf("...\n");
        }
    }

    printf("\nOutput range: [%d, %d]\n", min_out, max_out);
}

void test_impulse_response(SAM8905_FX &sam, int num_frames) {
    printf("\n=== Testing impulse response (%d frames) ===\n", num_frames);

    int32_t max_out = 0, min_out = 0;

    for (int f = 0; f < num_frames; f++) {
        // Single impulse at frame 0
        sam.input_l = 0;
        sam.input_r = (f == 0) ? 16000 : 0;

        sam.process_frame();

        if (sam.out_l > max_out) max_out = sam.out_l;
        if (sam.out_l < min_out) min_out = sam.out_l;

        if (f < 20 || sam.out_l != 0 || sam.out_r != 0) {
            printf("Frame %3d: in_r=%6d -> out_l=%8d out_r=%8d\n",
                   f, (f == 0) ? 16000 : 0, sam.out_l, sam.out_r);
        }

        // Stop if output goes silent for a while
        if (f > 100 && sam.out_l == 0 && sam.out_r == 0) {
            printf("Output silent at frame %d\n", f);
            break;
        }
    }

    printf("\nOutput range: [%d, %d]\n", min_out, max_out);
}

void analyze_algorithm(SAM8905_FX &sam, int alg_base, const char* name) {
    printf("\n=== Analyzing %s (base=0x%02X) ===\n", name, alg_base);

    int wxy_wsp_count = 0;
    int wacc_count = 0;
    int sram_read_count = 0;
    int input_read_count = 0;

    for (int i = 0; i < 62; i++) {  // Skip last 2 instructions
        uint16_t inst = sam.aram[alg_base + i];
        uint8_t mad = (inst >> 11) & 0xF;
        uint8_t emitter = (inst >> 9) & 0x3;
        bool wsp = (inst >> 8) & 1;
        bool wxy = !((inst >> 3) & 1);
        bool wwf = !((inst >> 1) & 1);
        bool wacc = !(inst & 1);
        bool clearb = !((inst >> 2) & 1);

        if (wxy && wsp) wxy_wsp_count++;
        if (wacc) wacc_count++;

        // Check for external waveform access patterns
        // WWE = RSP + clearB + WSP (SRAM write)
        if (emitter == 3 && clearb && wsp) {
            printf("  PC%02d: 0x%04X - WWE (SRAM write)\n", i, inst);
        }

        // WWF sets waveform register - look for patterns indicating SRAM or input
        if (wwf && i > 0) {
            // Check previous instruction for source of WF
            // This is a simplified heuristic
        }
    }

    printf("WXY+WSP (MIX update): %d instructions\n", wxy_wsp_count);
    printf("WACC (DAC output): %d instructions\n", wacc_count);
}

int main(int argc, char** argv) {
    SAM8905_FX sam;
    sam.reset();

    // Set 22kHz mode
    sam.control_reg = 0x08;  // SSR=1

    // Load A-RAM and D-RAM
    load_alg0_aram(sam);
    load_alg2_aram(sam);
    load_fx_dram(sam);

    printf("=== SAM8905 FX Test Harness ===\n");
    printf("22kHz mode, 64 instructions per algorithm\n");

    // Analyze algorithms
    analyze_algorithm(sam, 0x00, "ALG=0");
    analyze_algorithm(sam, 0x80, "ALG=2");

    // Test with verbose mode first
    sam.verbose = true;
    printf("\n=== Running single frame with verbose output ===\n");
    sam.input_l = 0;
    sam.input_r = 10000;  // Test input
    sam.process_frame();
    printf("Output: L=%d R=%d\n", sam.out_l, sam.out_r);

    // Now run silent tests
    sam.verbose = false;
    sam.reset();
    sam.control_reg = 0x08;
    load_alg0_aram(sam);
    load_alg2_aram(sam);
    load_fx_dram(sam);

    // Test impulse response
    test_impulse_response(sam, 500);

    // Reset and test with sine
    sam.reset();
    sam.control_reg = 0x08;
    load_alg0_aram(sam);
    load_alg2_aram(sam);
    load_fx_dram(sam);

    test_with_sine_input(sam, 100, 440);

    return 0;
}
