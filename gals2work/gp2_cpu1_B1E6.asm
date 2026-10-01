00B1E6: 0839 0007 0010 0EAE      btst    #$7, $100eae.l
00B1EE: 6702                     beq     $b1f2
00B1F0: 4E75                     rts
00B1F2: 0CB9 4845 4C50 0010 0100 cmpi.l  #$48454c50, $100100.l
00B1FC: 6702                     beq     $b200
00B1FE: 4E75                     rts
00B200: 3039 0010 0EEC           move.w  $100eec.l, D0
00B206: 0240 007F                andi.w  #$7f, D0
00B20A: 6702                     beq     $b20e
00B20C: 4E75                     rts
00B20E: 7600                     moveq   #$0, D3
00B210: 1239 0010 8F84           move.b  $108f84.l, D1
00B216: 13C1 0010 0F1C           move.b  D1, $100f1c.l
00B21C: 41FA 0008                lea     ($b226,PC), A0
00B220: 4EF9 0000 90CA           jmp     $90ca.l
00B226: 23C0 0010 0F0C           move.l  D0, $100f0c.l
00B22C: 0C39 0030 0010 0F0C      cmpi.b  #$30, $100f0c.l
00B234: 662C                     bne     $b262
00B236: 13FC 0020 0010 0F0C      move.b  #$20, $100f0c.l
00B23E: 0C39 0030 0010 0F0D      cmpi.b  #$30, $100f0d.l
00B246: 661A                     bne     $b262
00B248: 13FC 0020 0010 0F0D      move.b  #$20, $100f0d.l
00B250: 0C39 0030 0010 0F0E      cmpi.b  #$30, $100f0e.l
00B258: 6608                     bne     $b262
00B25A: 13FC 0020 0010 0F0E      move.b  #$20, $100f0e.l
00B262: 13FC 0048 0010 0F10      move.b  #$48, $100f10.l
00B26A: 13FC 0000 0010 0F11      move.b  #$0, $100f11.l
00B272: 41FA 000C                lea     ($b280,PC), A0
00B276: 4EB9 0000 91F8           jsr     $91f8.l
00B27C: 6000 0012                bra     $b290
00B280: 0218 0004                andi.b  #$4, (A0)+
00B284: 0030 0100 00D8           ori.b   #$0, (-$28,A0,D0.w)
00B28A: 0010 0F0C                ori.b   #$c, (A0)
00B28E: 0000 7200                ori.b   #$0, D0
00B292: 1239 0010 8FA9           move.b  $108fa9.l, D1
00B298: 13C1 0010 0F1D           move.b  D1, $100f1d.l
00B29E: 41FA 0008                lea     ($b2a8,PC), A0
00B2A2: 4EF9 0000 90CA           jmp     $90ca.l
00B2A8: 23C0 0010 0F0C           move.l  D0, $100f0c.l
00B2AE: 0C39 0030 0010 0F0C      cmpi.b  #$30, $100f0c.l
00B2B6: 662C                     bne     $b2e4
00B2B8: 13FC 0020 0010 0F0C      move.b  #$20, $100f0c.l
00B2C0: 0C39 0030 0010 0F0D      cmpi.b  #$30, $100f0d.l
00B2C8: 661A                     bne     $b2e4
00B2CA: 13FC 0020 0010 0F0D      move.b  #$20, $100f0d.l
00B2D2: 0C39 0030 0010 0F0E      cmpi.b  #$30, $100f0e.l
00B2DA: 6608                     bne     $b2e4
00B2DC: 13FC 0020 0010 0F0E      move.b  #$20, $100f0e.l
00B2E4: 13FC 0042 0010 0F10      move.b  #$42, $100f10.l
00B2EC: 13FC 0000 0010 0F11      move.b  #$0, $100f11.l
00B2F4: 41FA 000C                lea     ($b302,PC), A0
00B2F8: 4EB9 0000 91F8           jsr     $91f8.l
00B2FE: 6000 0012                bra     $b312
00B302: 021D 0004                andi.b  #$4, (A5)+
00B306: 0030 0100 00A8           ori.b   #$0, (-$58,A0,D0.w)
00B30C: 0010 0F0C                ori.b   #$c, (A0)
00B310: 0000 7200                ori.b   #$0, D0
00B314: 1239 0010 8FA5           move.b  $108fa5.l, D1
00B31A: 13C1 0010 0F1E           move.b  D1, $100f1e.l
00B320: 41FA 0008                lea     ($b32a,PC), A0
00B324: 4EF9 0000 90CA           jmp     $90ca.l
00B32A: 23C0 0010 0F0C           move.l  D0, $100f0c.l
00B330: 0C39 0030 0010 0F0C      cmpi.b  #$30, $100f0c.l
00B338: 662C                     bne     $b366
00B33A: 13FC 0020 0010 0F0C      move.b  #$20, $100f0c.l
00B342: 0C39 0030 0010 0F0D      cmpi.b  #$30, $100f0d.l
00B34A: 661A                     bne     $b366
00B34C: 13FC 0020 0010 0F0D      move.b  #$20, $100f0d.l
00B354: 0C39 0030 0010 0F0E      cmpi.b  #$30, $100f0e.l
00B35C: 6608                     bne     $b366
00B35E: 13FC 0020 0010 0F0E      move.b  #$20, $100f0e.l
00B366: 13FC 004A 0010 0F10      move.b  #$4a, $100f10.l
00B36E: 13FC 0000 0010 0F11      move.b  #$0, $100f11.l
00B376: 41FA 000C                lea     ($b384,PC), A0
00B37A: 4EB9 0000 91F8           jsr     $91f8.l
00B380: 6000 0012                bra     $b394
00B384: 0222 0004                andi.b  #$4, -(A2)
00B388: 0030 0100 0078           ori.b   #$0, ($78,A0,D0.w)
00B38E: 0010 0F0C                ori.b   #$c, (A0)
00B392: 0000 7200                ori.b   #$0, D0
00B396: 1239 0010 8FA6           move.b  $108fa6.l, D1
00B39C: 13C1 0010 0F1F           move.b  D1, $100f1f.l
00B3A2: 41FA 0008                lea     ($b3ac,PC), A0
00B3A6: 4EF9 0000 90CA           jmp     $90ca.l
00B3AC: 23C0 0010 0F0C           move.l  D0, $100f0c.l
00B3B2: 0C39 0030 0010 0F0C      cmpi.b  #$30, $100f0c.l
00B3BA: 662C                     bne     $b3e8
00B3BC: 13FC 0020 0010 0F0C      move.b  #$20, $100f0c.l
00B3C4: 0C39 0030 0010 0F0D      cmpi.b  #$30, $100f0d.l
00B3CC: 661A                     bne     $b3e8
00B3CE: 13FC 0020 0010 0F0D      move.b  #$20, $100f0d.l
00B3D6: 0C39 0030 0010 0F0E      cmpi.b  #$30, $100f0e.l
00B3DE: 6608                     bne     $b3e8
00B3E0: 13FC 0020 0010 0F0E      move.b  #$20, $100f0e.l
