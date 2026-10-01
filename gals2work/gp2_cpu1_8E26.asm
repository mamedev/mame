008E26: 41F9 0038 0000      lea     $380000.l, A0
008E2C: 43F9 0038 8000      lea     $388000.l, A1
008E32: 45F9 0010 0C26      lea     $100c26.l, A2
008E38: 4BF9 0010 0BA6      lea     $100ba6.l, A5
008E3E: 7000                moveq   #$0, D0
008E40: 3039 0010 0CA6      move.w  $100ca6.l, D0
008E46: 4840                swap    D0
008E48: E688                lsr.l   #3, D0
008E4A: 7800                moveq   #$0, D4
008E4C: 3839 0010 0CA6      move.w  $100ca6.l, D4
008E52: EB8C                lsl.l   #5, D4
008E54: 3A3C 000F           move.w  #$f, D5
008E58: 4A72 4800           tst.w   (A2,D4.l)
008E5C: 6600 002E           bne     $8e8c
008E60: 47F1 0800           lea     (A1,D0.l), A3
008E64: 49F0 0800           lea     (A0,D0.l), A4
008E68: 3C35 4800           move.w  (A5,D4.l), D6
008E6C: E84E                lsr.w   #4, D6
008E6E: 28DB                move.l  (A3)+, (A4)+
008E70: 28DB                move.l  (A3)+, (A4)+
008E72: 28DB                move.l  (A3)+, (A4)+
008E74: 28DB                move.l  (A3)+, (A4)+
008E76: 28DB                move.l  (A3)+, (A4)+
008E78: 28DB                move.l  (A3)+, (A4)+
008E7A: 28DB                move.l  (A3)+, (A4)+
008E7C: 28DB                move.l  (A3)+, (A4)+
008E7E: 51CE FFEE           dbra    D6, $8e6e
008E82: 0680 0000 0200      addi.l  #$200, D0
008E88: 6000 011A           bra     $8fa4
008E8C: 48E7 8000           movem.l D0, -(A7)
008E90: 3C35 4800           move.w  (A5,D4.l), D6
008E94: E84E                lsr.w   #4, D6
008E96: 3231 0800           move.w  (A1,D0.l), D1
008E9A: B270 0800           cmp.w   (A0,D0.l), D1
008E9E: 6704                beq     $8ea4
008EA0: 6100 0158           bsr     $8ffa
008EA4: 5480                addq.l  #2, D0
008EA6: 3231 0800           move.w  (A1,D0.l), D1
008EAA: B270 0800           cmp.w   (A0,D0.l), D1
008EAE: 6704                beq     $8eb4
008EB0: 6100 0148           bsr     $8ffa
008EB4: 5480                addq.l  #2, D0
008EB6: 3231 0800           move.w  (A1,D0.l), D1
008EBA: B270 0800           cmp.w   (A0,D0.l), D1
008EBE: 6704                beq     $8ec4
008EC0: 6100 0138           bsr     $8ffa
008EC4: 5480                addq.l  #2, D0
008EC6: 3231 0800           move.w  (A1,D0.l), D1
008ECA: B270 0800           cmp.w   (A0,D0.l), D1
008ECE: 6704                beq     $8ed4
008ED0: 6100 0128           bsr     $8ffa
008ED4: 5480                addq.l  #2, D0
008ED6: 3231 0800           move.w  (A1,D0.l), D1
008EDA: B270 0800           cmp.w   (A0,D0.l), D1
008EDE: 6704                beq     $8ee4
008EE0: 6100 0118           bsr     $8ffa
008EE4: 5480                addq.l  #2, D0
008EE6: 3231 0800           move.w  (A1,D0.l), D1
008EEA: B270 0800           cmp.w   (A0,D0.l), D1
008EEE: 6704                beq     $8ef4
008EF0: 6100 0108           bsr     $8ffa
008EF4: 5480                addq.l  #2, D0
008EF6: 3231 0800           move.w  (A1,D0.l), D1
008EFA: B270 0800           cmp.w   (A0,D0.l), D1
008EFE: 6704                beq     $8f04
008F00: 6100 00F8           bsr     $8ffa
008F04: 5480                addq.l  #2, D0
008F06: 3231 0800           move.w  (A1,D0.l), D1
008F0A: B270 0800           cmp.w   (A0,D0.l), D1
008F0E: 6704                beq     $8f14
008F10: 6100 00E8           bsr     $8ffa
008F14: 5480                addq.l  #2, D0
008F16: 3231 0800           move.w  (A1,D0.l), D1
008F1A: B270 0800           cmp.w   (A0,D0.l), D1
008F1E: 6704                beq     $8f24
008F20: 6100 00D8           bsr     $8ffa
008F24: 5480                addq.l  #2, D0
008F26: 3231 0800           move.w  (A1,D0.l), D1
008F2A: B270 0800           cmp.w   (A0,D0.l), D1
008F2E: 6704                beq     $8f34
008F30: 6100 00C8           bsr     $8ffa
008F34: 5480                addq.l  #2, D0
008F36: 3231 0800           move.w  (A1,D0.l), D1
008F3A: B270 0800           cmp.w   (A0,D0.l), D1
008F3E: 6704                beq     $8f44
008F40: 6100 00B8           bsr     $8ffa
008F44: 5480                addq.l  #2, D0
008F46: 3231 0800           move.w  (A1,D0.l), D1
008F4A: B270 0800           cmp.w   (A0,D0.l), D1
008F4E: 6704                beq     $8f54
008F50: 6100 00A8           bsr     $8ffa
008F54: 5480                addq.l  #2, D0
008F56: 3231 0800           move.w  (A1,D0.l), D1
008F5A: B270 0800           cmp.w   (A0,D0.l), D1
008F5E: 6704                beq     $8f64
008F60: 6100 0098           bsr     $8ffa
008F64: 5480                addq.l  #2, D0
008F66: 3231 0800           move.w  (A1,D0.l), D1
008F6A: B270 0800           cmp.w   (A0,D0.l), D1
008F6E: 6704                beq     $8f74
008F70: 6100 0088           bsr     $8ffa
008F74: 5480                addq.l  #2, D0
008F76: 3231 0800           move.w  (A1,D0.l), D1
008F7A: B270 0800           cmp.w   (A0,D0.l), D1
008F7E: 6704                beq     $8f84
008F80: 6100 0078           bsr     $8ffa
008F84: 5480                addq.l  #2, D0
008F86: 3231 0800           move.w  (A1,D0.l), D1
008F8A: B270 0800           cmp.w   (A0,D0.l), D1
008F8E: 6704                beq     $8f94
008F90: 6100 0068           bsr     $8ffa
008F94: 5480                addq.l  #2, D0
008F96: 51CE FEFE           dbra    D6, $8e96
008F9A: 4CDF 0001           movem.l (A7)+, D0
008F9E: 0680 0000 0200      addi.l  #$200, D0
008FA4: 5484                addq.l  #2, D4
008FA6: 51CD FEB0           dbra    D5, $8e58
008FAA: 5279 0010 0CA6      addq.w  #1, $100ca6.l
008FB0: 0279 0003 0010 0CA6 andi.w  #$3, $100ca6.l
008FB8: 4E75                rts
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
