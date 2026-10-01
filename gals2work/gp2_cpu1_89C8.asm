0089C8: 1039 0078 0003      move.b  $780003.l, D0
0089CE: 4600                not.b   D0
0089D0: 13C0 0010 0EB6      move.b  D0, $100eb6.l
0089D6: 1039 0078 0001      move.b  $780001.l, D0
0089DC: 4600                not.b   D0
0089DE: 13C0 0010 0EB7      move.b  D0, $100eb7.l
0089E4: 1039 0078 0000      move.b  $780000.l, D0
0089EA: 4600                not.b   D0
0089EC: 13C0 0010 0EB8      move.b  D0, $100eb8.l
0089F2: 1039 0078 0002      move.b  $780002.l, D0
0089F8: 4600                not.b   D0
0089FA: 13C0 0010 0EB9      move.b  D0, $100eb9.l
008A00: 1039 0078 0004      move.b  $780004.l, D0
008A06: 4600                not.b   D0
008A08: 13C0 0010 0EBA      move.b  D0, $100eba.l
008A0E: 1039 0078 0006      move.b  $780006.l, D0
008A14: 4600                not.b   D0
008A16: 13C0 0010 0EBB      move.b  D0, $100ebb.l
008A1C: 1039 0010 0EB8      move.b  $100eb8.l, D0
008A22: 1239 0010 0EBC      move.b  $100ebc.l, D1
008A28: B101                eor.b   D0, D1
008A2A: C200                and.b   D0, D1
008A2C: 13C1 0010 0EC0      move.b  D1, $100ec0.l
008A32: 13C0 0010 0EBC      move.b  D0, $100ebc.l
008A38: 1039 0010 0EB9      move.b  $100eb9.l, D0
008A3E: 1239 0010 0EBD      move.b  $100ebd.l, D1
008A44: B101                eor.b   D0, D1
008A46: C200                and.b   D0, D1
008A48: 13C1 0010 0EC1      move.b  D1, $100ec1.l
008A4E: 13C0 0010 0EBD      move.b  D0, $100ebd.l
008A54: 1039 0010 0EBA      move.b  $100eba.l, D0
008A5A: 1239 0010 0EBE      move.b  $100ebe.l, D1
008A60: B101                eor.b   D0, D1
008A62: C200                and.b   D0, D1
008A64: 13C1 0010 0EC2      move.b  D1, $100ec2.l
008A6A: 13C0 0010 0EBE      move.b  D0, $100ebe.l
008A70: 1039 0010 0EBB      move.b  $100ebb.l, D0
008A76: 1239 0010 0EBF      move.b  $100ebf.l, D1
008A7C: B101                eor.b   D0, D1
008A7E: C200                and.b   D0, D1
008A80: 13C1 0010 0EC3      move.b  D1, $100ec3.l
008A86: 13C0 0010 0EBF      move.b  D0, $100ebf.l
008A8C: 4E75                rts
008A8E: 0C39 000F 0010 8F82 cmpi.b  #$f, $108f82.l
008A96: 661C                bne     $8ab4
008A98: 33FC FFFF 0010 0EC8 move.w  #$ffff, $100ec8.l
008AA0: 33FC 0001 0010 0ECA move.w  #$1, $100eca.l
008AA8: 0239 00F3 0010 08D2 andi.b  #$f3, $1008d2.l
008AB0: 6000 0138           bra     $8bea
008AB4: 0C79 0009 0010 0EC8 cmpi.w  #$9, $100ec8.l
008ABC: 6D0C                blt     $8aca
008ABE: 0239 00F3 0010 08D2 andi.b  #$f3, $1008d2.l
008AC6: 6000 0122           bra     $8bea
008ACA: 0839 0007 0010 0EC0 btst    #$7, $100ec0.l
008AD2: 6710                beq     $8ae4
008AD4: 5279 0010 0EC4      addq.w  #1, $100ec4.l
008ADA: 5279 0010 0ED4      addq.w  #1, $100ed4.l
008AE0: 6000 000A           bra     $8aec
008AE4: 0039 0004 0010 08D2 ori.b   #$4, $1008d2.l
008AEC: 0839 0007 0010 0EC1 btst    #$7, $100ec1.l
008AF4: 6710                beq     $8b06
008AF6: 5279 0010 0EC6      addq.w  #1, $100ec6.l
008AFC: 5279 0010 0ED6      addq.w  #1, $100ed6.l
008B02: 6000 000A           bra     $8b0e
008B06: 0039 0008 0010 08D2 ori.b   #$8, $1008d2.l
008B0E: 41FA 00DC           lea     ($8bec,PC), A0
008B12: 7200                moveq   #$0, D1
008B14: 1239 0010 8F82      move.b  $108f82.l, D1
008B1A: E549                lsl.w   #2, D1
008B1C: 0C79 0009 0010 0EC8 cmpi.w  #$9, $100ec8.l
008B24: 6D0C                blt     $8b32
008B26: 0239 00F3 0010 08D2 andi.b  #$f3, $1008d2.l
008B2E: 6000 00BA           bra     $8bea
008B32: 7000                moveq   #$0, D0
008B34: 1030 1800           move.b  (A0,D1.l), D0
008B38: B079 0010 0EC4      cmp.w   $100ec4.l, D0
008B3E: 661E                bne     $8b5e
008B40: 9179 0010 0EC4      sub.w   D0, $100ec4.l
008B46: 1030 1801           move.b  ($1,A0,D1.l), D0
008B4A: D179 0010 0EC8      add.w   D0, $100ec8.l
008B50: D179 0010 0ECA      add.w   D0, $100eca.l
008B56: 33FC 0001 0010 C330 move.w  #$1, $10c330.l
008B5E: 0C79 0009 0010 0EC8 cmpi.w  #$9, $100ec8.l
008B66: 6D0C                blt     $8b74
008B68: 0239 00F3 0010 08D2 andi.b  #$f3, $1008d2.l
008B70: 6000 0078           bra     $8bea
008B74: 7000                moveq   #$0, D0
008B76: 1030 1802           move.b  ($2,A0,D1.l), D0
008B7A: B079 0010 0EC6      cmp.w   $100ec6.l, D0
008B80: 661E                bne     $8ba0
008B82: 9179 0010 0EC6      sub.w   D0, $100ec6.l
008B88: 1030 1803           move.b  ($3,A0,D1.l), D0
008B8C: D179 0010 0EC8      add.w   D0, $100ec8.l
008B92: D179 0010 0ECA      add.w   D0, $100eca.l
008B98: 33FC 0001 0010 C330 move.w  #$1, $10c330.l
008BA0: 0C79 0009 0010 0EC8 cmpi.w  #$9, $100ec8.l
008BA8: 6D0C                blt     $8bb6
008BAA: 0239 00F3 0010 08D2 andi.b  #$f3, $1008d2.l
008BB2: 6000 0036           bra     $8bea
008BB6: 0839 0007 0010 0EC3 btst    #$7, $100ec3.l
008BBE: 672A                beq     $8bea
008BC0: 5279 0010 0EC8      addq.w  #1, $100ec8.l
008BC6: 48E7 8080           movem.l D0/A0, -(A7)
