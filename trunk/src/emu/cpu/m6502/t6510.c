/*****************************************************************************
 *
 *   tbl6510.c
 *   6510 opcode functions and function pointer table
 *
 *   Copyright Juergen Buchmueller, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     pullmoll@t-online.de
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *   - Opcode information based on an Intel 386 '6510.asm' core
 *     written by R.F. van Ee (1993)
 *   - Cycle counts are guesswork :-)
 *
 *****************************************************************************/

#undef	OP
#define OP(nn) INLINE void m6510_##nn(m6502_Regs *cpustate)

/*****************************************************************************
 *****************************************************************************
 *
 *   plain vanilla 6502 opcodes
 *
 *****************************************************************************
 * op    temp     cycles             rdmem   opc  wrmem   ********************/

OP(00) {                  BRK;                 } /* 7 BRK */
OP(20) {                  JSR;                 } /* 6 JSR */
OP(40) {                  RTI;                 } /* 6 RTI */
OP(60) {                  RTS;                 } /* 6 RTS */
OP(80) { RDOPARG(); NOP;                 } /* 2 NOP IMM */
OP(a0) { int tmp; RD_IMM; LDY;                 } /* 2 LDY IMM */
OP(c0) { int tmp; RD_IMM; CPY;                 } /* 2 CPY IMM */
OP(e0) { int tmp; RD_IMM; CPX;                 } /* 2 CPX IMM */

OP(10) { BPL;                                  } /* 2-4 BPL REL */
OP(30) { BMI;                                  } /* 2-4 BMI REL */
OP(50) { BVC;                                  } /* 2-4 BVC REL */
OP(70) { BVS;                                  } /* 2-4 BVS REL */
OP(90) { BCC;                                  } /* 2-4 BCC REL */
OP(b0) { BCS;                                  } /* 2-4 BCS REL */
OP(d0) { BNE;                                  } /* 2-4 BNE REL */
OP(f0) { BEQ;                                  } /* 2-4 BEQ REL */

OP(01) { int tmp; RD_IDX; ORA;                 } /* 6 ORA IDX */
OP(21) { int tmp; RD_IDX; AND;                 } /* 6 AND IDX */
OP(41) { int tmp; RD_IDX; EOR;                 } /* 6 EOR IDX */
OP(61) { int tmp; RD_IDX; ADC;                 } /* 6 ADC IDX */
OP(81) { int tmp; STA; WR_IDX;                 } /* 6 STA IDX */
OP(a1) { int tmp; RD_IDX; LDA;                 } /* 6 LDA IDX */
OP(c1) { int tmp; RD_IDX; CMP;                 } /* 6 CMP IDX */
OP(e1) { int tmp; RD_IDX; SBC;                 } /* 6 SBC IDX */

OP(11) { int tmp; RD_IDY_P; ORA;               } /* 5 ORA IDY page penalty */
OP(31) { int tmp; RD_IDY_P; AND;               } /* 5 AND IDY page penalty */
OP(51) { int tmp; RD_IDY_P; EOR;               } /* 5 EOR IDY page penalty */
OP(71) { int tmp; RD_IDY_P; ADC;               } /* 5 ADC IDY page penalty */
OP(91) { int tmp; STA; WR_IDY_NP;              } /* 6 STA IDY */
OP(b1) { int tmp; RD_IDY_P; LDA;               } /* 5 LDA IDY page penalty */
OP(d1) { int tmp; RD_IDY_P; CMP;               } /* 5 CMP IDY page penalty */
OP(f1) { int tmp; RD_IDY_P; SBC;               } /* 5 SBC IDY page penalty */

OP(02) {                  KIL;                 } /* 1 KIL */
OP(22) {                  KIL;                 } /* 1 KIL */
OP(42) {                  KIL;                 } /* 1 KIL */
OP(62) {                  KIL;                 } /* 1 KIL */
OP(82) { RDOPARG(); NOP;                 } /* 2 NOP IMM */
OP(a2) { int tmp; RD_IMM; LDX;                 } /* 2 LDX IMM */
OP(c2) { RDOPARG(); NOP;                 } /* 2 NOP IMM */
OP(e2) { RDOPARG(); NOP;                 } /* 2 NOP IMM */

OP(12) { KIL;                                  } /* 1 KIL */
OP(32) { KIL;                                  } /* 1 KIL */
OP(52) { KIL;                                  } /* 1 KIL */
OP(72) { KIL;                                  } /* 1 KIL */
OP(92) { KIL;                                  } /* 1 KIL */
OP(b2) { KIL;                                  } /* 1 KIL */
OP(d2) { KIL;                                  } /* 1 KIL */
OP(f2) { KIL;                                  } /* 1 KIL */

OP(03) { int tmp; RD_IDX; WB_EA; SLO; WB_EA;   } /* 7 SLO IDX */
OP(23) { int tmp; RD_IDX; WB_EA; RLA; WB_EA;   } /* 7 RLA IDX */
OP(43) { int tmp; RD_IDX; WB_EA; SRE; WB_EA;   } /* 7 SRE IDX */
OP(63) { int tmp; RD_IDX; WB_EA; RRA; WB_EA;   } /* 7 RRA IDX */
OP(83) { int tmp;                SAX; WR_IDX;  } /* 6 SAX IDX */
OP(a3) { int tmp; RD_IDX; LAX;                 } /* 6 LAX IDX */
OP(c3) { int tmp; RD_IDX; WB_EA; DCP; WB_EA;   } /* 7 DCP IDX */
OP(e3) { int tmp; RD_IDX; WB_EA; ISB; WB_EA;   } /* 7 ISB IDX */

OP(13) { int tmp; RD_IDY_NP; WB_EA; SLO; WB_EA; } /* 7 SLO IDY */
OP(33) { int tmp; RD_IDY_NP; WB_EA; RLA; WB_EA; } /* 7 RLA IDY */
OP(53) { int tmp; RD_IDY_NP; WB_EA; SRE; WB_EA; } /* 7 SRE IDY */
OP(73) { int tmp; RD_IDY_NP; WB_EA; RRA; WB_EA; } /* 7 RRA IDY */
OP(93) { int tmp; EA_IDY_NP; SAH; WB_EA;        } /* 5 SAH IDY */
OP(b3) { int tmp; RD_IDY_P; LAX;                } /* 5 LAX IDY page penalty */
OP(d3) { int tmp; RD_IDY_NP; WB_EA; DCP; WB_EA; } /* 7 DCP IDY */
OP(f3) { int tmp; RD_IDY_NP; WB_EA; ISB; WB_EA; } /* 7 ISB IDY */

OP(04) { RD_ZPG_DISCARD; NOP;                  } /* 3 NOP ZPG */
OP(24) { int tmp; RD_ZPG; BIT;                  } /* 3 BIT ZPG */
OP(44) { RD_ZPG_DISCARD; NOP;                  } /* 3 NOP ZPG */
OP(64) { RD_ZPG_DISCARD; NOP;                  } /* 3 NOP ZPG */
OP(84) { int tmp; STY; WR_ZPG;                  } /* 3 STY ZPG */
OP(a4) { int tmp; RD_ZPG; LDY;                  } /* 3 LDY ZPG */
OP(c4) { int tmp; RD_ZPG; CPY;                  } /* 3 CPY ZPG */
OP(e4) { int tmp; RD_ZPG; CPX;                  } /* 3 CPX ZPG */

OP(14) { RD_ZPX_DISCARD; NOP;                  } /* 4 NOP ZPX */
OP(34) { RD_ZPX_DISCARD; NOP;                  } /* 4 NOP ZPX */
OP(54) { RD_ZPX_DISCARD; NOP;                  } /* 4 NOP ZPX */
OP(74) { RD_ZPX_DISCARD; NOP;                  } /* 4 NOP ZPX */
OP(94) { int tmp; STY; WR_ZPX;                  } /* 4 STY ZPX */
OP(b4) { int tmp; RD_ZPX; LDY;                  } /* 4 LDY ZPX */
OP(d4) { RD_ZPX_DISCARD; NOP;                  } /* 4 NOP ZPX */
OP(f4) { RD_ZPX_DISCARD; NOP;                  } /* 4 NOP ZPX */

OP(05) { int tmp; RD_ZPG; ORA;                  } /* 3 ORA ZPG */
OP(25) { int tmp; RD_ZPG; AND;                  } /* 3 AND ZPG */
OP(45) { int tmp; RD_ZPG; EOR;                  } /* 3 EOR ZPG */
OP(65) { int tmp; RD_ZPG; ADC;                  } /* 3 ADC ZPG */
OP(85) { int tmp; STA; WR_ZPG;                  } /* 3 STA ZPG */
OP(a5) { int tmp; RD_ZPG; LDA;                  } /* 3 LDA ZPG */
OP(c5) { int tmp; RD_ZPG; CMP;                  } /* 3 CMP ZPG */
OP(e5) { int tmp; RD_ZPG; SBC;                  } /* 3 SBC ZPG */

OP(15) { int tmp; RD_ZPX; ORA;                  } /* 4 ORA ZPX */
OP(35) { int tmp; RD_ZPX; AND;                  } /* 4 AND ZPX */
OP(55) { int tmp; RD_ZPX; EOR;                  } /* 4 EOR ZPX */
OP(75) { int tmp; RD_ZPX; ADC;                  } /* 4 ADC ZPX */
OP(95) { int tmp; STA; WR_ZPX;                  } /* 4 STA ZPX */
OP(b5) { int tmp; RD_ZPX; LDA;                  } /* 4 LDA ZPX */
OP(d5) { int tmp; RD_ZPX; CMP;                  } /* 4 CMP ZPX */
OP(f5) { int tmp; RD_ZPX; SBC;                  } /* 4 SBC ZPX */

OP(06) { int tmp; RD_ZPG; WB_EA; ASL; WB_EA;    } /* 5 ASL ZPG */
OP(26) { int tmp; RD_ZPG; WB_EA; ROL; WB_EA;    } /* 5 ROL ZPG */
OP(46) { int tmp; RD_ZPG; WB_EA; LSR; WB_EA;    } /* 5 LSR ZPG */
OP(66) { int tmp; RD_ZPG; WB_EA; ROR; WB_EA;    } /* 5 ROR ZPG */
OP(86) { int tmp; STX; WR_ZPG;                  } /* 3 STX ZPG */
OP(a6) { int tmp; RD_ZPG; LDX;                  } /* 3 LDX ZPG */
OP(c6) { int tmp; RD_ZPG; WB_EA; DEC; WB_EA;    } /* 5 DEC ZPG */
OP(e6) { int tmp; RD_ZPG; WB_EA; INC; WB_EA;    } /* 5 INC ZPG */

OP(16) { int tmp; RD_ZPX; WB_EA; ASL; WB_EA;    } /* 6 ASL ZPX */
OP(36) { int tmp; RD_ZPX; WB_EA; ROL; WB_EA;    } /* 6 ROL ZPX */
OP(56) { int tmp; RD_ZPX; WB_EA; LSR; WB_EA;    } /* 6 LSR ZPX */
OP(76) { int tmp; RD_ZPX; WB_EA; ROR; WB_EA;    } /* 6 ROR ZPX */
OP(96) { int tmp; STX; WR_ZPY;                  } /* 4 STX ZPY */
OP(b6) { int tmp; RD_ZPY; LDX;                  } /* 4 LDX ZPY */
OP(d6) { int tmp; RD_ZPX; WB_EA; DEC; WB_EA;    } /* 6 DEC ZPX */
OP(f6) { int tmp; RD_ZPX; WB_EA; INC; WB_EA;    } /* 6 INC ZPX */

OP(07) { int tmp; RD_ZPG; WB_EA; SLO; WB_EA;    } /* 5 SLO ZPG */
OP(27) { int tmp; RD_ZPG; WB_EA; RLA; WB_EA;    } /* 5 RLA ZPG */
OP(47) { int tmp; RD_ZPG; WB_EA; SRE; WB_EA;    } /* 5 SRE ZPG */
OP(67) { int tmp; RD_ZPG; WB_EA; RRA; WB_EA;    } /* 5 RRA ZPG */
OP(87) { int tmp; SAX; WR_ZPG;                  } /* 3 SAX ZPG */
OP(a7) { int tmp; RD_ZPG; LAX;                  } /* 3 LAX ZPG */
OP(c7) { int tmp; RD_ZPG; WB_EA; DCP; WB_EA;    } /* 5 DCP ZPG */
OP(e7) { int tmp; RD_ZPG; WB_EA; ISB; WB_EA;    } /* 5 ISB ZPG */

OP(17) { int tmp; RD_ZPX; WB_EA; SLO; WB_EA;    } /* 6 SLO ZPX */
OP(37) { int tmp; RD_ZPX; WB_EA; RLA; WB_EA;    } /* 6 RLA ZPX */
OP(57) { int tmp; RD_ZPX; WB_EA; SRE; WB_EA;    } /* 6 SRE ZPX */
OP(77) { int tmp; RD_ZPX; WB_EA; RRA; WB_EA;    } /* 6 RRA ZPX */
OP(97) { int tmp; SAX; WR_ZPY;                  } /* 4 SAX ZPY */
OP(b7) { int tmp; RD_ZPY; LAX;                  } /* 4 LAX ZPY */
OP(d7) { int tmp; RD_ZPX; WB_EA; DCP; WB_EA;    } /* 6 DCP ZPX */
OP(f7) { int tmp; RD_ZPX; WB_EA; ISB; WB_EA;    } /* 6 ISB ZPX */

OP(08) { RD_DUM; PHP;                           } /* 3 PHP */
OP(28) { RD_DUM; PLP;                           } /* 4 PLP */
OP(48) { RD_DUM; PHA;                           } /* 3 PHA */
OP(68) { RD_DUM; PLA;                           } /* 4 PLA */
OP(88) { RD_DUM; DEY;                           } /* 2 DEY */
OP(a8) { RD_DUM; TAY;                           } /* 2 TAY */
OP(c8) { RD_DUM; INY;                           } /* 2 INY */
OP(e8) { RD_DUM; INX;                           } /* 2 INX */

OP(18) { RD_DUM; CLC;                           } /* 2 CLC */
OP(38) { RD_DUM; SEC;                           } /* 2 SEC */
OP(58) { RD_DUM; CLI;                           } /* 2 CLI */
OP(78) { RD_DUM; SEI;                           } /* 2 SEI */
OP(98) { RD_DUM; TYA;                           } /* 2 TYA */
OP(b8) { RD_DUM; CLV;                           } /* 2 CLV */
OP(d8) { RD_DUM; CLD;                           } /* 2 CLD */
OP(f8) { RD_DUM; SED;                           } /* 2 SED */

OP(09) { int tmp; RD_IMM; ORA;                  } /* 2 ORA IMM */
OP(29) { int tmp; RD_IMM; AND;                  } /* 2 AND IMM */
OP(49) { int tmp; RD_IMM; EOR;                  } /* 2 EOR IMM */
OP(69) { int tmp; RD_IMM; ADC;                  } /* 2 ADC IMM */
OP(89) { RD_IMM_DISCARD; NOP;                  } /* 2 NOP IMM */
OP(a9) { int tmp; RD_IMM; LDA;                  } /* 2 LDA IMM */
OP(c9) { int tmp; RD_IMM; CMP;                  } /* 2 CMP IMM */
OP(e9) { int tmp; RD_IMM; SBC;                  } /* 2 SBC IMM */

OP(19) { int tmp; RD_ABY_P; ORA;                } /* 4 ORA ABY page penalty */
OP(39) { int tmp; RD_ABY_P; AND;                } /* 4 AND ABY page penalty */
OP(59) { int tmp; RD_ABY_P; EOR;                } /* 4 EOR ABY page penalty */
OP(79) { int tmp; RD_ABY_P; ADC;                } /* 4 ADC ABY page penalty */
OP(99) { int tmp; STA; WR_ABY_NP;               } /* 5 STA ABY */
OP(b9) { int tmp; RD_ABY_P; LDA;                } /* 4 LDA ABY page penalty */
OP(d9) { int tmp; RD_ABY_P; CMP;                } /* 4 CMP ABY page penalty */
OP(f9) { int tmp; RD_ABY_P; SBC;                } /* 4 SBC ABY page penalty */

OP(0a) { int tmp; RD_DUM; RD_ACC; ASL; WB_ACC;  } /* 2 ASL A */
OP(2a) { int tmp; RD_DUM; RD_ACC; ROL; WB_ACC;  } /* 2 ROL A */
OP(4a) { int tmp; RD_DUM; RD_ACC; LSR; WB_ACC;  } /* 2 LSR A */
OP(6a) { int tmp; RD_DUM; RD_ACC; ROR; WB_ACC;  } /* 2 ROR A */
OP(8a) { RD_DUM; TXA;                           } /* 2 TXA */
OP(aa) { RD_DUM; TAX;                           } /* 2 TAX */
OP(ca) { RD_DUM; DEX;                           } /* 2 DEX */
OP(ea) { RD_DUM; NOP;                           } /* 2 NOP */

OP(1a) { RD_DUM; NOP;                           } /* 2 NOP */
OP(3a) { RD_DUM; NOP;                           } /* 2 NOP */
OP(5a) { RD_DUM; NOP;                           } /* 2 NOP */
OP(7a) { RD_DUM; NOP;                           } /* 2 NOP */
OP(9a) { RD_DUM; TXS;                           } /* 2 TXS */
OP(ba) { RD_DUM; TSX;                           } /* 2 TSX */
OP(da) { RD_DUM; NOP;                           } /* 2 NOP */
OP(fa) { RD_DUM; NOP;                           } /* 2 NOP */

OP(0b) { int tmp; RD_IMM; ANC;                  } /* 2 ANC IMM */
OP(2b) { int tmp; RD_IMM; ANC;                  } /* 2 ANC IMM */
OP(4b) { int tmp; RD_IMM; ASR; WB_ACC;          } /* 2 ASR IMM */
OP(6b) { int tmp; RD_IMM; ARR; WB_ACC;          } /* 2 ARR IMM */
OP(8b) { int tmp; RD_IMM; AXA;                  } /* 2 AXA IMM */
OP(ab) { int tmp; RD_IMM; OAL_6510;             } /* 2 OAL IMM */
OP(cb) { int tmp; RD_IMM; ASX;                  } /* 2 ASX IMM */
OP(eb) { int tmp; RD_IMM; SBC;                  } /* 2 SBC IMM */

OP(1b) { int tmp; RD_ABY_NP; WB_EA; SLO; WB_EA; } /* 7 SLO ABY */
OP(3b) { int tmp; RD_ABY_NP; WB_EA; RLA; WB_EA; } /* 7 RLA ABY */
OP(5b) { int tmp; RD_ABY_NP; WB_EA; SRE; WB_EA; } /* 7 SRE ABY */
OP(7b) { int tmp; RD_ABY_NP; WB_EA; RRA; WB_EA; } /* 7 RRA ABY */
OP(9b) { int tmp; EA_ABY_NP; SSH; WB_EA;        } /* 5 SSH ABY */
OP(bb) { int tmp; RD_ABY_P; AST;                } /* 4 AST ABY page penalty */
OP(db) { int tmp; RD_ABY_NP; WB_EA; DCP; WB_EA; } /* 7 DCP ABY */
OP(fb) { int tmp; RD_ABY_NP; WB_EA; ISB; WB_EA; } /* 7 ISB ABY */

OP(0c) { RD_ABS_DISCARD; NOP;                  } /* 4 NOP ABS */
OP(2c) { int tmp; RD_ABS; BIT;                  } /* 4 BIT ABS */
OP(4c) { EA_ABS; JMP;                           } /* 3 JMP ABS */
OP(6c) { int tmp; EA_IND; JMP;                  } /* 5 JMP IND */
OP(8c) { int tmp; STY; WR_ABS;                  } /* 4 STY ABS */
OP(ac) { int tmp; RD_ABS; LDY;                  } /* 4 LDY ABS */
OP(cc) { int tmp; RD_ABS; CPY;                  } /* 4 CPY ABS */
OP(ec) { int tmp; RD_ABS; CPX;                  } /* 4 CPX ABS */

OP(1c) { RD_ABX_P_DISCARD; NOP;                } /* 4 NOP ABX page penalty */
OP(3c) { RD_ABX_P_DISCARD; NOP;                } /* 4 NOP ABX page penalty */
OP(5c) { RD_ABX_P_DISCARD; NOP;                } /* 4 NOP ABX page penalty */
OP(7c) { RD_ABX_P_DISCARD; NOP;                } /* 4 NOP ABX page penalty */
OP(9c) { int tmp; EA_ABX_NP; SYH; WB_EA;        } /* 5 SYH ABX */
OP(bc) { int tmp; RD_ABX_P; LDY;                } /* 4 LDY ABX page penalty */
OP(dc) { RD_ABX_P_DISCARD; NOP;                } /* 4 NOP ABX page penalty */
OP(fc) { RD_ABX_P_DISCARD; NOP;                } /* 4 NOP ABX page penalty */

OP(0d) { int tmp; RD_ABS; ORA;                  } /* 4 ORA ABS */
OP(2d) { int tmp; RD_ABS; AND;                  } /* 4 AND ABS */
OP(4d) { int tmp; RD_ABS; EOR;                  } /* 4 EOR ABS */
OP(6d) { int tmp; RD_ABS; ADC;                  } /* 4 ADC ABS */
OP(8d) { int tmp; STA; WR_ABS;                  } /* 4 STA ABS */
OP(ad) { int tmp; RD_ABS; LDA;                  } /* 4 LDA ABS */
OP(cd) { int tmp; RD_ABS; CMP;                  } /* 4 CMP ABS */
OP(ed) { int tmp; RD_ABS; SBC;                  } /* 4 SBC ABS */

OP(1d) { int tmp; RD_ABX_P; ORA;                } /* 4 ORA ABX page penalty */
OP(3d) { int tmp; RD_ABX_P; AND;                } /* 4 AND ABX page penalty */
OP(5d) { int tmp; RD_ABX_P; EOR;                } /* 4 EOR ABX page penalty */
OP(7d) { int tmp; RD_ABX_P; ADC;                } /* 4 ADC ABX page penalty */
OP(9d) { int tmp; STA; WR_ABX_NP;               } /* 5 STA ABX */
OP(bd) { int tmp; RD_ABX_P; LDA;                } /* 4 LDA ABX page penalty */
OP(dd) { int tmp; RD_ABX_P; CMP;                } /* 4 CMP ABX page penalty */
OP(fd) { int tmp; RD_ABX_P; SBC;                } /* 4 SBC ABX page penalty */

OP(0e) { int tmp; RD_ABS; WB_EA; ASL; WB_EA;    } /* 6 ASL ABS */
OP(2e) { int tmp; RD_ABS; WB_EA; ROL; WB_EA;    } /* 6 ROL ABS */
OP(4e) { int tmp; RD_ABS; WB_EA; LSR; WB_EA;    } /* 6 LSR ABS */
OP(6e) { int tmp; RD_ABS; WB_EA; ROR; WB_EA;    } /* 6 ROR ABS */
OP(8e) { int tmp; STX; WR_ABS;                  } /* 4 STX ABS */
OP(ae) { int tmp; RD_ABS; LDX;                  } /* 4 LDX ABS */
OP(ce) { int tmp; RD_ABS; WB_EA; DEC; WB_EA;    } /* 6 DEC ABS */
OP(ee) { int tmp; RD_ABS; WB_EA; INC; WB_EA;    } /* 6 INC ABS */

OP(1e) { int tmp; RD_ABX_NP; WB_EA; ASL; WB_EA; } /* 7 ASL ABX */
OP(3e) { int tmp; RD_ABX_NP; WB_EA; ROL; WB_EA; } /* 7 ROL ABX */
OP(5e) { int tmp; RD_ABX_NP; WB_EA; LSR; WB_EA; } /* 7 LSR ABX */
OP(7e) { int tmp; RD_ABX_NP; WB_EA; ROR; WB_EA; } /* 7 ROR ABX */
OP(9e) { int tmp; EA_ABY_NP; SXH; WB_EA;	} /* 5 SXH ABY */
OP(be) { int tmp; RD_ABY_P; LDX;		} /* 4 LDX ABY page penalty */
OP(de) { int tmp; RD_ABX_NP; WB_EA; DEC; WB_EA; } /* 7 DEC ABX */
OP(fe) { int tmp; RD_ABX_NP; WB_EA; INC; WB_EA; } /* 7 INC ABX */

OP(0f) { int tmp; RD_ABS; WB_EA; SLO; WB_EA;    } /* 6 SLO ABS */
OP(2f) { int tmp; RD_ABS; WB_EA; RLA; WB_EA;    } /* 6 RLA ABS */
OP(4f) { int tmp; RD_ABS; WB_EA; SRE; WB_EA;    } /* 6 SRE ABS */
OP(6f) { int tmp; RD_ABS; WB_EA; RRA; WB_EA;    } /* 6 RRA ABS */
OP(8f) { int tmp; SAX; WR_ABS;                  } /* 4 SAX ABS */
OP(af) { int tmp; RD_ABS; LAX;                  } /* 4 LAX ABS */
OP(cf) { int tmp; RD_ABS; WB_EA; DCP; WB_EA;    } /* 6 DCP ABS */
OP(ef) { int tmp; RD_ABS; WB_EA; ISB; WB_EA;    } /* 6 ISB ABS */

OP(1f) { int tmp; RD_ABX_NP; WB_EA; SLO; WB_EA; } /* 7 SLO ABX */
OP(3f) { int tmp; RD_ABX_NP; WB_EA; RLA; WB_EA; } /* 7 RLA ABX */
OP(5f) { int tmp; RD_ABX_NP; WB_EA; SRE; WB_EA; } /* 7 SRE ABX */
OP(7f) { int tmp; RD_ABX_NP; WB_EA; RRA; WB_EA; } /* 7 RRA ABX */
OP(9f) { int tmp; EA_ABY_NP; SAH; WB_EA;        } /* 5 SAH ABY */
OP(bf) { int tmp; RD_ABY_P; LAX;                } /* 4 LAX ABY page penalty */
OP(df) { int tmp; RD_ABX_NP; WB_EA; DCP; WB_EA; } /* 7 DCP ABX */
OP(ff) { int tmp; RD_ABX_NP; WB_EA; ISB; WB_EA; } /* 7 ISB ABX */

/* and here's the array of function pointers */

static void (*const insn6510[0x100])(m6502_Regs *cpustate) = {
	m6510_00,m6510_01,m6510_02,m6510_03,m6510_04,m6510_05,m6510_06,m6510_07,
	m6510_08,m6510_09,m6510_0a,m6510_0b,m6510_0c,m6510_0d,m6510_0e,m6510_0f,
	m6510_10,m6510_11,m6510_12,m6510_13,m6510_14,m6510_15,m6510_16,m6510_17,
	m6510_18,m6510_19,m6510_1a,m6510_1b,m6510_1c,m6510_1d,m6510_1e,m6510_1f,
	m6510_20,m6510_21,m6510_22,m6510_23,m6510_24,m6510_25,m6510_26,m6510_27,
	m6510_28,m6510_29,m6510_2a,m6510_2b,m6510_2c,m6510_2d,m6510_2e,m6510_2f,
	m6510_30,m6510_31,m6510_32,m6510_33,m6510_34,m6510_35,m6510_36,m6510_37,
	m6510_38,m6510_39,m6510_3a,m6510_3b,m6510_3c,m6510_3d,m6510_3e,m6510_3f,
	m6510_40,m6510_41,m6510_42,m6510_43,m6510_44,m6510_45,m6510_46,m6510_47,
	m6510_48,m6510_49,m6510_4a,m6510_4b,m6510_4c,m6510_4d,m6510_4e,m6510_4f,
	m6510_50,m6510_51,m6510_52,m6510_53,m6510_54,m6510_55,m6510_56,m6510_57,
	m6510_58,m6510_59,m6510_5a,m6510_5b,m6510_5c,m6510_5d,m6510_5e,m6510_5f,
	m6510_60,m6510_61,m6510_62,m6510_63,m6510_64,m6510_65,m6510_66,m6510_67,
	m6510_68,m6510_69,m6510_6a,m6510_6b,m6510_6c,m6510_6d,m6510_6e,m6510_6f,
	m6510_70,m6510_71,m6510_72,m6510_73,m6510_74,m6510_75,m6510_76,m6510_77,
	m6510_78,m6510_79,m6510_7a,m6510_7b,m6510_7c,m6510_7d,m6510_7e,m6510_7f,
	m6510_80,m6510_81,m6510_82,m6510_83,m6510_84,m6510_85,m6510_86,m6510_87,
	m6510_88,m6510_89,m6510_8a,m6510_8b,m6510_8c,m6510_8d,m6510_8e,m6510_8f,
	m6510_90,m6510_91,m6510_92,m6510_93,m6510_94,m6510_95,m6510_96,m6510_97,
	m6510_98,m6510_99,m6510_9a,m6510_9b,m6510_9c,m6510_9d,m6510_9e,m6510_9f,
	m6510_a0,m6510_a1,m6510_a2,m6510_a3,m6510_a4,m6510_a5,m6510_a6,m6510_a7,
	m6510_a8,m6510_a9,m6510_aa,m6510_ab,m6510_ac,m6510_ad,m6510_ae,m6510_af,
	m6510_b0,m6510_b1,m6510_b2,m6510_b3,m6510_b4,m6510_b5,m6510_b6,m6510_b7,
	m6510_b8,m6510_b9,m6510_ba,m6510_bb,m6510_bc,m6510_bd,m6510_be,m6510_bf,
	m6510_c0,m6510_c1,m6510_c2,m6510_c3,m6510_c4,m6510_c5,m6510_c6,m6510_c7,
	m6510_c8,m6510_c9,m6510_ca,m6510_cb,m6510_cc,m6510_cd,m6510_ce,m6510_cf,
	m6510_d0,m6510_d1,m6510_d2,m6510_d3,m6510_d4,m6510_d5,m6510_d6,m6510_d7,
	m6510_d8,m6510_d9,m6510_da,m6510_db,m6510_dc,m6510_dd,m6510_de,m6510_df,
	m6510_e0,m6510_e1,m6510_e2,m6510_e3,m6510_e4,m6510_e5,m6510_e6,m6510_e7,
	m6510_e8,m6510_e9,m6510_ea,m6510_eb,m6510_ec,m6510_ed,m6510_ee,m6510_ef,
	m6510_f0,m6510_f1,m6510_f2,m6510_f3,m6510_f4,m6510_f5,m6510_f6,m6510_f7,
	m6510_f8,m6510_f9,m6510_fa,m6510_fb,m6510_fc,m6510_fd,m6510_fe,m6510_ff
};

