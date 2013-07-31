#undef OP
#define OP(nn) void minx_cpu_device::minx_CE_##nn()

OP(00) { AD2_X8; m_BA = ( m_BA & 0xFF00 ) | ADD8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(01) { AD2_Y8; m_BA = ( m_BA & 0xFF00 ) | ADD8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(02) { AD2_XL; m_BA = ( m_BA & 0xFF00 ) | ADD8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(03) { AD2_YL; m_BA = ( m_BA & 0xFF00 ) | ADD8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(04) { AD1_IHL; WR( addr1, ADD8( RD( addr1 ), ( m_BA & 0x00FF ) ) ); }
OP(05) { AD1_IHL; WR( addr1, ADD8( RD( addr1 ), rdop() ) ); }
OP(06) { AD1_IHL; AD2_XIX; WR( addr1, ADD8( RD( addr1 ), RD( addr2 ) ) ); }
OP(07) { AD1_IHL; AD2_YIY; WR( addr1, ADD8( RD( addr1 ), RD( addr2 ) ) ); }
OP(08) { AD2_X8; m_BA = ( m_BA & 0xFF00 ) | ADDC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(09) { AD2_Y8; m_BA = ( m_BA & 0xFF00 ) | ADDC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(0A) { AD2_XL; m_BA = ( m_BA & 0xFF00 ) | ADDC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(0B) { AD2_YL; m_BA = ( m_BA & 0xFF00 ) | ADDC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(0C) { AD1_IHL; WR( addr1, ADDC8( RD( addr1 ), ( m_BA & 0x00FF ) ) ); }
OP(0D) { AD1_IHL; WR( addr1, ADDC8( RD( addr1 ), rdop() ) ); }
OP(0E) { AD1_IHL; AD2_XIX; WR( addr1, ADDC8( RD( addr1 ), RD( addr2 ) ) ); }
OP(0F) { AD1_IHL; AD2_YIY; WR( addr1, ADDC8( RD( addr1 ), RD( addr2 ) ) ); }

OP(10) { AD2_X8; m_BA = ( m_BA & 0xFF00 ) | SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(11) { AD2_Y8; m_BA = ( m_BA & 0xFF00 ) | SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(12) { AD2_XL; m_BA = ( m_BA & 0xFF00 ) | SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(13) { AD2_YL; m_BA = ( m_BA & 0xFF00 ) | SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(14) { AD1_IHL; WR( addr1, SUB8( RD( addr1 ), ( m_BA & 0x00FF ) ) ); }
OP(15) { AD1_IHL; WR( addr1, SUB8( RD( addr1 ), rdop() ) ); }
OP(16) { AD1_IHL; AD2_XIX; WR( addr1, SUB8( RD( addr1 ), RD( addr2 ) ) ); }
OP(17) { AD1_IHL; AD2_YIY; WR( addr1, SUB8( RD( addr1 ), RD( addr2 ) ) ); }
OP(18) { AD2_X8; m_BA = ( m_BA & 0xFF00 ) | SUBC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(19) { AD2_Y8; m_BA = ( m_BA & 0xFF00 ) | SUBC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(1A) { AD2_XL; m_BA = ( m_BA & 0xFF00 ) | SUBC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(1B) { AD2_YL; m_BA = ( m_BA & 0xFF00 ) | SUBC8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(1C) { AD1_IHL; WR( addr1, SUBC8( RD( addr1 ), ( m_BA & 0x00FF ) ) ); }
OP(1D) { AD1_IHL; WR( addr1, SUBC8( RD( addr1 ), rdop() ) ); }
OP(1E) { AD1_IHL; AD2_XIX; WR( addr1, SUBC8( RD( addr1 ), RD( addr2 ) ) ); }
OP(1F) { AD1_IHL; AD2_YIY; WR( addr1, SUBC8( RD( addr1 ), RD( addr2 ) ) ); }

OP(20) { AD2_X8; m_BA = ( m_BA & 0xFF00 ) | AND8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(21) { AD2_Y8; m_BA = ( m_BA & 0xFF00 ) | AND8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(22) { AD2_XL; m_BA = ( m_BA & 0xFF00 ) | AND8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(23) { AD2_YL; m_BA = ( m_BA & 0xFF00 ) | AND8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(24) { AD1_IHL; WR( addr1, AND8( RD( addr1 ), ( m_BA & 0x00FF ) ) ); }
OP(25) { AD1_IHL; WR( addr1, AND8( RD( addr1 ), rdop() ) ); }
OP(26) { AD1_IHL; AD2_XIX; WR( addr1, AND8( RD( addr1 ), RD( addr2 ) ) ); }
OP(27) { AD1_IHL; AD2_YIY; WR( addr1, AND8( RD( addr1 ), RD( addr2 ) ) ); }
OP(28) { AD2_X8; m_BA = ( m_BA & 0xFF00 ) | OR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(29) { AD2_Y8; m_BA = ( m_BA & 0xFF00 ) | OR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(2A) { AD2_XL; m_BA = ( m_BA & 0xFF00 ) | OR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(2B) { AD2_YL; m_BA = ( m_BA & 0xFF00 ) | OR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(2C) { AD1_IHL; WR( addr1, OR8( RD( addr1 ), ( m_BA & 0x00FF ) ) ); }
OP(2D) { AD1_IHL; WR( addr1, OR8( RD( addr1 ), rdop() ) ); }
OP(2E) { AD1_IHL; AD2_XIX; WR( addr1, OR8( RD( addr1 ), RD( addr2 ) ) ); }
OP(2F) { AD1_IHL; AD2_YIY; WR( addr1, OR8( RD( addr1 ), RD( addr2 ) ) ); }

OP(30) { AD2_X8; SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(31) { AD2_Y8; SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(32) { AD2_XL; SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(33) { AD2_YL; SUB8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(34) { AD1_IHL; SUB8( RD( addr1 ), ( m_BA & 0x00FF ) ); }
OP(35) { AD1_IHL; SUB8( RD( addr1 ), rdop() ); }
OP(36) { AD1_IHL; AD2_XIX; SUB8( RD( addr1 ), RD( addr2 ) ); }
OP(37) { AD1_IHL; AD2_YIY; SUB8( RD( addr1 ), RD( addr2 ) ); }
OP(38) { AD2_X8; m_BA = ( m_BA & 0xFF00 ) | XOR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(39) { AD2_Y8; m_BA = ( m_BA & 0xFF00 ) | XOR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(3A) { AD2_XL; m_BA = ( m_BA & 0xFF00 ) | XOR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(3B) { AD2_YL; m_BA = ( m_BA & 0xFF00 ) | XOR8( ( m_BA & 0x00FF ), RD( addr2 ) ); }
OP(3C) { AD1_IHL; WR( addr1, XOR8( RD( addr1 ), ( m_BA & 0x00FF ) ) ); }
OP(3D) { AD1_IHL; WR( addr1, XOR8( RD( addr1 ), rdop() ) ); }
OP(3E) { AD1_IHL; AD2_XIX; WR( addr1, XOR8( RD( addr1 ), RD( addr2 ) ) ); }
OP(3F) { AD1_IHL; AD2_YIY; WR( addr1, XOR8( RD( addr1 ), RD( addr2 ) ) ); }

OP(40) { AD2_X8; m_BA = ( m_BA & 0xFF00 ) | RD( addr2 ); }
OP(41) { AD2_Y8; m_BA = ( m_BA & 0xFF00 ) | RD( addr2 ); }
OP(42) { AD2_XL; m_BA = ( m_BA & 0xFF00 ) | RD( addr2 ); }
OP(43) { AD2_YL; m_BA = ( m_BA & 0xFF00 ) | RD( addr2 ); }
OP(44) { AD1_X8; WR( addr1, ( m_BA & 0x00FF ) ); }
OP(45) { AD1_Y8; WR( addr1, ( m_BA & 0x00FF ) ); }
OP(46) { AD1_XL; WR( addr1, ( m_BA & 0x00FF ) ); }
OP(47) { AD1_YL; WR( addr1, ( m_BA & 0x00FF ) ); }
OP(48) { AD2_X8; m_BA = ( m_BA & 0x00FF ) | ( RD( addr2 ) << 8 ); }
OP(49) { AD2_Y8; m_BA = ( m_BA & 0x00FF ) | ( RD( addr2 ) << 8 ); }
OP(4A) { AD2_XL; m_BA = ( m_BA & 0x00FF ) | ( RD( addr2 ) << 8 ); }
OP(4B) { AD2_YL; m_BA = ( m_BA & 0x00FF ) | ( RD( addr2 ) << 8 ); }
OP(4C) { AD1_X8; WR( addr1, ( m_BA >> 8 ) ); }
OP(4D) { AD1_Y8; WR( addr1, ( m_BA >> 8 ) ); }
OP(4E) { AD1_XL; WR( addr1, ( m_BA >> 8 ) ); }
OP(4F) { AD1_YL; WR( addr1, ( m_BA >> 8 ) ); }

OP(50) { AD2_X8; m_HL = ( m_HL & 0xFF00 ) | RD( addr2 ); }
OP(51) { AD2_Y8; m_HL = ( m_HL & 0xFF00 ) | RD( addr2 ); }
OP(52) { AD2_XL; m_HL = ( m_HL & 0xFF00 ) | RD( addr2 ); }
OP(53) { AD2_YL; m_HL = ( m_HL & 0xFF00 ) | RD( addr2 ); }
OP(54) { AD1_X8; WR( addr1, ( m_HL & 0x00FF ) ); }
OP(55) { AD1_Y8; WR( addr1, ( m_HL & 0x00FF ) ); }
OP(56) { AD1_XL; WR( addr1, ( m_HL & 0x00FF ) ); }
OP(57) { AD1_YL; WR( addr1, ( m_HL & 0x00FF ) ); }
OP(58) { AD2_X8; m_HL = ( m_HL & 0x00FF ) | ( RD( addr2 ) << 8 ); }
OP(59) { AD2_Y8; m_HL = ( m_HL & 0x00FF ) | ( RD( addr2 ) << 8 ); }
OP(5A) { AD2_XL; m_HL = ( m_HL & 0x00FF ) | ( RD( addr2 ) << 8 ); }
OP(5B) { AD2_YL; m_HL = ( m_HL & 0x00FF ) | ( RD( addr2 ) << 8 ); }
OP(5C) { AD1_X8; WR( addr1, ( m_HL >> 8 ) ); }
OP(5D) { AD1_Y8; WR( addr1, ( m_HL >> 8 ) ); }
OP(5E) { AD1_XL; WR( addr1, ( m_HL >> 8 ) ); }
OP(5F) { AD1_YL; WR( addr1, ( m_HL >> 8 ) ); }

OP(60) { AD1_IHL; AD2_X8; WR( addr1, RD( addr2 ) ); }
OP(61) { AD1_IHL; AD2_Y8; WR( addr1, RD( addr2 ) ); }
OP(62) { AD1_IHL; AD2_XL; WR( addr1, RD( addr2 ) ); }
OP(63) { AD1_IHL; AD2_YL; WR( addr1, RD( addr2 ) ); }
OP(64) { /* illegal operation? */ }
OP(65) { /* illegal operation? */ }
OP(66) { /* illegal operation? */ }
OP(67) { /* illegal operation? */ }
OP(68) { AD1_XIX; AD2_X8; WR( addr1, RD( addr2 ) ); }
OP(69) { AD1_XIX; AD2_Y8; WR( addr1, RD( addr2 ) ); }
OP(6A) { AD1_XIX; AD2_XL; WR( addr1, RD( addr2 ) ); }
OP(6B) { AD1_XIX; AD2_YL; WR( addr1, RD( addr2 ) ); }
OP(6C) { /* illegal operation? */ }
OP(6D) { /* illegal operation? */ }
OP(6E) { /* illegal operation? */ }
OP(6F) { /* illegal operation? */ }

OP(70) { /* illegal operation? */ }
OP(71) { /* illegal operation? */ }
OP(72) { /* illegal operation? */ }
OP(73) { /* illegal operation? */ }
OP(74) { /* illegal operation? */ }
OP(75) { /* illegal operation? */ }
OP(76) { /* illegal operation? */ }
OP(77) { /* illegal operation? */ }
OP(78) { AD1_YIY; AD2_X8; WR( addr1, RD( addr2 ) ); }
OP(79) { AD1_YIY; AD2_Y8; WR( addr1, RD( addr2 ) ); }
OP(7A) { AD1_YIY; AD2_XL; WR( addr1, RD( addr2 ) ); }
OP(7B) { AD1_YIY; AD2_YL; WR( addr1, RD( addr2 ) ); }
OP(7C) { /* illegal operation? */ }
OP(7D) { /* illegal operation? */ }
OP(7E) { /* illegal operation? */ }
OP(7F) { /* illegal operation? */ }

OP(80) { m_BA = ( m_BA & 0xFF00 ) | SAL8( m_BA & 0x00FF ); }
OP(81) { m_BA = ( m_BA & 0x00FF ) | ( SAL8( m_BA >> 8 )<< 8 ); }
OP(82) { AD1_IN8; WR( addr1, SAL8( RD( addr1 ) ) ); }
OP(83) { AD1_IHL; WR( addr1, SAL8( RD( addr1 ) ) ); }
OP(84) { m_BA = ( m_BA & 0xFF00 ) | SHL8( m_BA & 0x00FF ); }
OP(85) { m_BA = ( m_BA & 0x00FF ) | ( SHL8( m_BA >> 8 ) << 8 ); }
OP(86) { AD1_IN8; WR( addr1, SHL8( RD( addr1 ) ) ); }
OP(87) { AD1_IHL; WR( addr1, SHL8( RD( addr1 ) ) ); }
OP(88) { m_BA = ( m_BA & 0xFF00 ) | SAR8( m_BA & 0x00FF ); }
OP(89) { m_BA = ( m_BA & 0x00FF ) | ( SAR8( m_BA >> 8 ) << 8 ); }
OP(8A) { AD1_IN8; WR( addr1, SAR8( RD( addr1 ) ) ); }
OP(8B) { AD1_IHL; WR( addr1, SAR8( RD( addr1 ) ) ); }
OP(8C) { m_BA = ( m_BA & 0xFF00 ) | SHR8( m_BA & 0x00FF ); }
OP(8D) { m_BA = ( m_BA & 0x00FF ) | ( SHR8( m_BA >> 8 ) << 8 ); }
OP(8E) { AD1_IN8; WR( addr1, SHR8( RD( addr1 ) ) ); }
OP(8F) { AD1_IHL; WR( addr1, SHR8( RD( addr1 ) ) ); }

OP(90) { m_BA = ( m_BA & 0xFF00 ) | ROLC8( m_BA & 0x00FF ); }
OP(91) { m_BA = ( m_BA & 0x00FF ) | ( ROLC8( m_BA >> 8 ) << 8 ); }
OP(92) { AD1_IN8; WR( addr1, ROLC8( RD( addr1 ) ) ); }
OP(93) { AD1_IHL; WR( addr1, ROLC8( RD( addr1 ) ) ); }
OP(94) { m_BA = ( m_BA & 0xFF00 ) | ROL8( m_BA & 0x00FF ); }
OP(95) { m_BA = ( m_BA & 0x00FF ) | ( ROL8( m_BA >> 8 ) << 8 ); }
OP(96) { AD1_IN8; WR( addr1, ROL8( RD( addr1 ) ) ); }
OP(97) { AD1_IHL; WR( addr1, ROL8( RD( addr1 ) ) ); }
OP(98) { m_BA = ( m_BA & 0xFF00 ) | RORC8( m_BA & 0x00FF ); }
OP(99) { m_BA = ( m_BA & 0x00FF ) | ( RORC8( m_BA >> 8 ) << 8 ); }
OP(9A) { AD1_IN8; WR( addr1, RORC8( RD( addr1 ) ) ); }
OP(9B) { AD1_IHL; WR( addr1, RORC8( RD( addr1 ) ) ); }
OP(9C) { m_BA = ( m_BA & 0xFF00 ) | ROR8( m_BA & 0x00FF ); }
OP(9D) { m_BA = ( m_BA & 0x00FF ) | ( ROR8( m_BA >> 8 ) << 8 ); }
OP(9E) { AD1_IN8; WR( addr1, ROR8( RD( addr1 ) ) ); }
OP(9F) { AD1_IHL; WR( addr1, ROR8( RD( addr1 ) ) ); }

OP(A0) { m_BA = ( m_BA & 0xFF00 ) | NOT8( m_BA & 0x00FF ); }
OP(A1) { m_BA = ( m_BA & 0x00FF ) | ( NOT8( m_BA >> 8 ) << 8 ); }
OP(A2) { AD1_IN8; WR( addr1, NOT8( RD( addr1 ) ) ); }
OP(A3) { AD1_IHL; WR( addr1, NOT8( RD( addr1 ) ) ); }
OP(A4) { m_BA = ( m_BA & 0xFF00 ) | NEG8( m_BA & 0x00FF ); }
OP(A5) { m_BA = ( m_BA & 0x00FF ) | ( NEG8( m_BA >> 8 ) << 8 ); }
OP(A6) { AD1_IN8; WR( addr1, NEG8( RD( addr1 ) ) ); }
OP(A7) { AD1_IHL; WR( addr1, NEG8( RD( addr1 ) ) ); }
OP(A8) { m_BA = ( ( m_BA & 0x0080 ) ? ( 0xFF00 | m_BA ) : ( m_BA & 0x00FF ) ); }
OP(A9) { /* illegal operation? */ }
OP(AA) { /* illegal operation? */ }
OP(AB) { /* illegal operation? */ }
OP(AC) { /* illegal operation? */ }
OP(AD) { /* illegal operation? */ }
OP(AE) { /* HALT */ m_halted = 1; }
OP(AF) { }

OP(B0) { m_BA = ( m_BA & 0x00FF ) | ( AND8( ( m_BA >> 8 ), rdop() ) << 8 ); }
OP(B1) { m_HL = ( m_HL & 0xFF00 ) | AND8( ( m_HL & 0x00FF ), rdop() ); }
OP(B2) { m_HL = ( m_HL & 0x00FF ) | ( AND8( ( m_HL >> 8 ), rdop() ) << 8 ); }
OP(B3) { /* illegal operation? */ }
OP(B4) { m_BA = ( m_BA & 0x00FF ) | ( OR8( ( m_BA >> 8 ), rdop() ) << 8 ); }
OP(B5) { m_HL = ( m_HL & 0xFF00 ) | OR8( ( m_HL & 0x00FF ), rdop() ); }
OP(B6) { m_HL = ( m_HL & 0x00FF ) | ( OR8( ( m_HL >> 8 ), rdop() ) << 8 ); }
OP(B7) { /* illegal operation? */ }
OP(B8) { m_BA = ( m_BA & 0x00FF ) | ( XOR8( ( m_BA >> 8 ), rdop() ) << 8 ); }
OP(B9) { m_HL = ( m_HL & 0xFF00 ) | XOR8( ( m_HL & 0x00FF ), rdop() ); }
OP(BA) { m_HL = ( m_HL & 0x00FF ) | ( XOR8( ( m_HL >> 8 ), rdop() ) << 8 ); }
OP(BB) { /* illegal operation? */ }
OP(BC) { SUB8( ( m_BA >> 8 ), rdop() ); }
OP(BD) { SUB8( ( m_HL & 0x00FF), rdop() ); }
OP(BE) { SUB8( ( m_HL >> 8 ), rdop() ); }
OP(BF) { SUB8( m_N, rdop() ); }

OP(C0) { m_BA = ( m_BA & 0xFF00 ) | m_N; }
OP(C1) { m_BA = ( m_BA & 0xFF00 ) | m_F; }
OP(C2) { m_N = ( m_BA & 0x00FF ); }
OP(C3) { m_F = ( m_BA & 0x00FF ); }
OP(C4) { m_U = rdop(); }
OP(C5) { m_I = rdop(); }
OP(C6) { m_XI = rdop(); }
OP(C7) { m_YI = rdop(); }
OP(C8) { m_BA = ( m_BA & 0xFF00 ) | m_V; }
OP(C9) { m_BA = ( m_BA & 0xFF00 ) | m_I; }
OP(CA) { m_BA = ( m_BA & 0xFF00 ) | m_XI; }
OP(CB) { m_BA = ( m_BA & 0xFF00 ) | m_YI; }
OP(CC) { m_U = ( m_BA & 0x00FF ); }
OP(CD) { m_I = ( m_BA & 0x00FF ); }
OP(CE) { m_XI = ( m_BA & 0x00FF ); }
OP(CF) { m_YI = ( m_BA & 0x00FF ); }

OP(D0) { AD2_I16; m_BA = ( m_BA & 0xFF00 ) | RD( addr2 ); }
OP(D1) { AD2_I16; m_BA = ( m_BA & 0x00FF ) | ( RD( addr2 ) << 8 ); }
OP(D2) { AD2_I16; m_HL = ( m_HL & 0xFF00 ) | RD( addr2 ); }
OP(D3) { AD2_I16; m_HL = ( m_HL & 0x00FF ) | ( RD( addr2 ) << 8 ); }
OP(D4) { AD1_I16; WR( addr1, ( m_BA & 0x00FF ) ); }
OP(D5) { AD1_I16; WR( addr1, ( m_BA >> 8 ) ); }
OP(D6) { AD1_I16; WR( addr1, ( m_HL & 0x00FF ) ); }
OP(D7) { AD1_I16; WR( addr1, ( m_HL >> 8 ) ); }
OP(D8) { m_HL = ( m_HL & 0x00FF ) * ( m_BA & 0x00FF );  }
OP(D9) { int d = m_HL / ( m_BA & 0x00FF ); m_HL = ( ( m_HL - ( ( m_BA & 0x00FF ) * d ) ) << 8 ) | d; }
OP(DA) { /* illegal operation? */ }
OP(DB) { /* illegal operation? */ }
OP(DC) { /* illegal operation? */ }
OP(DD) { /* illegal operation? */ }
OP(DE) { /* illegal operation? */ }
OP(DF) { /* illegal operation? */ }

OP(E0) { INT8 d8 = rdop(); if ( ( ( m_F & ( FLAG_S | FLAG_O ) ) == FLAG_S ) || ( ( m_F & ( FLAG_S | FLAG_O ) ) == FLAG_O ) ) { JMP( m_PC + d8 - 1 ); } }
OP(E1) { INT8 d8 = rdop(); if ( ( m_F & FLAG_Z ) || ( ( m_F & ( FLAG_S | FLAG_O ) ) == FLAG_S ) || ( ( m_F & ( FLAG_S | FLAG_O ) ) == FLAG_O ) ) { JMP( m_PC + d8 - 1 ); } }
OP(E2) { INT8 d8 = rdop(); if ( !( m_F & FLAG_Z ) && ( ( ( m_F & ( FLAG_S | FLAG_O ) ) == 0 ) || ( ( m_F & ( FLAG_S | FLAG_O ) ) == ( FLAG_S | FLAG_O ) ) ) ) { JMP( m_PC + d8 - 1 ); } }
OP(E3) { INT8 d8 = rdop(); if ( ( ( m_F & ( FLAG_S | FLAG_O ) ) == 0 ) || ( ( m_F & ( FLAG_S | FLAG_O ) ) == ( FLAG_S | FLAG_O ) ) ) { JMP( m_PC + d8 - 1 ); } }
OP(E4) { INT8 d8 = rdop(); if ( ( m_F & FLAG_O ) ) { JMP( m_PC + d8 - 1 ); } }
OP(E5) { INT8 d8 = rdop(); if ( ! ( m_F & FLAG_O ) ) { JMP( m_PC + d8 - 1 ); } }
OP(E6) { INT8 d8 = rdop(); if ( ! ( m_F & FLAG_S ) ) { JMP( m_PC + d8 - 1 ); } }
OP(E7) { INT8 d8 = rdop(); if ( ( m_F & FLAG_S ) ) { JMP( m_PC + d8 - 1 ); } }
OP(E8) { INT8 d8 = rdop(); if ( ! ( m_E & EXEC_X0 ) ) { JMP( m_PC + d8 - 1 ); } }
OP(E9) { INT8 d8 = rdop(); if ( ! ( m_E & EXEC_X1 ) ) { JMP( m_PC + d8 - 1 ); } }
OP(EA) { INT8 d8 = rdop(); if ( ! ( m_E & EXEC_X2 ) ) { JMP( m_PC + d8 - 1 ); } }
OP(EB) { INT8 d8 = rdop(); if ( ! ( m_E & EXEC_DZ ) ) { JMP( m_PC + d8 - 1 ); } }
OP(EC) { INT8 d8 = rdop(); if ( ( m_E & EXEC_X0 ) ) { JMP( m_PC + d8 - 1 ); } }
OP(ED) { INT8 d8 = rdop(); if ( ( m_E & EXEC_X1 ) ) { JMP( m_PC + d8 - 1 ); } }
OP(EE) { INT8 d8 = rdop(); if ( ( m_E & EXEC_X2 ) ) { JMP( m_PC + d8 - 1 ); } }
OP(EF) { INT8 d8 = rdop(); if ( ( m_E & EXEC_DZ ) ) { JMP( m_PC + d8 - 1 ); } }

OP(F0) { INT8 d8 = rdop(); if ( ( ( m_F & ( FLAG_S | FLAG_O ) ) == FLAG_S ) || ( ( m_F & ( FLAG_S | FLAG_O ) ) == FLAG_O ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
OP(F1) { INT8 d8 = rdop(); if ( ( m_F & FLAG_Z ) || ( ( m_F & ( FLAG_S | FLAG_O ) ) == FLAG_S ) || ( ( m_F & ( FLAG_S | FLAG_O ) ) == FLAG_O ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
OP(F2) { INT8 d8 = rdop(); if ( !( m_F & FLAG_Z ) && ( ( ( m_F & ( FLAG_S | FLAG_O ) ) == 0 ) || ( ( m_F & ( FLAG_S | FLAG_O ) ) == ( FLAG_S | FLAG_O ) ) ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
OP(F3) { INT8 d8 = rdop(); if ( ( ( m_F & ( FLAG_S | FLAG_O ) ) == 0 ) || ( ( m_F & ( FLAG_S | FLAG_O ) ) == ( FLAG_S | FLAG_O ) ) ) { CALL( m_PC + d8 - 1 ); } }
OP(F4) { INT8 d8 = rdop(); if ( ( m_F & FLAG_O ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
OP(F5) { INT8 d8 = rdop(); if ( ! ( m_F & FLAG_O ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
OP(F6) { INT8 d8 = rdop(); if ( ! ( m_F & FLAG_S ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
OP(F7) { INT8 d8 = rdop(); if ( ( m_F & FLAG_S ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
OP(F8) { INT8 d8 = rdop(); if ( ! ( m_E & EXEC_X0 ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
OP(F9) { INT8 d8 = rdop(); if ( ! ( m_E & EXEC_X1 ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
OP(FA) { INT8 d8 = rdop(); if ( ! ( m_E & EXEC_X2 ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
OP(FB) { INT8 d8 = rdop(); if ( ! ( m_E & EXEC_DZ ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
OP(FC) { INT8 d8 = rdop(); if ( ( m_E & EXEC_X0 ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
OP(FD) { INT8 d8 = rdop(); if ( ( m_E & EXEC_X1 ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
OP(FE) { INT8 d8 = rdop(); if ( ( m_E & EXEC_X2 ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }
OP(FF) { INT8 d8 = rdop(); if ( ( m_E & EXEC_DZ ) ) { CALL( m_PC + d8 - 1 ); m_icount -= 12; } }

const minx_cpu_device::op_func minx_cpu_device::insnminx_CE[256] = {
	&minx_cpu_device::minx_CE_00, &minx_cpu_device::minx_CE_01, &minx_cpu_device::minx_CE_02, &minx_cpu_device::minx_CE_03, &minx_cpu_device::minx_CE_04, &minx_cpu_device::minx_CE_05, &minx_cpu_device::minx_CE_06, &minx_cpu_device::minx_CE_07,
	&minx_cpu_device::minx_CE_08, &minx_cpu_device::minx_CE_09, &minx_cpu_device::minx_CE_0A, &minx_cpu_device::minx_CE_0B, &minx_cpu_device::minx_CE_0C, &minx_cpu_device::minx_CE_0D, &minx_cpu_device::minx_CE_0E, &minx_cpu_device::minx_CE_0F,
	&minx_cpu_device::minx_CE_10, &minx_cpu_device::minx_CE_11, &minx_cpu_device::minx_CE_12, &minx_cpu_device::minx_CE_13, &minx_cpu_device::minx_CE_14, &minx_cpu_device::minx_CE_15, &minx_cpu_device::minx_CE_16, &minx_cpu_device::minx_CE_17,
	&minx_cpu_device::minx_CE_18, &minx_cpu_device::minx_CE_19, &minx_cpu_device::minx_CE_1A, &minx_cpu_device::minx_CE_1B, &minx_cpu_device::minx_CE_1C, &minx_cpu_device::minx_CE_1D, &minx_cpu_device::minx_CE_1E, &minx_cpu_device::minx_CE_1F,
	&minx_cpu_device::minx_CE_20, &minx_cpu_device::minx_CE_21, &minx_cpu_device::minx_CE_22, &minx_cpu_device::minx_CE_23, &minx_cpu_device::minx_CE_24, &minx_cpu_device::minx_CE_25, &minx_cpu_device::minx_CE_26, &minx_cpu_device::minx_CE_27,
	&minx_cpu_device::minx_CE_28, &minx_cpu_device::minx_CE_29, &minx_cpu_device::minx_CE_2A, &minx_cpu_device::minx_CE_2B, &minx_cpu_device::minx_CE_2C, &minx_cpu_device::minx_CE_2D, &minx_cpu_device::minx_CE_2E, &minx_cpu_device::minx_CE_2F,
	&minx_cpu_device::minx_CE_30, &minx_cpu_device::minx_CE_31, &minx_cpu_device::minx_CE_32, &minx_cpu_device::minx_CE_33, &minx_cpu_device::minx_CE_34, &minx_cpu_device::minx_CE_35, &minx_cpu_device::minx_CE_36, &minx_cpu_device::minx_CE_37,
	&minx_cpu_device::minx_CE_38, &minx_cpu_device::minx_CE_39, &minx_cpu_device::minx_CE_3A, &minx_cpu_device::minx_CE_3B, &minx_cpu_device::minx_CE_3C, &minx_cpu_device::minx_CE_3D, &minx_cpu_device::minx_CE_3E, &minx_cpu_device::minx_CE_3F,
	&minx_cpu_device::minx_CE_40, &minx_cpu_device::minx_CE_41, &minx_cpu_device::minx_CE_42, &minx_cpu_device::minx_CE_43, &minx_cpu_device::minx_CE_44, &minx_cpu_device::minx_CE_45, &minx_cpu_device::minx_CE_46, &minx_cpu_device::minx_CE_47,
	&minx_cpu_device::minx_CE_48, &minx_cpu_device::minx_CE_49, &minx_cpu_device::minx_CE_4A, &minx_cpu_device::minx_CE_4B, &minx_cpu_device::minx_CE_4C, &minx_cpu_device::minx_CE_4D, &minx_cpu_device::minx_CE_4E, &minx_cpu_device::minx_CE_4F,
	&minx_cpu_device::minx_CE_50, &minx_cpu_device::minx_CE_51, &minx_cpu_device::minx_CE_52, &minx_cpu_device::minx_CE_53, &minx_cpu_device::minx_CE_54, &minx_cpu_device::minx_CE_55, &minx_cpu_device::minx_CE_56, &minx_cpu_device::minx_CE_57,
	&minx_cpu_device::minx_CE_58, &minx_cpu_device::minx_CE_59, &minx_cpu_device::minx_CE_5A, &minx_cpu_device::minx_CE_5B, &minx_cpu_device::minx_CE_5C, &minx_cpu_device::minx_CE_5D, &minx_cpu_device::minx_CE_5E, &minx_cpu_device::minx_CE_5F,
	&minx_cpu_device::minx_CE_60, &minx_cpu_device::minx_CE_61, &minx_cpu_device::minx_CE_62, &minx_cpu_device::minx_CE_63, &minx_cpu_device::minx_CE_64, &minx_cpu_device::minx_CE_65, &minx_cpu_device::minx_CE_66, &minx_cpu_device::minx_CE_67,
	&minx_cpu_device::minx_CE_68, &minx_cpu_device::minx_CE_69, &minx_cpu_device::minx_CE_6A, &minx_cpu_device::minx_CE_6B, &minx_cpu_device::minx_CE_6C, &minx_cpu_device::minx_CE_6D, &minx_cpu_device::minx_CE_6E, &minx_cpu_device::minx_CE_6F,
	&minx_cpu_device::minx_CE_70, &minx_cpu_device::minx_CE_71, &minx_cpu_device::minx_CE_72, &minx_cpu_device::minx_CE_73, &minx_cpu_device::minx_CE_74, &minx_cpu_device::minx_CE_75, &minx_cpu_device::minx_CE_76, &minx_cpu_device::minx_CE_77,
	&minx_cpu_device::minx_CE_78, &minx_cpu_device::minx_CE_79, &minx_cpu_device::minx_CE_7A, &minx_cpu_device::minx_CE_7B, &minx_cpu_device::minx_CE_7C, &minx_cpu_device::minx_CE_7D, &minx_cpu_device::minx_CE_7E, &minx_cpu_device::minx_CE_7F,
	&minx_cpu_device::minx_CE_80, &minx_cpu_device::minx_CE_81, &minx_cpu_device::minx_CE_82, &minx_cpu_device::minx_CE_83, &minx_cpu_device::minx_CE_84, &minx_cpu_device::minx_CE_85, &minx_cpu_device::minx_CE_86, &minx_cpu_device::minx_CE_87,
	&minx_cpu_device::minx_CE_88, &minx_cpu_device::minx_CE_89, &minx_cpu_device::minx_CE_8A, &minx_cpu_device::minx_CE_8B, &minx_cpu_device::minx_CE_8C, &minx_cpu_device::minx_CE_8D, &minx_cpu_device::minx_CE_8E, &minx_cpu_device::minx_CE_8F,
	&minx_cpu_device::minx_CE_90, &minx_cpu_device::minx_CE_91, &minx_cpu_device::minx_CE_92, &minx_cpu_device::minx_CE_93, &minx_cpu_device::minx_CE_94, &minx_cpu_device::minx_CE_95, &minx_cpu_device::minx_CE_96, &minx_cpu_device::minx_CE_97,
	&minx_cpu_device::minx_CE_98, &minx_cpu_device::minx_CE_99, &minx_cpu_device::minx_CE_9A, &minx_cpu_device::minx_CE_9B, &minx_cpu_device::minx_CE_9C, &minx_cpu_device::minx_CE_9D, &minx_cpu_device::minx_CE_9E, &minx_cpu_device::minx_CE_9F,
	&minx_cpu_device::minx_CE_A0, &minx_cpu_device::minx_CE_A1, &minx_cpu_device::minx_CE_A2, &minx_cpu_device::minx_CE_A3, &minx_cpu_device::minx_CE_A4, &minx_cpu_device::minx_CE_A5, &minx_cpu_device::minx_CE_A6, &minx_cpu_device::minx_CE_A7,
	&minx_cpu_device::minx_CE_A8, &minx_cpu_device::minx_CE_A9, &minx_cpu_device::minx_CE_AA, &minx_cpu_device::minx_CE_AB, &minx_cpu_device::minx_CE_AC, &minx_cpu_device::minx_CE_AD, &minx_cpu_device::minx_CE_AE, &minx_cpu_device::minx_CE_AF,
	&minx_cpu_device::minx_CE_B0, &minx_cpu_device::minx_CE_B1, &minx_cpu_device::minx_CE_B2, &minx_cpu_device::minx_CE_B3, &minx_cpu_device::minx_CE_B4, &minx_cpu_device::minx_CE_B5, &minx_cpu_device::minx_CE_B6, &minx_cpu_device::minx_CE_B7,
	&minx_cpu_device::minx_CE_B8, &minx_cpu_device::minx_CE_B9, &minx_cpu_device::minx_CE_BA, &minx_cpu_device::minx_CE_BB, &minx_cpu_device::minx_CE_BC, &minx_cpu_device::minx_CE_BD, &minx_cpu_device::minx_CE_BE, &minx_cpu_device::minx_CE_BF,
	&minx_cpu_device::minx_CE_C0, &minx_cpu_device::minx_CE_C1, &minx_cpu_device::minx_CE_C2, &minx_cpu_device::minx_CE_C3, &minx_cpu_device::minx_CE_C4, &minx_cpu_device::minx_CE_C5, &minx_cpu_device::minx_CE_C6, &minx_cpu_device::minx_CE_C7,
	&minx_cpu_device::minx_CE_C8, &minx_cpu_device::minx_CE_C9, &minx_cpu_device::minx_CE_CA, &minx_cpu_device::minx_CE_CB, &minx_cpu_device::minx_CE_CC, &minx_cpu_device::minx_CE_CD, &minx_cpu_device::minx_CE_CE, &minx_cpu_device::minx_CE_CF,
	&minx_cpu_device::minx_CE_D0, &minx_cpu_device::minx_CE_D1, &minx_cpu_device::minx_CE_D2, &minx_cpu_device::minx_CE_D3, &minx_cpu_device::minx_CE_D4, &minx_cpu_device::minx_CE_D5, &minx_cpu_device::minx_CE_D6, &minx_cpu_device::minx_CE_D7,
	&minx_cpu_device::minx_CE_D8, &minx_cpu_device::minx_CE_D9, &minx_cpu_device::minx_CE_DA, &minx_cpu_device::minx_CE_DB, &minx_cpu_device::minx_CE_DC, &minx_cpu_device::minx_CE_DD, &minx_cpu_device::minx_CE_DE, &minx_cpu_device::minx_CE_DF,
	&minx_cpu_device::minx_CE_E0, &minx_cpu_device::minx_CE_E1, &minx_cpu_device::minx_CE_E2, &minx_cpu_device::minx_CE_E3, &minx_cpu_device::minx_CE_E4, &minx_cpu_device::minx_CE_E5, &minx_cpu_device::minx_CE_E6, &minx_cpu_device::minx_CE_E7,
	&minx_cpu_device::minx_CE_E8, &minx_cpu_device::minx_CE_E9, &minx_cpu_device::minx_CE_EA, &minx_cpu_device::minx_CE_EB, &minx_cpu_device::minx_CE_EC, &minx_cpu_device::minx_CE_ED, &minx_cpu_device::minx_CE_EE, &minx_cpu_device::minx_CE_EF,
	&minx_cpu_device::minx_CE_F0, &minx_cpu_device::minx_CE_F1, &minx_cpu_device::minx_CE_F2, &minx_cpu_device::minx_CE_F3, &minx_cpu_device::minx_CE_F4, &minx_cpu_device::minx_CE_F5, &minx_cpu_device::minx_CE_F6, &minx_cpu_device::minx_CE_F7,
	&minx_cpu_device::minx_CE_F8, &minx_cpu_device::minx_CE_F9, &minx_cpu_device::minx_CE_FA, &minx_cpu_device::minx_CE_FB, &minx_cpu_device::minx_CE_FC, &minx_cpu_device::minx_CE_FD, &minx_cpu_device::minx_CE_FE, &minx_cpu_device::minx_CE_FF
};

const int minx_cpu_device::insnminx_cycles_CE[256] = {
	16, 16, 16, 16, 16, 20, 20, 20, 16, 16, 16, 16, 16, 20, 20, 20,
	16, 16, 16, 16, 16, 20, 20, 20, 16, 16, 16, 16, 16, 20, 20, 20,
	16, 16, 16, 16, 16, 20, 20, 20, 16, 16, 16, 16, 16, 20, 20, 20,
	16, 16, 16, 16, 16, 20, 20, 20, 16, 16, 16, 16, 16, 20, 20, 20,

	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	20, 20, 20, 20,  1,  1,  1,  1, 20, 20, 20, 20,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,  1,  1, 20, 20, 20, 20,  1,  1,  1,  1,

	12, 12, 20, 16, 12, 12, 20, 16, 12, 12, 20, 16, 12, 12, 20, 16,
	12, 12, 20, 16, 12, 12, 20, 16, 12, 12, 20, 16, 12, 12, 20, 16,
	12, 12, 20, 16, 12, 12, 20, 16, 12,  1,  1,  1,  1,  1,  8,  8,
	12, 12, 12,  1, 12, 12, 12,  1, 20, 20, 20, 20, 12, 12, 12,  1,

	8,  8,  8, 12, 16, 12, 12, 12,  8,  8,  8,  8, 12,  8,  8,  8,
	20, 20, 20, 20, 20, 20, 20, 20, 48, 52,  1,  1,  1,  1,  1,  1,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12
};
