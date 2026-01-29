// PortAudio Audio Output for Ambika Retro
// Real-time audio output using PortAudio library

#include "audio_portaudio.h"
#include <portaudio.h>
#include <stdio.h>
#include <string.h>

// Audio state
static PaStream* stream = NULL;
static uint32_t sample_rate = 39216;
static int is_running = 0;
static AudioCallback user_callback = NULL;
static void* user_data = NULL;

// PortAudio callback
static int pa_callback(
    const void* input,
    void* output,
    unsigned long frame_count,
    const PaStreamCallbackTimeInfo* time_info,
    PaStreamCallbackFlags status_flags,
    void* user_data_ptr)
{
    (void)input;
    (void)time_info;
    (void)status_flags;
    (void)user_data_ptr;

    int16_t* out = (int16_t*)output;

    if (user_callback) {
        user_callback(out, (int)frame_count, user_data);
    } else {
        // Silence if no callback
        memset(out, 0, frame_count * sizeof(int16_t));
    }

    return paContinue;
}

int audio_portaudio_init(uint32_t rate) {
    sample_rate = rate;

    PaError err = Pa_Initialize();
    if (err != paNoError) {
        fprintf(stderr, "PortAudio init error: %s\n", Pa_GetErrorText(err));
        return -1;
    }

    // Open default output stream
    err = Pa_OpenDefaultStream(
        &stream,
        0,              // No input channels
        1,              // 1 output channel (mono)
        paInt16,        // 16-bit signed integer samples
        sample_rate,
        256,            // Frames per buffer (low latency)
        pa_callback,
        NULL
    );

    if (err != paNoError) {
        fprintf(stderr, "PortAudio open stream error: %s\n", Pa_GetErrorText(err));
        Pa_Terminate();
        return -1;
    }

    printf("PortAudio initialized: %d Hz, mono, 16-bit\n", sample_rate);
    return 0;
}

void audio_portaudio_set_callback(AudioCallback callback, void* data) {
    user_callback = callback;
    user_data = data;
}

int audio_portaudio_start(void) {
    if (!stream) return -1;

    PaError err = Pa_StartStream(stream);
    if (err != paNoError) {
        fprintf(stderr, "PortAudio start error: %s\n", Pa_GetErrorText(err));
        return -1;
    }

    is_running = 1;
    printf("PortAudio streaming started\n");
    return 0;
}

void audio_portaudio_stop(void) {
    if (!stream || !is_running) return;

    Pa_StopStream(stream);
    is_running = 0;
    printf("PortAudio streaming stopped\n");
}

void audio_portaudio_shutdown(void) {
    if (stream) {
        if (is_running) {
            Pa_StopStream(stream);
        }
        Pa_CloseStream(stream);
        stream = NULL;
    }

    Pa_Terminate();
    is_running = 0;
    printf("PortAudio shutdown\n");
}

uint32_t audio_portaudio_get_sample_rate(void) {
    return sample_rate;
}

int audio_portaudio_is_running(void) {
    return is_running;
}
