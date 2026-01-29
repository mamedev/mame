/**
 * SAM8905 Controller Firmware - LFO Module Implementation
 *
 * See sam_lfo.h for API documentation.
 */

#include "sam_lfo.h"
#include "sam_firmware.h"

/*============================================================================
 * Sine Table (extracted from MS4 ROM CODE:9833)
 *
 * Full 64-byte signed sine wave:
 *   Indices  0-16: Rising from 0 to +127 (0x7F)
 *   Indices 16-32: Falling from +127 to 0
 *   Indices 32-48: Falling from 0 to -127 (0x81)
 *   Indices 48-63: Rising from -127 back toward 0
 *
 * Note: This is a SIGNED waveform. Values 0x80-0xFF are negative.
 *============================================================================*/

CODE uint8_t g_sine_table[64] = {
    /* 0x00-0x0F: Rising positive half */
    0x00, 0x0C, 0x19, 0x25, 0x31, 0x3C, 0x47, 0x51,
    0x5A, 0x62, 0x6A, 0x70, 0x75, 0x7A, 0x7D, 0x7E,
    /* 0x10-0x1F: Falling positive half */
    0x7F, 0x7E, 0x7D, 0x7A, 0x75, 0x70, 0x6A, 0x62,
    0x5A, 0x51, 0x47, 0x3C, 0x31, 0x25, 0x19, 0x0C,
    /* 0x20-0x2F: Falling negative half */
    0x00, 0xF4, 0xE7, 0xDB, 0xCF, 0xC4, 0xB9, 0xAF,
    0xA6, 0x9E, 0x96, 0x90, 0x8B, 0x86, 0x83, 0x82,
    /* 0x30-0x3F: Rising negative half */
    0x81, 0x82, 0x83, 0x86, 0x8B, 0x90, 0x96, 0x9E,
    0xA6, 0xAF, 0xB9, 0xC4, 0xCF, 0xDB, 0xE7, 0xF4
};

/*============================================================================
 * global_mod_lfo_update (CODE:A314)
 *
 * Original disassembly:
 *   A314: MOV P2,#0x11           ; Select EXTMEM page 0x11
 *   A317: MOV A,0x80             ; Read mod_lfo_rate (XRAM:1180)
 *   A319: JZ A33D                ; Return if rate == 0
 *   A31B: SWAP A                 ; A = rate >> 4 | rate << 4
 *   A31C: RL A                   ; A = (rate >> 3) | (rate << 5)
 *   A31D: MOV R7,A               ; R7 = increment high bits
 *   A31E: SWAP A                 ; Back to get low bits
 *   A31F: RL A
 *   A320: ANL A,#0xE0            ; Low = (rate << 5) & 0xE0
 *   A322: ADD A,0x81             ; phase_lo += increment_lo
 *   A324: MOV 0x81,A
 *   A326: MOV A,R7
 *   A327: ANL A,#0x1F            ; High = (rate >> 3) & 0x1F
 *   A329: ADDC A,0x82            ; phase_hi += increment_hi + carry
 *   A32B: MOV 0x82,A
 *   A32D: RR A                   ; A = phase_hi >> 1
 *   A32E: ANL A,#0x3F            ; Mask to 6 bits for table index
 *   A330: MOV DPTR,#0x9833       ; Sine table address
 *   A333: MOVC A,@A+DPTR         ; Lookup sine value
 *   A334: MOV 0x83,A             ; Store to mod_lfo_output
 *   A336: RET
 *
 * The phase increment is rate × 32 (5-bit left shift).
 * At 16-bit accumulator, full cycle = 65536 / (rate * 32) = 2048/rate ticks.
 *============================================================================*/

void global_mod_lfo_update(void)
{
    /* Early exit if LFO disabled */
    if (g_extmem.mod_lfo_rate == 0) {
        return;
    }

    /* Compute phase increment: rate × 32 (rate << 5) */
    uint16_t increment = (uint16_t)g_extmem.mod_lfo_rate << 5;

    /* Load current 16-bit phase */
    uint16_t phase = ((uint16_t)g_extmem.mod_lfo_phase_hi << 8) |
                      g_extmem.mod_lfo_phase_lo;

    /* Advance phase (wraps naturally at 16 bits) */
    phase += increment;

    /* Store back */
    g_extmem.mod_lfo_phase_lo = (uint8_t)(phase & 0xFF);
    g_extmem.mod_lfo_phase_hi = (uint8_t)(phase >> 8);

    /* Sine lookup: use top 6 bits of phase (phase_hi >> 1) & 0x3F */
    uint8_t table_index = (g_extmem.mod_lfo_phase_hi >> 1) & 0x3F;
    g_extmem.mod_lfo_output = g_sine_table[table_index];
}
