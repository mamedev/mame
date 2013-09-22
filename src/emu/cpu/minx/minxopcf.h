#undef OP
#define OP(nn) void minx_cpu_device::minx_CF_##nn()

OP(00) { m_BA = ADD16( m_BA, m_BA ); }
OP(01) { m_BA = ADD16( m_BA, m_HL ); }
OP(02) { m_BA = ADD16( m_BA, m_X ); }
OP(03) { m_BA = ADD16( m_BA, m_Y ); }
OP(04) { m_BA = ADDC16( m_BA, m_BA ); }
OP(05) { m_BA = ADDC16( m_BA, m_HL ); }
OP(06) { m_BA = ADDC16( m_BA, m_X ); }
OP(07) { m_BA = ADDC16( m_BA, m_Y ); }
OP(08) { m_BA = SUB16( m_BA, m_BA ); }
OP(09) { m_BA = SUB16( m_BA, m_HL ); }
OP(0A) { m_BA = SUB16( m_BA, m_X ); }
OP(0B) { m_BA = SUB16( m_BA, m_Y ); }
OP(0C) { m_BA = SUBC16( m_BA, m_BA ); }
OP(0D) { m_BA = SUBC16( m_BA, m_HL ); }
OP(0E) { m_BA = SUBC16( m_BA, m_X ); }
OP(0F) { m_BA = SUBC16( m_BA, m_Y ); }

OP(10) { /* illegal instruction? */ }
OP(11) { /* illegal instruction? */ }
OP(12) { /* illegal instruction? */ }
OP(13) { /* illegal instruction? */ }
OP(14) { /* illegal instruction? */ }
OP(15) { /* illegal instruction? */ }
OP(16) { /* illegal instruction? */ }
OP(17) { /* illegal instruction? */ }
OP(18) { SUB16( m_BA, m_BA ); }
OP(19) { SUB16( m_BA, m_HL ); }
OP(1A) { SUB16( m_BA, m_X ); }
OP(1B) { SUB16( m_BA, m_Y ); }
OP(1C) { /* illegal instruction? */ }
OP(1D) { /* illegal instruction? */ }
OP(1E) { /* illegal instruction? */ }
OP(1F) { /* illegal instruction? */ }

OP(20) { m_HL = ADD16( m_HL, m_BA ); }
OP(21) { m_HL = ADD16( m_HL, m_HL ); }
OP(22) { m_HL = ADD16( m_HL, m_X ); }
OP(23) { m_HL = ADD16( m_HL, m_Y ); }
OP(24) { m_HL = ADDC16( m_HL, m_BA ); }
OP(25) { m_HL = ADDC16( m_HL, m_HL ); }
OP(26) { m_HL = ADDC16( m_HL, m_X ); }
OP(27) { m_HL = ADDC16( m_HL, m_Y ); }
OP(28) { m_HL = SUB16( m_HL, m_BA ); }
OP(29) { m_HL = SUB16( m_HL, m_HL ); }
OP(2A) { m_HL = SUB16( m_HL, m_X ); }
OP(2B) { m_HL = SUB16( m_HL, m_Y ); }
OP(2C) { m_HL = SUBC16( m_HL, m_BA ); }
OP(2D) { m_HL = SUBC16( m_HL, m_HL ); }
OP(2E) { m_HL = SUBC16( m_HL, m_X ); }
OP(2F) { m_HL = SUBC16( m_HL, m_Y ); }

OP(30) { /* illegal instruction? */ }
OP(31) { /* illegal instruction? */ }
OP(32) { /* illegal instruction? */ }
OP(33) { /* illegal instruction? */ }
OP(34) { /* illegal instruction? */ }
OP(35) { /* illegal instruction? */ }
OP(36) { /* illegal instruction? */ }
OP(37) { /* illegal instruction? */ }
OP(38) { SUB16( m_HL, m_BA ); }
OP(39) { SUB16( m_HL, m_HL ); }
OP(3A) { SUB16( m_HL, m_X ); }
OP(3B) { SUB16( m_HL, m_Y ); }
OP(3C) { /* illegal instruction? */ }
OP(3D) { /* illegal instruction? */ }
OP(3E) { /* illegal instruction? */ }
OP(3F) { /* illegal instruction? */ }

OP(40) { m_X = ADD16( m_X, m_BA ); }
OP(41) { m_X = ADD16( m_X, m_HL ); }
OP(42) { m_Y = ADD16( m_Y, m_BA ); }
OP(43) { m_Y = ADD16( m_Y, m_HL ); }
OP(44) { m_SP = ADD16( m_SP, m_BA ); }
OP(45) { m_SP = ADD16( m_SP, m_HL ); }
OP(46) { /* illegal instruction? */ }
OP(47) { /* illegal instruction? */ }
OP(48) { m_X = SUB16( m_X, m_BA ); }
OP(49) { m_X = SUB16( m_X, m_HL ); }
OP(4A) { m_Y = SUB16( m_Y, m_BA ); }
OP(4B) { m_Y = SUB16( m_Y, m_HL ); }
OP(4C) { m_SP = SUB16( m_SP, m_BA ); }
OP(4D) { m_SP = SUB16( m_SP, m_HL ); }
OP(4E) { /* illegal instruction? */ }
OP(4F) { /* illegal instruction? */ }

OP(50) { /* illegal instruction? */ }
OP(51) { /* illegal instruction? */ }
OP(52) { /* illegal instruction? */ }
OP(53) { /* illegal instruction? */ }
OP(54) { /* illegal instruction? */ }
OP(55) { /* illegal instruction? */ }
OP(56) { /* illegal instruction? */ }
OP(57) { /* illegal instruction? */ }
OP(58) { /* illegal instruction? */ }
OP(59) { /* illegal instruction? */ }
OP(5A) { /* illegal instruction? */ }
OP(5B) { /* illegal instruction? */ }
OP(5C) { SUB16( m_SP, m_BA ); }
OP(5D) { SUB16( m_SP, m_HL ); }
OP(5E) { /* illegal instruction? */ }
OP(5F) { /* illegal instruction? */ }

OP(60) { ADDC16( m_BA, rdop16() ); /* ??? */ }
OP(61) { ADDC16( m_HL, rdop16() ); /* ??? */ }
OP(62) { ADDC16( m_X, rdop16() ); /* ??? */ }
OP(63) { ADDC16( m_Y, rdop16() ); /* ??? */ }
OP(64) { /* illegal instruction? */ }
OP(65) { /* illegal instruction? */ }
OP(66) { /* illegal instruction? */ }
OP(67) { /* illegal instruction? */ }
OP(68) { m_SP = ADD16( m_SP, rdop16() ); }
OP(69) { /* illegal instruction? */ }
OP(6A) { m_SP = SUB16( m_SP, rdop16() ); }
OP(6B) { /* illegal instruction? */ }
OP(6C) { SUB16( m_SP, rdop16() ); }
OP(6D) { /* illegal instruction? */ }
OP(6E) { m_SP = rdop16(); }
OP(6F) { /* illegal instruction? */ }

OP(70) { UINT8 ofs8 = rdop(); m_BA = rd16( m_SP + ofs8 ); }
OP(71) { UINT8 ofs8 = rdop(); m_HL = rd16( m_SP + ofs8 ); }
OP(72) { UINT8 ofs8 = rdop(); m_X = rd16( m_SP + ofs8 ); }
OP(73) { UINT8 ofs8 = rdop(); m_Y = rd16( m_SP + ofs8 ); }
OP(74) { UINT8 ofs8 = rdop(); wr16( m_SP + ofs8, m_BA ); }
OP(75) { UINT8 ofs8 = rdop(); wr16( m_SP + ofs8, m_HL ); }
OP(76) { UINT8 ofs8 = rdop(); wr16( m_SP + ofs8, m_X ); }
OP(77) { UINT8 ofs8 = rdop(); wr16( m_SP + ofs8, m_Y ); }
OP(78) { AD2_I16; m_SP = rd16( addr2 ); }
OP(79) { /* illegal instruction? */ }
OP(7A) { /* illegal instruction? */ }
OP(7B) { /* illegal instruction? */ }
OP(7C) { AD1_I16; wr16( addr1, m_SP ); }
OP(7D) { /* illegal instruction? */ }
OP(7E) { /* illegal instruction? */ }
OP(7F) { /* illegal instruction? */ }

OP(80) { /* illegal instruction? */ }
OP(81) { /* illegal instruction? */ }
OP(82) { /* illegal instruction? */ }
OP(83) { /* illegal instruction? */ }
OP(84) { /* illegal instruction? */ }
OP(85) { /* illegal instruction? */ }
OP(86) { /* illegal instruction? */ }
OP(87) { /* illegal instruction? */ }
OP(88) { /* illegal instruction? */ }
OP(89) { /* illegal instruction? */ }
OP(8A) { /* illegal instruction? */ }
OP(8B) { /* illegal instruction? */ }
OP(8C) { /* illegal instruction? */ }
OP(8D) { /* illegal instruction? */ }
OP(8E) { /* illegal instruction? */ }
OP(8F) { /* illegal instruction? */ }

OP(90) { /* illegal instruction? */ }
OP(91) { /* illegal instruction? */ }
OP(92) { /* illegal instruction? */ }
OP(93) { /* illegal instruction? */ }
OP(94) { /* illegal instruction? */ }
OP(95) { /* illegal instruction? */ }
OP(96) { /* illegal instruction? */ }
OP(97) { /* illegal instruction? */ }
OP(98) { /* illegal instruction? */ }
OP(99) { /* illegal instruction? */ }
OP(9A) { /* illegal instruction? */ }
OP(9B) { /* illegal instruction? */ }
OP(9C) { /* illegal instruction? */ }
OP(9D) { /* illegal instruction? */ }
OP(9E) { /* illegal instruction? */ }
OP(9F) { /* illegal instruction? */ }

OP(A0) { /* illegal instruction? */ }
OP(A1) { /* illegal instruction? */ }
OP(A2) { /* illegal instruction? */ }
OP(A3) { /* illegal instruction? */ }
OP(A4) { /* illegal instruction? */ }
OP(A5) { /* illegal instruction? */ }
OP(A6) { /* illegal instruction? */ }
OP(A7) { /* illegal instruction? */ }
OP(A8) { /* illegal instruction? */ }
OP(A9) { /* illegal instruction? */ }
OP(AA) { /* illegal instruction? */ }
OP(AB) { /* illegal instruction? */ }
OP(AC) { /* illegal instruction? */ }
OP(AD) { /* illegal instruction? */ }
OP(AE) { /* illegal instruction? */ }
OP(AF) { /* illegal instruction? */ }

OP(B0) { PUSH8( m_BA & 0x00FF ); }
OP(B1) { PUSH8( m_BA >> 8 ); }
OP(B2) { PUSH8( m_HL & 0x00FF ); }
OP(B3) { PUSH8( m_HL >> 8 ); }
OP(B4) { m_BA = ( m_BA & 0xFF00 ) | POP8(); }
OP(B5) { m_BA = ( m_BA & 0x00FF ) | ( POP8() << 8 ); }
OP(B6) { m_HL = ( m_HL & 0xFF00 ) | POP8(); }
OP(B7) { m_HL = ( m_HL & 0x00FF ) | ( POP8() << 8 ); }
OP(B8) { PUSH16( m_BA ); PUSH16( m_HL ); PUSH16( m_X ); PUSH16( m_Y ); PUSH8( m_N ); }
OP(B9) { PUSH16( m_BA ); PUSH16( m_HL ); PUSH16( m_X ); PUSH16( m_Y ); PUSH8( m_N ); PUSH8( m_I ); PUSH8( m_XI ); PUSH8( m_YI ); }
OP(BA) { /* illegal instruction? */ }
OP(BB) { /* illegal instruction? */ }
OP(BC) { m_N = POP8(); m_Y = POP16(); m_X = POP16(); m_HL = POP16(); m_BA = POP16(); }
OP(BD) { m_YI = POP8(); m_XI = POP8(); m_I = POP8(); m_N = POP8(); m_Y = POP16(); m_X = POP16(); m_HL = POP16(); m_BA = POP16(); }
OP(BE) { /* illegal instruction? */ }
OP(BF) { /* illegal instruction? */ }

OP(C0) { AD2_IHL; m_BA = rd16( addr2 ); }
OP(C1) { AD2_IHL; m_HL = rd16( addr2 ); }
OP(C2) { AD2_IHL; m_X = rd16( addr2 ); }
OP(C3) { AD2_IHL; m_Y = rd16( addr2 ); }
OP(C4) { AD1_IHL; wr16( addr1, m_BA ); }
OP(C5) { AD1_IHL; wr16( addr1, m_HL ); }
OP(C6) { AD1_IHL; wr16( addr1, m_X ); }
OP(C7) { AD1_IHL; wr16( addr1, m_Y ); }
OP(C8) { /* illegal instruction? */ }
OP(C9) { /* illegal instruction? */ }
OP(CA) { /* illegal instruction? */ }
OP(CB) { /* illegal instruction? */ }
OP(CC) { /* illegal instruction? */ }
OP(CD) { /* illegal instruction? */ }
OP(CE) { /* illegal instruction? */ }
OP(CF) { /* illegal instruction? */ }

OP(D0) { AD2_XIX; m_BA = rd16( addr2 ); }
OP(D1) { AD2_XIX; m_HL = rd16( addr2 ); }
OP(D2) { AD2_XIX; m_X = rd16( addr2 ); }
OP(D3) { AD2_XIX; m_Y = rd16( addr2 ); }
OP(D4) { AD1_XIX; wr16( addr1, m_BA ); }
OP(D5) { AD1_XIX; wr16( addr1, m_HL ); }
OP(D6) { AD1_XIX; wr16( addr1, m_X ); }
OP(D7) { AD1_XIX; wr16( addr1, m_Y ); }
OP(D8) { AD2_YIY; m_BA = rd16( addr2 ); }
OP(D9) { AD2_YIY; m_HL = rd16( addr2 ); }
OP(DA) { AD2_YIY; m_X = rd16( addr2 ); }
OP(DB) { AD2_YIY; m_Y = rd16( addr2 ); }
OP(DC) { AD1_YIY; wr16( addr1, m_BA ); }
OP(DD) { AD1_YIY; wr16( addr1, m_HL ); }
OP(DE) { AD1_YIY; wr16( addr1, m_X ); }
OP(DF) { AD1_YIY; wr16( addr1, m_Y ); }

OP(E0) { } //{ m_BA = m_BA; }
OP(E1) { m_BA = m_HL; }
OP(E2) { m_BA = m_X; }
OP(E3) { m_BA = m_Y; }
OP(E4) { m_HL = m_BA; }
OP(E5) { } //{ m_HL = m_HL; }
OP(E6) { m_HL = m_X; }
OP(E7) { m_HL = m_Y; }
OP(E8) { m_X = m_BA; }
OP(E9) { m_X = m_HL; }
OP(EA) { } //{ m_X = m_X; }
OP(EB) { m_X = m_Y; }
OP(EC) { m_Y = m_BA; }
OP(ED) { m_Y = m_HL; }
OP(EE) { m_Y = m_X; }
OP(EF) { } //{ m_Y = m_Y; }

OP(F0) { m_SP = m_BA; }
OP(F1) { m_SP = m_HL; }
OP(F2) { m_SP = m_X; }
OP(F3) { m_SP = m_Y; }
OP(F4) { m_HL = m_SP; }
OP(F5) { m_HL = m_PC; }
OP(F6) { /* illegal instruction? */ }
OP(F7) { /* illegal instruction? */ }
OP(F8) { m_BA = m_SP; }
OP(F9) { m_BA = m_PC; }
OP(FA) { m_X = m_SP; }
OP(FB) { /* illegal instruction? */ }
OP(FC) { /* illegal instruction? */ }
OP(FD) { /* illegal instruction? */ }
OP(FE) { m_Y = m_SP; }
OP(FF) { /* illegal instruction? */ }

const minx_cpu_device::op_func minx_cpu_device::insnminx_CF[256] = {
	&minx_cpu_device::minx_CF_00, &minx_cpu_device::minx_CF_01, &minx_cpu_device::minx_CF_02, &minx_cpu_device::minx_CF_03, &minx_cpu_device::minx_CF_04, &minx_cpu_device::minx_CF_05, &minx_cpu_device::minx_CF_06, &minx_cpu_device::minx_CF_07,
	&minx_cpu_device::minx_CF_08, &minx_cpu_device::minx_CF_09, &minx_cpu_device::minx_CF_0A, &minx_cpu_device::minx_CF_0B, &minx_cpu_device::minx_CF_0C, &minx_cpu_device::minx_CF_0D, &minx_cpu_device::minx_CF_0E, &minx_cpu_device::minx_CF_0F,
	&minx_cpu_device::minx_CF_10, &minx_cpu_device::minx_CF_11, &minx_cpu_device::minx_CF_12, &minx_cpu_device::minx_CF_13, &minx_cpu_device::minx_CF_14, &minx_cpu_device::minx_CF_15, &minx_cpu_device::minx_CF_16, &minx_cpu_device::minx_CF_17,
	&minx_cpu_device::minx_CF_18, &minx_cpu_device::minx_CF_19, &minx_cpu_device::minx_CF_1A, &minx_cpu_device::minx_CF_1B, &minx_cpu_device::minx_CF_1C, &minx_cpu_device::minx_CF_1D, &minx_cpu_device::minx_CF_1E, &minx_cpu_device::minx_CF_1F,
	&minx_cpu_device::minx_CF_20, &minx_cpu_device::minx_CF_21, &minx_cpu_device::minx_CF_22, &minx_cpu_device::minx_CF_23, &minx_cpu_device::minx_CF_24, &minx_cpu_device::minx_CF_25, &minx_cpu_device::minx_CF_26, &minx_cpu_device::minx_CF_27,
	&minx_cpu_device::minx_CF_28, &minx_cpu_device::minx_CF_29, &minx_cpu_device::minx_CF_2A, &minx_cpu_device::minx_CF_2B, &minx_cpu_device::minx_CF_2C, &minx_cpu_device::minx_CF_2D, &minx_cpu_device::minx_CF_2E, &minx_cpu_device::minx_CF_2F,
	&minx_cpu_device::minx_CF_30, &minx_cpu_device::minx_CF_31, &minx_cpu_device::minx_CF_32, &minx_cpu_device::minx_CF_33, &minx_cpu_device::minx_CF_34, &minx_cpu_device::minx_CF_35, &minx_cpu_device::minx_CF_36, &minx_cpu_device::minx_CF_37,
	&minx_cpu_device::minx_CF_38, &minx_cpu_device::minx_CF_39, &minx_cpu_device::minx_CF_3A, &minx_cpu_device::minx_CF_3B, &minx_cpu_device::minx_CF_3C, &minx_cpu_device::minx_CF_3D, &minx_cpu_device::minx_CF_3E, &minx_cpu_device::minx_CF_3F,
	&minx_cpu_device::minx_CF_40, &minx_cpu_device::minx_CF_41, &minx_cpu_device::minx_CF_42, &minx_cpu_device::minx_CF_43, &minx_cpu_device::minx_CF_44, &minx_cpu_device::minx_CF_45, &minx_cpu_device::minx_CF_46, &minx_cpu_device::minx_CF_47,
	&minx_cpu_device::minx_CF_48, &minx_cpu_device::minx_CF_49, &minx_cpu_device::minx_CF_4A, &minx_cpu_device::minx_CF_4B, &minx_cpu_device::minx_CF_4C, &minx_cpu_device::minx_CF_4D, &minx_cpu_device::minx_CF_4E, &minx_cpu_device::minx_CF_4F,
	&minx_cpu_device::minx_CF_50, &minx_cpu_device::minx_CF_51, &minx_cpu_device::minx_CF_52, &minx_cpu_device::minx_CF_53, &minx_cpu_device::minx_CF_54, &minx_cpu_device::minx_CF_55, &minx_cpu_device::minx_CF_56, &minx_cpu_device::minx_CF_57,
	&minx_cpu_device::minx_CF_58, &minx_cpu_device::minx_CF_59, &minx_cpu_device::minx_CF_5A, &minx_cpu_device::minx_CF_5B, &minx_cpu_device::minx_CF_5C, &minx_cpu_device::minx_CF_5D, &minx_cpu_device::minx_CF_5E, &minx_cpu_device::minx_CF_5F,
	&minx_cpu_device::minx_CF_60, &minx_cpu_device::minx_CF_61, &minx_cpu_device::minx_CF_62, &minx_cpu_device::minx_CF_63, &minx_cpu_device::minx_CF_64, &minx_cpu_device::minx_CF_65, &minx_cpu_device::minx_CF_66, &minx_cpu_device::minx_CF_67,
	&minx_cpu_device::minx_CF_68, &minx_cpu_device::minx_CF_69, &minx_cpu_device::minx_CF_6A, &minx_cpu_device::minx_CF_6B, &minx_cpu_device::minx_CF_6C, &minx_cpu_device::minx_CF_6D, &minx_cpu_device::minx_CF_6E, &minx_cpu_device::minx_CF_6F,
	&minx_cpu_device::minx_CF_70, &minx_cpu_device::minx_CF_71, &minx_cpu_device::minx_CF_72, &minx_cpu_device::minx_CF_73, &minx_cpu_device::minx_CF_74, &minx_cpu_device::minx_CF_75, &minx_cpu_device::minx_CF_76, &minx_cpu_device::minx_CF_77,
	&minx_cpu_device::minx_CF_78, &minx_cpu_device::minx_CF_79, &minx_cpu_device::minx_CF_7A, &minx_cpu_device::minx_CF_7B, &minx_cpu_device::minx_CF_7C, &minx_cpu_device::minx_CF_7D, &minx_cpu_device::minx_CF_7E, &minx_cpu_device::minx_CF_7F,
	&minx_cpu_device::minx_CF_80, &minx_cpu_device::minx_CF_81, &minx_cpu_device::minx_CF_82, &minx_cpu_device::minx_CF_83, &minx_cpu_device::minx_CF_84, &minx_cpu_device::minx_CF_85, &minx_cpu_device::minx_CF_86, &minx_cpu_device::minx_CF_87,
	&minx_cpu_device::minx_CF_88, &minx_cpu_device::minx_CF_89, &minx_cpu_device::minx_CF_8A, &minx_cpu_device::minx_CF_8B, &minx_cpu_device::minx_CF_8C, &minx_cpu_device::minx_CF_8D, &minx_cpu_device::minx_CF_8E, &minx_cpu_device::minx_CF_8F,
	&minx_cpu_device::minx_CF_90, &minx_cpu_device::minx_CF_91, &minx_cpu_device::minx_CF_92, &minx_cpu_device::minx_CF_93, &minx_cpu_device::minx_CF_94, &minx_cpu_device::minx_CF_95, &minx_cpu_device::minx_CF_96, &minx_cpu_device::minx_CF_97,
	&minx_cpu_device::minx_CF_98, &minx_cpu_device::minx_CF_99, &minx_cpu_device::minx_CF_9A, &minx_cpu_device::minx_CF_9B, &minx_cpu_device::minx_CF_9C, &minx_cpu_device::minx_CF_9D, &minx_cpu_device::minx_CF_9E, &minx_cpu_device::minx_CF_9F,
	&minx_cpu_device::minx_CF_A0, &minx_cpu_device::minx_CF_A1, &minx_cpu_device::minx_CF_A2, &minx_cpu_device::minx_CF_A3, &minx_cpu_device::minx_CF_A4, &minx_cpu_device::minx_CF_A5, &minx_cpu_device::minx_CF_A6, &minx_cpu_device::minx_CF_A7,
	&minx_cpu_device::minx_CF_A8, &minx_cpu_device::minx_CF_A9, &minx_cpu_device::minx_CF_AA, &minx_cpu_device::minx_CF_AB, &minx_cpu_device::minx_CF_AC, &minx_cpu_device::minx_CF_AD, &minx_cpu_device::minx_CF_AE, &minx_cpu_device::minx_CF_AF,
	&minx_cpu_device::minx_CF_B0, &minx_cpu_device::minx_CF_B1, &minx_cpu_device::minx_CF_B2, &minx_cpu_device::minx_CF_B3, &minx_cpu_device::minx_CF_B4, &minx_cpu_device::minx_CF_B5, &minx_cpu_device::minx_CF_B6, &minx_cpu_device::minx_CF_B7,
	&minx_cpu_device::minx_CF_B8, &minx_cpu_device::minx_CF_B9, &minx_cpu_device::minx_CF_BA, &minx_cpu_device::minx_CF_BB, &minx_cpu_device::minx_CF_BC, &minx_cpu_device::minx_CF_BD, &minx_cpu_device::minx_CF_BE, &minx_cpu_device::minx_CF_BF,
	&minx_cpu_device::minx_CF_C0, &minx_cpu_device::minx_CF_C1, &minx_cpu_device::minx_CF_C2, &minx_cpu_device::minx_CF_C3, &minx_cpu_device::minx_CF_C4, &minx_cpu_device::minx_CF_C5, &minx_cpu_device::minx_CF_C6, &minx_cpu_device::minx_CF_C7,
	&minx_cpu_device::minx_CF_C8, &minx_cpu_device::minx_CF_C9, &minx_cpu_device::minx_CF_CA, &minx_cpu_device::minx_CF_CB, &minx_cpu_device::minx_CF_CC, &minx_cpu_device::minx_CF_CD, &minx_cpu_device::minx_CF_CE, &minx_cpu_device::minx_CF_CF,
	&minx_cpu_device::minx_CF_D0, &minx_cpu_device::minx_CF_D1, &minx_cpu_device::minx_CF_D2, &minx_cpu_device::minx_CF_D3, &minx_cpu_device::minx_CF_D4, &minx_cpu_device::minx_CF_D5, &minx_cpu_device::minx_CF_D6, &minx_cpu_device::minx_CF_D7,
	&minx_cpu_device::minx_CF_D8, &minx_cpu_device::minx_CF_D9, &minx_cpu_device::minx_CF_DA, &minx_cpu_device::minx_CF_DB, &minx_cpu_device::minx_CF_DC, &minx_cpu_device::minx_CF_DD, &minx_cpu_device::minx_CF_DE, &minx_cpu_device::minx_CF_DF,
	&minx_cpu_device::minx_CF_E0, &minx_cpu_device::minx_CF_E1, &minx_cpu_device::minx_CF_E2, &minx_cpu_device::minx_CF_E3, &minx_cpu_device::minx_CF_E4, &minx_cpu_device::minx_CF_E5, &minx_cpu_device::minx_CF_E6, &minx_cpu_device::minx_CF_E7,
	&minx_cpu_device::minx_CF_E8, &minx_cpu_device::minx_CF_E9, &minx_cpu_device::minx_CF_EA, &minx_cpu_device::minx_CF_EB, &minx_cpu_device::minx_CF_EC, &minx_cpu_device::minx_CF_ED, &minx_cpu_device::minx_CF_EE, &minx_cpu_device::minx_CF_EF,
	&minx_cpu_device::minx_CF_F0, &minx_cpu_device::minx_CF_F1, &minx_cpu_device::minx_CF_F2, &minx_cpu_device::minx_CF_F3, &minx_cpu_device::minx_CF_F4, &minx_cpu_device::minx_CF_F5, &minx_cpu_device::minx_CF_F6, &minx_cpu_device::minx_CF_F7,
	&minx_cpu_device::minx_CF_F8, &minx_cpu_device::minx_CF_F9, &minx_cpu_device::minx_CF_FA, &minx_cpu_device::minx_CF_FB, &minx_cpu_device::minx_CF_FC, &minx_cpu_device::minx_CF_FD, &minx_cpu_device::minx_CF_FE, &minx_cpu_device::minx_CF_FF
};

const int minx_cpu_device::insnminx_cycles_CF[256] = {
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	1,  1,  1,  1,  1,  1,  1,  1, 16, 16, 16, 16,  1,  1,  1,  1,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	1,  1,  1,  1,  1,  1,  1,  1, 16, 16, 16, 16,  1,  1,  1,  1,

	16, 16, 16, 16, 16, 16,  1,  1, 16, 16, 16, 16, 16, 16,  1,  1,
	1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 16, 16,  1,  1,
	16, 16, 16, 16,  1,  1,  1,  1, 16,  1, 16,  1, 16,  1, 16,  1,
	24, 24, 24, 24, 24, 24, 24, 24, 24,  1,  1,  1, 24,  1,  1,  1,

	1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	12, 12, 12, 12, 12, 12, 12, 12, 48, 60,  1,  1, 32, 40,  1,  1,

	20, 20, 20, 20, 20, 20, 20, 20,  1,  1,  1,  1,  1,  1,  1,  1,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	8,  8,  8,  8,  8,  8,  1,  1,  8,  8,  8,  1,  1,  1,  8,  1
};
