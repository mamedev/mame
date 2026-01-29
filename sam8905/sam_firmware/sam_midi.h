#pragma once

/**
 * SAM8905 Controller Firmware - MIDI Subsystem
 *
 * MIDI receive/transmit buffer management and message parsing.
 * Based on MS4 firmware analysis (ISR_UART_HANDLER at B630, SERIAL_HANDLER at C635).
 */

#include <stdint.h>
#include "sam_types.h"

/*============================================================================
 * MIDI Constants
 *============================================================================*/

/* Buffer sizes */
#define MIDI_RX_BUFFER_SIZE  255
#define MIDI_TX_BUFFER_SIZE  255

/* Real-time messages (filtered by ISR) */
#define MIDI_REALTIME_MIN    0xF8
#define MIDI_ACTIVE_SENSING  0xFE

/* Active sensing timeout reload value */
#define MIDI_ACTIVE_TIMEOUT  0x32

/* Status byte masks */
#define MIDI_STATUS_MASK     0xF0
#define MIDI_CHANNEL_MASK    0x0F

/* Message types */
#define MIDI_NOTE_OFF        0x80
#define MIDI_NOTE_ON         0x90
#define MIDI_POLY_AFTERTOUCH 0xA0
#define MIDI_CONTROL_CHANGE  0xB0
#define MIDI_PROGRAM_CHANGE  0xC0
#define MIDI_CHAN_AFTERTOUCH 0xD0
#define MIDI_PITCH_BEND      0xE0
#define MIDI_SYSTEM          0xF0

/* System messages */
#define MIDI_SYSEX_START     0xF0
#define MIDI_SYSEX_END       0xF7

/* SysEx manufacturer ID */
#define MIDI_MFRID_SOLTON    0x31

/*============================================================================
 * MIDI Buffer Functions
 *============================================================================*/

/**
 * Initialize MIDI subsystem (clear buffers)
 */
void midi_init(void);

/**
 * UART receive interrupt handler (CODE:B630)
 *
 * Call from hardware UART RX interrupt. Stores received byte in circular
 * buffer, handles Active Sensing timeout reset, filters real-time messages.
 *
 * @param byte  Received MIDI byte from UART SBUF
 */
void midi_rx_isr(uint8_t byte);

/**
 * Queue byte for transmission (CODE:midi_tx_queue_byte)
 *
 * @param byte  Byte to transmit
 * @return      1 if queued successfully, 0 if buffer full
 */
uint8_t midi_tx_queue(uint8_t byte);

/**
 * Get next byte from TX queue for transmission
 *
 * Call from UART TX interrupt or polling loop.
 *
 * @param byte  Pointer to receive byte
 * @return      1 if byte available, 0 if buffer empty
 */
uint8_t midi_tx_dequeue(uint8_t *byte);

/**
 * Check if RX buffer has data
 *
 * @return  Number of bytes in RX buffer
 */
uint8_t midi_rx_available(void);

/**
 * Check if TX buffer has space
 *
 * @return  1 if TX buffer has space, 0 if full
 */
uint8_t midi_tx_available(void);

/*============================================================================
 * MIDI Message Processing (CODE:C635)
 *============================================================================*/

/**
 * Process one MIDI byte from RX buffer
 *
 * Call from main loop. Handles running status, multi-byte messages,
 * and dispatches to appropriate handlers.
 *
 * @return  1 if a byte was processed, 0 if buffer empty
 */
uint8_t midi_process_byte(void);

/**
 * Process all available MIDI bytes
 *
 * Calls midi_process_byte() until RX buffer is empty.
 */
void midi_process_all(void);

/*============================================================================
 * MIDI Message Handlers (internal, called by midi_process_byte)
 *============================================================================*/

/**
 * Handle Note On/Off (CODE:B75F)
 *
 * @param channel   MIDI channel (0-15)
 * @param note      MIDI note number (0-127)
 * @param velocity  Note velocity (0 = note off)
 */
void midi_handle_note(uint8_t channel, uint8_t note, uint8_t velocity);

/**
 * Handle Control Change (CODE:C1A0)
 *
 * @param channel  MIDI channel (0-15)
 * @param cc       Controller number (0-127)
 * @param value    Controller value (0-127)
 */
void midi_handle_cc(uint8_t channel, uint8_t cc, uint8_t value);

/**
 * Handle Program Change (CODE:C45B)
 *
 * @param channel  MIDI channel (0-15)
 * @param program  Program number (0-127)
 */
void midi_handle_program_change(uint8_t channel, uint8_t program);

/**
 * Handle Channel Aftertouch
 *
 * @param channel  MIDI channel (0-15)
 * @param pressure Pressure value (0-127)
 */
void midi_handle_aftertouch(uint8_t channel, uint8_t pressure);

/**
 * Handle Pitch Bend (CODE:9AB0)
 *
 * @param channel  MIDI channel (0-15)
 * @param lsb      Pitch bend LSB (0-127)
 * @param msb      Pitch bend MSB (0-127)
 */
void midi_handle_pitch_bend(uint8_t channel, uint8_t lsb, uint8_t msb);

/**
 * Handle SysEx byte (state machine)
 *
 * @param byte  SysEx data byte
 */
void midi_handle_sysex(uint8_t byte);

/*============================================================================
 * MIDI Panic / All Notes Off (CODE:BDBC)
 *============================================================================*/

/**
 * All Notes Off on all channels
 *
 * Called on RX buffer overflow or All Notes Off CC.
 */
void midi_panic(void);

/**
 * All Notes Off on specific channel (CODE:BAE1)
 *
 * @param channel  MIDI channel (0-15)
 */
void midi_all_notes_off(uint8_t channel);

/*============================================================================
 * MIDI Configuration
 *============================================================================*/

/**
 * Set MIDI base channel
 *
 * @param channel  Base channel (0-15)
 */
void midi_set_base_channel(uint8_t channel);

/**
 * Get MIDI base channel
 *
 * @return  Current base channel (0-15)
 */
uint8_t midi_get_base_channel(void);

/**
 * Set OMNI mode
 *
 * @param enable  1 to enable OMNI (receive all channels), 0 to disable
 */
void midi_set_omni(uint8_t enable);

/**
 * Check if channel is accepted (within base + 4 range, or OMNI)
 *
 * @param channel  MIDI channel to check
 * @return         1 if accepted, 0 if not
 */
uint8_t midi_channel_accepted(uint8_t channel);
