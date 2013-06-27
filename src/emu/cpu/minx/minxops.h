#undef OP
#define OP(nn) INLINE void minx_##nn(minx_state *minx)

OP(00) { minx->BA = ( minx->BA & 0xFF00 ) | ADD8( minx, ( minx->BA & 0x00FF ), ( minx->BA & 0xFF ) ); }
OP(01) { minx->BA = ( minx->BA & 0xFF00 ) | ADD8( minx, ( minx->BA & 0x00FF ), ( minx->BA >> 8 ) ); }
OP(02) { minx->BA = ( minx->BA & 0xFF00 ) | ADD8( minx, ( minx->BA & 0x00FF ), rdop(minx) ); }
OP(03) { AD2_IHL; minx->BA = ( minx->BA & 0xFF00 ) | ADD8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(04) { AD2_IN8; minx->BA = ( minx->BA & 0xFF00 ) | ADD8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(05) { AD2_I16; minx->BA = ( minx->BA & 0xFF00 ) | ADD8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(06) { AD2_XIX; minx->BA = ( minx->BA & 0xFF00 ) | ADD8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(07) { AD2_YIY; minx->BA = ( minx->BA & 0xFF00 ) | ADD8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(08) { minx->BA = ( minx->BA & 0xFF00 ) | ADDC8( minx, ( minx->BA & 0x00FF ), ( minx->BA & 0xFF ) ); }
OP(09) { minx->BA = ( minx->BA & 0xFF00 ) | ADDC8( minx, ( minx->BA & 0x00FF ), ( minx->BA >> 8 ) ); }
OP(0A) { minx->BA = ( minx->BA & 0xFF00 ) | ADDC8( minx, ( minx->BA & 0x00FF ), rdop(minx) ); }
OP(0B) { AD2_IHL; minx->BA = ( minx->BA & 0xFF00 ) | ADDC8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(0C) { AD2_IN8; minx->BA = ( minx->BA & 0xFF00 ) | ADDC8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(0D) { AD2_I16; minx->BA = ( minx->BA & 0xFF00 ) | ADDC8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(0E) { AD2_XIX; minx->BA = ( minx->BA & 0xFF00 ) | ADDC8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(0F) { AD2_YIY; minx->BA = ( minx->BA & 0xFF00 ) | ADDC8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }

OP(10) { minx->BA = ( minx->BA & 0xFF00 ) | SUB8( minx, ( minx->BA & 0x00FF ), ( minx->BA & 0xFF ) ); }
OP(11) { minx->BA = ( minx->BA & 0xFF00 ) | SUB8( minx, ( minx->BA & 0x00FF ), ( minx->BA >> 8 ) ); }
OP(12) { minx->BA = ( minx->BA & 0xFF00 ) | SUB8( minx, ( minx->BA & 0x00FF ), rdop(minx) ); }
OP(13) { AD2_IHL; minx->BA = ( minx->BA & 0xFF00 ) | SUB8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(14) { AD2_IN8; minx->BA = ( minx->BA & 0xFF00 ) | SUB8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(15) { AD2_I16; minx->BA = ( minx->BA & 0xFF00 ) | SUB8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(16) { AD2_XIX; minx->BA = ( minx->BA & 0xFF00 ) | SUB8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(17) { AD2_YIY; minx->BA = ( minx->BA & 0xFF00 ) | SUB8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(18) { minx->BA = ( minx->BA & 0xFF00 ) | SUBC8( minx, ( minx->BA & 0x00FF ), ( minx->BA & 0xFF ) ); }
OP(19) { minx->BA = ( minx->BA & 0xFF00 ) | SUBC8( minx, ( minx->BA & 0x00FF ), ( minx->BA >> 8 ) ); }
OP(1A) { minx->BA = ( minx->BA & 0xFF00 ) | SUBC8( minx, ( minx->BA & 0x00FF ), rdop(minx) ); }
OP(1B) { AD2_IHL; minx->BA = ( minx->BA & 0xFF00 ) | SUBC8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(1C) { AD2_IN8; minx->BA = ( minx->BA & 0xFF00 ) | SUBC8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(1D) { AD2_I16; minx->BA = ( minx->BA & 0xFF00 ) | SUBC8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(1E) { AD2_XIX; minx->BA = ( minx->BA & 0xFF00 ) | SUBC8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(1F) { AD2_YIY; minx->BA = ( minx->BA & 0xFF00 ) | SUBC8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }

OP(20) { minx->BA = ( minx->BA & 0xFF00 ) | AND8( minx, ( minx->BA & 0x00FF ), ( minx->BA & 0xFF ) ); }
OP(21) { minx->BA = ( minx->BA & 0xFF00 ) | AND8( minx, ( minx->BA & 0x00FF ), ( minx->BA >> 8 ) ); }
OP(22) { minx->BA = ( minx->BA & 0xFF00 ) | AND8( minx, ( minx->BA & 0x00FF ), rdop(minx) ); }
OP(23) { AD2_IHL; minx->BA = ( minx->BA & 0xFF00 ) | AND8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(24) { AD2_IN8; minx->BA = ( minx->BA & 0xFF00 ) | AND8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(25) { AD2_I16; minx->BA = ( minx->BA & 0xFF00 ) | AND8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(26) { AD2_XIX; minx->BA = ( minx->BA & 0xFF00 ) | AND8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(27) { AD2_YIY; minx->BA = ( minx->BA & 0xFF00 ) | AND8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(28) { minx->BA = ( minx->BA & 0xFF00 ) | OR8( minx, ( minx->BA & 0x00FF ), ( minx->BA & 0xFF ) ); }
OP(29) { minx->BA = ( minx->BA & 0xFF00 ) | OR8( minx, ( minx->BA & 0x00FF ), ( minx->BA >> 8 ) ); }
OP(2A) { minx->BA = ( minx->BA & 0xFF00 ) | OR8( minx, ( minx->BA & 0x00FF ), rdop(minx) ); }
OP(2B) { AD2_IHL; minx->BA = ( minx->BA & 0xFF00 ) | OR8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(2C) { AD2_IN8; minx->BA = ( minx->BA & 0xFF00 ) | OR8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(2D) { AD2_I16; minx->BA = ( minx->BA & 0xFF00 ) | OR8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(2E) { AD2_XIX; minx->BA = ( minx->BA & 0xFF00 ) | OR8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(2F) { AD2_YIY; minx->BA = ( minx->BA & 0xFF00 ) | OR8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }

OP(30) { SUB8( minx, ( minx->BA & 0x00FF ), ( minx->BA & 0xFF ) ); }
OP(31) { SUB8( minx, ( minx->BA & 0x00FF ), ( minx->BA >> 8 ) ); }
OP(32) { SUB8( minx, ( minx->BA & 0x00FF ), rdop(minx) ); }
OP(33) { AD2_IHL; SUB8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(34) { AD2_IN8; SUB8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(35) { AD2_I16; SUB8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(36) { AD2_XIX; SUB8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(37) { AD2_YIY; SUB8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(38) { minx->BA = ( minx->BA & 0xFF00 ) | XOR8( minx, ( minx->BA & 0x00FF ), ( minx->BA & 0xFF ) ); }
OP(39) { minx->BA = ( minx->BA & 0xFF00 ) | XOR8( minx, ( minx->BA & 0x00FF ), ( minx->BA >> 8 ) ); }
OP(3A) { minx->BA = ( minx->BA & 0xFF00 ) | XOR8( minx, ( minx->BA & 0x00FF ), rdop(minx) ); }
OP(3B) { AD2_IHL; minx->BA = ( minx->BA & 0xFF00 ) | XOR8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(3C) { AD2_IN8; minx->BA = ( minx->BA & 0xFF00 ) | XOR8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(3D) { AD2_I16; minx->BA = ( minx->BA & 0xFF00 ) | XOR8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(3E) { AD2_XIX; minx->BA = ( minx->BA & 0xFF00 ) | XOR8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }
OP(3F) { AD2_YIY; minx->BA = ( minx->BA & 0xFF00 ) | XOR8( minx, ( minx->BA & 0x00FF ), RD( addr2 ) ); }

OP(40) { minx->BA = ( minx->BA & 0xFF00 ) | ( minx->BA & 0x00FF); }
OP(41) { minx->BA = ( minx->BA & 0xFF00 ) | ( minx->BA >> 8 ); }
OP(42) { minx->BA = ( minx->BA & 0xFF00 ) | ( minx->HL & 0x00FF); }
OP(43) { minx->BA = ( minx->BA & 0xFF00 ) | ( minx->HL >> 8 ); }
OP(44) { AD2_IN8; minx->BA = ( minx->BA & 0xFF00 ) | RD( addr2 ); }
OP(45) { AD2_IHL; minx->BA = ( minx->BA & 0xFF00 ) | RD( addr2 ); }
OP(46) { AD2_XIX; minx->BA = ( minx->BA & 0xFF00 ) | RD( addr2 ); }
OP(47) { AD2_YIY; minx->BA = ( minx->BA & 0xFF00 ) | RD( addr2 ); }
OP(48) { minx->BA = ( minx->BA & 0x00FF ) | ( ( minx->BA & 0x00FF) << 8 ); }
OP(49) { minx->BA = ( minx->BA & 0x00FF ) | ( ( minx->BA >> 8 ) << 8 ); }
OP(4A) { minx->BA = ( minx->BA & 0x00FF ) | ( ( minx->HL & 0x00FF) << 8 ); }
OP(4B) { minx->BA = ( minx->BA & 0x00FF ) | ( ( minx->HL >> 8 ) << 8 ); }
OP(4C) { AD2_IN8; minx->BA = ( minx->BA & 0x00FF ) | ( RD( addr2 ) << 8 ); }
OP(4D) { AD2_IHL; minx->BA = ( minx->BA & 0x00FF ) | ( RD( addr2 ) << 8 ); }
OP(4E) { AD2_XIX; minx->BA = ( minx->BA & 0x00FF ) | ( RD( addr2 ) << 8 ); }
OP(4F) { AD2_YIY; minx->BA = ( minx->BA & 0x00FF ) | ( RD( addr2 ) << 8 ); }

OP(50) { minx->HL = ( minx->HL & 0xFF00 ) | ( minx->BA & 0x00FF); }
OP(51) { minx->HL = ( minx->HL & 0xFF00 ) | ( minx->BA >> 8 ); }
OP(52) { minx->HL = ( minx->HL & 0xFF00 ) | ( minx->HL & 0x00FF); }
OP(53) { minx->HL = ( minx->HL & 0xFF00 ) | ( minx->HL >> 8 ); }
OP(54) { AD2_IN8; minx->HL = ( minx->HL & 0xFF00 ) | RD( addr2 ); }
OP(55) { AD2_IHL; minx->HL = ( minx->HL & 0xFF00 ) | RD( addr2 ); }
OP(56) { AD2_XIX; minx->HL = ( minx->HL & 0xFF00 ) | RD( addr2 ); }
OP(57) { AD2_YIY; minx->HL = ( minx->HL & 0xFF00 ) | RD( addr2 ); }
OP(58) { minx->HL = ( minx->HL & 0x00FF ) | ( ( minx->BA & 0x00FF) << 8 ); }
OP(59) { minx->HL = ( minx->HL & 0x00FF ) | ( ( minx->BA >> 8 ) << 8 ); }
OP(5A) { minx->HL = ( minx->HL & 0x00FF ) | ( ( minx->HL & 0x00FF) << 8 ); }
OP(5B) { minx->HL = ( minx->HL & 0x00FF ) | ( ( minx->HL >> 8 ) << 8 ); }
OP(5C) { AD2_IN8; minx->HL = ( minx->HL & 0x00FF ) | ( RD( addr2 ) << 8 ); }
OP(5D) { AD2_IHL; minx->HL = ( minx->HL & 0x00FF ) | ( RD( addr2 ) << 8 ); }
OP(5E) { AD2_XIX; minx->HL = ( minx->HL & 0x00FF ) | ( RD( addr2 ) << 8 ); }
OP(5F) { AD2_YIY; minx->HL = ( minx->HL & 0x00FF ) | ( RD( addr2 ) << 8 ); }

OP(60) { AD1_XIX; WR( addr1, ( minx->BA & 0x00FF ) ); }
OP(61) { AD1_XIX; WR( addr1, ( minx->BA >> 8 ) ); }
OP(62) { AD1_XIX; WR( addr1, ( minx->HL & 0x00FF ) ); }
OP(63) { AD1_XIX; WR( addr1, ( minx->HL >> 8 ) ); }
OP(64) { AD1_XIX; AD2_IN8; WR( addr1, RD( addr2 ) ); }
OP(65) { AD1_XIX; AD2_IHL; WR( addr1, RD( addr2 ) ); }
OP(66) { AD1_XIX; AD2_XIX; WR( addr1, RD( addr2 ) ); }
OP(67) { AD1_XIX; AD2_YIY; WR( addr1, RD( addr2 ) ); }
OP(68) { AD1_IHL; WR( addr1, ( minx->BA & 0x00FF ) ); }
OP(69) { AD1_IHL; WR( addr1, ( minx->BA >> 8 ) ); }
OP(6A) { AD1_IHL; WR( addr1, ( minx->HL & 0x00FF ) ); }
OP(6B) { AD1_IHL; WR( addr1, ( minx->HL >> 8 ) ); }
OP(6C) { AD1_IHL; AD2_IN8; WR( addr1, RD( addr2 ) ); }
OP(6D) { AD1_IHL; AD2_IHL; WR( addr1, RD( addr2 ) ); }
OP(6E) { AD1_IHL; AD2_XIX; WR( addr1, RD( addr2 ) ); }
OP(6F) { AD1_IHL; AD2_YIY; WR( addr1, RD( addr2 ) ); }

OP(70) { AD1_YIY; WR( addr1, ( minx->BA & 0x00FF ) ); }
OP(71) { AD1_YIY; WR( addr1, ( minx->BA >> 8 ) ); }
OP(72) { AD1_YIY; WR( addr1, ( minx->HL & 0x00FF ) ); }
OP(73) { AD1_YIY; WR( addr1, ( minx->HL >> 8 ) ); }
OP(74) { AD1_YIY; AD2_IN8; WR( addr1, RD( addr2 ) ); }
OP(75) { AD1_YIY; AD2_IHL; WR( addr1, RD( addr2 ) ); }
OP(76) { AD1_YIY; AD2_XIX; WR( addr1, RD( addr2 ) ); }
OP(77) { AD1_YIY; AD2_YIY; WR( addr1, RD( addr2 ) ); }
OP(78) { AD1_IN8; WR( addr1, ( minx->BA & 0x00FF ) ); }
OP(79) { AD1_IN8; WR( addr1, ( minx->BA >> 8 ) ); }
OP(7A) { AD1_IN8; WR( addr1, ( minx->HL & 0x00FF ) ); }
OP(7B) { AD1_IN8; WR( addr1, ( minx->HL >> 8 ) ); }
OP(7C) { /* illegal operation? */ }
OP(7D) { AD1_IN8; AD2_IHL; WR( addr1, RD( addr2 ) ); }
OP(7E) { AD1_IN8; AD2_XIX; WR( addr1, RD( addr2 ) ); }
OP(7F) { AD1_IN8; AD2_YIY; WR( addr1, RD( addr2 ) ); }

OP(80) { minx->BA = ( minx->BA & 0xFF00 ) | INC8( minx, minx->BA & 0x00FF ); }
OP(81) { minx->BA = ( minx->BA & 0x00FF ) | ( INC8( minx, minx->BA >> 8 ) << 8 ); }
OP(82) { minx->HL = ( minx->HL & 0xFF00 ) | INC8( minx, minx->HL & 0x00FF ); }
OP(83) { minx->HL = ( minx->HL & 0x00FF ) | ( INC8( minx, minx->HL >> 8 ) << 8 ); }
OP(84) { minx->N = INC8( minx, minx->N ); }
OP(85) { AD1_IN8; WR( addr1, INC8( minx, RD( addr1 ) ) ); }
OP(86) { AD1_IHL; WR( addr1, INC8( minx, RD( addr1 ) ) ); }
OP(87) { minx->SP = INC16( minx, minx->SP ); }
OP(88) { minx->BA = ( minx->BA & 0xFF00 ) | DEC8( minx, minx->BA & 0x00FF ); }
OP(89) { minx->BA = ( minx->BA & 0x00FF ) | ( DEC8( minx, minx->BA >> 8 ) << 8 ); }
OP(8A) { minx->HL = ( minx->HL & 0xFF00 ) | DEC8( minx, minx->HL & 0x00FF ); }
OP(8B) { minx->HL = ( minx->HL & 0x00FF ) | ( DEC8( minx, minx->HL >> 8 ) << 8 ); }
OP(8C) { minx->N = DEC8( minx, minx->N ); }
OP(8D) { AD1_IN8; WR( addr1, DEC8( minx, RD( addr1 ) ) ); }
OP(8E) { AD1_IHL; WR( addr1, DEC8( minx, RD( addr1 ) ) ); }
OP(8F) { minx->SP = DEC8( minx, minx->SP ); }

OP(90) { minx->BA = INC16( minx, minx->BA ); }
OP(91) { minx->HL = INC16( minx, minx->HL ); }
OP(92) { minx->X = INC16( minx, minx->X ); }
OP(93) { minx->Y = INC16( minx, minx->Y ); }
OP(94) { minx->F = ( AND8( minx, ( minx->BA & 0x00FF ), ( minx->BA >> 8 ) ) ) ? minx->F & ~FLAG_Z : minx->F | FLAG_Z;}
OP(95) { AD1_IHL; minx->F = ( AND8( minx, RD( addr1 ), rdop(minx) ) ) ? minx->F & ~FLAG_Z : minx->F | FLAG_Z; }
OP(96) { minx->F = ( AND8( minx, ( minx->BA & 0x00FF ), rdop(minx) ) ) ? minx->F & ~FLAG_Z : minx->F | FLAG_Z; }
OP(97) { minx->F = ( AND8( minx, ( minx->BA >> 8 ), rdop(minx) ) ) ? minx->F & ~FLAG_Z : minx->F | FLAG_Z; }
OP(98) { minx->BA = DEC16( minx, minx->BA ); }
OP(99) { minx->HL = DEC16( minx, minx->HL ); }
OP(9A) { minx->X = DEC16( minx, minx->X ); }
OP(9B) { minx->Y = DEC16( minx, minx->Y ); }
OP(9C) { minx->F = minx->F & rdop(minx); }
OP(9D) { minx->F = minx->F | rdop(minx); }
OP(9E) { minx->F = minx->F ^ rdop(minx); }
OP(9F) { minx->F = rdop(minx); }

OP(A0) { PUSH16( minx, minx->BA ); }
OP(A1) { PUSH16( minx, minx->HL ); }
OP(A2) { PUSH16( minx, minx->X ); }
OP(A3) { PUSH16( minx, minx->Y ); }
OP(A4) { PUSH8( minx, minx->N ); }
OP(A5) { PUSH8( minx, minx->I ); }
OP(A6) { PUSH8( minx, minx->XI ); PUSH8( minx, minx->YI ); }
OP(A7) { PUSH8( minx, minx->F ); }
OP(A8) { minx->BA = POP16(minx); }
OP(A9) { minx->HL = POP16(minx);}
OP(AA) { minx->X = POP16(minx); }
OP(AB) { minx->Y = POP16(minx); }
OP(AC) { minx->N = POP8(minx); }
OP(AD) { minx->I = POP8(minx); }
OP(AE) { minx->YI = POP8(minx); minx->XI = POP8(minx); }
OP(AF) { minx->F = POP8(minx); }

OP(B0) { UINT8 op = rdop(minx); minx->BA = ( minx->BA & 0xFF00 ) | op; }
OP(B1) { UINT8 op = rdop(minx); minx->BA = ( minx->BA & 0x00FF ) | ( op << 8 ); }
OP(B2) { UINT8 op = rdop(minx); minx->HL = ( minx->HL & 0xFF00 ) | op; }
OP(B3) { UINT8 op = rdop(minx); minx->HL = ( minx->HL & 0x00FF ) | ( op << 8 ); }
OP(B4) { UINT8 op = rdop(minx); minx->N = op; }
OP(B5) { AD1_IHL; UINT8 op = rdop(minx); WR( addr1, op); }
OP(B6) { AD1_XIX; UINT8 op = rdop(minx); WR( addr1, op ); }
OP(B7) { AD1_YIY; UINT8 op = rdop(minx); WR( addr1, op ); }
OP(B8) { AD2_I16; minx->BA = rd16( minx, addr2 ); }
OP(B9) { AD2_I16; minx->HL = rd16( minx, addr2 ); }
OP(BA) { AD2_I16; minx->X = rd16( minx, addr2 ); }
OP(BB) { AD2_I16; minx->Y = rd16( minx, addr2 ); }
OP(BC) { AD1_I16; wr16( minx, addr1, minx->BA ); }
OP(BD) { AD1_I16; wr16( minx, addr1, minx->HL ); }
OP(BE) { AD1_I16; wr16( minx, addr1, minx->X ); }
OP(BF) { AD1_I16; wr16( minx, addr1, minx->Y ); }

OP(C0) { minx->BA = ADD16( minx, minx->BA, rdop16(minx) ); }
OP(C1) { minx->HL = ADD16( minx, minx->HL, rdop16(minx) ); }
OP(C2) { minx->X = ADD16( minx, minx->X, rdop16(minx) ); }
OP(C3) { minx->Y = ADD16( minx, minx->Y, rdop16(minx) ); }
OP(C4) { minx->BA = rdop16(minx); }
OP(C5) { minx->HL = rdop16(minx); }
OP(C6) { minx->X = rdop16(minx); }
OP(C7) { minx->Y = rdop16(minx); }
OP(C8) { UINT16 t = minx->BA; minx->BA = minx->HL; minx->HL = t; }
OP(C9) { UINT16 t = minx->BA; minx->BA = minx->X; minx->X = t; }
OP(CA) { UINT16 t = minx->BA; minx->BA = minx->Y; minx->Y = t; }
OP(CB) { UINT16 t = minx->BA; minx->BA = minx->SP; minx->SP = t; }
OP(CC) { minx->BA = ( minx->BA >> 8 ) | ( ( minx->BA & 0x00FF ) << 8 ); }
OP(CD) { UINT8 t; AD2_IHL; t = RD( addr2 ); WR( addr2, ( minx->BA & 0x00FF ) ); minx->BA = ( minx->BA & 0xFF00 ) | t; }
OP(CE) { UINT8 op = rdop(minx); insnminx_CE[op](minx); minx->icount -= insnminx_cycles_CE[op]; }
OP(CF) { UINT8 op = rdop(minx); insnminx_CF[op](minx); minx->icount -= insnminx_cycles_CF[op]; }

OP(D0) { minx->BA = SUB16( minx, minx->BA, rdop16(minx) ); }
OP(D1) { minx->HL = SUB16( minx, minx->HL, rdop16(minx) ); }
OP(D2) { minx->X = SUB16( minx, minx->X, rdop16(minx) ); }
OP(D3) { minx->Y = SUB16( minx, minx->Y, rdop16(minx) ); }
OP(D4) { SUB16( minx, minx->BA, rdop16(minx) ); }
OP(D5) { SUB16( minx, minx->HL, rdop16(minx) ); }
OP(D6) { SUB16( minx, minx->X, rdop16(minx) ); }
OP(D7) { SUB16( minx, minx->Y, rdop16(minx) ); }
OP(D8) { AD1_IN8; WR( addr1, AND8( minx, RD( addr1 ), rdop(minx) ) ); }
OP(D9) { AD1_IN8; WR( addr1, OR8( minx, RD( addr1 ), rdop(minx) ) ); }
OP(DA) { AD1_IN8; WR( addr1, XOR8( minx, RD( addr1 ), rdop(minx) ) ); }
OP(DB) { AD1_IN8; SUB8( minx, RD( addr1 ), rdop(minx) ); }
OP(DC) { AD1_IN8; minx->F = ( AND8( minx, RD( addr1 ), rdop(minx) ) ) ? minx->F & ~FLAG_Z : minx->F | FLAG_Z; }
OP(DD) { AD1_IN8; WR( addr1, rdop(minx) ); }
OP(DE) { minx->BA = ( minx->BA & 0xFF00 ) | ( ( minx->BA & 0x000F ) | ( ( minx->BA & 0x0F00 ) >> 4 ) ); }
OP(DF) { minx->BA = ( ( minx->BA & 0x0080 ) ? 0xFF00 : 0x0000 ) | ( minx->BA & 0x000F ); }

OP(E0) { INT8 d8 = rdop(minx); if ( minx->F & FLAG_C ) { CALL( minx, minx->PC + d8 - 1 ); minx->icount -= 12; } }
OP(E1) { INT8 d8 = rdop(minx); if ( ! ( minx->F & FLAG_C ) ) { CALL( minx, minx->PC + d8- 1  ); minx->icount -= 12; } }
OP(E2) { INT8 d8 = rdop(minx); if ( minx->F & FLAG_Z ) { CALL( minx, minx->PC + d8 - 1 ); minx->icount -= 12; } }
OP(E3) { INT8 d8 = rdop(minx); if ( ! ( minx->F & FLAG_Z ) ) { CALL( minx, minx->PC + d8 - 1 ); minx->icount -= 12; } }
OP(E4) { INT8 d8 = rdop(minx); if ( minx->F & FLAG_C ) { JMP( minx, minx->PC + d8 - 1 ); } }
OP(E5) { INT8 d8 = rdop(minx); if ( ! ( minx->F & FLAG_C ) ) { JMP( minx, minx->PC + d8 - 1 ); } }
OP(E6) { INT8 d8 = rdop(minx); if ( minx->F & FLAG_Z ) { JMP( minx, minx->PC + d8 - 1 ); } }
OP(E7) { INT8 d8 = rdop(minx); if ( ! ( minx->F & FLAG_Z ) ) { JMP( minx, minx->PC + d8 - 1 ); } }
OP(E8) { UINT16 d16 = rdop16(minx); if ( minx->F & FLAG_C ) { CALL( minx, minx->PC + d16 - 1 ); minx->icount -= 12; } }
OP(E9) { UINT16 d16 = rdop16(minx); if ( ! ( minx->F & FLAG_C ) ) { CALL( minx, minx->PC + d16 - 1 ); minx->icount -= 12; } }
OP(EA) { UINT16 d16 = rdop16(minx); if ( minx->F & FLAG_Z ) { CALL( minx, minx->PC + d16 - 1 ); minx->icount -= 12; } }
OP(EB) { UINT16 d16 = rdop16(minx); if ( ! ( minx->F & FLAG_Z ) ) { CALL( minx, minx->PC + d16 - 1 ); minx->icount -= 12; } }
OP(EC) { UINT16 d16 = rdop16(minx); if ( minx->F & FLAG_C ) { JMP( minx, minx->PC + d16 - 1 ); } }
OP(ED) { UINT16 d16 = rdop16(minx); if ( ! ( minx->F & FLAG_C ) ) { JMP( minx, minx->PC + d16 - 1 ); } }
OP(EE) { UINT16 d16 = rdop16(minx); if ( minx->F & FLAG_Z ) { JMP( minx, minx->PC + d16 - 1 ); } }
OP(EF) { UINT16 d16 = rdop16(minx); if ( ! ( minx->F & FLAG_Z ) ) { JMP( minx, minx->PC + d16 - 1 ); } }

OP(F0) { INT8 d8 = rdop(minx); CALL( minx, minx->PC + d8 - 1 ); }
OP(F1) { INT8 d8 = rdop(minx); JMP( minx, minx->PC + d8 - 1 ); }
OP(F2) { UINT16 d16 = rdop16(minx); CALL( minx, minx->PC + d16 - 1 ); }
OP(F3) { UINT16 d16 = rdop16(minx); JMP( minx, minx->PC + d16 - 1 ); }
OP(F4) { JMP( minx, minx->HL ); }
OP(F5) { INT8 d8 = rdop(minx); minx->BA = minx->BA - 0x0100; if ( minx->BA & 0xFF00 ) { JMP( minx, minx->PC + d8 - 1 ); } }
OP(F6) { minx->BA = ( minx->BA & 0xFF00 ) | ( ( minx->BA & 0x00F0 ) >> 4 ) | ( ( minx->BA & 0x000F ) << 4 ); }
OP(F7) { UINT8 d; AD1_IHL; d = RD( addr1 ); WR( addr1, ( ( d & 0xF0 ) >> 4 ) | ( ( d & 0x0F ) << 4 ) ); }
OP(F8) { minx->PC = POP16(minx); minx->V = POP8(minx); minx->U = minx->V; }
OP(F9) { minx->F = POP8(minx); minx->PC = POP16(minx); minx->V = POP8(minx); minx->U = minx->V; }
OP(FA) { minx->PC = POP16(minx) + 2; minx->V = POP8(minx); minx->U = minx->V; }
OP(FB) { AD1_I16; CALL( minx, rd16( minx, addr1 ) ); }
OP(FC) { UINT8 i = rdop(minx) & 0xFE; CALL( minx, rd16( minx, i ) ); PUSH8( minx, minx->F ); }
OP(FD) { UINT8 i = rdop(minx) & 0xFE; JMP( minx, rd16( minx, i ) ); /* PUSH8( minx, minx->F );?? */ }
OP(FE) { /* illegal operation? */ }
OP(FF) { }

static void (*const insnminx[256])(minx_state *minx) = {
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

static const int insnminx_cycles[256] = {
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
