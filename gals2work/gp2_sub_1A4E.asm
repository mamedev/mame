001A4E: 33FC 0000 0050 0C02 move.w  #$0, $500c02.l
001A56: 33FC 0000 0054 0C02 move.w  #$0, $540c02.l
001A5E: 33FC 0000 0058 0C02 move.w  #$0, $580c02.l
001A66: 33FC 0000 005C 0C02 move.w  #$0, $5c0c02.l
001A6E: 41F9 0010 0000      lea     $100000.l, A0
001A74: 7000                moveq   #$0, D0
001A76: 223C 0000 FFFF      move.l  #$ffff, D1
001A7C: 20C0                move.l  D0, (A0)+
001A7E: 51C9 FFFC           dbra    D1, $1a7c
001A82: 41F9 0040 0000      lea     $400000.l, A0
001A88: 303C FFFF           move.w  #$ffff, D0
001A8C: 20C1                move.l  D1, (A0)+
001A8E: 51C8 FFFC           dbra    D0, $1a8c
001A92: 41F9 0044 0000      lea     $440000.l, A0
001A98: 303C FFFF           move.w  #$ffff, D0
001A9C: 20C1                move.l  D1, (A0)+
001A9E: 51C8 FFFC           dbra    D0, $1a9c
001AA2: 41F9 0048 0000      lea     $480000.l, A0
001AA8: 303C FFFF           move.w  #$ffff, D0
001AAC: 20C1                move.l  D1, (A0)+
001AAE: 51C8 FFFC           dbra    D0, $1aac
001AB2: 41F9 004C 0000      lea     $4c0000.l, A0
001AB8: 303C FFFF           move.w  #$ffff, D0
001ABC: 20C1                move.l  D1, (A0)+
001ABE: 51C8 FFFC           dbra    D0, $1abc
001AC2: 33FC FFF5 0050 0C00 move.w  #$fff5, $500c00.l
001ACA: 33FC FFC0 0050 0400 move.w  #$ffc0, $500400.l
001AD2: 33FC FFF5 0054 0C00 move.w  #$fff5, $540c00.l
001ADA: 33FC FFC0 0054 0400 move.w  #$ffc0, $540400.l
001AE2: 33FC FFF5 0058 0C00 move.w  #$fff5, $580c00.l
001AEA: 33FC FFC0 0058 0400 move.w  #$ffc0, $580400.l
001AF2: 33FC FFF5 005C 0C00 move.w  #$fff5, $5c0c00.l
001AFA: 33FC FFC0 005C 0400 move.w  #$ffc0, $5c0400.l
001B02: 41F9 004C 0000      lea     $4c0000.l, A0
001B08: 43E8 00A0           lea     ($a0,A0), A1
001B0C: 45E9 00A0           lea     ($a0,A1), A2
001B10: 47EA 00A0           lea     ($a0,A2), A3
001B14: 7000                moveq   #$0, D0
001B16: 7200                moveq   #$0, D1
001B18: 7400                moveq   #$0, D2
001B1A: 7600                moveq   #$0, D3
001B1C: 3C3C 0020           move.w  #$20, D6
001B20: 3A3C 0007           move.w  #$7, D5
001B24: 383C 0028           move.w  #$28, D4
001B28: 2848                movea.l A0, A4
001B2A: 20C0                move.l  D0, (A0)+
001B2C: 22C1                move.l  D1, (A1)+
001B2E: 24C2                move.l  D2, (A2)+
001B30: 26C3                move.l  D3, (A3)+
001B32: 51CC FFF6           dbra    D4, $1b2a
001B36: 41EC 0400           lea     ($400,A4), A0
001B3A: 43E8 00A0           lea     ($a0,A0), A1
001B3E: 45E9 00A0           lea     ($a0,A1), A2
001B42: 47EA 00A0           lea     ($a0,A2), A3
001B46: 51CD FFDC           dbra    D5, $1b24
001B4A: 0680 0001 0001      addi.l  #$10001, D0
001B50: 0681 0020 0020      addi.l  #$200020, D1
001B56: 0682 0400 0400      addi.l  #$4000400, D2
001B5C: 0683 0421 0421      addi.l  #$4210421, D3
001B62: 51CE FFBC           dbra    D6, $1b20
001B66: 33FC 0001 005C 0C02 move.w  #$1, $5c0c02.l
001B6E: 4DFA 0152           lea     ($1cc2,PC), A6
001B72: 41FA 0006           lea     ($1b7a,PC), A0
001B76: 6000 0152           bra     $1cca
001B7A: 0010 0000           ori.b   #$0, (A0)
001B7E: 0000 FFBF           ori.b   #$bf, D0
001B82: 41FA 0006           lea     ($1b8a,PC), A0
001B86: 6000 0142           bra     $1cca
001B8A: 0011 0000           ori.b   #$0, (A1)
001B8E: 0000 FFFF           ori.b   #$ff, D0
001B92: 41FA 0006           lea     ($1b9a,PC), A0
001B96: 6000 0132           bra     $1cca
001B9A: 0012 0000           ori.b   #$0, (A2)
001B9E: 0000 FFFF           ori.b   #$ff, D0
001BA2: 41FA 0006           lea     ($1baa,PC), A0
001BA6: 6000 0122           bra     $1cca
001BAA: 0013 0000           ori.b   #$0, (A3)
001BAE: 0000 FFFF           ori.b   #$ff, D0
001BB2: 6000 0106           bra     $1cba
001BB6: 41FA 0006           lea     ($1bbe,PC), A0
001BBA: 6000 010E           bra     $1cca
001BBE: 004C                dc.w    $004c; ILLEGAL
001BC0: 0000 0000           ori.b   #$0, D0
001BC4: FFFF                dc.w    $ffff; opcode 1111
001BC6: 41FA 0006           lea     ($1bce,PC), A0
001BCA: 6000 00FE           bra     $1cca
001BCE: 004D                dc.w    $004d; ILLEGAL
001BD0: 0000 0000           ori.b   #$0, D0
001BD4: FFFF                dc.w    $ffff; opcode 1111
001BD6: 41FA 0006           lea     ($1bde,PC), A0
001BDA: 6000 00EE           bra     $1cca
001BDE: 004E                dc.w    $004e; ILLEGAL
001BE0: 0000 0000           ori.b   #$0, D0
001BE4: FFFF                dc.w    $ffff; opcode 1111
001BE6: 41FA 0006           lea     ($1bee,PC), A0
001BEA: 6000 00DE           bra     $1cca
001BEE: 004F                dc.w    $004f; ILLEGAL
001BF0: 0000 0000           ori.b   #$0, D0
001BF4: FFFF                dc.w    $ffff; opcode 1111
001BF6: 41FA 0006           lea     ($1bfe,PC), A0
001BFA: 6000 00CE           bra     $1cca
001BFE: 0048                dc.w    $0048; ILLEGAL
001C00: 0000 0000           ori.b   #$0, D0
001C04: FFFF                dc.w    $ffff; opcode 1111
001C06: 41FA 0006           lea     ($1c0e,PC), A0
001C0A: 6000 00BE           bra     $1cca
001C0E: 0049                dc.w    $0049; ILLEGAL
001C10: 0000 0000           ori.b   #$0, D0
001C14: FFFF                dc.w    $ffff; opcode 1111
001C16: 41FA 0006           lea     ($1c1e,PC), A0
001C1A: 6000 00AE           bra     $1cca
001C1E: 004A                dc.w    $004a; ILLEGAL
001C20: 0000 0000           ori.b   #$0, D0
001C24: FFFF                dc.w    $ffff; opcode 1111
001C26: 41FA 0006           lea     ($1c2e,PC), A0
001C2A: 6000 009E           bra     $1cca
001C2E: 004B                dc.w    $004b; ILLEGAL
001C30: 0000 0000           ori.b   #$0, D0
001C34: FFFF                dc.w    $ffff; opcode 1111
001C36: 6000 0082           bra     $1cba
001C3A: 41FA 0006           lea     ($1c42,PC), A0
001C3E: 6000 008A           bra     $1cca
001C42: 0044 0000           ori.w   #$0, D4
001C46: 0000 FFFF           ori.b   #$ff, D0
001C4A: 41FA 0006           lea     ($1c52,PC), A0
