008A8E: 0C39 000F 0010 8F82 cmpi.b  #$f, $108f82.l
008A96: 661C                bne     $8ab4
008A98: 33FC FFFF 0010 0EC8 move.w  #$ffff, $100ec8.l
008AA0: 33FC 0001 0010 0ECA move.w  #$1, $100eca.l
008AA8: 0239 00F3 0010 08D2 andi.b  #$f3, $1008d2.l
008AB0: 6000 0138           bra     $8bea
008AB4: 0C79 0009 0010 0EC8 cmpi.w  #$9, $100ec8.l
008ABC: 6D0C                blt     $8aca
008ABE: 0239 00F3 0010 08D2 andi.b  #$f3, $1008d2.l
008AC6: 6000 0122           bra     $8bea
008ACA: 0839 0007 0010 0EC0 btst    #$7, $100ec0.l
008AD2: 6710                beq     $8ae4
008AD4: 5279 0010 0EC4      addq.w  #1, $100ec4.l
008ADA: 5279 0010 0ED4      addq.w  #1, $100ed4.l
008AE0: 6000 000A           bra     $8aec
008AE4: 0039 0004 0010 08D2 ori.b   #$4, $1008d2.l
008AEC: 0839 0007 0010 0EC1 btst    #$7, $100ec1.l
008AF4: 6710                beq     $8b06
008AF6: 5279 0010 0EC6      addq.w  #1, $100ec6.l
008AFC: 5279 0010 0ED6      addq.w  #1, $100ed6.l
008B02: 6000 000A           bra     $8b0e
008B06: 0039 0008 0010 08D2 ori.b   #$8, $1008d2.l
008B0E: 41FA 00DC           lea     ($8bec,PC), A0
008B12: 7200                moveq   #$0, D1
008B14: 1239 0010 8F82      move.b  $108f82.l, D1
008B1A: E549                lsl.w   #2, D1
008B1C: 0C79 0009 0010 0EC8 cmpi.w  #$9, $100ec8.l
008B24: 6D0C                blt     $8b32
008B26: 0239 00F3 0010 08D2 andi.b  #$f3, $1008d2.l
008B2E: 6000 00BA           bra     $8bea
008B32: 7000                moveq   #$0, D0
008B34: 1030 1800           move.b  (A0,D1.l), D0
008B38: B079 0010 0EC4      cmp.w   $100ec4.l, D0
008B3E: 661E                bne     $8b5e
008B40: 9179 0010 0EC4      sub.w   D0, $100ec4.l
008B46: 1030 1801           move.b  ($1,A0,D1.l), D0
008B4A: D179 0010 0EC8      add.w   D0, $100ec8.l
008B50: D179 0010 0ECA      add.w   D0, $100eca.l
008B56: 33FC 0001 0010 C330 move.w  #$1, $10c330.l
008B5E: 0C79 0009 0010 0EC8 cmpi.w  #$9, $100ec8.l
008B66: 6D0C                blt     $8b74
008B68: 0239 00F3 0010 08D2 andi.b  #$f3, $1008d2.l
008B70: 6000 0078           bra     $8bea
008B74: 7000                moveq   #$0, D0
008B76: 1030 1802           move.b  ($2,A0,D1.l), D0
008B7A: B079 0010 0EC6      cmp.w   $100ec6.l, D0
008B80: 661E                bne     $8ba0
008B82: 9179 0010 0EC6      sub.w   D0, $100ec6.l
008B88: 1030 1803           move.b  ($3,A0,D1.l), D0
008B8C: D179 0010 0EC8      add.w   D0, $100ec8.l
008B92: D179 0010 0ECA      add.w   D0, $100eca.l
008B98: 33FC 0001 0010 C330 move.w  #$1, $10c330.l
008BA0: 0C79 0009 0010 0EC8 cmpi.w  #$9, $100ec8.l
008BA8: 6D0C                blt     $8bb6
008BAA: 0239 00F3 0010 08D2 andi.b  #$f3, $1008d2.l
008BB2: 6000 0036           bra     $8bea
008BB6: 0839 0007 0010 0EC3 btst    #$7, $100ec3.l
008BBE: 672A                beq     $8bea
008BC0: 5279 0010 0EC8      addq.w  #1, $100ec8.l
008BC6: 48E7 8080           movem.l D0/A0, -(A7)
008BCA: 41F9 0010 C330      lea     $10c330.l, A0
008BD0: 7000                moveq   #$0, D0
008BD2: 1028 0003           move.b  ($3,A0), D0
008BD6: 11BC 0001 0004      move.b  #$1, ($4,A0,D0.w)
008BDC: 5200                addq.b  #1, D0
008BDE: 0200 007F           andi.b  #$7f, D0
008BE2: 1140 0003           move.b  D0, ($3,A0)
008BE6: 4CDF 0101           movem.l (A7)+, D0/A0
008BEA: 4E75                rts
008BEC: 0101                btst    D0, D1
008BEE: 0101                btst    D0, D1
008BF0: 0201 0201           andi.b  #$1, D1
008BF4: 0301                btst    D1, D1
008BF6: 0301                btst    D1, D1
008BF8: 0401 0401           subi.b  #$1, D1
008BFC: 0501                btst    D2, D1
008BFE: 0501                btst    D2, D1
008C00: 0201 0101           andi.b  #$1, D1
008C04: 0301                btst    D1, D1
008C06: 0101                btst    D0, D1
008C08: 0401 0101           subi.b  #$1, D1
008C0C: 0501                btst    D2, D1
008C0E: 0101                btst    D0, D1
008C10: 0201 0201           andi.b  #$1, D1
008C14: 0301                btst    D1, D1
008C16: 0201 0401           andi.b  #$1, D1
008C1A: 0201 0501           andi.b  #$1, D1
008C1E: 0201 0102           andi.b  #$2, D1
008C22: 0102                btst    D0, D2
008C24: 0103                btst    D0, D3
008C26: 0103                btst    D0, D3
008C28: FFFF                dc.w    $ffff; opcode 1111
008C2A: FFFF                dc.w    $ffff; opcode 1111
008C2C: 4279 0010 0ED4      clr.w   $100ed4.l
008C32: 4279 0010 0ED6      clr.w   $100ed6.l
008C38: 0239 00FE 0010 08D2 andi.b  #$fe, $1008d2.l
008C40: 4A79 0010 0ED4      tst.w   $100ed4.l
008C46: 673C                beq     $8c84
008C48: 5379 0010 0ED4      subq.w  #1, $100ed4.l
008C4E: 0039 0001 0010 08D2 ori.b   #$1, $1008d2.l
008C56: 3E3C 000E           move.w  #$e, D7
008C5A: 2C79 0010 010C      movea.l $10010c.l, A6
008C60: 3039 0010 0116      move.w  $100116.l, D0
008C66: 41FA 0014           lea     ($8c7c,PC), A0
008C6A: 2D88 0004           move.l  A0, ($4,A6,D0.w)
008C6E: 0247 0FFF           andi.w  #$fff, D7
008C72: 0047 5000           ori.w   #$5000, D7
008C76: 3D87 0000           move.w  D7, (A6,D0.w)
008C7A: 4E75                rts
008C7C: 0239 00FE 0010 08D2 andi.b  #$fe, $1008d2.l
008C84: 0239 00FD 0010 08D2 andi.b  #$fd, $1008d2.l
008C8C: 4A79 0010 0ED6      tst.w   $100ed6.l
