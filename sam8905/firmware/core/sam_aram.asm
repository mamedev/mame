;--------------------------------------------------------
; SAM8905 A-RAM Access Functions
;
; Extracted from MS4 firmware (ms4_05_r1_0.bin)
; Original addresses: 0xAD43-0xAD81, 0xD866-0xD888
;
; For use with SDAS8051 assembler (SDCC toolchain)
;--------------------------------------------------------

        .module sam_aram
        .optsdcc -mmcs51 --model-large

;--------------------------------------------------------
; Includes
;--------------------------------------------------------

        .include "sam_defs.inc"

;--------------------------------------------------------
; External References
;--------------------------------------------------------

        ; None - these are leaf functions

;--------------------------------------------------------
; Public Symbols
;--------------------------------------------------------

        .globl _sam_aram_write
        .globl _sam_aram_load_alg

;--------------------------------------------------------
; Code Segment
;--------------------------------------------------------

        .area CSEG (CODE)

;--------------------------------------------------------
; sam_aram_write - Write 32-word algorithm to A-RAM
;
; Extracted from MS4 CODE:AD43 (sam_write_aram)
;
; Writes a complete 32-instruction algorithm to A-RAM.
; Source can be CODE space (MOVC) or XRAM (MOVX).
;
; INPUTS:
;   DPTR = Pointer to algorithm data (64 bytes = 32 x 2)
;   R5 = Starting A-RAM address (typically alg_slot << 5)
;   IRAM 0x37 = Base control byte (SAM_CTRL_SHADOW)
;   Bit 0x01 = Source select (0=CODE, 1=XRAM)
;
; OUTPUTS:
;   DPTR = Points past end of source data
;   R5 = Address after last write (start + 32)
;
; MODIFIES:
;   A, R0, R5, R6, DPTR, Bit 0x01, Bit 0x02
;
; NOTES:
;   Returns immediately if DPTR == 0x0000 (null check)
;   Saves/restores source select bit via Bit 0x02
;   Sets P2=0x80 internally, restores from IRAM 0x3A
;--------------------------------------------------------
_sam_aram_write:
        ; Null pointer check
        mov     a, dpl
        orl     a, dph
        jnz     _aram_write_start
        ret                             ; Return if DPTR == 0

_aram_write_start:
        mov     r6, #SAM_ARAM_INSTS     ; R6 = 32 (loop counter)

        ; Save source select bit
        mov     c, BIT_ARAM_SRC_XRAM    ; C = bit 0x01
        mov     BIT_ARAM_SRC_SAVE, c    ; Save to bit 0x02

        ; Optimization: jump to loop (original has LJMP)
        sjmp    _aram_write_loop

_aram_write_loop:
        mov     P2, #SAM_P2_SELECT      ; P2 = 0x80 (select SAM)

        ; Write address
        mov     r0, #SAM_ADDR           ; R0 = 0
        mov     a, r5                   ; A = A-RAM address
        movx    @r0, a                  ; Write address
        inc     r5                      ; Increment address for next

        ; Read low byte from source
        inc     r0                      ; R0 = 1 (data low)
        jnb     BIT_ARAM_SRC_XRAM, _aram_src_code_lo
        movx    a, @dptr                ; Read from XRAM
        sjmp    _aram_write_lo
_aram_src_code_lo:
        clr     a
        movc    a, @a+dptr              ; Read from CODE
_aram_write_lo:
        inc     dptr                    ; Next source byte
        movx    @r0, a                  ; Write data low

        ; Read high byte from source
        inc     r0                      ; R0 = 2 (data high, 7 bits)
        jnb     BIT_ARAM_SRC_XRAM, _aram_src_code_hi
        movx    a, @dptr                ; Read from XRAM
        sjmp    _aram_write_hi
_aram_src_code_hi:
        clr     a
        movc    a, @a+dptr              ; Read from CODE
_aram_write_hi:
        inc     dptr                    ; Next source byte
        movx    @r0, a                  ; Write data high

        ; Write control with A-RAM select bit
        mov     r0, #SAM_CTRL           ; R0 = 4
        mov     a, SAM_CTRL_SHADOW      ; A = control shadow
        orl     a, #SAM_CTRL_ARAM       ; Set A-RAM select bit
        movx    @r0, a                  ; Trigger write

        ; Loop
        djnz    r6, _aram_write_loop

        ; Restore P2
        mov     P2, SAM_P2_SAVE

        ; Restore source select bit
        mov     c, BIT_ARAM_SRC_SAVE
        mov     BIT_ARAM_SRC_XRAM, c

        ret

;--------------------------------------------------------
; sam_aram_load_alg - Load algorithm from CODE space
;
; Extracted from MS4 CODE:D866 (sam_aram_load_alg0)
;
; Simplified version that always reads from CODE space.
; Originally hardcoded to load algorithm 0 from CODE:023A.
;
; INPUTS:
;   DPTR = Pointer to algorithm data in CODE (64 bytes)
;   R5 = Starting A-RAM address (alg_slot << 5)
;   IRAM 0x37 = Base control byte
;
; OUTPUTS:
;   DPTR = Points past end of source data
;   R5 = Address after last write
;
; MODIFIES:
;   A, R0, R5, R6, DPTR
;
; NOTES:
;   Always reads from CODE space (uses MOVC)
;   Does not save/restore P2 (leaves P2=0x80)
;--------------------------------------------------------
_sam_aram_load_alg:
        mov     P2, #SAM_P2_SELECT      ; P2 = 0x80
        mov     r6, #SAM_ARAM_INSTS     ; R6 = 32 (loop counter)

_aram_load_loop:
        ; Write address
        mov     r0, #SAM_ADDR           ; R0 = 0
        mov     a, r5                   ; A = A-RAM address
        movx    @r0, a                  ; Write address
        inc     r5                      ; Next address

        ; Read and write low byte
        inc     r0                      ; R0 = 1
        clr     a
        movc    a, @a+dptr              ; Read from CODE
        inc     dptr
        movx    @r0, a                  ; Write data low

        ; Read and write high byte
        inc     r0                      ; R0 = 2
        clr     a
        movc    a, @a+dptr              ; Read from CODE
        inc     dptr
        movx    @r0, a                  ; Write data high

        ; Write control with A-RAM select
        mov     r0, #SAM_CTRL           ; R0 = 4
        mov     a, SAM_CTRL_SHADOW      ; A = control shadow
        orl     a, #SAM_CTRL_ARAM       ; Set A-RAM bit
        movx    @r0, a                  ; Trigger write

        ; Loop
        djnz    r6, _aram_load_loop

        ret

;--------------------------------------------------------
; End of module
;--------------------------------------------------------
