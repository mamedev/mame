008CF6: 4A79 0010 0ED8      tst.w   $100ed8.l
008CFC: 6700 0020           beq     $8d1e
008D00: 0039 0001 0010 08D2 ori.b   #$1, $1008d2.l
008D08: 5379 0010 0ED8      subq.w  #1, $100ed8.l
008D0E: 6600 004E           bne     $8d5e
008D12: 33FC 0008 0010 0EDC move.w  #$8, $100edc.l
008D1A: 6000 0042           bra     $8d5e
008D1E: 4A79 0010 0EDC      tst.w   $100edc.l
008D24: 6700 0014           beq     $8d3a
008D28: 0239 00FE 0010 08D2 andi.b  #$fe, $1008d2.l
008D30: 5379 0010 0EDC      subq.w  #1, $100edc.l
008D36: 6000 0026           bra     $8d5e
008D3A: 4A79 0010 0ED4      tst.w   $100ed4.l
008D40: 6700 0014           beq     $8d56
008D44: 5379 0010 0ED4      subq.w  #1, $100ed4.l
008D4A: 33FC 0008 0010 0ED8 move.w  #$8, $100ed8.l
008D52: 6000 000A           bra     $8d5e
008D56: 0239 00FE 0010 08D2 andi.b  #$fe, $1008d2.l
008D5E: 4A79 0010 0EDA      tst.w   $100eda.l
008D64: 6700 0020           beq     $8d86
008D68: 0039 0002 0010 08D2 ori.b   #$2, $1008d2.l
008D70: 5379 0010 0EDA      subq.w  #1, $100eda.l
008D76: 6600 004E           bne     $8dc6
008D7A: 33FC 0008 0010 0EDE move.w  #$8, $100ede.l
008D82: 6000 0042           bra     $8dc6
008D86: 4A79 0010 0EDE      tst.w   $100ede.l
008D8C: 6700 0014           beq     $8da2
008D90: 0239 00FD 0010 08D2 andi.b  #$fd, $1008d2.l
008D98: 5379 0010 0EDE      subq.w  #1, $100ede.l
008D9E: 6000 0026           bra     $8dc6
008DA2: 4A79 0010 0ED6      tst.w   $100ed6.l
008DA8: 6700 0014           beq     $8dbe
008DAC: 5379 0010 0ED6      subq.w  #1, $100ed6.l
008DB2: 33FC 0008 0010 0EDA move.w  #$8, $100eda.l
008DBA: 6000 000A           bra     $8dc6
008DBE: 0239 00FD 0010 08D2 andi.b  #$fd, $1008d2.l
008DC6: 4E75                rts
008DC8: 48E7 C080           movem.l D0-D1/A0, -(A7)
008DCC: 41F9 0030 2C88      lea     $302c88.l, A0
008DD2: 303C 003A           move.w  #$3a, D0
008DD6: 7200                moveq   #$0, D1
008DD8: 20C1                move.l  D1, (A0)+
008DDA: 20C1                move.l  D1, (A0)+
008DDC: 20C1                move.l  D1, (A0)+
008DDE: 20C1                move.l  D1, (A0)+
008DE0: 20C1                move.l  D1, (A0)+
008DE2: 20C1                move.l  D1, (A0)+
008DE4: 20C1                move.l  D1, (A0)+
008DE6: 20C1                move.l  D1, (A0)+
008DE8: 51C8 FFEE           dbra    D0, $8dd8
008DEC: 41F9 0010 0C86      lea     $100c86.l, A0
008DF2: 303C 0007           move.w  #$7, D0
008DF6: 7200                moveq   #$0, D1
008DF8: 20C1                move.l  D1, (A0)+
008DFA: 51C8 FFFC           dbra    D0, $8df8
008DFE: 4CDF 0103           movem.l (A7)+, D0-D1/A0
008E02: 4E75                rts
008E04: 43F9 0030 2C88      lea     $302c88.l, A1
008E0A: 303C 003A           move.w  #$3a, D0
008E0E: 7200                moveq   #$0, D1
008E10: 22C1                move.l  D1, (A1)+
008E12: 22C1                move.l  D1, (A1)+
008E14: 22C1                move.l  D1, (A1)+
008E16: 22C1                move.l  D1, (A1)+
008E18: 22C1                move.l  D1, (A1)+
008E1A: 22C1                move.l  D1, (A1)+
008E1C: 22C1                move.l  D1, (A1)+
008E1E: 22C1                move.l  D1, (A1)+
008E20: 51C8 FFEE           dbra    D0, $8e10
008E24: 4ED0                jmp     (A0)
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
