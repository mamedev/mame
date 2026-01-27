;--------------------------------------------------------
; MS4 Utility Functions
;
; Extracted from MS4 firmware (ms4_05_r1_0.bin)
; Original address: 0xDCA8
;
; For use with SDAS8051 assembler (SDCC toolchain)
;--------------------------------------------------------

        .module util
        .optsdcc -mmcs51 --model-large

;--------------------------------------------------------
; Public Symbols
;--------------------------------------------------------

        .globl _dptr_add_r6r7

;--------------------------------------------------------
; Code Segment
;--------------------------------------------------------

        .area CSEG (CODE)

;--------------------------------------------------------
; dptr_add_r6r7 - Add R6:R7 to DPTR
;
; Extracted from MS4 CODE:DCA8
;
; Adds 16-bit value in R6:R7 to DPTR.
; R6 = high byte, R7 = low byte.
;
; INPUTS:
;   DPTR = Base address
;   R6 = Offset high byte
;   R7 = Offset low byte
;
; OUTPUTS:
;   DPTR = DPTR + R6:R7
;
; MODIFIES:
;   A, DPTR
;--------------------------------------------------------
_dptr_add_r6r7:
        mov     a, dpl          ; A = DPTR low
        add     a, r7           ; A = DPL + R7
        mov     dpl, a          ; DPL = result low

        mov     a, dph          ; A = DPTR high
        addc    a, r6           ; A = DPH + R6 + carry
        mov     dph, a          ; DPH = result high

        ret

;--------------------------------------------------------
; End of module
;--------------------------------------------------------

