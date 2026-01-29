// PortAudio Audio Output for Ambika Retro
// Real-time audio output using PortAudio library

#pragma once

#include <stdint.h>

// Audio callback function type
// Called by audio thread to request samples
// Returns number of samples written to buffer
typedef int (*AudioCallback)(int16_t* buffer, int frames, void* user_data);

// Initialize PortAudio with specified sample rate
// Returns 0 on success, -1 on error
int audio_portaudio_init(uint32_t sample_rate);

// Set the audio callback function
void audio_portaudio_set_callback(AudioCallback callback, void* user_data);

// Start audio streaming
// Returns 0 on success, -1 on error
int audio_portaudio_start(void);

// Stop audio streaming
void audio_portaudio_stop(void);

// Shutdown PortAudio
void audio_portaudio_shutdown(void);

// Get current sample rate
uint32_t audio_portaudio_get_sample_rate(void);

// Check if audio is running
int audio_portaudio_is_running(void);
