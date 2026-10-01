00B980: 0010 94B8      ori.b   #$b8, (A0)
00B984: 0284 007F FFFF andi.l  #$7fffff, D4
00B98A: 2044           movea.l D4, A0
00B98C: D1FC 0080 0000 adda.l  #$800000, A0
00B992: 4E75           rts
00B994: 286E 0008      movea.l ($8,A6), A4
00B998: 2A6E 000C      movea.l ($c,A6), A5
00B99C: 2054           movea.l (A4), A0
00B99E: 226C 0004      movea.l ($4,A4), A1
00B9A2: 2449           movea.l A1, A2
00B9A4: 3C2C 000A      move.w  ($a,A4), D6
00B9A8: 5246           addq.w  #1, D6
00B9AA: DC46           add.w   D6, D6
00B9AC: 5346           subq.w  #1, D6
00B9AE: 3A06           move.w  D6, D5
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
