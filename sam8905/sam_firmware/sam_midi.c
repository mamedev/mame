/**
 * SAM8905 Controller Firmware - MIDI Subsystem
 *
 * Implementation of MIDI receive/transmit and message processing.
 */

#include "sam_midi.h"
#include "sam_firmware.h"
#include <stddef.h>

/*============================================================================
 * Debug Output
 *
 * Only enabled for emulator builds (SAM_HW_PLATFORM defined in Makefile).
 * SDCC builds get no-op macros.
 *============================================================================*/

#ifdef SAM_HW_PLATFORM
#include <stdio.h>
#define DEBUG_MIDI(fmt, ...) do { printf("MIDI: " fmt "\n", ##__VA_ARGS__); fflush(stdout); } while(0)
#else
#define DEBUG_MIDI(fmt, ...) do { } while(0)
#endif

/*============================================================================
 * Test Support
 *
 * Test ROM hook allows unit tests to provide ROM data without modifying
 * the const g_rom array.
 *============================================================================*/

static uint8_t *s_midi_test_rom = NULL;

/**
 * Set test ROM buffer for MIDI functions.
 */
void midi_set_test_rom(uint8_t *rom)
{
    s_midi_test_rom = rom;
}

/**
 * Read a byte from ROM (or test buffer if set).
 */
static inline uint8_t midi_rom_read(uint16_t addr)
{
    if (s_midi_test_rom != NULL) {
        return s_midi_test_rom[addr];
    }
    return g_rom[addr];
}

/*============================================================================
 * MIDI Buffer Access Helpers
 *
 * The MIDI buffers are in EXTMEM (g_extmem). We access them via offsets
 * into the struct, mirroring the firmware's DPTR-based access.
 *============================================================================*/

/* Direct access to RX buffer (255 bytes at 0x13D4) */
static inline uint8_t *midi_rx_buf(void)
{
    return g_extmem.midi_rx_buffer;
}

/* Direct access to TX buffer (255 bytes at 0x12D5) */
static inline uint8_t *midi_tx_buf(void)
{
    return g_extmem.midi_tx_buffer;
}

/*============================================================================
 * midi_init
 *
 * Clear MIDI subsystem state.
 *============================================================================*/

void midi_init(void)
{
    /* Clear buffer state */
    g_extmem.midi_rx_read_pos = 0;
    g_extmem.midi_rx_write_pos = 0;
    g_extmem.midi_rx_count = 0;
    g_extmem.midi_tx_read_pos = 0;
    g_extmem.midi_tx_write_pos = 0;
    g_extmem.midi_tx_count = 0;

    /* Clear running status */
    g_extmem.midi_running_status = 0;
    g_extmem.midi_current_byte = 0;

    /* Clear SysEx state */
    g_extmem.sysex_state = 0;

    /* Default base channel 0 */
    g_extmem.midi_base_channel = 0;

    /* Clear MIDI-related flags */
    g_intmem.flags_21 &= ~(FLAG21_TX_IDLE | FLAG21_TX_OVERFLOW | FLAG21_RX_OVERFLOW |
                           FLAG21_MIDI_DATA2_PENDING | FLAG21_OMNI_MODE);
    g_intmem.flags_21 |= FLAG21_TX_IDLE;  /* TX starts idle */
}

/*============================================================================
 * midi_rx_isr (CODE:B630)
 *
 * UART receive interrupt handler. Called when a byte is received.
 *
 * Original disassembly:
 *   B630: JNB RI,$         ; wait for RI (usually already set when ISR called)
 *   B633: CLR RI
 *   B635: MOV A,SBUF       ; read received byte
 *   B637: CJNE A,#0xFE,B640  ; check for Active Sensing
 *   B63A: MOV DPTR,#0x1D9A ; reset active sensing timeout
 *   B63D: MOV A,#0x32
 *   B63F: MOVX @DPTR,A
 *   B640: SETB C
 *   B641: SUBB A,#0xF8     ; check if >= 0xF8 (real-time message)
 *   B643: JC B647          ; not real-time, continue
 *   B645: SJMP B66E        ; real-time: discard (skip to TX handling)
 *   B647: ... (buffer the byte)
 *============================================================================*/

void midi_rx_isr(uint8_t byte)
{
    /* Handle Active Sensing - reset timeout */
    if (byte == MIDI_ACTIVE_SENSING) {
        /* Active sensing timeout counter at 0x1D9A (in _region_1ce5) */
        /* For now, we skip timeout handling - just acknowledge the byte */
        return;
    }

    /* Filter real-time messages (0xF8-0xFF) */
    if (byte >= MIDI_REALTIME_MIN) {
        return;
    }

    /* Check for buffer overflow */
    if (g_extmem.midi_rx_count >= MIDI_RX_BUFFER_SIZE) {
        g_intmem.flags_21 |= FLAG21_RX_OVERFLOW;
        return;
    }

    /* Store byte in circular buffer */
    midi_rx_buf()[g_extmem.midi_rx_write_pos] = byte;
    g_extmem.midi_rx_write_pos++;
    if (g_extmem.midi_rx_write_pos >= MIDI_RX_BUFFER_SIZE) {
        g_extmem.midi_rx_write_pos = 0;
    }
    g_extmem.midi_rx_count++;
}

/*============================================================================
 * midi_tx_queue
 *
 * Queue a byte for transmission.
 *============================================================================*/

uint8_t midi_tx_queue(uint8_t byte)
{
    if (g_extmem.midi_tx_count >= MIDI_TX_BUFFER_SIZE) {
        g_intmem.flags_21 |= FLAG21_TX_OVERFLOW;
        return 0;
    }

    midi_tx_buf()[g_extmem.midi_tx_write_pos] = byte;
    g_extmem.midi_tx_write_pos++;
    if (g_extmem.midi_tx_write_pos >= MIDI_TX_BUFFER_SIZE) {
        g_extmem.midi_tx_write_pos = 0;
    }
    g_extmem.midi_tx_count++;

    /* Clear TX idle flag */
    g_intmem.flags_21 &= ~FLAG21_TX_IDLE;

    return 1;
}

/*============================================================================
 * midi_tx_dequeue
 *
 * Get next byte from TX queue for transmission.
 *============================================================================*/

uint8_t midi_tx_dequeue(uint8_t *byte)
{
    if (g_extmem.midi_tx_count == 0) {
        g_intmem.flags_21 |= FLAG21_TX_IDLE;
        return 0;
    }

    *byte = midi_tx_buf()[g_extmem.midi_tx_read_pos];
    g_extmem.midi_tx_read_pos++;
    if (g_extmem.midi_tx_read_pos >= MIDI_TX_BUFFER_SIZE) {
        g_extmem.midi_tx_read_pos = 0;
    }
    g_extmem.midi_tx_count--;

    if (g_extmem.midi_tx_count == 0) {
        g_intmem.flags_21 |= FLAG21_TX_IDLE;
    }

    return 1;
}

/*============================================================================
 * midi_rx_available / midi_tx_available
 *============================================================================*/

uint8_t midi_rx_available(void)
{
    return g_extmem.midi_rx_count;
}

uint8_t midi_tx_available(void)
{
    return g_extmem.midi_tx_count < MIDI_TX_BUFFER_SIZE;
}

/*============================================================================
 * midi_process_byte (CODE:C635 - SERIAL_HANDLER)
 *
 * Main loop MIDI parser. Reads one byte from RX buffer and processes it.
 * Handles running status and multi-byte message assembly.
 *
 * State variables:
 *   midi_running_status (0x12CC) - last status byte
 *   midi_current_byte (0x12D4) - current data byte
 *   flags_21.bit7 (0x0F) - waiting for second data byte
 *============================================================================*/

uint8_t midi_process_byte(void)
{
    uint8_t byte;
    uint8_t status;
    uint8_t channel;
    uint8_t data1, data2;

    /* Check for RX overflow - trigger panic */
    if (g_intmem.flags_21 & FLAG21_RX_OVERFLOW) {
        g_intmem.flags_21 &= ~FLAG21_RX_OVERFLOW;
        midi_panic();
        return 1;
    }

    /* Check if buffer has data */
    if (g_extmem.midi_rx_count == 0) {
        return 0;
    }

    /* Read byte from circular buffer */
    byte = midi_rx_buf()[g_extmem.midi_rx_read_pos];
    g_extmem.midi_rx_read_pos++;
    if (g_extmem.midi_rx_read_pos >= MIDI_RX_BUFFER_SIZE) {
        g_extmem.midi_rx_read_pos = 0;
    }
    g_extmem.midi_rx_count--;

    /* Check if this is a status byte (bit 7 set) */
    if (byte & 0x80) {
        /* System messages don't update running status */
        if (byte >= MIDI_SYSTEM) {
            if (byte == MIDI_SYSEX_START) {
                g_extmem.sysex_state = 1;  /* Start SysEx parsing */
            } else if (byte == MIDI_SYSEX_END) {
                g_extmem.sysex_state = 0;  /* End SysEx */
            }
            /* Other system messages ignored */
            return 1;
        }

        /* Channel voice message - update running status */
        g_extmem.midi_running_status = byte;
        g_intmem.flags_21 &= ~FLAG21_MIDI_DATA2_PENDING;
        return 1;
    }

    /* Data byte - need running status */
    if (g_extmem.midi_running_status == 0) {
        return 1;  /* No running status, ignore */
    }

    /* Handle SysEx data */
    if (g_extmem.sysex_state > 0) {
        midi_handle_sysex(byte);
        return 1;
    }

    status = g_extmem.midi_running_status & MIDI_STATUS_MASK;
    channel = g_extmem.midi_running_status & MIDI_CHANNEL_MASK;

    /* Check channel acceptance */
    if (!midi_channel_accepted(channel)) {
        /* Skip remaining bytes of this message */
        g_intmem.flags_21 &= ~FLAG21_MIDI_DATA2_PENDING;
        return 1;
    }

    /* Map channel relative to base */
    channel = (channel - g_extmem.midi_base_channel) & 0x0F;

    /* Handle based on message type */
    switch (status) {
        case MIDI_NOTE_OFF:
        case MIDI_NOTE_ON:
        case MIDI_POLY_AFTERTOUCH:
        case MIDI_CONTROL_CHANGE:
        case MIDI_PITCH_BEND:
            /* 3-byte messages - need two data bytes */
            if (!(g_intmem.flags_21 & FLAG21_MIDI_DATA2_PENDING)) {
                /* First data byte */
                g_extmem.midi_current_byte = byte;
                g_intmem.flags_21 |= FLAG21_MIDI_DATA2_PENDING;
            } else {
                /* Second data byte - complete message */
                data1 = g_extmem.midi_current_byte;
                data2 = byte;
                g_intmem.flags_21 &= ~FLAG21_MIDI_DATA2_PENDING;

                switch (status) {
                    case MIDI_NOTE_OFF:
                        midi_handle_note(channel, data1, 0);
                        break;
                    case MIDI_NOTE_ON:
                        midi_handle_note(channel, data1, data2);
                        break;
                    case MIDI_POLY_AFTERTOUCH:
                        /* Ignored in MS4 firmware */
                        break;
                    case MIDI_CONTROL_CHANGE:
                        midi_handle_cc(channel, data1, data2);
                        break;
                    case MIDI_PITCH_BEND:
                        midi_handle_pitch_bend(channel, data1, data2);
                        break;
                }
            }
            break;

        case MIDI_PROGRAM_CHANGE:
        case MIDI_CHAN_AFTERTOUCH:
            /* 2-byte messages - single data byte */
            if (status == MIDI_PROGRAM_CHANGE) {
                midi_handle_program_change(channel, byte);
            } else {
                midi_handle_aftertouch(channel, byte);
            }
            break;
    }

    return 1;
}

/*============================================================================
 * midi_process_all
 *
 * Process all available MIDI bytes.
 *============================================================================*/

void midi_process_all(void)
{
    while (midi_process_byte()) {
        /* Keep processing until buffer empty */
    }
}

/*============================================================================
 * midi_channel_accepted
 *
 * Check if channel is within accepted range (base + 4) or OMNI mode.
 *============================================================================*/

uint8_t midi_channel_accepted(uint8_t channel)
{
    uint8_t offset;

    /* OMNI mode accepts all channels */
    if (g_intmem.flags_21 & FLAG21_OMNI_MODE) {
        return 1;
    }

    /* Calculate channel offset from base */
    offset = (channel - g_extmem.midi_base_channel) & 0x0F;

    /* Accept channels 0-3 relative to base */
    return (offset < 4);
}

/*============================================================================
 * midi_set_base_channel / midi_get_base_channel / midi_set_omni
 *============================================================================*/

void midi_set_base_channel(uint8_t channel)
{
    g_extmem.midi_base_channel = channel & 0x0F;
}

uint8_t midi_get_base_channel(void)
{
    return g_extmem.midi_base_channel;
}

void midi_set_omni(uint8_t enable)
{
    if (enable) {
        g_intmem.flags_21 |= FLAG21_OMNI_MODE;
    } else {
        g_intmem.flags_21 &= ~FLAG21_OMNI_MODE;
    }
}

/*============================================================================
 * midi_panic (CODE:BDBC)
 *
 * All Notes Off on all channels. Called on RX buffer overflow.
 *
 * Original:
 *   BDBC: MOV R0,#0x00     ; channel = 0
 *   BDBE: MOV R1,#0x07     ; loop 7 times
 *   BDC0: PUSH 0x00
 *   BDC2: LCALL 0xBAE1     ; all_notes_off(channel)
 *   BDC5: POP 0x00
 *   BDC7: INC R0           ; next channel
 *   BDC8: DJNZ R1,BDC0
 *   BDCA: ... (reset buffer pointers)
 *============================================================================*/

void midi_panic(void)
{
    uint8_t ch;

    /* All notes off on channels 0-6 (as per original firmware) */
    for (ch = 0; ch < 7; ch++) {
        midi_all_notes_off(ch);
    }

    /* Reset RX buffer */
    g_extmem.midi_rx_read_pos = 0;
    g_extmem.midi_rx_write_pos = 0;
    g_extmem.midi_rx_count = 0;

    /* Clear running status */
    g_extmem.midi_running_status = 0;
    g_intmem.flags_21 &= ~FLAG21_MIDI_DATA2_PENDING;
}

/*============================================================================
 * Message Handlers - Weak Stub Implementations
 *
 * These are weak symbols that can be overridden by platform-specific
 * implementations (e.g., in main_emu.c for the emulator).
 * Will be filled in with real voice management as it gets ported.
 *============================================================================*/

#ifndef __SDCC
#define WEAK_STUB __attribute__((weak))
#else
#define WEAK_STUB
#endif

/**
 * midi_handle_note - Simplified note on/off handler
 *
 * This is a simplified implementation that:
 * - On note-on: allocates a voice and initializes D-RAM slots
 * - On note-off: finds the voice and marks it for release
 *
 * The original firmware has much more complex handling including:
 * - Per-channel note tracking tables at XRAM 0x14D5
 * - Dual-layer support (notes above threshold use ch+4)
 * - Sustain pedal hold (XRAM 0x1E50)
 * - Highest note tracking for mono mode
 *
 * This simplified version just allocates/releases voices directly.
 */
WEAK_STUB void midi_handle_note(uint8_t channel, uint8_t note, uint8_t velocity)
{
    uint8_t page;
    uint8_t next;
    uint8_t voice_note;
    uint8_t voice_channel;

    DEBUG_MIDI("Note %s ch=%d note=%d vel=%d",
               velocity ? "ON" : "OFF", channel, note, velocity);

    if (velocity == 0) {
        /* Note Off - find and release the voice playing this note */
        page = g_intmem.active_voice_list_head;
        while (page != VOICE_LIST_END) {
            /* Read note and channel from voice page */
            /* Note stored at offset 0xF8, channel in slot ID at 0xFC */
            voice_note = voice_page_read(page, 0xF8);
            voice_channel = voice_page_read(page, VOICE_PAGE_SLOT_ID) & 0x0F;

            next = voice_list_next(page);

            if (voice_note == note && voice_channel == channel) {
                /* Found it - mark for release */
                voice_deactivate(page);
                return;
            }

            page = next;
        }
        /* Voice not found - ignore (already released?) */
        return;
    }

    /* Note On - allocate and initialize a voice */

    /* Store MIDI parameters in intmem for voice_init_slots */
    g_intmem.midi_note = note;
    g_intmem.midi_velocity = velocity;
    g_intmem.current_slot_id = channel;

    /* Get program pointer for this channel
     * For now, use a fixed program pointer at CODE:0x0040 (MS4 program table)
     * Real implementation would look up from channel_current_prog table
     */
    uint8_t program_num = g_intmem.channel_current_prog[channel & 0x03];
    uint16_t program_ptr_addr = 0x0040 + (program_num * 2);

    /* Read program pointer (big-endian in ROM) */
    uint16_t program_ptr = ((uint16_t)midi_rom_read(program_ptr_addr) << 8) |
                           midi_rom_read(program_ptr_addr + 1);

    g_intmem.program_base_dph = (uint8_t)(program_ptr >> 8);
    g_intmem.program_base_dpl = (uint8_t)(program_ptr & 0xFF);

    /* Set up SAM control flags */
    g_intmem.sam_ctrl_flags = SAM_CTRL_DRAM_WR;

    /* Allocate and initialize voice slots */
    voice_init_slots();

    /* If allocation succeeded, set up D-RAM configuration */
    if (g_intmem.voice_page_num != VOICE_LIST_END) {
        uint8_t page = g_intmem.voice_page_num;

        /* Store note in voice page for later lookup */
        voice_page_write(page, 0xF8, note);

        /* Set up D-RAM config dispatch parameters */
        /* Voice init data pointer is at program_base + 10-11 (little-endian) */
        uint16_t voice_data_ptr = midi_rom_read(program_ptr + 10) |
                                  ((uint16_t)midi_rom_read(program_ptr + 11) << 8);

        g_intmem.rom_data_ptr_lo = (uint8_t)(voice_data_ptr & 0xFF);
        g_intmem.rom_data_ptr_hi = (uint8_t)(voice_data_ptr >> 8);
        g_intmem.voice_slot_base = 0;
        g_intmem.remaining_slots = 16;  /* Process all 16 D-RAM words */
        g_intmem.dram_address_counter = swap_nibbles(page);

        /* Run D-RAM config dispatch to initialize voice parameters */
        dram_config_dispatch();
    }
}

WEAK_STUB void midi_handle_cc(uint8_t channel, uint8_t cc, uint8_t value)
{
    DEBUG_MIDI("CC ch=%d cc=%d val=%d", channel, cc, value);
    /* TODO: Dispatch CC to handlers */
    (void)channel;
    (void)cc;
    (void)value;
}

/**
 * midi_handle_program_change - Program change handler (CODE:C45B)
 *
 * Loads program data from ROM based on program number.
 *
 * Original firmware behavior:
 * 1. Look up program pointer from table at CODE:0x0040 (MS4)
 * 2. Store pointer at EXTMEM 0x14D3-0x14D4 (little-endian)
 * 3. Copy 8 bytes from program+0x16 to EXTMEM 0x11E6-0x11ED
 *
 * This allows subsequent note-on events to use the new program data.
 */
WEAK_STUB void midi_handle_program_change(uint8_t channel, uint8_t program)
{
    uint16_t program_ptr_addr;
    uint16_t program_ptr;
    uint8_t i;

    DEBUG_MIDI("Program Change ch=%d prog=%d", channel, program);

    /* Clamp program number to valid range (0-65 for MS4) */
    if (program >= 66) {
        program = 65;
    }

    /* Store program number for this channel (only 4 channels tracked) */
    if ((channel & 0x03) == channel) {
        g_intmem.channel_current_prog[channel] = program;
    }

    /* Calculate program pointer table address */
    /* MS4 uses CODE:0x0040 with big-endian 2-byte entries */
    program_ptr_addr = 0x0040 + (program * 2);

    /* Read program pointer (big-endian in ROM) */
    program_ptr = ((uint16_t)midi_rom_read(program_ptr_addr) << 8) |
                  midi_rom_read(program_ptr_addr + 1);

    /* Store pointer at EXTMEM 0x14D3-0x14D4 (little-endian) */
    g_extmem.current_program_base[0] = (uint8_t)(program_ptr & 0xFF);
    g_extmem.current_program_base[1] = (uint8_t)(program_ptr >> 8);

    /* Copy 8 bytes from program+0x16 to program_init_copy */
    for (i = 0; i < 8; i++) {
        g_extmem.program_init_copy[i] = midi_rom_read(program_ptr + 0x16 + i);
    }
}

WEAK_STUB void midi_handle_aftertouch(uint8_t channel, uint8_t pressure)
{
    /* Store (pressure - 64) at channel-indexed locations */
    /* TODO: Store at 0x11C8[ch] and 0x11C4[ch] */
    (void)channel;
    (void)pressure;
}

/**
 * midi_handle_pitch_bend - Pitch bend handler (CODE:C801 → 9AB0)
 *
 * Stores pitch bend value in the pitch_bend for real-time pitch modulation.
 *
 * Original firmware behavior:
 * 1. Convert 14-bit bend to signed value: (MSB << 7 | LSB) - 0x2000
 * 2. Scale by sensitivity from EXTMEM 0x11E5 using lookup tables at CODE:9873/9880
 * 3. Store 16-bit result at EXTMEM 0x1194 + channel*2 (little-endian)
 * 4. Repeat for channel+4 (dual-layer support)
 *
 * Simplification: We store a simplified signed 16-bit value directly.
 * The periodic voice update uses this to modulate active voice pitches.
 *
 * @param channel  MIDI channel (0-15)
 * @param lsb      Pitch bend LSB (bits 0-6 of 14-bit value)
 * @param msb      Pitch bend MSB (bits 7-13 of 14-bit value)
 */
WEAK_STUB void midi_handle_pitch_bend(uint8_t channel, uint8_t lsb, uint8_t msb)
{
    int16_t bend_value;
    uint16_t table_offset;

    /* Convert 14-bit MIDI pitch bend to signed value
     * MIDI pitch bend: 0x0000 = max down, 0x2000 = center, 0x3FFF = max up
     * Signed result: -8192 to +8191
     */
    bend_value = (int16_t)(((uint16_t)msb << 7) | lsb) - 0x2000;

    /* Store in pitch_bend at 0x1194 + channel*2 (little-endian)
     * The original firmware applies sensitivity scaling here, but we
     * simplify by storing the raw value. The periodic update or D-RAM
     * handler can apply scaling when needed.
     */
    table_offset = channel * 2;
    if (table_offset < 32) {  /* Safety check: 16 channels × 2 bytes */
        g_extmem.pitch_bend[table_offset] = (uint8_t)(bend_value >> 8);     /* High byte */
        g_extmem.pitch_bend[table_offset + 1] = (uint8_t)(bend_value & 0xFF); /* Low byte */
    }

    /* Original firmware also updates channel+4 for dual-layer support.
     * Simplification: We skip the dual-layer update.
     */
}

void midi_handle_sysex(uint8_t byte)
{
    /*
     * SysEx state machine (state at 0x12C3):
     * State 1: Check manufacturer ID == 0x31
     * State 2: Store data byte 1 at 0x1D18
     * State 3: Store data byte 2 at 0x1D19, call voice enable handler
     * State 4: Ignore remaining bytes
     */
    switch (g_extmem.sysex_state) {
        case 1:
            if (byte == MIDI_MFRID_SOLTON) {
                g_extmem.sysex_state = 2;
            } else {
                g_extmem.sysex_state = 4;  /* Wrong manufacturer, ignore */
            }
            break;

        case 2:
            /* Store mask1 - TODO: store at 0x1D18 */
            g_extmem.sysex_state = 3;
            break;

        case 3:
            /* Store mask2, call voice enable handler */
            /* TODO: Implement sysex_voice_enable */
            g_extmem.sysex_state = 4;
            break;

        case 4:
        default:
            /* Ignore remaining bytes */
            break;
    }
}

WEAK_STUB void midi_all_notes_off(uint8_t channel)
{
    /* TODO: Kill all notes on channel */
    (void)channel;
}
