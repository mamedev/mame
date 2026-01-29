#pragma once

/**
 * SAM8905 Controller Firmware
 *
 * Main header - includes all firmware components.
 *
 * DREAM S.A. reference firmware for SAM8905 DSP control.
 * Used in: Solton/Hohner MS4, Wersi Keyfox 10, Hohner XE9/XE9L
 */

#include "sam_config.h"
#include "sam_types.h"
#include "sam_hw.h"
#include "sam_rom.h"
#include "sam_utils.h"
#include "sam_math.h"

/*============================================================================
 * Global State
 *============================================================================*/

extern IDATA intmem_t g_intmem;
extern IDATA intmem_upper_t g_intmem_upper;
extern XDATA extmem_t g_extmem;

/*============================================================================
 * Initialization Functions
 *============================================================================*/

/**
 * Clear all external RAM to zero
 * Original: extmem_clear_all (CODE:D454)
 */
void sam_extmem_clear_all(void);

/**
 * Clear extended voice pages
 * Original: voice_pages_clear (CODE:B70B)
 */
void sam_voice_pages_clear(void);

/**
 * Initialize slot manager (free list, channel assignments)
 * Original: slot_manager_init (CODE:9904)
 */
void sam_slot_manager_init(void);

/**
 * Compute and fill pitch frequency tables
 * Original: pitch_table_init (CODE:9B16)
 */
void sam_pitch_table_init(void);

/**
 * Full system initialization
 * Original: init_pitch_and_voices (CODE:98AD)
 */
void sam_init_all(void);

/*============================================================================
 * Voice Management
 *============================================================================*/

/**
 * Allocate and initialize voice slots for note-on
 * Original: voice_init_slots (CODE:9A2D)
 *
 * @param channel   MIDI channel (0-3)
 * @param note      MIDI note number
 * @param velocity  Note velocity
 */
void sam_voice_init(uint8_t channel, uint8_t note, uint8_t velocity);

/**
 * Release voice (note-off)
 * Original: voice_deactivate (CODE:A69C)
 *
 * @param voice_page  Voice page number
 */
void sam_voice_release(uint8_t voice_page);

/**
 * Kill all voices on channel
 * Original: voice_kill_channel (CODE:A785)
 *
 * @param channel  MIDI channel
 */
void sam_voice_kill_channel(uint8_t channel);

/**
 * Periodic voice update (envelopes, LFOs)
 * Original: periodic_voice_update (CODE:9BA7)
 *
 * Called every ~11ms from main loop when tick counter >= 2.
 */
void sam_periodic_voice_update(void);

/*============================================================================
 * MIDI Handling
 *============================================================================*/

/**
 * UART interrupt handler (receive byte to buffer)
 * Original: isr_uart_handler (CODE:B630)
 *
 * Call from UART RX interrupt.
 */
void sam_midi_rx_isr(void);

/**
 * Process one MIDI byte from RX buffer
 * Original: serial_handler (CODE:C635)
 *
 * Call from main loop.
 */
void sam_midi_process(void);

/**
 * Queue byte for MIDI TX
 * Original: midi_tx_queue_byte
 *
 * @param byte  Byte to transmit
 */
void sam_midi_tx_queue(uint8_t byte);

/**
 * All notes off on all channels
 * Original: midi_panic_all_channels (CODE:BDBC)
 */
void sam_midi_panic(void);

/*============================================================================
 * Timer Handling
 *============================================================================*/

/**
 * Timer 1 interrupt handler
 * Original: timer1_isr (CODE:D440)
 *
 * Increments tick counters in Bank 2 R6/R7.
 * Call from Timer 1 interrupt.
 */
void sam_timer1_isr(void);

/*============================================================================
 * Main Loop
 *============================================================================*/

/**
 * Main event loop
 * Original: main_loop (CODE:DA30)
 *
 * Never returns. Processes MIDI and periodic updates.
 */
void sam_main_loop(void);

/**
 * Entry point from reset
 * Original: reset_entry (CODE:DCBC)
 */
void sam_reset(void);
