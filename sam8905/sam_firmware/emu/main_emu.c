/**
 * SAM8905 Firmware Emulator - Main Loop
 *
 * Main loop structure matching MS4 firmware (MAINLOOP_da30):
 * 1. Poll MIDI → push bytes to RX buffer via midi_rx_isr()
 * 2. Process MIDI bytes via midi_process_byte()
 * 3. Check tick counters → call periodic updates
 * 4. Generate audio samples (handled by PortAudio callback)
 *
 * Uses ported firmware MIDI subsystem (sam_midi.c).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#include "sam8905_emu.h"
#include "audio_portaudio.h"
#include "midi_alsa_raw.h"

/* Include firmware headers (SAM_DEVICE_MS4 defined in Makefile) */
#include "../sam_firmware.h"

/*============================================================================
 * Global State - Firmware Memory
 *============================================================================*/

/* These are the global memory structures used by the firmware */
IDATA intmem_t g_intmem;
IDATA intmem_upper_t g_intmem_upper;
XDATA extmem_t g_extmem;

static Sam8905Emu g_sam_emu;
static volatile int g_running = 1;

/*============================================================================
 * MIDI RX Callback - Feeds firmware's midi_rx_isr()
 *============================================================================*/

static void midi_rx_callback(uint8_t byte, void *user_data)
{
    (void)user_data;

    /* Call firmware's UART ISR handler */
    midi_rx_isr(byte);
}

/*============================================================================
 * MIDI Message Handlers - Called by firmware's midi_process_byte()
 *
 * These override the stub implementations in sam_midi.c.
 * Eventually these will call the ported voice management code.
 *============================================================================*/

void midi_handle_note(uint8_t channel, uint8_t note, uint8_t velocity)
{
    printf("Note %s: ch=%d note=%d vel=%d\n",
           velocity > 0 ? "On " : "Off", channel, note, velocity);

    /* TODO: Call sam_voice_init() when voice management is ported */
    /* For now, just play a simple tone on slot 0 */
    if (velocity > 0) {
        /* Calculate phase increment for note */
        /* phase_inc = freq_hz * 4096 / 44100 */
        /* A4 (note 69) = 440 Hz → phase_inc ≈ 41 */
        double freq = 440.0 * pow(2.0, (note - 69) / 12.0);
        uint16_t phase_inc = (uint16_t)(freq * 4096.0 / 44100.0);

        /* Set up slot 0 */
        sam8905_write_dram(SAM8905_DRAM_ADDR(0, 1), SAM8905_DPHI(phase_inc));
        sam8905_write_dram(SAM8905_DRAM_ADDR(0, 2),
            SAM8905_AMP_MIX(velocity * 8, 7, 7));
        sam8905_write_dram(SAM8905_DRAM_ADDR(0, 15), SAM8905_SLOT_ACTIVE_44K(0));
    } else {
        /* Note off */
        sam8905_write_dram(SAM8905_DRAM_ADDR(0, 15), SAM8905_SLOT_IDLE);
    }
}

void midi_handle_cc(uint8_t channel, uint8_t cc, uint8_t value)
{
    printf("CC: ch=%d cc=%d val=%d\n", channel, cc, value);
    /* TODO: Dispatch to handlers when ported */
}

void midi_handle_program_change(uint8_t channel, uint8_t program)
{
    printf("Program Change: ch=%d prog=%d\n", channel, program);
    /* TODO: Load program from ROM when ported */
}

void midi_handle_aftertouch(uint8_t channel, uint8_t pressure)
{
    /* Ignored for now */
    (void)channel;
    (void)pressure;
}

void midi_handle_pitch_bend(uint8_t channel, uint8_t lsb, uint8_t msb)
{
    printf("Pitch Bend: ch=%d val=%d\n", channel, ((int)msb << 7) | lsb);
    /* TODO: Update pitch bend when ported */
}

void midi_all_notes_off(uint8_t channel)
{
    printf("All Notes Off: ch=%d\n", channel);
    /* TODO: Kill all voices on channel when ported */
    /* For now, just silence slot 0 */
    sam8905_write_dram(SAM8905_DRAM_ADDR(0, 15), SAM8905_SLOT_IDLE);
}

/*============================================================================
 * Audio Callback
 *============================================================================*/

static int audio_callback(int16_t *buffer, int frames, void *user_data)
{
    (void)user_data;

    for (int i = 0; i < frames; i++) {
        int32_t left, right;
        sam8905_process_frame(&left, &right);

        /* Mix to mono for now */
        buffer[i] = (int16_t)((left + right) / 2);
    }

    return frames;
}

/*============================================================================
 * Timer Simulation
 *============================================================================*/

static struct timespec last_tick_time;
static const long TICK_INTERVAL_NS = 5500000;  /* 5.5ms in nanoseconds */

static void timer_init(void)
{
    clock_gettime(CLOCK_MONOTONIC, &last_tick_time);
}

static void timer_update(void)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    long elapsed_ns = (now.tv_sec - last_tick_time.tv_sec) * 1000000000L
                    + (now.tv_nsec - last_tick_time.tv_nsec);

    /* Generate ticks at ~5.5ms intervals */
    while (elapsed_ns >= TICK_INTERVAL_NS) {
        TICK_FAST(&g_intmem)++;
        TICK_SLOW(&g_intmem)++;
        elapsed_ns -= TICK_INTERVAL_NS;
        last_tick_time.tv_nsec += TICK_INTERVAL_NS;
        if (last_tick_time.tv_nsec >= 1000000000L) {
            last_tick_time.tv_nsec -= 1000000000L;
            last_tick_time.tv_sec++;
        }
    }
}

/*============================================================================
 * Signal Handler
 *============================================================================*/

static void signal_handler(int sig)
{
    (void)sig;
    g_running = 0;
}

/*============================================================================
 * Algorithm Setup (simple sinus oscillator)
 *============================================================================*/

static void setup_sinus_algorithm(void)
{
    /*
     * Simple sinus oscillator algorithm (from programmer's guide):
     * PHI=0, DPHI=1, AMP=2
     *
     * Instructions:
     * 0: RM PHI, <WA,WPHI,WSP>   - Load phase, set WF=sinus
     * 1: RM DPHI, <WB>           - Load phase increment
     * 2: RM AMP, <WXY,WSP>       - Load amplitude, X=sin(PHI)
     * 3: RADD PHI, <WM>          - Update phase = PHI + DPHI
     * 4: RSP                     - NOP
     * 5: RSP, <WACC>             - Accumulate output
     */

    printf("Loading sinus oscillator algorithm...\n");

    sam8905_write_aram(SAM8905_ARAM_ADDR_44K(0, 0), 0x016F);  /* RM 0, <WA,WPHI,WSP> */
    sam8905_write_aram(SAM8905_ARAM_ADDR_44K(0, 1), 0x08BF);  /* RM 1, <WB> */
    sam8905_write_aram(SAM8905_ARAM_ADDR_44K(0, 2), 0x11F7);  /* RM 2, <WXY,WSP> */
    sam8905_write_aram(SAM8905_ARAM_ADDR_44K(0, 3), 0x02DF);  /* RADD 0, <WM> */
    sam8905_write_aram(SAM8905_ARAM_ADDR_44K(0, 4), 0x06FF);  /* RSP */
    sam8905_write_aram(SAM8905_ARAM_ADDR_44K(0, 5), 0x06FE);  /* RSP, <WACC> */

    /* Fill rest with NOPs */
    for (int i = 6; i < 30; i++) {
        sam8905_write_aram(SAM8905_ARAM_ADDR_44K(0, i), 0x06FF);
    }

    /* All slots start idle */
    for (int slot = 0; slot < 16; slot++) {
        sam8905_write_dram(SAM8905_DRAM_ADDR(slot, 15), SAM8905_SLOT_IDLE);
    }
}

/*============================================================================
 * Main
 *============================================================================*/

static void print_usage(const char *prog)
{
    printf("Usage: %s [options]\n", prog);
    printf("Options:\n");
    printf("  -m <port>   Connect to MIDI port (substring match)\n");
    printf("              e.g.: -m \"Midi Through\" or -m \"USB\"\n");
    printf("  -l          List available MIDI ports and exit\n");
    printf("  -h          Show this help\n");
}

int main(int argc, char *argv[])
{
    const char *midi_port = NULL;
    int list_only = 0;

    /* Parse command line */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
            midi_port = argv[++i];
        } else if (strcmp(argv[i], "-l") == 0) {
            list_only = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    printf("SAM8905 Firmware Emulator\n");
    printf("========================\n\n");

    /* Setup signal handler */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /* Initialize firmware memory structures */
    printf("Initializing firmware...\n");
    memset(&g_intmem, 0, sizeof(g_intmem));
    memset(&g_intmem_upper, 0, sizeof(g_intmem_upper));
    memset(&g_extmem, 0, sizeof(g_extmem));

    /* Initialize firmware MIDI subsystem */
    midi_init();
    midi_set_omni(1);  /* Accept all MIDI channels for now */

    /* Initialize SAM8905 emulator */
    printf("Initializing SAM8905 emulator...\n");
    sam8905_emu_init(&g_sam_emu);
    sam8905_emu_set_instance(&g_sam_emu);
    setup_sinus_algorithm();

    /* Initialize ALSA MIDI */
    printf("Initializing ALSA MIDI...\n");
    if (midi_raw_init("SAM8905 Synth", midi_rx_callback, NULL) < 0) {
        fprintf(stderr, "Failed to initialize MIDI\n");
        return 1;
    }

    /* List ports if requested */
    if (list_only) {
        midi_raw_list_ports();
        midi_raw_shutdown();
        return 0;
    }

    /* Auto-connect to specified port, or just list available ports */
    if (midi_port) {
        if (midi_raw_connect(midi_port) < 0) {
            midi_raw_list_ports();
            fprintf(stderr, "\nFailed to connect to MIDI port '%s'\n", midi_port);
            midi_raw_shutdown();
            return 1;
        }
    } else {
        midi_raw_list_ports();
        printf("\nNo MIDI port specified. Use -m <port> to auto-connect.\n");
        printf("Or connect manually with: aconnect <source> %d:0\n\n",
               midi_raw_get_client_id());
    }

    /* Initialize PortAudio */
    printf("\nInitializing audio...\n");
    if (audio_portaudio_init(44100) < 0) {
        fprintf(stderr, "Failed to initialize audio\n");
        midi_raw_shutdown();
        return 1;
    }
    audio_portaudio_set_callback(audio_callback, NULL);

    /* Start audio */
    if (audio_portaudio_start() < 0) {
        fprintf(stderr, "Failed to start audio\n");
        audio_portaudio_shutdown();
        midi_raw_shutdown();
        return 1;
    }

    /* Initialize timer */
    timer_init();

    printf("\nRunning... Press Ctrl+C to quit\n");
    printf("Connect MIDI and play notes!\n\n");

    /*
     * Main loop - matches MS4 MAINLOOP_da30:
     * 1. Poll ALSA MIDI → bytes go to firmware RX buffer via midi_rx_isr()
     * 2. Process MIDI bytes via firmware's midi_process_byte()
     * 3. Check tick counters, call periodic updates
     */
    while (g_running) {
        /* Poll MIDI input - bytes are pushed to firmware via midi_rx_callback */
        midi_raw_poll();

        /* Process one MIDI byte using ported firmware parser */
        midi_process_byte();

        /* Update timer ticks */
        timer_update();

        /* Check fast tick (Bank 2 R7 = INTMEM 0x17) */
        if (TICK_FAST(&g_intmem) >= 2) {
            /* Would call sam_periodic_voice_update() here */
            /* For now, nothing to do */
            TICK_FAST(&g_intmem) -= 2;
        }

        /* Check slow tick (Bank 2 R6 = INTMEM 0x16) */
        if (TICK_SLOW(&g_intmem) >= 2) {
            /* Would handle active sense timeout here */
            TICK_SLOW(&g_intmem) = 0;
        }

        /* Small sleep to avoid spinning CPU */
        usleep(100);
    }

    printf("\nShutting down...\n");

    audio_portaudio_stop();
    audio_portaudio_shutdown();
    midi_raw_shutdown();

    printf("Done.\n");
    return 0;
}
