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
00BCC4: 41FA 0014      lea     ($bcda,PC), A0
00BCC8: 2D88 0004      move.l  A0, ($4,A6,D0.w)
00BCCC: 0236 007F 0000 andi.b  #$7f, (A6,D0.w)
00BCD2: 0036 0040 0000 ori.b   #$40, (A6,D0.w)
00BCD8: 4E75           rts
00BCDA: 2A6E 000C      movea.l ($c,A6), A5
00BCDE: 4CD5 17FF      movem.l (A5), D0-D7/A0-A2/A4
00BCE2: 383C 03FF      move.w  #$3ff, D4
00BCE6: 51C8 FF9E      dbra    D0, $bc86
00BCEA: 608E           bra     $bc7a
00BCEC: 1218           move.b  (A0)+, D1
00BCEE: 1418           move.b  (A0)+, D2
00BCF0: 14C1           move.b  D1, (A2)+
00BCF2: 14C2           move.b  D2, (A2)+
00BCF4: 548A           addq.l  #2, A2
00BCF6: 51CD 0022      dbra    D5, $bd1a
00BCFA: 43E9 0400      lea     ($400,A1), A1
00BCFE: 2449           movea.l A1, A2
00BD00: 3A06           move.w  D6, D5
00BD02: E21B           ror.b   #1, D3
00BD04: 6500 000C      bcs     $bd12
00BD08: 548A           addq.l  #2, A2
00BD0A: 4A43           tst.w   D3
00BD0C: 6B00 0004      bmi     $bd12
00BD10: 5345           subq.w  #1, D5
00BD12: 51CF 0006      dbra    D7, $bd1a
00BD16: 6000 0040      bra     $bd58
00BD1A: 51CC 0034      dbra    D4, $bd50
00BD1E: 48D5 17FF      movem.l D0-D7/A0-A2/A4, (A5)
00BD22: 2C79 0010 94A8 movea.l $1094a8.l, A6
00BD28: 3039 0010 94B2 move.w  $1094b2.l, D0
00BD2E: 41FA 0014      lea     ($bd44,PC), A0
00BD32: 2D88 0004      move.l  A0, ($4,A6,D0.w)
00BD36: 0236 007F 0000 andi.b  #$7f, (A6,D0.w)
00BD3C: 0036 0040 0000 ori.b   #$40, (A6,D0.w)
00BD42: 4E75           rts
00BD44: 2A6E 000C      movea.l ($c,A6), A5
00BD48: 4CD5 17FF      movem.l (A5), D0-D7/A0-A2/A4
00BD4C: 383C 03FF      move.w  #$3ff, D4
00BD50: 51C8 FF9E      dbra    D0, $bcf0
00BD54: 6000 FF24      bra     $bc7a
00BD58: 183C 00AA      move.b  #$aa, D4
00BD5C: 3A3C 03FF      move.w  #$3ff, D5
00BD60: 206C 0004      movea.l ($4,A4), A0
00BD64: 5488           addq.l  #2, A0
00BD66: 2248           movea.l A0, A1
00BD68: 3E2C 0008      move.w  ($8,A4), D7
00BD6C: 5547           subq.w  #2, D7
00BD6E: 4847           swap    D7
