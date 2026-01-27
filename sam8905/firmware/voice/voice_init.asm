;--------------------------------------------------------
; MS4 Voice Initialization Functions
;
; Extracted from MS4 firmware (ms4_05_r1_0.bin)
; Original addresses: 0x9A2D, 0xB4BF
;
; For use with SDAS8051 assembler (SDCC toolchain)
;--------------------------------------------------------

        .module voice_init
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
        .globl _sam_aram_write
        .globl _slot_allocate         ; FUN_CODE_a9cf

;--------------------------------------------------------
; Public Symbols
;--------------------------------------------------------

        .globl _voice_init_slots
        .globl _voice_assign_algorithm

;--------------------------------------------------------
; SFR Definitions
;--------------------------------------------------------

P2      .equ    0xA0
B       .equ    0xF0

;--------------------------------------------------------
; Code Segment
;--------------------------------------------------------

        .area CSEG (CODE)

;--------------------------------------------------------
; voice_assign_algorithm - Assign algorithm to voice
;
; Extracted from MS4 CODE:B4BF (153 bytes)
;
; Assigns an algorithm to a voice slot. Handles algorithm
; table management and calls sam_aram_write to load the
; algorithm data.
;
; INPUTS:
;   IRAM 0x34 = MIDI channel (VOICE_CHANNEL)
;   IRAM 0x35 = Voice page high (VOICE_PAGE_HI)
;   IRAM 0x36 = Voice page low (VOICE_PAGE_LO)
;   B = Algorithm number
;
; OUTPUTS:
;   None
;
; MODIFIES:
;   A, R1, R2, R5, R6, R7, DPTR, B, IRAM 0x41-0x42
;
; CALLS:
;   FUN_CODE_b49f, sam_aram_write
;
; NOTES:
;   Initial delay loop: 4 × 224 × 4 cycles = 3584 cycles
;   Manages algorithm table at XRAM 0x11EE
;--------------------------------------------------------
_voice_assign_algorithm:
        ; Delay loop
        mov     r2, #0x04               ; Outer count = 4
_vaa_delay_outer:
        mov     r5, #0xE0               ; Inner count = 224
_vaa_delay_inner:
        nop
        nop
        nop
        nop
        djnz    r5, _vaa_delay_inner
        djnz    r2, _vaa_delay_outer

        ; Get algorithm info
        mov     a, VOICE_CHANNEL
        mov     r7, a
        lcall   0xB49F                  ; FUN_CODE_b49f (get algo info)

        ; Check if channel >= 16
        mov     a, VOICE_CHANNEL
        cjne    a, #0x10, _vaa_check_range
_vaa_check_range:
        jc      _vaa_load_algo          ; If < 16, load algorithm

        ; Channel >= 16: Search algorithm table
        mov     r6, #0x00               ; R6 = table index
        mov     dptr, #ALG_TABLE_BASE   ; DPTR = 0x11EE

_vaa_search_loop:
        movx    a, @dptr                ; Read table entry
        jb      BIT_ALG_NEW, _vaa_check_match  ; If new alg flag set
        jz      _vaa_empty_slot         ; If zero, empty slot

_vaa_check_match:
        cjne    a, B, _vaa_not_match    ; Compare with requested alg
        ; Match found - copy slot mapping
        mov     a, VOICE_CHANNEL
        add     a, #ALG_MAP_BASE        ; 0x34 + 0x8E = channel->alg map
        mov     r1, a
        mov     a, r6
        add     a, #ALG_SLOT_BASE       ; 0x9E = slot->alg map
        mov     r0, a
        mov     a, @r0                  ; Get slot from table
        mov     @r1, a                  ; Copy to channel map
        ljmp    _vaa_load_algo

_vaa_not_match:
        cjne    a, #0xFF, _vaa_next     ; If not 0xFF, continue search
        ljmp    _vaa_not_found          ; End of table, not found

_vaa_next:
        inc     dptr
        inc     r6
        cjne    r6, #ALG_TABLE_SIZE, _vaa_search_loop  ; Loop if < 8

_vaa_not_found:
        ljmp    0x0000                  ; Error - algorithm table full

_vaa_empty_slot:
        ; Allocate empty slot in table
        mov     a, B
        movx    @dptr, a                ; Store algorithm number
        inc     dptr
        movx    a, @dptr                ; Check next entry
        cjne    a, #0xFF, _vaa_check_match  ; If not end, check match
        setb    BIT_ALG_NEW             ; Set new algorithm flag
        sjmp    _vaa_check_match

_vaa_load_algo:
        ; Load algorithm to SAM A-RAM
        mov     P2, VOICE_PAGE_HI       ; Set voice page

        ; Get program data pointer
        mov     a, VOICE_PAGE_LO
        add     a, #0x05
        mov     r0, a
        movx    a, @r0                  ; Read ptr high
        mov     PROG_PTR_LO, a
        inc     r0
        movx    a, @r0                  ; Read ptr low
        mov     PROG_PTR_HI, a

        ; Get source flag from offset 9
        mov     a, VOICE_PAGE_LO
        add     a, #0x09
        mov     r0, a
        movx    a, @r0
        mov     c, 0xE0                 ; Get bit 0 (source select)
        mov     BIT_SRC_XRAM, c         ; Store to bit 0x01

        ; Calculate A-RAM data pointer
        mov     a, PROG_PTR_HI
        add     a, #0x0A                ; Offset to A-RAM data
        mov     dpl, a
        mov     a, PROG_PTR_LO
        addc    a, #0x00
        mov     dph, a

        ; Handle XRAM vs CODE source
        jnb     BIT_SRC_XRAM, _vaa_src_code
        movx    a, @dptr                ; Read from XRAM
        sjmp    _vaa_got_ptr_lo
_vaa_src_code:
        clr     a
        movc    a, @a+dptr              ; Read from CODE
_vaa_got_ptr_lo:
        mov     B, a                    ; B = ptr low
        inc     dptr

        jnb     BIT_SRC_XRAM, _vaa_src_code2
        movx    a, @dptr                ; Read from XRAM
        sjmp    _vaa_got_ptr_hi
_vaa_src_code2:
        clr     a
        movc    a, @a+dptr              ; Read from CODE
_vaa_got_ptr_hi:
        mov     dph, a                  ; DPH = ptr high
        mov     dpl, B                  ; DPL = ptr low

        ; Calculate A-RAM starting address
        mov     a, VOICE_CHANNEL
        add     a, #ALG_MAP_BASE        ; Get alg slot for channel
        mov     r1, a
        mov     a, @r1                  ; A = slot number
        swap    a                       ; A = slot << 4
        rl      a                       ; A = slot << 5
        mov     r5, a                   ; R5 = A-RAM address

        lcall   _sam_aram_write         ; Write algorithm to A-RAM

        ret

;--------------------------------------------------------
; voice_init_slots - Initialize voice SAM slots
;
; Extracted from MS4 CODE:9A2D (131 bytes)
;
; Allocates and initializes SAM D-RAM slots for a voice.
; Writes initial control words to each allocated slot.
;
; INPUTS:
;   IRAM 0x34 = MIDI channel (VOICE_CHANNEL)
;   IRAM 0x35 = Voice page high (VOICE_PAGE_HI)
;   IRAM 0x36 = Voice page low (VOICE_PAGE_LO)
;   Bit 0x01 = Source select (0=CODE, 1=XRAM)
;
; OUTPUTS:
;   None
;
; MODIFIES:
;   A, R0, R2, R3, R4, R5, R6, R7, DPTR, B, IRAM 0x4A
;
; CALLS:
;   slot_allocate (FUN_CODE_a9cf), sam_dram_write
;--------------------------------------------------------
_voice_init_slots:
        ; Get slot count from program data
        mov     dph, VOICE_PAGE_HI
        mov     a, #0x09                ; Offset to flags byte
        add     a, VOICE_PAGE_LO
        mov     dpl, a

        jnb     BIT_SRC_XRAM, _vis_src_code
        movx    a, @dptr                ; Read from XRAM
        sjmp    _vis_got_flags
_vis_src_code:
        clr     a
        movc    a, @a+dptr              ; Read from CODE
_vis_got_flags:
        anl     a, #SAM_SLOT_MASK       ; Mask to slot count (bits 3:0)
        mov     VOICE_SLOT_COUNT, a     ; Save slot count
        mov     B, a                    ; B = loop counter

        ; Allocate slots
        lcall   _slot_allocate          ; FUN_CODE_a9cf - returns first slot in A
        mov     r7, a                   ; R7 = first allocated slot
        mov     P2, a                   ; P2 = slot (page select)

        ; Initialize slot control area
        mov     r0, #0xFB
        mov     a, #0x20
        movx    @r0, a                  ; Write 0x20 to 0xFB

        inc     r0                      ; R0 = 0xFC
        mov     a, VOICE_CHANNEL
        swap    a                       ; Channel in high nibble
        movx    @r0, a                  ; Write to 0xFC

        ; Mark slot 0x42 as unused
        mov     r0, #0x42
        mov     a, #0xFF
        movx    @r0, a

        ; Initialize slot params (0x70-0x73 = 0x0F, 0x74-0x7F = 0xFF)
        mov     r0, #0x70
        mov     r6, #0x04
        mov     a, #0x0F
_vis_init_0f:
        movx    @r0, a
        inc     r0
        djnz    r6, _vis_init_0f

        mov     r6, #0x0C
        mov     a, #0xFF
_vis_init_ff:
        movx    @r0, a
        inc     r0
        djnz    r6, _vis_init_ff

_vis_slot_loop:
        ; Write D-RAM control word for this slot
        mov     r2, #0x01               ; Data low = 1
        mov     r3, VOICE_CHANNEL       ; Data mid = channel
        mov     r4, #0x00               ; Data high = 0

        mov     a, r7                   ; A = slot number
        swap    a                       ; A = slot << 4
        orl     a, #SAM_DRAM_CTRL_WORD  ; A |= 0x0F (word 15)
        mov     r5, a                   ; R5 = D-RAM address

        mov     P2, #SAM_P2_SELECT      ; P2 = 0x80 (select SAM)
        lcall   _sam_dram_write         ; Write control word

        ; Get next slot from linked list
        mov     a, r7
        mov     P2, a                   ; Select current slot page
        add     a, #0x7E                ; Offset to next pointer
        mov     r0, a
        mov     a, @r0                  ; Read next slot
        mov     r7, a                   ; R7 = next slot

        djnz    B, _vis_slot_loop       ; Loop for all slots

        ; Copy envelope parameters
        mov     dph, VOICE_PAGE_HI
        mov     a, #0x16                ; Offset to envelope data
        add     a, VOICE_PAGE_LO
        mov     dpl, a
        jnc     _vis_no_carry
        inc     dph
_vis_no_carry:

        mov     P2, #0x11               ; Select page 0x11 for envelope
        mov     r0, #0xE6               ; Destination offset
        mov     B, #0x08                ; 8 bytes to copy

_vis_copy_env:
        jnb     BIT_SRC_XRAM, _vis_env_code
        movx    a, @dptr
        sjmp    _vis_env_got
_vis_env_code:
        clr     a
        movc    a, @a+dptr
_vis_env_got:
        movx    @r0, a                  ; Copy to page
        inc     dptr
        inc     r0
        mov     a, r0
        jnz     _vis_env_nowrap
        inc     P2                      ; Wrap to next page
_vis_env_nowrap:
        djnz    B, _vis_copy_env

        ret

;--------------------------------------------------------
; End of module
;--------------------------------------------------------

