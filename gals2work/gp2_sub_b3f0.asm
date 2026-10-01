00B3F0: 6000 FDD4      bra     $b1c6
00B3F4: 51CD 0058      dbra    D5, $b44e
00B3F8: 48D5 07FF      movem.l D0-D7/A0-A2, (A5)
00B3FC: 41F9 0010 B6D0 lea     $10b6d0.l, A0
00B402: 302E 0002      move.w  ($2,A6), D0
00B406: 0440 005A      subi.w  #$5a, D0
00B40A: 31BC 0001 0000 move.w  #$1, (A0,D0.w)
00B410: 2C79 0010 94A8 movea.l $1094a8.l, A6
00B416: 3039 0010 94B2 move.w  $1094b2.l, D0
00B41C: 41FA 0014      lea     ($b432,PC), A0
00B420: 2D88 0004      move.l  A0, ($4,A6,D0.w)
00B424: 0236 007F 0000 andi.b  #$7f, (A6,D0.w)
00B42A: 0036 0040 0000 ori.b   #$40, (A6,D0.w)
