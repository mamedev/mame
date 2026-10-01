00B0E0: 202C 0004      move.l  ($4,A4), D0
00B0E4: 6100 00A0      bsr     $b186
00B0E8: 2248           movea.l A0, A1
00B0EA: 2448           movea.l A0, A2
00B0EC: 2054           movea.l (A4), A0
00B0EE: 362C 0008      move.w  ($8,A4), D3
00B0F2: 342C 000A      move.w  ($a,A4), D2
00B0F6: 3A3C 0FFF      move.w  #$fff, D5
00B0FA: 7200           moveq   #$0, D1
00B0FC: 1218           move.b  (A0)+, D1
00B0FE: 1018           move.b  (A0)+, D0
00B100: E148           lsl.w   #8, D0
00B102: 1018           move.b  (A0)+, D0
00B104: 3802           move.w  D2, D4
00B106: 34C0           move.w  D0, (A2)+
00B108: 006A 8000 FFFE ori.w   #$8000, (-$2,A2)
00B10E: 51C9 000C      dbra    D1, $b11c
00B112: 7200           moveq   #$0, D1
00B114: 1218           move.b  (A0)+, D1
00B116: 1018           move.b  (A0)+, D0
00B118: E148           lsl.w   #8, D0
00B11A: 1018           move.b  (A0)+, D0
00B11C: 51CD 0034      dbra    D5, $b152
00B120: 48D5 071F      movem.l D0-D4/A0-A2, (A5)
00B124: 2C79 0010 94A8 movea.l $1094a8.l, A6
00B12A: 3039 0010 94B2 move.w  $1094b2.l, D0
00B130: 41FA 0014      lea     ($b146,PC), A0
00B134: 2D88 0004      move.l  A0, ($4,A6,D0.w)
00B138: 0236 007F 0000 andi.b  #$7f, (A6,D0.w)
00B13E: 0036 0040 0000 ori.b   #$40, (A6,D0.w)
00B144: 4E75           rts
00B146: 2A6E 000C      movea.l ($c,A6), A5
00B14A: 4CD5 071F      movem.l (A5), D0-D4/A0-A2
00B14E: 3A3C 0FFF      move.w  #$fff, D5
00B152: 51CC FFB2      dbra    D4, $b106
00B156: 43E9 0400      lea     ($400,A1), A1
00B15A: 2449           movea.l A1, A2
00B15C: 51CB FFA6      dbra    D3, $b104
00B160: 2C79 0010 94A8 movea.l $1094a8.l, A6
00B166: 3039 0010 94B2 move.w  $1094b2.l, D0
00B16C: 41FA 0014      lea     ($b182,PC), A0
00B170: 2D88 0004      move.l  A0, ($4,A6,D0.w)
00B174: 0276 E000 0000 andi.w  #$e000, (A6,D0.w)
00B17A: 0036 00C0 0000 ori.b   #$c0, (A6,D0.w)
00B180: 4E75           rts
00B182: 6000 FF54      bra     $b0d8
00B186: 0800 001F      btst    #$1f, D0
00B18A: 6600 0006      bne     $b192
00B18E: 2040           movea.l D0, A0
00B190: 4E75           rts
00B192: 2200           move.l  D0, D1
00B194: 4841           swap    D1
00B196: ED59           rol.w   #6, D1
00B198: 0241 000C      andi.w  #$c, D1
00B19C: 207B 1018      movea.l ($18,PC,D1.w), A0
00B1A0: 7200           moveq   #$0, D1
00B1A2: 3200           move.w  D0, D1
00B1A4: D241           add.w   D1, D1
00B1A6: 7400           moveq   #$0, D2
00B1A8: 4840           swap    D0
00B1AA: 1400           move.b  D0, D2
00B1AC: 700A           moveq   #$a, D0
00B1AE: E1AA           lsl.l   D0, D2
00B1B0: D282           add.l   D2, D1
00B1B2: D1C1           adda.l  D1, A0
00B1B4: 4E75           rts
00B1B6: 0040 0000      ori.w   #$0, D0
00B1BA: 0044 0000      ori.w   #$0, D4
00B1BE: 0048           dc.w    $0048; ILLEGAL
00B1C0: 0000 004C      ori.b   #$4c, D0
00B1C4: 0000 286E      ori.b   #$6e, D0
00B1C8: 0008           dc.w    $0008; ILLEGAL
00B1CA: 2A6E 000C      movea.l ($c,A6), A5
00B1CE: 2814           move.l  (A4), D4
00B1D0: 6100 0284      bsr     $b456
00B1D4: 226C 0004      movea.l ($4,A4), A1
00B1D8: 2449           movea.l A1, A2
00B1DA: 3C2C 000A      move.w  ($a,A4), D6
00B1DE: 3E2C 0008      move.w  ($8,A4), D7
00B1E2: 3406           move.w  D6, D2
00B1E4: 946C 000E      sub.w   ($e,A4), D2
00B1E8: 3A3C 0FFF      move.w  #$fff, D5
00B1EC: 7000           moveq   #$0, D0
00B1EE: 1018           move.b  (A0)+, D0
00B1F0: 5284           addq.l  #1, D4
00B1F2: 0884 0017      bclr    #$17, D4
00B1F6: 6700 0016      beq     $b20e
00B1FA: 5243           addq.w  #1, D3
00B1FC: 33C3 007C 0000 move.w  D3, $7c0000.l
00B202: 33C3 0010 94B8 move.w  D3, $1094b8.l
00B208: 91FC 0080 0000 suba.l  #$800000, A0
00B20E: 0880 0007      bclr    #$7, D0
00B212: 6600 0144      bne     $b358
00B216: B446           cmp.w   D6, D2
00B218: 6300 004A      bls     $b264
00B21C: 5288           addq.l  #1, A0
00B21E: 5284           addq.l  #1, D4
00B220: 0884 0017      bclr    #$17, D4
00B224: 6700 0016      beq     $b23c
00B228: 5243           addq.w  #1, D3
00B22A: 33C3 007C 0000 move.w  D3, $7c0000.l
00B230: 33C3 0010 94B8 move.w  D3, $1094b8.l
00B236: 91FC 0080 0000 suba.l  #$800000, A0
00B23C: 5288           addq.l  #1, A0
00B23E: 5284           addq.l  #1, D4
00B240: 0884 0017      bclr    #$17, D4
00B244: 6700 0016      beq     $b25c
00B248: 5243           addq.w  #1, D3
00B24A: 33C3 007C 0000 move.w  D3, $7c0000.l
00B250: 33C3 0010 94B8 move.w  D3, $1094b8.l
00B256: 91FC 0080 0000 suba.l  #$800000, A0
00B25C: 51CE 0098      dbra    D6, $b2f6
00B260: 6000 004C      bra     $b2ae
00B264: 14D8           move.b  (A0)+, (A2)+
00B266: 5284           addq.l  #1, D4
00B268: 0884 0017      bclr    #$17, D4
00B26C: 6700 0016      beq     $b284
00B270: 5243           addq.w  #1, D3
00B272: 33C3 007C 0000 move.w  D3, $7c0000.l
00B278: 33C3 0010 94B8 move.w  D3, $1094b8.l
00B27E: 91FC 0080 0000 suba.l  #$800000, A0
00B284: 14D8           move.b  (A0)+, (A2)+
00B286: 5284           addq.l  #1, D4
00B288: 0884 0017      bclr    #$17, D4
00B28C: 6700 0016      beq     $b2a4
00B290: 5243           addq.w  #1, D3
00B292: 33C3 007C 0000 move.w  D3, $7c0000.l
00B298: 33C3 0010 94B8 move.w  D3, $1094b8.l
00B29E: 91FC 0080 0000 suba.l  #$800000, A0
00B2A4: 006A 8000 FFFE ori.w   #$8000, (-$2,A2)
00B2AA: 51CE 004A      dbra    D6, $b2f6
00B2AE: 43E9 0400      lea     ($400,A1), A1
00B2B2: 2449           movea.l A1, A2
00B2B4: 3C2C 000A      move.w  ($a,A4), D6
00B2B8: 51CF 003C      dbra    D7, $b2f6
00B2BC: 41F9 0010 B6D0 lea     $10b6d0.l, A0
00B2C2: 302E 0002      move.w  ($2,A6), D0
00B2C6: 0440 005A      subi.w  #$5a, D0
00B2CA: 31BC FFFF 0000 move.w  #$ffff, (A0,D0.w)
00B2D0: 2C79 0010 94A8 movea.l $1094a8.l, A6
00B2D6: 3039 0010 94B2 move.w  $1094b2.l, D0
00B2DC: 41FA 0014      lea     ($b2f2,PC), A0
