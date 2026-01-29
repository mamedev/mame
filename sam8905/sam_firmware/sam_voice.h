#pragma once

/**
 * SAM8905 Controller Firmware - Voice Management
 *
 * Voice allocation, deallocation, and linked list management.
 * Based on MS4 firmware analysis.
 *
 * Key data structures:
 * - Free list: linked list at INTMEM 0x7E-0x8D, head at 0x53
 * - Active voice list: linked list using same addresses, head at 0x54
 * - Pending voice list: tail pointer at 0x55
 *
 * Voice pages: EXTMEM pages 0x00-0x0F (256 bytes each)
 * Each voice page has status at offset 0xFB, next pointer at page+0x7E in INTMEM
 */

#include <stdint.h>
#include "sam_types.h"

/* External declarations for inline functions */
extern XDATA extmem_t g_extmem;

/*============================================================================
 * Constants
 *
 * Note: VOICE_LIST_END and other common constants are in sam_types.h
 *============================================================================*/

/* Voice page offsets (not in sam_types.h) */
#ifndef VOICE_PAGE_ACTIVE
#define VOICE_PAGE_ACTIVE     0x42  /* Active marker (0xFF when active) */
#endif
#ifndef VOICE_PAGE_STATUS
#define VOICE_PAGE_STATUS     0xFB  /* Status flags */
#endif
#ifndef VOICE_PAGE_SLOT_ID
#define VOICE_PAGE_SLOT_ID    0xFC  /* Slot ID (nibble-swapped) */
#endif

/* Output routing offsets */
#define VOICE_PAGE_ROUTE1     0xF9  /* Output routing byte 1 */
#define VOICE_PAGE_ROUTE2     0xFA  /* Output routing byte 2 */
#define VOICE_PAGE_ROUTE3     0xFB  /* Output routing byte 3 (bits 2:0) */

/* Voice page status bits (at offset 0xFB) */
#define VOICE_STATUS_RELEASE  0x20  /* bit 5: voice in release phase */
#define VOICE_STATUS_ACTIVE   0x08  /* bit 3: voice is active/sounding */

/*============================================================================
 * Linked List Primitives
 *
 * The firmware uses INTMEM addresses 0x7E-0x8D as a linked list array.
 * Each entry at address (page + 0x7E) contains the next page number.
 * 0xFF marks end of list.
 *============================================================================*/

/**
 * Get next page in linked list
 *
 * @param page  Current page number (0-15)
 * @return      Next page number, or VOICE_LIST_END (0xFF)
 */
uint8_t voice_list_next(uint8_t page);

/**
 * Set next page in linked list
 *
 * @param page  Current page number (0-15)
 * @param next  Next page number, or VOICE_LIST_END (0xFF)
 */
void voice_list_set_next(uint8_t page, uint8_t next);

/*============================================================================
 * Free List Management
 *
 * Free list head at INTMEM 0x53 (dram_slot_free_list)
 * Available slot count at INTMEM 0x56 (dram_slot_count)
 *============================================================================*/

/**
 * Allocate voice page(s) from free list (CODE:A9CF)
 *
 * Allocates slot_count (0x4A) pages from the free list.
 * If not enough pages available, calls voice_steal_find().
 *
 * Side effects:
 * - Updates dram_slot_free_list (0x53)
 * - Updates dram_slot_count (0x56)
 * - Updates active_voice_list_head (0x54) if was empty
 * - Updates pending_voice_list (0x55)
 *
 * @return  First allocated page number, or 0xFF if failed
 */
uint8_t voice_slot_allocate(void);

/**
 * Return voice page to free list (part of CODE:9946)
 *
 * Returns a single page to the head of the free list.
 *
 * @param page  Page number to free (0-15)
 */
void voice_slot_free_to_list(uint8_t page);

/*============================================================================
 * Active Voice List Management
 *
 * Active voice list head at INTMEM 0x54 (active_voice_list_head)
 * Pending voice list tail at INTMEM 0x55 (pending_voice_list)
 *============================================================================*/

/**
 * Remove voice from active list (CODE:9946 partial)
 *
 * Unlinks a voice page from the active voice list.
 * Also updates pending_voice_list if this was the tail.
 *
 * @param page  Page number to remove (0-15)
 */
void voice_list_remove(uint8_t page);

/**
 * Add voice to active list tail
 *
 * Links a voice page to the pending_voice_list.
 *
 * @param page  Page number to add (0-15)
 */
void voice_list_add_pending(uint8_t page);

/*============================================================================
 * Voice Stealing
 *============================================================================*/

/**
 * Find a voice to steal (CODE:AA12)
 *
 * Searches active voice list for a voice that can be stolen.
 * Prefers voices in release phase or with lower priority.
 *
 * @return  Page number of voice to steal, or 0xFF if none suitable
 */
uint8_t voice_steal_find(void);

/*============================================================================
 * Voice Page Operations
 *============================================================================*/

/**
 * Clear SAM D-RAM slots for a voice (CODE:99BC)
 *
 * Writes zeros to all D-RAM words for the voice's slots.
 * Also clears voice page status at offset 0xFB.
 *
 * @param page  Voice page number (0-15)
 */
void voice_slots_clear(uint8_t page);

/**
 * Free a voice completely (CODE:9946)
 *
 * Full voice deallocation:
 * 1. Clear SAM D-RAM slots
 * 2. Remove from active voice list
 * 3. Return page to free list
 *
 * Uses voice_page_num (0x3A) as input.
 *
 * @return  Next voice in list (for iteration)
 */
uint8_t voice_free(void);

/*============================================================================
 * Voice Initialization
 *============================================================================*/

/**
 * Initialize voice slots for note-on (CODE:9A2D)
 *
 * Main note-on entry point. Allocates D-RAM slots and initializes
 * voice page with program data.
 *
 * Inputs from INTMEM Bank 1:
 * - 0x08 (R0): MIDI channel (0-15)
 * - 0x09 (R1): MIDI note number (0-127)
 * - 0x0A (R2): MIDI velocity (0-127)
 *
 * Uses:
 * - slot_count (0x4A): Number of D-RAM slots needed
 * - program_base_dpl/dph (0x36/0x35): Program data pointer
 */
void voice_init_slots(void);

/**
 * Process next D-RAM slot in voice init (CODE:AB40)
 *
 * Advances to next D-RAM word and dispatches config handler.
 * Called in a loop during voice_init_slots.
 */
void voice_init_next_slot(void);

/**
 * D-RAM config dispatch loop (CODE:AB4C)
 *
 * Reads dispatch bytes from voice data stream and calls
 * appropriate handlers based on bits 5:3.
 *
 * Terminates when bit 7 is set or handler 0x20 called.
 */
void voice_dram_config_dispatch(void);

/**
 * Copy envelope data and process table (CODE:AB73)
 *
 * Copies 7-byte envelope data to voice slot and processes
 * the envelope table pointer.
 */
void voice_init_copy_and_envelope(void);

/*============================================================================
 * Voice Release
 *============================================================================*/

/**
 * Mark voice for release (note-off) (CODE:A69C)
 *
 * Sets release flag in voice page. Voice continues sounding
 * through release phase until envelope reaches zero.
 *
 * @param page  Voice page number (0-15)
 */
void voice_deactivate(uint8_t page);

/**
 * Kill all voices on channel (CODE:A785)
 *
 * Immediately silences all voices on a MIDI channel.
 *
 * @param channel  MIDI channel (0-15)
 */
void voice_kill_channel(uint8_t channel);

/*============================================================================
 * Voice Page Access Helpers
 *
 * voice_page is an array of voice_page_t structs in extmem.
 * For raw byte access, we cast to uint8_t*.
 *============================================================================*/

/**
 * Get pointer to voice page in EXTMEM (raw byte access)
 *
 * @param page  Page number (0-15)
 * @return      Pointer to 256-byte page as raw bytes
 */
static inline uint8_t *voice_page_ptr(uint8_t page)
{
    return (uint8_t *)&g_extmem.voice_page[page];
}

/**
 * Read byte from voice page
 *
 * @param page    Page number (0-15)
 * @param offset  Offset within page (0-255)
 * @return        Byte value
 */
static inline uint8_t voice_page_read(uint8_t page, uint8_t offset)
{
    uint8_t *ptr = (uint8_t *)&g_extmem.voice_page[page];
    return ptr[offset];
}

/**
 * Write byte to voice page
 *
 * @param page    Page number (0-15)
 * @param offset  Offset within page (0-255)
 * @param value   Byte value to write
 */
static inline void voice_page_write(uint8_t page, uint8_t offset, uint8_t value)
{
    uint8_t *ptr = (uint8_t *)&g_extmem.voice_page[page];
    ptr[offset] = value;
}

/*============================================================================
 * Test Support
 *============================================================================*/

/**
 * Set test ROM buffer (for unit testing)
 *
 * When set to non-NULL, ROM access functions will read from this
 * buffer instead of g_rom. Pass NULL to revert to normal operation.
 *
 * @param rom  Pointer to test ROM buffer, or NULL
 */
void voice_set_test_rom(uint8_t *rom);
