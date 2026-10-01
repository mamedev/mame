009076: 4EB9 0000 BE32      jsr     $be32.l
00907C: 4E75                rts
00907E: 1010                move.b  (A0), D0
009080: E948                lsl.w   #4, D0
009082: 1010                move.b  (A0), D0
009084: 0240 0F0F           andi.w  #$f0f, D0
009088: 0040 3030           ori.w   #$3030, D0
00908C: 0C40 3A00           cmpi.w  #$3a00, D0
009090: 6D04                blt     $9096
009092: 0640 0700           addi.w  #$700, D0
009096: 0C00 003A           cmpi.b  #$3a, D0
00909A: 6D04                blt     $90a0
00909C: 0600 0007           addi.b  #$7, D0
0090A0: 4840                swap    D0
0090A2: 1028 0001           move.b  ($1,A0), D0
0090A6: E948                lsl.w   #4, D0
0090A8: 1028 0001           move.b  ($1,A0), D0
0090AC: 0240 0F0F           andi.w  #$f0f, D0
0090B0: 0040 3030           ori.w   #$3030, D0
0090B4: 0C40 3A00           cmpi.w  #$3a00, D0
0090B8: 6D04                blt     $90be
0090BA: 0640 0700           addi.w  #$700, D0
0090BE: 0C00 003A           cmpi.b  #$3a, D0
0090C2: 6D04                blt     $90c8
0090C4: 0600 0007           addi.b  #$7, D0
0090C8: 4E75                rts
0090CA: 3001                move.w  D1, D0
0090CC: E048                lsr.w   #8, D0
0090CE: 4841                swap    D1
0090D0: 1200                move.b  D0, D1
0090D2: 3001                move.w  D1, D0
0090D4: E948                lsl.w   #4, D0
0090D6: 1001                move.b  D1, D0
0090D8: 0240 0F0F           andi.w  #$f0f, D0
0090DC: 0040 3030           ori.w   #$3030, D0
0090E0: 0C40 3A00           cmpi.w  #$3a00, D0
0090E4: 6D04                blt     $90ea
0090E6: 0640 0700           addi.w  #$700, D0
0090EA: 0C00 003A           cmpi.b  #$3a, D0
0090EE: 6D04                blt     $90f4
0090F0: 0600 0007           addi.b  #$7, D0
0090F4: 4840                swap    D0
0090F6: 4841                swap    D1
0090F8: 1001                move.b  D1, D0
0090FA: E948                lsl.w   #4, D0
0090FC: 1001                move.b  D1, D0
0090FE: 0240 0F0F           andi.w  #$f0f, D0
009102: 0040 3030           ori.w   #$3030, D0
009106: 0C40 3A00           cmpi.w  #$3a00, D0
00910A: 6D04                blt     $9110
00910C: 0640 0700           addi.w  #$700, D0
009110: 0C00 003A           cmpi.b  #$3a, D0
009114: 6D04                blt     $911a
009116: 0600 0007           addi.b  #$7, D0
00911A: 4ED0                jmp     (A0)
00911C: 5298                addq.l  #1, (A0)+
00911E: 5298                addq.l  #1, (A0)+
009120: 5298                addq.l  #1, (A0)+
009122: 5298                addq.l  #1, (A0)+
009124: 5298                addq.l  #1, (A0)+
009126: 5298                addq.l  #1, (A0)+
009128: 5298                addq.l  #1, (A0)+
00912A: 5298                addq.l  #1, (A0)+
00912C: 4E75                rts
00912E: 41F9 0010 0000      lea     $100000.l, A0
009134: 7000                moveq   #$0, D0
009136: 3039 0010 08D4      move.w  $1008d4.l, D0
00913C: 0679 0200 0010 08D4 addi.w  #$200, $1008d4.l
009144: 323C 01FF           move.w  #$1ff, D1
009148: 1430 0800           move.b  (A0,D0.l), D2
00914C: B430 0800           cmp.b   (A0,D0.l), D2
009150: 6608                bne     $915a
009152: 5280                addq.l  #1, D0
009154: 51C9 FFF2           dbra    D1, $9148
009158: 4E75                rts
00915A: 41F9 0031 0000      lea     $310000.l, A0
009160: 303C 007F           move.w  #$7f, D0
009164: 223C C1F0 7C1F      move.l  #$c1f07c1f, D1
00916A: 20C1                move.l  D1, (A0)+
00916C: 51C8 FFFC           dbra    D0, $916a
009170: 4E75                rts
009172: 43F9 0030 2000      lea     $302000.l, A1
009178: 2468 000A           movea.l ($a,A0), A2
00917C: 6000 001C           bra     $919a
009180: 43F9 0030 2000      lea     $302000.l, A1
009186: 2468 000A           movea.l ($a,A0), A2
00918A: 6000 000E           bra     $919a
00918E: 43F9 0030 2000      lea     $302000.l, A1
009194: 2468 000A           movea.l ($a,A0), A2
009198: 2452                movea.l (A2), A2
00919A: 7200                moveq   #$0, D1
00919C: 3210                move.w  (A0), D1
00919E: E789                lsl.l   #3, D1
0091A0: D3C1                adda.l  D1, A1
0091A2: 3828 0004           move.w  ($4,A0), D4
0091A6: 0244 003F           andi.w  #$3f, D4
0091AA: 0044 00C0           ori.w   #$c0, D4
0091AE: 0044 1800           ori.w   #$1800, D4
0091B2: 3A28 0006           move.w  ($6,A0), D5
0091B6: ED4D                lsl.w   #6, D5
0091B8: 3C28 0008           move.w  ($8,A0), D6
0091BC: ED4E                lsl.w   #6, D6
0091BE: 3028 0002           move.w  ($2,A0), D0
0091C2: 7200                moveq   #$0, D1
0091C4: 121A                move.b  (A2)+, D1
0091C6: 6700 0024           beq     $91ec
0091CA: 0C01 0020           cmpi.b  #$20, D1
0091CE: 6600 0008           bne     $91d8
0091D2: 0446 0200           subi.w  #$200, D6
0091D6: 60EC                bra     $91c4
0091D8: 32C4                move.w  D4, (A1)+
0091DA: 32C1                move.w  D1, (A1)+
0091DC: 32C5                move.w  D5, (A1)+
0091DE: 32C6                move.w  D6, (A1)+
0091E0: 0446 0200           subi.w  #$200, D6
0091E4: 51C8 FFDE           dbra    D0, $91c4
0091E8: 6000 000A           bra     $91f4
0091EC: 4299                clr.l   (A1)+
0091EE: 4299                clr.l   (A1)+
0091F0: 51C8 FFFA           dbra    D0, $91ec
0091F4: 4EE8 000E           jmp     ($e,A0)
0091F8: 48E7 FFE0           movem.l D0-D7/A0-A2, -(A7)
0091FC: 43F9 0030 2000      lea     $302000.l, A1
009202: 7200                moveq   #$0, D1
009204: 3210                move.w  (A0), D1
009206: E789                lsl.l   #3, D1
009208: D3C1                adda.l  D1, A1
00920A: 2468 000A           movea.l ($a,A0), A2
00920E: 3828 0004           move.w  ($4,A0), D4
009212: 0244 003F           andi.w  #$3f, D4
009216: 0044 00C0           ori.w   #$c0, D4
00921A: 0044 1800           ori.w   #$1800, D4
00921E: 3A28 0006           move.w  ($6,A0), D5
009222: ED4D                lsl.w   #6, D5
009224: 3C28 0008           move.w  ($8,A0), D6
009228: ED4E                lsl.w   #6, D6
00922A: 3406                move.w  D6, D2
00922C: 3E28 000E           move.w  ($e,A0), D7
009230: ED4F                lsl.w   #6, D7
009232: 0C47 4000           cmpi.w  #$4000, D7
009236: 6D04                blt     $923c
009238: 0447 0020           subi.w  #$20, D7
00923C: 3028 0002           move.w  ($2,A0), D0
009240: 7200                moveq   #$0, D1
009242: 121A                move.b  (A2)+, D1
009244: 6700 0036           beq     $927c
009248: 0C01 0020           cmpi.b  #$20, D1
00924C: 6600 0008           bne     $9256
009250: 0446 0200           subi.w  #$200, D6
009254: 60EA                bra     $9240
009256: 0C01 000D           cmpi.b  #$d, D1
00925A: 6600 000A           bne     $9266
00925E: 0645 0200           addi.w  #$200, D5
009262: 3C02                move.w  D2, D6
009264: 60DA                bra     $9240
009266: D247                add.w   D7, D1
009268: 32C4                move.w  D4, (A1)+
00926A: 32C1                move.w  D1, (A1)+
00926C: 32C5                move.w  D5, (A1)+
00926E: 32C6                move.w  D6, (A1)+
009270: 0446 0200           subi.w  #$200, D6
009274: 51C8 FFCA           dbra    D0, $9240
