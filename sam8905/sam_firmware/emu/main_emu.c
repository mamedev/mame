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
#include "sam_rom_emu.h"

/* Include firmware headers (SAM_DEVICE_MS4 defined in Makefile) */
#include "../sam_firmware.h"

/*============================================================================
 * Global State - Firmware Memory (defined in sam_init.c)
 *============================================================================*/

/* These are declared in sam_init.c, we just reference them here */
extern IDATA intmem_t g_intmem;
extern IDATA intmem_upper_t g_intmem_upper;
extern XDATA extmem_t g_extmem;

static Sam8905Emu g_sam_emu;
static volatile int g_running = 1;

/*============================================================================
 * MIDI RX Callback - Feeds firmware's midi_rx_isr()
 *============================================================================*/

static void midi_rx_callback(uint8_t byte, void *user_data)
{
    (void)user_data;

    /* Debug: print received MIDI bytes */
    printf("MIDI RX: 0x%02X\n", byte);
    fflush(stdout);

    /* Call firmware's UART ISR handler */
    midi_rx_isr(byte);
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
    printf("\nReceived signal, shutting down...\n");
    fflush(stdout);
}

static void setup_signals(void)
{
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;  /* Don't restart syscalls */
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

/*============================================================================
 * MS4 Program Loading
 *
 * Loads a program from MS4 ROM:
 * - Program pointer table at 0x0040 (big-endian)
 * - A-RAM data at program+10 (little-endian pointer to 64 bytes)
 * - Voice init data at program+13 (little-endian pointer)
 *============================================================================*/

#define MS4_PROGRAM_TABLE   0x0040
#define MS4_NUM_PROGRAMS    66

static int load_ms4_program(int program_num)
{
    uint16_t ptr_addr, prog_addr, aram_ptr;
    char name[9];
    uint8_t flags, slot_count;
    int i;

    if (!rom_is_loaded()) {
        fprintf(stderr, "No ROM loaded\n");
        return -1;
    }

    if (program_num < 0 || program_num >= MS4_NUM_PROGRAMS) {
        fprintf(stderr, "Program number must be 0-%d\n", MS4_NUM_PROGRAMS - 1);
        return -1;
    }

    /* Read program pointer (big-endian) */
    ptr_addr = MS4_PROGRAM_TABLE + (program_num * 2);
    prog_addr = (g_rom[ptr_addr] << 8) | g_rom[ptr_addr + 1];

    if (prog_addr == 0xFFFF || prog_addr >= 0x10000) {
        fprintf(stderr, "Invalid program address\n");
        return -1;
    }

    /* Read program name (8 bytes, space-padded) */
    for (i = 0; i < 8; i++) {
        name[i] = g_rom[prog_addr + i];
    }
    name[8] = '\0';

    /* Read flags (offset 9) */
    flags = g_rom[prog_addr + 9];
    slot_count = flags & 0x0F;

    /* Read A-RAM pointer (offset 10-11, little-endian) */
    aram_ptr = g_rom[prog_addr + 10] | (g_rom[prog_addr + 11] << 8);

    printf("Loading MS4 program %d: '%s'\n", program_num, name);
    printf("  Address: 0x%04X, Flags: 0x%02X, Slots: %d\n", prog_addr, flags, slot_count);
    printf("  A-RAM ptr: 0x%04X\n", aram_ptr);

    /* Load A-RAM algorithm (32 words, stored as low/high byte pairs) */
    if (aram_ptr == 0 || aram_ptr + 64 > 0x10000) {
        printf("  Warning: Invalid A-RAM pointer, using test algorithm\n");
        /* Fall back to simple sine for testing */
        sam8905_write_aram(SAM8905_ARAM_ADDR_44K(0, 0), 0x016F);
        sam8905_write_aram(SAM8905_ARAM_ADDR_44K(0, 1), 0x08BF);
        sam8905_write_aram(SAM8905_ARAM_ADDR_44K(0, 2), 0x11F7);
        sam8905_write_aram(SAM8905_ARAM_ADDR_44K(0, 3), 0x02DF);
        sam8905_write_aram(SAM8905_ARAM_ADDR_44K(0, 4), 0x06FF);
        sam8905_write_aram(SAM8905_ARAM_ADDR_44K(0, 5), 0x06FE);
        for (i = 6; i < 32; i++) {
            sam8905_write_aram(SAM8905_ARAM_ADDR_44K(0, i), 0x7FFF);
        }
    } else {
        printf("  Loading A-RAM algorithm from 0x%04X:\n    ", aram_ptr);
        for (i = 0; i < 32; i++) {
            uint8_t lo = g_rom[aram_ptr + i * 2];
            uint8_t hi = g_rom[aram_ptr + i * 2 + 1];
            uint16_t word = (hi << 8) | lo;
            sam8905_write_aram(SAM8905_ARAM_ADDR_44K(0, i), word);
            if (i < 8) printf("0x%04X ", word);
        }
        printf("...\n");
    }

    /* Initialize all D-RAM slots to idle */
    for (i = 0; i < 16; i++) {
        sam8905_write_dram(SAM8905_DRAM_ADDR(i, 15), SAM8905_SLOT_IDLE);
    }

    /* Store program info for MIDI handler */
    g_intmem.channel_current_prog[0] = program_num;
    g_intmem.channel_current_prog[1] = program_num;
    g_intmem.channel_current_prog[2] = program_num;
    g_intmem.channel_current_prog[3] = program_num;

    printf("  Program loaded, ready for MIDI\n");
    return 0;
}

static void setup_test_algorithm(void)
{
    /*
     * Simple sinus oscillator algorithm (from programmer's guide):
     * For testing without ROM.
     */
    printf("Loading test sinus oscillator algorithm...\n");

    sam8905_write_aram(SAM8905_ARAM_ADDR_44K(0, 0), 0x016F);  /* RM 0, <WA,WPHI,WSP> */
    sam8905_write_aram(SAM8905_ARAM_ADDR_44K(0, 1), 0x08BF);  /* RM 1, <WB> */
    sam8905_write_aram(SAM8905_ARAM_ADDR_44K(0, 2), 0x11F7);  /* RM 2, <WXY,WSP> */
    sam8905_write_aram(SAM8905_ARAM_ADDR_44K(0, 3), 0x02DF);  /* RADD 0, <WM> */
    sam8905_write_aram(SAM8905_ARAM_ADDR_44K(0, 4), 0x06FF);  /* RSP */
    sam8905_write_aram(SAM8905_ARAM_ADDR_44K(0, 5), 0x06FE);  /* RSP, <WACC> */

    for (int i = 6; i < 32; i++) {
        sam8905_write_aram(SAM8905_ARAM_ADDR_44K(0, i), 0x7FFF);
    }

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
    printf("  -r <file>   Load ROM file (e.g., ms4_05_r1_0.bin)\n");
    printf("  -p <num>    Load program number from ROM (0-65, requires -r)\n");
    printf("  -m <port>   Connect to MIDI port (substring match)\n");
    printf("              e.g.: -m \"Midi Through\" or -m \"USB\"\n");
    printf("  -l          List available MIDI ports and exit\n");
    printf("  -t          Test mode: inject note-on and run a few updates\n");
    printf("  -h          Show this help\n");
}

int main(int argc, char *argv[])
{
    const char *midi_port = NULL;
    const char *rom_path = NULL;
    int program_num = -1;  /* -1 = use test algorithm */
    int list_only = 0;
    int test_mode = 0;

    /* Parse command line */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
            rom_path = argv[++i];
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            program_num = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
            midi_port = argv[++i];
        } else if (strcmp(argv[i], "-l") == 0) {
            list_only = 1;
        } else if (strcmp(argv[i], "-t") == 0) {
            test_mode = 1;
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
    setup_signals();

    /* Initialize firmware memory structures */
    printf("Initializing firmware...\n");
    memset(&g_intmem, 0, sizeof(g_intmem));
    memset(&g_intmem_upper, 0, sizeof(g_intmem_upper));
    memset(&g_extmem, 0, sizeof(g_extmem));

    /* Load ROM if specified */
    if (rom_path) {
        if (rom_load(rom_path) < 0) {
            fprintf(stderr, "Failed to load ROM\n");
            return 1;
        }
    } else {
        rom_init_empty();
    }

    /* Initialize firmware MIDI subsystem */
    midi_init();
    midi_set_omni(1);  /* Accept all MIDI channels for now */

    /* Initialize SAM8905 emulator */
    printf("Initializing SAM8905 emulator...\n");
    sam8905_emu_init(&g_sam_emu);
    sam8905_emu_set_instance(&g_sam_emu);

    /* Initialize SAM8905 and firmware state */
    printf("Initializing SAM8905 and firmware...\n");
    sam_init_all();  /* Full init: SAM chip, free list, pitch tables, etc. */

    /* Load algorithm */
    if (program_num >= 0 && rom_is_loaded()) {
        if (load_ms4_program(program_num) < 0) {
            fprintf(stderr, "Failed to load program %d\n", program_num);
            return 1;
        }
    } else {
        setup_test_algorithm();
    }

    /* Skip MIDI/audio init in test mode */
    if (!test_mode) {
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
    }

    /* Initialize timer */
    timer_init();

    /* Test mode: inject MIDI directly and run a few updates */
    if (test_mode) {
        printf("\n=== TEST MODE: Injecting Note-On ===\n");
        fflush(stdout);

        /* Inject Note-On: channel 0, note 60 (C4), velocity 100 */
        printf("Injecting: Note-On ch=0 note=60 vel=100\n");
        midi_rx_isr(0x90);  /* Note-On, channel 0 */
        midi_process_byte();
        midi_rx_isr(60);    /* Note number */
        midi_process_byte();
        midi_rx_isr(100);   /* Velocity */
        midi_process_byte();

        printf("\n=== Running periodic updates ===\n");
        fflush(stdout);

        /* Run a few periodic updates */
        for (int i = 0; i < 5; i++) {
            printf("--- Periodic update %d ---\n", i + 1);
            fflush(stdout);
            periodic_voice_update();
        }

        /* Dump slot 0 state */
        printf("\n=== Slot 0 state ===\n");
        printf("DRAM[0][0]  = 0x%05X (pitch)\n", sam8905_read_dram(0x00));
        printf("DRAM[0][1]  = 0x%05X (amplitude)\n", sam8905_read_dram(0x01));
        printf("DRAM[0][15] = 0x%05X (control)\n", sam8905_read_dram(0x0F));
        printf("  idle bit (11): %d\n", (sam8905_read_dram(0x0F) >> 11) & 1);
        printf("  alg bits (10:8): %d\n", (sam8905_read_dram(0x0F) >> 8) & 7);

        /* Dump first A-RAM instructions */
        printf("\n=== A-RAM algorithm 0 ===\n");
        for (int i = 0; i < 8; i++) {
            printf("  ARAM[%d] = 0x%04X\n", i, sam8905_read_aram(i));
        }

        /* Generate some audio samples and check for non-zero output */
        printf("\n=== Generating audio samples ===\n");
        fflush(stdout);
        int32_t left, right;
        int non_zero_count = 0;
        int32_t max_sample = 0;
        for (int i = 0; i < 1000; i++) {
            sam8905_process_frame(&left, &right);
            if (left != 0 || right != 0) {
                non_zero_count++;
                if (left > max_sample) max_sample = left;
                if (right > max_sample) max_sample = right;
                if (left < -max_sample) max_sample = -left;
                if (right < -max_sample) max_sample = -right;
            }
            if (i < 10 || (i % 100 == 0)) {
                printf("  Sample %d: L=%d R=%d\n", i, left, right);
            }
        }
        printf("Non-zero samples: %d/1000, max amplitude: %d\n", non_zero_count, max_sample);

        printf("\n=== Test complete ===\n");
        return 0;
    }

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
            /* Call periodic voice update to process envelopes and write D-RAM */
            periodic_voice_update();
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
