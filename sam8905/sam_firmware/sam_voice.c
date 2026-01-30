/**
 * SAM8905 Controller Firmware - Voice Management
 *
 * Implementation of voice allocation, deallocation, and linked list management.
 */

#include "sam_voice.h"
#include "sam_firmware.h"
#include "sam_dram_config.h"
#include <stddef.h>  /* For NULL */

/*============================================================================
 * Linked List Primitives
 *
 * The linked list uses INTMEM addresses 0x7E-0x8D.
 * For pages 0-1: g_intmem.dram_free_list_0/1 (0x7E-0x7F)
 * For pages 2-15: g_intmem_upper.dram_free_list[page-2] (0x80-0x8D)
 *============================================================================*/

uint8_t voice_list_next(uint8_t page)
{
    if (page == 0) {
        return g_intmem.dram_free_list_0;
    } else if (page == 1) {
        return g_intmem.dram_free_list_1;
    } else if (page < 16) {
        return g_intmem_upper.dram_free_list[page - 2];
    }
    return VOICE_LIST_END;
}

void voice_list_set_next(uint8_t page, uint8_t next)
{
    if (page == 0) {
        g_intmem.dram_free_list_0 = next;
    } else if (page == 1) {
        g_intmem.dram_free_list_1 = next;
    } else if (page < 16) {
        g_intmem_upper.dram_free_list[page - 2] = next;
    }
}

/*============================================================================
 * voice_slot_free_to_list
 *
 * Return a single page to the head of the free list.
 *
 * Original pattern (from voice_free at 9946):
 *   *(byte *)(voice_page_num + 0x7e) = dram_slot_free_list;
 *   dram_slot_free_list = voice_page_num;
 *   dram_slot_count = dram_slot_count + 1;
 *============================================================================*/

void voice_slot_free_to_list(uint8_t page)
{
    /* Link new head to old head */
    voice_list_set_next(page, g_intmem.dram_slot_free_list);

    /* Update head to new page */
    g_intmem.dram_slot_free_list = page;

    /* Increment available count */
    g_intmem.dram_slot_count++;
}

/*============================================================================
 * voice_list_remove (CODE:9946 partial)
 *
 * Remove a voice page from the active voice list.
 *
 * Original disassembly (9960-999B):
 *   voice_list_prev = active_voice_list_head;
 *   if (active_voice_list_head == voice_page_num) {
 *       active_voice_list_head = *(byte *)(voice_page_num + 0x7e);
 *   } else {
 *       do {
 *           voice_list_prev = bVar1;
 *           if (voice_list_prev == 0xff) break;
 *           bVar1 = *(byte *)(voice_list_prev + 0x7e);
 *       } while (*(byte *)(voice_list_prev + 0x7e) != voice_page_num);
 *   }
 *   // Then unlink and update pending_voice_list...
 *============================================================================*/

void voice_list_remove(uint8_t page)
{
    uint8_t prev;
    uint8_t curr;

    /* Special case: removing head of list */
    if (g_intmem.active_voice_list_head == page) {
        g_intmem.active_voice_list_head = voice_list_next(page);
    } else {
        /* Find predecessor in list */
        prev = g_intmem.active_voice_list_head;
        while (prev != VOICE_LIST_END) {
            curr = voice_list_next(prev);
            if (curr == page) {
                break;
            }
            prev = curr;
        }

        /* Unlink: prev->next = page->next */
        if (prev != VOICE_LIST_END) {
            voice_list_set_next(prev, voice_list_next(page));
        }

        /* Update voice_list_prev for other functions that need it */
        g_intmem.voice_list_prev = prev;
    }

    /* Handle pending_voice_list update */
    if (page == g_intmem.pending_voice_list) {
        if (g_intmem.active_voice_list_head == VOICE_LIST_END) {
            g_intmem.pending_voice_list = VOICE_LIST_END;
        } else {
            /* pending_voice_list becomes voice_list_prev */
            g_intmem.pending_voice_list = g_intmem.voice_list_prev;
            voice_list_set_next(g_intmem.voice_list_prev, VOICE_LIST_END);
        }
    }
}

/*============================================================================
 * voice_list_add_pending
 *
 * Add a voice page to the tail of the active list (pending_voice_list).
 *
 * This is used during slot_allocate to chain newly allocated pages.
 *============================================================================*/

void voice_list_add_pending(uint8_t page)
{
    if (g_intmem.pending_voice_list != VOICE_LIST_END) {
        /* Link current tail to new page */
        voice_list_set_next(g_intmem.pending_voice_list, page);
    }

    /* New page becomes tail */
    g_intmem.pending_voice_list = page;
    voice_list_set_next(page, VOICE_LIST_END);
}

/*============================================================================
 * voice_slots_clear (CODE:99BC)
 *
 * Clear SAM D-RAM slots for a voice page.
 *
 * Original disassembly:
 *   99bc: E53A        MOV A,0x3a           ; voice_page_num
 *   99be: FE          MOV R6,A
 *   99bf: 12A523      LCALL 0xa523         ; sam_dram_set_active (read-modify-write)
 *   99c2: 78FB        MOV R0,#0xfb
 *   99c4: E4          CLR A
 *   99c5: F2          MOVX @R0,A           ; clear page[0xFB] (status)
 *   ...
 *   99fa: E53A        MOV A,0x3a
 *   99fc: C4          SWAP A               ; nibble swap for D-RAM address
 *   99fd: FD          MOV R5,A
 *   99fe: 908000      MOV DPTR,#0x8000     ; SAM base
 *   9a01: F0          MOVX @DPTR,A         ; write address
 *   9a02: 0582        INC DPL
 *   9a04: E4          CLR A
 *   9a05: F0          MOVX @DPTR,A         ; data1 = 0
 *   9a06: 0582        INC DPL
 *   9a08: F0          MOVX @DPTR,A         ; data2 = 0
 *   9a09: 0582        INC DPL
 *   9a0b: F0          MOVX @DPTR,A         ; data3 = 0
 *   9a0c: 0582        INC DPL
 *   9a0e: 7E0F        MOV R6,#0xf          ; 15 more slots
 *   9a10: E537        MOV A,0x37           ; sam_ctrl_flags
 *   9a12: F0          MOVX @DPTR,A         ; write control (trigger)
 *   9a13: 0D          INC R5               ; next address
 *   9a14: 758200      MOV DPL,#0x0
 *   9a17: ED          MOV A,R5
 *   9a18: F0          MOVX @DPTR,A         ; write address
 *   9a19: 758204      MOV DPL,#0x4
 *   9a1c: DEF2        DJNZ R6,0x9a10       ; loop 15 times
 *============================================================================*/

void voice_slots_clear(uint8_t page)
{
    uint8_t addr;
    uint8_t i;

    /* Set control word for this slot (word 15) */
    /* Original firmware calls sam_dram_set_active (CODE:A523) which does read-modify-write */
    sam_init_slots();  /* This writes word 15 for all slots - TODO: may need per-slot version */

    /* Clear voice page status */
    voice_page_write(page, VOICE_PAGE_STATUS, 0x00);

    /* Calculate D-RAM base address: nibble-swapped page number */
    /* Page 0 → addr 0x00, Page 1 → addr 0x10, etc. */
    addr = swap_nibbles(page);

    /* Clear all 16 D-RAM words for this slot */
    /* First word: write address and zero data */
    sam_write_reg(SAM_REG_ADDR_DATA, addr);
    sam_write_reg(SAM_REG_DATA1, 0x00);
    sam_write_reg(SAM_REG_DATA2, 0x00);
    sam_write_reg(SAM_REG_DATA3, 0x00);
    sam_write_reg(SAM_REG_CTRL, g_intmem.sam_ctrl_flags);

    /* Remaining 15 words */
    for (i = 1; i < 16; i++) {
        addr++;
        sam_write_reg(SAM_REG_ADDR_DATA, addr);
        /* Data registers still 0 from above */
        sam_write_reg(SAM_REG_CTRL, g_intmem.sam_ctrl_flags);
    }
}

/*============================================================================
 * voice_free (CODE:9946)
 *
 * Full voice deallocation: clear D-RAM, remove from list, return to free list.
 *
 * Original:
 *   if ((voice_page_num & 0xf0) != 0) {
 *       return START_i_dcbc();  // reset on error
 *   }
 *   voice_slots_clear();
 *   // ... remove from active list ...
 *   // ... return to free list ...
 *   return next_voice;
 *============================================================================*/

uint8_t voice_free(void)
{
    uint8_t page = g_intmem.voice_page_num;
    uint8_t next;

    /* Sanity check: page must be 0-15 */
    if ((page & 0xF0) != 0) {
        /* Invalid page - this would trigger reset in original firmware */
        /* We just return end-of-list to avoid crash */
        return VOICE_LIST_END;
    }

    /* Get next voice before we modify the list */
    next = voice_list_next(page);

    /* Clear SAM D-RAM slots */
    voice_slots_clear(page);

    /* Remove from active voice list */
    voice_list_remove(page);

    /* Return page to free list */
    voice_slot_free_to_list(page);

    return next;
}

/*============================================================================
 * voice_steal_find (CODE:AA12)
 *
 * Find a voice suitable for stealing when no free slots available.
 *
 * Original searches active list for:
 * 1. Voices in release phase (bit 5 of page[0xFB] set)
 * 2. Voices with specific status conditions
 *
 * Original:
 *   voice_page_num = active_voice_list_head;
 *   while ((P2 = voice_page_num,
 *          (DAT_EXTMEM_00fb >> 3 & 1) == 0 ||  // bit 3 not set
 *          ((DAT_EXTMEM_00fb >> 5 & 1) == 1))  // OR bit 5 set (release)
 *         ) {
 *       voice_page_num = *(byte *)(voice_page_num + 0x7e);
 *       if (voice_page_num == 0xff) {
 *           // Second pass: look for any stealable voice
 *           ...
 *       }
 *   }
 *   voice_free();
 *   return 0;
 *============================================================================*/

uint8_t voice_steal_find(void)
{
    uint8_t page;
    uint8_t status;

    /* First pass: look for voice in release phase */
    page = g_intmem.active_voice_list_head;
    while (page != VOICE_LIST_END) {
        status = voice_page_read(page, VOICE_PAGE_STATUS);

        /* Check if voice is in release (bit 5 set) and active (bit 3 set) */
        if ((status & VOICE_STATUS_RELEASE) && (status & VOICE_STATUS_ACTIVE)) {
            /* Found a releasing voice - steal it */
            g_intmem.voice_page_num = page;
            voice_free();
            return 0;  /* Success - caller should retry allocation */
        }

        page = voice_list_next(page);
    }

    /* Second pass: look for any voice that can be stolen */
    page = g_intmem.active_voice_list_head;
    while (page != VOICE_LIST_END) {
        status = voice_page_read(page, VOICE_PAGE_STATUS);

        /* Check if voice has non-zero status (0xF8 check in original) */
        if (voice_page_read(page, 0xF8) != 0) {
            g_intmem.voice_page_num = page;
            voice_free();
            return 0;
        }

        page = voice_list_next(page);
    }

    /* No voice can be stolen */
    return VOICE_LIST_END;
}

/*============================================================================
 * voice_slot_allocate (CODE:A9CF)
 *
 * Allocate slot_count (0x4A) pages from free list.
 *
 * Original disassembly:
 *   a9cf: E556        MOV A,0x56           ; dram_slot_count
 *   a9d1: C3          CLR CY
 *   a9d2: 954A        SUBB A,0x4a          ; - slot_count
 *   a9d4: 5006        JNC 0xa9dc           ; if enough slots, continue
 *   a9d6: 12AA12      LCALL 0xaa12         ; else voice_steal_find
 *   a9d9: 60F4        JZ 0xa9cf            ; if stole one, retry
 *   a9db: 22          RET                  ; else return (A=0xFF)
 *   a9dc: F556        MOV 0x56,A           ; update dram_slot_count
 *   a9de: E554        MOV A,0x54           ; active_voice_list_head
 *   a9e0: B4FF04      CJNE A,#0xff,0xa9e7  ; if not empty, skip
 *   a9e3: E553        MOV A,0x53           ; dram_slot_free_list
 *   a9e5: F554        MOV 0x54,A           ; active_voice_list_head = free_list
 *   a9e7: A853        MOV R0,0x53          ; R0 = free_list head
 *   a9e9: AD55        MOV R5,0x55          ; R5 = old pending_voice_list
 *   a9eb: AA4A        MOV R2,0x4a          ; R2 = slot_count (loop counter)
 *   a9ed: 8855        MOV 0x55,R0          ; pending_voice_list = R0
 *   a9ef: E8          MOV A,R0
 *   a9f0: 247E        ADD A,#0x7e          ; address of next pointer
 *   a9f2: F8          MOV R0,A
 *   a9f3: E6          MOV A,@R0            ; get next in list
 *   a9f4: F8          MOV R0,A             ; R0 = next
 *   a9f5: DAF6        DJNZ R2,0xa9ed       ; loop slot_count times
 *   a9f7: BDFF02      CJNE R5,#0xff,0xa9fc ; if old pending != 0xFF
 *   a9fa: 8007        SJMP 0xaa03
 *   a9fc: ED          MOV A,R5             ; link old pending to free_list
 *   a9fd: 247E        ADD A,#0x7e
 *   a9ff: F9          MOV R1,A
 *   aa00: E553        MOV A,0x53
 *   aa02: F7          MOV @R1,A
 *   aa03: E553        MOV A,0x53           ; return value = first allocated
 *   aa05: FC          MOV R4,A
 *   aa06: E555        MOV A,0x55           ; new pending_voice_list
 *   aa08: 247E        ADD A,#0x7e
 *   aa0a: F9          MOV R1,A
 *   aa0b: 74FF        MOV A,#0xff
 *   aa0d: F7          MOV @R1,A            ; terminate list
 *   aa0e: 8853        MOV 0x53,R0          ; update free_list head
 *   aa10: EC          MOV A,R4
 *   aa11: 22          RET                  ; return first allocated
 *============================================================================*/

uint8_t voice_slot_allocate(void)
{
    uint8_t slots_needed;
    uint8_t remaining;
    uint8_t first_allocated;
    uint8_t current;
    uint8_t old_pending;
    uint8_t i;

    slots_needed = g_intmem.slot_count;

retry:
    /* Check if enough slots available */
    if (slots_needed > g_intmem.dram_slot_count) {
        /* Not enough - try to steal a voice */
        if (voice_steal_find() == 0) {
            /* Successfully stole one - retry allocation */
            goto retry;
        }
        /* Could not steal - allocation fails */
        return VOICE_LIST_END;
    }

    /* Update remaining slot count */
    remaining = g_intmem.dram_slot_count - slots_needed;
    g_intmem.dram_slot_count = remaining;

    /* If active list was empty, initialize it */
    if (g_intmem.active_voice_list_head == VOICE_LIST_END) {
        g_intmem.active_voice_list_head = g_intmem.dram_slot_free_list;
    }

    /* Save old pending and first allocated */
    old_pending = g_intmem.pending_voice_list;
    first_allocated = g_intmem.dram_slot_free_list;
    current = first_allocated;

    /* Walk through free list, taking slots_needed pages */
    for (i = 0; i < slots_needed; i++) {
        g_intmem.pending_voice_list = current;
        current = voice_list_next(current);
    }

    /* Link old pending to head of free list (if old pending existed) */
    if (old_pending != VOICE_LIST_END) {
        voice_list_set_next(old_pending, g_intmem.dram_slot_free_list);
    }

    /* Terminate new pending list */
    voice_list_set_next(g_intmem.pending_voice_list, VOICE_LIST_END);

    /* Update free list head to point past allocated slots */
    g_intmem.dram_slot_free_list = current;

    return first_allocated;
}

/*============================================================================
 * voice_deactivate (CODE:A69C)
 *
 * Mark voice for release (note-off). Voice continues through release phase.
 *
 * TODO: Implement based on Ghidra analysis
 *============================================================================*/

void voice_deactivate(uint8_t page)
{
    uint8_t status;

    /* Set release flag in voice page status */
    status = voice_page_read(page, VOICE_PAGE_STATUS);
    status |= VOICE_STATUS_RELEASE;
    voice_page_write(page, VOICE_PAGE_STATUS, status);

    /* Clear active flag - voice will be freed when envelope reaches zero */
    g_intmem.flags_20 |= FLAG20_VOICE_RELEASE;
}

/*============================================================================
 * voice_kill_channel (CODE:A785)
 *
 * Immediately kill all voices on a channel.
 *
 * TODO: Implement based on Ghidra analysis
 *============================================================================*/

void voice_kill_channel(uint8_t channel)
{
    uint8_t page;
    uint8_t next;
    uint8_t voice_channel;

    page = g_intmem.active_voice_list_head;
    while (page != VOICE_LIST_END) {
        /* Get channel from voice page (stored somewhere in page data) */
        /* TODO: Find exact offset where channel is stored */
        voice_channel = voice_page_read(page, 0x00) & 0x0F;  /* Placeholder */

        next = voice_list_next(page);

        if (voice_channel == channel) {
            g_intmem.voice_page_num = page;
            voice_free();
        }

        page = next;
    }
}

/*============================================================================
 * voice_init_slots (CODE:9A2D)
 *
 * Initialize SAM D-RAM slots for a voice. Main note-on entry point.
 *
 * Original disassembly:
 *   9a2d: E536        MOV A,0x36           ; program_base_dpl
 *   9a2f: 2409        ADD A,#0x9           ; offset 9 = flags byte
 *   9a31: F582        MOV DPL,A
 *   9a33: E535        MOV A,0x35           ; program_base_dph
 *   9a35: 3400        ADDC A,#0x0
 *   9a37: F583        MOV DPH,A
 *   9a39: ...         ; Read flags, extract slot_count
 *   9a45: F54A        MOV 0x4a,A           ; slot_count
 *   9a47: 12A9CF      LCALL slot_allocate
 *
 * Reads slot_count from program_base+9 (lower 4 bits).
 * Allocates voice pages from free list.
 * For each slot:
 *   - Sets voice page status (0xFB) = 0x20
 *   - Sets slot ID (0xFC) = nibble-swapped current_slot_id
 *   - Sets active marker (0x42) = 0xFF
 *   - Fills 0x70-0x73 with 0x0F (D-RAM flags)
 *   - Fills 0x74-0x7F with 0xFF
 *   - Writes D-RAM word 15 with algorithm info
 * Copies 8 bytes from program_base+22 to XRAM:11E6.
 *============================================================================*/

/* ROM data (extern) */
extern CODE uint8_t g_rom[];

/* Test hook: if non-NULL, use this instead of g_rom for testing */
static uint8_t *s_voice_test_rom = NULL;

/**
 * Set test ROM buffer (for unit testing)
 * Pass NULL to revert to using g_rom.
 */
void voice_set_test_rom(uint8_t *rom)
{
    s_voice_test_rom = rom;
}

/**
 * Read byte from ROM at address (internal helper)
 */
static inline uint8_t voice_rom_read(uint16_t addr)
{
    if (s_voice_test_rom != NULL) {
        return s_voice_test_rom[addr];
    }
    return g_rom[addr];
}

void voice_init_slots(void)
{
    uint16_t program_ptr;
    uint8_t flags_byte;
    uint8_t slot_count;
    uint8_t first_page;
    uint8_t page;
    uint8_t i;
    uint8_t offset;
    uint8_t dram_word15;

    /* Get program pointer */
    program_ptr = ((uint16_t)g_intmem.program_base_dph << 8) | g_intmem.program_base_dpl;

    /* Read flags byte from program_base + 9 */
    flags_byte = voice_rom_read(program_ptr + 9);

    /* Extract slot count (lower 4 bits) */
    slot_count = flags_byte & 0x0F;
    g_intmem.slot_count = slot_count;

    if (slot_count == 0) {
        return;  /* No slots to allocate */
    }

    /* Allocate voice pages from free list */
    first_page = voice_slot_allocate();
    if (first_page == VOICE_LIST_END) {
        return;  /* Allocation failed */
    }

    /* Initialize each allocated voice page */
    page = first_page;
    for (i = 0; i < slot_count && page != VOICE_LIST_END; i++) {
        g_intmem.voice_page_num = page;

        /* Set voice page status (offset 0xFB) = 0x20 */
        voice_page_write(page, VOICE_PAGE_STATUS, 0x20);

        /* Set slot ID (offset 0xFC) = nibble-swapped current_slot_id */
        voice_page_write(page, VOICE_PAGE_SLOT_ID,
                        swap_nibbles(g_intmem.current_slot_id));

        /* Set active marker (offset 0x42) = 0xFF */
        voice_page_write(page, VOICE_PAGE_ACTIVE, 0xFF);

        /* Fill D-RAM slot flags: 0x70-0x73 = 0x0F */
        for (offset = 0x70; offset < 0x74; offset++) {
            voice_page_write(page, offset, 0x0F);
        }

        /* Fill D-RAM slot flags: 0x74-0x7F = 0xFF */
        for (offset = 0x74; offset < 0x80; offset++) {
            voice_page_write(page, offset, 0xFF);
        }

        /* Write D-RAM word 15: algorithm + mix info
         * Value: (page_nibble_swap | 0x0F) : 0x00 : 0x00 : ctrl
         * The nibble-swapped page gives the D-RAM slot base address
         * 0x0F in high nibble sets full MIX level
         */
        dram_word15 = swap_nibbles(page) | 0x0F;
        sam_write_reg(SAM_REG_ADDR_DATA, swap_nibbles(page) | 0x0F);  /* D-RAM addr = slot*16 + 15 */
        sam_write_reg(SAM_REG_DATA1, g_intmem.current_slot_id);
        sam_write_reg(SAM_REG_DATA2, 0x00);
        sam_write_reg(SAM_REG_DATA3, dram_word15);
        sam_write_reg(SAM_REG_CTRL, g_intmem.sam_ctrl_flags);

        /* Get next voice page from linked list */
        page = voice_list_next(page);
    }

    /* Copy 8 bytes from program_base + 22 (0x16) to XRAM:11E6 (program_init_copy) */
    for (i = 0; i < 8; i++) {
        g_extmem.program_init_copy[i] = voice_rom_read(program_ptr + 0x16 + i);
    }

    /* Store first allocated page for further processing */
    g_intmem.voice_page_num = first_page;
}

/*============================================================================
 * voice_init_next_slot (CODE:AB40)
 *
 * Advances to next D-RAM word and continues dispatch.
 * This is essentially a wrapper that calls dram_config_advance_and_dispatch.
 *============================================================================*/

void voice_init_next_slot(void)
{
    /* This is now handled by dram_config_advance_and_dispatch in sam_dram_config.c */
    dram_config_advance_and_dispatch();
}

/*============================================================================
 * voice_dram_config_dispatch (CODE:AB4C)
 *
 * D-RAM config dispatch loop wrapper.
 * Now implemented in sam_dram_config.c
 *============================================================================*/

/* Forward to sam_dram_config.c implementation */
/* void voice_dram_config_dispatch(void) - use dram_config_dispatch() directly */

/*============================================================================
 * voice_init_copy_and_envelope (CODE:AB73)
 *
 * Copies envelope data to voice slot and processes envelope table.
 * Called during D-RAM configuration when setting up LFO/envelope blocks.
 *
 * Voice init data layout (7 bytes copied to slot[0..6]):
 *   [0]: Envelope segment table pointer - low byte
 *   [1]: Envelope segment table pointer - high byte
 *   [2]: Control flags (bit7=enable, bit6=sign, bits 6,3 for clear condition)
 *   [3]: Reserved
 *   [4]: Envelope sustain/attack parameter
 *   [5]: Envelope depth/modulation parameter
 *   [6]: Envelope rate
 *
 * After copy:
 *   - Clears slot[7]
 *   - Conditionally clears slot[8:9] based on flags
 *   - Scans envelope table for initial segment
 *   - Sets up rate in slot[10:11] = rate << 2
 *
 * ⚠️ PORTING SHORTCUTS:
 * - Velocity modulation of envelope params not implemented
 * - Envelope table scanning simplified
 * - velocity_curve_lookup replaced with linear
 *============================================================================*/

void voice_init_copy_and_envelope(void)
{
    uint8_t page = g_intmem.voice_page_num;
    uint8_t slot_base = g_intmem.voice_slot_base;

    /* Get ROM pointer from voice_data_ptr */
    uint16_t rom_ptr = ((uint16_t)g_intmem.voice_data_ptr_hi << 8) |
                       g_intmem.voice_data_ptr_lo;

    /* Check skip flag (_0_3 in original) */
    if (g_intmem.flags_20 & 0x08) {
        /* Skip this block, continue to next */
        voice_init_next_slot();
        return;
    }

    /* Copy 7 bytes from ROM to voice slot */
    for (uint8_t i = 0; i < 7; i++) {
        uint8_t value = ROM_BYTE(rom_ptr + i);
        voice_page_write(page, slot_base + i, value);
    }

    /* Clear slot[7] (output value) */
    voice_page_write(page, slot_base + 7, 0);

    /* Read control flags from slot[2] */
    uint8_t ctrl_flags = voice_page_read(page, slot_base + 2);

    /* Conditionally clear slot[8:9] (phase accumulator) */
    /* Original: if ((ctrl_flags & 0x48) != 0x40) clear */
    if ((ctrl_flags & 0x48) != 0x40) {
        voice_page_write(page, slot_base + 8, 0);
        voice_page_write(page, slot_base + 9, 0);
    }

    /* Get envelope table pointer from slot[0:1] */
    uint8_t env_ptr_lo = voice_page_read(page, slot_base);
    uint8_t env_ptr_hi = voice_page_read(page, slot_base + 1);
    uint16_t env_ptr = ((uint16_t)env_ptr_hi << 8) | env_ptr_lo;

    /* If envelope pointer is non-zero, scan envelope table */
    if (env_ptr != 0) {
        /* Scan 3-byte envelope entries until terminator found
         * Terminator: byte[2] != 0 OR (byte[1] & 0xF8) != 0 OR (byte[1] & 7) != 0
         */
        while (1) {
            /* Copy 3 bytes to slot[0xD:0xF] */
            uint8_t e0 = ROM_BYTE(env_ptr);
            uint8_t e1 = ROM_BYTE(env_ptr + 1);
            uint8_t e2 = ROM_BYTE(env_ptr + 2);

            voice_page_write(page, slot_base + 0x0D, e0);
            voice_page_write(page, slot_base + 0x0E, e1);
            voice_page_write(page, slot_base + 0x0F, e2);

            /* Check terminator conditions */
            if (e2 != 0 || (e1 & 0xF8) != 0) {
                break;  /* Found active entry */
            }

            /* Also copy e0 to slot[9] and check e1 bits 2:0 */
            voice_page_write(page, slot_base + 9, e0);

            if ((e1 & 0x07) != 0) {
                break;  /* Terminator found */
            }

            /* Advance envelope pointer by 3 */
            env_ptr += 3;

            /* Update slot[0:1] with new pointer (16-bit add with carry) */
            env_ptr_lo = voice_page_read(page, slot_base);
            voice_page_write(page, slot_base, env_ptr_lo + 3);
            if (env_ptr_lo > 0xFC) {
                env_ptr_hi = voice_page_read(page, slot_base + 1);
                voice_page_write(page, slot_base + 1, env_ptr_hi + 1);
            }
        }

        /* Clear slot[0xC] (level output) */
        voice_page_write(page, slot_base + 0x0C, 0);

        /* Handle bit 6 of control flags (sign flag) */
        if (ctrl_flags & 0x40) {
            uint8_t level = voice_page_read(page, slot_base + 0x0C);
            voice_page_write(page, slot_base + 0x0C, level | 0x80);
        }
    }

    /* Check envelope enable (bit 7 of ctrl_flags) */
    if (!(ctrl_flags & 0x80)) {
        /* No envelope enable - continue to next slot */
        voice_init_next_slot();
        return;
    }

    /* Envelope enabled - set up rate and level */
    uint8_t sustain = voice_page_read(page, slot_base + 4);

    /* Set initial level: 0x7F if sustain==0, else 0 */
    uint8_t level = (sustain == 0) ? 0x7F : 0x00;
    voice_page_write(page, slot_base + 6, level);

    /* Get original rate from slot[6] (was overwritten with level) */
    /* Note: We need to read rate BEFORE writing level - use the ROM value */
    uint8_t rate = ROM_BYTE(rom_ptr + 6);

    /* Set up rate in slot[10:11] = rate << 2 */
    if (rate == 0) {
        /* Zero rate: use default */
        uint8_t default_rate = (sustain == 0) ? 3 : 1;
        voice_page_write(page, slot_base + 10, default_rate);
        voice_page_write(page, slot_base + 11, 0);
    } else {
        /* rate << 2 = rate * 4 */
        voice_page_write(page, slot_base + 10, rate << 2);
        voice_page_write(page, slot_base + 11, rate >> 6);  /* High bits */
    }

    /* Clear slot[7] again (envelope output cleared at start of envelope) */
    voice_page_write(page, slot_base + 7, 0);

    /* Continue to next slot */
    voice_init_next_slot();
}
