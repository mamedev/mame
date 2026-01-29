// ALSA MIDI Input for Ambika Retro
// MIDI input using ALSA sequencer API

// Define _GNU_SOURCE before any includes to avoid struct timespec redefinition
#define _GNU_SOURCE

#include "midi_alsa.h"
#include <stdio.h>
#include <string.h>
#include <alloca.h>
#include <alsa/asoundlib.h>

// ALSA sequencer state
static snd_seq_t* seq_handle = NULL;
static int port_id = -1;
static MidiCallback user_callback = NULL;
static void* user_data = NULL;

int midi_alsa_init(const char* client_name) {
    int err;

    // Open ALSA sequencer
    err = snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_INPUT, SND_SEQ_NONBLOCK);
    if (err < 0) {
        fprintf(stderr, "ALSA MIDI: Could not open sequencer: %s\n", snd_strerror(err));
        return -1;
    }

    // Set client name
    snd_seq_set_client_name(seq_handle, client_name);

    // Create input port
    port_id = snd_seq_create_simple_port(
        seq_handle,
        "MIDI In",
        SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
        SND_SEQ_PORT_TYPE_APPLICATION
    );

    if (port_id < 0) {
        fprintf(stderr, "ALSA MIDI: Could not create port: %s\n", snd_strerror(port_id));
        snd_seq_close(seq_handle);
        seq_handle = NULL;
        return -1;
    }

    printf("ALSA MIDI initialized: %s (port %d:%d)\n",
           client_name, snd_seq_client_id(seq_handle), port_id);
    printf("Connect a MIDI device with: aconnect <source> %d:%d\n",
           snd_seq_client_id(seq_handle), port_id);

    return 0;
}

void midi_alsa_set_callback(MidiCallback callback, void* data) {
    user_callback = callback;
    user_data = data;
}

int midi_alsa_poll(void) {
    if (!seq_handle) return 0;

    int count = 0;
    snd_seq_event_t* ev;

    while (snd_seq_event_input(seq_handle, &ev) >= 0) {
        MidiMessage msg = {0};

        switch (ev->type) {
            case SND_SEQ_EVENT_NOTEON:
                msg.status = MIDI_NOTE_ON;
                msg.channel = ev->data.note.channel;
                msg.data1 = ev->data.note.note;
                msg.data2 = ev->data.note.velocity;
                // Note on with velocity 0 is note off
                if (msg.data2 == 0) {
                    msg.status = MIDI_NOTE_OFF;
                }
                break;

            case SND_SEQ_EVENT_NOTEOFF:
                msg.status = MIDI_NOTE_OFF;
                msg.channel = ev->data.note.channel;
                msg.data1 = ev->data.note.note;
                msg.data2 = ev->data.note.velocity;
                break;

            case SND_SEQ_EVENT_KEYPRESS:
                msg.status = MIDI_POLY_PRESSURE;
                msg.channel = ev->data.note.channel;
                msg.data1 = ev->data.note.note;
                msg.data2 = ev->data.note.velocity;
                break;

            case SND_SEQ_EVENT_CONTROLLER:
                msg.status = MIDI_CONTROL_CHANGE;
                msg.channel = ev->data.control.channel;
                msg.data1 = ev->data.control.param;
                msg.data2 = ev->data.control.value;
                break;

            case SND_SEQ_EVENT_PGMCHANGE:
                msg.status = MIDI_PROGRAM_CHANGE;
                msg.channel = ev->data.control.channel;
                msg.data1 = ev->data.control.value;
                msg.data2 = 0;
                break;

            case SND_SEQ_EVENT_CHANPRESS:
                msg.status = MIDI_CHANNEL_PRESSURE;
                msg.channel = ev->data.control.channel;
                msg.data1 = ev->data.control.value;
                msg.data2 = 0;
                break;

            case SND_SEQ_EVENT_PITCHBEND:
                msg.status = MIDI_PITCH_BEND;
                msg.channel = ev->data.control.channel;
                // ALSA pitch bend is -8192 to +8191, convert to 0-16383
                {
                    int pb = ev->data.control.value + 8192;
                    msg.data1 = pb & 0x7F;
                    msg.data2 = (pb >> 7) & 0x7F;
                }
                break;

            default:
                // Skip other events
                snd_seq_free_event(ev);
                continue;
        }

        // Call user callback
        if (user_callback) {
            user_callback(&msg, user_data);
        }

        snd_seq_free_event(ev);
        count++;
    }

    return count;
}

int midi_alsa_is_connected(void) {
    return seq_handle != NULL;
}

void midi_alsa_list_ports(void) {
    if (!seq_handle) {
        printf("ALSA MIDI not initialized\n");
        return;
    }

    snd_seq_client_info_t* cinfo;
    snd_seq_port_info_t* pinfo;

    snd_seq_client_info_alloca(&cinfo);
    snd_seq_port_info_alloca(&pinfo);

    printf("\nAvailable MIDI input ports:\n");

    snd_seq_client_info_set_client(cinfo, -1);
    while (snd_seq_query_next_client(seq_handle, cinfo) >= 0) {
        int client = snd_seq_client_info_get_client(cinfo);

        snd_seq_port_info_set_client(pinfo, client);
        snd_seq_port_info_set_port(pinfo, -1);

        while (snd_seq_query_next_port(seq_handle, pinfo) >= 0) {
            unsigned int caps = snd_seq_port_info_get_capability(pinfo);

            // Only show output ports (we can read from them)
            if (caps & SND_SEQ_PORT_CAP_READ) {
                printf("  %3d:%-3d  %s - %s\n",
                       snd_seq_port_info_get_client(pinfo),
                       snd_seq_port_info_get_port(pinfo),
                       snd_seq_client_info_get_name(cinfo),
                       snd_seq_port_info_get_name(pinfo));
            }
        }
    }
}

void midi_alsa_shutdown(void) {
    if (seq_handle) {
        snd_seq_close(seq_handle);
        seq_handle = NULL;
        port_id = -1;
    }
    printf("ALSA MIDI shutdown\n");
}
