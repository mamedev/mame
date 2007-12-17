
#undef OP
#define OP(nn) INLINE void minx_##nn(void)

OP(00) { regs.BA = ( regs.BA & 0xFF00 ) | ADD8( ( regs.BA & 0x00FF ), ( regs.BA & 0xFF ) ); }
OP(01) { regs.BA = ( regs.BA & 0xFF00 ) | ADD8( ( regs.BA & 0x00FF ), ( regs.BA >> 8 ) ); }
OP(02) { regs.BA = ( regs.BA & 0xFF00 ) | ADD8( ( regs.BA & 0x00FF ), rdop() ); }
OP(03) { AD2_IHL; regs.BA = ( regs.BA & 0xFF00 ) | ADD8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(04) { AD2_IN8; regs.BA = ( regs.BA & 0xFF00 ) | ADD8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(05) { AD2_I16; regs.BA = ( regs.BA & 0xFF00 ) | ADD8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(06) { AD2_XIX; regs.BA = ( regs.BA & 0xFF00 ) | ADD8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(07) { AD2_YIY; regs.BA = ( regs.BA & 0xFF00 ) | ADD8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(08) { regs.BA = ( regs.BA & 0xFF00 ) | ADDC8( ( regs.BA & 0x00FF ), ( regs.BA & 0xFF ) ); }
OP(09) { regs.BA = ( regs.BA & 0xFF00 ) | ADDC8( ( regs.BA & 0x00FF ), ( regs.BA >> 8 ) ); }
OP(0A) { regs.BA = ( regs.BA & 0xFF00 ) | ADDC8( ( regs.BA & 0x00FF ), rdop() ); }
OP(0B) { AD2_IHL; regs.BA = ( regs.BA & 0xFF00 ) | ADDC8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(0C) { AD2_IN8; regs.BA = ( regs.BA & 0xFF00 ) | ADDC8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(0D) { AD2_I16; regs.BA = ( regs.BA & 0xFF00 ) | ADDC8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(0E) { AD2_XIX; regs.BA = ( regs.BA & 0xFF00 ) | ADDC8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(0F) { AD2_YIY; regs.BA = ( regs.BA & 0xFF00 ) | ADDC8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }

OP(10) { regs.BA = ( regs.BA & 0xFF00 ) | SUB8( ( regs.BA & 0x00FF ), ( regs.BA & 0xFF ) ); }
OP(11) { regs.BA = ( regs.BA & 0xFF00 ) | SUB8( ( regs.BA & 0x00FF ), ( regs.BA >> 8 ) ); }
OP(12) { regs.BA = ( regs.BA & 0xFF00 ) | SUB8( ( regs.BA & 0x00FF ), rdop() ); }
OP(13) { AD2_IHL; regs.BA = ( regs.BA & 0xFF00 ) | SUB8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(14) { AD2_IN8; regs.BA = ( regs.BA & 0xFF00 ) | SUB8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(15) { AD2_I16; regs.BA = ( regs.BA & 0xFF00 ) | SUB8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(16) { AD2_XIX; regs.BA = ( regs.BA & 0xFF00 ) | SUB8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(17) { AD2_YIY; regs.BA = ( regs.BA & 0xFF00 ) | SUB8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(18) { regs.BA = ( regs.BA & 0xFF00 ) | SUBC8( ( regs.BA & 0x00FF ), ( regs.BA & 0xFF ) ); }
OP(19) { regs.BA = ( regs.BA & 0xFF00 ) | SUBC8( ( regs.BA & 0x00FF ), ( regs.BA >> 8 ) ); }
OP(1A) { regs.BA = ( regs.BA & 0xFF00 ) | SUBC8( ( regs.BA & 0x00FF ), rdop() ); }
OP(1B) { AD2_IHL; regs.BA = ( regs.BA & 0xFF00 ) | SUBC8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(1C) { AD2_IN8; regs.BA = ( regs.BA & 0xFF00 ) | SUBC8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(1D) { AD2_I16; regs.BA = ( regs.BA & 0xFF00 ) | SUBC8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(1E) { AD2_XIX; regs.BA = ( regs.BA & 0xFF00 ) | SUBC8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(1F) { AD2_YIY; regs.BA = ( regs.BA & 0xFF00 ) | SUBC8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }

OP(20) { regs.BA = ( regs.BA & 0xFF00 ) | AND8( ( regs.BA & 0x00FF ), ( regs.BA & 0xFF ) ); }
OP(21) { regs.BA = ( regs.BA & 0xFF00 ) | AND8( ( regs.BA & 0x00FF ), ( regs.BA >> 8 ) ); }
OP(22) { regs.BA = ( regs.BA & 0xFF00 ) | AND8( ( regs.BA & 0x00FF ), rdop() ); }
OP(23) { AD2_IHL; regs.BA = ( regs.BA & 0xFF00 ) | AND8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(24) { AD2_IN8; regs.BA = ( regs.BA & 0xFF00 ) | AND8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(25) { AD2_I16; regs.BA = ( regs.BA & 0xFF00 ) | AND8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(26) { AD2_XIX; regs.BA = ( regs.BA & 0xFF00 ) | AND8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(27) { AD2_YIY; regs.BA = ( regs.BA & 0xFF00 ) | AND8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(28) { regs.BA = ( regs.BA & 0xFF00 ) | OR8( ( regs.BA & 0x00FF ), ( regs.BA & 0xFF ) ); }
OP(29) { regs.BA = ( regs.BA & 0xFF00 ) | OR8( ( regs.BA & 0x00FF ), ( regs.BA >> 8 ) ); }
OP(2A) { regs.BA = ( regs.BA & 0xFF00 ) | OR8( ( regs.BA & 0x00FF ), rdop() ); }
OP(2B) { AD2_IHL; regs.BA = ( regs.BA & 0xFF00 ) | OR8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(2C) { AD2_IN8; regs.BA = ( regs.BA & 0xFF00 ) | OR8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(2D) { AD2_I16; regs.BA = ( regs.BA & 0xFF00 ) | OR8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(2E) { AD2_XIX; regs.BA = ( regs.BA & 0xFF00 ) | OR8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(2F) { AD2_YIY; regs.BA = ( regs.BA & 0xFF00 ) | OR8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }

OP(30) { SUB8( ( regs.BA & 0x00FF ), ( regs.BA & 0xFF ) ); }
OP(31) { SUB8( ( regs.BA & 0x00FF ), ( regs.BA >> 8 ) ); }
OP(32) { SUB8( ( regs.BA & 0x00FF ), rdop() ); }
OP(33) { AD2_IHL; SUB8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(34) { AD2_IN8; SUB8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(35) { AD2_I16; SUB8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(36) { AD2_XIX; SUB8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(37) { AD2_YIY; SUB8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(38) { regs.BA = ( regs.BA & 0xFF00 ) | XOR8( ( regs.BA & 0x00FF ), ( regs.BA & 0xFF ) ); }
OP(39) { regs.BA = ( regs.BA & 0xFF00 ) | XOR8( ( regs.BA & 0x00FF ), ( regs.BA >> 8 ) ); }
OP(3A) { regs.BA = ( regs.BA & 0xFF00 ) | XOR8( ( regs.BA & 0x00FF ), rdop() ); }
OP(3B) { AD2_IHL; regs.BA = ( regs.BA & 0xFF00 ) | XOR8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(3C) { AD2_IN8; regs.BA = ( regs.BA & 0xFF00 ) | XOR8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(3D) { AD2_I16; regs.BA = ( regs.BA & 0xFF00 ) | XOR8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(3E) { AD2_XIX; regs.BA = ( regs.BA & 0xFF00 ) | XOR8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }
OP(3F) { AD2_YIY; regs.BA = ( regs.BA & 0xFF00 ) | XOR8( ( regs.BA & 0x00FF ), rd( addr2 ) ); }

OP(40) { regs.BA = ( regs.BA & 0xFF00 ) | ( regs.BA & 0x00FF); }
OP(41) { regs.BA = ( regs.BA & 0xFF00 ) | ( regs.BA >> 8 ); }
OP(42) { regs.BA = ( regs.BA & 0xFF00 ) | ( regs.HL & 0x00FF); }
OP(43) { regs.BA = ( regs.BA & 0xFF00 ) | ( regs.HL >> 8 ); }
OP(44) { AD2_IN8; regs.BA = ( regs.BA & 0xFF00 ) | rd( addr2 ); }
OP(45) { AD2_I16; regs.BA = ( regs.BA & 0xFF00 ) | rd( addr2 ); }
OP(46) { AD2_XIX; regs.BA = ( regs.BA & 0xFF00 ) | rd( addr2 ); }
OP(47) { AD2_YIY; regs.BA = ( regs.BA & 0xFF00 ) | rd( addr2 ); }
OP(48) { regs.BA = ( regs.BA & 0x00FF ) | ( ( regs.BA & 0x00FF) << 8 ); }
OP(49) { regs.BA = ( regs.BA & 0x00FF ) | ( ( regs.BA >> 8 ) << 8 ); }
OP(4A) { regs.BA = ( regs.BA & 0x00FF ) | ( ( regs.HL & 0x00FF) << 8 ); }
OP(4B) { regs.BA = ( regs.BA & 0x00FF ) | ( ( regs.HL >> 8 ) << 8 ); }
OP(4C) { AD2_IN8; regs.BA = ( regs.BA & 0x00FF ) | ( rd( addr2 ) << 8 ); }
OP(4D) { AD2_I16; regs.BA = ( regs.BA & 0x00FF ) | ( rd( addr2 ) << 8 ); }
OP(4E) { AD2_XIX; regs.BA = ( regs.BA & 0x00FF ) | ( rd( addr2 ) << 8 ); }
OP(4F) { AD2_YIY; regs.BA = ( regs.BA & 0x00FF ) | ( rd( addr2 ) << 8 ); }

OP(50) { regs.HL = ( regs.HL & 0xFF00 ) | ( regs.BA & 0x00FF); }
OP(51) { regs.HL = ( regs.HL & 0xFF00 ) | ( regs.BA >> 8 ); }
OP(52) { regs.HL = ( regs.HL & 0xFF00 ) | ( regs.HL & 0x00FF); }
OP(53) { regs.HL = ( regs.HL & 0xFF00 ) | ( regs.HL >> 8 ); }
OP(54) { AD2_IN8; regs.HL = ( regs.HL & 0xFF00 ) | rd( addr2 ); }
OP(55) { AD2_I16; regs.HL = ( regs.HL & 0xFF00 ) | rd( addr2 ); }
OP(56) { AD2_XIX; regs.HL = ( regs.HL & 0xFF00 ) | rd( addr2 ); }
OP(57) { AD2_YIY; regs.HL = ( regs.HL & 0xFF00 ) | rd( addr2 ); }
OP(58) { regs.HL = ( regs.HL & 0x00FF ) | ( ( regs.BA & 0x00FF) << 8 ); }
OP(59) { regs.HL = ( regs.HL & 0x00FF ) | ( ( regs.BA >> 8 ) << 8 ); }
OP(5A) { regs.HL = ( regs.HL & 0x00FF ) | ( ( regs.HL & 0x00FF) << 8 ); }
OP(5B) { regs.HL = ( regs.HL & 0x00FF ) | ( ( regs.HL >> 8 ) << 8 ); }
OP(5C) { AD2_IN8; regs.HL = ( regs.HL & 0x00FF ) | ( rd( addr2 ) << 8 ); }
OP(5D) { AD2_I16; regs.HL = ( regs.HL & 0x00FF ) | ( rd( addr2 ) << 8 ); }
OP(5E) { AD2_XIX; regs.HL = ( regs.HL & 0x00FF ) | ( rd( addr2 ) << 8 ); }
OP(5F) { AD2_YIY; regs.HL = ( regs.HL & 0x00FF ) | ( rd( addr2 ) << 8 ); }

OP(60) { AD1_XIX; wr( addr1, ( regs.BA & 0x00FF ) ); }
OP(61) { AD1_XIX; wr( addr1, ( regs.BA >> 8 ) ); }
OP(62) { AD1_XIX; wr( addr1, ( regs.HL & 0x00FF ) ); }
OP(63) { AD1_XIX; wr( addr1, ( regs.HL >> 8 ) ); }
OP(64) { AD1_XIX; AD2_IN8; wr( addr1, rd( addr2 ) ); }
OP(65) { AD1_XIX; AD2_IHL; wr( addr1, rd( addr2 ) ); }
OP(66) { AD1_XIX; AD2_XIX; wr( addr1, rd( addr2 ) ); }
OP(67) { AD1_XIX; AD2_YIY; wr( addr1, rd( addr2 ) ); }
OP(68) { AD1_IHL; wr( addr1, ( regs.BA & 0x00FF ) ); }
OP(69) { AD1_IHL; wr( addr1, ( regs.BA >> 8 ) ); }
OP(6A) { AD1_IHL; wr( addr1, ( regs.HL & 0x00FF ) ); }
OP(6B) { AD1_IHL; wr( addr1, ( regs.HL >> 8 ) ); }
OP(6C) { AD1_IHL; AD2_IN8; wr( addr1, rd( addr2 ) ); }
OP(6D) { AD1_IHL; AD2_IHL; wr( addr1, rd( addr2 ) ); }
OP(6E) { AD1_IHL; AD2_XIX; wr( addr1, rd( addr2 ) ); }
OP(6F) { AD1_IHL; AD2_YIY; wr( addr1, rd( addr2 ) ); }

OP(70) { AD1_YIY; wr( addr1, ( regs.BA & 0x00FF ) ); }
OP(71) { AD1_YIY; wr( addr1, ( regs.BA >> 8 ) ); }
OP(72) { AD1_YIY; wr( addr1, ( regs.HL & 0x00FF ) ); }
OP(73) { AD1_YIY; wr( addr1, ( regs.HL >> 8 ) ); }
OP(74) { AD1_YIY; AD2_IN8; wr( addr1, rd( addr2 ) ); }
OP(75) { AD1_YIY; AD2_IHL; wr( addr1, rd( addr2 ) ); }
OP(76) { AD1_YIY; AD2_XIX; wr( addr1, rd( addr2 ) ); }
OP(77) { AD1_YIY; AD2_YIY; wr( addr1, rd( addr2 ) ); }
OP(78) { AD1_IN8; wr( addr1, ( regs.BA & 0x00FF ) ); }
OP(79) { AD1_IN8; wr( addr1, ( regs.BA >> 8 ) ); }
OP(7A) { AD1_IN8; wr( addr1, ( regs.HL & 0x00FF ) ); }
OP(7B) { AD1_IN8; wr( addr1, ( regs.HL >> 8 ) ); }
OP(7C) { /* illegal operation? */ }
OP(7D) { AD1_IN8; AD2_IHL; wr( addr1, rd( addr2 ) ); }
OP(7E) { AD1_IN8; AD2_XIX; wr( addr1, rd( addr2 ) ); }
OP(7F) { AD1_IN8; AD2_YIY; wr( addr1, rd( addr2 ) ); }

OP(80) { regs.BA = ( regs.BA & 0xFF00 ) | INC8( regs.BA & 0x00FF ); }
OP(81) { regs.BA = ( regs.BA & 0x00FF ) | ( INC8( regs.BA >> 8 ) << 8 ); }
OP(82) { regs.HL = ( regs.HL & 0xFF00 ) | INC8( regs.HL & 0x00FF ); }
OP(83) { regs.HL = ( regs.HL & 0x00FF ) | ( INC8( regs.HL >> 8 ) << 8 ); }
OP(84) { regs.N = INC8( regs.N ); }
OP(85) { AD1_IN8; wr( addr1, INC8( rd( addr1 ) ) ); }
OP(86) { AD1_IHL; wr( addr1, INC8( rd( addr1 ) ) ); }
OP(87) { regs.SP = INC16( regs.SP ); }
OP(88) { regs.BA = ( regs.BA & 0xFF00 ) | DEC8( regs.BA & 0x00FF ); }
OP(89) { regs.BA = ( regs.BA & 0x00FF ) | ( DEC8( regs.BA >> 8 ) << 8 ); }
OP(8A) { regs.HL = ( regs.HL & 0xFF00 ) | DEC8( regs.HL & 0x00FF ); }
OP(8B) { regs.HL = ( regs.HL & 0x00FF ) | ( DEC8( regs.HL >> 8 ) << 8 ); }
OP(8C) { regs.N = DEC8( regs.N ); }
OP(8D) { AD1_IN8; wr( addr1, DEC8( rd( addr1 ) ) ); }
OP(8E) { AD1_IHL; wr( addr1, DEC8( rd( addr1 ) ) ); }
OP(8F) { regs.SP = DEC8( regs.SP ); }

OP(90) { regs.BA = INC16( regs.BA ); }
OP(91) { regs.HL = INC16( regs.HL ); }
OP(92) { regs.X = INC16( regs.X ); }
OP(93) { regs.Y = INC16( regs.Y ); }
OP(94) { regs.F = ( AND8( ( regs.BA & 0x00FF ), ( regs.BA >> 8 ) ) ) ? regs.F & ~FLAG_Z : regs.F | FLAG_Z;}
OP(95) { AD1_IHL; regs.F = ( AND8( rd( addr1 ), rdop() ) ) ? regs.F & ~FLAG_Z : regs.F | FLAG_Z; }
OP(96) { regs.F = ( AND8( ( regs.BA & 0x00FF ), rdop() ) ) ? regs.F & ~FLAG_Z : regs.F | FLAG_Z; }
OP(97) { regs.F = ( AND8( ( regs.BA >> 8 ), rdop() ) ) ? regs.F & ~FLAG_Z : regs.F | FLAG_Z; }
OP(98) { regs.BA = DEC16( regs.BA ); }
OP(99) { regs.HL = DEC16( regs.HL ); }
OP(9A) { regs.X = DEC16( regs.X ); }
OP(9B) { regs.Y = DEC16( regs.Y ); }
OP(9C) { regs.F = regs.F & rdop(); }
OP(9D) { regs.F = regs.F | rdop(); }
OP(9E) { regs.F = regs.F ^ rdop(); }
OP(9F) { regs.F = rdop(); }

OP(A0) { PUSH16( regs.BA ); }
OP(A1) { PUSH16( regs.HL ); }
OP(A2) { PUSH16( regs.X ); }
OP(A3) { PUSH16( regs.Y ); }
OP(A4) { PUSH8( regs.N ); }
OP(A5) { PUSH8( regs.I ); }
OP(A6) { PUSH8( regs.XI ); PUSH8( regs.YI ); }
OP(A7) { PUSH8( regs.F ); }
OP(A8) { regs.BA = POP16(); }
OP(A9) { regs.HL = POP16();}
OP(AA) { regs.X = POP16(); }
OP(AB) { regs.Y = POP16(); }
OP(AC) { regs.N = POP8(); }
OP(AD) { regs.I = POP8(); }
OP(AE) { regs.YI = POP8(); regs.XI = POP8(); }
OP(AF) { regs.F = POP8(); }

OP(B0) { UINT8 op = rdop(); regs.BA = ( regs.BA & 0xFF00 ) | op; }
OP(B1) { UINT8 op = rdop(); regs.BA = ( regs.BA & 0x00FF ) | ( op << 8 ); }
OP(B2) { UINT8 op = rdop(); regs.HL = ( regs.HL & 0xFF00 ) | op; }
OP(B3) { UINT8 op = rdop(); regs.HL = ( regs.HL & 0x00FF ) | ( op << 8 ); }
OP(B4) { UINT8 op = rdop(); regs.N = op; }
OP(B5) { UINT8 op = rdop(); wr( ( regs.I << 16 ) | regs.HL, op); }
OP(B6) { UINT8 op = rdop(); wr( ( regs.X << 16 ), op ); }
OP(B7) { UINT8 op = rdop(); wr( ( regs.Y << 16 ), op ); }
OP(B8) { AD2_I16; regs.BA = rd16( addr2 ); }
OP(B9) { AD2_I16; regs.HL = rd16( addr2 ); }
OP(BA) { AD2_I16; regs.X = rd16( addr2 ); }
OP(BB) { AD2_I16; regs.Y = rd16( addr2 ); }
OP(BC) { AD1_I16; wr( addr1, regs.BA ); }
OP(BD) { AD1_I16; wr( addr1, regs.HL ); }
OP(BE) { AD1_I16; wr16( addr1, regs.X ); }
OP(BF) { AD1_I16; wr16( addr1, regs.Y ); }

OP(C0) { regs.BA = ADD16( regs.BA, rdop16() ); }
OP(C1) { regs.HL = ADD16( regs.HL, rdop16() ); }
OP(C2) { regs.X = ADD16( regs.X, rdop16() ); }
OP(C3) { regs.Y = ADD16( regs.Y, rdop16() ); }
OP(C4) { regs.BA = rdop16(); }
OP(C5) { regs.HL = rdop16(); }
OP(C6) { regs.X = rdop16(); }
OP(C7) { regs.Y = rdop16(); }
OP(C8) { UINT16 t = regs.BA; regs.BA = regs.HL; regs.HL = t; }
OP(C9) { UINT16 t = regs.BA; regs.BA = regs.X; regs.X = t; }
OP(CA) { UINT16 t = regs.BA; regs.BA = regs.Y; regs.Y = t; }
OP(CB) { UINT16 t = regs.BA; regs.BA = regs.SP; regs.SP = t; }
OP(CC) { regs.BA = ( regs.BA >> 8 ) | ( ( regs.BA & 0x00FF ) << 8 ); }
OP(CD) { UINT8 t; AD2_IHL; t = rd( addr2 ); wr( addr2, ( regs.BA & 0x00FF ) ); regs.BA = ( regs.BA & 0xFF00 ) | t; }
OP(CE) { UINT8 op = rdop(); insnminx_CE[op](); minx_icount -= insnminx_cycles_CE[op]; }
OP(CF) { UINT8 op = rdop(); insnminx_CF[op](); minx_icount -= insnminx_cycles_CF[op]; }

OP(D0) { regs.BA = SUB16( regs.BA, rdop16() ); }
OP(D1) { regs.HL = SUB16( regs.HL, rdop16() ); }
OP(D2) { regs.X = SUB16( regs.X, rdop16() ); }
OP(D3) { regs.Y = SUB16( regs.Y, rdop16() ); }
OP(D4) { SUB16( regs.BA, rdop16() ); }
OP(D5) { SUB16( regs.HL, rdop16() ); }
OP(D6) { SUB16( regs.X, rdop16() ); }
OP(D7) { SUB16( regs.Y, rdop16() ); }
OP(D8) { AD1_IN8; wr( addr1, AND8( rd( addr1 ), rdop() ) ); }
OP(D9) { AD1_IN8; wr( addr1, OR8( rd( addr1 ), rdop() ) ); }
OP(DA) { AD1_IN8; wr( addr1, XOR8( rd( addr1 ), rdop() ) ); }
OP(DB) { AD1_IN8; SUB8( rd( addr1 ), rdop() ); }
OP(DC) { AD1_IN8; regs.F = ( AND8( rd( addr1 ), rdop() ) ) ? regs.F & ~FLAG_Z : regs.F | FLAG_Z; }
OP(DD) { AD1_IN8; wr( addr1, rdop() ); }
OP(DE) { regs.BA = ( regs.BA & 0xFF00 ) | ( ( regs.BA & 0x000F ) | ( ( regs.BA & 0x0F00 ) >> 4 ) ); }
OP(DF) { regs.BA = ( ( regs.BA & 0x0080 ) ? 0xFF00 : 0x0000 ) | ( regs.BA & 0x000F ); }

OP(E0) { INT8 d8 = rdop(); if ( regs.F & FLAG_C ) { CALL( regs.PC + d8 - 1 ); } }
OP(E1) { INT8 d8 = rdop(); if ( ! ( regs.F & FLAG_C ) ) { CALL( regs.PC + d8- 1  ); } }
OP(E2) { INT8 d8 = rdop(); if ( regs.F & FLAG_Z ) { CALL( regs.PC + d8 - 1 ); } }
OP(E3) { INT8 d8 = rdop(); if ( ! ( regs.F & FLAG_Z ) ) { CALL( regs.PC + d8 - 1 ); } }
OP(E4) { INT8 d8 = rdop(); if ( regs.F & FLAG_C ) { JMP( regs.PC + d8 - 1 ); } }
OP(E5) { INT8 d8 = rdop(); if ( ! ( regs.F & FLAG_C ) ) { JMP( regs.PC + d8 - 1 ); } }
OP(E6) { INT8 d8 = rdop(); if ( regs.F & FLAG_Z ) { JMP( regs.PC + d8 - 1 ); } }
OP(E7) { INT8 d8 = rdop(); if ( ! ( regs.F & FLAG_Z ) ) { JMP( regs.PC + d8 - 1 ); } }
OP(E8) { UINT16 d16 = rdop16(); if ( regs.F & FLAG_C ) { CALL( regs.PC + d16 - 1 ); } }
OP(E9) { UINT16 d16 = rdop16(); if ( ! ( regs.F & FLAG_C ) ) { CALL( regs.PC + d16 - 1 ); } }
OP(EA) { UINT16 d16 = rdop16(); if ( regs.F & FLAG_Z ) { CALL( regs.PC + d16 - 1 ); } }
OP(EB) { UINT16 d16 = rdop16(); if ( ! ( regs.F & FLAG_Z ) ) { CALL( regs.PC + d16 - 1 ); } }
OP(EC) { UINT16 d16 = rdop16(); if ( regs.F & FLAG_C ) { JMP( regs.PC + d16 - 1 ); } }
OP(ED) { UINT16 d16 = rdop16(); if ( ! ( regs.F & FLAG_C ) ) { JMP( regs.PC + d16 - 1 ); } }
OP(EE) { UINT16 d16 = rdop16(); if ( regs.F & FLAG_Z ) { JMP( regs.PC + d16 - 1 ); } }
OP(EF) { UINT16 d16 = rdop16(); if ( ! ( regs.F & FLAG_Z ) ) { JMP( regs.PC + d16 - 1 ); } }

OP(F0) { INT8 d8 = rdop(); CALL( regs.PC + d8 - 1 ); }
OP(F1) { INT8 d8 = rdop(); JMP( regs.PC + d8 - 1 ); }
OP(F2) { UINT16 d16 = rdop16(); CALL( regs.PC + d16 - 1 ); }
OP(F3) { UINT16 d16 = rdop16(); JMP( regs.PC + d16 - 1 ); }
OP(F4) { JMP( regs.HL ); }
OP(F5) { INT8 d8 = rdop(); regs.BA = regs.BA - 0x0100; if ( regs.BA & 0xFF00 ) { JMP( regs.PC + d8 - 1 ); } }
OP(F6) { regs.BA = ( regs.BA & 0xFF00 ) | ( ( regs.BA & 0x00F0 ) >> 4 ) | ( ( regs.BA & 0x000F ) << 4 ); }
OP(F7) { UINT8 d; AD1_IHL; d = rd( addr1 ); wr( addr1, ( ( d & 0xF0 ) >> 4 ) | ( ( d & 0x0F ) << 4 ) ); }
OP(F8) { regs.PC = POP16(); regs.V = POP8(); }
OP(F9) { regs.F = POP8(); regs.PC = POP16(); regs.V = POP8(); }
OP(FA) { regs.PC = POP16() + 2; regs.V = POP8(); }
OP(FB) { AD1_I16; CALL( rd16( addr1 ) ); }
OP(FC) { UINT16 i = rdop() << 1; CALL( i ); PUSH8( regs.F ); }
OP(FD) { UINT16 i = rdop() << 1; JMP( i ); /* PUSH8( regs.F );?? */ }
OP(FE) { /* illegal operation? */ }
OP(FF) { }

static void (*insnminx[256])(void) = {
	minx_00, minx_01, minx_02, minx_03, minx_04, minx_05, minx_06, minx_07,
	minx_08, minx_09, minx_0A, minx_0B, minx_0C, minx_0D, minx_0E, minx_0F,
	minx_10, minx_11, minx_12, minx_13, minx_14, minx_15, minx_16, minx_17,
	minx_18, minx_19, minx_1A, minx_1B, minx_1C, minx_1D, minx_1E, minx_1F,
	minx_20, minx_21, minx_22, minx_23, minx_24, minx_25, minx_26, minx_27,
	minx_28, minx_29, minx_2A, minx_2B, minx_2C, minx_2D, minx_2E, minx_2F,
	minx_30, minx_31, minx_32, minx_33, minx_34, minx_35, minx_36, minx_37,
	minx_38, minx_39, minx_3A, minx_3B, minx_3C, minx_3D, minx_3E, minx_3F,
	minx_40, minx_41, minx_42, minx_43, minx_44, minx_45, minx_46, minx_47,
	minx_48, minx_49, minx_4A, minx_4B, minx_4C, minx_4D, minx_4E, minx_4F,
	minx_50, minx_51, minx_52, minx_53, minx_54, minx_55, minx_56, minx_57,
	minx_58, minx_59, minx_5A, minx_5B, minx_5C, minx_5D, minx_5E, minx_5F,
	minx_60, minx_61, minx_62, minx_63, minx_64, minx_65, minx_66, minx_67,
	minx_68, minx_69, minx_6A, minx_6B, minx_6C, minx_6D, minx_6E, minx_6F,
	minx_70, minx_71, minx_72, minx_73, minx_74, minx_75, minx_76, minx_77,
	minx_78, minx_79, minx_7A, minx_7B, minx_7C, minx_7D, minx_7E, minx_7F,
	minx_80, minx_81, minx_82, minx_83, minx_84, minx_85, minx_86, minx_87,
	minx_88, minx_89, minx_8A, minx_8B, minx_8C, minx_8D, minx_8E, minx_8F,
	minx_90, minx_91, minx_92, minx_93, minx_94, minx_95, minx_96, minx_97,
	minx_98, minx_99, minx_9A, minx_9B, minx_9C, minx_9D, minx_9E, minx_9F,
	minx_A0, minx_A1, minx_A2, minx_A3, minx_A4, minx_A5, minx_A6, minx_A7,
	minx_A8, minx_A9, minx_AA, minx_AB, minx_AC, minx_AD, minx_AE, minx_AF,
	minx_B0, minx_B1, minx_B2, minx_B3, minx_B4, minx_B5, minx_B6, minx_B7,
	minx_B8, minx_B9, minx_BA, minx_BB, minx_BC, minx_BD, minx_BE, minx_BF,
	minx_C0, minx_C1, minx_C2, minx_C3, minx_C4, minx_C5, minx_C6, minx_C7,
	minx_C8, minx_C9, minx_CA, minx_CB, minx_CC, minx_CD, minx_CE, minx_CF,
	minx_D0, minx_D1, minx_D2, minx_D3, minx_D4, minx_D5, minx_D6, minx_D7,
	minx_D8, minx_D9, minx_DA, minx_DB, minx_DC, minx_DD, minx_DE, minx_DF,
	minx_E0, minx_E1, minx_E2, minx_E3, minx_E4, minx_E5, minx_E6, minx_E7,
	minx_E8, minx_E9, minx_EA, minx_EB, minx_EC, minx_ED, minx_EE, minx_EF,
	minx_F0, minx_F1, minx_F2, minx_F3, minx_F4, minx_F5, minx_F6, minx_F7,
	minx_F8, minx_F9, minx_FA, minx_FB, minx_FC, minx_FD, minx_FE, minx_FF
};

static int insnminx_cycles[256] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

