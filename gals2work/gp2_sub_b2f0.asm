00B2E0: 2D88 0004      move.l  A0, ($4,A6,D0.w)
00B2E4: 0276 E000 0000 andi.w  #$e000, (A6,D0.w)
00B2EA: 0036 00C0 0000 ori.b   #$c0, (A6,D0.w)
00B2F0: 4E75           rts
00B2F2: 6000 FED2      bra     $b1c6
00B2F6: 51CD 0058      dbra    D5, $b350
00B2FA: 48D5 07FF      movem.l D0-D7/A0-A2, (A5)
00B2FE: 41F9 0010 B6D0 lea     $10b6d0.l, A0
00B304: 302E 0002      move.w  ($2,A6), D0
00B308: 0440 005A      subi.w  #$5a, D0
00B30C: 31BC 0001 0000 move.w  #$1, (A0,D0.w)
00B312: 2C79 0010 94A8 movea.l $1094a8.l, A6
00B318: 3039 0010 94B2 move.w  $1094b2.l, D0
00B31E: 41FA 0014      lea     ($b334,PC), A0
