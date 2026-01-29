/**
 * ALSA MIDI Input (Raw Byte Mode)
 *
 * Opens ALSA sequencer port and decodes events to raw MIDI bytes.
 */

#define _GNU_SOURCE

#include "midi_alsa_raw.h"
#include <stdio.h>
#include <string.h>
#include <alsa/asoundlib.h>

static snd_seq_t *seq_handle = NULL;
static int port_id = -1;
static snd_midi_event_t *midi_decoder = NULL;
static midi_byte_callback_t g_callback = NULL;
static void *g_user_data = NULL;

int midi_raw_init(const char *client_name, midi_byte_callback_t callback, void *user_data)
{
    int err;

    g_callback = callback;
    g_user_data = user_data;

    /* Open ALSA sequencer */
    err = snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_INPUT, SND_SEQ_NONBLOCK);
    if (err < 0) {
        fprintf(stderr, "ALSA MIDI: Could not open sequencer: %s\n", snd_strerror(err));
        return -1;
    }

    snd_seq_set_client_name(seq_handle, client_name);

    /* Create input port */
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

    /* Create MIDI event decoder (seq event → raw bytes) */
    err = snd_midi_event_new(256, &midi_decoder);
    if (err < 0) {
        fprintf(stderr, "ALSA MIDI: Could not create decoder: %s\n", snd_strerror(err));
        snd_seq_close(seq_handle);
        seq_handle = NULL;
        return -1;
    }

    /* Disable running status in decoder output (send full status bytes) */
    snd_midi_event_no_status(midi_decoder, 1);

    printf("ALSA MIDI initialized: %s (port %d:%d)\n",
           client_name, snd_seq_client_id(seq_handle), port_id);
    printf("Connect MIDI with: aconnect <source> %d:%d\n",
           snd_seq_client_id(seq_handle), port_id);

    return 0;
}

int midi_raw_poll(void)
{
    if (!seq_handle || !g_callback) return 0;

    int total_bytes = 0;
    snd_seq_event_t *ev;

    while (snd_seq_event_input(seq_handle, &ev) >= 0) {
        uint8_t buf[16];

        /* Decode sequencer event to raw MIDI bytes */
        long len = snd_midi_event_decode(midi_decoder, buf, sizeof(buf), ev);
        if (len > 0) {
            for (long i = 0; i < len; i++) {
                g_callback(buf[i], g_user_data);
            }
            total_bytes += (int)len;
        }

        snd_seq_free_event(ev);
    }

    return total_bytes;
}

void midi_raw_list_ports(void)
{
    if (!seq_handle) {
        printf("ALSA MIDI not initialized\n");
        return;
    }

    snd_seq_client_info_t *cinfo;
    snd_seq_port_info_t *pinfo;

    snd_seq_client_info_alloca(&cinfo);
    snd_seq_port_info_alloca(&pinfo);

    printf("\nAvailable MIDI ports:\n");

    snd_seq_client_info_set_client(cinfo, -1);
    while (snd_seq_query_next_client(seq_handle, cinfo) >= 0) {
        int client = snd_seq_client_info_get_client(cinfo);

        snd_seq_port_info_set_client(pinfo, client);
        snd_seq_port_info_set_port(pinfo, -1);

        while (snd_seq_query_next_port(seq_handle, pinfo) >= 0) {
            unsigned int caps = snd_seq_port_info_get_capability(pinfo);

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

int midi_raw_connect(const char *port_name)
{
    if (!seq_handle) {
        fprintf(stderr, "ALSA MIDI not initialized\n");
        return -1;
    }

    snd_seq_client_info_t *cinfo;
    snd_seq_port_info_t *pinfo;

    snd_seq_client_info_alloca(&cinfo);
    snd_seq_port_info_alloca(&pinfo);

    snd_seq_client_info_set_client(cinfo, -1);
    while (snd_seq_query_next_client(seq_handle, cinfo) >= 0) {
        int client = snd_seq_client_info_get_client(cinfo);

        snd_seq_port_info_set_client(pinfo, client);
        snd_seq_port_info_set_port(pinfo, -1);

        while (snd_seq_query_next_port(seq_handle, pinfo) >= 0) {
            unsigned int caps = snd_seq_port_info_get_capability(pinfo);

            /* Only consider ports that can send (READ capability) */
            if (!(caps & SND_SEQ_PORT_CAP_READ)) {
                continue;
            }

            /* Check if port name contains the search string */
            const char *client_name = snd_seq_client_info_get_name(cinfo);
            const char *pname = snd_seq_port_info_get_name(pinfo);

            if (strcasestr(client_name, port_name) || strcasestr(pname, port_name)) {
                int src_client = snd_seq_port_info_get_client(pinfo);
                int src_port = snd_seq_port_info_get_port(pinfo);

                /* Subscribe to this port */
                int err = snd_seq_connect_from(seq_handle, port_id, src_client, src_port);
                if (err < 0) {
                    fprintf(stderr, "ALSA MIDI: Could not connect to %d:%d: %s\n",
                            src_client, src_port, snd_strerror(err));
                    return -1;
                }

                printf("Connected to MIDI port: %d:%d %s - %s\n",
                       src_client, src_port, client_name, pname);
                return 0;
            }
        }
    }

    fprintf(stderr, "ALSA MIDI: No port matching '%s' found\n", port_name);
    return -1;
}

int midi_raw_get_client_id(void)
{
    if (!seq_handle) return -1;
    return snd_seq_client_id(seq_handle);
}

void midi_raw_shutdown(void)
{
    if (midi_decoder) {
        snd_midi_event_free(midi_decoder);
        midi_decoder = NULL;
    }
    if (seq_handle) {
        snd_seq_close(seq_handle);
        seq_handle = NULL;
        port_id = -1;
    }
    g_callback = NULL;
    g_user_data = NULL;
}
