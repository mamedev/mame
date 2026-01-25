;--------------------------------------------------------
; SAM8905 D-RAM Access Functions
;
; Extracted from MS4 firmware (ms4_05_r1_0.bin)
; Original addresses: 0xA4BC-0xA564
;
; For use with SDAS8051 assembler (SDCC toolchain)
;
; IMPORTANT: P2 must be set to 0x80 before calling these
; functions. Caller is responsible for save/restore of P2.
;--------------------------------------------------------

        .module sam_dram
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

        .globl _sam_dram_write
        .globl _sam_dram_write_wait
        .globl _sam_dram_read
        .globl _sam_dram_write_word15
        .globl _sam_dram_clear_all

;--------------------------------------------------------
; Code Segment
;--------------------------------------------------------

        .area CSEG (CODE)

;--------------------------------------------------------
; sam_dram_write - Write single D-RAM word
;
; Extracted from MS4 CODE:A4BC (sam_write_dram)
;
; INPUTS:
;   R5 = D-RAM address (0-255)
;   R2 = Data low byte [7:0]
;   R3 = Data mid byte [15:8]
;   R4 = Data high byte [18:16]
;   IRAM 0x37 = Control byte (SAM_CTRL_SHADOW)
;
; OUTPUTS:
;   None
;
; MODIFIES:
;   A, R0
;
; NOTES:
;   P2 must be 0x80 before calling
;   Control byte typically has SAM_CTRL_WE set
;--------------------------------------------------------
_sam_dram_write:
        mov     r0, #SAM_ADDR           ; R0 = 0 (address register)
        mov     a, r5                   ; A = address
        movx    @r0, a                  ; Write address to SAM

        mov     a, r2                   ; A = data low
        inc     r0                      ; R0 = 1 (data low register)
        movx    @r0, a                  ; Write data low

        mov     a, r3                   ; A = data mid
        inc     r0                      ; R0 = 2 (data mid register)
        movx    @r0, a                  ; Write data mid

        mov     a, r4                   ; A = data high
        inc     r0                      ; R0 = 3 (data high register)
        movx    @r0, a                  ; Write data high

        mov     a, SAM_CTRL_SHADOW      ; A = control byte
        inc     r0                      ; R0 = 4 (control register)
        movx    @r0, a                  ; Write control (triggers write)

        ret

;--------------------------------------------------------
; sam_dram_write_wait - Write D-RAM word with wait states
;
; Extracted from MS4 CODE:A4CE (FUN_CODE_a4ce)
;
; Same as sam_dram_write but includes NOP delays after
; writing control register. Used when timing is critical.
;
; INPUTS:
;   R5 = D-RAM address (0-255)
;   R2 = Data low byte [7:0]
;   R3 = Data mid byte [15:8]
;   R4 = Data high byte [18:16]
;   IRAM 0x37 = Control byte
;
; OUTPUTS:
;   None
;
; MODIFIES:
;   A, R0
;--------------------------------------------------------
_sam_dram_write_wait:
        mov     r0, #SAM_ADDR           ; R0 = 0 (address register)
        mov     a, r5                   ; A = address
        movx    @r0, a                  ; Write address

        mov     a, r2                   ; A = data low
        inc     r0                      ; R0 = 1
        movx    @r0, a                  ; Write data low

        mov     a, r3                   ; A = data mid
        inc     r0                      ; R0 = 2
        movx    @r0, a                  ; Write data mid

        mov     a, r4                   ; A = data high
        inc     r0                      ; R0 = 3
        movx    @r0, a                  ; Write data high

        mov     a, SAM_CTRL_SHADOW      ; A = control byte
        inc     r0                      ; R0 = 4
        movx    @r0, a                  ; Write control

        ; Wait states for SAM to complete write
        nop
        nop
        nop
        nop
        movx    @r0, a                  ; Re-assert control
        movx    a, @r0                  ; Dummy read (sync)

        ret

;--------------------------------------------------------
; sam_dram_read - Read single D-RAM word
;
; Extracted from MS4 CODE:A4E6 (FUN_CODE_a4e6)
;
; INPUTS:
;   R5 = D-RAM address (0-255)
;   IRAM 0x37 = Control byte (will be masked for read)
;
; OUTPUTS:
;   R2 = Data low byte [7:0]
;   R3 = Data mid byte [15:8]
;   R4 = Data high byte [18:16]
;
; MODIFIES:
;   A, R0, R2, R3, R4
;
; NOTES:
;   Clears WE bit in control for read operation
;--------------------------------------------------------
_sam_dram_read:
        mov     r0, #SAM_ADDR           ; R0 = 0 (address register)
        mov     a, r5                   ; A = address
        movx    @r0, a                  ; Write address to SAM

        mov     a, SAM_CTRL_SHADOW      ; A = control byte
        anl     a, #0xFE                ; Clear WE bit (read mode)
        mov     r0, #SAM_CTRL           ; R0 = 4 (control register)
        movx    @r0, a                  ; Write control (triggers read)

        mov     r0, #SAM_DATA_LO        ; R0 = 1 (data low register)
        nop                             ; Wait for SAM
        nop
        movx    a, @r0                  ; Read data low
        mov     r2, a                   ; R2 = data low

        inc     r0                      ; R0 = 2 (data mid register)
        movx    a, @r0                  ; Read data mid
        mov     r3, a                   ; R3 = data mid

        inc     r0                      ; R0 = 3 (data high register)
        movx    a, @r0                  ; Read data high
        mov     r4, a                   ; R4 = data high

        ret

;--------------------------------------------------------
; sam_dram_write_word15 - Write slot control word
;
; Extracted from MS4 CODE:A523 (sam_dram_write_word15)
;
; Reads current control word (word 15), sets IDLE bit,
; and writes it back. Used to activate/deactivate slots.
;
; INPUTS:
;   R6 = Slot number (0-15)
;
; OUTPUTS:
;   A = Address that was read (for chaining)
;
; MODIFIES:
;   A, R0, R2, R3, R4, R5
;
; CALLS:
;   sam_dram_read, sam_dram_write_wait
;--------------------------------------------------------
_sam_dram_write_word15:
        mov     P2, #SAM_P2_SELECT      ; P2 = 0x80 (select SAM)

        ; Compute address: (slot << 4) | 0x0F
        mov     a, r6                   ; A = slot number
        swap    a                       ; A = slot << 4
        orl     a, #SAM_DRAM_CTRL_WORD  ; A |= 0x0F
        mov     r5, a                   ; R5 = address

        ; Read current control word
        lcall   _sam_dram_read          ; R2/R3/R4 = current data

        ; Set IDLE bit in high byte
        mov     a, r3                   ; A = data mid
        orl     a, #SAM_IDLE_BIT        ; Set bit 3 (IDLE = bit 11)
        mov     r3, a                   ; R3 = modified mid

        ; Write back
        lcall   _sam_dram_write_wait    ; Write with wait

        ; Read back address for return value
        mov     r0, #SAM_ADDR
        movx    a, @r0

        ; Restore P2
        mov     P2, SAM_P2_SAVE

        ret

;--------------------------------------------------------
; sam_dram_clear_all - Clear all D-RAM
;
; Extracted from MS4 CODE:A53C (sam_dram_clear_all)
;
; Clears all 256 D-RAM addresses to zero, then sets
; IDLE bit in control word for all 16 slots.
;
; INPUTS:
;   None
;
; OUTPUTS:
;   None
;
; MODIFIES:
;   A, R0, R2, R3, R4, R5, R6
;
; CALLS:
;   sam_dram_write, sam_dram_write_word15
;
; NOTES:
;   Sets SAM_CTRL_SHADOW to 0x05 (D-RAM write enable)
;--------------------------------------------------------
_sam_dram_clear_all:
        mov     SAM_CTRL_SHADOW, #0x05  ; D-RAM write, no A-RAM

        ; Clear data registers
        clr     a
        mov     r5, a                   ; R5 = address (starts at 0)

        ; Clear all 256 D-RAM words
_dram_clear_loop:
        mov     P2, #SAM_P2_SELECT      ; P2 = 0x80
        mov     r2, a                   ; R2 = 0 (data low)
        mov     r3, a                   ; R3 = 0 (data mid)
        mov     r4, a                   ; R4 = 0 (data high)
        lcall   _sam_dram_write         ; Write zero

        clr     a                       ; A = 0
        djnz    r5, _dram_clear_loop    ; Loop 256 times (R5 wraps 0->255->0)

        ; Dummy read (sync)
        movx    a, @r0

        ; Set IDLE bit in all 16 slot control words
        mov     r6, #0                  ; R6 = slot counter
_slot_idle_loop:
        lcall   _sam_dram_write_word15  ; Set IDLE for slot R6
        inc     r6                      ; Next slot
        cjne    r6, #SAM_DRAM_SLOTS, _slot_idle_loop

        ; Clear A-RAM select bit in control shadow
        anl     SAM_CTRL_SHADOW, #0xFB  ; Clear bit 2

        ; Final sync
        mov     P2, #SAM_P2_SELECT
        mov     r0, #SAM_ADDR
        movx    a, @r0                  ; Dummy read
        movx    a, @r0                  ; Dummy read

        ; Restore P2
        mov     P2, SAM_P2_SAVE

        ret

;--------------------------------------------------------
; End of module
;--------------------------------------------------------
