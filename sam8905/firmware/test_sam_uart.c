// test_sam_uart.c - SAM8905 test firmware with UART/MIDI support
// Compile: sdcc --model-small -mmcs51 test_sam_uart.c
//
// Based on analysis of original Keyfox10 firmware UART IRQ handler.
// Receives MIDI data via serial interrupt and stores in circular buffer.

#include <8051.h>
#include <stdint.h>

// ============================================================
// Hardware Definitions
// ============================================================

// P3.5 = T1 - controls SAM chip access
__sfr __at(0xB0) P3;
#define T1_BIT  0x20

// SAM8905 SND chip registers (at 0x8000 when T1=1)
__xdata __at(0x8000) volatile uint8_t SAM_SND_ADDR;
__xdata __at(0x8001) volatile uint8_t SAM_SND_DATA_L;
__xdata __at(0x8002) volatile uint8_t SAM_SND_DATA_M;
__xdata __at(0x8003) volatile uint8_t SAM_SND_DATA_H;
__xdata __at(0x8004) volatile uint8_t SAM_SND_CTRL;

// Control register bits
#define CTRL_WR   0x01  // Write mode
#define CTRL_SEL  0x02  // 0=D-RAM, 1=A-RAM
#define CTRL_IDL  0x04  // Idle (stops processing)
#define CTRL_SSR  0x08  // Sample rate (0=44kHz, 1=22kHz)

// ============================================================
// UART Circular Buffer (matches original firmware pattern)
// ============================================================

#define RX_BUFFER_SIZE 256

// Buffer in external RAM (like original firmware)
__xdata __at(0x1B20) uint8_t rx_buffer[RX_BUFFER_SIZE];
__xdata __at(0x1A1A) volatile uint8_t rx_count;      // Bytes in buffer
__xdata __at(0x1A1B) volatile uint8_t rx_write_idx;  // Write position
__xdata __at(0x1A1C) volatile uint8_t rx_read_idx;   // Read position

// Temporary storage in internal RAM (like original: 0x7D)
__data __at(0x7D) uint8_t uart_temp;

// Flags (bit-addressable area)
__bit rx_overflow;    // Buffer overflow flag
__bit tx_ready;       // TX complete flag

// ============================================================
// UART IRQ Handler
// Based on original firmware at 0xC1E3
// ============================================================

void uart_isr(void) __interrupt(4) __using(3)
{
    // Original firmware saves T1 state - we do the same
    __bit t1_save = P3_5;
    P3_5 = 1;  // Ensure SAM access enabled during ISR

    // Handle RX interrupt
    if (RI) {
        RI = 0;
        uart_temp = SBUF;

        // Check for buffer space
        if (rx_count < 0xFF) {
            // Store byte in circular buffer
            rx_buffer[rx_write_idx] = uart_temp;
            rx_count++;
            rx_write_idx++;
            // Index wraps automatically (uint8_t)
        } else {
            rx_overflow = 1;
        }
    }

    // Handle TX interrupt (for completeness)
    if (TI) {
        TI = 0;
        tx_ready = 1;
    }

    // Restore T1 state
    P3_5 = t1_save;
}

// ============================================================
// UART Functions
// ============================================================

void uart_init(void)
{
    // Initialize buffer
    rx_count = 0;
    rx_write_idx = 0;
    rx_read_idx = 0;
    rx_overflow = 0;
    tx_ready = 1;

    // Configure Timer 1 for baud rate generation
    // 16MHz crystal, MIDI baud rate 31250
    // Mode 2 (8-bit auto-reload), SMOD=1
    // TH1 = 256 - (16000000 / (16 * 12 * 31250)) = 256 - 2.67 ≈ 253 (0xFD)
    // This gives approximately 31250 baud

    TMOD = (TMOD & 0x0F) | 0x20;  // Timer 1: Mode 2 (8-bit auto-reload)
    TH1 = 0xFD;                   // Reload value for ~31250 baud
    TL1 = 0xFD;

    // Enable double baud rate for better accuracy
    PCON |= 0x80;  // SMOD = 1

    // Configure serial port
    // Mode 1: 8-bit UART, variable baud rate
    // REN: Receive enable
    SCON = 0x50;  // Mode 1, REN=1

    // Start Timer 1
    TR1 = 1;

    // Enable serial interrupt
    ES = 1;

    // Enable global interrupts
    EA = 1;
}

// Check if data available
uint8_t uart_available(void)
{
    return rx_count;
}

// Read one byte (non-blocking, returns 0 if empty)
uint8_t uart_read(void)
{
    uint8_t data;

    if (rx_count == 0) {
        return 0;
    }

    EA = 0;  // Disable interrupts during buffer access
    data = rx_buffer[rx_read_idx];
    rx_read_idx++;
    rx_count--;
    EA = 1;  // Re-enable interrupts

    return data;
}

// Send one byte (blocking)
void uart_send(uint8_t data)
{
    while (!tx_ready);
    tx_ready = 0;
    SBUF = data;
}

// ============================================================
// SAM8905 Functions
// ============================================================

void sam_enable(void) {
    P3 |= T1_BIT;
}

void sam_write_aram(uint8_t addr, uint16_t inst) {
    SAM_SND_ADDR = addr;
    SAM_SND_DATA_L = inst & 0xFF;
    SAM_SND_DATA_M = (inst >> 8) & 0x7F;
    SAM_SND_CTRL = CTRL_SEL | CTRL_WR;
}

void sam_write_dram(uint8_t addr, uint32_t param) {
    SAM_SND_ADDR = addr;
    SAM_SND_DATA_L = param & 0xFF;
    SAM_SND_DATA_M = (param >> 8) & 0xFF;
    SAM_SND_DATA_H = (param >> 16) & 0x07;
    SAM_SND_CTRL = CTRL_WR;
}

void sam_start(void) {
    SAM_SND_CTRL = 0;
}

void sam_stop(void) {
    SAM_SND_CTRL = CTRL_IDL;
}

// Initialize all slots to idle
void idle_all_slots(void) {
    uint8_t slot, addr;

    for (slot = 0; slot < 16; slot++) {
        for (addr = 0; addr < 16; addr++) {
            sam_write_dram(slot * 16 + addr, 0);
        }
        sam_write_dram(slot * 16 + 15, 0x00800);  // IDLE bit
    }

    for (addr = 0; addr < 32; addr++) {
        sam_write_aram(addr, 0x06FF);  // NOP
    }
}

// Load sinus oscillator (from SAM8905 Programmer's Guide)
void load_sinus_oscillator(uint16_t phase_inc) {
    uint16_t amplitude = 0x400;
    uint8_t mix_l = 7, mix_r = 7;

    sam_write_dram(0x00, 0);
    sam_write_dram(0x01, (uint32_t)phase_inc << 7);
    sam_write_dram(0x02, ((uint32_t)amplitude << 7) | (mix_l << 3) | mix_r);
    sam_write_dram(0x0F, 0x00000);  // ACTIVE

    sam_write_aram(0x00, 0x016F);  // RM 0, <WA, WPHI, WSP>
    sam_write_aram(0x01, 0x08BF);  // RM 1, <WB>
    sam_write_aram(0x02, 0x11F7);  // RM 2, <WXY, WSP>
    sam_write_aram(0x03, 0x02DF);  // RADD 0, <WM>
    sam_write_aram(0x04, 0x06FF);  // RSP (NOP)
    sam_write_aram(0x05, 0x06FE);  // RSP, <WACC>
}

// ============================================================
// 7-Segment Display Driver
// Based on analysis of original firmware at 0xB08C
// Uses 3x 74HC164 shift registers (IC7 -> IC6 -> IC10)
// ============================================================

// P1 pin definitions for display
// P1.3 = CLK_DISP (shift register clock)
// P1.4 = ENABLE
// P1.5 = MODE
// P1.6 = DATA (serial data)
__sbit __at(0x93) P1_CLK_DISP;
__sbit __at(0x94) P1_ENABLE;
__sbit __at(0x95) P1_MODE;
__sbit __at(0x96) P1_DATA;

// 7-segment patterns (active HIGH, bit 7 = DP)
// Segment mapping: bit0=a, bit1=b, bit2=c, bit3=d, bit4=e, bit5=f, bit6=g, bit7=dp
__code uint8_t seg7_digits[16] = {
    0x3F,  // 0: abcdef
    0x06,  // 1: bc
    0x5B,  // 2: abdeg
    0x4F,  // 3: abcdg
    0x66,  // 4: bcfg
    0x6D,  // 5: acdfg
    0x7D,  // 6: acdefg
    0x07,  // 7: abc
    0x7F,  // 8: abcdefg
    0x6F,  // 9: abcdfg
    0x77,  // A: abcefg
    0x7C,  // b: cdefg
    0x39,  // C: adef
    0x5E,  // d: bcdeg
    0x79,  // E: adefg
    0x71,  // F: aefg
};

// Special patterns
#define SEG7_BLANK  0x00
#define SEG7_MINUS  0x40  // g only
#define SEG7_DP     0x80  // decimal point

// Display buffer (3 digits: digit0=rightmost, digit2=leftmost)
__xdata __at(0x1CDA) uint8_t disp_buffer[3];

// Shift out one byte to display, MSB first
void disp_shift_byte(uint8_t data) {
    uint8_t i;
    for (i = 0; i < 8; i++) {
        // Set DATA based on MSB
        if (data & 0x80) {
            P1_DATA = 0;  // Active low for 74HC164
        } else {
            P1_DATA = 1;
        }
        // Clock pulse
        P1_CLK_DISP = 1;
        P1_CLK_DISP = 0;
        // Shift to next bit
        data <<= 1;
    }
}

// Update the 7-segment display with buffer contents
void disp_update(void) {
    // Setup: MODE=0, ENABLE=0
    P1_MODE = 0;
    P1_ENABLE = 0;

    // Initial clock pulse
    P1_CLK_DISP = 1;
    P1_CLK_DISP = 0;

    // Set MODE=1 for shift operation
    P1_MODE = 1;

    // Shift out 3 bytes (24 bits total)
    // Order: digit2 (leftmost) -> digit1 -> digit0 (rightmost)
    disp_shift_byte(disp_buffer[2]);
    disp_shift_byte(disp_buffer[1]);
    disp_shift_byte(disp_buffer[0]);

    // Latch: ENABLE=1
    P1_ENABLE = 1;
}

// Display a 3-digit decimal number (0-999)
void disp_number(uint16_t num) {
    if (num > 999) num = 999;

    disp_buffer[2] = seg7_digits[num / 100];        // Hundreds
    disp_buffer[1] = seg7_digits[(num / 10) % 10];  // Tens
    disp_buffer[0] = seg7_digits[num % 10];         // Ones

    // Blank leading zeros
    if (num < 100) disp_buffer[2] = SEG7_BLANK;
    if (num < 10)  disp_buffer[1] = SEG7_BLANK;

    disp_update();
}

// Display a hex byte on rightmost 2 digits
void disp_hex(uint8_t val) {
    disp_buffer[2] = SEG7_BLANK;
    disp_buffer[1] = seg7_digits[(val >> 4) & 0x0F];
    disp_buffer[0] = seg7_digits[val & 0x0F];
    disp_update();
}

// ============================================================
// MIDI Processing
// ============================================================

// MIDI status bytes
#define MIDI_NOTE_OFF    0x80
#define MIDI_NOTE_ON     0x90
#define MIDI_PROGRAM     0xC0
#define MIDI_SYSEX       0xF0
#define MIDI_SYSEX_END   0xF7
#define MIDI_ACTIVE_SENS 0xFE
#define MIDI_RESET       0xFF

// Simple MIDI state machine
__data uint8_t midi_status = 0;
__data uint8_t midi_data1 = 0;
__data uint8_t midi_expect = 0;

// Convert MIDI note to phase increment
// phase_inc = 4096 * freq / 44100
// freq = 440 * 2^((note-69)/12)
// Simplified lookup table for octave 4 (notes 60-71)
__code uint16_t note_phase_inc[12] = {
    24,  // C4  (261.63 Hz) - 4096 * 261.63 / 44100
    26,  // C#4 (277.18 Hz)
    27,  // D4  (293.66 Hz)
    29,  // D#4 (311.13 Hz)
    31,  // E4  (329.63 Hz)
    33,  // F4  (349.23 Hz)
    35,  // F#4 (369.99 Hz)
    37,  // G4  (392.00 Hz)
    39,  // G#4 (415.30 Hz)
    41,  // A4  (440.00 Hz)
    44,  // A#4 (466.16 Hz)
    46,  // B4  (493.88 Hz)
};

uint16_t note_to_phase_inc(uint8_t note) {
    uint8_t octave = note / 12;
    uint8_t semitone = note % 12;
    uint16_t base = note_phase_inc[semitone];

    // Adjust for octave (octave 4 = MIDI notes 48-59)
    // Shift up/down based on octave relative to 4
    if (octave > 4) {
        base <<= (octave - 4);
    } else if (octave < 4) {
        base >>= (4 - octave);
    }

    return base;
}

void process_midi_byte(uint8_t byte) {
    // Real-time messages - ignore
    if (byte >= 0xF8) {
        return;
    }

    // Status byte?
    if (byte & 0x80) {
        midi_status = byte;
        midi_expect = 0;

        // Determine expected data bytes
        switch (byte & 0xF0) {
            case MIDI_NOTE_OFF:
            case MIDI_NOTE_ON:
                midi_expect = 2;
                break;
            case MIDI_PROGRAM:
                midi_expect = 1;
                break;
            default:
                midi_status = 0;  // Unsupported, ignore
                break;
        }
        return;
    }

    // Data byte
    if (midi_status == 0) return;

    if (midi_expect == 2) {
        midi_data1 = byte;
        midi_expect = 1;
    } else if (midi_expect == 1) {
        // Complete message
        uint8_t cmd = midi_status & 0xF0;
        uint8_t velocity = byte;

        if (cmd == MIDI_NOTE_ON && velocity > 0) {
            // Note on - set frequency and display note
            uint16_t phase_inc = note_to_phase_inc(midi_data1);
            load_sinus_oscillator(phase_inc);
            disp_number(midi_data1);  // Show MIDI note number
        } else if (cmd == MIDI_NOTE_OFF || (cmd == MIDI_NOTE_ON && velocity == 0)) {
            // Note off - silence
            sam_write_dram(0x0F, 0x00800);  // Set IDLE
            disp_number(0);  // Clear display
        }

        // Reset for running status
        if ((midi_status & 0xF0) == MIDI_NOTE_OFF ||
            (midi_status & 0xF0) == MIDI_NOTE_ON) {
            midi_expect = 2;
        } else {
            midi_expect = 1;
        }
    }
}

// ============================================================
// Main
// ============================================================

void main(void) {
    // Initialize hardware
    sam_enable();
    sam_stop();
    idle_all_slots();

    // Initialize UART for MIDI
    uart_init();

    // Initialize display - show "A4" indicator (note 69)
    disp_number(69);

    // Load default sound (A4 = 440Hz, phase_inc=41)
    load_sinus_oscillator(41);

    // Start SAM
    sam_start();

    // Main loop - process incoming MIDI
    while (1) {
        while (uart_available()) {
            uint8_t byte = uart_read();
            process_midi_byte(byte);
        }

        // Could add other processing here
    }
}
