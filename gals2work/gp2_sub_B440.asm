00B440: 007C 0000           ori     #$0, SR
00B444: 33C3 0010 94B8      move.w  D3, $1094b8.l
00B44A: 3A3C 0FFF           move.w  #$fff, D5
00B44E: 51C8 FF4A           dbra    D0, $b39a
00B452: 6000 FD98           bra     $b1ec
00B456: 2604                move.l  D4, D3
00B458: 4843                swap    D3
00B45A: EE4B                lsr.w   #7, D3
00B45C: 0243 0003           andi.w  #$3, D3
00B460: 33C3 007C 0000      move.w  D3, $7c0000.l
00B466: 33C3 0010 94B8      move.w  D3, $1094b8.l
00B46C: 0284 007F FFFF      andi.l  #$7fffff, D4
00B472: 2044                movea.l D4, A0
00B474: D1FC 0080 0000      adda.l  #$800000, A0
00B47A: 4E75                rts
00B47C: 286E 0008           movea.l ($8,A6), A4
00B480: 2A6E 000C           movea.l ($c,A6), A5
00B484: 302C 0004           move.w  ($4,A4), D0
00B488: 6100 01C0           bsr     $b64a
00B48C: 2814                move.l  (A4), D4
00B48E: 6100 01E8           bsr     $b678
00B492: 3C3C 00FF           move.w  #$ff, D6
00B496: 3E3C 00FF           move.w  #$ff, D7
00B49A: 3A3C 0FFF           move.w  #$fff, D5
00B49E: 7000                moveq   #$0, D0
00B4A0: 1018                move.b  (A0)+, D0
00B4A2: 5284                addq.l  #1, D4
00B4A4: 0884 0017           bclr    #$17, D4
00B4A8: 6700 002C           beq     $b4d6
00B4AC: 5279 0010 94B8      addq.w  #1, $1094b8.l
00B4B2: 0279 0003 0010 94B8 andi.w  #$3, $1094b8.l
00B4BA: 48E7 8000           movem.l D0, -(A7)
00B4BE: 3039 0010 94B8      move.w  $1094b8.l, D0
