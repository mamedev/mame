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
    move.w #$f, D0
FetchNewPalette:
    move.l A0, $100400.l
    movea.l (A0), A0
    move.w #$f, D1
    bsr    UploadPaletteChunk
    movea.l $100400.l, A0
    adda.w #$4, A0
    dbra  D0, FetchNewPalette
    rts

UploadPaletteChunk:
    move.w (A0)+, (A1)+
    dbra  D1, UploadPaletteChunk
    rts

    ORG    $0800                    ; sound code snippet
    ori.w  #$80, D0
    move.w D0, $80040.l
    move.w #$10, $80040.l
    rts

    ORG    $1000                    ; tile upload snippet
TileUploadStart:
    move.w (A2)+, (A1)+
    dbra   D0, TileUploadStart
    rts

    ORG    $1800                    ; (another) palette code snippet
    movem.l D0-D7/A1-A6, -(A7)      ; save stack (A0 intentionally updated by this snippet)
    move.w #$f, D0
    bsr   FetchNewPalette
    movem.l (A7)+, D0-D7/A1-A6
    rts

    ORG    $2000                    ; daireikai RNG snippet (guess)
    move.w D5, $100400.l
    move.w $f000c.l, D5
    add.l  D6, D5
    andi.w #$3f, D5
    move.w D5, $f000c.l
    move.w $100400.l, D5
    rts
    ORG    $2800                    ; daireikai layer clearances
    movem.l D0-D7/A0-A6, -(A7)
    lea    $90000.l, A6
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
    lea    $94000.l, A6
    move.w #$1fff, D1
    move.w #$00ff, D0
    bsr ClearLayer
    movem.l (A7)+, D0-D7/A0-A6
    rts
    ORG    $2900
    movem.l D0-D7/A0-A6, -(A7)
    lea    $98000.l, A6
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
