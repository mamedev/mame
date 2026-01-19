// test_sam.c - Minimal SAM8905 test firmware for Keyfox10
// Compile: sdcc --model-small -mmcs51 test_sam.c
//
// This firmware tests SAM8905 internal waveform generation to determine
// the correct interpretation of WF register bits (R, I positions and semantics).

#include <8051.h>

// P3.5 = T1 - controls SAM chip access
__sfr __at(0xB0) P3;
#define T1_BIT  0x20

// SAM8905 SND chip registers (at 0x8000 when T1=1)
__xdata __at(0x8000) volatile unsigned char SAM_SND_ADDR;
__xdata __at(0x8001) volatile unsigned char SAM_SND_DATA_L;
__xdata __at(0x8002) volatile unsigned char SAM_SND_DATA_M;
__xdata __at(0x8003) volatile unsigned char SAM_SND_DATA_H;
__xdata __at(0x8004) volatile unsigned char SAM_SND_CTRL;

// Control register bits
#define CTRL_WR   0x01  // Write mode
#define CTRL_SEL  0x02  // 0=D-RAM, 1=A-RAM
#define CTRL_IDL  0x04  // Idle (stops processing)
#define CTRL_SSR  0x08  // Sample rate (0=44kHz, 1=22kHz)

// Test case selection (WF values to try)
// INT=1 (bit 8) for internal waveform
#define WF_SINUS_PLAIN   0x100  // INT=1, all other bits 0
#define WF_SINUS_BIT6    0x140  // INT=1, bit6=1
#define WF_SINUS_BIT7    0x180  // INT=1, bit7=1
#define WF_RAMP_BIT6     0x140  // Same as above - is this RAMP or SINUS?
#define WF_RAMP_BIT7     0x180  // Same as above - is this RAMP or SINUS?
#define WF_RAMP_SEL2     0x1A0  // INT=1, bit7=1, SEL=2 (if R=bit7)
#define WF_RAMP_SEL2_ALT 0x160  // INT=1, bit6=1, SEL=2 (if R=bit6)

// Enable SAM chip access
void sam_enable(void) {
    P3 |= T1_BIT;  // Set T1=1
}

// Write 15-bit instruction to SND A-RAM
// Sequence: ADDR -> DATA -> CTRL (CTRL write triggers the RAM write)
void sam_write_aram(unsigned char addr, unsigned int inst) {
    SAM_SND_ADDR = addr;
    SAM_SND_DATA_L = inst & 0xFF;
    SAM_SND_DATA_M = (inst >> 8) & 0x7F;
    SAM_SND_CTRL = CTRL_SEL | CTRL_WR;  // Select A-RAM, write mode - triggers write
}

// Write 19-bit parameter to SND D-RAM
// Sequence: ADDR -> DATA -> CTRL (CTRL write triggers the RAM write)
void sam_write_dram(unsigned char addr, unsigned long param) {
    SAM_SND_ADDR = addr;
    SAM_SND_DATA_L = param & 0xFF;
    SAM_SND_DATA_M = (param >> 8) & 0xFF;
    SAM_SND_DATA_H = (param >> 16) & 0x07;
    SAM_SND_CTRL = CTRL_WR;  // Select D-RAM, write mode - triggers write
}

// Start SAM processing
void sam_start(void) {
    SAM_SND_CTRL = 0;  // 44kHz mode (SSR=0), IDL=0 (running)
}

// Stop SAM processing
void sam_stop(void) {
    SAM_SND_CTRL = CTRL_IDL;  // 44kHz mode (SSR=0), IDL=1 (stopped)
}

// Initialize all slots to idle state
// This prevents unused slots from contributing noise to the output
void idle_all_slots(void) {
    unsigned char slot;
    unsigned char addr;

    // For each of the 16 slots
    for (slot = 0; slot < 16; slot++) {
        // Clear all 16 D-RAM words for this slot
        for (addr = 0; addr < 16; addr++) {
            sam_write_dram(slot * 16 + addr, 0);
        }
        // Set param15 (word 15) with IDLE bit set (bit 11 = 0x800)
        sam_write_dram(slot * 16 + 15, 0x00800);
    }

    // Fill all A-RAM with NOPs (0x06FF = RSP with no receivers)
    // In 44kHz mode, algorithm 0 uses 32 instructions (A-RAM[0-31])
    for (addr = 0; addr < 32; addr++) {
        sam_write_aram(addr, 0x06FF);
    }
}

// Load sinus oscillator from SAM8905 Programmer's Guide
// Uses 6 instructions as shown in sandbox notebook
void load_test_program(unsigned int wf_value) {
    // D-RAM setup for slot 0:
    // D[0] = PHI (phase) - starts at 0
    // D[1] = DPHI (phase increment) - for 440Hz: 4096 * 440 / 44100 ≈ 41
    // D[2] = AMP with mix bits - amplitude in upper bits, mix in lower
    // D[15] = ALG=0, IDLE=0

    unsigned int phase_inc = 41;  // ~440Hz at 44.1kHz
    unsigned int amplitude = 0x400;  // Full amplitude (Q0.11 = 0.5)
    unsigned char mix_l = 7, mix_r = 7;  // Full volume (7 = no attenuation)

    // D[0] = PHI starts at 0
    sam_write_dram(0x00, 0);
    // D[1] = DPHI in upper bits (bus[18:7] -> Y)
    sam_write_dram(0x01, (unsigned long)phase_inc << 7);
    // D[2] = AMP + mix (amplitude in bits 18:7, mix_l in bits 5:3, mix_r in bits 2:0)
    sam_write_dram(0x02, ((unsigned long)amplitude << 7) | (mix_l << 3) | mix_r);
    // D[15] = ALG=0, IDLE=0 (slot 0 is ACTIVE)
    sam_write_dram(0x0F, 0x00000);

    // A-RAM: Sinus Oscillator from Programmer's Guide
    // PHI=0, DPHI=1, AMP=2
    //
    // RM   PHI,  <WA,WPHI,WSP>   ; A=PHI, PHI reg=D[0], WF=0x100 (sinus)
    // RM   DPHI, <WB>            ; B=D[1] (phase increment)
    // RM   AMP,  <WXY,WSP>       ; X=sin(PHI), Y=AMP, mix updated
    // RADD PHI,  <WM>            ; D[0]=A+B (PHI+DPHI)
    // RSP                        ; NOP (wait for multiplier)
    // RSP       ,<WACC>          ; accumulate AMP x sin(PHI)

    // Instructions from working sandbox (verified to produce audio):
    sam_write_aram(0x00, 0x016F);  // RM 0, <WA, WPHI, WSP>
    sam_write_aram(0x01, 0x08BF);  // RM 1, <WB>
    sam_write_aram(0x02, 0x11F7);  // RM 2, <WXY, WSP>
    sam_write_aram(0x03, 0x02DF);  // RADD 0, <WM>
    sam_write_aram(0x04, 0x06FF);  // RSP (NOP)
    sam_write_aram(0x05, 0x06FE);  // RSP, <WACC>
}

// Delay loop
void delay(unsigned int count) {
    while (count--) {
        // Empty loop
    }
}

void main(void) {
    unsigned char test_case = 0;

    // Enable SAM chip access
    sam_enable();

    // Stop SAM first
    sam_stop();

    // Initialize all slots to idle state first
    // This prevents unused slots from contributing noise
    idle_all_slots();

    // Load test program into slot 0 (clears IDLE for slot 0)
    load_test_program(WF_SINUS_PLAIN);

    // Start SAM
    sam_start();

    // Main loop - could add button input to cycle through test cases
    while(1) {
        // For now, just run continuously
        // Hardware testing: record audio for each WF value

        delay(50000);  // Let it run

        // Cycle through test cases (if button input was available)
        // test_case = (test_case + 1) % 4;
        // switch(test_case) {
        //     case 0: load_test_program(WF_SINUS_PLAIN); break;
        //     case 1: load_test_program(WF_SINUS_BIT6); break;
        //     case 2: load_test_program(WF_SINUS_BIT7); break;
        //     case 3: load_test_program(WF_RAMP_SEL2); break;
        // }
    }
}
