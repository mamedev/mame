;--------------------------------------------------------
; MS4 Voice Utility Functions
;
; Extracted from MS4 firmware (ms4_05_r1_0.bin)
; Original addresses: 0xD417, 0xB70B
;
; For use with SDAS8051 assembler (SDCC toolchain)
;--------------------------------------------------------

        .module voice_util
        .optsdcc -mmcs51 --model-large

;--------------------------------------------------------
; Includes
;--------------------------------------------------------

        .include "voice_defs.inc"

;--------------------------------------------------------
; External References
;--------------------------------------------------------

        .globl _dptr_add_r6r7

;--------------------------------------------------------
; Public Symbols
;--------------------------------------------------------

        .globl _find_active_voice
        .globl _voice_pages_clear

;--------------------------------------------------------
; Code Segment
;--------------------------------------------------------

        .area CSEG (CODE)

;--------------------------------------------------------
; find_active_voice - Find first active voice in table
;
; Extracted from MS4 CODE:D417
;
; Searches voice table backward for first active voice.
; Returns voice index or 0xFF if none found.
;
; INPUTS:
;   IRAM 0x27 = Voice table high byte (VOICE_TABLE_HI)
;   IRAM 0x28 = Voice table low byte (VOICE_TABLE_LO)
;   IRAM 0x29 = Number of voices to search (VOICE_COUNT)
;
; OUTPUTS:
;   A = Voice index (0-based) or 0xFF if not found
;
; MODIFIES:
;   A, DPTR, IRAM 0x29 (VOICE_COUNT)
;--------------------------------------------------------
_find_active_voice:
        mov     dph, VOICE_TABLE_HI     ; DPTR = voice table base
        mov     dpl, VOICE_TABLE_LO

        mov     a, VOICE_COUNT          ; A = voice count
        jz      _fav_not_found          ; If zero, not found

        ; Point to last voice entry
        dec     a                       ; A = count - 1
        add     a, dpl                  ; DPL += index
        mov     dpl, a
        clr     a
        addc    a, dph                  ; Add carry to DPH
        mov     dph, a

_fav_search_loop:
        movx    a, @dptr                ; Read voice entry
        jnz     _fav_found              ; If non-zero, found

        ; Move to previous entry
        dec     dpl
        jnc     _fav_dec_done
        dec     dph
_fav_dec_done:
        djnz    VOICE_COUNT, _fav_search_loop

_fav_not_found:
        mov     a, #0xFF                ; Return not found
        ret

_fav_found:
        clr     c
        mov     a, dpl                  ; A = current DPL
        subb    a, VOICE_TABLE_LO       ; A = index from base
        ret

;--------------------------------------------------------
; voice_pages_clear - Clear voice page tables
;
; Extracted from MS4 CODE:B70B
;
; Clears two XRAM regions used for voice page data:
;   - XRAM 0x14D5-0x1CD4 (0x800 bytes)
;   - XRAM 0x1CD5-0x1CE4 (16 bytes)
;
; INPUTS:
;   None
;
; OUTPUTS:
;   None
;
; MODIFIES:
;   A, R0, R6, R7, DPTR, IRAM 0x08-0x09
;
; CALLS:
;   dptr_add_r6r7
;--------------------------------------------------------
_voice_pages_clear:
        ; Clear first region: 0x14D5, 0x800 bytes
        mov     LOOP_CNT_LO, #0x00      ; Counter = 0x0000
        mov     LOOP_CNT_HI, #0x00

_vpc_loop1:
        mov     r6, LOOP_CNT_HI         ; R6:R7 = counter
        mov     r7, LOOP_CNT_LO

        ; Check if counter >= 0x0800 (borrow from 0x07FF - counter)
        mov     a, #0xFF
        clr     c
        subb    a, r7                   ; 0xFF - R7
        mov     a, #0x07
        subb    a, r6                   ; 0x07 - R6 - borrow
        jc      _vpc_region2            ; If borrow, done with region 1

        mov     dptr, #VOICE_PAGES_1    ; DPTR = 0x14D5
        lcall   _dptr_add_r6r7          ; DPTR += R6:R7

        mov     a, #0x00
        movx    @dptr, a                ; Clear byte

        ; Increment 16-bit counter
        mov     r0, #LOOP_CNT_LO
        mov     a, #0x01
        add     a, @r0
        mov     @r0, a
        jnc     _vpc_inc1_done
        dec     r0
        clr     a
        addc    a, @r0
        mov     @r0, a
_vpc_inc1_done:
        jnc     _vpc_loop1

        ; Clear second region: 0x1CD5, 16 bytes
_vpc_region2:
        mov     LOOP_CNT_LO, #0x00      ; Counter = 0x0000
        mov     LOOP_CNT_HI, #0x00

_vpc_loop2:
        mov     r6, LOOP_CNT_HI         ; R6:R7 = counter
        mov     r7, LOOP_CNT_LO

        ; Check if counter >= 0x10 (16 bytes)
        mov     a, #0x0F
        cjne    r6, #0x00, _vpc_done    ; If R6 != 0, done
        subb    a, r7                   ; 0x0F - R7 (C was cleared by CJNE)
        jc      _vpc_done               ; If borrow, done

        mov     dptr, #VOICE_PAGES_2    ; DPTR = 0x1CD5
        lcall   _dptr_add_r6r7          ; DPTR += R6:R7

        mov     a, #0x00
        movx    @dptr, a                ; Clear byte

        ; Increment 16-bit counter
        mov     r0, #LOOP_CNT_LO
        mov     a, #0x01
        add     a, @r0
        mov     @r0, a
        jnc     _vpc_inc2_done
        dec     r0
        clr     a
        addc    a, @r0
        mov     @r0, a
_vpc_inc2_done:
        jnc     _vpc_loop2

_vpc_done:
        ret

;--------------------------------------------------------
; End of module
;--------------------------------------------------------

