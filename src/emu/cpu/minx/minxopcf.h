#undef OP
#define OP(nn) INLINE void minx_CF_##nn(minx_state *minx)

OP(00) { minx->BA = ADD16( minx, minx->BA, minx->BA ); }
OP(01) { minx->BA = ADD16( minx, minx->BA, minx->HL ); }
OP(02) { minx->BA = ADD16( minx, minx->BA, minx->X ); }
OP(03) { minx->BA = ADD16( minx, minx->BA, minx->Y ); }
OP(04) { minx->BA = ADDC16( minx, minx->BA, minx->BA ); }
OP(05) { minx->BA = ADDC16( minx, minx->BA, minx->HL ); }
OP(06) { minx->BA = ADDC16( minx, minx->BA, minx->X ); }
OP(07) { minx->BA = ADDC16( minx, minx->BA, minx->Y ); }
OP(08) { minx->BA = SUB16( minx, minx->BA, minx->BA ); }
OP(09) { minx->BA = SUB16( minx, minx->BA, minx->HL ); }
OP(0A) { minx->BA = SUB16( minx, minx->BA, minx->X ); }
OP(0B) { minx->BA = SUB16( minx, minx->BA, minx->Y ); }
OP(0C) { minx->BA = SUBC16( minx, minx->BA, minx->BA ); }
OP(0D) { minx->BA = SUBC16( minx, minx->BA, minx->HL ); }
OP(0E) { minx->BA = SUBC16( minx, minx->BA, minx->X ); }
OP(0F) { minx->BA = SUBC16( minx, minx->BA, minx->Y ); }

OP(10) { /* illegal instruction? */ }
OP(11) { /* illegal instruction? */ }
OP(12) { /* illegal instruction? */ }
OP(13) { /* illegal instruction? */ }
OP(14) { /* illegal instruction? */ }
OP(15) { /* illegal instruction? */ }
OP(16) { /* illegal instruction? */ }
OP(17) { /* illegal instruction? */ }
OP(18) { SUB16( minx, minx->BA, minx->BA ); }
OP(19) { SUB16( minx, minx->BA, minx->HL ); }
OP(1A) { SUB16( minx, minx->BA, minx->X ); }
OP(1B) { SUB16( minx, minx->BA, minx->Y ); }
OP(1C) { /* illegal instruction? */ }
OP(1D) { /* illegal instruction? */ }
OP(1E) { /* illegal instruction? */ }
OP(1F) { /* illegal instruction? */ }

OP(20) { minx->HL = ADD16( minx, minx->HL, minx->BA ); }
OP(21) { minx->HL = ADD16( minx, minx->HL, minx->HL ); }
OP(22) { minx->HL = ADD16( minx, minx->HL, minx->X ); }
OP(23) { minx->HL = ADD16( minx, minx->HL, minx->Y ); }
OP(24) { minx->HL = ADDC16( minx, minx->HL, minx->BA ); }
OP(25) { minx->HL = ADDC16( minx, minx->HL, minx->HL ); }
OP(26) { minx->HL = ADDC16( minx, minx->HL, minx->X ); }
OP(27) { minx->HL = ADDC16( minx, minx->HL, minx->Y ); }
OP(28) { minx->HL = SUB16( minx, minx->HL, minx->BA ); }
OP(29) { minx->HL = SUB16( minx, minx->HL, minx->HL ); }
OP(2A) { minx->HL = SUB16( minx, minx->HL, minx->X ); }
OP(2B) { minx->HL = SUB16( minx, minx->HL, minx->Y ); }
OP(2C) { minx->HL = SUBC16( minx, minx->HL, minx->BA ); }
OP(2D) { minx->HL = SUBC16( minx, minx->HL, minx->HL ); }
OP(2E) { minx->HL = SUBC16( minx, minx->HL, minx->X ); }
OP(2F) { minx->HL = SUBC16( minx, minx->HL, minx->Y ); }

OP(30) { /* illegal instruction? */ }
OP(31) { /* illegal instruction? */ }
OP(32) { /* illegal instruction? */ }
OP(33) { /* illegal instruction? */ }
OP(34) { /* illegal instruction? */ }
OP(35) { /* illegal instruction? */ }
OP(36) { /* illegal instruction? */ }
OP(37) { /* illegal instruction? */ }
OP(38) { SUB16( minx, minx->HL, minx->BA ); }
OP(39) { SUB16( minx, minx->HL, minx->HL ); }
OP(3A) { SUB16( minx, minx->HL, minx->X ); }
OP(3B) { SUB16( minx, minx->HL, minx->Y ); }
OP(3C) { /* illegal instruction? */ }
OP(3D) { /* illegal instruction? */ }
OP(3E) { /* illegal instruction? */ }
OP(3F) { /* illegal instruction? */ }

OP(40) { minx->X = ADD16( minx, minx->X, minx->BA ); }
OP(41) { minx->X = ADD16( minx, minx->X, minx->HL ); }
OP(42) { minx->Y = ADD16( minx, minx->Y, minx->BA ); }
OP(43) { minx->Y = ADD16( minx, minx->Y, minx->HL ); }
OP(44) { minx->SP = ADD16( minx, minx->SP, minx->BA ); }
OP(45) { minx->SP = ADD16( minx, minx->SP, minx->HL ); }
OP(46) { /* illegal instruction? */ }
OP(47) { /* illegal instruction? */ }
OP(48) { minx->X = SUB16( minx, minx->X, minx->BA ); }
OP(49) { minx->X = SUB16( minx, minx->X, minx->HL ); }
OP(4A) { minx->Y = SUB16( minx, minx->Y, minx->BA ); }
OP(4B) { minx->Y = SUB16( minx, minx->Y, minx->HL ); }
OP(4C) { minx->SP = SUB16( minx, minx->SP, minx->BA ); }
OP(4D) { minx->SP = SUB16( minx, minx->SP, minx->HL ); }
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
OP(5C) { SUB16( minx, minx->SP, minx->BA ); }
OP(5D) { SUB16( minx, minx->SP, minx->HL ); }
OP(5E) { /* illegal instruction? */ }
OP(5F) { /* illegal instruction? */ }

OP(60) { ADDC16( minx, minx->BA, rdop16(minx) ); /* ??? */ }
OP(61) { ADDC16( minx, minx->HL, rdop16(minx) ); /* ??? */ }
OP(62) { ADDC16( minx, minx->X, rdop16(minx) ); /* ??? */ }
OP(63) { ADDC16( minx, minx->Y, rdop16(minx) ); /* ??? */ }
OP(64) { /* illegal instruction? */ }
OP(65) { /* illegal instruction? */ }
OP(66) { /* illegal instruction? */ }
OP(67) { /* illegal instruction? */ }
OP(68) { minx->SP = ADD16( minx, minx->SP, rdop16(minx) ); }
OP(69) { /* illegal instruction? */ }
OP(6A) { minx->SP = SUB16( minx, minx->SP, rdop16(minx) ); }
OP(6B) { /* illegal instruction? */ }
OP(6C) { SUB16( minx, minx->SP, rdop16(minx) ); }
OP(6D) { /* illegal instruction? */ }
OP(6E) { minx->SP = rdop16(minx); }
OP(6F) { /* illegal instruction? */ }

OP(70) { UINT8 ofs8 = rdop(minx); minx->BA = rd16( minx, minx->SP + ofs8 ); }
OP(71) { UINT8 ofs8 = rdop(minx); minx->HL = rd16( minx, minx->SP + ofs8 ); }
OP(72) { UINT8 ofs8 = rdop(minx); minx->X = rd16( minx, minx->SP + ofs8 ); }
OP(73) { UINT8 ofs8 = rdop(minx); minx->Y = rd16( minx, minx->SP + ofs8 ); }
OP(74) { UINT8 ofs8 = rdop(minx); wr16( minx, minx->SP + ofs8, minx->BA ); }
OP(75) { UINT8 ofs8 = rdop(minx); wr16( minx, minx->SP + ofs8, minx->HL ); }
OP(76) { UINT8 ofs8 = rdop(minx); wr16( minx, minx->SP + ofs8, minx->X ); }
OP(77) { UINT8 ofs8 = rdop(minx); wr16( minx, minx->SP + ofs8, minx->Y ); }
OP(78) { AD2_I16; minx->SP = rd16( minx, addr2 ); }
OP(79) { /* illegal instruction? */ }
OP(7A) { /* illegal instruction? */ }
OP(7B) { /* illegal instruction? */ }
OP(7C) { AD1_I16; wr16( minx, addr1, minx->SP ); }
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

OP(B0) { PUSH8( minx, minx->BA & 0x00FF ); }
OP(B1) { PUSH8( minx, minx->BA >> 8 ); }
OP(B2) { PUSH8( minx, minx->HL & 0x00FF ); }
OP(B3) { PUSH8( minx, minx->HL >> 8 ); }
OP(B4) { minx->BA = ( minx->BA & 0xFF00 ) | POP8(minx); }
OP(B5) { minx->BA = ( minx->BA & 0x00FF ) | ( POP8(minx) << 8 ); }
OP(B6) { minx->HL = ( minx->HL & 0xFF00 ) | POP8(minx); }
OP(B7) { minx->HL = ( minx->HL & 0x00FF ) | ( POP8(minx) << 8 ); }
OP(B8) { PUSH16( minx, minx->BA ); PUSH16( minx, minx->HL ); PUSH16( minx, minx->X ); PUSH16( minx, minx->Y ); PUSH8( minx, minx->N ); }
OP(B9) { PUSH16( minx, minx->BA ); PUSH16( minx, minx->HL ); PUSH16( minx, minx->X ); PUSH16( minx, minx->Y ); PUSH8( minx, minx->N ); PUSH8( minx, minx->I ); PUSH8( minx, minx->XI ); PUSH8( minx, minx->YI ); }
OP(BA) { /* illegal instruction? */ }
OP(BB) { /* illegal instruction? */ }
OP(BC) { minx->N = POP8(minx); minx->Y = POP16(minx); minx->X = POP16(minx); minx->HL = POP16(minx); minx->BA = POP16(minx); }
OP(BD) { minx->YI = POP8(minx); minx->XI = POP8(minx); minx->I = POP8(minx); minx->N = POP8(minx); minx->Y = POP16(minx); minx->X = POP16(minx); minx->HL = POP16(minx); minx->BA = POP16(minx); }
OP(BE) { /* illegal instruction? */ }
OP(BF) { /* illegal instruction? */ }

OP(C0) { AD2_IHL; minx->BA = rd16( minx, addr2 ); }
OP(C1) { AD2_IHL; minx->HL = rd16( minx, addr2 ); }
OP(C2) { AD2_IHL; minx->X = rd16( minx, addr2 ); }
OP(C3) { AD2_IHL; minx->Y = rd16( minx, addr2 ); }
OP(C4) { AD1_IHL; wr16( minx, addr1, minx->BA ); }
OP(C5) { AD1_IHL; wr16( minx, addr1, minx->HL ); }
OP(C6) { AD1_IHL; wr16( minx, addr1, minx->X ); }
OP(C7) { AD1_IHL; wr16( minx, addr1, minx->Y ); }
OP(C8) { /* illegal instruction? */ }
OP(C9) { /* illegal instruction? */ }
OP(CA) { /* illegal instruction? */ }
OP(CB) { /* illegal instruction? */ }
OP(CC) { /* illegal instruction? */ }
OP(CD) { /* illegal instruction? */ }
OP(CE) { /* illegal instruction? */ }
OP(CF) { /* illegal instruction? */ }

OP(D0) { AD2_XIX; minx->BA = rd16( minx, addr2 ); }
OP(D1) { AD2_XIX; minx->HL = rd16( minx, addr2 ); }
OP(D2) { AD2_XIX; minx->X = rd16( minx, addr2 ); }
OP(D3) { AD2_XIX; minx->Y = rd16( minx, addr2 ); }
OP(D4) { AD1_XIX; wr16( minx, addr1, minx->BA ); }
OP(D5) { AD1_XIX; wr16( minx, addr1, minx->HL ); }
OP(D6) { AD1_XIX; wr16( minx, addr1, minx->X ); }
OP(D7) { AD1_XIX; wr16( minx, addr1, minx->Y ); }
OP(D8) { AD2_YIY; minx->BA = rd16( minx, addr2 ); }
OP(D9) { AD2_YIY; minx->HL = rd16( minx, addr2 ); }
OP(DA) { AD2_YIY; minx->X = rd16( minx, addr2 ); }
OP(DB) { AD2_YIY; minx->Y = rd16( minx, addr2 ); }
OP(DC) { AD1_YIY; wr16( minx, addr1, minx->BA ); }
OP(DD) { AD1_YIY; wr16( minx, addr1, minx->HL ); }
OP(DE) { AD1_YIY; wr16( minx, addr1, minx->X ); }
OP(DF) { AD1_YIY; wr16( minx, addr1, minx->Y ); }

OP(E0) { minx->BA = minx->BA; }
OP(E1) { minx->BA = minx->HL; }
OP(E2) { minx->BA = minx->X; }
OP(E3) { minx->BA = minx->Y; }
OP(E4) { minx->HL = minx->BA; }
OP(E5) { minx->HL = minx->HL; }
OP(E6) { minx->HL = minx->X; }
OP(E7) { minx->HL = minx->Y; }
OP(E8) { minx->X = minx->BA; }
OP(E9) { minx->X = minx->HL; }
OP(EA) { minx->X = minx->X; }
OP(EB) { minx->X = minx->Y; }
OP(EC) { minx->Y = minx->BA; }
OP(ED) { minx->Y = minx->HL; }
OP(EE) { minx->Y = minx->X; }
OP(EF) { minx->Y = minx->Y; }

OP(F0) { minx->SP = minx->BA; }
OP(F1) { minx->SP = minx->HL; }
OP(F2) { minx->SP = minx->X; }
OP(F3) { minx->SP = minx->Y; }
OP(F4) { minx->HL = minx->SP; }
OP(F5) { minx->HL = minx->PC; }
OP(F6) { /* illegal instruction? */ }
OP(F7) { /* illegal instruction? */ }
OP(F8) { minx->BA = minx->SP; }
OP(F9) { minx->BA = minx->PC; }
OP(FA) { minx->X = minx->SP; }
OP(FB) { /* illegal instruction? */ }
OP(FC) { /* illegal instruction? */ }
OP(FD) { /* illegal instruction? */ }
OP(FE) { minx->Y = minx->SP; }
OP(FF) { /* illegal instruction? */ }

static void (*const insnminx_CF[256])(minx_state *minx) = {
	minx_CF_00, minx_CF_01, minx_CF_02, minx_CF_03, minx_CF_04, minx_CF_05, minx_CF_06, minx_CF_07,
	minx_CF_08, minx_CF_09, minx_CF_0A, minx_CF_0B, minx_CF_0C, minx_CF_0D, minx_CF_0E, minx_CF_0F,
	minx_CF_10, minx_CF_11, minx_CF_12, minx_CF_13, minx_CF_14, minx_CF_15, minx_CF_16, minx_CF_17,
	minx_CF_18, minx_CF_19, minx_CF_1A, minx_CF_1B, minx_CF_1C, minx_CF_1D, minx_CF_1E, minx_CF_1F,
	minx_CF_20, minx_CF_21, minx_CF_22, minx_CF_23, minx_CF_24, minx_CF_25, minx_CF_26, minx_CF_27,
	minx_CF_28, minx_CF_29, minx_CF_2A, minx_CF_2B, minx_CF_2C, minx_CF_2D, minx_CF_2E, minx_CF_2F,
	minx_CF_30, minx_CF_31, minx_CF_32, minx_CF_33, minx_CF_34, minx_CF_35, minx_CF_36, minx_CF_37,
	minx_CF_38, minx_CF_39, minx_CF_3A, minx_CF_3B, minx_CF_3C, minx_CF_3D, minx_CF_3E, minx_CF_3F,
	minx_CF_40, minx_CF_41, minx_CF_42, minx_CF_43, minx_CF_44, minx_CF_45, minx_CF_46, minx_CF_47,
	minx_CF_48, minx_CF_49, minx_CF_4A, minx_CF_4B, minx_CF_4C, minx_CF_4D, minx_CF_4E, minx_CF_4F,
	minx_CF_50, minx_CF_51, minx_CF_52, minx_CF_53, minx_CF_54, minx_CF_55, minx_CF_56, minx_CF_57,
	minx_CF_58, minx_CF_59, minx_CF_5A, minx_CF_5B, minx_CF_5C, minx_CF_5D, minx_CF_5E, minx_CF_5F,
	minx_CF_60, minx_CF_61, minx_CF_62, minx_CF_63, minx_CF_64, minx_CF_65, minx_CF_66, minx_CF_67,
	minx_CF_68, minx_CF_69, minx_CF_6A, minx_CF_6B, minx_CF_6C, minx_CF_6D, minx_CF_6E, minx_CF_6F,
	minx_CF_70, minx_CF_71, minx_CF_72, minx_CF_73, minx_CF_74, minx_CF_75, minx_CF_76, minx_CF_77,
	minx_CF_78, minx_CF_79, minx_CF_7A, minx_CF_7B, minx_CF_7C, minx_CF_7D, minx_CF_7E, minx_CF_7F,
	minx_CF_80, minx_CF_81, minx_CF_82, minx_CF_83, minx_CF_84, minx_CF_85, minx_CF_86, minx_CF_87,
	minx_CF_88, minx_CF_89, minx_CF_8A, minx_CF_8B, minx_CF_8C, minx_CF_8D, minx_CF_8E, minx_CF_8F,
	minx_CF_90, minx_CF_91, minx_CF_92, minx_CF_93, minx_CF_94, minx_CF_95, minx_CF_96, minx_CF_97,
	minx_CF_98, minx_CF_99, minx_CF_9A, minx_CF_9B, minx_CF_9C, minx_CF_9D, minx_CF_9E, minx_CF_9F,
	minx_CF_A0, minx_CF_A1, minx_CF_A2, minx_CF_A3, minx_CF_A4, minx_CF_A5, minx_CF_A6, minx_CF_A7,
	minx_CF_A8, minx_CF_A9, minx_CF_AA, minx_CF_AB, minx_CF_AC, minx_CF_AD, minx_CF_AE, minx_CF_AF,
	minx_CF_B0, minx_CF_B1, minx_CF_B2, minx_CF_B3, minx_CF_B4, minx_CF_B5, minx_CF_B6, minx_CF_B7,
	minx_CF_B8, minx_CF_B9, minx_CF_BA, minx_CF_BB, minx_CF_BC, minx_CF_BD, minx_CF_BE, minx_CF_BF,
	minx_CF_C0, minx_CF_C1, minx_CF_C2, minx_CF_C3, minx_CF_C4, minx_CF_C5, minx_CF_C6, minx_CF_C7,
	minx_CF_C8, minx_CF_C9, minx_CF_CA, minx_CF_CB, minx_CF_CC, minx_CF_CD, minx_CF_CE, minx_CF_CF,
	minx_CF_D0, minx_CF_D1, minx_CF_D2, minx_CF_D3, minx_CF_D4, minx_CF_D5, minx_CF_D6, minx_CF_D7,
	minx_CF_D8, minx_CF_D9, minx_CF_DA, minx_CF_DB, minx_CF_DC, minx_CF_DD, minx_CF_DE, minx_CF_DF,
	minx_CF_E0, minx_CF_E1, minx_CF_E2, minx_CF_E3, minx_CF_E4, minx_CF_E5, minx_CF_E6, minx_CF_E7,
	minx_CF_E8, minx_CF_E9, minx_CF_EA, minx_CF_EB, minx_CF_EC, minx_CF_ED, minx_CF_EE, minx_CF_EF,
	minx_CF_F0, minx_CF_F1, minx_CF_F2, minx_CF_F3, minx_CF_F4, minx_CF_F5, minx_CF_F6, minx_CF_F7,
	minx_CF_F8, minx_CF_F9, minx_CF_FA, minx_CF_FB, minx_CF_FC, minx_CF_FD, minx_CF_FE, minx_CF_FF
};

static const int insnminx_cycles_CF[256] = {
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
