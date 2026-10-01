00B430: 4E75           rts
00B432: 286E 0008      movea.l ($8,A6), A4
00B436: 2A6E 000C      movea.l ($c,A6), A5
00B43A: 4CD5 07FF      movem.l (A5), D0-D7/A0-A2
00B43E: 33C3 007C 0000 move.w  D3, $7c0000.l
00B444: 33C3 0010 94B8 move.w  D3, $1094b8.l
00B44A: 3A3C 0FFF      move.w  #$fff, D5
00B44E: 51C8 FF4A      dbra    D0, $b39a
00B452: 6000 FD98      bra     $b1ec
00B456: 2604           move.l  D4, D3
00B458: 4843           swap    D3
00B45A: EE4B           lsr.w   #7, D3
00B45C: 0243 0003      andi.w  #$3, D3
00B460: 33C3 007C 0000 move.w  D3, $7c0000.l
00B466: 33C3 0010 94B8 move.w  D3, $1094b8.l
00B46C: 0284 007F FFFF andi.l  #$7fffff, D4
