;--------------------------------------------------------
; MS4 Voice Management Functions
;
; Extracted from MS4 firmware (ms4_05_r1_0.bin)
; Original addresses: 0xA69C-0xA7B2
;
; For use with SDAS8051 assembler (SDCC toolchain)
;--------------------------------------------------------

        .module voice_mgmt
        .optsdcc -mmcs51 --model-large

;--------------------------------------------------------
; Includes
;--------------------------------------------------------

        .include "../core/sam_defs.inc"
        .include "voice_defs.inc"

;--------------------------------------------------------
; External References
;--------------------------------------------------------

        .globl _sam_dram_write
        .globl _indexed_array_access

;--------------------------------------------------------
; Public Symbols
;--------------------------------------------------------

        .globl _voice_deactivate
        .globl _voice_kill_channel

;--------------------------------------------------------
; SFR Definitions
;--------------------------------------------------------

P2      .equ    0xA0

;--------------------------------------------------------
; Code Segment
;--------------------------------------------------------

        .area CSEG (CODE)

;--------------------------------------------------------
; voice_deactivate - Deactivate a voice
;
; Extracted from MS4 CODE:A69C (233 bytes)
;
; Deactivates the voice on the current page, clearing slot
; assignments and resetting SAM D-RAM control words.
;
; INPUTS:
;   IRAM 0x3A = Voice page number (P2 value)
;
; OUTPUTS:
;   None
;
; MODIFIES:
;   A, R0, R1, R2, R3, R4, R5, R6, DPTR, IRAM 0x38, 0x3B, B
;
; NOTES:
;   Complex function with multiple paths depending on slot
;   configuration. Contains LJMP 0x0000 as error handler.
;--------------------------------------------------------
_voice_deactivate:
        mov     a, VOICE_P2_SAVE        ; A = voice page
        mov     P2, a                   ; Select voice page
        swap    a                       ; A = page << 4 (slot addr base)
        mov     VOICE_SLOT_ADDR, a      ; Save slot address

        ; Get voice data pointer from page
        mov     r0, #0xFE               ; R0 = voice ptr offset
        movx    a, @r0                  ; A = low byte
        mov     dpl, a
        inc     r0
        movx    a, @r0                  ; A = high byte
        mov     dph, a

        ; Clear active flags (keep bits 7,6,4)
        movx    a, @dptr
        anl     a, #0xD0                ; Preserve 0xD0 bits
        movx    @dptr, a

        ; Clear slot assignment entries
        clr     a
        mov     r0, a                   ; R0 = 0

_vd_clear_loop:
        add     a, #0x02                ; Skip to slot entry
        mov     r0, a
        movx    a, @r0                  ; Read slot entry
        cjne    a, #0xFF, _vd_clear_slots
        sjmp    _vd_check_linked

_vd_clear_slots:
        mov     r6, #0x0E               ; Clear 14 bytes
        clr     a
_vd_clear_inner:
        movx    @r0, a                  ; Clear byte
        inc     r0
        djnz    r6, _vd_clear_inner

        mov     a, r0
        cjne    a, #0x70, _vd_loop_check
_vd_loop_check:
        jc      _vd_clear_loop          ; Continue if R0 < 0x70
        ljmp    0x0000                  ; Error - should not reach

_vd_check_linked:
        mov     r0, #0x02
        mov     a, #0xFF
        movx    @r0, a                  ; Mark slot 2 as unused

        mov     r0, #0x80               ; Check linked slots
        movx    a, @r0
        cjne    a, #0xFF, _vd_process_linked
        ljmp    _vd_done

_vd_next_linked:
        inc     VOICE_SLOT_ADDR         ; Next slot
        mov     a, r0
        add     a, #0x08                ; Next slot entry
        mov     r0, a
        cjne    a, #0xF8, _vd_check_link_entry

_vd_done:
        mov     r0, #0xFB
        mov     a, #0x08
        movx    @r0, a                  ; Write 0x08 to 0xFB

        mov     r0, #0x70
        mov     a, #0xFF
        movx    @r0, a                  ; Mark 0x70 unused
        ret

_vd_process_linked:
        cjne    a, #0x28, _vd_check_slot_type
        ljmp    _vd_handle_0x28

_vd_check_link_entry:
        movx    a, @r0
        cjne    a, #0xFF, _vd_process_linked
        ljmp    _vd_done

_vd_check_slot_type:
        anl     a, #0x7A
        cjne    a, #0x12, _vd_next_linked

        ; Slot type 0x12 - write SAM control
        mov     dptr, #0x8000           ; SAM base
        movx    a, @r0                  ; Read slot data
        jb      0xE2, _vd_slot_type2    ; Check bit 5

        ; Type 1: Write address, clear data, write control
        mov     a, VOICE_SLOT_ADDR
        movx    @dptr, a                ; Write address
        inc     dpl
        inc     dpl
        clr     a
        movx    @dptr, a                ; Write 0 to data 2
        inc     dpl
        movx    @dptr, a                ; Write 0 to data 3
        inc     dpl
        mov     a, VOICE_CTRL_SHADOW
        movx    @dptr, a                ; Write control
        sjmp    _vd_next_linked

_vd_slot_type2:
        ; Type 2: Decrement address, read back, check status
        mov     a, VOICE_SLOT_ADDR
        dec     a
        movx    @dptr, a                ; Write addr-1
        mov     dpl, #0x04              ; Control register
        mov     a, VOICE_CTRL_SHADOW
        anl     a, #0xFE                ; Clear WE
        movx    @dptr, a                ; Trigger read
        dec     dpl                     ; Data register 3
        nop
        nop
        movx    a, @dptr                ; Read data
        anl     a, #0x07
        jz      _vd_next_linked         ; If zero, continue

        cjne    a, #0x07, _vd_slot_write

        ; Check for decay complete
        dec     dpl                     ; Data register 2
        movx    a, @dptr
        cpl     a
        inc     a
        anl     a, #0xE0
        jnz     _vd_slot_write          ; Not zero, do write

        ; Decay complete - write slot reset
        mov     a, VOICE_SLOT_ADDR
        mov     dpl, #0x00
        movx    @dptr, a                ; Address
        inc     dpl
        mov     a, #0x01
        movx    @dptr, a                ; Data low = 1
        inc     dpl
        dec     a
        movx    @dptr, a                ; Data mid = 0
        inc     dpl
        movx    @dptr, a                ; Data high = 0
        inc     dpl
        mov     a, VOICE_CTRL_SHADOW
        movx    @dptr, a                ; Control
        sjmp    _vd_next_linked

_vd_slot_write:
        mov     a, VOICE_SLOT_ADDR
        mov     dpl, #0x00
        movx    @dptr, a                ; Address
        inc     dpl
        mov     a, #0x43
        movx    @dptr, a                ; Data low = 0x43
        inc     dpl
        mov     a, #0x05
        movx    @dptr, a                ; Data mid = 0x05
        inc     dpl
        clr     a
        movx    @dptr, a                ; Data high = 0
        inc     dpl
        mov     a, VOICE_CTRL_SHADOW
        movx    @dptr, a                ; Control
        ljmp    _vd_next_linked

_vd_handle_0x28:
        mov     B, r0                   ; Save R0
        mov     r6, VOICE_P2_SAVE       ; R6 = page
        lcall   _indexed_array_access   ; External call
        inc     VOICE_SLOT_ADDR
        mov     a, B
        add     a, #0x08
        mov     0x3B, a                 ; Save next offset
        add     a, #0x03
        mov     r0, a
        movx    a, @r0
        anl     a, #0x18                ; Check bits 4,3
        cjne    a, #0x18, _vd_restore_ptr
        mov     r0, 0x3B
        ljmp    _vd_next_linked

_vd_restore_ptr:
        mov     r0, 0x3B
        ljmp    _vd_next_linked

;--------------------------------------------------------
; voice_kill_channel - Kill all voices on channel
;
; Extracted from MS4 CODE:A785 (46 bytes)
;
; Iterates through voice linked list, deactivating all
; voices that match the specified MIDI channel.
;
; INPUTS:
;   IRAM 0x34 = MIDI channel to kill (0-15)
;   IRAM 0x54 = Voice list head (0xFF = empty)
;
; OUTPUTS:
;   None
;
; MODIFIES:
;   A, R0, R1, P2
;
; CALLS:
;   voice_deactivate
;
; NOTES:
;   Follows linked list via offset 0x7E in each page.
;   Sets 0x02 and 0x70 to 0xFF after deactivation.
;--------------------------------------------------------
_voice_kill_channel:
        mov     a, VOICE_LIST_HEAD      ; A = first voice page
        cjne    a, #0xFF, _vkc_process
        ljmp    _vkc_done               ; List empty, return

_vkc_process:
        mov     VOICE_P2_SAVE, a        ; Save page
        mov     P2, a                   ; Select page

        mov     r1, #0xFC               ; Get channel from page
        movx    a, @r1
        swap    a
        anl     a, #0x0F                ; Channel in bits 7:4

        xrl     a, VOICE_CHANNEL        ; Compare with target
        jnz     _vkc_next               ; No match, skip

        ; Match - deactivate this voice
        lcall   _voice_deactivate

        ; Restore page and clear slots
        mov     P2, VOICE_P2_SAVE
        mov     a, #0xFF
        mov     r0, #0x02
        movx    @r0, a                  ; Clear slot 2
        mov     r0, #0x70
        movx    @r0, a                  ; Clear slot 0x70

_vkc_next:
        mov     a, VOICE_P2_SAVE        ; Get saved page
        add     a, #0x7E                ; Offset to next pointer
        mov     r1, a
        mov     a, @r1                  ; Read next page
        cjne    a, #0xFF, _vkc_process  ; Continue if not end

_vkc_done:
        ret

;--------------------------------------------------------
; End of module
;--------------------------------------------------------

