#pragma once

/**
 * SAM8905 ROM Access - Emulator Implementation
 */

/**
 * Load ROM from file
 *
 * @param filename  Path to ROM file
 * @return          0 on success, -1 on error
 */
int rom_load(const char *filename);

/**
 * Check if ROM is loaded
 *
 * @return  1 if ROM is loaded, 0 otherwise
 */
int rom_is_loaded(void);

/**
 * Initialize ROM with zeros (for testing without ROM file)
 */
void rom_init_empty(void);
