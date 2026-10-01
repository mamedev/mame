00B1C6: 286E 0008      movea.l ($8,A6), A4
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
