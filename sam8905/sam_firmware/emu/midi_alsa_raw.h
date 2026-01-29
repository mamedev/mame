#pragma once

/**
 * ALSA MIDI Input (Raw Byte Mode)
 *
 * Opens ALSA sequencer port and decodes events to raw MIDI bytes.
 * Uses a callback to deliver bytes - suitable for feeding firmware UART buffer.
 */

#include <stdint.h>

/* Callback type for receiving raw MIDI bytes */
typedef void (*midi_byte_callback_t)(uint8_t byte, void *user_data);

/**
 * Initialize ALSA MIDI in raw byte mode.
 *
 * @param client_name  Name visible in aconnect
 * @param callback     Function called for each MIDI byte received
 * @param user_data    Passed to callback
 * @return 0 on success, -1 on error
 */
int midi_raw_init(const char *client_name, midi_byte_callback_t callback, void *user_data);

/**
 * Poll for incoming MIDI events (non-blocking).
 * Decodes sequencer events to raw bytes and calls callback.
 *
 * @return Number of bytes delivered
 */
int midi_raw_poll(void);

/**
 * List available MIDI ports
 */
void midi_raw_list_ports(void);

/**
 * Connect to a MIDI port by name (substring match).
 *
 * @param port_name  Name to search for (e.g., "Midi Through", "USB MIDI")
 * @return 0 on success, -1 if not found
 */
int midi_raw_connect(const char *port_name);

/**
 * Get our ALSA client ID (for manual aconnect)
 *
 * @return Client ID, or -1 if not initialized
 */
int midi_raw_get_client_id(void);

/**
 * Shutdown ALSA MIDI
 */
void midi_raw_shutdown(void);
