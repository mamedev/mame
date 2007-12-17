
#undef OP
#define OP(nn) INLINE void minx_CF_##nn(void)

OP(00) { regs.BA = ADD16( regs.BA, regs.BA ); }
OP(01) { regs.BA = ADD16( regs.BA, regs.HL ); }
OP(02) { regs.BA = ADD16( regs.BA, regs.X ); }
OP(03) { regs.BA = ADD16( regs.BA, regs.Y ); }
OP(04) { regs.BA = ADDC16( regs.BA, regs.BA ); }
OP(05) { regs.BA = ADDC16( regs.BA, regs.HL ); }
OP(06) { regs.BA = ADDC16( regs.BA, regs.X ); }
OP(07) { regs.BA = ADDC16( regs.BA, regs.Y ); }
OP(08) { regs.BA = SUB16( regs.BA, regs.BA ); }
OP(09) { regs.BA = SUB16( regs.BA, regs.HL ); }
OP(0A) { regs.BA = SUB16( regs.BA, regs.X ); }
OP(0B) { regs.BA = SUB16( regs.BA, regs.Y ); }
OP(0C) { regs.BA = SUBC16( regs.BA, regs.BA ); }
OP(0D) { regs.BA = SUBC16( regs.BA, regs.HL ); }
OP(0E) { regs.BA = SUBC16( regs.BA, regs.X ); }
OP(0F) { regs.BA = SUBC16( regs.BA, regs.Y ); }

OP(10) { /* illegal instruction? */ }
OP(11) { /* illegal instruction? */ }
OP(12) { /* illegal instruction? */ }
OP(13) { /* illegal instruction? */ }
OP(14) { /* illegal instruction? */ }
OP(15) { /* illegal instruction? */ }
OP(16) { /* illegal instruction? */ }
OP(17) { /* illegal instruction? */ }
OP(18) { SUB16( regs.BA, regs.BA ); }
OP(19) { SUB16( regs.BA, regs.HL ); }
OP(1A) { SUB16( regs.BA, regs.X ); }
OP(1B) { SUB16( regs.BA, regs.Y ); }
OP(1C) { /* illegal instruction? */ }
OP(1D) { /* illegal instruction? */ }
OP(1E) { /* illegal instruction? */ }
OP(1F) { /* illegal instruction? */ }

OP(20) { regs.HL = ADD16( regs.HL, regs.BA ); }
OP(21) { regs.HL = ADD16( regs.HL, regs.HL ); }
OP(22) { regs.HL = ADD16( regs.HL, regs.X ); }
OP(23) { regs.HL = ADD16( regs.HL, regs.Y ); }
OP(24) { regs.HL = ADDC16( regs.HL, regs.BA ); }
OP(25) { regs.HL = ADDC16( regs.HL, regs.HL ); }
OP(26) { regs.HL = ADDC16( regs.HL, regs.X ); }
OP(27) { regs.HL = ADDC16( regs.HL, regs.Y ); }
OP(28) { regs.HL = SUB16( regs.HL, regs.BA ); }
OP(29) { regs.HL = SUB16( regs.HL, regs.HL ); }
OP(2A) { regs.HL = SUB16( regs.HL, regs.X ); }
OP(2B) { regs.HL = SUB16( regs.HL, regs.Y ); }
OP(2C) { regs.HL = SUBC16( regs.HL, regs.BA ); }
OP(2D) { regs.HL = SUBC16( regs.HL, regs.HL ); }
OP(2E) { regs.HL = SUBC16( regs.HL, regs.X ); }
OP(2F) { regs.HL = SUBC16( regs.HL, regs.Y ); }

OP(30) { /* illegal instruction? */ }
OP(31) { /* illegal instruction? */ }
OP(32) { /* illegal instruction? */ }
OP(33) { /* illegal instruction? */ }
OP(34) { /* illegal instruction? */ }
OP(35) { /* illegal instruction? */ }
OP(36) { /* illegal instruction? */ }
OP(37) { /* illegal instruction? */ }
OP(38) { SUB16( regs.HL, regs.BA ); }
OP(39) { SUB16( regs.HL, regs.HL ); }
OP(3A) { SUB16( regs.HL, regs.X ); }
OP(3B) { SUB16( regs.HL, regs.Y ); }
OP(3C) { /* illegal instruction? */ }
OP(3D) { /* illegal instruction? */ }
OP(3E) { /* illegal instruction? */ }
OP(3F) { /* illegal instruction? */ }

OP(40) { regs.X = ADD16( regs.X, regs.BA ); }
OP(41) { regs.X = ADD16( regs.X, regs.HL ); }
OP(42) { regs.Y = ADD16( regs.Y, regs.BA ); }
OP(43) { regs.Y = ADD16( regs.Y, regs.HL ); }
OP(44) { regs.SP = ADD16( regs.SP, regs.BA ); }
OP(45) { regs.SP = ADD16( regs.SP, regs.HL ); }
OP(46) { /* illegal instruction? */ }
OP(47) { /* illegal instruction? */ }
OP(48) { regs.X = SUB16( regs.X, regs.BA ); }
OP(49) { regs.X = SUB16( regs.X, regs.HL ); }
OP(4A) { regs.Y = SUB16( regs.Y, regs.BA ); }
OP(4B) { regs.Y = SUB16( regs.Y, regs.HL ); }
OP(4C) { regs.SP = SUB16( regs.SP, regs.BA ); }
OP(4D) { regs.SP = SUB16( regs.SP, regs.HL ); }
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
OP(5C) { SUB16( regs.SP, regs.BA ); }
OP(5D) { SUB16( regs.SP, regs.HL ); }
OP(5E) { /* illegal instruction? */ }
OP(5F) { /* illegal instruction? */ }

OP(60) { ADDC16( regs.BA, rdop16() ); /* ??? */ }
OP(61) { ADDC16( regs.HL, rdop16() ); /* ??? */ }
OP(62) { ADDC16( regs.X, rdop16() ); /* ??? */ }
OP(63) { ADDC16( regs.Y, rdop16() ); /* ??? */ }
OP(64) { /* illegal instruction? */ }
OP(65) { /* illegal instruction? */ }
OP(66) { /* illegal instruction? */ }
OP(67) { /* illegal instruction? */ }
OP(68) { regs.SP = ADD16( regs.SP, rdop16() ); }
OP(69) { /* illegal instruction? */ }
OP(6A) { regs.SP = SUB16( regs.SP, rdop16() ); }
OP(6B) { /* illegal instruction? */ }
OP(6C) { SUB16( regs.SP, rdop16() ); }
OP(6D) { /* illegal instruction? */ }
OP(6E) { regs.SP = rdop16(); }
OP(6F) { /* illegal instruction? */ }

OP(70) { UINT8 ofs8 = rdop(); regs.BA = rd( regs.SP + ofs8 ); }
OP(71) { UINT8 ofs8 = rdop(); regs.HL = rd( regs.SP + ofs8 ); }
OP(72) { UINT8 ofs8 = rdop(); regs.X = rd( regs.SP + ofs8 ); }
OP(73) { UINT8 ofs8 = rdop(); regs.Y = rd( regs.SP + ofs8 ); }
OP(74) { UINT8 ofs8 = rdop(); wr( regs.SP + ofs8, regs.BA ); }
OP(75) { UINT8 ofs8 = rdop(); wr( regs.SP + ofs8, regs.HL ); }
OP(76) { UINT8 ofs8 = rdop(); wr( regs.SP + ofs8, regs.X ); }
OP(77) { UINT8 ofs8 = rdop(); wr( regs.SP + ofs8, regs.Y ); }
OP(78) { AD2_I16; regs.SP = rd16( addr2 ); }
OP(79) { /* illegal instruction? */ }
OP(7A) { /* illegal instruction? */ }
OP(7B) { /* illegal instruction? */ }
OP(7C) { AD1_I16; wr16( addr1, regs.SP ); }
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

OP(B0) { PUSH8( regs.BA & 0x00FF ); }
OP(B1) { PUSH8( regs.BA >> 8 ); }
OP(B2) { PUSH8( regs.HL & 0x00FF ); }
OP(B3) { PUSH8( regs.HL >> 8 ); }
OP(B4) { regs.BA = ( regs.BA & 0xFF00 ) | POP8(); }
OP(B5) { regs.BA = ( regs.BA & 0x00FF ) | ( POP8() << 8 ); }
OP(B6) { regs.HL = ( regs.HL & 0xFF00 ) | POP8(); }
OP(B7) { regs.HL = ( regs.HL & 0x00FF ) | ( POP8() << 8 ); }
OP(B8) { PUSH16( regs.BA ); PUSH16( regs.HL ); PUSH16( regs.X ); PUSH16( regs.Y ); PUSH8( regs.N ); }
OP(B9) { PUSH16( regs.BA ); PUSH16( regs.HL ); PUSH16( regs.X ); PUSH16( regs.Y ); PUSH8( regs.N ); PUSH8( regs.I ); PUSH8( regs.XI ); PUSH8( regs.YI ); }
OP(BA) { /* illegal instruction? */ }
OP(BB) { /* illegal instruction? */ }
OP(BC) { regs.N = POP8(); regs.Y = POP16(); regs.X = POP16(); regs.HL = POP16(); regs.BA = POP16(); }
OP(BD) { regs.YI = POP8(); regs.XI = POP8(); regs.I = POP8(); regs.N = POP8(); regs.Y = POP16(); regs.X = POP16(); regs.HL = POP16(); regs.BA = POP16(); }
OP(BE) { /* illegal instruction? */ }
OP(BF) { /* illegal instruction? */ }

OP(C0) { AD2_IHL; regs.BA = rd16( addr2 ); }
OP(C1) { AD2_IHL; regs.HL = rd16( addr2 ); }
OP(C2) { AD2_IHL; regs.X = rd16( addr2 ); }
OP(C3) { AD2_IHL; regs.Y = rd16( addr2 ); }
OP(C4) { AD1_IHL; wr16( addr1, regs.BA ); }
OP(C5) { AD1_IHL; wr16( addr1, regs.HL ); }
OP(C6) { AD1_IHL; wr16( addr1, regs.X ); }
OP(C7) { AD1_IHL; wr16( addr1, regs.Y ); }
OP(C8) { /* illegal instruction? */ }
OP(C9) { /* illegal instruction? */ }
OP(CA) { /* illegal instruction? */ }
OP(CB) { /* illegal instruction? */ }
OP(CC) { /* illegal instruction? */ }
OP(CD) { /* illegal instruction? */ }
OP(CE) { /* illegal instruction? */ }
OP(CF) { /* illegal instruction? */ }

OP(D0) { AD2_XIX; regs.BA = rd16( addr2 ); }
OP(D1) { AD2_XIX; regs.HL = rd16( addr2 ); }
OP(D2) { AD2_XIX; regs.X = rd16( addr2 ); }
OP(D3) { AD2_XIX; regs.Y = rd16( addr2 ); }
OP(D4) { AD1_XIX; wr16( addr1, regs.BA ); }
OP(D5) { AD1_XIX; wr16( addr1, regs.HL ); }
OP(D6) { AD1_XIX; wr16( addr1, regs.X ); }
OP(D7) { AD1_XIX; wr16( addr1, regs.Y ); }
OP(D8) { AD2_YIY; regs.BA = rd16( addr2 ); }
OP(D9) { AD2_YIY; regs.HL = rd16( addr2 ); }
OP(DA) { AD2_YIY; regs.X = rd16( addr2 ); }
OP(DB) { AD2_YIY; regs.Y = rd16( addr2 ); }
OP(DC) { AD1_YIY; wr16( addr1, regs.BA ); }
OP(DD) { AD1_YIY; wr16( addr1, regs.HL ); }
OP(DE) { AD1_YIY; wr16( addr1, regs.X ); }
OP(DF) { AD1_YIY; wr16( addr1, regs.Y ); }

OP(E0) { regs.BA = regs.BA; }
OP(E1) { regs.BA = regs.HL; }
OP(E2) { regs.BA = regs.X; }
OP(E3) { regs.BA = regs.Y; }
OP(E4) { regs.HL = regs.BA; }
OP(E5) { regs.HL = regs.HL; }
OP(E6) { regs.HL = regs.X; }
OP(E7) { regs.HL = regs.Y; }
OP(E8) { regs.X = regs.BA; }
OP(E9) { regs.X = regs.HL; }
OP(EA) { regs.X = regs.X; }
OP(EB) { regs.X = regs.Y; }
OP(EC) { regs.Y = regs.BA; }
OP(ED) { regs.Y = regs.HL; }
OP(EE) { regs.Y = regs.X; }
OP(EF) { regs.Y = regs.Y; }

OP(F0) { regs.SP = regs.BA; }
OP(F1) { regs.SP = regs.HL; }
OP(F2) { regs.SP = regs.X; }
OP(F3) { regs.SP = regs.Y; }
OP(F4) { regs.HL = regs.SP; }
OP(F5) { regs.HL = regs.PC; }
OP(F6) { /* illegal instruction? */ }
OP(F7) { /* illegal instruction? */ }
OP(F8) { regs.BA = regs.SP; }
OP(F9) { regs.BA = regs.PC; }
OP(FA) { regs.X = regs.SP; }
OP(FB) { /* illegal instruction? */ }
OP(FC) { /* illegal instruction? */ }
OP(FD) { /* illegal instruction? */ }
OP(FE) { regs.Y = regs.SP; }
OP(FF) { /* illegal instruction? */ }

static void (*insnminx_CF[256])(void) = {
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

static int insnminx_cycles_CF[256] = {
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

