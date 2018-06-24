 ; license:LGPL-2.1+
 ; copyright-holders:Angelo Salese
 ;*****************************************************************
 ; Jaleco Mahjong code simulation snippets for 1st MCU games
 ;
 ; Games using this:
 ; Urashima Mahjong
 ; Mahjong Daireikai
 ; Mahjong Channel Zoom-In
 ;
 ; For details @see jalmah.cpp
 ;
 ;
START:
	ORG    $0                       ; palette code snippet (test mode & gameplay)
	move.w #$1, $80016.l            ; set priority number (for urashima)
	move.w #$f, D0                  ; set loop 1
FetchNewPalette:
	move.l A0, $100400.l            ; save address to our local buffer (address currently points to an index of actual palette to transfer)
	movea.l (A0), A0                ; read ROM location for the actual palette source
	move.w #$f, D1                  ; set loop 2 variable
	bsr    UploadPaletteChunk       ; jump to uploading routine
	movea.l $100400.l, A0           ; restore address index, increment by next address
	adda.w #$4, A0
	dbra  D0, FetchNewPalette
	rts

UploadPaletteChunk:
	move.w (A0)+, (A1)+             ; copy palette source in ROM to palette RAM
	dbra  D1, UploadPaletteChunk
	rts

	ORG    $0800                    ; sound code snippet
	movem.l D1, -(A7)
	move.w D0, D1                   ; D0 is raw sound command, low byte is command number, everything else is banking and probably voice channel (unhandled)
	ror.w  #$8, D1
	and.w  #$f, D1                  ; take D1 as raw banking number
	bsr    CheckSoundZA
	bsr    CheckSoundBank
	move.w #$8, $80040.l            ; clear channel
	bsr    SoundStatus
	andi.w #$3f, D0                 ; take command number and write it to OKI port
	ori.w  #$80, D0
	move.w D0, $80040.l
	bsr    SoundStatus
	move.w #$10, $80040.l           ; play channel
	movem.l (A7)+, D1
	rts
SoundStatus:
	movem.l D0, -(A7)
ReadStatus:
	move.w $80040.l, D0             ; check OKI voice status
	andi.w #$1,D0
	bne  ReadStatus
	movem.l (A7)+, D0
	rts
; guess: special bits set in register D0 affect banking (oki can play up to 64 samples so bits 6 and 7 cannot be used)
; we simulate "ZA" writing but use the other bank register (jalmah can do both, cfr. ports $80018 and $8001a
CheckSoundZA:
	btst  #$6, D0                    ; urashima BGM at start of game
	beq   NoZA1
	ror.w #$1, D1                    ; shift one down
	rts
NoZA1:
	btst  #$07, D0                   ; mjzoomin BGM at start of game
	beq   NoZA2
	move.w #$8, D1                   ; hardcode to bank 3
NoZA2:
	rts
CheckSoundBank:
	cmpi.w #$1, D1                   ; verify our raw D1 and set base bank accordingly
	beq   SetBank0
	cmpi.w #$2, D1
	beq   SetBank1
	cmpi.w #$4, D1
	beq   SetBank2
	move.w #$3, $80018.l
	rts
SetBank0:
	move.w #$0, $80018.l
	rts
SetBank1:
	move.w #$1, $80018.l
	rts
SetBank2:
	move.w #$2, $80018.l
	rts

	ORG    $1000                    ; tile upload snippet
TileUploadStart:
	move.w (A2)+, (A1)+             ; just take A2 source (tiles) to A1 destination VRAM
	dbra   D0, TileUploadStart
	rts

	ORG    $1800                    ; (another) palette code snippet
	movem.l D0-D7/A1-A6, -(A7)      ; save stack (A0 intentionally updated by this snippet)
	move.w #$f, D0                  ; what's the difference here? Same as above but no hardcoded priority?
	bsr   FetchNewPalette
	movem.l (A7)+, D0-D7/A1-A6
	rts

	ORG    $2000                    ; daireikai RNG snippet (guess)
	move.w D5, $100400.l
	move.w $f000c.l, D5             ; take current RNG number
	add.l  D6, D5                   ; add D6
	andi.w #$3f, D5
	move.w D5, $f000c.l             ; round it
	move.w $100400.l, D5
	rts
	ORG    $2800                    ; daireikai layer clearances
	movem.l D0-D7/A0-A6, -(A7)
	lea    $90000.l, A6             ; layer 0
	move.w #$1fff, D1
	move.w #$00ff, D0
	bsr    ClearLayer
	movem.l (A7)+, D0-D7/A0-A6
	rts
ClearLayer:
	move.w D0, (A6)+
	dbra   D1, ClearLayer
	rts
	ORG    $2880
	movem.l D0-D7/A0-A6, -(A7)
	lea    $94000.l, A6              ; layer 1
	move.w #$1fff, D1
	move.w #$00ff, D0
	bsr ClearLayer
	movem.l (A7)+, D0-D7/A0-A6
	rts
	ORG    $2900
	movem.l D0-D7/A0-A6, -(A7)
	lea    $98000.l, A6              ; layer 2
	move.w #$1fff, D1
	move.w #$f0ff, D0
	bsr ClearLayer
	movem.l (A7)+, D0-D7/A0-A6
	rts

	ORG    $fffe           ; extend binary ROM to 0x10000 in size
	dc.w   $ffff
	END START


*~Font name~Courier New~
*~Font size~10~
*~Tab type~1~
*~Tab size~4~
