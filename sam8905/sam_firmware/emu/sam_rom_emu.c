/**
 * SAM8905 ROM Access - Emulator Implementation
 *
 * Provides g_rom array for the emulator, loaded from ROM file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../sam_types.h"

/*============================================================================
 * ROM Data Storage
 *============================================================================*/

/* 64KB ROM for MS4 (could be larger for other devices) */
#define ROM_SIZE 0x10000

/* Global ROM array - accessed via sam_rom.h macros */
uint8_t g_rom[ROM_SIZE];

/* Flag to track if ROM is loaded */
static int g_rom_loaded = 0;

/*============================================================================
 * ROM Loading
 *============================================================================*/

/**
 * Load ROM from file
 *
 * @param filename  Path to ROM file
 * @return          0 on success, -1 on error
 */
int rom_load(const char *filename)
{
    FILE *f;
    size_t bytes_read;

    f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "ROM: Could not open '%s'\n", filename);
        return -1;
    }

    /* Clear ROM first */
    memset(g_rom, 0xFF, ROM_SIZE);

    /* Read ROM file */
    bytes_read = fread(g_rom, 1, ROM_SIZE, f);
    fclose(f);

    if (bytes_read == 0) {
        fprintf(stderr, "ROM: Could not read '%s'\n", filename);
        return -1;
    }

    printf("ROM: Loaded %zu bytes from '%s'\n", bytes_read, filename);
    g_rom_loaded = 1;

    return 0;
}

/**
 * Check if ROM is loaded
 *
 * @return  1 if ROM is loaded, 0 otherwise
 */
int rom_is_loaded(void)
{
    return g_rom_loaded;
}

/**
 * Initialize ROM with zeros (for testing without ROM file)
 */
void rom_init_empty(void)
{
    memset(g_rom, 0x00, ROM_SIZE);
    g_rom_loaded = 0;
    printf("ROM: Initialized empty (no ROM file)\n");
}
