00BC40: 00C0           dc.w    $00c0; ILLEGAL
00BC42: 0000 4E75      ori.b   #$75, D0
00BC46: 6000 FEA8      bra     $baf0
00BC4A: 286E 0008      movea.l ($8,A6), A4
00BC4E: 2A6E 000C      movea.l ($c,A6), A5
00BC52: 2054           movea.l (A4), A0
00BC54: 226C 0004      movea.l ($4,A4), A1
00BC58: 2449           movea.l A1, A2
00BC5A: 363C 00AA      move.w  #$aa, D3
00BC5E: 3C2C 000A      move.w  ($a,A4), D6
00BC62: 5246           addq.w  #1, D6
00BC64: E24E           lsr.w   #1, D6
00BC66: 6500 0008      bcs     $bc70
00BC6A: 5346           subq.w  #1, D6
00BC6C: 0043 8000      ori.w   #$8000, D3
00BC70: 3A06           move.w  D6, D5
00BC72: 3E2C 0008      move.w  ($8,A4), D7
00BC76: 383C 03FF      move.w  #$3ff, D4
00BC7A: 7000           moveq   #$0, D0
00BC7C: 1018           move.b  (A0)+, D0
00BC7E: 0880 0007      bclr    #$7, D0
00BC82: 6600 0068      bne     $bcec
00BC86: 14D8           move.b  (A0)+, (A2)+
00BC88: 14D8           move.b  (A0)+, (A2)+
00BC8A: 548A           addq.l  #2, A2
00BC8C: 51CD 0022      dbra    D5, $bcb0
00BC90: 43E9 0400      lea     ($400,A1), A1
00BC94: 2449           movea.l A1, A2
00BC96: 3A06           move.w  D6, D5
00BC98: E21B           ror.b   #1, D3
00BC9A: 6500 000C      bcs     $bca8
00BC9E: 548A           addq.l  #2, A2
00BCA0: 4A43           tst.w   D3
00BCA2: 6B00 0004      bmi     $bca8
00BCA6: 5345           subq.w  #1, D5
00BCA8: 51CF 0006      dbra    D7, $bcb0
00BCAC: 6000 00AA      bra     $bd58
00BCB0: 51CC 0034      dbra    D4, $bce6
00BCB4: 48D5 17FF      movem.l D0-D7/A0-A2/A4, (A5)
00BCB8: 2C79 0010 94A8 movea.l $1094a8.l, A6
00BCBE: 3039 0010 94B2 move.w  $1094b2.l, D0
