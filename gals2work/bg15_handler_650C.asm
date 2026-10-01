00650C: 33FC 0001 0010 B764 move.w  #$1, $10b764.l
006514: 323C 01B0           move.w  #$1b0, D1
006518: 4E4D                trap    #$d
00651A: 33FC 0800 0010 B74C move.w  #$800, $10b74c.l
006522: 13FC 0000 0010 B745 move.b  #$0, $10b745.l
00652A: 6000 F0B4           bra     $55e0
00652E: 33FC 0001 0010 B762 move.w  #$1, $10b762.l
006536: 323C 01AA           move.w  #$1aa, D1
00653A: 4E4D                trap    #$d
00653C: 33FC 0800 0010 B74A move.w  #$800, $10b74a.l
006544: 13FC 0000 0010 B744 move.b  #$0, $10b744.l
00654C: 33FC 0001 0010 B764 move.w  #$1, $10b764.l
006554: 323C 01B0           move.w  #$1b0, D1
006558: 4E4D                trap    #$d
00655A: 33FC 0800 0010 B74C move.w  #$800, $10b74c.l
006562: 13FC 0000 0010 B745 move.b  #$0, $10b745.l
00656A: 6000 F074           bra     $55e0
00656E: 33FC 0800 0010 B746 move.w  #$800, $10b746.l
006576: 13FC 0000 0010 B742 move.b  #$0, $10b742.l
00657E: 33FC 0800 0010 B748 move.w  #$800, $10b748.l
006586: 13FC 0000 0010 B743 move.b  #$0, $10b743.l
00658E: 33FC 0800 0010 B74A move.w  #$800, $10b74a.l
006596: 13FC 0000 0010 B744 move.b  #$0, $10b744.l
00659E: 33FC 0800 0010 B74C move.w  #$800, $10b74c.l
0065A6: 13FC 0000 0010 B745 move.b  #$0, $10b745.l
0065AE: 33FC 0001 0010 B75E move.w  #$1, $10b75e.l
0065B6: 33FC 0001 0010 B760 move.w  #$1, $10b760.l
0065BE: 33FC 0001 0010 B762 move.w  #$1, $10b762.l
0065C6: 33FC 0001 0010 B764 move.w  #$1, $10b764.l
0065CE: 33FC 0000 0010 B4CA move.w  #$0, $10b4ca.l
0065D6: 33FC 0000 0010 B4CC move.w  #$0, $10b4cc.l
0065DE: 33FC 0000 0010 B4CE move.w  #$0, $10b4ce.l
0065E6: 33FC 0000 0010 B4D0 move.w  #$0, $10b4d0.l
0065EE: 2079 0010 94A4      movea.l $1094a4.l, A0
0065F4: 3010                move.w  (A0), D0
0065F6: 31BC 4000 0004      move.w  #$4000, ($4,A0,D0.w)
0065FC: 31BC 01B0 0006      move.w  #$1b0, ($6,A0,D0.w)
006602: 21BC 0000 0000 0008 move.l  #$0, ($8,A0,D0.w)
00660A: 0650 0008           addi.w  #$8, (A0)
00660E: 0250 00FF           andi.w  #$ff, (A0)
006612: 4A39 0010 B5D4      tst.b   $10b5d4.l
006618: 6600 01F4           bne     $680e
00661C: 0C39 0009 0010 B5D6 cmpi.b  #$9, $10b5d6.l
006624: 6C00 01E8           bge     $680e
006628: 0C39 000A 0010 B5D7 cmpi.b  #$a, $10b5d7.l
006630: 6C00 01DC           bge     $680e
006634: 0C39 0003 0010 B5DA cmpi.b  #$3, $10b5da.l
00663C: 6C00 01D0           bge     $680e
006640: 0C39 0001 0010 B5DA cmpi.b  #$1, $10b5da.l
006648: 6622                bne     $666c
00664A: 0C39 00FF 0010 B5D9 cmpi.b  #-$1, $10b5d9.l
006652: 6718                beq     $666c
006654: 7001                moveq   #$1, D0
006656: 1039 0010 B5D9      move.b  $10b5d9.l, D0
00665C: E748                lsl.w   #3, D0
00665E: 41F9 0000 AE04      lea     $ae04.l, A0
006664: 3030 0002           move.w  ($2,A0,D0.w), D0
006668: 6000 006E           bra     $66d8
00666C: 41F9 0000 A4F4      lea     $a4f4.l, A0
006672: 7200                moveq   #$0, D1
006674: 1239 0010 B5D4      move.b  $10b5d4.l, D1
00667A: D281                add.l   D1, D1
00667C: D281                add.l   D1, D1
00667E: 2070 1800           movea.l (A0,D1.l), A0
006682: 7000                moveq   #$0, D0
006684: 7200                moveq   #$0, D1
006686: 1039 0010 B5D6      move.b  $10b5d6.l, D0
00668C: 1239 0010 B5D7      move.b  $10b5d7.l, D1
006692: E188                lsl.l   #8, D0
006694: E989                lsl.l   #4, D1
006696: D081                add.l   D1, D0
006698: 3030 0800           move.w  (A0,D0.l), D0
00669C: 2A6E 000C           movea.l ($c,A6), A5
0066A0: 3A80                move.w  D0, (A5)
0066A2: 2079 0000 46C0      movea.l $46c0.l, A0
0066A8: 0839 0003 0010 B5D3 btst    #$3, $10b5d3.l
0066B0: 6706                beq     $66b8
0066B2: 2079 0000 46C4      movea.l $46c4.l, A0
0066B8: 0280 0000 FFFF      andi.l  #$ffff, D0
0066BE: D080                add.l   D0, D0
0066C0: 3030 0802           move.w  ($2,A0,D0.l), D0
0066C4: 41F0 0000           lea     (A0,D0.w), A0
0066C8: 2B48 0002           move.l  A0, ($2,A5)
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
0068CC: 20C1                move.l  D1, (A0)+
0068CE: 20C1                move.l  D1, (A0)+
0068D0: 20C1                move.l  D1, (A0)+
0068D2: 20C1                move.l  D1, (A0)+
0068D4: 20C1                move.l  D1, (A0)+
0068D6: 20C1                move.l  D1, (A0)+
0068D8: 20C1                move.l  D1, (A0)+
0068DA: 20C1                move.l  D1, (A0)+
0068DC: 20C1                move.l  D1, (A0)+
0068DE: 20C1                move.l  D1, (A0)+
0068E0: 20C1                move.l  D1, (A0)+
0068E2: 20C1                move.l  D1, (A0)+
0068E4: 20C1                move.l  D1, (A0)+
0068E6: 20C1                move.l  D1, (A0)+
0068E8: 20C1                move.l  D1, (A0)+
0068EA: 20C1                move.l  D1, (A0)+
0068EC: 20C1                move.l  D1, (A0)+
0068EE: 20C1                move.l  D1, (A0)+
0068F0: 20C1                move.l  D1, (A0)+
0068F2: 20C1                move.l  D1, (A0)+
0068F4: 20C1                move.l  D1, (A0)+
0068F6: 20C1                move.l  D1, (A0)+
0068F8: 20C1                move.l  D1, (A0)+
0068FA: 20C1                move.l  D1, (A0)+
0068FC: 20C1                move.l  D1, (A0)+
0068FE: 20C1                move.l  D1, (A0)+
006900: 20C1                move.l  D1, (A0)+
006902: 20C1                move.l  D1, (A0)+
006904: 20C1                move.l  D1, (A0)+
006906: 20C1                move.l  D1, (A0)+
006908: 20C1                move.l  D1, (A0)+
00690A: 20C1                move.l  D1, (A0)+
00690C: 20C1                move.l  D1, (A0)+
00690E: 20C1                move.l  D1, (A0)+
006910: 20C1                move.l  D1, (A0)+
006912: 20C1                move.l  D1, (A0)+
006914: 20C1                move.l  D1, (A0)+
006916: 20C1                move.l  D1, (A0)+
006918: 20C1                move.l  D1, (A0)+
00691A: 20C1                move.l  D1, (A0)+
00691C: 20C1                move.l  D1, (A0)+
00691E: 20C1                move.l  D1, (A0)+
006920: 20C1                move.l  D1, (A0)+
006922: 20C1                move.l  D1, (A0)+
006924: 20C1                move.l  D1, (A0)+
006926: 20C1                move.l  D1, (A0)+
006928: 20C1                move.l  D1, (A0)+
00692A: 20C1                move.l  D1, (A0)+
00692C: 20C1                move.l  D1, (A0)+
00692E: 20C1                move.l  D1, (A0)+
006930: 20C1                move.l  D1, (A0)+
006932: 20C1                move.l  D1, (A0)+
006934: 20C1                move.l  D1, (A0)+
006936: 20C1                move.l  D1, (A0)+
006938: 20C1                move.l  D1, (A0)+
00693A: 20C1                move.l  D1, (A0)+
00693C: 20C1                move.l  D1, (A0)+
00693E: 20C1                move.l  D1, (A0)+
006940: 20C1                move.l  D1, (A0)+
006942: 20C1                move.l  D1, (A0)+
006944: 20C1                move.l  D1, (A0)+
006946: 20C1                move.l  D1, (A0)+
006948: 20C1                move.l  D1, (A0)+
00694A: 20C1                move.l  D1, (A0)+
00694C: 20C1                move.l  D1, (A0)+
00694E: 20C1                move.l  D1, (A0)+
006950: 20C1                move.l  D1, (A0)+
006952: 20C1                move.l  D1, (A0)+
006954: 20C1                move.l  D1, (A0)+
006956: 20C1                move.l  D1, (A0)+
006958: 20C1                move.l  D1, (A0)+
00695A: 20C1                move.l  D1, (A0)+
00695C: 20C1                move.l  D1, (A0)+
00695E: 20C1                move.l  D1, (A0)+
006960: 20C1                move.l  D1, (A0)+
006962: 20C1                move.l  D1, (A0)+
006964: 20C1                move.l  D1, (A0)+
006966: 20C1                move.l  D1, (A0)+
006968: 20C1                move.l  D1, (A0)+
00696A: 20C1                move.l  D1, (A0)+
00696C: 20C1                move.l  D1, (A0)+
00696E: 20C1                move.l  D1, (A0)+
006970: 20C1                move.l  D1, (A0)+
006972: 20C1                move.l  D1, (A0)+
006974: 20C1                move.l  D1, (A0)+
006976: 20C1                move.l  D1, (A0)+
006978: 41E8 0200           lea     ($200,A0), A0
00697C: 51C8 FEFA           dbra    D0, $6878
006980: 33FC 00CC 0010 B4D0 move.w  #$cc, $10b4d0.l
006988: 33FC 0800 0010 B74C move.w  #$800, $10b74c.l
006990: 13FC 00FF 0010 B745 move.b  #$ff, $10b745.l
006998: 7000                moveq   #$0, D0
00699A: 4A79 0000 AE2C      tst.w   $ae2c.l
0069A0: 6714                beq     $69b6
0069A2: 41F9 0000 AE2C      lea     $ae2c.l, A0
0069A8: 1039 0010 B5D5      move.b  $10b5d5.l, D0
0069AE: B050                cmp.w   (A0), D0
0069B0: 6F04                ble     $69b6
0069B2: 9050                sub.w   (A0), D0
0069B4: 60F8                bra     $69ae
0069B6: E348                lsl.w   #1, D0
0069B8: 3030 0002           move.w  ($2,A0,D0.w), D0
0069BC: 2A6E 000C           movea.l ($c,A6), A5
0069C0: 3B40 0002           move.w  D0, ($2,A5)
0069C4: 33C0 0010 B660      move.w  D0, $10b660.l
0069CA: 13FC 0000 0010 B67F move.b  #$0, $10b67f.l
0069D2: 33FC FFFA 0010 B680 move.w  #$fffa, $10b680.l
0069DA: 33FC FFF5 0010 B682 move.w  #$fff5, $10b682.l
0069E2: 323C 0066           move.w  #$66, D1
0069E6: 4E4D                trap    #$d
0069E8: 3E3C 0007           move.w  #$7, D7
0069EC: 2C79 0010 94A8      movea.l $1094a8.l, A6
0069F2: 3039 0010 94B2      move.w  $1094b2.l, D0
0069F8: 41FA 0014           lea     ($6a0e,PC), A0
0069FC: 2D88 0004           move.l  A0, ($4,A6,D0.w)
006A00: 0247 0FFF           andi.w  #$fff, D7
006A04: 0047 5000           ori.w   #$5000, D7
006A08: 3D87 0000           move.w  D7, (A6,D0.w)
006A0C: 4E75                rts
006A0E: 2C79 0010 94A8      movea.l $1094a8.l, A6
006A14: 3039 0010 94B2      move.w  $1094b2.l, D0
006A1A: 41FA 0014           lea     ($6a30,PC), A0
006A1E: 2D88 0004           move.l  A0, ($4,A6,D0.w)
006A22: 0236 007F 0000      andi.b  #$7f, (A6,D0.w)
006A28: 0036 0040 0000      ori.b   #$40, (A6,D0.w)
006A2E: 4E75                rts
006A30: 323C 0066           move.w  #$66, D1
006A34: 4E4F                trap    #$f
006A36: 8040                or.w    D0, D0
006A38: 66D4                bne     $6a0e
006A3A: 0C50 C000           cmpi.w  #-$4000, (A0)
006A3E: 66CE                bne     $6a0e
006A40: 33FC 00CC 0010 B4CE move.w  #$cc, $10b4ce.l
006A48: 33FC 0600 0010 B74A move.w  #$600, $10b74a.l
006A50: 13FC 00FF 0010 B744 move.b  #$ff, $10b744.l
006A58: 41F9 0010 A4CA      lea     $10a4ca.l, A0
006A5E: 43F9 0010 ACCA      lea     $10acca.l, A1
006A64: 7200                moveq   #$0, D1
006A66: 20C1                move.l  D1, (A0)+
006A68: 22C1                move.l  D1, (A1)+
006A6A: 20C1                move.l  D1, (A0)+
006A6C: 22C1                move.l  D1, (A1)+
006A6E: 20C1                move.l  D1, (A0)+
006A70: 22C1                move.l  D1, (A1)+
006A72: 20C1                move.l  D1, (A0)+
006A74: 22C1                move.l  D1, (A1)+
006A76: 20C1                move.l  D1, (A0)+
006A78: 22C1                move.l  D1, (A1)+
006A7A: 20C1                move.l  D1, (A0)+
006A7C: 22C1                move.l  D1, (A1)+
006A7E: 20C1                move.l  D1, (A0)+
006A80: 22C1                move.l  D1, (A1)+
006A82: 20C1                move.l  D1, (A0)+
006A84: 22C1                move.l  D1, (A1)+
006A86: 20C1                move.l  D1, (A0)+
006A88: 22C1                move.l  D1, (A1)+
006A8A: 20C1                move.l  D1, (A0)+
006A8C: 22C1                move.l  D1, (A1)+
006A8E: 20C1                move.l  D1, (A0)+
006A90: 22C1                move.l  D1, (A1)+
006A92: 20C1                move.l  D1, (A0)+
006A94: 22C1                move.l  D1, (A1)+
006A96: 20C1                move.l  D1, (A0)+
006A98: 22C1                move.l  D1, (A1)+
006A9A: 20C1                move.l  D1, (A0)+
006A9C: 22C1                move.l  D1, (A1)+
006A9E: 20C1                move.l  D1, (A0)+
006AA0: 22C1                move.l  D1, (A1)+
006AA2: 20C1                move.l  D1, (A0)+
006AA4: 22C1                move.l  D1, (A1)+
006AA6: 20C1                move.l  D1, (A0)+
006AA8: 22C1                move.l  D1, (A1)+
006AAA: 20C1                move.l  D1, (A0)+
006AAC: 22C1                move.l  D1, (A1)+
006AAE: 20C1                move.l  D1, (A0)+
006AB0: 22C1                move.l  D1, (A1)+
006AB2: 20C1                move.l  D1, (A0)+
006AB4: 22C1                move.l  D1, (A1)+
006AB6: 20C1                move.l  D1, (A0)+
006AB8: 22C1                move.l  D1, (A1)+
006ABA: 20C1                move.l  D1, (A0)+
006ABC: 22C1                move.l  D1, (A1)+
006ABE: 20C1                move.l  D1, (A0)+
006AC0: 22C1                move.l  D1, (A1)+
006AC2: 20C1                move.l  D1, (A0)+
006AC4: 22C1                move.l  D1, (A1)+
006AC6: 20C1                move.l  D1, (A0)+
006AC8: 22C1                move.l  D1, (A1)+
006ACA: 20C1                move.l  D1, (A0)+
006ACC: 22C1                move.l  D1, (A1)+
006ACE: 20C1                move.l  D1, (A0)+
006AD0: 22C1                move.l  D1, (A1)+
006AD2: 20C1                move.l  D1, (A0)+
006AD4: 22C1                move.l  D1, (A1)+
006AD6: 20C1                move.l  D1, (A0)+
006AD8: 22C1                move.l  D1, (A1)+
006ADA: 20C1                move.l  D1, (A0)+
006ADC: 22C1                move.l  D1, (A1)+
006ADE: 20C1                move.l  D1, (A0)+
006AE0: 22C1                move.l  D1, (A1)+
006AE2: 20C1                move.l  D1, (A0)+
006AE4: 22C1                move.l  D1, (A1)+
006AE6: 20C1                move.l  D1, (A0)+
006AE8: 22C1                move.l  D1, (A1)+
006AEA: 20C1                move.l  D1, (A0)+
006AEC: 22C1                move.l  D1, (A1)+
006AEE: 20C1                move.l  D1, (A0)+
006AF0: 22C1                move.l  D1, (A1)+
006AF2: 20C1                move.l  D1, (A0)+
006AF4: 22C1                move.l  D1, (A1)+
006AF6: 20C1                move.l  D1, (A0)+
006AF8: 22C1                move.l  D1, (A1)+
006AFA: 20C1                move.l  D1, (A0)+
006AFC: 22C1                move.l  D1, (A1)+
006AFE: 20C1                move.l  D1, (A0)+
006B00: 22C1                move.l  D1, (A1)+
006B02: 20C1                move.l  D1, (A0)+
006B04: 22C1                move.l  D1, (A1)+
006B06: 20C1                move.l  D1, (A0)+
006B08: 22C1                move.l  D1, (A1)+
006B0A: 20C1                move.l  D1, (A0)+
006B0C: 22C1                move.l  D1, (A1)+
006B0E: 20C1                move.l  D1, (A0)+
006B10: 22C1                move.l  D1, (A1)+
006B12: 20C1                move.l  D1, (A0)+
006B14: 22C1                move.l  D1, (A1)+
006B16: 20C1                move.l  D1, (A0)+
006B18: 22C1                move.l  D1, (A1)+
006B1A: 20C1                move.l  D1, (A0)+
006B1C: 22C1                move.l  D1, (A1)+
006B1E: 20C1                move.l  D1, (A0)+
006B20: 22C1                move.l  D1, (A1)+
006B22: 20C1                move.l  D1, (A0)+
006B24: 22C1                move.l  D1, (A1)+
006B26: 20C1                move.l  D1, (A0)+
006B28: 22C1                move.l  D1, (A1)+
006B2A: 20C1                move.l  D1, (A0)+
006B2C: 22C1                move.l  D1, (A1)+
006B2E: 20C1                move.l  D1, (A0)+
006B30: 22C1                move.l  D1, (A1)+
006B32: 20C1                move.l  D1, (A0)+
006B34: 22C1                move.l  D1, (A1)+
006B36: 20C1                move.l  D1, (A0)+
006B38: 22C1                move.l  D1, (A1)+
006B3A: 20C1                move.l  D1, (A0)+
006B3C: 22C1                move.l  D1, (A1)+
006B3E: 20C1                move.l  D1, (A0)+
006B40: 22C1                move.l  D1, (A1)+
006B42: 20C1                move.l  D1, (A0)+
006B44: 22C1                move.l  D1, (A1)+
006B46: 20C1                move.l  D1, (A0)+
006B48: 22C1                move.l  D1, (A1)+
006B4A: 20C1                move.l  D1, (A0)+
006B4C: 22C1                move.l  D1, (A1)+
006B4E: 20C1                move.l  D1, (A0)+
006B50: 22C1                move.l  D1, (A1)+
006B52: 20C1                move.l  D1, (A0)+
006B54: 22C1                move.l  D1, (A1)+
006B56: 20C1                move.l  D1, (A0)+
006B58: 22C1                move.l  D1, (A1)+
006B5A: 20C1                move.l  D1, (A0)+
006B5C: 22C1                move.l  D1, (A1)+
006B5E: 20C1                move.l  D1, (A0)+
006B60: 22C1                move.l  D1, (A1)+
006B62: 20C1                move.l  D1, (A0)+
006B64: 22C1                move.l  D1, (A1)+
006B66: 20C1                move.l  D1, (A0)+
006B68: 22C1                move.l  D1, (A1)+
006B6A: 20C1                move.l  D1, (A0)+
006B6C: 22C1                move.l  D1, (A1)+
006B6E: 20C1                move.l  D1, (A0)+
006B70: 22C1                move.l  D1, (A1)+
006B72: 20C1                move.l  D1, (A0)+
006B74: 22C1                move.l  D1, (A1)+
006B76: 20C1                move.l  D1, (A0)+
006B78: 22C1                move.l  D1, (A1)+
006B7A: 20C1                move.l  D1, (A0)+
006B7C: 22C1                move.l  D1, (A1)+
006B7E: 20C1                move.l  D1, (A0)+
006B80: 22C1                move.l  D1, (A1)+
006B82: 20C1                move.l  D1, (A0)+
006B84: 22C1                move.l  D1, (A1)+
006B86: 20C1                move.l  D1, (A0)+
006B88: 22C1                move.l  D1, (A1)+
006B8A: 20C1                move.l  D1, (A0)+
006B8C: 22C1                move.l  D1, (A1)+
006B8E: 20C1                move.l  D1, (A0)+
006B90: 22C1                move.l  D1, (A1)+
006B92: 20C1                move.l  D1, (A0)+
006B94: 22C1                move.l  D1, (A1)+
006B96: 20C1                move.l  D1, (A0)+
006B98: 22C1                move.l  D1, (A1)+
006B9A: 20C1                move.l  D1, (A0)+
006B9C: 22C1                move.l  D1, (A1)+
006B9E: 20C1                move.l  D1, (A0)+
006BA0: 22C1                move.l  D1, (A1)+
006BA2: 20C1                move.l  D1, (A0)+
006BA4: 22C1                move.l  D1, (A1)+
006BA6: 20C1                move.l  D1, (A0)+
006BA8: 22C1                move.l  D1, (A1)+
006BAA: 20C1                move.l  D1, (A0)+
006BAC: 22C1                move.l  D1, (A1)+
006BAE: 20C1                move.l  D1, (A0)+
006BB0: 22C1                move.l  D1, (A1)+
006BB2: 20C1                move.l  D1, (A0)+
006BB4: 22C1                move.l  D1, (A1)+
006BB6: 20C1                move.l  D1, (A0)+
006BB8: 22C1                move.l  D1, (A1)+
006BBA: 20C1                move.l  D1, (A0)+
006BBC: 22C1                move.l  D1, (A1)+
006BBE: 20C1                move.l  D1, (A0)+
006BC0: 22C1                move.l  D1, (A1)+
006BC2: 20C1                move.l  D1, (A0)+
006BC4: 22C1                move.l  D1, (A1)+
006BC6: 20C1                move.l  D1, (A0)+
006BC8: 22C1                move.l  D1, (A1)+
006BCA: 20C1                move.l  D1, (A0)+
006BCC: 22C1                move.l  D1, (A1)+
006BCE: 20C1                move.l  D1, (A0)+
006BD0: 22C1                move.l  D1, (A1)+
006BD2: 20C1                move.l  D1, (A0)+
006BD4: 22C1                move.l  D1, (A1)+
006BD6: 20C1                move.l  D1, (A0)+
006BD8: 22C1                move.l  D1, (A1)+
006BDA: 20C1                move.l  D1, (A0)+
006BDC: 22C1                move.l  D1, (A1)+
006BDE: 20C1                move.l  D1, (A0)+
006BE0: 22C1                move.l  D1, (A1)+
006BE2: 20C1                move.l  D1, (A0)+
006BE4: 22C1                move.l  D1, (A1)+
006BE6: 20C1                move.l  D1, (A0)+
006BE8: 22C1                move.l  D1, (A1)+
006BEA: 20C1                move.l  D1, (A0)+
006BEC: 22C1                move.l  D1, (A1)+
006BEE: 20C1                move.l  D1, (A0)+
006BF0: 22C1                move.l  D1, (A1)+
006BF2: 20C1                move.l  D1, (A0)+
006BF4: 22C1                move.l  D1, (A1)+
006BF6: 20C1                move.l  D1, (A0)+
006BF8: 22C1                move.l  D1, (A1)+
006BFA: 20C1                move.l  D1, (A0)+
006BFC: 22C1                move.l  D1, (A1)+
006BFE: 20C1                move.l  D1, (A0)+
006C00: 22C1                move.l  D1, (A1)+
006C02: 20C1                move.l  D1, (A0)+
006C04: 22C1                move.l  D1, (A1)+
006C06: 20C1                move.l  D1, (A0)+
006C08: 22C1                move.l  D1, (A1)+
006C0A: 20C1                move.l  D1, (A0)+
006C0C: 22C1                move.l  D1, (A1)+
006C0E: 20C1                move.l  D1, (A0)+
006C10: 22C1                move.l  D1, (A1)+
006C12: 20C1                move.l  D1, (A0)+
006C14: 22C1                move.l  D1, (A1)+
006C16: 20C1                move.l  D1, (A0)+
006C18: 22C1                move.l  D1, (A1)+
006C1A: 20C1                move.l  D1, (A0)+
006C1C: 22C1                move.l  D1, (A1)+
006C1E: 20C1                move.l  D1, (A0)+
006C20: 22C1                move.l  D1, (A1)+
006C22: 20C1                move.l  D1, (A0)+
006C24: 22C1                move.l  D1, (A1)+
006C26: 20C1                move.l  D1, (A0)+
006C28: 22C1                move.l  D1, (A1)+
006C2A: 20C1                move.l  D1, (A0)+
006C2C: 22C1                move.l  D1, (A1)+
006C2E: 20C1                move.l  D1, (A0)+
006C30: 22C1                move.l  D1, (A1)+
006C32: 20C1                move.l  D1, (A0)+
006C34: 22C1                move.l  D1, (A1)+
006C36: 20C1                move.l  D1, (A0)+
006C38: 22C1                move.l  D1, (A1)+
006C3A: 20C1                move.l  D1, (A0)+
006C3C: 22C1                move.l  D1, (A1)+
006C3E: 20C1                move.l  D1, (A0)+
006C40: 22C1                move.l  D1, (A1)+
006C42: 20C1                move.l  D1, (A0)+
006C44: 22C1                move.l  D1, (A1)+
006C46: 20C1                move.l  D1, (A0)+
006C48: 22C1                move.l  D1, (A1)+
006C4A: 20C1                move.l  D1, (A0)+
006C4C: 22C1                move.l  D1, (A1)+
006C4E: 20C1                move.l  D1, (A0)+
006C50: 22C1                move.l  D1, (A1)+
006C52: 20C1                move.l  D1, (A0)+
006C54: 22C1                move.l  D1, (A1)+
006C56: 20C1                move.l  D1, (A0)+
006C58: 22C1                move.l  D1, (A1)+
006C5A: 20C1                move.l  D1, (A0)+
006C5C: 22C1                move.l  D1, (A1)+
006C5E: 20C1                move.l  D1, (A0)+
006C60: 22C1                move.l  D1, (A1)+
006C62: 20C1                move.l  D1, (A0)+
006C64: 22C1                move.l  D1, (A1)+
006C66: 20C1                move.l  D1, (A0)+
006C68: 22C1                move.l  D1, (A1)+
006C6A: 20C1                move.l  D1, (A0)+
006C6C: 22C1                move.l  D1, (A1)+
006C6E: 20C1                move.l  D1, (A0)+
006C70: 22C1                move.l  D1, (A1)+
006C72: 20C1                move.l  D1, (A0)+
006C74: 22C1                move.l  D1, (A1)+
006C76: 20C1                move.l  D1, (A0)+
006C78: 22C1                move.l  D1, (A1)+
006C7A: 20C1                move.l  D1, (A0)+
006C7C: 22C1                move.l  D1, (A1)+
006C7E: 20C1                move.l  D1, (A0)+
006C80: 22C1                move.l  D1, (A1)+
006C82: 20C1                move.l  D1, (A0)+
006C84: 22C1                move.l  D1, (A1)+
006C86: 20C1                move.l  D1, (A0)+
006C88: 22C1                move.l  D1, (A1)+
006C8A: 20C1                move.l  D1, (A0)+
006C8C: 22C1                move.l  D1, (A1)+
006C8E: 20C1                move.l  D1, (A0)+
006C90: 22C1                move.l  D1, (A1)+
006C92: 20C1                move.l  D1, (A0)+
006C94: 22C1                move.l  D1, (A1)+
006C96: 20C1                move.l  D1, (A0)+
006C98: 22C1                move.l  D1, (A1)+
006C9A: 20C1                move.l  D1, (A0)+
006C9C: 22C1                move.l  D1, (A1)+
006C9E: 20C1                move.l  D1, (A0)+
006CA0: 22C1                move.l  D1, (A1)+
006CA2: 20C1                move.l  D1, (A0)+
006CA4: 22C1                move.l  D1, (A1)+
006CA6: 20C1                move.l  D1, (A0)+
006CA8: 22C1                move.l  D1, (A1)+
006CAA: 20C1                move.l  D1, (A0)+
006CAC: 22C1                move.l  D1, (A1)+
006CAE: 20C1                move.l  D1, (A0)+
006CB0: 22C1                move.l  D1, (A1)+
006CB2: 20C1                move.l  D1, (A0)+
006CB4: 22C1                move.l  D1, (A1)+
006CB6: 20C1                move.l  D1, (A0)+
006CB8: 22C1                move.l  D1, (A1)+
006CBA: 20C1                move.l  D1, (A0)+
006CBC: 22C1                move.l  D1, (A1)+
006CBE: 20C1                move.l  D1, (A0)+
006CC0: 22C1                move.l  D1, (A1)+
006CC2: 20C1                move.l  D1, (A0)+
006CC4: 22C1                move.l  D1, (A1)+
006CC6: 20C1                move.l  D1, (A0)+
006CC8: 22C1                move.l  D1, (A1)+
006CCA: 20C1                move.l  D1, (A0)+
006CCC: 22C1                move.l  D1, (A1)+
006CCE: 20C1                move.l  D1, (A0)+
006CD0: 22C1                move.l  D1, (A1)+
006CD2: 20C1                move.l  D1, (A0)+
006CD4: 22C1                move.l  D1, (A1)+
006CD6: 20C1                move.l  D1, (A0)+
006CD8: 22C1                move.l  D1, (A1)+
006CDA: 20C1                move.l  D1, (A0)+
006CDC: 22C1                move.l  D1, (A1)+
006CDE: 20C1                move.l  D1, (A0)+
006CE0: 22C1                move.l  D1, (A1)+
006CE2: 20C1                move.l  D1, (A0)+
006CE4: 22C1                move.l  D1, (A1)+
006CE6: 20C1                move.l  D1, (A0)+
006CE8: 22C1                move.l  D1, (A1)+
006CEA: 20C1                move.l  D1, (A0)+
006CEC: 22C1                move.l  D1, (A1)+
006CEE: 20C1                move.l  D1, (A0)+
006CF0: 22C1                move.l  D1, (A1)+
006CF2: 20C1                move.l  D1, (A0)+
006CF4: 22C1                move.l  D1, (A1)+
006CF6: 20C1                move.l  D1, (A0)+
006CF8: 22C1                move.l  D1, (A1)+
006CFA: 20C1                move.l  D1, (A0)+
006CFC: 22C1                move.l  D1, (A1)+
006CFE: 20C1                move.l  D1, (A0)+
006D00: 22C1                move.l  D1, (A1)+
006D02: 20C1                move.l  D1, (A0)+
006D04: 22C1                move.l  D1, (A1)+
006D06: 20C1                move.l  D1, (A0)+
006D08: 22C1                move.l  D1, (A1)+
006D0A: 20C1                move.l  D1, (A0)+
