00B9B0: 3E2C 0008      move.w  ($8,A4), D7
00B9B4: 5247           addq.w  #1, D7
00B9B6: DE47           add.w   D7, D7
00B9B8: 5347           subq.w  #1, D7
00B9BA: 363C 00AA      move.w  #$aa, D3
00B9BE: 383C 03FF      move.w  #$3ff, D4
00B9C2: 7000           moveq   #$0, D0
00B9C4: 1018           move.b  (A0)+, D0
00B9C6: 0880 0007      bclr    #$7, D0
00B9CA: 6600 0084      bne     $ba50
00B9CE: 4A43           tst.w   D3
00B9D0: 6A00 000C      bpl     $b9de
00B9D4: 5488           addq.l  #2, A0
00B9D6: 51CD 003A      dbra    D5, $ba12
00B9DA: 6000 001A      bra     $b9f6
00B9DE: E21B           ror.b   #1, D3
00B9E0: 6400 000C      bcc     $b9ee
00B9E4: 5488           addq.l  #2, A0
00B9E6: 51CD 002A      dbra    D5, $ba12
00B9EA: 6000 000A      bra     $b9f6
00B9EE: 14D8           move.b  (A0)+, (A2)+
00B9F0: 14D8           move.b  (A0)+, (A2)+
00B9F2: 51CD 001E      dbra    D5, $ba12
00B9F6: 0A43 8000      eori.w  #$8000, D3
00B9FA: 6B00 0006      bmi     $ba02
00B9FE: 43E9 0400      lea     ($400,A1), A1
00BA02: 2449           movea.l A1, A2
00BA04: 3A06           move.w  D6, D5
00BA06: 163C 00AA      move.b  #$aa, D3
00BA0A: 51CF 0006      dbra    D7, $ba12
00BA0E: 6000 00BA      bra     $baca
00BA12: 51CC 0034      dbra    D4, $ba48
00BA16: 48D5 07FF      movem.l D0-D7/A0-A2, (A5)
00BA1A: 2C79 0010 94A8 movea.l $1094a8.l, A6
00BA20: 3039 0010 94B2 move.w  $1094b2.l, D0
00BA26: 41FA 0014      lea     ($ba3c,PC), A0
00BA2A: 2D88 0004      move.l  A0, ($4,A6,D0.w)
00BA2E: 0236 007F 0000 andi.b  #$7f, (A6,D0.w)
00BA34: 0036 0040 0000 ori.b   #$40, (A6,D0.w)
00BA3A: 4E75           rts
00BA3C: 2A6E 000C      movea.l ($c,A6), A5
00BA40: 4CD5 07FF      movem.l (A5), D0-D7/A0-A2
00BA44: 383C 03FF      move.w  #$3ff, D4
00BA48: 51C8 FF84      dbra    D0, $b9ce
00BA4C: 6000 FF74      bra     $b9c2
00BA50: 1218           move.b  (A0)+, D1
00BA52: 1418           move.b  (A0)+, D2
00BA54: 4A43           tst.w   D3
00BA56: 6A00 000A      bpl     $ba62
00BA5A: 51CD 0030      dbra    D5, $ba8c
00BA5E: 6000 0010      bra     $ba70
00BA62: E21B           ror.b   #1, D3
00BA64: 6500 0006      bcs     $ba6c
00BA68: 14C1           move.b  D1, (A2)+
00BA6A: 14C2           move.b  D2, (A2)+
00BA6C: 51CD 001E      dbra    D5, $ba8c
00BA70: 0A43 8000      eori.w  #$8000, D3
00BA74: 6B00 0006      bmi     $ba7c
00BA78: 43E9 0400      lea     ($400,A1), A1
00BA7C: 2449           movea.l A1, A2
00BA7E: 3A06           move.w  D6, D5
00BA80: 163C 00AA      move.b  #$aa, D3
00BA84: 51CF 0006      dbra    D7, $ba8c
00BA88: 6000 0040      bra     $baca
00BA8C: 51CC 0034      dbra    D4, $bac2
00BA90: 48D5 07FF      movem.l D0-D7/A0-A2, (A5)
00BA94: 2C79 0010 94A8 movea.l $1094a8.l, A6
00BA9A: 3039 0010 94B2 move.w  $1094b2.l, D0
00BAA0: 41FA 0014      lea     ($bab6,PC), A0
00BAA4: 2D88 0004      move.l  A0, ($4,A6,D0.w)
00BAA8: 0236 007F 0000 andi.b  #$7f, (A6,D0.w)
00BAAE: 0036 0040 0000 ori.b   #$40, (A6,D0.w)
