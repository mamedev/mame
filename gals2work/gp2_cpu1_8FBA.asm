008FBA: 3039 0010 0EAC      move.w  $100eac.l, D0
008FC0: B079 0010 0EA8      cmp.w   $100ea8.l, D0
008FC6: 6D00 0030           blt     $8ff8
008FCA: 42B9 0010 0EAA      clr.l   $100eaa.l
008FD0: 41F9 0031 0000      lea     $310000.l, A0
008FD6: 43F9 0010 0CA8      lea     $100ca8.l, A1
008FDC: 7000                moveq   #$0, D0
008FDE: 3231 0000           move.w  (A1,D0.w), D1
008FE2: B270 0000           cmp.w   (A0,D0.w), D1
008FE6: 6700 0006           beq     $8fee
008FEA: 6100 000E           bsr     $8ffa
008FEE: 5440                addq.w  #2, D0
008FF0: 0C40 0200           cmpi.w  #$200, D0
008FF4: 6D00 FFE8           blt     $8fde
008FF8: 4E75                rts
008FFA: 3401                move.w  D1, D2
008FFC: 0242 7C00           andi.w  #$7c00, D2
009000: 3630 0800           move.w  (A0,D0.l), D3
009004: 0243 7C00           andi.w  #$7c00, D3
009008: B642                cmp.w   D2, D3
00900A: 6712                beq     $901e
00900C: 6E0A                bgt     $9018
00900E: 0670 0400 0800      addi.w  #$400, (A0,D0.l)
009014: 6000 0008           bra     $901e
009018: 0470 0400 0800      subi.w  #$400, (A0,D0.l)
00901E: 3401                move.w  D1, D2
009020: 0242 03E0           andi.w  #$3e0, D2
009024: 3630 0800           move.w  (A0,D0.l), D3
009028: 0243 03E0           andi.w  #$3e0, D3
00902C: B642                cmp.w   D2, D3
00902E: 6712                beq     $9042
009030: 6E0A                bgt     $903c
009032: 0670 0020 0800      addi.w  #$20, (A0,D0.l)
009038: 6000 0008           bra     $9042
00903C: 0470 0020 0800      subi.w  #$20, (A0,D0.l)
009042: 3401                move.w  D1, D2
009044: 0242 001F           andi.w  #$1f, D2
009048: 3630 0800           move.w  (A0,D0.l), D3
00904C: 0243 001F           andi.w  #$1f, D3
009050: B642                cmp.w   D2, D3
009052: 6712                beq     $9066
009054: 6E0A                bgt     $9060
009056: 0670 0001 0800      addi.w  #$1, (A0,D0.l)
00905C: 6000 0008           bra     $9066
009060: 0470 0001 0800      subi.w  #$1, (A0,D0.l)
009066: 0241 8000           andi.w  #$8000, D1
00906A: 0270 7FFF 0800      andi.w  #$7fff, (A0,D0.l)
009070: 8370 0000           or.w    D1, (A0,D0.w)
009074: 4E75                rts
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
