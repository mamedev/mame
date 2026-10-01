001B60: 0421 51CE           subi.b  #-$32, -(A1)
001B64: FFBC                dc.w    $ffbc; opcode 1111
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
001C4E: 6000 007A           bra     $1cca
001C52: 0045 0000           ori.w   #$0, D5
001C56: 0000 FFFF           ori.b   #$ff, D0
001C5A: 41FA 0006           lea     ($1c62,PC), A0
001C5E: 6000 006A           bra     $1cca
001C62: 0046 0000           ori.w   #$0, D6
001C66: 0000 FFFF           ori.b   #$ff, D0
001C6A: 41FA 0006           lea     ($1c72,PC), A0
001C6E: 6000 005A           bra     $1cca
001C72: 0047 0000           ori.w   #$0, D7
001C76: 0000 FFFF           ori.b   #$ff, D0
001C7A: 41FA 0006           lea     ($1c82,PC), A0
001C7E: 6000 004A           bra     $1cca
001C82: 0040 0000           ori.w   #$0, D0
001C86: 0000 FFFF           ori.b   #$ff, D0
001C8A: 41FA 0006           lea     ($1c92,PC), A0
001C8E: 6000 003A           bra     $1cca
001C92: 0041 0000           ori.w   #$0, D1
001C96: 0000 FFFF           ori.b   #$ff, D0
001C9A: 41FA 0006           lea     ($1ca2,PC), A0
001C9E: 6000 002A           bra     $1cca
001CA2: 0042 0000           ori.w   #$0, D2
001CA6: 0000 FFFF           ori.b   #$ff, D0
001CAA: 41FA 0006           lea     ($1cb2,PC), A0
001CAE: 6000 001A           bra     $1cca
001CB2: 0043 0000           ori.w   #$0, D3
001CB6: 0000 FFFF           ori.b   #$ff, D0
001CBA: 5447                addq.w  #2, D7
001CBC: 4EF9 0000 10B4      jmp     $10b4.l
001CC2: 00FF                dc.w    $00ff; ILLEGAL
001CC4: 55AA DDBB           subq.l  #2, (-$2245,A2)
001CC8: 9900                subx.b  D0, D4
001CCA: 7C00                moveq   #$0, D6
001CCC: 2A50                movea.l (A0), A5
001CCE: 7200                moveq   #$0, D1
001CD0: 2A28 0004           move.l  ($4,A0), D5
001CD4: 3401                move.w  D1, D2
001CD6: 1BB6 2000 5800      move.b  (A6,D2.w), (A5,D5.l)
001CDC: 51CA 0004           dbra    D2, $1ce2
001CE0: 7406                moveq   #$6, D2
001CE2: 51CD FFF2           dbra    D5, $1cd6
001CE6: 4279 0070 0000      clr.w   $700000.l
001CEC: 0485 0001 0000      subi.l  #$10000, D5
001CF2: 64E2                bcc     $1cd6
001CF4: 7600                moveq   #$0, D3
001CF6: 2A28 0004           move.l  ($4,A0), D5
001CFA: 2401                move.l  D1, D2
001CFC: 4279 0070 0000      clr.w   $700000.l
001D02: 1835 5800           move.b  (A5,D5.l), D4
001D06: 1036 2000           move.b  (A6,D2.w), D0
001D0A: B800                cmp.b   D0, D4
001D0C: 6624                bne     $1d32
001D0E: 51CA 0004           dbra    D2, $1d14
001D12: 7406                moveq   #$6, D2
001D14: 1B83 5800           move.b  D3, (A5,D5.l)
001D18: 51CD FFE2           dbra    D5, $1cfc
001D1C: 0485 0001 0000      subi.l  #$10000, D5
001D22: 64D8                bcc     $1cfc
001D24: 51C9 FFAA           dbra    D1, $1cd0
001D28: 51CE FFA4           dbra    D6, $1cce
001D2C: 7000                moveq   #$0, D0
001D2E: 4EE8 0008           jmp     ($8,A0)
001D32: 4BF5 5800           lea     (A5,D5.l), A5
001D36: 23CD 0010 000C      move.l  A5, $10000c.l
001D3C: 33FC 0001 0050 0C02 move.w  #$1, $500c02.l
001D44: 33FC 0001 0054 0C02 move.w  #$1, $540c02.l
001D4C: 33FC 0001 0058 0C02 move.w  #$1, $580c02.l
001D54: 33FC 0001 005C 0C02 move.w  #$1, $5c0c02.l
001D5C: 7000                moveq   #$0, D0
001D5E: 7200                moveq   #$0, D1
001D60: 7400                moveq   #$0, D2
001D62: 7600                moveq   #$0, D3
001D64: 49F9 0040 0000      lea     $400000.l, A4
001D6A: 43F9 0044 0000      lea     $440000.l, A1
001D70: 45F9 0048 0000      lea     $480000.l, A2
001D76: 47F9 004C 0000      lea     $4c0000.l, A3
001D7C: 323C FFFF           move.w  #$ffff, D1
001D80: 28C0                move.l  D0, (A4)+
001D82: 22C1                move.l  D1, (A1)+
001D84: 24C2                move.l  D2, (A2)+
001D86: 26C3                move.l  D3, (A3)+
001D88: 51C9 FFF6           dbra    D1, $1d80
001D8C: 0C80 7FFF 7FFF      cmpi.l  #$7fff7fff, D0
001D92: 671A                beq     $1dae
001D94: 0680 0421 0421      addi.l  #$4210421, D0
001D9A: 0681 0001 0001      addi.l  #$10001, D1
001DA0: 0682 0020 0020      addi.l  #$200020, D2
001DA6: 0683 0400 0400      addi.l  #$4000400, D3
001DAC: 60B6                bra     $1d64
001DAE: 60FE                bra     $1dae
001DB0: 4EF9 0000 2990      jmp     $2990.l
001DB6: 41F8 0000           lea     $0.w, A0
001DBA: 303C FFFF           move.w  #$ffff, D0
001DBE: 7200                moveq   #$0, D1
001DC0: D258                add.w   (A0)+, D1
001DC2: D258                add.w   (A0)+, D1
001DC4: 51C8 FFFA           dbra    D0, $1dc0
001DC8: 6000 0292           bra     $205c
001DCC: 48E7 8000           movem.l D0, -(A7)
001DD0: 3039 0010 94B8      move.w  $1094b8.l, D0
001DD6: 33C0 007C 0000      move.w  D0, $7c0000.l
001DDC: 4CDF 0001           movem.l (A7)+, D0
001DE0: 41F9 0080 0000      lea     $800000.l, A0
001DE6: 303C FFFF           move.w  #$ffff, D0
001DEA: 7400                moveq   #$0, D2
001DEC: D458                add.w   (A0)+, D2
001DEE: D458                add.w   (A0)+, D2
001DF0: D458                add.w   (A0)+, D2
001DF2: D458                add.w   (A0)+, D2
001DF4: D458                add.w   (A0)+, D2
001DF6: D458                add.w   (A0)+, D2
001DF8: D458                add.w   (A0)+, D2
001DFA: D458                add.w   (A0)+, D2
001DFC: D458                add.w   (A0)+, D2
001DFE: D458                add.w   (A0)+, D2
001E00: D458                add.w   (A0)+, D2
001E02: D458                add.w   (A0)+, D2
001E04: D458                add.w   (A0)+, D2
001E06: D458                add.w   (A0)+, D2
001E08: D458                add.w   (A0)+, D2
001E0A: D458                add.w   (A0)+, D2
001E0C: D458                add.w   (A0)+, D2
001E0E: D458                add.w   (A0)+, D2
001E10: D458                add.w   (A0)+, D2
001E12: D458                add.w   (A0)+, D2
001E14: D458                add.w   (A0)+, D2
001E16: D458                add.w   (A0)+, D2
001E18: D458                add.w   (A0)+, D2
001E1A: D458                add.w   (A0)+, D2
001E1C: D458                add.w   (A0)+, D2
001E1E: D458                add.w   (A0)+, D2
001E20: D458                add.w   (A0)+, D2
001E22: D458                add.w   (A0)+, D2
001E24: D458                add.w   (A0)+, D2
001E26: D458                add.w   (A0)+, D2
001E28: D458                add.w   (A0)+, D2
001E2A: D458                add.w   (A0)+, D2
001E2C: D458                add.w   (A0)+, D2
001E2E: D458                add.w   (A0)+, D2
001E30: D458                add.w   (A0)+, D2
001E32: D458                add.w   (A0)+, D2
001E34: D458                add.w   (A0)+, D2
001E36: D458                add.w   (A0)+, D2
001E38: D458                add.w   (A0)+, D2
001E3A: D458                add.w   (A0)+, D2
001E3C: D458                add.w   (A0)+, D2
001E3E: D458                add.w   (A0)+, D2
001E40: D458                add.w   (A0)+, D2
001E42: D458                add.w   (A0)+, D2
001E44: D458                add.w   (A0)+, D2
001E46: D458                add.w   (A0)+, D2
001E48: D458                add.w   (A0)+, D2
001E4A: D458                add.w   (A0)+, D2
001E4C: D458                add.w   (A0)+, D2
001E4E: D458                add.w   (A0)+, D2
001E50: D458                add.w   (A0)+, D2
001E52: D458                add.w   (A0)+, D2
001E54: D458                add.w   (A0)+, D2
001E56: D458                add.w   (A0)+, D2
001E58: D458                add.w   (A0)+, D2
001E5A: D458                add.w   (A0)+, D2
001E5C: D458                add.w   (A0)+, D2
001E5E: D458                add.w   (A0)+, D2
001E60: D458                add.w   (A0)+, D2
001E62: D458                add.w   (A0)+, D2
001E64: D458                add.w   (A0)+, D2
001E66: D458                add.w   (A0)+, D2
001E68: D458                add.w   (A0)+, D2
001E6A: D458                add.w   (A0)+, D2
001E6C: 51C8 FF7E           dbra    D0, $1dec
001E70: 48E7 8000           movem.l D0, -(A7)
001E74: 3039 0010 94B8      move.w  $1094b8.l, D0
001E7A: 33C0 007C 0000      move.w  D0, $7c0000.l
001E80: 4CDF 0001           movem.l (A7)+, D0
001E84: 41F9 0080 0000      lea     $800000.l, A0
001E8A: 303C FFFF           move.w  #$ffff, D0
001E8E: 7600                moveq   #$0, D3
001E90: D658                add.w   (A0)+, D3
001E92: D658                add.w   (A0)+, D3
001E94: D658                add.w   (A0)+, D3
001E96: D658                add.w   (A0)+, D3
001E98: D658                add.w   (A0)+, D3
001E9A: D658                add.w   (A0)+, D3
001E9C: D658                add.w   (A0)+, D3
001E9E: D658                add.w   (A0)+, D3
001EA0: D658                add.w   (A0)+, D3
001EA2: D658                add.w   (A0)+, D3
001EA4: D658                add.w   (A0)+, D3
001EA6: D658                add.w   (A0)+, D3
001EA8: D658                add.w   (A0)+, D3
001EAA: D658                add.w   (A0)+, D3
001EAC: D658                add.w   (A0)+, D3
001EAE: D658                add.w   (A0)+, D3
001EB0: D658                add.w   (A0)+, D3
001EB2: D658                add.w   (A0)+, D3
001EB4: D658                add.w   (A0)+, D3
001EB6: D658                add.w   (A0)+, D3
001EB8: D658                add.w   (A0)+, D3
001EBA: D658                add.w   (A0)+, D3
001EBC: D658                add.w   (A0)+, D3
001EBE: D658                add.w   (A0)+, D3
001EC0: D658                add.w   (A0)+, D3
001EC2: D658                add.w   (A0)+, D3
001EC4: D658                add.w   (A0)+, D3
001EC6: D658                add.w   (A0)+, D3
001EC8: D658                add.w   (A0)+, D3
001ECA: D658                add.w   (A0)+, D3
001ECC: D658                add.w   (A0)+, D3
001ECE: D658                add.w   (A0)+, D3
001ED0: D658                add.w   (A0)+, D3
001ED2: D658                add.w   (A0)+, D3
001ED4: D658                add.w   (A0)+, D3
001ED6: D658                add.w   (A0)+, D3
001ED8: D658                add.w   (A0)+, D3
001EDA: D658                add.w   (A0)+, D3
001EDC: D658                add.w   (A0)+, D3
001EDE: D658                add.w   (A0)+, D3
001EE0: D658                add.w   (A0)+, D3
001EE2: D658                add.w   (A0)+, D3
001EE4: D658                add.w   (A0)+, D3
001EE6: D658                add.w   (A0)+, D3
001EE8: D658                add.w   (A0)+, D3
001EEA: D658                add.w   (A0)+, D3
001EEC: D658                add.w   (A0)+, D3
001EEE: D658                add.w   (A0)+, D3
001EF0: D658                add.w   (A0)+, D3
001EF2: D658                add.w   (A0)+, D3
001EF4: D658                add.w   (A0)+, D3
001EF6: D658                add.w   (A0)+, D3
001EF8: D658                add.w   (A0)+, D3
001EFA: D658                add.w   (A0)+, D3
001EFC: D658                add.w   (A0)+, D3
001EFE: D658                add.w   (A0)+, D3
001F00: D658                add.w   (A0)+, D3
001F02: D658                add.w   (A0)+, D3
001F04: D658                add.w   (A0)+, D3
001F06: D658                add.w   (A0)+, D3
001F08: D658                add.w   (A0)+, D3
001F0A: D658                add.w   (A0)+, D3
001F0C: D658                add.w   (A0)+, D3
001F0E: D658                add.w   (A0)+, D3
001F10: 51C8 FF7E           dbra    D0, $1e90
001F14: 48E7 8000           movem.l D0, -(A7)
001F18: 3039 0010 94B8      move.w  $1094b8.l, D0
001F1E: 33C0 007C 0000      move.w  D0, $7c0000.l
001F24: 4CDF 0001           movem.l (A7)+, D0
001F28: 41F9 0080 0000      lea     $800000.l, A0
001F2E: 303C FFFF           move.w  #$ffff, D0
001F32: 7800                moveq   #$0, D4
001F34: D858                add.w   (A0)+, D4
001F36: D858                add.w   (A0)+, D4
001F38: D858                add.w   (A0)+, D4
001F3A: D858                add.w   (A0)+, D4
001F3C: D858                add.w   (A0)+, D4
001F3E: D858                add.w   (A0)+, D4
001F40: D858                add.w   (A0)+, D4
001F42: D858                add.w   (A0)+, D4
001F44: D858                add.w   (A0)+, D4
001F46: D858                add.w   (A0)+, D4
001F48: D858                add.w   (A0)+, D4
001F4A: D858                add.w   (A0)+, D4
001F4C: D858                add.w   (A0)+, D4
001F4E: D858                add.w   (A0)+, D4
001F50: D858                add.w   (A0)+, D4
001F52: D858                add.w   (A0)+, D4
001F54: D858                add.w   (A0)+, D4
001F56: D858                add.w   (A0)+, D4
001F58: D858                add.w   (A0)+, D4
001F5A: D858                add.w   (A0)+, D4
001F5C: D858                add.w   (A0)+, D4
001F5E: D858                add.w   (A0)+, D4
