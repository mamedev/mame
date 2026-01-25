;--------------------------------------------------------
; SAM8905 Hardware Definitions
;
; Extracted from MS4 firmware (ms4_05_r1_0.bin)
; For use with SDAS8051 assembler (SDCC toolchain)
;
; SAM8905 is memory-mapped at 0x8000 when P2=0x80
;--------------------------------------------------------

;--------------------------------------------------------
; SAM8905 Register Addresses (accent indirect via R0)
;
; Access pattern:
;   MOV P2, #0x80       ; Set high address byte
;   MOV R0, #SAM_ADDR   ; Register offset
;   MOVX @R0, A         ; Write to SAM
;   MOVX A, @R0         ; Read from SAM
;--------------------------------------------------------

SAM_ADDR        .equ    0x00    ; Address register (8-bit)
SAM_DATA_LO     .equ    0x01    ; Data low byte [7:0]
SAM_DATA_MID    .equ    0x02    ; Data mid byte [15:8]
SAM_DATA_HI     .equ    0x03    ; Data high byte [18:16] (3 bits for D-RAM, 7 bits for A-RAM)
SAM_CTRL        .equ    0x04    ; Control register

;--------------------------------------------------------
; SAM8905 Control Register Bits
;--------------------------------------------------------

SAM_CTRL_WE     .equ    0x01    ; Write Enable (active high)
SAM_CTRL_ARAM   .equ    0x02    ; A-RAM select (1=A-RAM, 0=D-RAM)
SAM_CTRL_DRAM   .equ    0x00    ; D-RAM select (explicit zero)
SAM_CTRL_SSR    .equ    0x08    ; Sample Rate Select (0=44.1kHz, 1=22.05kHz)

;--------------------------------------------------------
; SAM8905 Memory Layout
;--------------------------------------------------------

; D-RAM: 256 x 19-bit words (16 slots x 16 words)
; Address = (slot << 4) | word_offset
; Word 15 of each slot is the control word

SAM_DRAM_SLOTS      .equ    16      ; Number of D-RAM slots
SAM_DRAM_WORDS      .equ    16      ; Words per slot
SAM_DRAM_TOTAL      .equ    256     ; Total D-RAM words

SAM_DRAM_CTRL_WORD  .equ    0x0F    ; Control word offset within slot

; A-RAM: 256 x 15-bit words (8 algorithms x 32 instructions)
; Address = (algorithm << 5) | pc

SAM_ARAM_ALGS       .equ    8       ; Number of algorithm slots
SAM_ARAM_INSTS      .equ    32      ; Instructions per algorithm
SAM_ARAM_TOTAL      .equ    256     ; Total A-RAM words

;--------------------------------------------------------
; D-RAM Control Word (Word 15) Bit Layout
;
; Bits [18:12] = X (user-defined, 7 bits)
; Bit  [11]    = IDLE (1=slot inactive, no WACC output)
; Bits [10:8]  = ALG (algorithm select, 0-7)
; Bit  [7]     = M (interrupt mask)
; Bits [6:0]   = unused
;--------------------------------------------------------

SAM_IDLE_BIT        .equ    0x08    ; IDLE bit in high byte (bit 11 = byte2 bit 3)
SAM_ALG_MASK        .equ    0x07    ; ALG field mask in high byte (bits 10:8 = byte2 bits 2:0)

;--------------------------------------------------------
; Port 2 Values for SAM Access
;--------------------------------------------------------

SAM_P2_SELECT       .equ    0x80    ; P2 value to select SAM (A15=1)

;--------------------------------------------------------
; Internal RAM Locations (from MS4 firmware)
;
; These may need adjustment for new projects
;--------------------------------------------------------

SAM_CTRL_SHADOW     .equ    0x37    ; Shadow of SAM control register
SAM_P2_SAVE         .equ    0x3A    ; Saved P2 value for restore

;--------------------------------------------------------
; Bit Addresses (for JNB/JB/SETB/CLR instructions)
;--------------------------------------------------------

BIT_ARAM_SRC_XRAM   .equ    0x01    ; 1=source from XRAM, 0=source from CODE
BIT_ARAM_SRC_SAVE   .equ    0x02    ; Saved source select bit
