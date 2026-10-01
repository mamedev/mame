010BD0: FF70                dc.w    $ff70; opcode 1111
010BD2: 3039 0010 B77C      move.w  $10b77c.l, D0
010BD8: B079 0010 B764      cmp.w   $10b764.l, D0
010BDE: 665C                bne     $10c3c
010BE0: 0C79 0001 0010 B764 cmpi.w  #$1, $10b764.l
010BE8: 660C                bne     $10bf6
010BEA: 33FC 000C 0010 B764 move.w  #$c, $10b764.l
010BF2: 6000 0040           bra     $10c34
010BF6: 0C79 000C 0010 B764 cmpi.w  #$c, $10b764.l
010BFE: 660C                bne     $10c0c
010C00: 33FC 0180 0010 B764 move.w  #$180, $10b764.l
010C08: 6000 002A           bra     $10c34
010C0C: 0C79 0180 0010 B764 cmpi.w  #$180, $10b764.l
010C14: 660C                bne     $10c22
010C16: 33FC 3000 0010 B764 move.w  #$3000, $10b764.l
010C1E: 6000 0014           bra     $10c34
010C22: 0C79 3000 0010 B764 cmpi.w  #$3000, $10b764.l
010C2A: 6610                bne     $10c3c
010C2C: 33FC 000C 0010 B764 move.w  #$c, $10b764.l
010C34: 33FC 0106 0010 B76C move.w  #$106, $10b76c.l
010C3C: 7E04                moveq   #$4, D7
010C3E: 2C79 0010 94A8      movea.l $1094a8.l, A6
010C44: 3039 0010 94B2      move.w  $1094b2.l, D0
010C4A: 41FA 0014           lea     ($10c60,PC), A0
010C4E: 2D88 0004           move.l  A0, ($4,A6,D0.w)
010C52: 0247 0FFF           andi.w  #$fff, D7
010C56: 0047 5000           ori.w   #$5000, D7
010C5A: 3D87 0000           move.w  D7, (A6,D0.w)
010C5E: 4E75                rts
010C60: 6000 FF70           bra     $10bd2
010C64: 48E7 8000           movem.l D0, -(A7)
010C68: 7000                moveq   #$0, D0
010C6A: 1010                move.b  (A0), D0
010C6C: E948                lsl.w   #4, D0
010C6E: 0640 0030           addi.w  #$30, D0
010C72: 4840                swap    D0
010C74: 0828 0000 0001      btst    #$0, ($1,A0)
010C7A: 6704                beq     $10c80
010C7C: 0640 0200           addi.w  #$200, D0
010C80: 2040                movea.l D0, A0
010C82: 4CDF 0001           movem.l (A7)+, D0
010C86: 4E75                rts
010C88: FFFF                dc.w    $ffff; opcode 1111
010C8A: FFFF                dc.w    $ffff; opcode 1111
010C8C: FFFF                dc.w    $ffff; opcode 1111
010C8E: FFFF                dc.w    $ffff; opcode 1111
010C90: FFFF                dc.w    $ffff; opcode 1111
010C92: FFFF                dc.w    $ffff; opcode 1111
010C94: FFFF                dc.w    $ffff; opcode 1111
010C96: FFFF                dc.w    $ffff; opcode 1111
010C98: FFFF                dc.w    $ffff; opcode 1111
010C9A: FFFF                dc.w    $ffff; opcode 1111
010C9C: FFFF                dc.w    $ffff; opcode 1111
010C9E: FFFF                dc.w    $ffff; opcode 1111
010CA0: FFFF                dc.w    $ffff; opcode 1111
010CA2: FFFF                dc.w    $ffff; opcode 1111
010CA4: FFFF                dc.w    $ffff; opcode 1111
010CA6: FFFF                dc.w    $ffff; opcode 1111
010CA8: FFFF                dc.w    $ffff; opcode 1111
010CAA: FFFF                dc.w    $ffff; opcode 1111
010CAC: FFFF                dc.w    $ffff; opcode 1111
010CAE: FFFF                dc.w    $ffff; opcode 1111
010CB0: FFFF                dc.w    $ffff; opcode 1111
010CB2: FFFF                dc.w    $ffff; opcode 1111
010CB4: FFFF                dc.w    $ffff; opcode 1111
010CB6: FFFF                dc.w    $ffff; opcode 1111
010CB8: FFFF                dc.w    $ffff; opcode 1111
010CBA: FFFF                dc.w    $ffff; opcode 1111
010CBC: FFFF                dc.w    $ffff; opcode 1111
010CBE: FFFF                dc.w    $ffff; opcode 1111
010CC0: FFFF                dc.w    $ffff; opcode 1111
010CC2: FFFF                dc.w    $ffff; opcode 1111
010CC4: FFFF                dc.w    $ffff; opcode 1111
010CC6: FFFF                dc.w    $ffff; opcode 1111
010CC8: FFFF                dc.w    $ffff; opcode 1111
010CCA: FFFF                dc.w    $ffff; opcode 1111
010CCC: FFFF                dc.w    $ffff; opcode 1111
010CCE: FFFF                dc.w    $ffff; opcode 1111
