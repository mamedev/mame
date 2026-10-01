0066CC: 3B7C 0008 0100      move.w  #$8, ($100,A5)
0066D2: 3B7C 0008 0102      move.w  #$8, ($102,A5)
0066D8: 2A6E 000C           movea.l ($c,A6), A5
0066DC: 3B7C 003C 0104      move.w  #$3c, ($104,A5)
0066E2: 3B7C 003C 0106      move.w  #$3c, ($106,A5)
0066E8: 3B7C 003C 0108      move.w  #$3c, ($108,A5)
0066EE: 3015                move.w  (A5), D0
0066F0: 33C0 0010 B690      move.w  D0, $10b690.l
0066F6: 13FC 0000 0010 B6AF move.b  #$0, $10b6af.l
0066FE: 33FC 0002 0010 B6B0 move.w  #$2, $10b6b0.l
006706: 33FC FFF5 0010 B6B2 move.w  #$fff5, $10b6b2.l
00670E: 323C 006C           move.w  #$6c, D1
006712: 4E4D                trap    #$d
006714: 323C 009C           move.w  #$9c, D1
006718: 4E4D                trap    #$d
00671A: 33FC 0000 0010 B6E2 move.w  #$0, $10b6e2.l
006722: 33FC 0000 0010 B712 move.w  #$0, $10b712.l
00672A: 7E00                moveq   #$0, D7
00672C: 2C79 0010 94A8      movea.l $1094a8.l, A6
006732: 3039 0010 94B2      move.w  $1094b2.l, D0
006738: 41FA 0014           lea     ($674e,PC), A0
00673C: 2D88 0004           move.l  A0, ($4,A6,D0.w)
006740: 0247 0FFF           andi.w  #$fff, D7
006744: 0047 5000           ori.w   #$5000, D7
006748: 3D87 0000           move.w  D7, (A6,D0.w)
00674C: 4E75                rts
00674E: 2A6E 000C           movea.l ($c,A6), A5
006752: 536D 0104           subq.w  #1, ($104,A5)
006756: 660C                bne     $6764
006758: 536D 0100           subq.w  #1, ($100,A5)
00675C: 6700 0094           beq     $67f2
006760: 6000 FF76           bra     $66d8
006764: 0C79 0001 0010 B6E2 cmpi.w  #$1, $10b6e2.l
00676C: 66BC                bne     $672a
00676E: 7E00                moveq   #$0, D7
006770: 2C79 0010 94A8      movea.l $1094a8.l, A6
006776: 3039 0010 94B2      move.w  $1094b2.l, D0
00677C: 41FA 0014           lea     ($6792,PC), A0
006780: 2D88 0004           move.l  A0, ($4,A6,D0.w)
006784: 0247 0FFF           andi.w  #$fff, D7
006788: 0047 5000           ori.w   #$5000, D7
00678C: 3D87 0000           move.w  D7, (A6,D0.w)
006790: 4E75                rts
006792: 2A6E 000C           movea.l ($c,A6), A5
006796: 536D 0106           subq.w  #1, ($106,A5)
00679A: 660A                bne     $67a6
00679C: 536D 0102           subq.w  #1, ($102,A5)
0067A0: 6750                beq     $67f2
0067A2: 6000 FF34           bra     $66d8
0067A6: 0C79 FFFF 0010 B6E2 cmpi.w  #-$1, $10b6e2.l
0067AE: 66BE                bne     $676e
0067B0: 7E00                moveq   #$0, D7
0067B2: 2C79 0010 94A8      movea.l $1094a8.l, A6
0067B8: 3039 0010 94B2      move.w  $1094b2.l, D0
0067BE: 41FA 0014           lea     ($67d4,PC), A0
0067C2: 2D88 0004           move.l  A0, ($4,A6,D0.w)
0067C6: 0247 0FFF           andi.w  #$fff, D7
0067CA: 0047 5000           ori.w   #$5000, D7
0067CE: 3D87 0000           move.w  D7, (A6,D0.w)
0067D2: 4E75                rts
0067D4: 2A6E 000C           movea.l ($c,A6), A5
0067D8: 206D 0002           movea.l ($2,A5), A0
0067DC: 0C50 FFFC           cmpi.w  #-$4, (A0)
0067E0: 6610                bne     $67f2
0067E2: 536D 0108           subq.w  #1, ($108,A5)
0067E6: 670A                beq     $67f2
0067E8: 0C79 FFFF 0010 B712 cmpi.w  #-$1, $10b712.l
0067F0: 66BE                bne     $67b0
0067F2: 33FC 00CC 0010 B4D0 move.w  #$cc, $10b4d0.l
0067FA: 33FC 0800 0010 B74C move.w  #$800, $10b74c.l
006802: 13FC 00FF 0010 B745 move.b  #$ff, $10b745.l
00680A: 6000 018C           bra     $6998
00680E: 2A6E 000C           movea.l ($c,A6), A5
006812: 45ED 0180           lea     ($180,A5), A2
006816: 4BEA 0004           lea     ($4,A2), A5
00681A: 24BC 4845 4C50      move.l  #$48454c50, (A2)
006820: 2ABC 0000 0000      move.l  #$0, (A5)
006826: 3B4A 0002           move.w  A2, ($2,A5)
00682A: 2B7C 0000 0100 0004 move.l  #$100, ($4,A5)
006832: 3B7C 0004 0008      move.w  #$4, ($8,A5)
006838: 43F9 0010 1012      lea     $101012.l, A1
00683E: 7200                moveq   #$0, D1
006840: 1211                move.b  (A1), D1
006842: 13BC 0002 1801      move.b  #$2, ($1,A1,D1.l)
006848: 338A 1802           move.w  A2, ($2,A1,D1.l)
00684C: 13BC 00FF 1804      move.b  #$ff, ($4,A1,D1.l)
006852: 5801                addq.b  #4, D1
006854: 0241 003F           andi.w  #$3f, D1
006858: 1281                move.b  D1, (A1)
00685A: 41F9 004C 0000      lea     $4c0000.l, A0
006860: 0839 0001 0010 B6AE btst    #$1, $10b6ae.l
006868: 6704                beq     $686e
00686A: 41E8 0200           lea     ($200,A0), A0
00686E: 223C 8000 8000      move.l  #$80008000, D1
006874: 303C 00FF           move.w  #$ff, D0
006878: 20C1                move.l  D1, (A0)+
00687A: 20C1                move.l  D1, (A0)+
00687C: 20C1                move.l  D1, (A0)+
00687E: 20C1                move.l  D1, (A0)+
006880: 20C1                move.l  D1, (A0)+
006882: 20C1                move.l  D1, (A0)+
006884: 20C1                move.l  D1, (A0)+
006886: 20C1                move.l  D1, (A0)+
006888: 20C1                move.l  D1, (A0)+
00688A: 20C1                move.l  D1, (A0)+
00688C: 20C1                move.l  D1, (A0)+
00688E: 20C1                move.l  D1, (A0)+
006890: 20C1                move.l  D1, (A0)+
006892: 20C1                move.l  D1, (A0)+
006894: 20C1                move.l  D1, (A0)+
006896: 20C1                move.l  D1, (A0)+
006898: 20C1                move.l  D1, (A0)+
00689A: 20C1                move.l  D1, (A0)+
00689C: 20C1                move.l  D1, (A0)+
00689E: 20C1                move.l  D1, (A0)+
0068A0: 20C1                move.l  D1, (A0)+
0068A2: 20C1                move.l  D1, (A0)+
0068A4: 20C1                move.l  D1, (A0)+
0068A6: 20C1                move.l  D1, (A0)+
0068A8: 20C1                move.l  D1, (A0)+
0068AA: 20C1                move.l  D1, (A0)+
0068AC: 20C1                move.l  D1, (A0)+
0068AE: 20C1                move.l  D1, (A0)+
0068B0: 20C1                move.l  D1, (A0)+
0068B2: 20C1                move.l  D1, (A0)+
0068B4: 20C1                move.l  D1, (A0)+
0068B6: 20C1                move.l  D1, (A0)+
0068B8: 20C1                move.l  D1, (A0)+
0068BA: 20C1                move.l  D1, (A0)+
0068BC: 20C1                move.l  D1, (A0)+
0068BE: 20C1                move.l  D1, (A0)+
0068C0: 20C1                move.l  D1, (A0)+
0068C2: 20C1                move.l  D1, (A0)+
0068C4: 20C1                move.l  D1, (A0)+
0068C6: 20C1                move.l  D1, (A0)+
0068C8: 20C1                move.l  D1, (A0)+
0068CA: 20C1                move.l  D1, (A0)+
