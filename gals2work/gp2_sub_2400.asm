002400: 33FC 00CC 0010 B4CE      move.w  #$cc, $10b4ce.l
002408: 33FC 00CC 0010 B4D0      move.w  #$cc, $10b4d0.l
002410: 41F9 0002 8000           lea     $28000.l, A0
002416: 43F9 0010 0010           lea     $100010.l, A1
00241C: 303C 0800                move.w  #$800, D0
002420: 32D8                     move.w  (A0)+, (A1)+
002422: 51C8 FFFC                dbra    D0, $2420
002426: 5447                     addq.w  #2, D7
002428: 4EF9 0000 10B4           jmp     $10b4.l
00242E: 23FC 0010 110A 0010 94A4 move.l  #$10110a, $1094a4.l
002438: 23FC 0010 121A 0010 94A8 move.l  #$10121a, $1094a8.l
002442: 23FC 0010 141A 0010 94AC move.l  #$10141a, $1094ac.l
00244C: 2079 0010 94A4           movea.l $1094a4.l, A0
002452: 3010                     move.w  (A0), D0
002454: 31BC 4000 0004           move.w  #$4000, ($4,A0,D0.w)
00245A: 31BC 0000 0006           move.w  #$0, ($6,A0,D0.w)
002460: 42B0 0008                clr.l   ($8,A0,D0.w)
002464: 0650 0008                addi.w  #$8, (A0)
002468: 0250 FEFF                andi.w  #$feff, (A0)
00246C: 41F9 0010 0000           lea     $100000.l, A0
002472: 43FA 004C                lea     ($24c0,PC), A1
002476: 20D9                     move.l  (A1)+, (A0)+
002478: 20D9                     move.l  (A1)+, (A0)+
00247A: 20D9                     move.l  (A1)+, (A0)+
00247C: 20D9                     move.l  (A1)+, (A0)+
00247E: 33FC 0000 0078 0000      move.w  #$0, $780000.l
002486: 33FC 0001 0078 0000      move.w  #$1, $780000.l
00248E: 303C 0014                move.w  #$14, D0
002492: 51C8 FFFE                dbra    D0, $2492
002496: 33FC 0000 0078 0000      move.w  #$0, $780000.l
00249E: 08F9 0006 0010 941A      bset    #$6, $10941a.l
0024A6: 08F9 0006 0010 941C      bset    #$6, $10941c.l
0024AE: 08F9 0006 0010 941E      bset    #$6, $10941e.l
0024B6: 46FC 2000                move    #$2000, SR
0024BA: 4EF9 0000 1774           jmp     $1774.l
0024C0: 0010 1012                ori.b   #$12, (A0)
0024C4: 0010 1FFE                ori.b   #$fe, (A0)
0024C8: 0011 0000                ori.b   #$0, (A1)
0024CC: 0010 0010                ori.b   #$10, (A0)
0024D0: 0010 1FFC                ori.b   #$fc, (A0)
0024D4: 4E70                     reset
0024D6: 4EF9 0000 2990           jmp     $2990.l
0024DC: E848                     lsr.w   #4, D0
0024DE: 0800 000B                btst    #$b, D0
0024E2: 6600 0046                bne     $252a
0024E6: 0800 000A                btst    #$a, D0
0024EA: 6600 001C                bne     $2508
0024EE: 0240 03FE                andi.w  #$3fe, D0
0024F2: 323B 007A                move.w  ($7a,PC,D0.w), D1
0024F6: 4440                     neg.w   D0
0024F8: 0640 0400                addi.w  #$400, D0
0024FC: 303B 0070                move.w  ($70,PC,D0.w), D0
002500: 3600                     move.w  D0, D3
002502: 3401                     move.w  D1, D2
002504: 4442                     neg.w   D2
002506: 4E75                     rts
002508: 0240 03FE                andi.w  #$3fe, D0
00250C: 4440                     neg.w   D0
00250E: 0640 0400                addi.w  #$400, D0
002512: 323B 005A                move.w  ($5a,PC,D0.w), D1
002516: 4440                     neg.w   D0
002518: 0640 0400                addi.w  #$400, D0
00251C: 303B 0050                move.w  ($50,PC,D0.w), D0
002520: 4440                     neg.w   D0
002522: 3600                     move.w  D0, D3
002524: 3401                     move.w  D1, D2
002526: 4442                     neg.w   D2
002528: 4E75                     rts
00252A: 0800 000A                btst    #$a, D0
00252E: 6600 001E                bne     $254e
002532: 0240 03FE                andi.w  #$3fe, D0
002536: 323B 0036                move.w  ($36,PC,D0.w), D1
00253A: 4440                     neg.w   D0
00253C: 0640 0400                addi.w  #$400, D0
002540: 303B 002C                move.w  ($2c,PC,D0.w), D0
002544: 3401                     move.w  D1, D2
002546: 4440                     neg.w   D0
002548: 4441                     neg.w   D1
00254A: 3600                     move.w  D0, D3
00254C: 4E75                     rts
00254E: 0240 03FE                andi.w  #$3fe, D0
002552: 4440                     neg.w   D0
002554: 0640 0400                addi.w  #$400, D0
002558: 323B 0014                move.w  ($14,PC,D0.w), D1
00255C: 4440                     neg.w   D0
00255E: 0640 0400                addi.w  #$400, D0
002562: 303B 000A                move.w  ($a,PC,D0.w), D0
002566: 3600                     move.w  D0, D3
002568: 3401                     move.w  D1, D2
00256A: 4441                     neg.w   D1
00256C: 4E75                     rts
00256E: 0000 0032                ori.b   #$32, D0
002572: 0065 0097                ori.w   #$97, -(A5)
002576: 00C9                     dc.w    $00c9; ILLEGAL
002578: 00FB                     dc.w    $00fb; ILLEGAL
00257A: 012E 0160                btst    D0, ($160,A6)
00257E: 0192                     bclr    D0, (A2)
002580: 01C4                     bset    D0, D4
002582: 01F7 0229                bset    D0, INVALID 37
002586: 025B 028D                andi.w  #$28d, (A3)+
00258A: 02C0                     dc.w    $02c0; ILLEGAL
00258C: 02F2                     dc.w    $02f2; ILLEGAL
00258E: 0324                     btst    D1, -(A4)
002590: 0356                     bchg    D1, (A6)
002592: 0388 03BB                movep.w D1, ($3bb,A0)
002596: 03ED 041F                bset    D1, ($41f,A5)
00259A: 0451 0483                subi.w  #$483, (A1)
00259E: 04B5 04E7 051A 054C      subi.l  #$4e7051a, INVALID 35
0025A6: 057E                     dc.w    $057e; ILLEGAL
0025A8: 05B0 05E2                bclr    D2, INVALID 30
0025AC: 0614 0646                addi.b  #$46, (A4)
0025B0: 0678 06AA 06DC           addi.w  #$6aa, $6dc.w
0025B6: 070E 0740                movep.w ($740,A6), D3
0025BA: 0772 07A4                bchg    D3, INVALID 32
0025BE: 07D6                     bset    D3, (A6)
0025C0: 0807 0839                btst    #$39, D7
0025C4: 086B 089D 08CF           bchg    #$9d, ($8cf,A3)
0025CA: 0901                     btst    D4, D1
0025CC: 0932 0964                btst    D4, INVALID 32
0025D0: 0996                     bclr    D4, (A6)
0025D2: 09C7                     bset    D4, D7
0025D4: 09F9 0A2B 0A5C           bset    D4, $a2b0a5c.l
0025DA: 0A8E                     dc.w    $0a8e; ILLEGAL
0025DC: 0AC0                     dc.w    $0ac0; ILLEGAL
0025DE: 0AF1                     dc.w    $0af1; ILLEGAL
0025E0: 0B23                     btst    D5, -(A3)
0025E2: 0B54                     bchg    D5, (A4)
0025E4: 0B85                     bclr    D5, D5
0025E6: 0BB7 0BE8                bclr    D5, INVALID 37
0025EA: 0C1A 0C4B                cmpi.b  #$4b, (A2)+
0025EE: 0C7C                     dc.w    $0c7c; ILLEGAL
0025F0: 0CAE 0CDF 0D10 0D41      cmpi.l  #$cdf0d10, ($d41,A6)
0025F8: 0D72 0DA4                bchg    D6, INVALID 32
0025FC: 0DD5                     bset    D6, (A5)
0025FE: 0E06                     dc.w    $0e06; ILLEGAL
