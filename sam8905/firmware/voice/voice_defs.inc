;--------------------------------------------------------
; MS4 Voice Management Definitions
;
; IRAM addresses used by voice management functions
; Extracted from MS4 firmware (ms4_05_r1_0.bin)
;
; For use with SDAS8051 assembler (SDCC toolchain)
;--------------------------------------------------------

;--------------------------------------------------------
; IRAM Addresses - Voice Table Pointers
;--------------------------------------------------------

VOICE_TABLE_HI      .equ    0x27    ; Voice table high byte (DPH)
VOICE_TABLE_LO      .equ    0x28    ; Voice table low byte (DPL)
VOICE_COUNT         .equ    0x29    ; Number of voices to search

;--------------------------------------------------------
; IRAM Addresses - Voice State
;--------------------------------------------------------

VOICE_CHANNEL       .equ    0x34    ; Current MIDI channel (0-15)
VOICE_PAGE_HI       .equ    0x35    ; Voice page high byte
VOICE_PAGE_LO       .equ    0x36    ; Voice page low byte
VOICE_CTRL_SHADOW   .equ    0x37    ; SAM control register shadow
VOICE_SLOT_ADDR     .equ    0x38    ; Current slot address (slot << 4)
VOICE_P2_SAVE       .equ    0x3A    ; Saved P2 value

;--------------------------------------------------------
; IRAM Addresses - Program Data Pointers
;--------------------------------------------------------

PROG_PTR_HI         .equ    0x41    ; Program data pointer high
PROG_PTR_LO         .equ    0x42    ; Program data pointer low
VOICE_SLOT_COUNT    .equ    0x4A    ; Number of SAM slots for voice

;--------------------------------------------------------
; IRAM Addresses - Voice Linked List
;--------------------------------------------------------

VOICE_LIST_HEAD     .equ    0x54    ; First voice page (0xFF = empty)
; Pages are at 0x00-0x0F (16 pages)
; Each page has next pointer at offset 0x7E

;--------------------------------------------------------
; IRAM Addresses - Loop Counters (temporary)
;--------------------------------------------------------

LOOP_CNT_HI         .equ    0x08    ; 16-bit loop counter high
LOOP_CNT_LO         .equ    0x09    ; 16-bit loop counter low

;--------------------------------------------------------
; Voice Page Offsets (accessed via P2 as page select)
;--------------------------------------------------------

VPAGE_FLAGS         .equ    0x00    ; Voice flags byte
VPAGE_SLOT_INFO     .equ    0x02    ; Slot assignment info
VPAGE_NEXT          .equ    0x7E    ; Next voice page in list
VPAGE_SLOT_BASE     .equ    0x80    ; Slot data base offset

;--------------------------------------------------------
; Voice Page Flags (at offset 0x00)
;--------------------------------------------------------

VFLAG_ACTIVE        .equ    0x20    ; Voice is active
VFLAG_SUSTAIN       .equ    0x10    ; Voice is sustained
; Bits 7,6,4 preserved on deactivate (AND #0xD0)

;--------------------------------------------------------
; XRAM Addresses - Voice Tables
;--------------------------------------------------------

VOICE_PAGES_1       .equ    0x14D5  ; First voice pages region (0x800 bytes)
VOICE_PAGES_2       .equ    0x1CD5  ; Second voice pages region (16 bytes)

;--------------------------------------------------------
; Algorithm Table
;--------------------------------------------------------

ALG_TABLE_BASE      .equ    0x11EE  ; Algorithm assignment table (XRAM)
ALG_TABLE_SIZE      .equ    8       ; Max 8 unique algorithms
ALG_MAP_BASE        .equ    0x8E    ; IRAM offset for alg->slot map
ALG_SLOT_BASE       .equ    0x9E    ; IRAM offset for slot->alg map

;--------------------------------------------------------
; SAM Slot Constants
;--------------------------------------------------------

SAM_SLOT_MASK       .equ    0x0F    ; Slot count mask (bits 3:0)
SAM_MAX_SLOTS       .equ    16      ; Maximum SAM slots

;--------------------------------------------------------
; Bit Addresses (20h-2Fh byte addressable)
;--------------------------------------------------------

BIT_ALG_NEW         .equ    0x0A    ; New algorithm flag (byte 0x21, bit 2)
BIT_SRC_XRAM        .equ    0x01    ; Source is XRAM (byte 0x20, bit 1)

