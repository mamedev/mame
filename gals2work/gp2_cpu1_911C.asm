00911C: 5298                     addq.l  #1, (A0)+
00911E: 5298                     addq.l  #1, (A0)+
009120: 5298                     addq.l  #1, (A0)+
009122: 5298                     addq.l  #1, (A0)+
009124: 5298                     addq.l  #1, (A0)+
009126: 5298                     addq.l  #1, (A0)+
009128: 5298                     addq.l  #1, (A0)+
00912A: 5298                     addq.l  #1, (A0)+
00912C: 4E75                     rts
00912E: 41F9 0010 0000           lea     $100000.l, A0
009134: 7000                     moveq   #$0, D0
009136: 3039 0010 08D4           move.w  $1008d4.l, D0
00913C: 0679 0200 0010 08D4      addi.w  #$200, $1008d4.l
009144: 323C 01FF                move.w  #$1ff, D1
009148: 1430 0800                move.b  (A0,D0.l), D2
00914C: B430 0800                cmp.b   (A0,D0.l), D2
009150: 6608                     bne     $915a
009152: 5280                     addq.l  #1, D0
009154: 51C9 FFF2                dbra    D1, $9148
009158: 4E75                     rts
00915A: 41F9 0031 0000           lea     $310000.l, A0
009160: 303C 007F                move.w  #$7f, D0
009164: 223C C1F0 7C1F           move.l  #$c1f07c1f, D1
00916A: 20C1                     move.l  D1, (A0)+
00916C: 51C8 FFFC                dbra    D0, $916a
009170: 4E75                     rts
009172: 43F9 0030 2000           lea     $302000.l, A1
009178: 2468 000A                movea.l ($a,A0), A2
00917C: 6000 001C                bra     $919a
009180: 43F9 0030 2000           lea     $302000.l, A1
009186: 2468 000A                movea.l ($a,A0), A2
00918A: 6000 000E                bra     $919a
00918E: 43F9 0030 2000           lea     $302000.l, A1
009194: 2468 000A                movea.l ($a,A0), A2
009198: 2452                     movea.l (A2), A2
00919A: 7200                     moveq   #$0, D1
00919C: 3210                     move.w  (A0), D1
00919E: E789                     lsl.l   #3, D1
0091A0: D3C1                     adda.l  D1, A1
0091A2: 3828 0004                move.w  ($4,A0), D4
0091A6: 0244 003F                andi.w  #$3f, D4
0091AA: 0044 00C0                ori.w   #$c0, D4
0091AE: 0044 1800                ori.w   #$1800, D4
0091B2: 3A28 0006                move.w  ($6,A0), D5
0091B6: ED4D                     lsl.w   #6, D5
0091B8: 3C28 0008                move.w  ($8,A0), D6
0091BC: ED4E                     lsl.w   #6, D6
0091BE: 3028 0002                move.w  ($2,A0), D0
0091C2: 7200                     moveq   #$0, D1
0091C4: 121A                     move.b  (A2)+, D1
0091C6: 6700 0024                beq     $91ec
0091CA: 0C01 0020                cmpi.b  #$20, D1
0091CE: 6600 0008                bne     $91d8
0091D2: 0446 0200                subi.w  #$200, D6
0091D6: 60EC                     bra     $91c4
0091D8: 32C4                     move.w  D4, (A1)+
0091DA: 32C1                     move.w  D1, (A1)+
0091DC: 32C5                     move.w  D5, (A1)+
0091DE: 32C6                     move.w  D6, (A1)+
0091E0: 0446 0200                subi.w  #$200, D6
0091E4: 51C8 FFDE                dbra    D0, $91c4
0091E8: 6000 000A                bra     $91f4
0091EC: 4299                     clr.l   (A1)+
0091EE: 4299                     clr.l   (A1)+
0091F0: 51C8 FFFA                dbra    D0, $91ec
0091F4: 4EE8 000E                jmp     ($e,A0)
0091F8: 48E7 FFE0                movem.l D0-D7/A0-A2, -(A7)
0091FC: 43F9 0030 2000           lea     $302000.l, A1
009202: 7200                     moveq   #$0, D1
009204: 3210                     move.w  (A0), D1
009206: E789                     lsl.l   #3, D1
009208: D3C1                     adda.l  D1, A1
00920A: 2468 000A                movea.l ($a,A0), A2
00920E: 3828 0004                move.w  ($4,A0), D4
009212: 0244 003F                andi.w  #$3f, D4
009216: 0044 00C0                ori.w   #$c0, D4
00921A: 0044 1800                ori.w   #$1800, D4
00921E: 3A28 0006                move.w  ($6,A0), D5
009222: ED4D                     lsl.w   #6, D5
009224: 3C28 0008                move.w  ($8,A0), D6
009228: ED4E                     lsl.w   #6, D6
00922A: 3406                     move.w  D6, D2
00922C: 3E28 000E                move.w  ($e,A0), D7
009230: ED4F                     lsl.w   #6, D7
009232: 0C47 4000                cmpi.w  #$4000, D7
009236: 6D04                     blt     $923c
009238: 0447 0020                subi.w  #$20, D7
00923C: 3028 0002                move.w  ($2,A0), D0
009240: 7200                     moveq   #$0, D1
009242: 121A                     move.b  (A2)+, D1
009244: 6700 0036                beq     $927c
009248: 0C01 0020                cmpi.b  #$20, D1
00924C: 6600 0008                bne     $9256
009250: 0446 0200                subi.w  #$200, D6
009254: 60EA                     bra     $9240
009256: 0C01 000D                cmpi.b  #$d, D1
00925A: 6600 000A                bne     $9266
00925E: 0645 0200                addi.w  #$200, D5
009262: 3C02                     move.w  D2, D6
009264: 60DA                     bra     $9240
009266: D247                     add.w   D7, D1
009268: 32C4                     move.w  D4, (A1)+
00926A: 32C1                     move.w  D1, (A1)+
00926C: 32C5                     move.w  D5, (A1)+
00926E: 32C6                     move.w  D6, (A1)+
009270: 0446 0200                subi.w  #$200, D6
009274: 51C8 FFCA                dbra    D0, $9240
009278: 6000 000A                bra     $9284
00927C: 4299                     clr.l   (A1)+
00927E: 4299                     clr.l   (A1)+
009280: 51C8 FFFA                dbra    D0, $927c
009284: 4CDF 07FF                movem.l (A7)+, D0-D7/A0-A2
009288: 4E75                     rts
00928A: 23FC 0030 2FC8 0010 0EE0 move.l  #$302fc8, $100ee0.l
009294: 4E75                     rts
009296: 2079 0010 0EE0           movea.l $100ee0.l, A0
00929C: 43F9 0030 3400           lea     $303400.l, A1
0092A2: 203C 8000 8000           move.l  #$80008000, D0
0092A8: 4298                     clr.l   (A0)+
0092AA: 20C0                     move.l  D0, (A0)+
0092AC: B1C9                     cmpa.l  A1, A0
0092AE: 65F8                     bcs     $92a8
0092B0: 23FC 0030 2FC8 0010 0EE0 move.l  #$302fc8, $100ee0.l
0092BA: 4E75                     rts
0092BC: 48E7 FFE0                movem.l D0-D7/A0-A2, -(A7)
0092C0: 2279 0010 0EE0           movea.l $100ee0.l, A1
0092C6: 2450                     movea.l (A0), A2
0092C8: 3828 0004                move.w  ($4,A0), D4
0092CC: 0244 003F                andi.w  #$3f, D4
0092D0: 0044 00C0                ori.w   #$c0, D4
0092D4: 0044 1800                ori.w   #$1800, D4
0092D8: 3A28 0008                move.w  ($8,A0), D5
0092DC: ED4D                     lsl.w   #6, D5
0092DE: 3C28 000A                move.w  ($a,A0), D6
0092E2: ED4E                     lsl.w   #6, D6
0092E4: 3E28 0006                move.w  ($6,A0), D7
0092E8: ED4F                     lsl.w   #6, D7
0092EA: 0C47 4000                cmpi.w  #$4000, D7
0092EE: 6D04                     blt     $92f4
0092F0: 0447 0020                subi.w  #$20, D7
0092F4: 7200                     moveq   #$0, D1
0092F6: 121A                     move.b  (A2)+, D1
0092F8: 6700 0020                beq     $931a
0092FC: 0C01 0020                cmpi.b  #$20, D1
009300: 6600 0008                bne     $930a
009304: 0446 0200                subi.w  #$200, D6
009308: 60EA                     bra     $92f4
00930A: D247                     add.w   D7, D1
00930C: 32C4                     move.w  D4, (A1)+
00930E: 32C1                     move.w  D1, (A1)+
009310: 32C5                     move.w  D5, (A1)+
009312: 32C6                     move.w  D6, (A1)+
009314: 0446 0200                subi.w  #$200, D6
009318: 60DA                     bra     $92f4
00931A: 23C9 0010 0EE0           move.l  A1, $100ee0.l
