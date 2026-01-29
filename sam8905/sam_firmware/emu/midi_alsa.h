// ALSA MIDI Input for Ambika Retro
// MIDI input using ALSA sequencer API

#pragma once

#include <stdint.h>

// MIDI message types
#define MIDI_NOTE_OFF       0x80
#define MIDI_NOTE_ON        0x90
#define MIDI_POLY_PRESSURE  0xA0
#define MIDI_CONTROL_CHANGE 0xB0
#define MIDI_PROGRAM_CHANGE 0xC0
#define MIDI_CHANNEL_PRESSURE 0xD0
#define MIDI_PITCH_BEND     0xE0
#define MIDI_SYSTEM         0xF0

// MIDI message structure
typedef struct {
    uint8_t status;
    uint8_t data1;
    uint8_t data2;
    uint8_t channel;
} MidiMessage;

// MIDI callback function type
typedef void (*MidiCallback)(MidiMessage* msg, void* user_data);

// Initialize ALSA MIDI sequencer
// client_name: Name to register with ALSA (e.g., "Ambika Retro")
// Returns 0 on success, -1 on error
int midi_alsa_init(const char* client_name);

// Set MIDI callback function
void midi_alsa_set_callback(MidiCallback callback, void* user_data);

// Poll for MIDI messages (non-blocking)
// Returns number of messages processed
int midi_alsa_poll(void);

// Check if MIDI is connected
int midi_alsa_is_connected(void);

// List available MIDI ports
void midi_alsa_list_ports(void);

// Shutdown ALSA MIDI
void midi_alsa_shutdown(void);
