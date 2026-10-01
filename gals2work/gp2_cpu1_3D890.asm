03D890: 0016 67C8           ori.b   #$c8, (A6)
03D894: 608E                bra     $3d824
03D896: 4AB9 0010 C43A      tst.l   $10c43a.l
03D89C: 6602                bne     $3d8a0
03D89E: 4E75                rts
03D8A0: 48E7 FFFE           movem.l D0-D7/A0-A6, -(A7)
03D8A4: 2C79 0010 C434      movea.l $10c434.l, A6
03D8AA: 2A79 0010 C43A      movea.l $10c43a.l, A5
03D8B0: 4ED5                jmp     (A5)
03D8B2: 4279 0010 C438      clr.w   $10c438.l
03D8B8: 3C39 0010 C438      move.w  $10c438.l, D6
03D8BE: 2A7B 6004           movea.l ($4,PC,D6.w), A5
03D8C2: 4ED5                jmp     (A5)
03D8C4: 0003 D8E8           ori.b   #$e8, D3
03D8C8: 0003 D96E           ori.b   #$6e, D3
03D8CC: 0003 D9A6           ori.b   #$a6, D3
03D8D0: 0003 DC7E           ori.b   #$7e, D3
03D8D4: 0003 DD82           ori.b   #$82, D3
03D8D8: 0003 DDA2           ori.b   #$a2, D3
03D8DC: 0003 DD92           ori.b   #$92, D3
03D8E0: 0003 DD4A           ori.b   #$4a, D3
03D8E4: 0003 DE44           ori.b   #$44, D3
03D8E8: 4239 0010 C446      clr.b   $10c446.l
03D8EE: 41F9 0010 C330      lea     $10c330.l, A0
03D8F4: 3010                move.w  (A0), D0
03D8F6: 6700 001A           beq     $3d912
03D8FA: 4250                clr.w   (A0)
03D8FC: 0C00 0080           cmpi.b  #-$80, D0
03D900: 6600 0006           bne     $3d908
03D904: 6000 005C           bra     $3d962
03D908: 13C0 0010 C446      move.b  D0, $10c446.l
03D90E: 6000 0046           bra     $3d956
03D912: 1028 0002           move.b  ($2,A0), D0
03D916: B028 0003           cmp.b   ($3,A0), D0
03D91A: 6700 003A           beq     $3d956
03D91E: 5228 0002           addq.b  #1, ($2,A0)
03D922: 0228 007F 0002      andi.b  #$7f, ($2,A0)
03D928: 1230 0004           move.b  ($4,A0,D0.w), D1
03D92C: 6700 0028           beq     $3d956
03D930: 4230 0004           clr.b   ($4,A0,D0.w)
03D934: 0839 0000 0010 8F8B btst    #$0, $108f8b.l
03D93C: 670A                beq     $3d948
03D93E: 0839 0006 0010 8F8A btst    #$6, $108f8a.l
03D946: 660E                bne     $3d956
03D948: 0C01 0080           cmpi.b  #-$80, D1
03D94C: 6700 0014           beq     $3d962
03D950: 13C1 0010 C446      move.b  D1, $10c446.l
03D956: 33FC 0008 0010 C438 move.w  #$8, $10c438.l
03D95E: 4EFA FF58           jmp     ($3d8b8,PC)
03D962: 33FC 0004 0010 C438 move.w  #$4, $10c438.l
03D96A: 4EFA FF4C           jmp     ($3d8b8,PC)
03D96E: 33FC 0078 00C0 0000 move.w  #$78, $c00000.l
03D976: 33FC 0078 00C4 0000 move.w  #$78, $c40000.l
03D97E: 41F9 0010 C48A      lea     $10c48a.l, A0
03D984: 7023                moveq   #$23, D0
03D986: 4298                clr.l   (A0)+
03D988: 51C8 FFFC           dbra    D0, $3d986
03D98C: 41F9 0010 C446      lea     $10c446.l, A0
03D992: 7010                moveq   #$10, D0
03D994: 4298                clr.l   (A0)+
03D996: 51C8 FFFC           dbra    D0, $3d994
03D99A: 33FC 0000 0010 C438 move.w  #$0, $10c438.l
03D9A2: 4EFA FF14           jmp     ($3d8b8,PC)
03D9A6: 1239 0010 C446      move.b  $10c446.l, D1
03D9AC: 6700 01DE           beq     $3db8c
03D9B0: 13C1 0010 C51A      move.b  D1, $10c51a.l
03D9B6: 4239 0010 C446      clr.b   $10c446.l
03D9BC: 0241 00FF           andi.w  #$ff, D1
03D9C0: D241                add.w   D1, D1
03D9C2: 3436 1000           move.w  (A6,D1.w), D2
03D9C6: 43F6 2000           lea     (A6,D2.w), A1
03D9CA: 7200                moveq   #$0, D1
03D9CC: 1219                move.b  (A1)+, D1
03D9CE: 13C1 0010 C51B      move.b  D1, $10c51b.l
03D9D4: 1219                move.b  (A1)+, D1
03D9D6: 13C1 0010 C51C      move.b  D1, $10c51c.l
03D9DC: 45F9 0010 C51E      lea     $10c51e.l, A2
03D9E2: 5301                subq.b  #1, D1
03D9E4: 3419                move.w  (A1)+, D2
03D9E6: 47F6 2000           lea     (A6,D2.w), A3
03D9EA: 24CB                move.l  A3, (A2)+
03D9EC: 51C9 FFF6           dbra    D1, $3d9e4
03D9F0: 43F9 0010 C51E      lea     $10c51e.l, A1
03D9F6: 2451                movea.l (A1), A2
03D9F8: 3212                move.w  (A2), D1
03D9FA: 1436 1000           move.b  (A6,D1.w), D2
03D9FE: 13C2 0010 C52F      move.b  D2, $10c52f.l
03DA04: 1436 1001           move.b  ($1,A6,D1.w), D2
03DA08: 13C2 0010 C530      move.b  D2, $10c530.l
03DA0E: 3436 1002           move.w  ($2,A6,D1.w), D2
03DA12: 33C2 0010 C532      move.w  D2, $10c532.l
03DA18: 1436 1004           move.b  ($4,A6,D1.w), D2
03DA1C: 13C2 0010 C534      move.b  D2, $10c534.l
03DA22: 1436 1005           move.b  ($5,A6,D1.w), D2
03DA26: 13C2 0010 C52E      move.b  D2, $10c52e.l
03DA2C: 0839 0007 0010 C51B btst    #$7, $10c51b.l
03DA34: 6700 0060           beq     $3da96
03DA38: 45F9 0010 C48C      lea     $10c48c.l, A2
03DA3E: 7203                moveq   #$3, D1
03DA40: 7440                moveq   #$40, D2
03DA42: 0812 0007           btst    #$7, (A2)
03DA46: 6700 0018           beq     $3da60
03DA4A: 45EA FFFE           lea     (-$2,A2), A2
03DA4E: 7608                moveq   #$8, D3
03DA50: 425A                clr.w   (A2)+
03DA52: 51CB FFFC           dbra    D3, $3da50
03DA56: 33C2 00C0 0000      move.w  D2, $c00000.l
03DA5C: 45EA FFF0           lea     (-$10,A2), A2
03DA60: 45EA 0012           lea     ($12,A2), A2
03DA64: E24A                lsr.w   #1, D2
03DA66: 51C9 FFDA           dbra    D1, $3da42
03DA6A: 7203                moveq   #$3, D1
03DA6C: 7440                moveq   #$40, D2
03DA6E: 0812 0007           btst    #$7, (A2)
03DA72: 6700 0018           beq     $3da8c
03DA76: 45EA FFFE           lea     (-$2,A2), A2
03DA7A: 7608                moveq   #$8, D3
03DA7C: 425A                clr.w   (A2)+
03DA7E: 51CB FFFC           dbra    D3, $3da7c
03DA82: 33C2 00C4 0000      move.w  D2, $c40000.l
03DA88: 45EA FFF0           lea     (-$10,A2), A2
03DA8C: 45EA 0012           lea     ($12,A2), A2
