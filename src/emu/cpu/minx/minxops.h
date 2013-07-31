#undef OP
#define OP(nn) void minx_cpu_device::minx_##nn()

OP(00) { m_BA = ( m_BA & 0xFF00 ) | ADD8( ( m_BA & 0x00FF ), ( m_BA & 0xFF ) ); }
OP(01) { m_BA = ( m_BA & 0xFF00 ) | ADD8( ( m_BA & 0x00FF ), ( m_BA >> 8 ) ); }
OP(02) { m_BA = ( m_BA & 0xFF00 ) | ADD8( ( m_BA & 0x00FF ), rdop() ); }
OP(03) { AD2_IHL; m_BA = ( m_BA & 0xFF00 ) | ADD8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(04) { AD2_IN8; m_BA = ( m_BA & 0xFF00 ) | ADD8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(05) { AD2_I16; m_BA = ( m_BA & 0xFF00 ) | ADD8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(06) { AD2_XIX; m_BA = ( m_BA & 0xFF00 ) | ADD8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(07) { AD2_YIY; m_BA = ( m_BA & 0xFF00 ) | ADD8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(08) { m_BA = ( m_BA & 0xFF00 ) | ADDC8( ( m_BA & 0x00FF ), ( m_BA & 0xFF ) ); }
OP(09) { m_BA = ( m_BA & 0xFF00 ) | ADDC8( ( m_BA & 0x00FF ), ( m_BA >> 8 ) ); }
OP(0A) { m_BA = ( m_BA & 0xFF00 ) | ADDC8( ( m_BA & 0x00FF ), rdop() ); }
OP(0B) { AD2_IHL; m_BA = ( m_BA & 0xFF00 ) | ADDC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(0C) { AD2_IN8; m_BA = ( m_BA & 0xFF00 ) | ADDC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(0D) { AD2_I16; m_BA = ( m_BA & 0xFF00 ) | ADDC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(0E) { AD2_XIX; m_BA = ( m_BA & 0xFF00 ) | ADDC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(0F) { AD2_YIY; m_BA = ( m_BA & 0xFF00 ) | ADDC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }

OP(10) { m_BA = ( m_BA & 0xFF00 ) | SUB8( ( m_BA & 0x00FF ), ( m_BA & 0xFF ) ); }
OP(11) { m_BA = ( m_BA & 0xFF00 ) | SUB8( ( m_BA & 0x00FF ), ( m_BA >> 8 ) ); }
OP(12) { m_BA = ( m_BA & 0xFF00 ) | SUB8( ( m_BA & 0x00FF ), rdop() ); }
OP(13) { AD2_IHL; m_BA = ( m_BA & 0xFF00 ) | SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(14) { AD2_IN8; m_BA = ( m_BA & 0xFF00 ) | SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(15) { AD2_I16; m_BA = ( m_BA & 0xFF00 ) | SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(16) { AD2_XIX; m_BA = ( m_BA & 0xFF00 ) | SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(17) { AD2_YIY; m_BA = ( m_BA & 0xFF00 ) | SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(18) { m_BA = ( m_BA & 0xFF00 ) | SUBC8( ( m_BA & 0x00FF ), ( m_BA & 0xFF ) ); }
OP(19) { m_BA = ( m_BA & 0xFF00 ) | SUBC8( ( m_BA & 0x00FF ), ( m_BA >> 8 ) ); }
OP(1A) { m_BA = ( m_BA & 0xFF00 ) | SUBC8( ( m_BA & 0x00FF ), rdop() ); }
OP(1B) { AD2_IHL; m_BA = ( m_BA & 0xFF00 ) | SUBC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(1C) { AD2_IN8; m_BA = ( m_BA & 0xFF00 ) | SUBC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(1D) { AD2_I16; m_BA = ( m_BA & 0xFF00 ) | SUBC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(1E) { AD2_XIX; m_BA = ( m_BA & 0xFF00 ) | SUBC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(1F) { AD2_YIY; m_BA = ( m_BA & 0xFF00 ) | SUBC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }

OP(20) { m_BA = ( m_BA & 0xFF00 ) | AND8( ( m_BA & 0x00FF ), ( m_BA & 0xFF ) ); }
OP(21) { m_BA = ( m_BA & 0xFF00 ) | AND8( ( m_BA & 0x00FF ), ( m_BA >> 8 ) ); }
OP(22) { m_BA = ( m_BA & 0xFF00 ) | AND8( ( m_BA & 0x00FF ), rdop() ); }
OP(23) { AD2_IHL; m_BA = ( m_BA & 0xFF00 ) | AND8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(24) { AD2_IN8; m_BA = ( m_BA & 0xFF00 ) | AND8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(25) { AD2_I16; m_BA = ( m_BA & 0xFF00 ) | AND8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(26) { AD2_XIX; m_BA = ( m_BA & 0xFF00 ) | AND8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(27) { AD2_YIY; m_BA = ( m_BA & 0xFF00 ) | AND8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(28) { m_BA = ( m_BA & 0xFF00 ) | OR8( ( m_BA & 0x00FF ), ( m_BA & 0xFF ) ); }
OP(29) { m_BA = ( m_BA & 0xFF00 ) | OR8( ( m_BA & 0x00FF ), ( m_BA >> 8 ) ); }
OP(2A) { m_BA = ( m_BA & 0xFF00 ) | OR8( ( m_BA & 0x00FF ), rdop() ); }
OP(2B) { AD2_IHL; m_BA = ( m_BA & 0xFF00 ) | OR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(2C) { AD2_IN8; m_BA = ( m_BA & 0xFF00 ) | OR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(2D) { AD2_I16; m_BA = ( m_BA & 0xFF00 ) | OR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(2E) { AD2_XIX; m_BA = ( m_BA & 0xFF00 ) | OR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(2F) { AD2_YIY; m_BA = ( m_BA & 0xFF00 ) | OR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }

OP(30) { SUB8( ( m_BA & 0x00FF ), ( m_BA & 0xFF ) ); }
OP(31) { SUB8( ( m_BA & 0x00FF ), ( m_BA >> 8 ) ); }
OP(32) { SUB8( ( m_BA & 0x00FF ), rdop() ); }
OP(33) { AD2_IHL; SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(34) { AD2_IN8; SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(35) { AD2_I16; SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(36) { AD2_XIX; SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(37) { AD2_YIY; SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(38) { m_BA = ( m_BA & 0xFF00 ) | XOR8( ( m_BA & 0x00FF ), ( m_BA & 0xFF ) ); }
OP(39) { m_BA = ( m_BA & 0xFF00 ) | XOR8( ( m_BA & 0x00FF ), ( m_BA >> 8 ) ); }
OP(3A) { m_BA = ( m_BA & 0xFF00 ) | XOR8( ( m_BA & 0x00FF ), rdop() ); }
OP(3B) { AD2_IHL; m_BA = ( m_BA & 0xFF00 ) | XOR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(3C) { AD2_IN8; m_BA = ( m_BA & 0xFF00 ) | XOR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(3D) { AD2_I16; m_BA = ( m_BA & 0xFF00 ) | XOR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(3E) { AD2_XIX; m_BA = ( m_BA & 0xFF00 ) | XOR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(3F) { AD2_YIY; m_BA = ( m_BA & 0xFF00 ) | XOR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }

OP(40) { m_BA = ( m_BA & 0xFF00 ) | ( m_BA & 0x00FF); }
OP(41) { m_BA = ( m_BA & 0xFF00 ) | ( m_BA >> 8 ); }
OP(42) { m_BA = ( m_BA & 0xFF00 ) | ( m_HL & 0x00FF); }
OP(43) { m_BA = ( m_BA & 0xFF00 ) | ( m_HL >> 8 ); }
OP(44) { AD2_IN8; m_BA = ( m_BA & 0xFF00 ) | RD( addr2 ); }
OP(45) { AD2_IHL; m_BA = ( m_BA & 0xFF00 ) | RD( addr2 ); }
OP(46) { AD2_XIX; m_BA = ( m_BA & 0xFF00 ) | RD( addr2 ); }
OP(47) { AD2_YIY; m_BA = ( m_BA & 0xFF00 ) | RD( addr2 ); }
OP(48) { m_BA = ( m_BA & 0x00FF ) | ( ( m_BA & 0x00FF) << 8 ); }
OP(49) { m_BA = ( m_BA & 0x00FF ) | ( ( m_BA >> 8 ) << 8 ); }
OP(4A) { m_BA = ( m_BA & 0x00FF ) | ( ( m_HL & 0x00FF) << 8 ); }
OP(4B) { m_BA = ( m_BA & 0x00FF ) | ( ( m_HL >> 8 ) << 8 ); }
OP(4C) { AD2_IN8; m_BA = ( m_BA & 0x00FF ) | ( RD( addr2 ) << 8 ); }
OP(4D) { AD2_IHL; m_BA = ( m_BA & 0x00FF ) | ( RD( addr2 ) << 8 ); }
OP(4E) { AD2_XIX; m_BA = ( m_BA & 0x00FF ) | ( RD( addr2 ) << 8 ); }
OP(4F) { AD2_YIY; m_BA = ( m_BA & 0x00FF ) | ( RD( addr2 ) << 8 ); }

OP(50) { m_HL = ( m_HL & 0xFF00 ) | ( m_BA & 0x00FF); }
OP(51) { m_HL = ( m_HL & 0xFF00 ) | ( m_BA >> 8 ); }
OP(52) { m_HL = ( m_HL & 0xFF00 ) | ( m_HL & 0x00FF); }
OP(53) { m_HL = ( m_HL & 0xFF00 ) | ( m_HL >> 8 ); }
OP(54) { AD2_IN8; m_HL = ( m_HL & 0xFF00 ) | RD( addr2 ); }
OP(55) { AD2_IHL; m_HL = ( m_HL & 0xFF00 ) | RD( addr2 ); }
OP(56) { AD2_XIX; m_HL = ( m_HL & 0xFF00 ) | RD( addr2 ); }
OP(57) { AD2_YIY; m_HL = ( m_HL & 0xFF00 ) | RD( addr2 ); }
OP(58) { m_HL = ( m_HL & 0x00FF ) | ( ( m_BA & 0x00FF) << 8 ); }
OP(59) { m_HL = ( m_HL & 0x00FF ) | ( ( m_BA >> 8 ) << 8 ); }
OP(5A) { m_HL = ( m_HL & 0x00FF ) | ( ( m_HL & 0x00FF) << 8 ); }
OP(5B) { m_HL = ( m_HL & 0x00FF ) | ( ( m_HL >> 8 ) << 8 ); }
OP(5C) { AD2_IN8; m_HL = ( m_HL & 0x00FF ) | ( RD( addr2 ) << 8 ); }
OP(5D) { AD2_IHL; m_HL = ( m_HL & 0x00FF ) | ( RD( addr2 ) << 8 ); }
OP(5E) { AD2_XIX; m_HL = ( m_HL & 0x00FF ) | ( RD( addr2 ) << 8 ); }
OP(5F) { AD2_YIY; m_HL = ( m_HL & 0x00FF ) | ( RD( addr2 ) << 8 ); }

OP(60) { AD1_XIX; WR( addr1, ( m_BA & 0x00FF ) ); }
OP(61) { AD1_XIX; WR( addr1, ( m_BA >> 8 ) ); }
OP(62) { AD1_XIX; WR( addr1, ( m_HL & 0x00FF ) ); }
OP(63) { AD1_XIX; WR( addr1, ( m_HL >> 8 ) ); }
OP(64) { AD1_XIX; AD2_IN8; WR( addr1, RD( addr2 ) ); }
OP(65) { AD1_XIX; AD2_IHL; WR( addr1, RD( addr2 ) ); }
OP(66) { AD1_XIX; AD2_XIX; WR( addr1, RD( addr2 ) ); }
OP(67) { AD1_XIX; AD2_YIY; WR( addr1, RD( addr2 ) ); }
OP(68) { AD1_IHL; WR( addr1, ( m_BA & 0x00FF ) ); }
OP(69) { AD1_IHL; WR( addr1, ( m_BA >> 8 ) ); }
OP(6A) { AD1_IHL; WR( addr1, ( m_HL & 0x00FF ) ); }
OP(6B) { AD1_IHL; WR( addr1, ( m_HL >> 8 ) ); }
OP(6C) { AD1_IHL; AD2_IN8; WR( addr1, RD( addr2 ) ); }
OP(6D) { AD1_IHL; AD2_IHL; WR( addr1, RD( addr2 ) ); }
OP(6E) { AD1_IHL; AD2_XIX; WR( addr1, RD( addr2 ) ); }
OP(6F) { AD1_IHL; AD2_YIY; WR( addr1, RD( addr2 ) ); }

OP(70) { AD1_YIY; WR( addr1, ( m_BA & 0x00FF ) ); }
OP(71) { AD1_YIY; WR( addr1, ( m_BA >> 8 ) ); }
OP(72) { AD1_YIY; WR( addr1, ( m_HL & 0x00FF ) ); }
OP(73) { AD1_YIY; WR( addr1, ( m_HL >> 8 ) ); }
OP(74) { AD1_YIY; AD2_IN8; WR( addr1, RD( addr2 ) ); }
OP(75) { AD1_YIY; AD2_IHL; WR( addr1, RD( addr2 ) ); }
OP(76) { AD1_YIY; AD2_XIX; WR( addr1, RD( addr2 ) ); }
OP(77) { AD1_YIY; AD2_YIY; WR( addr1, RD( addr2 ) ); }
OP(78) { AD1_IN8; WR( addr1, ( m_BA & 0x00FF ) ); }
OP(79) { AD1_IN8; WR( addr1, ( m_BA >> 8 ) ); }
OP(7A) { AD1_IN8; WR( addr1, ( m_HL & 0x00FF ) ); }
OP(7B) { AD1_IN8; WR( addr1, ( m_HL >> 8 ) ); }
OP(7C) { /* illegal operation? */ }
OP(7D) { AD1_IN8; AD2_IHL; WR( addr1, RD( addr2 ) ); }
OP(7E) { AD1_IN8; AD2_XIX; WR( addr1, RD( addr2 ) ); }
OP(7F) { AD1_IN8; AD2_YIY; WR( addr1, RD( addr2 ) ); }

OP(80) { m_BA = ( m_BA & 0xFF00 ) | INC8( m_BA & 0x00FF ); }
OP(81) { m_BA = ( m_BA & 0x00FF ) | ( INC8( m_BA >> 8 ) << 8 ); }
OP(82) { m_HL = ( m_HL & 0xFF00 ) | INC8( m_HL & 0x00FF ); }
OP(83) { m_HL = ( m_HL & 0x00FF ) | ( INC8( m_HL >> 8 ) << 8 ); }
OP(84) { m_N = INC8( m_N ); }
OP(85) { AD1_IN8; WR( addr1, INC8( RD( addr1 ) ) ); }
OP(86) { AD1_IHL; WR( addr1, INC8( RD( addr1 ) ) ); }
OP(87) { m_SP = INC16( m_SP ); }
OP(88) { m_BA = ( m_BA & 0xFF00 ) | DEC8( m_BA & 0x00FF ); }
OP(89) { m_BA = ( m_BA & 0x00FF ) | ( DEC8( m_BA >> 8 ) << 8 ); }
OP(8A) { m_HL = ( m_HL & 0xFF00 ) | DEC8( m_HL & 0x00FF ); }
OP(8B) { m_HL = ( m_HL & 0x00FF ) | ( DEC8( m_HL >> 8 ) << 8 ); }
OP(8C) { m_N = DEC8( m_N ); }
OP(8D) { AD1_IN8; WR( addr1, DEC8( RD( addr1 ) ) ); }
OP(8E) { AD1_IHL; WR( addr1, DEC8( RD( addr1 ) ) ); }
OP(8F) { m_SP = DEC8( m_SP ); }

OP(90) { m_BA = INC16( m_BA ); }
OP(91) { m_HL = INC16( m_HL ); }
OP(92) { m_X = INC16( m_X ); }
OP(93) { m_Y = INC16( m_Y ); }
OP(94) { m_F = ( AND8( ( m_BA & 0x00FF ), ( m_BA >> 8 ) ) ) ? m_F & ~FLAG_Z : m_F | FLAG_Z;}
OP(95) { AD1_IHL; m_F = ( AND8( RD( addr1 ), rdop() ) ) ? m_F & ~FLAG_Z : m_F | FLAG_Z; }
OP(96) { m_F = ( AND8( ( m_BA & 0x00FF ), rdop() ) ) ? m_F & ~FLAG_Z : m_F | FLAG_Z; }
OP(97) { m_F = ( AND8( ( m_BA >> 8 ), rdop() ) ) ? m_F & ~FLAG_Z : m_F | FLAG_Z; }
OP(98) { m_BA = DEC16( m_BA ); }
OP(99) { m_HL = DEC16( m_HL ); }
OP(9A) { m_X = DEC16( m_X ); }
OP(9B) { m_Y = DEC16( m_Y ); }
OP(9C) { m_F = m_F & rdop(); }
OP(9D) { m_F = m_F | rdop(); }
OP(9E) { m_F = m_F ^ rdop(); }
OP(9F) { m_F = rdop(); }

OP(A0) { PUSH16( m_BA ); }
OP(A1) { PUSH16( m_HL ); }
OP(A2) { PUSH16( m_X ); }
OP(A3) { PUSH16( m_Y ); }
OP(A4) { PUSH8( m_N ); }
OP(A5) { PUSH8( m_I ); }
OP(A6) { PUSH8( m_XI ); PUSH8( m_YI ); }
OP(A7) { PUSH8( m_F ); }
OP(A8) { m_BA = POP16(); }
OP(A9) { m_HL = POP16();}
OP(AA) { m_X = POP16(); }
OP(AB) { m_Y = POP16(); }
OP(AC) { m_N = POP8(); }
OP(AD) { m_I = POP8(); }
OP(AE) { m_YI = POP8(); m_XI = POP8(); }
OP(AF) { m_F = POP8(); }

OP(B0) { UINT8 op = rdop(); m_BA = ( m_BA & 0xFF00 ) | op; }
OP(B1) { UINT8 op = rdop(); m_BA = ( m_BA & 0x00FF ) | ( op << 8 ); }
OP(B2) { UINT8 op = rdop(); m_HL = ( m_HL & 0xFF00 ) | op; }
OP(B3) { UINT8 op = rdop(); m_HL = ( m_HL & 0x00FF ) | ( op << 8 ); }
OP(B4) { UINT8 op = rdop(); m_N = op; }
OP(B5) { AD1_IHL; UINT8 op = rdop(); WR( addr1, op); }
OP(B6) { AD1_XIX; UINT8 op = rdop(); WR( addr1, op ); }
OP(B7) { AD1_YIY; UINT8 op = rdop(); WR( addr1, op ); }
OP(B8) { AD2_I16; m_BA = rd16( addr2 ); }
OP(B9) { AD2_I16; m_HL = rd16( addr2 ); }
OP(BA) { AD2_I16; m_X = rd16( addr2 ); }
OP(BB) { AD2_I16; m_Y = rd16( addr2 ); }
OP(BC) { AD1_I16; wr16( addr1, m_BA ); }
OP(BD) { AD1_I16; wr16( addr1, m_HL ); }
OP(BE) { AD1_I16; wr16( addr1, m_X ); }
OP(BF) { AD1_I16; wr16( addr1, m_Y ); }

OP(C0) { m_BA = ADD16( m_BA, rdop16() ); }
OP(C1) { m_HL = ADD16( m_HL, rdop16() ); }
OP(C2) { m_X = ADD16( m_X, rdop16() ); }
OP(C3) { m_Y = ADD16( m_Y, rdop16() ); }
OP(C4) { m_BA = rdop16(); }
OP(C5) { m_HL = rdop16(); }
OP(C6) { m_X = rdop16(); }
OP(C7) { m_Y = rdop16(); }
OP(C8) { UINT16 t = m_BA; m_BA = m_HL; m_HL = t; }
OP(C9) { UINT16 t = m_BA; m_BA = m_X; m_X = t; }
OP(CA) { UINT16 t = m_BA; m_BA = m_Y; m_Y = t; }
OP(CB) { UINT16 t = m_BA; m_BA = m_SP; m_SP = t; }
OP(CC) { m_BA = ( m_BA >> 8 ) | ( ( m_BA & 0x00FF ) << 8 ); }
OP(CD) { UINT8 t; AD2_IHL; t = RD( addr2 ); WR( addr2, ( m_BA & 0x00FF ) ); m_BA = ( m_BA & 0xFF00 ) | t; }
OP(CE) { UINT8 op = rdop(); (this->*insnminx_CE[op])(); m_icount -= insnminx_cycles_CE[op]; }
OP(CF) { UINT8 op = rdop(); (this->*insnminx_CF[op])(); m_icount -= insnminx_cycles_CF[op]; }

OP(D0) { m_BA = SUB16( m_BA, rdop16() ); }
OP(D1) { m_HL = SUB16( m_HL, rdop16() ); }
OP(D2) { m_X = SUB16( m_X, rdop16() ); }
OP(D3) { m_Y = SUB16( m_Y, rdop16() ); }
OP(D4) { SUB16( m_BA, rdop16() ); }
OP(D5) { SUB16( m_HL, rdop16() ); }
OP(D6) { SUB16( m_X, rdop16() ); }
OP(D7) { SUB16( m_Y, rdop16() ); }
OP(D8) { AD1_IN8; WR( addr1, AND8( RD( addr1 ), rdop() ) ); }
OP(D9) { AD1_IN8; WR( addr1, OR8( RD( addr1 ), rdop() ) ); }
OP(DA) { AD1_IN8; WR( addr1, XOR8( RD( addr1 ), rdop() ) ); }
OP(DB) { AD1_IN8; SUB8( RD( addr1 ), rdop() ); }
OP(DC) { AD1_IN8; m_F = ( AND8( RD( addr1 ), rdop() ) ) ? m_F & ~FLAG_Z : m_F | FLAG_Z; }
OP(DD) { AD1_IN8; WR( addr1, rdop() ); }
OP(DE) { m_BA = ( m_BA & 0xFF00 ) | ( ( m_BA & 0x000F ) | ( ( m_BA & 0x0F00 ) >> 4 ) ); }
OP(DF) { m_BA = ( ( m_BA & 0x0080 ) ? 0xFF00 : 0x0000 ) | ( m_BA & 0x000F ); }

OP(E0) { INT8 d8 = rdop(); if ( m_F & FLAG_C ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
OP(E1) { INT8 d8 = rdop(); if ( ! ( m_F & FLAG_C ) ) { CALL( m_PC + d8- 1  ); m_icount -= 12; } }
OP(E2) { INT8 d8 = rdop(); if ( m_F & FLAG_Z ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
OP(E3) { INT8 d8 = rdop(); if ( ! ( m_F & FLAG_Z ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
OP(E4) { INT8 d8 = rdop(); if ( m_F & FLAG_C ) { JMP( m_PC + d8 - 1 ); } }
OP(E5) { INT8 d8 = rdop(); if ( ! ( m_F & FLAG_C ) ) { JMP( m_PC + d8 - 1 ); } }
OP(E6) { INT8 d8 = rdop(); if ( m_F & FLAG_Z ) { JMP( m_PC + d8 - 1 ); } }
OP(E7) { INT8 d8 = rdop(); if ( ! ( m_F & FLAG_Z ) ) { JMP( m_PC + d8 - 1 ); } }
OP(E8) { UINT16 d16 = rdop16(); if ( m_F & FLAG_C ) { CALL( m_PC + d16 - 1 ); m_icount -= 12; } }
OP(E9) { UINT16 d16 = rdop16(); if ( ! ( m_F & FLAG_C ) ) { CALL( m_PC + d16 - 1 ); m_icount -= 12; } }
OP(EA) { UINT16 d16 = rdop16(); if ( m_F & FLAG_Z ) { CALL( m_PC + d16 - 1 ); m_icount -= 12; } }
OP(EB) { UINT16 d16 = rdop16(); if ( ! ( m_F & FLAG_Z ) ) { CALL( m_PC + d16 - 1 ); m_icount -= 12; } }
OP(EC) { UINT16 d16 = rdop16(); if ( m_F & FLAG_C ) { JMP( m_PC + d16 - 1 ); } }
OP(ED) { UINT16 d16 = rdop16(); if ( ! ( m_F & FLAG_C ) ) { JMP( m_PC + d16 - 1 ); } }
OP(EE) { UINT16 d16 = rdop16(); if ( m_F & FLAG_Z ) { JMP( m_PC + d16 - 1 ); } }
OP(EF) { UINT16 d16 = rdop16(); if ( ! ( m_F & FLAG_Z ) ) { JMP( m_PC + d16 - 1 ); } }

OP(F0) { INT8 d8 = rdop(); CALL( m_PC + d8 - 1 ); }
OP(F1) { INT8 d8 = rdop(); JMP( m_PC + d8 - 1 ); }
OP(F2) { UINT16 d16 = rdop16(); CALL( m_PC + d16 - 1 ); }
OP(F3) { UINT16 d16 = rdop16(); JMP( m_PC + d16 - 1 ); }
OP(F4) { JMP( m_HL ); }
OP(F5) { INT8 d8 = rdop(); m_BA = m_BA - 0x0100; if ( m_BA & 0xFF00 ) { JMP( m_PC + d8 - 1 ); } }
OP(F6) { m_BA = ( m_BA & 0xFF00 ) | ( ( m_BA & 0x00F0 ) >> 4 ) | ( ( m_BA & 0x000F ) << 4 ); }
OP(F7) { UINT8 d; AD1_IHL; d = RD( addr1 ); WR( addr1, ( ( d & 0xF0 ) >> 4 ) | ( ( d & 0x0F ) << 4 ) ); }
OP(F8) { m_PC = POP16(); m_V = POP8(); m_U = m_V; }
OP(F9) { m_F = POP8(); m_PC = POP16(); m_V = POP8(); m_U = m_V; }
OP(FA) { m_PC = POP16() + 2; m_V = POP8(); m_U = m_V; }
OP(FB) { AD1_I16; CALL( rd16( addr1 ) ); }
OP(FC) { UINT8 i = rdop() & 0xFE; CALL( rd16( i ) ); PUSH8( m_F ); }
OP(FD) { UINT8 i = rdop() & 0xFE; JMP( rd16( i ) ); /* PUSH8( m_F );?? */ }
OP(FE) { /* illegal operation? */ }
OP(FF) { }

const minx_cpu_device::op_func minx_cpu_device::insnminx[256] = {
	&minx_cpu_device::minx_00, &minx_cpu_device::minx_01, &minx_cpu_device::minx_02, &minx_cpu_device::minx_03, &minx_cpu_device::minx_04, &minx_cpu_device::minx_05, &minx_cpu_device::minx_06, &minx_cpu_device::minx_07,
	&minx_cpu_device::minx_08, &minx_cpu_device::minx_09, &minx_cpu_device::minx_0A, &minx_cpu_device::minx_0B, &minx_cpu_device::minx_0C, &minx_cpu_device::minx_0D, &minx_cpu_device::minx_0E, &minx_cpu_device::minx_0F,
	&minx_cpu_device::minx_10, &minx_cpu_device::minx_11, &minx_cpu_device::minx_12, &minx_cpu_device::minx_13, &minx_cpu_device::minx_14, &minx_cpu_device::minx_15, &minx_cpu_device::minx_16, &minx_cpu_device::minx_17,
	&minx_cpu_device::minx_18, &minx_cpu_device::minx_19, &minx_cpu_device::minx_1A, &minx_cpu_device::minx_1B, &minx_cpu_device::minx_1C, &minx_cpu_device::minx_1D, &minx_cpu_device::minx_1E, &minx_cpu_device::minx_1F,
	&minx_cpu_device::minx_20, &minx_cpu_device::minx_21, &minx_cpu_device::minx_22, &minx_cpu_device::minx_23, &minx_cpu_device::minx_24, &minx_cpu_device::minx_25, &minx_cpu_device::minx_26, &minx_cpu_device::minx_27,
	&minx_cpu_device::minx_28, &minx_cpu_device::minx_29, &minx_cpu_device::minx_2A, &minx_cpu_device::minx_2B, &minx_cpu_device::minx_2C, &minx_cpu_device::minx_2D, &minx_cpu_device::minx_2E, &minx_cpu_device::minx_2F,
	&minx_cpu_device::minx_30, &minx_cpu_device::minx_31, &minx_cpu_device::minx_32, &minx_cpu_device::minx_33, &minx_cpu_device::minx_34, &minx_cpu_device::minx_35, &minx_cpu_device::minx_36, &minx_cpu_device::minx_37,
	&minx_cpu_device::minx_38, &minx_cpu_device::minx_39, &minx_cpu_device::minx_3A, &minx_cpu_device::minx_3B, &minx_cpu_device::minx_3C, &minx_cpu_device::minx_3D, &minx_cpu_device::minx_3E, &minx_cpu_device::minx_3F,
	&minx_cpu_device::minx_40, &minx_cpu_device::minx_41, &minx_cpu_device::minx_42, &minx_cpu_device::minx_43, &minx_cpu_device::minx_44, &minx_cpu_device::minx_45, &minx_cpu_device::minx_46, &minx_cpu_device::minx_47,
	&minx_cpu_device::minx_48, &minx_cpu_device::minx_49, &minx_cpu_device::minx_4A, &minx_cpu_device::minx_4B, &minx_cpu_device::minx_4C, &minx_cpu_device::minx_4D, &minx_cpu_device::minx_4E, &minx_cpu_device::minx_4F,
	&minx_cpu_device::minx_50, &minx_cpu_device::minx_51, &minx_cpu_device::minx_52, &minx_cpu_device::minx_53, &minx_cpu_device::minx_54, &minx_cpu_device::minx_55, &minx_cpu_device::minx_56, &minx_cpu_device::minx_57,
	&minx_cpu_device::minx_58, &minx_cpu_device::minx_59, &minx_cpu_device::minx_5A, &minx_cpu_device::minx_5B, &minx_cpu_device::minx_5C, &minx_cpu_device::minx_5D, &minx_cpu_device::minx_5E, &minx_cpu_device::minx_5F,
	&minx_cpu_device::minx_60, &minx_cpu_device::minx_61, &minx_cpu_device::minx_62, &minx_cpu_device::minx_63, &minx_cpu_device::minx_64, &minx_cpu_device::minx_65, &minx_cpu_device::minx_66, &minx_cpu_device::minx_67,
	&minx_cpu_device::minx_68, &minx_cpu_device::minx_69, &minx_cpu_device::minx_6A, &minx_cpu_device::minx_6B, &minx_cpu_device::minx_6C, &minx_cpu_device::minx_6D, &minx_cpu_device::minx_6E, &minx_cpu_device::minx_6F,
	&minx_cpu_device::minx_70, &minx_cpu_device::minx_71, &minx_cpu_device::minx_72, &minx_cpu_device::minx_73, &minx_cpu_device::minx_74, &minx_cpu_device::minx_75, &minx_cpu_device::minx_76, &minx_cpu_device::minx_77,
	&minx_cpu_device::minx_78, &minx_cpu_device::minx_79, &minx_cpu_device::minx_7A, &minx_cpu_device::minx_7B, &minx_cpu_device::minx_7C, &minx_cpu_device::minx_7D, &minx_cpu_device::minx_7E, &minx_cpu_device::minx_7F,
	&minx_cpu_device::minx_80, &minx_cpu_device::minx_81, &minx_cpu_device::minx_82, &minx_cpu_device::minx_83, &minx_cpu_device::minx_84, &minx_cpu_device::minx_85, &minx_cpu_device::minx_86, &minx_cpu_device::minx_87,
	&minx_cpu_device::minx_88, &minx_cpu_device::minx_89, &minx_cpu_device::minx_8A, &minx_cpu_device::minx_8B, &minx_cpu_device::minx_8C, &minx_cpu_device::minx_8D, &minx_cpu_device::minx_8E, &minx_cpu_device::minx_8F,
	&minx_cpu_device::minx_90, &minx_cpu_device::minx_91, &minx_cpu_device::minx_92, &minx_cpu_device::minx_93, &minx_cpu_device::minx_94, &minx_cpu_device::minx_95, &minx_cpu_device::minx_96, &minx_cpu_device::minx_97,
	&minx_cpu_device::minx_98, &minx_cpu_device::minx_99, &minx_cpu_device::minx_9A, &minx_cpu_device::minx_9B, &minx_cpu_device::minx_9C, &minx_cpu_device::minx_9D, &minx_cpu_device::minx_9E, &minx_cpu_device::minx_9F,
	&minx_cpu_device::minx_A0, &minx_cpu_device::minx_A1, &minx_cpu_device::minx_A2, &minx_cpu_device::minx_A3, &minx_cpu_device::minx_A4, &minx_cpu_device::minx_A5, &minx_cpu_device::minx_A6, &minx_cpu_device::minx_A7,
	&minx_cpu_device::minx_A8, &minx_cpu_device::minx_A9, &minx_cpu_device::minx_AA, &minx_cpu_device::minx_AB, &minx_cpu_device::minx_AC, &minx_cpu_device::minx_AD, &minx_cpu_device::minx_AE, &minx_cpu_device::minx_AF,
	&minx_cpu_device::minx_B0, &minx_cpu_device::minx_B1, &minx_cpu_device::minx_B2, &minx_cpu_device::minx_B3, &minx_cpu_device::minx_B4, &minx_cpu_device::minx_B5, &minx_cpu_device::minx_B6, &minx_cpu_device::minx_B7,
	&minx_cpu_device::minx_B8, &minx_cpu_device::minx_B9, &minx_cpu_device::minx_BA, &minx_cpu_device::minx_BB, &minx_cpu_device::minx_BC, &minx_cpu_device::minx_BD, &minx_cpu_device::minx_BE, &minx_cpu_device::minx_BF,
	&minx_cpu_device::minx_C0, &minx_cpu_device::minx_C1, &minx_cpu_device::minx_C2, &minx_cpu_device::minx_C3, &minx_cpu_device::minx_C4, &minx_cpu_device::minx_C5, &minx_cpu_device::minx_C6, &minx_cpu_device::minx_C7,
	&minx_cpu_device::minx_C8, &minx_cpu_device::minx_C9, &minx_cpu_device::minx_CA, &minx_cpu_device::minx_CB, &minx_cpu_device::minx_CC, &minx_cpu_device::minx_CD, &minx_cpu_device::minx_CE, &minx_cpu_device::minx_CF,
	&minx_cpu_device::minx_D0, &minx_cpu_device::minx_D1, &minx_cpu_device::minx_D2, &minx_cpu_device::minx_D3, &minx_cpu_device::minx_D4, &minx_cpu_device::minx_D5, &minx_cpu_device::minx_D6, &minx_cpu_device::minx_D7,
	&minx_cpu_device::minx_D8, &minx_cpu_device::minx_D9, &minx_cpu_device::minx_DA, &minx_cpu_device::minx_DB, &minx_cpu_device::minx_DC, &minx_cpu_device::minx_DD, &minx_cpu_device::minx_DE, &minx_cpu_device::minx_DF,
	&minx_cpu_device::minx_E0, &minx_cpu_device::minx_E1, &minx_cpu_device::minx_E2, &minx_cpu_device::minx_E3, &minx_cpu_device::minx_E4, &minx_cpu_device::minx_E5, &minx_cpu_device::minx_E6, &minx_cpu_device::minx_E7,
	&minx_cpu_device::minx_E8, &minx_cpu_device::minx_E9, &minx_cpu_device::minx_EA, &minx_cpu_device::minx_EB, &minx_cpu_device::minx_EC, &minx_cpu_device::minx_ED, &minx_cpu_device::minx_EE, &minx_cpu_device::minx_EF,
	&minx_cpu_device::minx_F0, &minx_cpu_device::minx_F1, &minx_cpu_device::minx_F2, &minx_cpu_device::minx_F3, &minx_cpu_device::minx_F4, &minx_cpu_device::minx_F5, &minx_cpu_device::minx_F6, &minx_cpu_device::minx_F7,
	&minx_cpu_device::minx_F8, &minx_cpu_device::minx_F9, &minx_cpu_device::minx_FA, &minx_cpu_device::minx_FB, &minx_cpu_device::minx_FC, &minx_cpu_device::minx_FD, &minx_cpu_device::minx_FE, &minx_cpu_device::minx_FF
};

const int minx_cpu_device::insnminx_cycles[256] = {
	8,  8,  8,  8, 12, 16,  8,  8,  8,  8,  8,  8, 12, 16,  8,  8,
	8,  8,  8,  8, 12, 16,  8,  8,  8,  8,  8,  8, 12, 16,  8,  8,
	8,  8,  8,  8, 12, 16,  8,  8,  8,  8,  8,  8, 12, 16,  8,  8,
	8,  8,  8,  8, 12, 16,  8,  8,  8,  8,  8,  8, 12, 16,  8,  8,

	4,  4,  4,  4, 12,  8,  8,  8,  4,  4,  4,  4, 12,  8,  8,  8,
	4,  4,  4,  4, 12,  8,  8,  8,  4,  4,  4,  4, 12,  8,  8,  8,
	8,  8,  8,  8, 16, 12, 12, 12,  8,  8,  8,  8, 16, 12, 12, 12,
	8,  8,  8,  8, 16, 12, 12, 12, 12, 12, 12, 12,  1, 16, 16, 16,

	8,  8,  8,  8,  8, 16, 12,  8,  8,  8,  8,  8,  8, 16, 12,  8,
	8,  8,  8,  8,  8, 12,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12,
	16, 16, 16, 16, 12, 12, 16, 12, 12, 12, 12, 12,  8,  8, 12,  8,
	8,  8,  8,  8,  8, 12, 12, 12, 20, 20, 20, 20,  1,  1,  1,  1,

	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,  8, 12,  0,  0,
	12, 12, 12, 12, 12, 12, 12, 12, 20, 20, 20, 16, 16, 16,  8,  8,
	8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 12, 12, 12, 12, 12,
	20,  8, 24, 12,  8,  1,  8, 12,  8,  8,  8, 20, 20,  1,  1,  8
};
