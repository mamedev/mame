// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*** t11: Portable DEC T-11 emulator ******************************************

    Actual opcode implementation.  Excuse the excessive use of macros, it
    was the only way I could bear to type all this in!

*****************************************************************************/


/* given a register index 'r', this computes the effective address for a byte-sized operation
   and puts the result in 'ea' */
#define MAKE_EAB_RGD(r) ea = REGD(r)
#define MAKE_EAB_IN(r)  ea = REGD(r); REGW(r) += ((r) < 6 ? 1 : 2)
#define MAKE_EAB_INS(r) ea = REGD(r); REGW(r) += ((r) < 6 ? 1 : 2)
#define MAKE_EAB_IND(r) ea = REGD(r); REGW(r) += 2; ea = RWORD(ea)
#define MAKE_EAB_DE(r)  REGW(r) -= ((r) < 6 ? 1 : 2); ea = REGD(r)
#define MAKE_EAB_DED(r) REGW(r) -= 2; ea = REGD(r); ea = RWORD(ea)
#define MAKE_EAB_IX(r)  ea = ROPCODE(); ea = (ea + REGD(r)) & 0xffff
#define MAKE_EAB_IXD(r) ea = ROPCODE(); ea = (ea + REGD(r)) & 0xffff; ea = RWORD(ea)

/* given a register index 'r', this computes the effective address for a word-sized operation
   and puts the result in 'ea' */
/* note that word accesses ignore the low bit!! this fixes APB! */
#define MAKE_EAW_RGD(r) MAKE_EAB_RGD(r)
#define MAKE_EAW_IN(r)  ea = REGD(r); REGW(r) += 2
#define MAKE_EAW_IND(r) MAKE_EAB_IND(r)
#define MAKE_EAW_DE(r)  REGW(r) -= 2; ea = REGD(r)
#define MAKE_EAW_DED(r) MAKE_EAB_DED(r)
#define MAKE_EAW_IX(r)  MAKE_EAB_IX(r)
#define MAKE_EAW_IXD(r) MAKE_EAB_IXD(r)

/* extracts the source/destination register index from the opcode into 'sreg' or 'dreg' */
#define GET_SREG sreg = (op >> 6) & 7
#define GET_DREG dreg = op & 7

/* for a byte-sized source operand: extracts 'sreg', computes 'ea', and loads the value into 'source' */
#define GET_SB_RG  GET_SREG; source = REGB(sreg)
#define GET_SB_RGD GET_SREG; MAKE_EAB_RGD(sreg); source = RBYTE(ea)
#define GET_SB_IN  GET_SREG; if (sreg == 7) { source = ROPCODE(); } else { MAKE_EAB_IN(sreg); source = RBYTE(ea); }
#define GET_SB_IND GET_SREG; if (sreg == 7) { ea = ROPCODE(); } else { MAKE_EAB_IND(sreg); } source = RBYTE(ea)
#define GET_SB_DE  GET_SREG; MAKE_EAB_DE(sreg); source = RBYTE(ea)
#define GET_SB_DED GET_SREG; MAKE_EAB_DED(sreg); source = RBYTE(ea)
#define GET_SB_IX  GET_SREG; MAKE_EAB_IX(sreg); source = RBYTE(ea)
#define GET_SB_IXD GET_SREG; MAKE_EAB_IXD(sreg); source = RBYTE(ea)

/* for a word-sized source operand: extracts 'sreg', computes 'ea', and loads the value into 'source' */
#define GET_SW_RG  GET_SREG; source = REGD(sreg)
#define GET_SW_RGD GET_SREG; MAKE_EAW_RGD(sreg); source = RWORD(ea)
#define GET_SW_IN  GET_SREG; if (sreg == 7) { source = ROPCODE(); } else { MAKE_EAW_IN(sreg); source = RWORD(ea); }
#define GET_SW_IND GET_SREG; if (sreg == 7) { ea = ROPCODE(); } else { MAKE_EAW_IND(sreg); } source = RWORD(ea)
#define GET_SW_DE  GET_SREG; MAKE_EAW_DE(sreg); source = RWORD(ea)
#define GET_SW_DED GET_SREG; MAKE_EAW_DED(sreg); source = RWORD(ea)
#define GET_SW_IX  GET_SREG; MAKE_EAW_IX(sreg); source = RWORD(ea)
#define GET_SW_IXD GET_SREG; MAKE_EAW_IXD(sreg); source = RWORD(ea)

/* for a byte-sized destination operand: extracts 'dreg', computes 'ea', and loads the value into 'dest' */
#define GET_DB_RG  GET_DREG; dest = REGB(dreg)
#define GET_DB_RGD GET_DREG; MAKE_EAB_RGD(dreg); dest = RBYTE(ea)
#define GET_DB_IN  GET_DREG; MAKE_EAB_IN(dreg); dest = RBYTE(ea)
#define GET_DB_IND GET_DREG; if (dreg == 7) { ea = ROPCODE(); } else { MAKE_EAB_IND(dreg); } dest = RBYTE(ea)
#define GET_DB_DE  GET_DREG; MAKE_EAB_DE(dreg); dest = RBYTE(ea)
#define GET_DB_DED GET_DREG; MAKE_EAB_DED(dreg); dest = RBYTE(ea)
#define GET_DB_IX  GET_DREG; MAKE_EAB_IX(dreg); dest = RBYTE(ea)
#define GET_DB_IXD GET_DREG; MAKE_EAB_IXD(dreg); dest = RBYTE(ea)

/* for a word-sized destination operand: extracts 'dreg', computes 'ea', and loads the value into 'dest' */
#define GET_DW_RG  GET_DREG; dest = REGD(dreg)
#define GET_DW_RGD GET_DREG; MAKE_EAW_RGD(dreg); dest = RWORD(ea)
#define GET_DW_IN  GET_DREG; MAKE_EAW_IN(dreg); dest = RWORD(ea)
#define GET_DW_IND GET_DREG; if (dreg == 7) { ea = ROPCODE(); } else { MAKE_EAW_IND(dreg); } dest = RWORD(ea)
#define GET_DW_DE  GET_DREG; MAKE_EAW_DE(dreg); dest = RWORD(ea)
#define GET_DW_DED GET_DREG; MAKE_EAW_DED(dreg); dest = RWORD(ea)
#define GET_DW_IX  GET_DREG; MAKE_EAW_IX(dreg); dest = RWORD(ea)
#define GET_DW_IXD GET_DREG; MAKE_EAW_IXD(dreg); dest = RWORD(ea)

/* writes a value to a previously computed 'ea' */
#define PUT_DB_EA(v) WBYTE(ea, (v))
#define PUT_DW_EA(v) WWORD(ea, (v))

/* writes a value to a previously computed 'dreg' register */
#define PUT_DB_DREG(v) REGB(dreg) = (v)
#define PUT_DW_DREG(v) REGW(dreg) = (v)

/* for a byte-sized destination operand: extracts 'dreg', computes 'ea', and writes 'v' to it */
#define PUT_DB_RG(v)  GET_DREG; REGB(dreg) = (v)
#define PUT_DB_RGD(v) GET_DREG; MAKE_EAB_RGD(dreg); WBYTE(ea, (v))
#define PUT_DB_IN(v)  GET_DREG; MAKE_EAB_IN(dreg); WBYTE(ea, (v))
#define PUT_DB_IND(v) GET_DREG; if (dreg == 7) { ea = ROPCODE(); } else { MAKE_EAB_IND(dreg); } WBYTE(ea, (v))
#define PUT_DB_DE(v)  GET_DREG; MAKE_EAB_DE(dreg); WBYTE(ea, (v))
#define PUT_DB_DED(v) GET_DREG; MAKE_EAB_DED(dreg); WBYTE(ea, (v))
#define PUT_DB_IX(v)  GET_DREG; MAKE_EAB_IX(dreg); WBYTE(ea, (v))
#define PUT_DB_IXD(v) GET_DREG; MAKE_EAB_IXD(dreg); WBYTE(ea, (v))

/* for a word-sized destination operand: extracts 'dreg', computes 'ea', and writes 'v' to it */
#define PUT_DW_RG(v)  GET_DREG; REGW(dreg) = (v)
#define PUT_DW_RGD(v) GET_DREG; MAKE_EAW_RGD(dreg); WWORD(ea, (v))
#define PUT_DW_IN(v)  GET_DREG; MAKE_EAW_IN(dreg); WWORD(ea, (v))
#define PUT_DW_IND(v) GET_DREG; if (dreg == 7) { ea = ROPCODE(); } else { MAKE_EAW_IND(dreg); } WWORD(ea, (v))
#define PUT_DW_DE(v)  GET_DREG; MAKE_EAW_DE(dreg); WWORD(ea, (v))
#define PUT_DW_DED(v) GET_DREG; MAKE_EAW_DED(dreg); WWORD(ea, (v))
#define PUT_DW_IX(v)  GET_DREG; MAKE_EAW_IX(dreg); WWORD(ea, (v))
#define PUT_DW_IXD(v) GET_DREG; MAKE_EAW_IXD(dreg); WWORD(ea, (v))

/* flag clearing; must be done before setting */
#define CLR_ZV   (PSW &= ~(ZFLAG | VFLAG))
#define CLR_NZV  (PSW &= ~(NFLAG | ZFLAG | VFLAG))
#define CLR_NZVC (PSW &= ~(NFLAG | ZFLAG | VFLAG | CFLAG))

/* set individual flags byte-sized */
#define SETB_N (PSW |= (result >> 4) & 0x08)
#define SETB_Z (PSW |= ((result & 0xff) == 0) << 2)
#define SETB_V (PSW |= ((source ^ dest ^ result ^ (result >> 1)) >> 6) & 0x02)
#define SETB_C (PSW |= (result >> 8) & 0x01)
#define SETB_NZ SETB_N; SETB_Z
#define SETB_NZV SETB_N; SETB_Z; SETB_V
#define SETB_NZVC SETB_N; SETB_Z; SETB_V; SETB_C

/* set individual flags word-sized */
#define SETW_N (PSW |= (result >> 12) & 0x08)
#define SETW_Z (PSW |= ((result & 0xffff) == 0) << 2)
#define SETW_V (PSW |= ((source ^ dest ^ result ^ (result >> 1)) >> 14) & 0x02)
#define SETW_C (PSW |= (result >> 16) & 0x01)
#define SETW_NZ SETW_N; SETW_Z
#define SETW_NZV SETW_N; SETW_Z; SETW_V
#define SETW_NZVC SETW_N; SETW_Z; SETW_V; SETW_C

/* operations */
/* ADC: dst += C */
#define ADC_R(d)    int dreg, source, dest, result;     source = GET_C; GET_DW_##d; CLR_NZVC; result = dest + source; SETW_NZVC; PUT_DW_DREG(result)
#define ADC_M(d)    int dreg, source, dest, result, ea; source = GET_C; GET_DW_##d; CLR_NZVC; result = dest + source; SETW_NZVC; PUT_DW_EA(result)
#define ADCB_R(d)   int dreg, source, dest, result;     source = GET_C; GET_DB_##d; CLR_NZVC; result = dest + source; SETB_NZVC; PUT_DB_DREG(result)
#define ADCB_M(d)   int dreg, source, dest, result, ea; source = GET_C; GET_DB_##d; CLR_NZVC; result = dest + source; SETB_NZVC; PUT_DB_EA(result)
/* ADD: dst += src */
#define ADD_R(s,d)  int sreg, dreg, source, dest, result;     GET_SW_##s; GET_DW_##d; CLR_NZVC; result = dest + source; SETW_NZVC; PUT_DW_DREG(result)
#define ADD_X(s,d)  int sreg, dreg, source, dest, result, ea; GET_SW_##s; GET_DW_##d; CLR_NZVC; result = dest + source; SETW_NZVC; PUT_DW_DREG(result)
#define ADD_M(s,d)  int sreg, dreg, source, dest, result, ea; GET_SW_##s; GET_DW_##d; CLR_NZVC; result = dest + source; SETW_NZVC; PUT_DW_EA(result)
/* ASL: dst = (dst << 1); C = (dst >> 7) */
#define ASL_R(d)    int dreg, dest, result;     GET_DW_##d; CLR_NZVC; result = dest << 1; SETW_NZ; PSW |= (dest >> 15) & 1; PSW |= ((PSW << 1) ^ (PSW >> 2)) & 2; PUT_DW_DREG(result)
#define ASL_M(d)    int dreg, dest, result, ea; GET_DW_##d; CLR_NZVC; result = dest << 1; SETW_NZ; PSW |= (dest >> 15) & 1; PSW |= ((PSW << 1) ^ (PSW >> 2)) & 2; PUT_DW_EA(result)
#define ASLB_R(d)   int dreg, dest, result;     GET_DB_##d; CLR_NZVC; result = dest << 1; SETB_NZ; PSW |= (dest >> 7) & 1;  PSW |= ((PSW << 1) ^ (PSW >> 2)) & 2; PUT_DB_DREG(result)
#define ASLB_M(d)   int dreg, dest, result, ea; GET_DB_##d; CLR_NZVC; result = dest << 1; SETB_NZ; PSW |= (dest >> 7) & 1;  PSW |= ((PSW << 1) ^ (PSW >> 2)) & 2; PUT_DB_EA(result)
/* ASR: dst = (dst << 1); C = (dst >> 7) */
#define ASR_R(d)    int dreg, dest, result;     GET_DW_##d; CLR_NZVC; result = (dest >> 1) | (dest & 0x8000); SETW_NZ; PSW |= dest & 1; PSW |= ((PSW << 1) ^ (PSW >> 2)) & 2; PUT_DW_DREG(result)
#define ASR_M(d)    int dreg, dest, result, ea; GET_DW_##d; CLR_NZVC; result = (dest >> 1) | (dest & 0x8000); SETW_NZ; PSW |= dest & 1; PSW |= ((PSW << 1) ^ (PSW >> 2)) & 2; PUT_DW_EA(result)
#define ASRB_R(d)   int dreg, dest, result;     GET_DB_##d; CLR_NZVC; result = (dest >> 1) | (dest & 0x80);   SETB_NZ; PSW |= dest & 1; PSW |= ((PSW << 1) ^ (PSW >> 2)) & 2; PUT_DB_DREG(result)
#define ASRB_M(d)   int dreg, dest, result, ea; GET_DB_##d; CLR_NZVC; result = (dest >> 1) | (dest & 0x80);   SETB_NZ; PSW |= dest & 1; PSW |= ((PSW << 1) ^ (PSW >> 2)) & 2; PUT_DB_EA(result)
/* BIC: dst &= ~src */
#define BIC_R(s,d)  int sreg, dreg, source, dest, result;     GET_SW_##s; GET_DW_##d; CLR_NZV; result = dest & ~source; SETW_NZ; PUT_DW_DREG(result)
#define BIC_X(s,d)  int sreg, dreg, source, dest, result, ea; GET_SW_##s; GET_DW_##d; CLR_NZV; result = dest & ~source; SETW_NZ; PUT_DW_DREG(result)
#define BIC_M(s,d)  int sreg, dreg, source, dest, result, ea; GET_SW_##s; GET_DW_##d; CLR_NZV; result = dest & ~source; SETW_NZ; PUT_DW_EA(result)
#define BICB_R(s,d) int sreg, dreg, source, dest, result;     GET_SB_##s; GET_DB_##d; CLR_NZV; result = dest & ~source; SETB_NZ; PUT_DB_DREG(result)
#define BICB_X(s,d) int sreg, dreg, source, dest, result, ea; GET_SB_##s; GET_DB_##d; CLR_NZV; result = dest & ~source; SETB_NZ; PUT_DB_DREG(result)
#define BICB_M(s,d) int sreg, dreg, source, dest, result, ea; GET_SB_##s; GET_DB_##d; CLR_NZV; result = dest & ~source; SETB_NZ; PUT_DB_EA(result)
/* BIS: dst |= src */
#define BIS_R(s,d)  int sreg, dreg, source, dest, result;     GET_SW_##s; GET_DW_##d; CLR_NZV; result = dest | source; SETW_NZ; PUT_DW_DREG(result)
#define BIS_X(s,d)  int sreg, dreg, source, dest, result, ea; GET_SW_##s; GET_DW_##d; CLR_NZV; result = dest | source; SETW_NZ; PUT_DW_DREG(result)
#define BIS_M(s,d)  int sreg, dreg, source, dest, result, ea; GET_SW_##s; GET_DW_##d; CLR_NZV; result = dest | source; SETW_NZ; PUT_DW_EA(result)
#define BISB_R(s,d) int sreg, dreg, source, dest, result;     GET_SB_##s; GET_DB_##d; CLR_NZV; result = dest | source; SETB_NZ; PUT_DB_DREG(result)
#define BISB_X(s,d) int sreg, dreg, source, dest, result, ea; GET_SB_##s; GET_DB_##d; CLR_NZV; result = dest | source; SETB_NZ; PUT_DB_DREG(result)
#define BISB_M(s,d) int sreg, dreg, source, dest, result, ea; GET_SB_##s; GET_DB_##d; CLR_NZV; result = dest | source; SETB_NZ; PUT_DB_EA(result)
/* BIT: flags = dst & src */
#define BIT_R(s,d)  int sreg, dreg, source, dest, result;     GET_SW_##s; GET_DW_##d; CLR_NZV; result = dest & source; SETW_NZ;
#define BIT_M(s,d)  int sreg, dreg, source, dest, result, ea; GET_SW_##s; GET_DW_##d; CLR_NZV; result = dest & source; SETW_NZ;
#define BITB_R(s,d) int sreg, dreg, source, dest, result;     GET_SB_##s; GET_DB_##d; CLR_NZV; result = dest & source; SETB_NZ;
#define BITB_M(s,d) int sreg, dreg, source, dest, result, ea; GET_SB_##s; GET_DB_##d; CLR_NZV; result = dest & source; SETB_NZ;
/* BR: if (condition) branch */
#define BR(c)       if (c) { PC += 2 * (signed char)(op & 0xff); }
/* CLR: dst = 0 */
#define CLR_R(d)    int dreg;     PUT_DW_##d(0); CLR_NZVC; SET_Z
#define CLR_M(d)    int dreg, ea; PUT_DW_##d(0); CLR_NZVC; SET_Z
#define CLRB_R(d)   int dreg;     PUT_DB_##d(0); CLR_NZVC; SET_Z
#define CLRB_M(d)   int dreg, ea; PUT_DB_##d(0); CLR_NZVC; SET_Z
/* CMP: flags = src - dst */
#define CMP_R(s,d)  int sreg, dreg, source, dest, result;     GET_SW_##s; GET_DW_##d; CLR_NZVC; result = source - dest; SETW_NZVC;
#define CMP_M(s,d)  int sreg, dreg, source, dest, result, ea; GET_SW_##s; GET_DW_##d; CLR_NZVC; result = source - dest; SETW_NZVC;
#define CMPB_R(s,d) int sreg, dreg, source, dest, result;     GET_SB_##s; GET_DB_##d; CLR_NZVC; result = source - dest; SETB_NZVC;
#define CMPB_M(s,d) int sreg, dreg, source, dest, result, ea; GET_SB_##s; GET_DB_##d; CLR_NZVC; result = source - dest; SETB_NZVC;
/* COM: dst = ~dst */
#define COM_R(d)    int dreg, dest, result;     GET_DW_##d; CLR_NZVC; result = ~dest; SETW_NZ; SET_C; PUT_DW_DREG(result)
#define COM_M(d)    int dreg, dest, result, ea; GET_DW_##d; CLR_NZVC; result = ~dest; SETW_NZ; SET_C; PUT_DW_EA(result)
#define COMB_R(d)   int dreg, dest, result;     GET_DB_##d; CLR_NZVC; result = ~dest; SETB_NZ; SET_C; PUT_DB_DREG(result)
#define COMB_M(d)   int dreg, dest, result, ea; GET_DB_##d; CLR_NZVC; result = ~dest; SETB_NZ; SET_C; PUT_DB_EA(result)
/* DEC: dst -= 1 */
#define DEC_R(d)    int dreg, dest, result;     GET_DW_##d; CLR_NZV; result = dest - 1; SETW_NZ; if (dest == 0x8000) SET_V; PUT_DW_DREG(result)
#define DEC_M(d)    int dreg, dest, result, ea; GET_DW_##d; CLR_NZV; result = dest - 1; SETW_NZ; if (dest == 0x8000) SET_V; PUT_DW_EA(result)
#define DECB_R(d)   int dreg, dest, result;     GET_DB_##d; CLR_NZV; result = dest - 1; SETB_NZ; if (dest == 0x80)   SET_V; PUT_DB_DREG(result)
#define DECB_M(d)   int dreg, dest, result, ea; GET_DB_##d; CLR_NZV; result = dest - 1; SETB_NZ; if (dest == 0x80)   SET_V; PUT_DB_EA(result)
/* INC: dst += 1 */
#define INC_R(d)    int dreg, dest, result;     GET_DW_##d; CLR_NZV; result = dest + 1; SETW_NZ; if (dest == 0x7fff) SET_V; PUT_DW_DREG(result)
#define INC_M(d)    int dreg, dest, result, ea; GET_DW_##d; CLR_NZV; result = dest + 1; SETW_NZ; if (dest == 0x7fff) SET_V; PUT_DW_EA(result)
#define INCB_R(d)   int dreg, dest, result;     GET_DB_##d; CLR_NZV; result = dest + 1; SETB_NZ; if (dest == 0x7f)   SET_V; PUT_DB_DREG(result)
#define INCB_M(d)   int dreg, dest, result, ea; GET_DB_##d; CLR_NZV; result = dest + 1; SETB_NZ; if (dest == 0x7f)   SET_V; PUT_DB_EA(result)
/* JMP: PC = ea */
#define JMP(d)      int dreg, ea; GET_DREG; MAKE_EAW_##d(dreg); PC = ea
/* JSR: PUSH src, src = PC, PC = ea */
#define JSR(d)      int sreg, dreg, ea; GET_SREG; GET_DREG; MAKE_EAW_##d(dreg); PUSH(REGW(sreg)); REGW(sreg) = PC; PC = ea
/* MFPS: dst = flags */
#define MFPS_R(d)   int dreg, result;     result = PSW; CLR_NZV; SETB_NZ; PUT_DW_##d((signed char)result)
#define MFPS_M(d)   int dreg, result, ea; result = PSW; CLR_NZV; SETB_NZ; PUT_DB_##d(result)
/* MOV: dst = src */
#define MOV_R(s,d)  int sreg, dreg, source, result;     GET_SW_##s; CLR_NZV; result = source; SETW_NZ; PUT_DW_##d(result)
#define MOV_M(s,d)  int sreg, dreg, source, result, ea; GET_SW_##s; CLR_NZV; result = source; SETW_NZ; PUT_DW_##d(result)
#define MOVB_R(s,d) int sreg, dreg, source, result;     GET_SB_##s; CLR_NZV; result = source; SETB_NZ; PUT_DW_##d((signed char)result)
#define MOVB_X(s,d) int sreg, dreg, source, result, ea; GET_SB_##s; CLR_NZV; result = source; SETB_NZ; PUT_DW_##d((signed char)result)
#define MOVB_M(s,d) int sreg, dreg, source, result, ea; GET_SB_##s; CLR_NZV; result = source; SETB_NZ; PUT_DB_##d(result)
/* MTPS: flags = src */
#define MTPS_R(d)   int dreg, dest;     GET_DW_##d; PSW = (PSW & ~0xef) | (dest & 0xef); t11_check_irqs()
#define MTPS_M(d)   int dreg, dest, ea; GET_DW_##d; PSW = (PSW & ~0xef) | (dest & 0xef); t11_check_irqs()
/* NEG: dst = -dst */
#define NEG_R(d)    int dreg, dest, result;     GET_DW_##d; CLR_NZVC; result = -dest; SETW_NZ; if (dest == 0x8000) SET_V; if (result) SET_C; PUT_DW_DREG(result)
#define NEG_M(d)    int dreg, dest, result, ea; GET_DW_##d; CLR_NZVC; result = -dest; SETW_NZ; if (dest == 0x8000) SET_V; if (result) SET_C; PUT_DW_EA(result)
#define NEGB_R(d)   int dreg, dest, result;     GET_DB_##d; CLR_NZVC; result = -dest; SETB_NZ; if (dest == 0x80)   SET_V; if (result) SET_C; PUT_DB_DREG(result)
#define NEGB_M(d)   int dreg, dest, result, ea; GET_DB_##d; CLR_NZVC; result = -dest; SETB_NZ; if (dest == 0x80)   SET_V; if (result) SET_C; PUT_DB_EA(result)
/* ROL: dst = (dst << 1) | C; C = (dst >> 7) */
#define ROL_R(d)    int dreg, dest, result;     GET_DW_##d; result = (dest << 1) | GET_C; CLR_NZVC; SETW_NZ; PSW |= (dest >> 15) & 1; PSW |= ((PSW << 1) ^ (PSW >> 2)) & 2; PUT_DW_DREG(result)
#define ROL_M(d)    int dreg, dest, result, ea; GET_DW_##d; result = (dest << 1) | GET_C; CLR_NZVC; SETW_NZ; PSW |= (dest >> 15) & 1; PSW |= ((PSW << 1) ^ (PSW >> 2)) & 2; PUT_DW_EA(result)
#define ROLB_R(d)   int dreg, dest, result;     GET_DB_##d; result = (dest << 1) | GET_C; CLR_NZVC; SETB_NZ; PSW |= (dest >> 7) & 1;  PSW |= ((PSW << 1) ^ (PSW >> 2)) & 2; PUT_DB_DREG(result)
#define ROLB_M(d)   int dreg, dest, result, ea; GET_DB_##d; result = (dest << 1) | GET_C; CLR_NZVC; SETB_NZ; PSW |= (dest >> 7) & 1;  PSW |= ((PSW << 1) ^ (PSW >> 2)) & 2; PUT_DB_EA(result)
/* ROR: dst = (dst >> 1) | (C << 7); C = dst & 1 */
#define ROR_R(d)    int dreg, dest, result;     GET_DW_##d; result = (dest >> 1) | (GET_C << 15); CLR_NZVC; SETW_NZ; PSW |= dest & 1; PSW |= ((PSW << 1) ^ (PSW >> 2)) & 2; PUT_DW_DREG(result)
#define ROR_M(d)    int dreg, dest, result, ea; GET_DW_##d; result = (dest >> 1) | (GET_C << 15); CLR_NZVC; SETW_NZ; PSW |= dest & 1; PSW |= ((PSW << 1) ^ (PSW >> 2)) & 2; PUT_DW_EA(result)
#define RORB_R(d)   int dreg, dest, result;     GET_DB_##d; result = (dest >> 1) | (GET_C << 7);  CLR_NZVC; SETB_NZ; PSW |= dest & 1; PSW |= ((PSW << 1) ^ (PSW >> 2)) & 2; PUT_DB_DREG(result)
#define RORB_M(d)   int dreg, dest, result, ea; GET_DB_##d; result = (dest >> 1) | (GET_C << 7);  CLR_NZVC; SETB_NZ; PSW |= dest & 1; PSW |= ((PSW << 1) ^ (PSW >> 2)) & 2; PUT_DB_EA(result)
/* SBC: dst -= C */
#define SBC_R(d)    int dreg, source, dest, result;     source = GET_C; GET_DW_##d; CLR_NZVC; result = dest - source; SETW_NZVC; PUT_DW_DREG(result)
#define SBC_M(d)    int dreg, source, dest, result, ea; source = GET_C; GET_DW_##d; CLR_NZVC; result = dest - source; SETW_NZVC; PUT_DW_EA(result)
#define SBCB_R(d)   int dreg, source, dest, result;     source = GET_C; GET_DB_##d; CLR_NZVC; result = dest - source; SETB_NZVC; PUT_DB_DREG(result)
#define SBCB_M(d)   int dreg, source, dest, result, ea; source = GET_C; GET_DB_##d; CLR_NZVC; result = dest - source; SETB_NZVC; PUT_DB_EA(result)
/* SUB: dst -= src */
#define SUB_R(s,d)  int sreg, dreg, source, dest, result;     GET_SW_##s; GET_DW_##d; CLR_NZVC; result = dest - source; SETW_NZVC; PUT_DW_DREG(result)
#define SUB_X(s,d)  int sreg, dreg, source, dest, result, ea; GET_SW_##s; GET_DW_##d; CLR_NZVC; result = dest - source; SETW_NZVC; PUT_DW_DREG(result)
#define SUB_M(s,d)  int sreg, dreg, source, dest, result, ea; GET_SW_##s; GET_DW_##d; CLR_NZVC; result = dest - source; SETW_NZVC; PUT_DW_EA(result)
#define SUBB_R(s,d) int sreg, dreg, source, dest, result;     GET_SB_##s; GET_DB_##d; CLR_NZVC; result = dest - source; SETB_NZVC; PUT_DB_DREG(result)
#define SUBB_X(s,d) int sreg, dreg, source, dest, result, ea; GET_SB_##s; GET_DB_##d; CLR_NZVC; result = dest - source; SETB_NZVC; PUT_DB_DREG(result)
#define SUBB_M(s,d) int sreg, dreg, source, dest, result, ea; GET_SB_##s; GET_DB_##d; CLR_NZVC; result = dest - source; SETB_NZVC; PUT_DB_EA(result)
/* SWAB: dst = (dst >> 8) + (dst << 8) */
#define SWAB_R(d)   int dreg, dest, result;     GET_DW_##d; CLR_NZVC; result = ((dest >> 8) & 0xff) + (dest << 8); SETB_NZ; PUT_DW_DREG(result)
#define SWAB_M(d)   int dreg, dest, result, ea; GET_DW_##d; CLR_NZVC; result = ((dest >> 8) & 0xff) + (dest << 8); SETB_NZ; PUT_DW_EA(result)
/* SXT: dst = sign-extend dst */
#define SXT_R(d)    int dreg, result;     CLR_ZV; if (GET_N) result = -1; else { result = 0; SET_Z; } PUT_DW_##d(result)
#define SXT_M(d)    int dreg, result, ea; CLR_ZV; if (GET_N) result = -1; else { result = 0; SET_Z; } PUT_DW_##d(result)
/* TST: dst = ~dst */
#define TST_R(d)    int dreg, dest, result;     GET_DW_##d; CLR_NZVC; result = dest; SETW_NZ;
#define TST_M(d)    int dreg, dest, result, ea; GET_DW_##d; CLR_NZVC; result = dest; SETW_NZ;
#define TSTB_R(d)   int dreg, dest, result;     GET_DB_##d; CLR_NZVC; result = dest; SETB_NZ;
#define TSTB_M(d)   int dreg, dest, result, ea; GET_DB_##d; CLR_NZVC; result = dest; SETB_NZ;
/* XOR: dst ^= src */
#define XOR_R(d)    int sreg, dreg, source, dest, result;     GET_SREG; source = REGW(sreg); GET_DW_##d; CLR_NZV; result = dest ^ source; SETW_NZ; PUT_DW_DREG(result)
#define XOR_M(d)    int sreg, dreg, source, dest, result, ea; GET_SREG; source = REGW(sreg); GET_DW_##d; CLR_NZV; result = dest ^ source; SETW_NZ; PUT_DW_EA(result)



void t11_device::op_0000(UINT16 op)
{
	switch (op & 0x3f)
	{
		case 0x00:  /* HALT  */ halt(op); break;
		case 0x01:  /* WAIT  */ m_icount = 0; m_wait_state = 1; break;
		case 0x02:  /* RTI   */ m_icount -= 24; PC = POP(); PSW = POP(); t11_check_irqs(); break;
		case 0x03:  /* BPT   */ m_icount -= 48; PUSH(PSW); PUSH(PC); PC = RWORD(0x0c); PSW = RWORD(0x0e); t11_check_irqs(); break;
		case 0x04:  /* IOT   */ m_icount -= 48; PUSH(PSW); PUSH(PC); PC = RWORD(0x10); PSW = RWORD(0x12); t11_check_irqs(); break;
		case 0x05:  /* RESET */ m_icount -= 110; break;
		case 0x06:  /* RTT   */ m_icount -= 33; PC = POP(); PSW = POP(); t11_check_irqs(); break;
		default:    illegal(op); break;
	}
}

void t11_device::halt(UINT16 op)
{
	m_icount -= 48;
	PUSH(PSW);
	PUSH(PC);
	PC = RWORD(0x04);
	PSW = RWORD(0x06);
	t11_check_irqs();
}

void t11_device::illegal(UINT16 op)
{
	m_icount -= 48;
	PUSH(PSW);
	PUSH(PC);
	PC = RWORD(0x08);
	PSW = RWORD(0x0a);
	t11_check_irqs();
}

void t11_device::mark(UINT16 op)
{
	m_icount -= 36;

	SP = SP + 2 * (op & 0x3f);
	PC = REGW(5);
	REGW(5) = POP();
}

void t11_device::jmp_rgd(UINT16 op)       { m_icount -= 15; { JMP(RGD); } }
void t11_device::jmp_in(UINT16 op)        { m_icount -= 18; { JMP(IN);  } }
void t11_device::jmp_ind(UINT16 op)       { m_icount -= 18; { JMP(IND); } }
void t11_device::jmp_de(UINT16 op)        { m_icount -= 18; { JMP(DE);  } }
void t11_device::jmp_ded(UINT16 op)       { m_icount -= 21; { JMP(DED); } }
void t11_device::jmp_ix(UINT16 op)        { m_icount -= 21; { JMP(IX);  } }
void t11_device::jmp_ixd(UINT16 op)       { m_icount -= 27; { JMP(IXD); } }

void t11_device::rts(UINT16 op)
{
	int dreg;
	m_icount -= 21;
	GET_DREG;
	PC = REGD(dreg);
	REGW(dreg) = POP();
}

void t11_device::ccc(UINT16 op)         { m_icount -= 18; { PSW &= ~(op & 15); } }
void t11_device::scc(UINT16 op)         { m_icount -= 18; { PSW |=  (op & 15); } }

void t11_device::swab_rg(UINT16 op)       { m_icount -= 12; { SWAB_R(RG); } }
void t11_device::swab_rgd(UINT16 op)      { m_icount -= 21; { SWAB_M(RGD); } }
void t11_device::swab_in(UINT16 op)       { m_icount -= 21; { SWAB_M(IN); } }
void t11_device::swab_ind(UINT16 op)      { m_icount -= 27; { SWAB_M(IND); } }
void t11_device::swab_de(UINT16 op)       { m_icount -= 24; { SWAB_M(DE); } }
void t11_device::swab_ded(UINT16 op)      { m_icount -= 30; { SWAB_M(DED); } }
void t11_device::swab_ix(UINT16 op)       { m_icount -= 30; { SWAB_M(IX); } }
void t11_device::swab_ixd(UINT16 op)      { m_icount -= 36; { SWAB_M(IXD); } }

void t11_device::br(UINT16 op)            { m_icount -= 12; { BR(1); } }
void t11_device::bne(UINT16 op)           { m_icount -= 12; { BR(!GET_Z); } }
void t11_device::beq(UINT16 op)           { m_icount -= 12; { BR( GET_Z); } }
void t11_device::bge(UINT16 op)           { m_icount -= 12; { BR(!((GET_N >> 2) ^ GET_V)); } }
void t11_device::blt(UINT16 op)           { m_icount -= 12; { BR(((GET_N >> 2) ^ GET_V)); } }
void t11_device::bgt(UINT16 op)           { m_icount -= 12; { BR(!GET_Z && !((GET_N >> 2) ^ GET_V)); } }
void t11_device::ble(UINT16 op)           { m_icount -= 12; { BR( GET_Z || ((GET_N >> 2) ^ GET_V)); } }

void t11_device::jsr_rgd(UINT16 op)       { m_icount -= 27; { JSR(RGD); } }
void t11_device::jsr_in(UINT16 op)        { m_icount -= 30; { JSR(IN);  } }
void t11_device::jsr_ind(UINT16 op)       { m_icount -= 30; { JSR(IND); } }
void t11_device::jsr_de(UINT16 op)        { m_icount -= 30; { JSR(DE);  } }
void t11_device::jsr_ded(UINT16 op)       { m_icount -= 33; { JSR(DED); } }
void t11_device::jsr_ix(UINT16 op)        { m_icount -= 33; { JSR(IX);  } }
void t11_device::jsr_ixd(UINT16 op)       { m_icount -= 39; { JSR(IXD); } }

void t11_device::clr_rg(UINT16 op)        { m_icount -= 12; { CLR_R(RG);  } }
void t11_device::clr_rgd(UINT16 op)       { m_icount -= 21; { CLR_M(RGD); } }
void t11_device::clr_in(UINT16 op)        { m_icount -= 21; { CLR_M(IN);  } }
void t11_device::clr_ind(UINT16 op)       { m_icount -= 27; { CLR_M(IND); } }
void t11_device::clr_de(UINT16 op)        { m_icount -= 24; { CLR_M(DE);  } }
void t11_device::clr_ded(UINT16 op)       { m_icount -= 30; { CLR_M(DED); } }
void t11_device::clr_ix(UINT16 op)        { m_icount -= 30; { CLR_M(IX);  } }
void t11_device::clr_ixd(UINT16 op)       { m_icount -= 36; { CLR_M(IXD); } }

void t11_device::com_rg(UINT16 op)        { m_icount -= 12; { COM_R(RG);  } }
void t11_device::com_rgd(UINT16 op)       { m_icount -= 21; { COM_M(RGD); } }
void t11_device::com_in(UINT16 op)        { m_icount -= 21; { COM_M(IN);  } }
void t11_device::com_ind(UINT16 op)       { m_icount -= 27; { COM_M(IND); } }
void t11_device::com_de(UINT16 op)        { m_icount -= 24; { COM_M(DE);  } }
void t11_device::com_ded(UINT16 op)       { m_icount -= 30; { COM_M(DED); } }
void t11_device::com_ix(UINT16 op)        { m_icount -= 30; { COM_M(IX);  } }
void t11_device::com_ixd(UINT16 op)       { m_icount -= 36; { COM_M(IXD); } }

void t11_device::inc_rg(UINT16 op)        { m_icount -= 12; { INC_R(RG);  } }
void t11_device::inc_rgd(UINT16 op)       { m_icount -= 21; { INC_M(RGD); } }
void t11_device::inc_in(UINT16 op)        { m_icount -= 21; { INC_M(IN);  } }
void t11_device::inc_ind(UINT16 op)       { m_icount -= 27; { INC_M(IND); } }
void t11_device::inc_de(UINT16 op)        { m_icount -= 24; { INC_M(DE);  } }
void t11_device::inc_ded(UINT16 op)       { m_icount -= 30; { INC_M(DED); } }
void t11_device::inc_ix(UINT16 op)        { m_icount -= 30; { INC_M(IX);  } }
void t11_device::inc_ixd(UINT16 op)       { m_icount -= 36; { INC_M(IXD); } }

void t11_device::dec_rg(UINT16 op)        { m_icount -= 12; { DEC_R(RG);  } }
void t11_device::dec_rgd(UINT16 op)       { m_icount -= 21; { DEC_M(RGD); } }
void t11_device::dec_in(UINT16 op)        { m_icount -= 21; { DEC_M(IN);  } }
void t11_device::dec_ind(UINT16 op)       { m_icount -= 27; { DEC_M(IND); } }
void t11_device::dec_de(UINT16 op)        { m_icount -= 24; { DEC_M(DE);  } }
void t11_device::dec_ded(UINT16 op)       { m_icount -= 30; { DEC_M(DED); } }
void t11_device::dec_ix(UINT16 op)        { m_icount -= 30; { DEC_M(IX);  } }
void t11_device::dec_ixd(UINT16 op)       { m_icount -= 36; { DEC_M(IXD); } }

void t11_device::neg_rg(UINT16 op)        { m_icount -= 12; { NEG_R(RG);  } }
void t11_device::neg_rgd(UINT16 op)       { m_icount -= 21; { NEG_M(RGD); } }
void t11_device::neg_in(UINT16 op)        { m_icount -= 21; { NEG_M(IN);  } }
void t11_device::neg_ind(UINT16 op)       { m_icount -= 27; { NEG_M(IND); } }
void t11_device::neg_de(UINT16 op)        { m_icount -= 24; { NEG_M(DE);  } }
void t11_device::neg_ded(UINT16 op)       { m_icount -= 30; { NEG_M(DED); } }
void t11_device::neg_ix(UINT16 op)        { m_icount -= 30; { NEG_M(IX);  } }
void t11_device::neg_ixd(UINT16 op)       { m_icount -= 36; { NEG_M(IXD); } }

void t11_device::adc_rg(UINT16 op)        { m_icount -= 12; { ADC_R(RG);  } }
void t11_device::adc_rgd(UINT16 op)       { m_icount -= 21; { ADC_M(RGD); } }
void t11_device::adc_in(UINT16 op)        { m_icount -= 21; { ADC_M(IN);  } }
void t11_device::adc_ind(UINT16 op)       { m_icount -= 27; { ADC_M(IND); } }
void t11_device::adc_de(UINT16 op)        { m_icount -= 24; { ADC_M(DE);  } }
void t11_device::adc_ded(UINT16 op)       { m_icount -= 30; { ADC_M(DED); } }
void t11_device::adc_ix(UINT16 op)        { m_icount -= 30; { ADC_M(IX);  } }
void t11_device::adc_ixd(UINT16 op)       { m_icount -= 36; { ADC_M(IXD); } }

void t11_device::sbc_rg(UINT16 op)        { m_icount -= 12; { SBC_R(RG);  } }
void t11_device::sbc_rgd(UINT16 op)       { m_icount -= 21; { SBC_M(RGD); } }
void t11_device::sbc_in(UINT16 op)        { m_icount -= 21; { SBC_M(IN);  } }
void t11_device::sbc_ind(UINT16 op)       { m_icount -= 27; { SBC_M(IND); } }
void t11_device::sbc_de(UINT16 op)        { m_icount -= 24; { SBC_M(DE);  } }
void t11_device::sbc_ded(UINT16 op)       { m_icount -= 30; { SBC_M(DED); } }
void t11_device::sbc_ix(UINT16 op)        { m_icount -= 30; { SBC_M(IX);  } }
void t11_device::sbc_ixd(UINT16 op)       { m_icount -= 36; { SBC_M(IXD); } }

void t11_device::tst_rg(UINT16 op)        { m_icount -= 12; { TST_R(RG);  } }
void t11_device::tst_rgd(UINT16 op)       { m_icount -= 18; { TST_M(RGD); } }
void t11_device::tst_in(UINT16 op)        { m_icount -= 18; { TST_M(IN);  } }
void t11_device::tst_ind(UINT16 op)       { m_icount -= 24; { TST_M(IND); } }
void t11_device::tst_de(UINT16 op)        { m_icount -= 21; { TST_M(DE);  } }
void t11_device::tst_ded(UINT16 op)       { m_icount -= 27; { TST_M(DED); } }
void t11_device::tst_ix(UINT16 op)        { m_icount -= 27; { TST_M(IX);  } }
void t11_device::tst_ixd(UINT16 op)       { m_icount -= 33; { TST_M(IXD); } }

void t11_device::ror_rg(UINT16 op)        { m_icount -= 12; { ROR_R(RG);  } }
void t11_device::ror_rgd(UINT16 op)       { m_icount -= 21; { ROR_M(RGD); } }
void t11_device::ror_in(UINT16 op)        { m_icount -= 21; { ROR_M(IN);  } }
void t11_device::ror_ind(UINT16 op)       { m_icount -= 27; { ROR_M(IND); } }
void t11_device::ror_de(UINT16 op)        { m_icount -= 24; { ROR_M(DE);  } }
void t11_device::ror_ded(UINT16 op)       { m_icount -= 30; { ROR_M(DED); } }
void t11_device::ror_ix(UINT16 op)        { m_icount -= 30; { ROR_M(IX);  } }
void t11_device::ror_ixd(UINT16 op)       { m_icount -= 36; { ROR_M(IXD); } }

void t11_device::rol_rg(UINT16 op)        { m_icount -= 12; { ROL_R(RG);  } }
void t11_device::rol_rgd(UINT16 op)       { m_icount -= 21; { ROL_M(RGD); } }
void t11_device::rol_in(UINT16 op)        { m_icount -= 21; { ROL_M(IN);  } }
void t11_device::rol_ind(UINT16 op)       { m_icount -= 27; { ROL_M(IND); } }
void t11_device::rol_de(UINT16 op)        { m_icount -= 24; { ROL_M(DE);  } }
void t11_device::rol_ded(UINT16 op)       { m_icount -= 30; { ROL_M(DED); } }
void t11_device::rol_ix(UINT16 op)        { m_icount -= 30; { ROL_M(IX);  } }
void t11_device::rol_ixd(UINT16 op)       { m_icount -= 36; { ROL_M(IXD); } }

void t11_device::asr_rg(UINT16 op)        { m_icount -= 12; { ASR_R(RG);  } }
void t11_device::asr_rgd(UINT16 op)       { m_icount -= 21; { ASR_M(RGD); } }
void t11_device::asr_in(UINT16 op)        { m_icount -= 21; { ASR_M(IN);  } }
void t11_device::asr_ind(UINT16 op)       { m_icount -= 27; { ASR_M(IND); } }
void t11_device::asr_de(UINT16 op)        { m_icount -= 24; { ASR_M(DE);  } }
void t11_device::asr_ded(UINT16 op)       { m_icount -= 30; { ASR_M(DED); } }
void t11_device::asr_ix(UINT16 op)        { m_icount -= 30; { ASR_M(IX);  } }
void t11_device::asr_ixd(UINT16 op)       { m_icount -= 36; { ASR_M(IXD); } }

void t11_device::asl_rg(UINT16 op)        { m_icount -= 12; { ASL_R(RG);  } }
void t11_device::asl_rgd(UINT16 op)       { m_icount -= 21; { ASL_M(RGD); } }
void t11_device::asl_in(UINT16 op)        { m_icount -= 21; { ASL_M(IN);  } }
void t11_device::asl_ind(UINT16 op)       { m_icount -= 27; { ASL_M(IND); } }
void t11_device::asl_de(UINT16 op)        { m_icount -= 24; { ASL_M(DE);  } }
void t11_device::asl_ded(UINT16 op)       { m_icount -= 30; { ASL_M(DED); } }
void t11_device::asl_ix(UINT16 op)        { m_icount -= 30; { ASL_M(IX);  } }
void t11_device::asl_ixd(UINT16 op)       { m_icount -= 36; { ASL_M(IXD); } }

void t11_device::sxt_rg(UINT16 op)        { m_icount -= 12; { SXT_R(RG);  } }
void t11_device::sxt_rgd(UINT16 op)       { m_icount -= 21; { SXT_M(RGD); } }
void t11_device::sxt_in(UINT16 op)        { m_icount -= 21; { SXT_M(IN);  } }
void t11_device::sxt_ind(UINT16 op)       { m_icount -= 27; { SXT_M(IND); } }
void t11_device::sxt_de(UINT16 op)        { m_icount -= 24; { SXT_M(DE);  } }
void t11_device::sxt_ded(UINT16 op)       { m_icount -= 30; { SXT_M(DED); } }
void t11_device::sxt_ix(UINT16 op)        { m_icount -= 30; { SXT_M(IX);  } }
void t11_device::sxt_ixd(UINT16 op)       { m_icount -= 36; { SXT_M(IXD); } }

void t11_device::mov_rg_rg(UINT16 op)     { m_icount -=  9+ 3; { MOV_R(RG,RG);   } }
void t11_device::mov_rg_rgd(UINT16 op)    { m_icount -=  9+12; { MOV_M(RG,RGD);  } }
void t11_device::mov_rg_in(UINT16 op)     { m_icount -=  9+12; { MOV_M(RG,IN);   } }
void t11_device::mov_rg_ind(UINT16 op)    { m_icount -=  9+18; { MOV_M(RG,IND);  } }
void t11_device::mov_rg_de(UINT16 op)     { m_icount -=  9+15; { MOV_M(RG,DE);   } }
void t11_device::mov_rg_ded(UINT16 op)    { m_icount -=  9+21; { MOV_M(RG,DED);  } }
void t11_device::mov_rg_ix(UINT16 op)     { m_icount -=  9+21; { MOV_M(RG,IX);   } }
void t11_device::mov_rg_ixd(UINT16 op)    { m_icount -=  9+27; { MOV_M(RG,IXD);  } }
void t11_device::mov_rgd_rg(UINT16 op)    { m_icount -= 15+ 3; { MOV_M(RGD,RG);  } }
void t11_device::mov_rgd_rgd(UINT16 op)   { m_icount -= 15+12; { MOV_M(RGD,RGD); } }
void t11_device::mov_rgd_in(UINT16 op)    { m_icount -= 15+12; { MOV_M(RGD,IN);  } }
void t11_device::mov_rgd_ind(UINT16 op)   { m_icount -= 15+18; { MOV_M(RGD,IND); } }
void t11_device::mov_rgd_de(UINT16 op)    { m_icount -= 15+15; { MOV_M(RGD,DE);  } }
void t11_device::mov_rgd_ded(UINT16 op)   { m_icount -= 15+21; { MOV_M(RGD,DED); } }
void t11_device::mov_rgd_ix(UINT16 op)    { m_icount -= 15+21; { MOV_M(RGD,IX);  } }
void t11_device::mov_rgd_ixd(UINT16 op)   { m_icount -= 15+27; { MOV_M(RGD,IXD); } }
void t11_device::mov_in_rg(UINT16 op)     { m_icount -= 15+ 3; { MOV_M(IN,RG);   } }
void t11_device::mov_in_rgd(UINT16 op)    { m_icount -= 15+12; { MOV_M(IN,RGD);  } }
void t11_device::mov_in_in(UINT16 op)     { m_icount -= 15+12; { MOV_M(IN,IN);   } }
void t11_device::mov_in_ind(UINT16 op)    { m_icount -= 15+18; { MOV_M(IN,IND);  } }
void t11_device::mov_in_de(UINT16 op)     { m_icount -= 15+15; { MOV_M(IN,DE);   } }
void t11_device::mov_in_ded(UINT16 op)    { m_icount -= 15+21; { MOV_M(IN,DED);  } }
void t11_device::mov_in_ix(UINT16 op)     { m_icount -= 15+21; { MOV_M(IN,IX);   } }
void t11_device::mov_in_ixd(UINT16 op)    { m_icount -= 15+27; { MOV_M(IN,IXD);  } }
void t11_device::mov_ind_rg(UINT16 op)    { m_icount -= 21+ 3; { MOV_M(IND,RG);  } }
void t11_device::mov_ind_rgd(UINT16 op)   { m_icount -= 21+12; { MOV_M(IND,RGD); } }
void t11_device::mov_ind_in(UINT16 op)    { m_icount -= 21+12; { MOV_M(IND,IN);  } }
void t11_device::mov_ind_ind(UINT16 op)   { m_icount -= 21+18; { MOV_M(IND,IND); } }
void t11_device::mov_ind_de(UINT16 op)    { m_icount -= 21+15; { MOV_M(IND,DE);  } }
void t11_device::mov_ind_ded(UINT16 op)   { m_icount -= 21+21; { MOV_M(IND,DED); } }
void t11_device::mov_ind_ix(UINT16 op)    { m_icount -= 21+21; { MOV_M(IND,IX);  } }
void t11_device::mov_ind_ixd(UINT16 op)   { m_icount -= 21+27; { MOV_M(IND,IXD); } }
void t11_device::mov_de_rg(UINT16 op)     { m_icount -= 18+ 3; { MOV_M(DE,RG);   } }
void t11_device::mov_de_rgd(UINT16 op)    { m_icount -= 18+12; { MOV_M(DE,RGD);  } }
void t11_device::mov_de_in(UINT16 op)     { m_icount -= 18+12; { MOV_M(DE,IN);   } }
void t11_device::mov_de_ind(UINT16 op)    { m_icount -= 18+18; { MOV_M(DE,IND);  } }
void t11_device::mov_de_de(UINT16 op)     { m_icount -= 18+15; { MOV_M(DE,DE);   } }
void t11_device::mov_de_ded(UINT16 op)    { m_icount -= 18+21; { MOV_M(DE,DED);  } }
void t11_device::mov_de_ix(UINT16 op)     { m_icount -= 18+21; { MOV_M(DE,IX);   } }
void t11_device::mov_de_ixd(UINT16 op)    { m_icount -= 18+27; { MOV_M(DE,IXD);  } }
void t11_device::mov_ded_rg(UINT16 op)    { m_icount -= 24+ 3; { MOV_M(DED,RG);  } }
void t11_device::mov_ded_rgd(UINT16 op)   { m_icount -= 24+12; { MOV_M(DED,RGD); } }
void t11_device::mov_ded_in(UINT16 op)    { m_icount -= 24+12; { MOV_M(DED,IN);  } }
void t11_device::mov_ded_ind(UINT16 op)   { m_icount -= 24+18; { MOV_M(DED,IND); } }
void t11_device::mov_ded_de(UINT16 op)    { m_icount -= 24+15; { MOV_M(DED,DE);  } }
void t11_device::mov_ded_ded(UINT16 op)   { m_icount -= 24+21; { MOV_M(DED,DED); } }
void t11_device::mov_ded_ix(UINT16 op)    { m_icount -= 24+21; { MOV_M(DED,IX);  } }
void t11_device::mov_ded_ixd(UINT16 op)   { m_icount -= 24+27; { MOV_M(DED,IXD); } }
void t11_device::mov_ix_rg(UINT16 op)     { m_icount -= 24+ 3; { MOV_M(IX,RG);   } }
void t11_device::mov_ix_rgd(UINT16 op)    { m_icount -= 24+12; { MOV_M(IX,RGD);  } }
void t11_device::mov_ix_in(UINT16 op)     { m_icount -= 24+12; { MOV_M(IX,IN);   } }
void t11_device::mov_ix_ind(UINT16 op)    { m_icount -= 24+18; { MOV_M(IX,IND);  } }
void t11_device::mov_ix_de(UINT16 op)     { m_icount -= 24+15; { MOV_M(IX,DE);   } }
void t11_device::mov_ix_ded(UINT16 op)    { m_icount -= 24+21; { MOV_M(IX,DED);  } }
void t11_device::mov_ix_ix(UINT16 op)     { m_icount -= 24+21; { MOV_M(IX,IX);   } }
void t11_device::mov_ix_ixd(UINT16 op)    { m_icount -= 24+27; { MOV_M(IX,IXD);  } }
void t11_device::mov_ixd_rg(UINT16 op)    { m_icount -= 30+ 3; { MOV_M(IXD,RG);  } }
void t11_device::mov_ixd_rgd(UINT16 op)   { m_icount -= 30+12; { MOV_M(IXD,RGD); } }
void t11_device::mov_ixd_in(UINT16 op)    { m_icount -= 30+12; { MOV_M(IXD,IN);  } }
void t11_device::mov_ixd_ind(UINT16 op)   { m_icount -= 30+18; { MOV_M(IXD,IND); } }
void t11_device::mov_ixd_de(UINT16 op)    { m_icount -= 30+15; { MOV_M(IXD,DE);  } }
void t11_device::mov_ixd_ded(UINT16 op)   { m_icount -= 30+21; { MOV_M(IXD,DED); } }
void t11_device::mov_ixd_ix(UINT16 op)    { m_icount -= 30+21; { MOV_M(IXD,IX);  } }
void t11_device::mov_ixd_ixd(UINT16 op)   { m_icount -= 30+27; { MOV_M(IXD,IXD); } }

void t11_device::cmp_rg_rg(UINT16 op)     { m_icount -=  9+ 3; { CMP_R(RG,RG);   } }
void t11_device::cmp_rg_rgd(UINT16 op)    { m_icount -=  9+ 9; { CMP_M(RG,RGD);  } }
void t11_device::cmp_rg_in(UINT16 op)     { m_icount -=  9+ 9; { CMP_M(RG,IN);   } }
void t11_device::cmp_rg_ind(UINT16 op)    { m_icount -=  9+15; { CMP_M(RG,IND);  } }
void t11_device::cmp_rg_de(UINT16 op)     { m_icount -=  9+12; { CMP_M(RG,DE);   } }
void t11_device::cmp_rg_ded(UINT16 op)    { m_icount -=  9+18; { CMP_M(RG,DED);  } }
void t11_device::cmp_rg_ix(UINT16 op)     { m_icount -=  9+18; { CMP_M(RG,IX);   } }
void t11_device::cmp_rg_ixd(UINT16 op)    { m_icount -=  9+24; { CMP_M(RG,IXD);  } }
void t11_device::cmp_rgd_rg(UINT16 op)    { m_icount -= 15+ 3; { CMP_M(RGD,RG);  } }
void t11_device::cmp_rgd_rgd(UINT16 op)   { m_icount -= 15+ 9; { CMP_M(RGD,RGD); } }
void t11_device::cmp_rgd_in(UINT16 op)    { m_icount -= 15+ 9; { CMP_M(RGD,IN);  } }
void t11_device::cmp_rgd_ind(UINT16 op)   { m_icount -= 15+15; { CMP_M(RGD,IND); } }
void t11_device::cmp_rgd_de(UINT16 op)    { m_icount -= 15+12; { CMP_M(RGD,DE);  } }
void t11_device::cmp_rgd_ded(UINT16 op)   { m_icount -= 15+18; { CMP_M(RGD,DED); } }
void t11_device::cmp_rgd_ix(UINT16 op)    { m_icount -= 15+18; { CMP_M(RGD,IX);  } }
void t11_device::cmp_rgd_ixd(UINT16 op)   { m_icount -= 15+24; { CMP_M(RGD,IXD); } }
void t11_device::cmp_in_rg(UINT16 op)     { m_icount -= 15+ 3; { CMP_M(IN,RG);   } }
void t11_device::cmp_in_rgd(UINT16 op)    { m_icount -= 15+ 9; { CMP_M(IN,RGD);  } }
void t11_device::cmp_in_in(UINT16 op)     { m_icount -= 15+ 9; { CMP_M(IN,IN);   } }
void t11_device::cmp_in_ind(UINT16 op)    { m_icount -= 15+15; { CMP_M(IN,IND);  } }
void t11_device::cmp_in_de(UINT16 op)     { m_icount -= 15+12; { CMP_M(IN,DE);   } }
void t11_device::cmp_in_ded(UINT16 op)    { m_icount -= 15+18; { CMP_M(IN,DED);  } }
void t11_device::cmp_in_ix(UINT16 op)     { m_icount -= 15+18; { CMP_M(IN,IX);   } }
void t11_device::cmp_in_ixd(UINT16 op)    { m_icount -= 15+24; { CMP_M(IN,IXD);  } }
void t11_device::cmp_ind_rg(UINT16 op)    { m_icount -= 21+ 3; { CMP_M(IND,RG);  } }
void t11_device::cmp_ind_rgd(UINT16 op)   { m_icount -= 21+ 9; { CMP_M(IND,RGD); } }
void t11_device::cmp_ind_in(UINT16 op)    { m_icount -= 21+ 9; { CMP_M(IND,IN);  } }
void t11_device::cmp_ind_ind(UINT16 op)   { m_icount -= 21+15; { CMP_M(IND,IND); } }
void t11_device::cmp_ind_de(UINT16 op)    { m_icount -= 21+12; { CMP_M(IND,DE);  } }
void t11_device::cmp_ind_ded(UINT16 op)   { m_icount -= 21+18; { CMP_M(IND,DED); } }
void t11_device::cmp_ind_ix(UINT16 op)    { m_icount -= 21+18; { CMP_M(IND,IX);  } }
void t11_device::cmp_ind_ixd(UINT16 op)   { m_icount -= 21+24; { CMP_M(IND,IXD); } }
void t11_device::cmp_de_rg(UINT16 op)     { m_icount -= 18+ 3; { CMP_M(DE,RG);   } }
void t11_device::cmp_de_rgd(UINT16 op)    { m_icount -= 18+ 9; { CMP_M(DE,RGD);  } }
void t11_device::cmp_de_in(UINT16 op)     { m_icount -= 18+ 9; { CMP_M(DE,IN);   } }
void t11_device::cmp_de_ind(UINT16 op)    { m_icount -= 18+15; { CMP_M(DE,IND);  } }
void t11_device::cmp_de_de(UINT16 op)     { m_icount -= 18+12; { CMP_M(DE,DE);   } }
void t11_device::cmp_de_ded(UINT16 op)    { m_icount -= 18+18; { CMP_M(DE,DED);  } }
void t11_device::cmp_de_ix(UINT16 op)     { m_icount -= 18+18; { CMP_M(DE,IX);   } }
void t11_device::cmp_de_ixd(UINT16 op)    { m_icount -= 18+24; { CMP_M(DE,IXD);  } }
void t11_device::cmp_ded_rg(UINT16 op)    { m_icount -= 24+ 3; { CMP_M(DED,RG);  } }
void t11_device::cmp_ded_rgd(UINT16 op)   { m_icount -= 24+ 9; { CMP_M(DED,RGD); } }
void t11_device::cmp_ded_in(UINT16 op)    { m_icount -= 24+ 9; { CMP_M(DED,IN);  } }
void t11_device::cmp_ded_ind(UINT16 op)   { m_icount -= 24+15; { CMP_M(DED,IND); } }
void t11_device::cmp_ded_de(UINT16 op)    { m_icount -= 24+12; { CMP_M(DED,DE);  } }
void t11_device::cmp_ded_ded(UINT16 op)   { m_icount -= 24+18; { CMP_M(DED,DED); } }
void t11_device::cmp_ded_ix(UINT16 op)    { m_icount -= 24+18; { CMP_M(DED,IX);  } }
void t11_device::cmp_ded_ixd(UINT16 op)   { m_icount -= 24+24; { CMP_M(DED,IXD); } }
void t11_device::cmp_ix_rg(UINT16 op)     { m_icount -= 24+ 3; { CMP_M(IX,RG);   } }
void t11_device::cmp_ix_rgd(UINT16 op)    { m_icount -= 24+ 9; { CMP_M(IX,RGD);  } }
void t11_device::cmp_ix_in(UINT16 op)     { m_icount -= 24+ 9; { CMP_M(IX,IN);   } }
void t11_device::cmp_ix_ind(UINT16 op)    { m_icount -= 24+15; { CMP_M(IX,IND);  } }
void t11_device::cmp_ix_de(UINT16 op)     { m_icount -= 24+12; { CMP_M(IX,DE);   } }
void t11_device::cmp_ix_ded(UINT16 op)    { m_icount -= 24+18; { CMP_M(IX,DED);  } }
void t11_device::cmp_ix_ix(UINT16 op)     { m_icount -= 24+18; { CMP_M(IX,IX);   } }
void t11_device::cmp_ix_ixd(UINT16 op)    { m_icount -= 24+24; { CMP_M(IX,IXD);  } }
void t11_device::cmp_ixd_rg(UINT16 op)    { m_icount -= 30+ 3; { CMP_M(IXD,RG);  } }
void t11_device::cmp_ixd_rgd(UINT16 op)   { m_icount -= 30+ 9; { CMP_M(IXD,RGD); } }
void t11_device::cmp_ixd_in(UINT16 op)    { m_icount -= 30+ 9; { CMP_M(IXD,IN);  } }
void t11_device::cmp_ixd_ind(UINT16 op)   { m_icount -= 30+15; { CMP_M(IXD,IND); } }
void t11_device::cmp_ixd_de(UINT16 op)    { m_icount -= 30+12; { CMP_M(IXD,DE);  } }
void t11_device::cmp_ixd_ded(UINT16 op)   { m_icount -= 30+18; { CMP_M(IXD,DED); } }
void t11_device::cmp_ixd_ix(UINT16 op)    { m_icount -= 30+18; { CMP_M(IXD,IX);  } }
void t11_device::cmp_ixd_ixd(UINT16 op)   { m_icount -= 30+24; { CMP_M(IXD,IXD); } }

void t11_device::bit_rg_rg(UINT16 op)     { m_icount -=  9+ 3; { BIT_R(RG,RG);   } }
void t11_device::bit_rg_rgd(UINT16 op)    { m_icount -=  9+ 9; { BIT_M(RG,RGD);  } }
void t11_device::bit_rg_in(UINT16 op)     { m_icount -=  9+ 9; { BIT_M(RG,IN);   } }
void t11_device::bit_rg_ind(UINT16 op)    { m_icount -=  9+15; { BIT_M(RG,IND);  } }
void t11_device::bit_rg_de(UINT16 op)     { m_icount -=  9+12; { BIT_M(RG,DE);   } }
void t11_device::bit_rg_ded(UINT16 op)    { m_icount -=  9+18; { BIT_M(RG,DED);  } }
void t11_device::bit_rg_ix(UINT16 op)     { m_icount -=  9+18; { BIT_M(RG,IX);   } }
void t11_device::bit_rg_ixd(UINT16 op)    { m_icount -=  9+24; { BIT_M(RG,IXD);  } }
void t11_device::bit_rgd_rg(UINT16 op)    { m_icount -= 15+ 3; { BIT_M(RGD,RG);  } }
void t11_device::bit_rgd_rgd(UINT16 op)   { m_icount -= 15+ 9; { BIT_M(RGD,RGD); } }
void t11_device::bit_rgd_in(UINT16 op)    { m_icount -= 15+ 9; { BIT_M(RGD,IN);  } }
void t11_device::bit_rgd_ind(UINT16 op)   { m_icount -= 15+15; { BIT_M(RGD,IND); } }
void t11_device::bit_rgd_de(UINT16 op)    { m_icount -= 15+12; { BIT_M(RGD,DE);  } }
void t11_device::bit_rgd_ded(UINT16 op)   { m_icount -= 15+18; { BIT_M(RGD,DED); } }
void t11_device::bit_rgd_ix(UINT16 op)    { m_icount -= 15+18; { BIT_M(RGD,IX);  } }
void t11_device::bit_rgd_ixd(UINT16 op)   { m_icount -= 15+24; { BIT_M(RGD,IXD); } }
void t11_device::bit_in_rg(UINT16 op)     { m_icount -= 15+ 3; { BIT_M(IN,RG);   } }
void t11_device::bit_in_rgd(UINT16 op)    { m_icount -= 15+ 9; { BIT_M(IN,RGD);  } }
void t11_device::bit_in_in(UINT16 op)     { m_icount -= 15+ 9; { BIT_M(IN,IN);   } }
void t11_device::bit_in_ind(UINT16 op)    { m_icount -= 15+15; { BIT_M(IN,IND);  } }
void t11_device::bit_in_de(UINT16 op)     { m_icount -= 15+12; { BIT_M(IN,DE);   } }
void t11_device::bit_in_ded(UINT16 op)    { m_icount -= 15+18; { BIT_M(IN,DED);  } }
void t11_device::bit_in_ix(UINT16 op)     { m_icount -= 15+18; { BIT_M(IN,IX);   } }
void t11_device::bit_in_ixd(UINT16 op)    { m_icount -= 15+24; { BIT_M(IN,IXD);  } }
void t11_device::bit_ind_rg(UINT16 op)    { m_icount -= 21+ 3; { BIT_M(IND,RG);  } }
void t11_device::bit_ind_rgd(UINT16 op)   { m_icount -= 21+ 9; { BIT_M(IND,RGD); } }
void t11_device::bit_ind_in(UINT16 op)    { m_icount -= 21+ 9; { BIT_M(IND,IN);  } }
void t11_device::bit_ind_ind(UINT16 op)   { m_icount -= 21+15; { BIT_M(IND,IND); } }
void t11_device::bit_ind_de(UINT16 op)    { m_icount -= 21+12; { BIT_M(IND,DE);  } }
void t11_device::bit_ind_ded(UINT16 op)   { m_icount -= 21+18; { BIT_M(IND,DED); } }
void t11_device::bit_ind_ix(UINT16 op)    { m_icount -= 21+18; { BIT_M(IND,IX);  } }
void t11_device::bit_ind_ixd(UINT16 op)   { m_icount -= 21+24; { BIT_M(IND,IXD); } }
void t11_device::bit_de_rg(UINT16 op)     { m_icount -= 18+ 3; { BIT_M(DE,RG);   } }
void t11_device::bit_de_rgd(UINT16 op)    { m_icount -= 18+ 9; { BIT_M(DE,RGD);  } }
void t11_device::bit_de_in(UINT16 op)     { m_icount -= 18+ 9; { BIT_M(DE,IN);   } }
void t11_device::bit_de_ind(UINT16 op)    { m_icount -= 18+15; { BIT_M(DE,IND);  } }
void t11_device::bit_de_de(UINT16 op)     { m_icount -= 18+12; { BIT_M(DE,DE);   } }
void t11_device::bit_de_ded(UINT16 op)    { m_icount -= 18+18; { BIT_M(DE,DED);  } }
void t11_device::bit_de_ix(UINT16 op)     { m_icount -= 18+18; { BIT_M(DE,IX);   } }
void t11_device::bit_de_ixd(UINT16 op)    { m_icount -= 18+24; { BIT_M(DE,IXD);  } }
void t11_device::bit_ded_rg(UINT16 op)    { m_icount -= 24+ 3; { BIT_M(DED,RG);  } }
void t11_device::bit_ded_rgd(UINT16 op)   { m_icount -= 24+ 9; { BIT_M(DED,RGD); } }
void t11_device::bit_ded_in(UINT16 op)    { m_icount -= 24+ 9; { BIT_M(DED,IN);  } }
void t11_device::bit_ded_ind(UINT16 op)   { m_icount -= 24+15; { BIT_M(DED,IND); } }
void t11_device::bit_ded_de(UINT16 op)    { m_icount -= 24+12; { BIT_M(DED,DE);  } }
void t11_device::bit_ded_ded(UINT16 op)   { m_icount -= 24+18; { BIT_M(DED,DED); } }
void t11_device::bit_ded_ix(UINT16 op)    { m_icount -= 24+18; { BIT_M(DED,IX);  } }
void t11_device::bit_ded_ixd(UINT16 op)   { m_icount -= 24+24; { BIT_M(DED,IXD); } }
void t11_device::bit_ix_rg(UINT16 op)     { m_icount -= 24+ 3; { BIT_M(IX,RG);   } }
void t11_device::bit_ix_rgd(UINT16 op)    { m_icount -= 24+ 9; { BIT_M(IX,RGD);  } }
void t11_device::bit_ix_in(UINT16 op)     { m_icount -= 24+ 9; { BIT_M(IX,IN);   } }
void t11_device::bit_ix_ind(UINT16 op)    { m_icount -= 24+15; { BIT_M(IX,IND);  } }
void t11_device::bit_ix_de(UINT16 op)     { m_icount -= 24+12; { BIT_M(IX,DE);   } }
void t11_device::bit_ix_ded(UINT16 op)    { m_icount -= 24+18; { BIT_M(IX,DED);  } }
void t11_device::bit_ix_ix(UINT16 op)     { m_icount -= 24+18; { BIT_M(IX,IX);   } }
void t11_device::bit_ix_ixd(UINT16 op)    { m_icount -= 24+24; { BIT_M(IX,IXD);  } }
void t11_device::bit_ixd_rg(UINT16 op)    { m_icount -= 30+ 3; { BIT_M(IXD,RG);  } }
void t11_device::bit_ixd_rgd(UINT16 op)   { m_icount -= 30+ 9; { BIT_M(IXD,RGD); } }
void t11_device::bit_ixd_in(UINT16 op)    { m_icount -= 30+ 9; { BIT_M(IXD,IN);  } }
void t11_device::bit_ixd_ind(UINT16 op)   { m_icount -= 30+15; { BIT_M(IXD,IND); } }
void t11_device::bit_ixd_de(UINT16 op)    { m_icount -= 30+12; { BIT_M(IXD,DE);  } }
void t11_device::bit_ixd_ded(UINT16 op)   { m_icount -= 30+18; { BIT_M(IXD,DED); } }
void t11_device::bit_ixd_ix(UINT16 op)    { m_icount -= 30+18; { BIT_M(IXD,IX);  } }
void t11_device::bit_ixd_ixd(UINT16 op)   { m_icount -= 30+24; { BIT_M(IXD,IXD); } }

void t11_device::bic_rg_rg(UINT16 op)     { m_icount -=  9+ 3; { BIC_R(RG,RG);   } }
void t11_device::bic_rg_rgd(UINT16 op)    { m_icount -=  9+12; { BIC_M(RG,RGD);  } }
void t11_device::bic_rg_in(UINT16 op)     { m_icount -=  9+12; { BIC_M(RG,IN);   } }
void t11_device::bic_rg_ind(UINT16 op)    { m_icount -=  9+18; { BIC_M(RG,IND);  } }
void t11_device::bic_rg_de(UINT16 op)     { m_icount -=  9+15; { BIC_M(RG,DE);   } }
void t11_device::bic_rg_ded(UINT16 op)    { m_icount -=  9+21; { BIC_M(RG,DED);  } }
void t11_device::bic_rg_ix(UINT16 op)     { m_icount -=  9+21; { BIC_M(RG,IX);   } }
void t11_device::bic_rg_ixd(UINT16 op)    { m_icount -=  9+27; { BIC_M(RG,IXD);  } }
void t11_device::bic_rgd_rg(UINT16 op)    { m_icount -= 15+ 3; { BIC_X(RGD,RG);  } }
void t11_device::bic_rgd_rgd(UINT16 op)   { m_icount -= 15+12; { BIC_M(RGD,RGD); } }
void t11_device::bic_rgd_in(UINT16 op)    { m_icount -= 15+12; { BIC_M(RGD,IN);  } }
void t11_device::bic_rgd_ind(UINT16 op)   { m_icount -= 15+18; { BIC_M(RGD,IND); } }
void t11_device::bic_rgd_de(UINT16 op)    { m_icount -= 15+15; { BIC_M(RGD,DE);  } }
void t11_device::bic_rgd_ded(UINT16 op)   { m_icount -= 15+21; { BIC_M(RGD,DED); } }
void t11_device::bic_rgd_ix(UINT16 op)    { m_icount -= 15+21; { BIC_M(RGD,IX);  } }
void t11_device::bic_rgd_ixd(UINT16 op)   { m_icount -= 15+27; { BIC_M(RGD,IXD); } }
void t11_device::bic_in_rg(UINT16 op)     { m_icount -= 15+ 3; { BIC_X(IN,RG);   } }
void t11_device::bic_in_rgd(UINT16 op)    { m_icount -= 15+12; { BIC_M(IN,RGD);  } }
void t11_device::bic_in_in(UINT16 op)     { m_icount -= 15+12; { BIC_M(IN,IN);   } }
void t11_device::bic_in_ind(UINT16 op)    { m_icount -= 15+18; { BIC_M(IN,IND);  } }
void t11_device::bic_in_de(UINT16 op)     { m_icount -= 15+15; { BIC_M(IN,DE);   } }
void t11_device::bic_in_ded(UINT16 op)    { m_icount -= 15+21; { BIC_M(IN,DED);  } }
void t11_device::bic_in_ix(UINT16 op)     { m_icount -= 15+21; { BIC_M(IN,IX);   } }
void t11_device::bic_in_ixd(UINT16 op)    { m_icount -= 15+27; { BIC_M(IN,IXD);  } }
void t11_device::bic_ind_rg(UINT16 op)    { m_icount -= 21+ 3; { BIC_X(IND,RG);  } }
void t11_device::bic_ind_rgd(UINT16 op)   { m_icount -= 21+12; { BIC_M(IND,RGD); } }
void t11_device::bic_ind_in(UINT16 op)    { m_icount -= 21+12; { BIC_M(IND,IN);  } }
void t11_device::bic_ind_ind(UINT16 op)   { m_icount -= 21+18; { BIC_M(IND,IND); } }
void t11_device::bic_ind_de(UINT16 op)    { m_icount -= 21+15; { BIC_M(IND,DE);  } }
void t11_device::bic_ind_ded(UINT16 op)   { m_icount -= 21+21; { BIC_M(IND,DED); } }
void t11_device::bic_ind_ix(UINT16 op)    { m_icount -= 21+21; { BIC_M(IND,IX);  } }
void t11_device::bic_ind_ixd(UINT16 op)   { m_icount -= 21+27; { BIC_M(IND,IXD); } }
void t11_device::bic_de_rg(UINT16 op)     { m_icount -= 18+ 3; { BIC_X(DE,RG);   } }
void t11_device::bic_de_rgd(UINT16 op)    { m_icount -= 18+12; { BIC_M(DE,RGD);  } }
void t11_device::bic_de_in(UINT16 op)     { m_icount -= 18+12; { BIC_M(DE,IN);   } }
void t11_device::bic_de_ind(UINT16 op)    { m_icount -= 18+18; { BIC_M(DE,IND);  } }
void t11_device::bic_de_de(UINT16 op)     { m_icount -= 18+15; { BIC_M(DE,DE);   } }
void t11_device::bic_de_ded(UINT16 op)    { m_icount -= 18+21; { BIC_M(DE,DED);  } }
void t11_device::bic_de_ix(UINT16 op)     { m_icount -= 18+21; { BIC_M(DE,IX);   } }
void t11_device::bic_de_ixd(UINT16 op)    { m_icount -= 18+27; { BIC_M(DE,IXD);  } }
void t11_device::bic_ded_rg(UINT16 op)    { m_icount -= 24+ 3; { BIC_X(DED,RG);  } }
void t11_device::bic_ded_rgd(UINT16 op)   { m_icount -= 24+12; { BIC_M(DED,RGD); } }
void t11_device::bic_ded_in(UINT16 op)    { m_icount -= 24+12; { BIC_M(DED,IN);  } }
void t11_device::bic_ded_ind(UINT16 op)   { m_icount -= 24+18; { BIC_M(DED,IND); } }
void t11_device::bic_ded_de(UINT16 op)    { m_icount -= 24+15; { BIC_M(DED,DE);  } }
void t11_device::bic_ded_ded(UINT16 op)   { m_icount -= 24+21; { BIC_M(DED,DED); } }
void t11_device::bic_ded_ix(UINT16 op)    { m_icount -= 24+21; { BIC_M(DED,IX);  } }
void t11_device::bic_ded_ixd(UINT16 op)   { m_icount -= 24+27; { BIC_M(DED,IXD); } }
void t11_device::bic_ix_rg(UINT16 op)     { m_icount -= 24+ 3; { BIC_X(IX,RG);   } }
void t11_device::bic_ix_rgd(UINT16 op)    { m_icount -= 24+12; { BIC_M(IX,RGD);  } }
void t11_device::bic_ix_in(UINT16 op)     { m_icount -= 24+12; { BIC_M(IX,IN);   } }
void t11_device::bic_ix_ind(UINT16 op)    { m_icount -= 24+18; { BIC_M(IX,IND);  } }
void t11_device::bic_ix_de(UINT16 op)     { m_icount -= 24+15; { BIC_M(IX,DE);   } }
void t11_device::bic_ix_ded(UINT16 op)    { m_icount -= 24+21; { BIC_M(IX,DED);  } }
void t11_device::bic_ix_ix(UINT16 op)     { m_icount -= 24+21; { BIC_M(IX,IX);   } }
void t11_device::bic_ix_ixd(UINT16 op)    { m_icount -= 24+27; { BIC_M(IX,IXD);  } }
void t11_device::bic_ixd_rg(UINT16 op)    { m_icount -= 30+ 3; { BIC_X(IXD,RG);  } }
void t11_device::bic_ixd_rgd(UINT16 op)   { m_icount -= 30+12; { BIC_M(IXD,RGD); } }
void t11_device::bic_ixd_in(UINT16 op)    { m_icount -= 30+12; { BIC_M(IXD,IN);  } }
void t11_device::bic_ixd_ind(UINT16 op)   { m_icount -= 30+18; { BIC_M(IXD,IND); } }
void t11_device::bic_ixd_de(UINT16 op)    { m_icount -= 30+15; { BIC_M(IXD,DE);  } }
void t11_device::bic_ixd_ded(UINT16 op)   { m_icount -= 30+21; { BIC_M(IXD,DED); } }
void t11_device::bic_ixd_ix(UINT16 op)    { m_icount -= 30+21; { BIC_M(IXD,IX);  } }
void t11_device::bic_ixd_ixd(UINT16 op)   { m_icount -= 30+27; { BIC_M(IXD,IXD); } }

void t11_device::bis_rg_rg(UINT16 op)     { m_icount -=  9+ 3; { BIS_R(RG,RG);   } }
void t11_device::bis_rg_rgd(UINT16 op)    { m_icount -=  9+12; { BIS_M(RG,RGD);  } }
void t11_device::bis_rg_in(UINT16 op)     { m_icount -=  9+12; { BIS_M(RG,IN);   } }
void t11_device::bis_rg_ind(UINT16 op)    { m_icount -=  9+18; { BIS_M(RG,IND);  } }
void t11_device::bis_rg_de(UINT16 op)     { m_icount -=  9+15; { BIS_M(RG,DE);   } }
void t11_device::bis_rg_ded(UINT16 op)    { m_icount -=  9+21; { BIS_M(RG,DED);  } }
void t11_device::bis_rg_ix(UINT16 op)     { m_icount -=  9+21; { BIS_M(RG,IX);   } }
void t11_device::bis_rg_ixd(UINT16 op)    { m_icount -=  9+27; { BIS_M(RG,IXD);  } }
void t11_device::bis_rgd_rg(UINT16 op)    { m_icount -= 15+ 3; { BIS_X(RGD,RG);  } }
void t11_device::bis_rgd_rgd(UINT16 op)   { m_icount -= 15+12; { BIS_M(RGD,RGD); } }
void t11_device::bis_rgd_in(UINT16 op)    { m_icount -= 15+12; { BIS_M(RGD,IN);  } }
void t11_device::bis_rgd_ind(UINT16 op)   { m_icount -= 15+18; { BIS_M(RGD,IND); } }
void t11_device::bis_rgd_de(UINT16 op)    { m_icount -= 15+15; { BIS_M(RGD,DE);  } }
void t11_device::bis_rgd_ded(UINT16 op)   { m_icount -= 15+21; { BIS_M(RGD,DED); } }
void t11_device::bis_rgd_ix(UINT16 op)    { m_icount -= 15+21; { BIS_M(RGD,IX);  } }
void t11_device::bis_rgd_ixd(UINT16 op)   { m_icount -= 15+27; { BIS_M(RGD,IXD); } }
void t11_device::bis_in_rg(UINT16 op)     { m_icount -= 15+ 3; { BIS_X(IN,RG);   } }
void t11_device::bis_in_rgd(UINT16 op)    { m_icount -= 15+12; { BIS_M(IN,RGD);  } }
void t11_device::bis_in_in(UINT16 op)     { m_icount -= 15+12; { BIS_M(IN,IN);   } }
void t11_device::bis_in_ind(UINT16 op)    { m_icount -= 15+18; { BIS_M(IN,IND);  } }
void t11_device::bis_in_de(UINT16 op)     { m_icount -= 15+15; { BIS_M(IN,DE);   } }
void t11_device::bis_in_ded(UINT16 op)    { m_icount -= 15+21; { BIS_M(IN,DED);  } }
void t11_device::bis_in_ix(UINT16 op)     { m_icount -= 15+21; { BIS_M(IN,IX);   } }
void t11_device::bis_in_ixd(UINT16 op)    { m_icount -= 15+27; { BIS_M(IN,IXD);  } }
void t11_device::bis_ind_rg(UINT16 op)    { m_icount -= 21+ 3; { BIS_X(IND,RG);  } }
void t11_device::bis_ind_rgd(UINT16 op)   { m_icount -= 21+12; { BIS_M(IND,RGD); } }
void t11_device::bis_ind_in(UINT16 op)    { m_icount -= 21+12; { BIS_M(IND,IN);  } }
void t11_device::bis_ind_ind(UINT16 op)   { m_icount -= 21+18; { BIS_M(IND,IND); } }
void t11_device::bis_ind_de(UINT16 op)    { m_icount -= 21+15; { BIS_M(IND,DE);  } }
void t11_device::bis_ind_ded(UINT16 op)   { m_icount -= 21+21; { BIS_M(IND,DED); } }
void t11_device::bis_ind_ix(UINT16 op)    { m_icount -= 21+21; { BIS_M(IND,IX);  } }
void t11_device::bis_ind_ixd(UINT16 op)   { m_icount -= 21+27; { BIS_M(IND,IXD); } }
void t11_device::bis_de_rg(UINT16 op)     { m_icount -= 18+ 3; { BIS_X(DE,RG);   } }
void t11_device::bis_de_rgd(UINT16 op)    { m_icount -= 18+12; { BIS_M(DE,RGD);  } }
void t11_device::bis_de_in(UINT16 op)     { m_icount -= 18+12; { BIS_M(DE,IN);   } }
void t11_device::bis_de_ind(UINT16 op)    { m_icount -= 18+18; { BIS_M(DE,IND);  } }
void t11_device::bis_de_de(UINT16 op)     { m_icount -= 18+15; { BIS_M(DE,DE);   } }
void t11_device::bis_de_ded(UINT16 op)    { m_icount -= 18+21; { BIS_M(DE,DED);  } }
void t11_device::bis_de_ix(UINT16 op)     { m_icount -= 18+21; { BIS_M(DE,IX);   } }
void t11_device::bis_de_ixd(UINT16 op)    { m_icount -= 18+27; { BIS_M(DE,IXD);  } }
void t11_device::bis_ded_rg(UINT16 op)    { m_icount -= 24+ 3; { BIS_X(DED,RG);  } }
void t11_device::bis_ded_rgd(UINT16 op)   { m_icount -= 24+12; { BIS_M(DED,RGD); } }
void t11_device::bis_ded_in(UINT16 op)    { m_icount -= 24+12; { BIS_M(DED,IN);  } }
void t11_device::bis_ded_ind(UINT16 op)   { m_icount -= 24+18; { BIS_M(DED,IND); } }
void t11_device::bis_ded_de(UINT16 op)    { m_icount -= 24+15; { BIS_M(DED,DE);  } }
void t11_device::bis_ded_ded(UINT16 op)   { m_icount -= 24+21; { BIS_M(DED,DED); } }
void t11_device::bis_ded_ix(UINT16 op)    { m_icount -= 24+21; { BIS_M(DED,IX);  } }
void t11_device::bis_ded_ixd(UINT16 op)   { m_icount -= 24+27; { BIS_M(DED,IXD); } }
void t11_device::bis_ix_rg(UINT16 op)     { m_icount -= 24+ 3; { BIS_X(IX,RG);   } }
void t11_device::bis_ix_rgd(UINT16 op)    { m_icount -= 24+12; { BIS_M(IX,RGD);  } }
void t11_device::bis_ix_in(UINT16 op)     { m_icount -= 24+12; { BIS_M(IX,IN);   } }
void t11_device::bis_ix_ind(UINT16 op)    { m_icount -= 24+18; { BIS_M(IX,IND);  } }
void t11_device::bis_ix_de(UINT16 op)     { m_icount -= 24+15; { BIS_M(IX,DE);   } }
void t11_device::bis_ix_ded(UINT16 op)    { m_icount -= 24+21; { BIS_M(IX,DED);  } }
void t11_device::bis_ix_ix(UINT16 op)     { m_icount -= 24+21; { BIS_M(IX,IX);   } }
void t11_device::bis_ix_ixd(UINT16 op)    { m_icount -= 24+27; { BIS_M(IX,IXD);  } }
void t11_device::bis_ixd_rg(UINT16 op)    { m_icount -= 30+ 3; { BIS_X(IXD,RG);  } }
void t11_device::bis_ixd_rgd(UINT16 op)   { m_icount -= 30+12; { BIS_M(IXD,RGD); } }
void t11_device::bis_ixd_in(UINT16 op)    { m_icount -= 30+12; { BIS_M(IXD,IN);  } }
void t11_device::bis_ixd_ind(UINT16 op)   { m_icount -= 30+18; { BIS_M(IXD,IND); } }
void t11_device::bis_ixd_de(UINT16 op)    { m_icount -= 30+15; { BIS_M(IXD,DE);  } }
void t11_device::bis_ixd_ded(UINT16 op)   { m_icount -= 30+21; { BIS_M(IXD,DED); } }
void t11_device::bis_ixd_ix(UINT16 op)    { m_icount -= 30+21; { BIS_M(IXD,IX);  } }
void t11_device::bis_ixd_ixd(UINT16 op)   { m_icount -= 30+27; { BIS_M(IXD,IXD); } }

void t11_device::add_rg_rg(UINT16 op)     { m_icount -=  9+ 3; { ADD_R(RG,RG);   } }
void t11_device::add_rg_rgd(UINT16 op)    { m_icount -=  9+12; { ADD_M(RG,RGD);  } }
void t11_device::add_rg_in(UINT16 op)     { m_icount -=  9+12; { ADD_M(RG,IN);   } }
void t11_device::add_rg_ind(UINT16 op)    { m_icount -=  9+18; { ADD_M(RG,IND);  } }
void t11_device::add_rg_de(UINT16 op)     { m_icount -=  9+15; { ADD_M(RG,DE);   } }
void t11_device::add_rg_ded(UINT16 op)    { m_icount -=  9+21; { ADD_M(RG,DED);  } }
void t11_device::add_rg_ix(UINT16 op)     { m_icount -=  9+21; { ADD_M(RG,IX);   } }
void t11_device::add_rg_ixd(UINT16 op)    { m_icount -=  9+27; { ADD_M(RG,IXD);  } }
void t11_device::add_rgd_rg(UINT16 op)    { m_icount -= 15+ 3; { ADD_X(RGD,RG);  } }
void t11_device::add_rgd_rgd(UINT16 op)   { m_icount -= 15+12; { ADD_M(RGD,RGD); } }
void t11_device::add_rgd_in(UINT16 op)    { m_icount -= 15+12; { ADD_M(RGD,IN);  } }
void t11_device::add_rgd_ind(UINT16 op)   { m_icount -= 15+18; { ADD_M(RGD,IND); } }
void t11_device::add_rgd_de(UINT16 op)    { m_icount -= 15+15; { ADD_M(RGD,DE);  } }
void t11_device::add_rgd_ded(UINT16 op)   { m_icount -= 15+21; { ADD_M(RGD,DED); } }
void t11_device::add_rgd_ix(UINT16 op)    { m_icount -= 15+21; { ADD_M(RGD,IX);  } }
void t11_device::add_rgd_ixd(UINT16 op)   { m_icount -= 15+27; { ADD_M(RGD,IXD); } }
void t11_device::add_in_rg(UINT16 op)     { m_icount -= 15+ 3; { ADD_X(IN,RG);   } }
void t11_device::add_in_rgd(UINT16 op)    { m_icount -= 15+12; { ADD_M(IN,RGD);  } }
void t11_device::add_in_in(UINT16 op)     { m_icount -= 15+12; { ADD_M(IN,IN);   } }
void t11_device::add_in_ind(UINT16 op)    { m_icount -= 15+18; { ADD_M(IN,IND);  } }
void t11_device::add_in_de(UINT16 op)     { m_icount -= 15+15; { ADD_M(IN,DE);   } }
void t11_device::add_in_ded(UINT16 op)    { m_icount -= 15+21; { ADD_M(IN,DED);  } }
void t11_device::add_in_ix(UINT16 op)     { m_icount -= 15+21; { ADD_M(IN,IX);   } }
void t11_device::add_in_ixd(UINT16 op)    { m_icount -= 15+27; { ADD_M(IN,IXD);  } }
void t11_device::add_ind_rg(UINT16 op)    { m_icount -= 21+ 3; { ADD_X(IND,RG);  } }
void t11_device::add_ind_rgd(UINT16 op)   { m_icount -= 21+12; { ADD_M(IND,RGD); } }
void t11_device::add_ind_in(UINT16 op)    { m_icount -= 21+12; { ADD_M(IND,IN);  } }
void t11_device::add_ind_ind(UINT16 op)   { m_icount -= 21+18; { ADD_M(IND,IND); } }
void t11_device::add_ind_de(UINT16 op)    { m_icount -= 21+15; { ADD_M(IND,DE);  } }
void t11_device::add_ind_ded(UINT16 op)   { m_icount -= 21+21; { ADD_M(IND,DED); } }
void t11_device::add_ind_ix(UINT16 op)    { m_icount -= 21+21; { ADD_M(IND,IX);  } }
void t11_device::add_ind_ixd(UINT16 op)   { m_icount -= 21+27; { ADD_M(IND,IXD); } }
void t11_device::add_de_rg(UINT16 op)     { m_icount -= 18+ 3; { ADD_X(DE,RG);   } }
void t11_device::add_de_rgd(UINT16 op)    { m_icount -= 18+12; { ADD_M(DE,RGD);  } }
void t11_device::add_de_in(UINT16 op)     { m_icount -= 18+12; { ADD_M(DE,IN);   } }
void t11_device::add_de_ind(UINT16 op)    { m_icount -= 18+18; { ADD_M(DE,IND);  } }
void t11_device::add_de_de(UINT16 op)     { m_icount -= 18+15; { ADD_M(DE,DE);   } }
void t11_device::add_de_ded(UINT16 op)    { m_icount -= 18+21; { ADD_M(DE,DED);  } }
void t11_device::add_de_ix(UINT16 op)     { m_icount -= 18+21; { ADD_M(DE,IX);   } }
void t11_device::add_de_ixd(UINT16 op)    { m_icount -= 18+27; { ADD_M(DE,IXD);  } }
void t11_device::add_ded_rg(UINT16 op)    { m_icount -= 24+ 3; { ADD_X(DED,RG);  } }
void t11_device::add_ded_rgd(UINT16 op)   { m_icount -= 24+12; { ADD_M(DED,RGD); } }
void t11_device::add_ded_in(UINT16 op)    { m_icount -= 24+12; { ADD_M(DED,IN);  } }
void t11_device::add_ded_ind(UINT16 op)   { m_icount -= 24+18; { ADD_M(DED,IND); } }
void t11_device::add_ded_de(UINT16 op)    { m_icount -= 24+15; { ADD_M(DED,DE);  } }
void t11_device::add_ded_ded(UINT16 op)   { m_icount -= 24+21; { ADD_M(DED,DED); } }
void t11_device::add_ded_ix(UINT16 op)    { m_icount -= 24+21; { ADD_M(DED,IX);  } }
void t11_device::add_ded_ixd(UINT16 op)   { m_icount -= 24+27; { ADD_M(DED,IXD); } }
void t11_device::add_ix_rg(UINT16 op)     { m_icount -= 24+ 3; { ADD_X(IX,RG);   } }
void t11_device::add_ix_rgd(UINT16 op)    { m_icount -= 24+12; { ADD_M(IX,RGD);  } }
void t11_device::add_ix_in(UINT16 op)     { m_icount -= 24+12; { ADD_M(IX,IN);   } }
void t11_device::add_ix_ind(UINT16 op)    { m_icount -= 24+18; { ADD_M(IX,IND);  } }
void t11_device::add_ix_de(UINT16 op)     { m_icount -= 24+15; { ADD_M(IX,DE);   } }
void t11_device::add_ix_ded(UINT16 op)    { m_icount -= 24+21; { ADD_M(IX,DED);  } }
void t11_device::add_ix_ix(UINT16 op)     { m_icount -= 24+21; { ADD_M(IX,IX);   } }
void t11_device::add_ix_ixd(UINT16 op)    { m_icount -= 24+27; { ADD_M(IX,IXD);  } }
void t11_device::add_ixd_rg(UINT16 op)    { m_icount -= 30+ 3; { ADD_X(IXD,RG);  } }
void t11_device::add_ixd_rgd(UINT16 op)   { m_icount -= 30+12; { ADD_M(IXD,RGD); } }
void t11_device::add_ixd_in(UINT16 op)    { m_icount -= 30+12; { ADD_M(IXD,IN);  } }
void t11_device::add_ixd_ind(UINT16 op)   { m_icount -= 30+18; { ADD_M(IXD,IND); } }
void t11_device::add_ixd_de(UINT16 op)    { m_icount -= 30+15; { ADD_M(IXD,DE);  } }
void t11_device::add_ixd_ded(UINT16 op)   { m_icount -= 30+21; { ADD_M(IXD,DED); } }
void t11_device::add_ixd_ix(UINT16 op)    { m_icount -= 30+21; { ADD_M(IXD,IX);  } }
void t11_device::add_ixd_ixd(UINT16 op)   { m_icount -= 30+27; { ADD_M(IXD,IXD); } }

void t11_device::xor_rg(UINT16 op)        { m_icount -= 12; { XOR_R(RG);  } }
void t11_device::xor_rgd(UINT16 op)       { m_icount -= 21; { XOR_M(RGD); } }
void t11_device::xor_in(UINT16 op)        { m_icount -= 21; { XOR_M(IN);  } }
void t11_device::xor_ind(UINT16 op)       { m_icount -= 27; { XOR_M(IND); } }
void t11_device::xor_de(UINT16 op)        { m_icount -= 24; { XOR_M(DE);  } }
void t11_device::xor_ded(UINT16 op)       { m_icount -= 30; { XOR_M(DED); } }
void t11_device::xor_ix(UINT16 op)        { m_icount -= 30; { XOR_M(IX);  } }
void t11_device::xor_ixd(UINT16 op)       { m_icount -= 36; { XOR_M(IXD); } }

void t11_device::sob(UINT16 op)
{
	int sreg, source;

	m_icount -= 18;
	GET_SREG; source = REGD(sreg);
	source -= 1;
	REGW(sreg) = source;
	if (source)
		PC -= 2 * (op & 0x3f);
}

void t11_device::bpl(UINT16 op)           { m_icount -= 12; { BR(!GET_N); } }
void t11_device::bmi(UINT16 op)           { m_icount -= 12; { BR( GET_N); } }
void t11_device::bhi(UINT16 op)           { m_icount -= 12; { BR(!GET_C && !GET_Z); } }
void t11_device::blos(UINT16 op)          { m_icount -= 12; { BR( GET_C ||  GET_Z); } }
void t11_device::bvc(UINT16 op)           { m_icount -= 12; { BR(!GET_V); } }
void t11_device::bvs(UINT16 op)           { m_icount -= 12; { BR( GET_V); } }
void t11_device::bcc(UINT16 op)           { m_icount -= 12; { BR(!GET_C); } }
void t11_device::bcs(UINT16 op)           { m_icount -= 12; { BR( GET_C); } }

void t11_device::emt(UINT16 op)
{
	m_icount -= 48;
	PUSH(PSW);
	PUSH(PC);
	PC = RWORD(0x18);
	PSW = RWORD(0x1a);
	t11_check_irqs();
}

void t11_device::trap(UINT16 op)
{
	m_icount -= 48;
	PUSH(PSW);
	PUSH(PC);
	PC = RWORD(0x1c);
	PSW = RWORD(0x1e);
	t11_check_irqs();
}

void t11_device::clrb_rg(UINT16 op)       { m_icount -= 12; { CLRB_R(RG);  } }
void t11_device::clrb_rgd(UINT16 op)      { m_icount -= 21; { CLRB_M(RGD); } }
void t11_device::clrb_in(UINT16 op)       { m_icount -= 21; { CLRB_M(IN);  } }
void t11_device::clrb_ind(UINT16 op)      { m_icount -= 27; { CLRB_M(IND); } }
void t11_device::clrb_de(UINT16 op)       { m_icount -= 24; { CLRB_M(DE);  } }
void t11_device::clrb_ded(UINT16 op)      { m_icount -= 30; { CLRB_M(DED); } }
void t11_device::clrb_ix(UINT16 op)       { m_icount -= 30; { CLRB_M(IX);  } }
void t11_device::clrb_ixd(UINT16 op)      { m_icount -= 36; { CLRB_M(IXD); } }

void t11_device::comb_rg(UINT16 op)       { m_icount -= 12; { COMB_R(RG);  } }
void t11_device::comb_rgd(UINT16 op)      { m_icount -= 21; { COMB_M(RGD); } }
void t11_device::comb_in(UINT16 op)       { m_icount -= 21; { COMB_M(IN);  } }
void t11_device::comb_ind(UINT16 op)      { m_icount -= 27; { COMB_M(IND); } }
void t11_device::comb_de(UINT16 op)       { m_icount -= 24; { COMB_M(DE);  } }
void t11_device::comb_ded(UINT16 op)      { m_icount -= 30; { COMB_M(DED); } }
void t11_device::comb_ix(UINT16 op)       { m_icount -= 30; { COMB_M(IX);  } }
void t11_device::comb_ixd(UINT16 op)      { m_icount -= 36; { COMB_M(IXD); } }

void t11_device::incb_rg(UINT16 op)       { m_icount -= 12; { INCB_R(RG);  } }
void t11_device::incb_rgd(UINT16 op)      { m_icount -= 21; { INCB_M(RGD); } }
void t11_device::incb_in(UINT16 op)       { m_icount -= 21; { INCB_M(IN);  } }
void t11_device::incb_ind(UINT16 op)      { m_icount -= 27; { INCB_M(IND); } }
void t11_device::incb_de(UINT16 op)       { m_icount -= 24; { INCB_M(DE);  } }
void t11_device::incb_ded(UINT16 op)      { m_icount -= 30; { INCB_M(DED); } }
void t11_device::incb_ix(UINT16 op)       { m_icount -= 30; { INCB_M(IX);  } }
void t11_device::incb_ixd(UINT16 op)      { m_icount -= 36; { INCB_M(IXD); } }

void t11_device::decb_rg(UINT16 op)       { m_icount -= 12; { DECB_R(RG);  } }
void t11_device::decb_rgd(UINT16 op)      { m_icount -= 21; { DECB_M(RGD); } }
void t11_device::decb_in(UINT16 op)       { m_icount -= 21; { DECB_M(IN);  } }
void t11_device::decb_ind(UINT16 op)      { m_icount -= 27; { DECB_M(IND); } }
void t11_device::decb_de(UINT16 op)       { m_icount -= 24; { DECB_M(DE);  } }
void t11_device::decb_ded(UINT16 op)      { m_icount -= 30; { DECB_M(DED); } }
void t11_device::decb_ix(UINT16 op)       { m_icount -= 30; { DECB_M(IX);  } }
void t11_device::decb_ixd(UINT16 op)      { m_icount -= 36; { DECB_M(IXD); } }

void t11_device::negb_rg(UINT16 op)       { m_icount -= 12; { NEGB_R(RG);  } }
void t11_device::negb_rgd(UINT16 op)      { m_icount -= 21; { NEGB_M(RGD); } }
void t11_device::negb_in(UINT16 op)       { m_icount -= 21; { NEGB_M(IN);  } }
void t11_device::negb_ind(UINT16 op)      { m_icount -= 27; { NEGB_M(IND); } }
void t11_device::negb_de(UINT16 op)       { m_icount -= 24; { NEGB_M(DE);  } }
void t11_device::negb_ded(UINT16 op)      { m_icount -= 30; { NEGB_M(DED); } }
void t11_device::negb_ix(UINT16 op)       { m_icount -= 30; { NEGB_M(IX);  } }
void t11_device::negb_ixd(UINT16 op)      { m_icount -= 36; { NEGB_M(IXD); } }

void t11_device::adcb_rg(UINT16 op)       { m_icount -= 12; { ADCB_R(RG);  } }
void t11_device::adcb_rgd(UINT16 op)      { m_icount -= 21; { ADCB_M(RGD); } }
void t11_device::adcb_in(UINT16 op)       { m_icount -= 21; { ADCB_M(IN);  } }
void t11_device::adcb_ind(UINT16 op)      { m_icount -= 27; { ADCB_M(IND); } }
void t11_device::adcb_de(UINT16 op)       { m_icount -= 24; { ADCB_M(DE);  } }
void t11_device::adcb_ded(UINT16 op)      { m_icount -= 30; { ADCB_M(DED); } }
void t11_device::adcb_ix(UINT16 op)       { m_icount -= 30; { ADCB_M(IX);  } }
void t11_device::adcb_ixd(UINT16 op)      { m_icount -= 36; { ADCB_M(IXD); } }

void t11_device::sbcb_rg(UINT16 op)       { m_icount -= 12; { SBCB_R(RG);  } }
void t11_device::sbcb_rgd(UINT16 op)      { m_icount -= 21; { SBCB_M(RGD); } }
void t11_device::sbcb_in(UINT16 op)       { m_icount -= 21; { SBCB_M(IN);  } }
void t11_device::sbcb_ind(UINT16 op)      { m_icount -= 27; { SBCB_M(IND); } }
void t11_device::sbcb_de(UINT16 op)       { m_icount -= 24; { SBCB_M(DE);  } }
void t11_device::sbcb_ded(UINT16 op)      { m_icount -= 30; { SBCB_M(DED); } }
void t11_device::sbcb_ix(UINT16 op)       { m_icount -= 30; { SBCB_M(IX);  } }
void t11_device::sbcb_ixd(UINT16 op)      { m_icount -= 36; { SBCB_M(IXD); } }

void t11_device::tstb_rg(UINT16 op)       { m_icount -= 12; { TSTB_R(RG);  } }
void t11_device::tstb_rgd(UINT16 op)      { m_icount -= 18; { TSTB_M(RGD); } }
void t11_device::tstb_in(UINT16 op)       { m_icount -= 18; { TSTB_M(IN);  } }
void t11_device::tstb_ind(UINT16 op)      { m_icount -= 24; { TSTB_M(IND); } }
void t11_device::tstb_de(UINT16 op)       { m_icount -= 21; { TSTB_M(DE);  } }
void t11_device::tstb_ded(UINT16 op)      { m_icount -= 27; { TSTB_M(DED); } }
void t11_device::tstb_ix(UINT16 op)       { m_icount -= 27; { TSTB_M(IX);  } }
void t11_device::tstb_ixd(UINT16 op)      { m_icount -= 33; { TSTB_M(IXD); } }

void t11_device::rorb_rg(UINT16 op)       { m_icount -= 12; { RORB_R(RG);  } }
void t11_device::rorb_rgd(UINT16 op)      { m_icount -= 21; { RORB_M(RGD); } }
void t11_device::rorb_in(UINT16 op)       { m_icount -= 21; { RORB_M(IN);  } }
void t11_device::rorb_ind(UINT16 op)      { m_icount -= 27; { RORB_M(IND); } }
void t11_device::rorb_de(UINT16 op)       { m_icount -= 24; { RORB_M(DE);  } }
void t11_device::rorb_ded(UINT16 op)      { m_icount -= 30; { RORB_M(DED); } }
void t11_device::rorb_ix(UINT16 op)       { m_icount -= 30; { RORB_M(IX);  } }
void t11_device::rorb_ixd(UINT16 op)      { m_icount -= 36; { RORB_M(IXD); } }

void t11_device::rolb_rg(UINT16 op)       { m_icount -= 12; { ROLB_R(RG);  } }
void t11_device::rolb_rgd(UINT16 op)      { m_icount -= 21; { ROLB_M(RGD); } }
void t11_device::rolb_in(UINT16 op)       { m_icount -= 21; { ROLB_M(IN);  } }
void t11_device::rolb_ind(UINT16 op)      { m_icount -= 27; { ROLB_M(IND); } }
void t11_device::rolb_de(UINT16 op)       { m_icount -= 24; { ROLB_M(DE);  } }
void t11_device::rolb_ded(UINT16 op)      { m_icount -= 30; { ROLB_M(DED); } }
void t11_device::rolb_ix(UINT16 op)       { m_icount -= 30; { ROLB_M(IX);  } }
void t11_device::rolb_ixd(UINT16 op)      { m_icount -= 36; { ROLB_M(IXD); } }

void t11_device::asrb_rg(UINT16 op)       { m_icount -= 12; { ASRB_R(RG);  } }
void t11_device::asrb_rgd(UINT16 op)      { m_icount -= 21; { ASRB_M(RGD); } }
void t11_device::asrb_in(UINT16 op)       { m_icount -= 21; { ASRB_M(IN);  } }
void t11_device::asrb_ind(UINT16 op)      { m_icount -= 27; { ASRB_M(IND); } }
void t11_device::asrb_de(UINT16 op)       { m_icount -= 24; { ASRB_M(DE);  } }
void t11_device::asrb_ded(UINT16 op)      { m_icount -= 30; { ASRB_M(DED); } }
void t11_device::asrb_ix(UINT16 op)       { m_icount -= 30; { ASRB_M(IX);  } }
void t11_device::asrb_ixd(UINT16 op)      { m_icount -= 36; { ASRB_M(IXD); } }

void t11_device::aslb_rg(UINT16 op)       { m_icount -= 12; { ASLB_R(RG);  } }
void t11_device::aslb_rgd(UINT16 op)      { m_icount -= 21; { ASLB_M(RGD); } }
void t11_device::aslb_in(UINT16 op)       { m_icount -= 21; { ASLB_M(IN);  } }
void t11_device::aslb_ind(UINT16 op)      { m_icount -= 27; { ASLB_M(IND); } }
void t11_device::aslb_de(UINT16 op)       { m_icount -= 24; { ASLB_M(DE);  } }
void t11_device::aslb_ded(UINT16 op)      { m_icount -= 30; { ASLB_M(DED); } }
void t11_device::aslb_ix(UINT16 op)       { m_icount -= 30; { ASLB_M(IX);  } }
void t11_device::aslb_ixd(UINT16 op)      { m_icount -= 36; { ASLB_M(IXD); } }

void t11_device::mtps_rg(UINT16 op)       { m_icount -= 24; { MTPS_R(RG);  } }
void t11_device::mtps_rgd(UINT16 op)      { m_icount -= 30; { MTPS_M(RGD); } }
void t11_device::mtps_in(UINT16 op)       { m_icount -= 30; { MTPS_M(IN);  } }
void t11_device::mtps_ind(UINT16 op)      { m_icount -= 36; { MTPS_M(IND); } }
void t11_device::mtps_de(UINT16 op)       { m_icount -= 33; { MTPS_M(DE);  } }
void t11_device::mtps_ded(UINT16 op)      { m_icount -= 39; { MTPS_M(DED); } }
void t11_device::mtps_ix(UINT16 op)       { m_icount -= 39; { MTPS_M(IX);  } }
void t11_device::mtps_ixd(UINT16 op)      { m_icount -= 45; { MTPS_M(IXD); } }

void t11_device::mfps_rg(UINT16 op)       { m_icount -= 12; { MFPS_R(RG);  } }
void t11_device::mfps_rgd(UINT16 op)      { m_icount -= 21; { MFPS_M(RGD); } }
void t11_device::mfps_in(UINT16 op)       { m_icount -= 21; { MFPS_M(IN);  } }
void t11_device::mfps_ind(UINT16 op)      { m_icount -= 27; { MFPS_M(IND); } }
void t11_device::mfps_de(UINT16 op)       { m_icount -= 24; { MFPS_M(DE);  } }
void t11_device::mfps_ded(UINT16 op)      { m_icount -= 30; { MFPS_M(DED); } }
void t11_device::mfps_ix(UINT16 op)       { m_icount -= 30; { MFPS_M(IX);  } }
void t11_device::mfps_ixd(UINT16 op)      { m_icount -= 36; { MFPS_M(IXD); } }

void t11_device::movb_rg_rg(UINT16 op)     { m_icount -=  9+ 3; { MOVB_R(RG,RG);   } }
void t11_device::movb_rg_rgd(UINT16 op)    { m_icount -=  9+12; { MOVB_M(RG,RGD);  } }
void t11_device::movb_rg_in(UINT16 op)     { m_icount -=  9+12; { MOVB_M(RG,IN);   } }
void t11_device::movb_rg_ind(UINT16 op)    { m_icount -=  9+18; { MOVB_M(RG,IND);  } }
void t11_device::movb_rg_de(UINT16 op)     { m_icount -=  9+15; { MOVB_M(RG,DE);   } }
void t11_device::movb_rg_ded(UINT16 op)    { m_icount -=  9+21; { MOVB_M(RG,DED);  } }
void t11_device::movb_rg_ix(UINT16 op)     { m_icount -=  9+21; { MOVB_M(RG,IX);   } }
void t11_device::movb_rg_ixd(UINT16 op)    { m_icount -=  9+27; { MOVB_M(RG,IXD);  } }
void t11_device::movb_rgd_rg(UINT16 op)    { m_icount -= 15+ 3; { MOVB_X(RGD,RG);  } }
void t11_device::movb_rgd_rgd(UINT16 op)   { m_icount -= 15+12; { MOVB_M(RGD,RGD); } }
void t11_device::movb_rgd_in(UINT16 op)    { m_icount -= 15+12; { MOVB_M(RGD,IN);  } }
void t11_device::movb_rgd_ind(UINT16 op)   { m_icount -= 15+18; { MOVB_M(RGD,IND); } }
void t11_device::movb_rgd_de(UINT16 op)    { m_icount -= 15+15; { MOVB_M(RGD,DE);  } }
void t11_device::movb_rgd_ded(UINT16 op)   { m_icount -= 15+21; { MOVB_M(RGD,DED); } }
void t11_device::movb_rgd_ix(UINT16 op)    { m_icount -= 15+21; { MOVB_M(RGD,IX);  } }
void t11_device::movb_rgd_ixd(UINT16 op)   { m_icount -= 15+27; { MOVB_M(RGD,IXD); } }
void t11_device::movb_in_rg(UINT16 op)     { m_icount -= 15+ 3; { MOVB_X(IN,RG);   } }
void t11_device::movb_in_rgd(UINT16 op)    { m_icount -= 15+12; { MOVB_M(IN,RGD);  } }
void t11_device::movb_in_in(UINT16 op)     { m_icount -= 15+12; { MOVB_M(IN,IN);   } }
void t11_device::movb_in_ind(UINT16 op)    { m_icount -= 15+18; { MOVB_M(IN,IND);  } }
void t11_device::movb_in_de(UINT16 op)     { m_icount -= 15+15; { MOVB_M(IN,DE);   } }
void t11_device::movb_in_ded(UINT16 op)    { m_icount -= 15+21; { MOVB_M(IN,DED);  } }
void t11_device::movb_in_ix(UINT16 op)     { m_icount -= 15+21; { MOVB_M(IN,IX);   } }
void t11_device::movb_in_ixd(UINT16 op)    { m_icount -= 15+27; { MOVB_M(IN,IXD);  } }
void t11_device::movb_ind_rg(UINT16 op)    { m_icount -= 21+ 3; { MOVB_X(IND,RG);  } }
void t11_device::movb_ind_rgd(UINT16 op)   { m_icount -= 21+12; { MOVB_M(IND,RGD); } }
void t11_device::movb_ind_in(UINT16 op)    { m_icount -= 21+12; { MOVB_M(IND,IN);  } }
void t11_device::movb_ind_ind(UINT16 op)   { m_icount -= 21+18; { MOVB_M(IND,IND); } }
void t11_device::movb_ind_de(UINT16 op)    { m_icount -= 21+15; { MOVB_M(IND,DE);  } }
void t11_device::movb_ind_ded(UINT16 op)   { m_icount -= 21+21; { MOVB_M(IND,DED); } }
void t11_device::movb_ind_ix(UINT16 op)    { m_icount -= 21+21; { MOVB_M(IND,IX);  } }
void t11_device::movb_ind_ixd(UINT16 op)   { m_icount -= 21+27; { MOVB_M(IND,IXD); } }
void t11_device::movb_de_rg(UINT16 op)     { m_icount -= 18+ 3; { MOVB_X(DE,RG);   } }
void t11_device::movb_de_rgd(UINT16 op)    { m_icount -= 18+12; { MOVB_M(DE,RGD);  } }
void t11_device::movb_de_in(UINT16 op)     { m_icount -= 18+12; { MOVB_M(DE,IN);   } }
void t11_device::movb_de_ind(UINT16 op)    { m_icount -= 18+18; { MOVB_M(DE,IND);  } }
void t11_device::movb_de_de(UINT16 op)     { m_icount -= 18+15; { MOVB_M(DE,DE);   } }
void t11_device::movb_de_ded(UINT16 op)    { m_icount -= 18+21; { MOVB_M(DE,DED);  } }
void t11_device::movb_de_ix(UINT16 op)     { m_icount -= 18+21; { MOVB_M(DE,IX);   } }
void t11_device::movb_de_ixd(UINT16 op)    { m_icount -= 18+27; { MOVB_M(DE,IXD);  } }
void t11_device::movb_ded_rg(UINT16 op)    { m_icount -= 24+ 3; { MOVB_X(DED,RG);  } }
void t11_device::movb_ded_rgd(UINT16 op)   { m_icount -= 24+12; { MOVB_M(DED,RGD); } }
void t11_device::movb_ded_in(UINT16 op)    { m_icount -= 24+12; { MOVB_M(DED,IN);  } }
void t11_device::movb_ded_ind(UINT16 op)   { m_icount -= 24+18; { MOVB_M(DED,IND); } }
void t11_device::movb_ded_de(UINT16 op)    { m_icount -= 24+15; { MOVB_M(DED,DE);  } }
void t11_device::movb_ded_ded(UINT16 op)   { m_icount -= 24+21; { MOVB_M(DED,DED); } }
void t11_device::movb_ded_ix(UINT16 op)    { m_icount -= 24+21; { MOVB_M(DED,IX);  } }
void t11_device::movb_ded_ixd(UINT16 op)   { m_icount -= 24+27; { MOVB_M(DED,IXD); } }
void t11_device::movb_ix_rg(UINT16 op)     { m_icount -= 24+ 3; { MOVB_X(IX,RG);   } }
void t11_device::movb_ix_rgd(UINT16 op)    { m_icount -= 24+12; { MOVB_M(IX,RGD);  } }
void t11_device::movb_ix_in(UINT16 op)     { m_icount -= 24+12; { MOVB_M(IX,IN);   } }
void t11_device::movb_ix_ind(UINT16 op)    { m_icount -= 24+18; { MOVB_M(IX,IND);  } }
void t11_device::movb_ix_de(UINT16 op)     { m_icount -= 24+15; { MOVB_M(IX,DE);   } }
void t11_device::movb_ix_ded(UINT16 op)    { m_icount -= 24+21; { MOVB_M(IX,DED);  } }
void t11_device::movb_ix_ix(UINT16 op)     { m_icount -= 24+21; { MOVB_M(IX,IX);   } }
void t11_device::movb_ix_ixd(UINT16 op)    { m_icount -= 24+27; { MOVB_M(IX,IXD);  } }
void t11_device::movb_ixd_rg(UINT16 op)    { m_icount -= 30+ 3; { MOVB_X(IXD,RG);  } }
void t11_device::movb_ixd_rgd(UINT16 op)   { m_icount -= 30+12; { MOVB_M(IXD,RGD); } }
void t11_device::movb_ixd_in(UINT16 op)    { m_icount -= 30+12; { MOVB_M(IXD,IN);  } }
void t11_device::movb_ixd_ind(UINT16 op)   { m_icount -= 30+18; { MOVB_M(IXD,IND); } }
void t11_device::movb_ixd_de(UINT16 op)    { m_icount -= 30+15; { MOVB_M(IXD,DE);  } }
void t11_device::movb_ixd_ded(UINT16 op)   { m_icount -= 30+21; { MOVB_M(IXD,DED); } }
void t11_device::movb_ixd_ix(UINT16 op)    { m_icount -= 30+21; { MOVB_M(IXD,IX);  } }
void t11_device::movb_ixd_ixd(UINT16 op)   { m_icount -= 30+27; { MOVB_M(IXD,IXD); } }

void t11_device::cmpb_rg_rg(UINT16 op)     { m_icount -=  9+ 3; { CMPB_R(RG,RG);   } }
void t11_device::cmpb_rg_rgd(UINT16 op)    { m_icount -=  9+ 9; { CMPB_M(RG,RGD);  } }
void t11_device::cmpb_rg_in(UINT16 op)     { m_icount -=  9+ 9; { CMPB_M(RG,IN);   } }
void t11_device::cmpb_rg_ind(UINT16 op)    { m_icount -=  9+15; { CMPB_M(RG,IND);  } }
void t11_device::cmpb_rg_de(UINT16 op)     { m_icount -=  9+12; { CMPB_M(RG,DE);   } }
void t11_device::cmpb_rg_ded(UINT16 op)    { m_icount -=  9+18; { CMPB_M(RG,DED);  } }
void t11_device::cmpb_rg_ix(UINT16 op)     { m_icount -=  9+18; { CMPB_M(RG,IX);   } }
void t11_device::cmpb_rg_ixd(UINT16 op)    { m_icount -=  9+24; { CMPB_M(RG,IXD);  } }
void t11_device::cmpb_rgd_rg(UINT16 op)    { m_icount -= 15+ 3; { CMPB_M(RGD,RG);  } }
void t11_device::cmpb_rgd_rgd(UINT16 op)   { m_icount -= 15+ 9; { CMPB_M(RGD,RGD); } }
void t11_device::cmpb_rgd_in(UINT16 op)    { m_icount -= 15+ 9; { CMPB_M(RGD,IN);  } }
void t11_device::cmpb_rgd_ind(UINT16 op)   { m_icount -= 15+15; { CMPB_M(RGD,IND); } }
void t11_device::cmpb_rgd_de(UINT16 op)    { m_icount -= 15+12; { CMPB_M(RGD,DE);  } }
void t11_device::cmpb_rgd_ded(UINT16 op)   { m_icount -= 15+18; { CMPB_M(RGD,DED); } }
void t11_device::cmpb_rgd_ix(UINT16 op)    { m_icount -= 15+18; { CMPB_M(RGD,IX);  } }
void t11_device::cmpb_rgd_ixd(UINT16 op)   { m_icount -= 15+24; { CMPB_M(RGD,IXD); } }
void t11_device::cmpb_in_rg(UINT16 op)     { m_icount -= 15+ 3; { CMPB_M(IN,RG);   } }
void t11_device::cmpb_in_rgd(UINT16 op)    { m_icount -= 15+ 9; { CMPB_M(IN,RGD);  } }
void t11_device::cmpb_in_in(UINT16 op)     { m_icount -= 15+ 9; { CMPB_M(IN,IN);   } }
void t11_device::cmpb_in_ind(UINT16 op)    { m_icount -= 15+15; { CMPB_M(IN,IND);  } }
void t11_device::cmpb_in_de(UINT16 op)     { m_icount -= 15+12; { CMPB_M(IN,DE);   } }
void t11_device::cmpb_in_ded(UINT16 op)    { m_icount -= 15+18; { CMPB_M(IN,DED);  } }
void t11_device::cmpb_in_ix(UINT16 op)     { m_icount -= 15+18; { CMPB_M(IN,IX);   } }
void t11_device::cmpb_in_ixd(UINT16 op)    { m_icount -= 15+24; { CMPB_M(IN,IXD);  } }
void t11_device::cmpb_ind_rg(UINT16 op)    { m_icount -= 21+ 3; { CMPB_M(IND,RG);  } }
void t11_device::cmpb_ind_rgd(UINT16 op)   { m_icount -= 21+ 9; { CMPB_M(IND,RGD); } }
void t11_device::cmpb_ind_in(UINT16 op)    { m_icount -= 21+ 9; { CMPB_M(IND,IN);  } }
void t11_device::cmpb_ind_ind(UINT16 op)   { m_icount -= 21+15; { CMPB_M(IND,IND); } }
void t11_device::cmpb_ind_de(UINT16 op)    { m_icount -= 21+12; { CMPB_M(IND,DE);  } }
void t11_device::cmpb_ind_ded(UINT16 op)   { m_icount -= 21+18; { CMPB_M(IND,DED); } }
void t11_device::cmpb_ind_ix(UINT16 op)    { m_icount -= 21+18; { CMPB_M(IND,IX);  } }
void t11_device::cmpb_ind_ixd(UINT16 op)   { m_icount -= 21+24; { CMPB_M(IND,IXD); } }
void t11_device::cmpb_de_rg(UINT16 op)     { m_icount -= 18+ 3; { CMPB_M(DE,RG);   } }
void t11_device::cmpb_de_rgd(UINT16 op)    { m_icount -= 18+ 9; { CMPB_M(DE,RGD);  } }
void t11_device::cmpb_de_in(UINT16 op)     { m_icount -= 18+ 9; { CMPB_M(DE,IN);   } }
void t11_device::cmpb_de_ind(UINT16 op)    { m_icount -= 18+15; { CMPB_M(DE,IND);  } }
void t11_device::cmpb_de_de(UINT16 op)     { m_icount -= 18+12; { CMPB_M(DE,DE);   } }
void t11_device::cmpb_de_ded(UINT16 op)    { m_icount -= 18+18; { CMPB_M(DE,DED);  } }
void t11_device::cmpb_de_ix(UINT16 op)     { m_icount -= 18+18; { CMPB_M(DE,IX);   } }
void t11_device::cmpb_de_ixd(UINT16 op)    { m_icount -= 18+24; { CMPB_M(DE,IXD);  } }
void t11_device::cmpb_ded_rg(UINT16 op)    { m_icount -= 24+ 3; { CMPB_M(DED,RG);  } }
void t11_device::cmpb_ded_rgd(UINT16 op)   { m_icount -= 24+ 9; { CMPB_M(DED,RGD); } }
void t11_device::cmpb_ded_in(UINT16 op)    { m_icount -= 24+ 9; { CMPB_M(DED,IN);  } }
void t11_device::cmpb_ded_ind(UINT16 op)   { m_icount -= 24+15; { CMPB_M(DED,IND); } }
void t11_device::cmpb_ded_de(UINT16 op)    { m_icount -= 24+12; { CMPB_M(DED,DE);  } }
void t11_device::cmpb_ded_ded(UINT16 op)   { m_icount -= 24+18; { CMPB_M(DED,DED); } }
void t11_device::cmpb_ded_ix(UINT16 op)    { m_icount -= 24+18; { CMPB_M(DED,IX);  } }
void t11_device::cmpb_ded_ixd(UINT16 op)   { m_icount -= 24+24; { CMPB_M(DED,IXD); } }
void t11_device::cmpb_ix_rg(UINT16 op)     { m_icount -= 24+ 3; { CMPB_M(IX,RG);   } }
void t11_device::cmpb_ix_rgd(UINT16 op)    { m_icount -= 24+ 9; { CMPB_M(IX,RGD);  } }
void t11_device::cmpb_ix_in(UINT16 op)     { m_icount -= 24+ 9; { CMPB_M(IX,IN);   } }
void t11_device::cmpb_ix_ind(UINT16 op)    { m_icount -= 24+15; { CMPB_M(IX,IND);  } }
void t11_device::cmpb_ix_de(UINT16 op)     { m_icount -= 24+12; { CMPB_M(IX,DE);   } }
void t11_device::cmpb_ix_ded(UINT16 op)    { m_icount -= 24+18; { CMPB_M(IX,DED);  } }
void t11_device::cmpb_ix_ix(UINT16 op)     { m_icount -= 24+18; { CMPB_M(IX,IX);   } }
void t11_device::cmpb_ix_ixd(UINT16 op)    { m_icount -= 24+24; { CMPB_M(IX,IXD);  } }
void t11_device::cmpb_ixd_rg(UINT16 op)    { m_icount -= 30+ 3; { CMPB_M(IXD,RG);  } }
void t11_device::cmpb_ixd_rgd(UINT16 op)   { m_icount -= 30+ 9; { CMPB_M(IXD,RGD); } }
void t11_device::cmpb_ixd_in(UINT16 op)    { m_icount -= 30+ 9; { CMPB_M(IXD,IN);  } }
void t11_device::cmpb_ixd_ind(UINT16 op)   { m_icount -= 30+15; { CMPB_M(IXD,IND); } }
void t11_device::cmpb_ixd_de(UINT16 op)    { m_icount -= 30+12; { CMPB_M(IXD,DE);  } }
void t11_device::cmpb_ixd_ded(UINT16 op)   { m_icount -= 30+18; { CMPB_M(IXD,DED); } }
void t11_device::cmpb_ixd_ix(UINT16 op)    { m_icount -= 30+18; { CMPB_M(IXD,IX);  } }
void t11_device::cmpb_ixd_ixd(UINT16 op)   { m_icount -= 30+24; { CMPB_M(IXD,IXD); } }

void t11_device::bitb_rg_rg(UINT16 op)     { m_icount -=  9+ 3; { BITB_R(RG,RG);   } }
void t11_device::bitb_rg_rgd(UINT16 op)    { m_icount -=  9+ 9; { BITB_M(RG,RGD);  } }
void t11_device::bitb_rg_in(UINT16 op)     { m_icount -=  9+ 9; { BITB_M(RG,IN);   } }
void t11_device::bitb_rg_ind(UINT16 op)    { m_icount -=  9+15; { BITB_M(RG,IND);  } }
void t11_device::bitb_rg_de(UINT16 op)     { m_icount -=  9+12; { BITB_M(RG,DE);   } }
void t11_device::bitb_rg_ded(UINT16 op)    { m_icount -=  9+18; { BITB_M(RG,DED);  } }
void t11_device::bitb_rg_ix(UINT16 op)     { m_icount -=  9+18; { BITB_M(RG,IX);   } }
void t11_device::bitb_rg_ixd(UINT16 op)    { m_icount -=  9+24; { BITB_M(RG,IXD);  } }
void t11_device::bitb_rgd_rg(UINT16 op)    { m_icount -= 15+ 3; { BITB_M(RGD,RG);  } }
void t11_device::bitb_rgd_rgd(UINT16 op)   { m_icount -= 15+ 9; { BITB_M(RGD,RGD); } }
void t11_device::bitb_rgd_in(UINT16 op)    { m_icount -= 15+ 9; { BITB_M(RGD,IN);  } }
void t11_device::bitb_rgd_ind(UINT16 op)   { m_icount -= 15+15; { BITB_M(RGD,IND); } }
void t11_device::bitb_rgd_de(UINT16 op)    { m_icount -= 15+12; { BITB_M(RGD,DE);  } }
void t11_device::bitb_rgd_ded(UINT16 op)   { m_icount -= 15+18; { BITB_M(RGD,DED); } }
void t11_device::bitb_rgd_ix(UINT16 op)    { m_icount -= 15+18; { BITB_M(RGD,IX);  } }
void t11_device::bitb_rgd_ixd(UINT16 op)   { m_icount -= 15+24; { BITB_M(RGD,IXD); } }
void t11_device::bitb_in_rg(UINT16 op)     { m_icount -= 15+ 3; { BITB_M(IN,RG);   } }
void t11_device::bitb_in_rgd(UINT16 op)    { m_icount -= 15+ 9; { BITB_M(IN,RGD);  } }
void t11_device::bitb_in_in(UINT16 op)     { m_icount -= 15+ 9; { BITB_M(IN,IN);   } }
void t11_device::bitb_in_ind(UINT16 op)    { m_icount -= 15+15; { BITB_M(IN,IND);  } }
void t11_device::bitb_in_de(UINT16 op)     { m_icount -= 15+12; { BITB_M(IN,DE);   } }
void t11_device::bitb_in_ded(UINT16 op)    { m_icount -= 15+18; { BITB_M(IN,DED);  } }
void t11_device::bitb_in_ix(UINT16 op)     { m_icount -= 15+18; { BITB_M(IN,IX);   } }
void t11_device::bitb_in_ixd(UINT16 op)    { m_icount -= 15+24; { BITB_M(IN,IXD);  } }
void t11_device::bitb_ind_rg(UINT16 op)    { m_icount -= 21+ 3; { BITB_M(IND,RG);  } }
void t11_device::bitb_ind_rgd(UINT16 op)   { m_icount -= 21+ 9; { BITB_M(IND,RGD); } }
void t11_device::bitb_ind_in(UINT16 op)    { m_icount -= 21+ 9; { BITB_M(IND,IN);  } }
void t11_device::bitb_ind_ind(UINT16 op)   { m_icount -= 21+15; { BITB_M(IND,IND); } }
void t11_device::bitb_ind_de(UINT16 op)    { m_icount -= 21+12; { BITB_M(IND,DE);  } }
void t11_device::bitb_ind_ded(UINT16 op)   { m_icount -= 21+18; { BITB_M(IND,DED); } }
void t11_device::bitb_ind_ix(UINT16 op)    { m_icount -= 21+18; { BITB_M(IND,IX);  } }
void t11_device::bitb_ind_ixd(UINT16 op)   { m_icount -= 21+24; { BITB_M(IND,IXD); } }
void t11_device::bitb_de_rg(UINT16 op)     { m_icount -= 18+ 3; { BITB_M(DE,RG);   } }
void t11_device::bitb_de_rgd(UINT16 op)    { m_icount -= 18+ 9; { BITB_M(DE,RGD);  } }
void t11_device::bitb_de_in(UINT16 op)     { m_icount -= 18+ 9; { BITB_M(DE,IN);   } }
void t11_device::bitb_de_ind(UINT16 op)    { m_icount -= 18+15; { BITB_M(DE,IND);  } }
void t11_device::bitb_de_de(UINT16 op)     { m_icount -= 18+12; { BITB_M(DE,DE);   } }
void t11_device::bitb_de_ded(UINT16 op)    { m_icount -= 18+18; { BITB_M(DE,DED);  } }
void t11_device::bitb_de_ix(UINT16 op)     { m_icount -= 18+18; { BITB_M(DE,IX);   } }
void t11_device::bitb_de_ixd(UINT16 op)    { m_icount -= 18+24; { BITB_M(DE,IXD);  } }
void t11_device::bitb_ded_rg(UINT16 op)    { m_icount -= 24+ 3; { BITB_M(DED,RG);  } }
void t11_device::bitb_ded_rgd(UINT16 op)   { m_icount -= 24+ 9; { BITB_M(DED,RGD); } }
void t11_device::bitb_ded_in(UINT16 op)    { m_icount -= 24+ 9; { BITB_M(DED,IN);  } }
void t11_device::bitb_ded_ind(UINT16 op)   { m_icount -= 24+15; { BITB_M(DED,IND); } }
void t11_device::bitb_ded_de(UINT16 op)    { m_icount -= 24+12; { BITB_M(DED,DE);  } }
void t11_device::bitb_ded_ded(UINT16 op)   { m_icount -= 24+18; { BITB_M(DED,DED); } }
void t11_device::bitb_ded_ix(UINT16 op)    { m_icount -= 24+18; { BITB_M(DED,IX);  } }
void t11_device::bitb_ded_ixd(UINT16 op)   { m_icount -= 24+24; { BITB_M(DED,IXD); } }
void t11_device::bitb_ix_rg(UINT16 op)     { m_icount -= 24+ 3; { BITB_M(IX,RG);   } }
void t11_device::bitb_ix_rgd(UINT16 op)    { m_icount -= 24+ 9; { BITB_M(IX,RGD);  } }
void t11_device::bitb_ix_in(UINT16 op)     { m_icount -= 24+ 9; { BITB_M(IX,IN);   } }
void t11_device::bitb_ix_ind(UINT16 op)    { m_icount -= 24+15; { BITB_M(IX,IND);  } }
void t11_device::bitb_ix_de(UINT16 op)     { m_icount -= 24+12; { BITB_M(IX,DE);   } }
void t11_device::bitb_ix_ded(UINT16 op)    { m_icount -= 24+18; { BITB_M(IX,DED);  } }
void t11_device::bitb_ix_ix(UINT16 op)     { m_icount -= 24+18; { BITB_M(IX,IX);   } }
void t11_device::bitb_ix_ixd(UINT16 op)    { m_icount -= 24+24; { BITB_M(IX,IXD);  } }
void t11_device::bitb_ixd_rg(UINT16 op)    { m_icount -= 30+ 3; { BITB_M(IXD,RG);  } }
void t11_device::bitb_ixd_rgd(UINT16 op)   { m_icount -= 30+ 9; { BITB_M(IXD,RGD); } }
void t11_device::bitb_ixd_in(UINT16 op)    { m_icount -= 30+ 9; { BITB_M(IXD,IN);  } }
void t11_device::bitb_ixd_ind(UINT16 op)   { m_icount -= 30+15; { BITB_M(IXD,IND); } }
void t11_device::bitb_ixd_de(UINT16 op)    { m_icount -= 30+12; { BITB_M(IXD,DE);  } }
void t11_device::bitb_ixd_ded(UINT16 op)   { m_icount -= 30+18; { BITB_M(IXD,DED); } }
void t11_device::bitb_ixd_ix(UINT16 op)    { m_icount -= 30+18; { BITB_M(IXD,IX);  } }
void t11_device::bitb_ixd_ixd(UINT16 op)   { m_icount -= 30+24; { BITB_M(IXD,IXD); } }

void t11_device::bicb_rg_rg(UINT16 op)     { m_icount -=  9+ 3; { BICB_R(RG,RG);   } }
void t11_device::bicb_rg_rgd(UINT16 op)    { m_icount -=  9+12; { BICB_M(RG,RGD);  } }
void t11_device::bicb_rg_in(UINT16 op)     { m_icount -=  9+12; { BICB_M(RG,IN);   } }
void t11_device::bicb_rg_ind(UINT16 op)    { m_icount -=  9+18; { BICB_M(RG,IND);  } }
void t11_device::bicb_rg_de(UINT16 op)     { m_icount -=  9+15; { BICB_M(RG,DE);   } }
void t11_device::bicb_rg_ded(UINT16 op)    { m_icount -=  9+21; { BICB_M(RG,DED);  } }
void t11_device::bicb_rg_ix(UINT16 op)     { m_icount -=  9+21; { BICB_M(RG,IX);   } }
void t11_device::bicb_rg_ixd(UINT16 op)    { m_icount -=  9+27; { BICB_M(RG,IXD);  } }
void t11_device::bicb_rgd_rg(UINT16 op)    { m_icount -= 15+ 3; { BICB_X(RGD,RG);  } }
void t11_device::bicb_rgd_rgd(UINT16 op)   { m_icount -= 15+12; { BICB_M(RGD,RGD); } }
void t11_device::bicb_rgd_in(UINT16 op)    { m_icount -= 15+12; { BICB_M(RGD,IN);  } }
void t11_device::bicb_rgd_ind(UINT16 op)   { m_icount -= 15+18; { BICB_M(RGD,IND); } }
void t11_device::bicb_rgd_de(UINT16 op)    { m_icount -= 15+15; { BICB_M(RGD,DE);  } }
void t11_device::bicb_rgd_ded(UINT16 op)   { m_icount -= 15+21; { BICB_M(RGD,DED); } }
void t11_device::bicb_rgd_ix(UINT16 op)    { m_icount -= 15+21; { BICB_M(RGD,IX);  } }
void t11_device::bicb_rgd_ixd(UINT16 op)   { m_icount -= 15+27; { BICB_M(RGD,IXD); } }
void t11_device::bicb_in_rg(UINT16 op)     { m_icount -= 15+ 3; { BICB_X(IN,RG);   } }
void t11_device::bicb_in_rgd(UINT16 op)    { m_icount -= 15+12; { BICB_M(IN,RGD);  } }
void t11_device::bicb_in_in(UINT16 op)     { m_icount -= 15+12; { BICB_M(IN,IN);   } }
void t11_device::bicb_in_ind(UINT16 op)    { m_icount -= 15+18; { BICB_M(IN,IND);  } }
void t11_device::bicb_in_de(UINT16 op)     { m_icount -= 15+15; { BICB_M(IN,DE);   } }
void t11_device::bicb_in_ded(UINT16 op)    { m_icount -= 15+21; { BICB_M(IN,DED);  } }
void t11_device::bicb_in_ix(UINT16 op)     { m_icount -= 15+21; { BICB_M(IN,IX);   } }
void t11_device::bicb_in_ixd(UINT16 op)    { m_icount -= 15+27; { BICB_M(IN,IXD);  } }
void t11_device::bicb_ind_rg(UINT16 op)    { m_icount -= 21+ 3; { BICB_X(IND,RG);  } }
void t11_device::bicb_ind_rgd(UINT16 op)   { m_icount -= 21+12; { BICB_M(IND,RGD); } }
void t11_device::bicb_ind_in(UINT16 op)    { m_icount -= 21+12; { BICB_M(IND,IN);  } }
void t11_device::bicb_ind_ind(UINT16 op)   { m_icount -= 21+18; { BICB_M(IND,IND); } }
void t11_device::bicb_ind_de(UINT16 op)    { m_icount -= 21+15; { BICB_M(IND,DE);  } }
void t11_device::bicb_ind_ded(UINT16 op)   { m_icount -= 21+21; { BICB_M(IND,DED); } }
void t11_device::bicb_ind_ix(UINT16 op)    { m_icount -= 21+21; { BICB_M(IND,IX);  } }
void t11_device::bicb_ind_ixd(UINT16 op)   { m_icount -= 21+27; { BICB_M(IND,IXD); } }
void t11_device::bicb_de_rg(UINT16 op)     { m_icount -= 18+ 3; { BICB_X(DE,RG);   } }
void t11_device::bicb_de_rgd(UINT16 op)    { m_icount -= 18+12; { BICB_M(DE,RGD);  } }
void t11_device::bicb_de_in(UINT16 op)     { m_icount -= 18+12; { BICB_M(DE,IN);   } }
void t11_device::bicb_de_ind(UINT16 op)    { m_icount -= 18+18; { BICB_M(DE,IND);  } }
void t11_device::bicb_de_de(UINT16 op)     { m_icount -= 18+15; { BICB_M(DE,DE);   } }
void t11_device::bicb_de_ded(UINT16 op)    { m_icount -= 18+21; { BICB_M(DE,DED);  } }
void t11_device::bicb_de_ix(UINT16 op)     { m_icount -= 18+21; { BICB_M(DE,IX);   } }
void t11_device::bicb_de_ixd(UINT16 op)    { m_icount -= 18+27; { BICB_M(DE,IXD);  } }
void t11_device::bicb_ded_rg(UINT16 op)    { m_icount -= 24+ 3; { BICB_X(DED,RG);  } }
void t11_device::bicb_ded_rgd(UINT16 op)   { m_icount -= 24+12; { BICB_M(DED,RGD); } }
void t11_device::bicb_ded_in(UINT16 op)    { m_icount -= 24+12; { BICB_M(DED,IN);  } }
void t11_device::bicb_ded_ind(UINT16 op)   { m_icount -= 24+18; { BICB_M(DED,IND); } }
void t11_device::bicb_ded_de(UINT16 op)    { m_icount -= 24+15; { BICB_M(DED,DE);  } }
void t11_device::bicb_ded_ded(UINT16 op)   { m_icount -= 24+21; { BICB_M(DED,DED); } }
void t11_device::bicb_ded_ix(UINT16 op)    { m_icount -= 24+21; { BICB_M(DED,IX);  } }
void t11_device::bicb_ded_ixd(UINT16 op)   { m_icount -= 24+27; { BICB_M(DED,IXD); } }
void t11_device::bicb_ix_rg(UINT16 op)     { m_icount -= 24+ 3; { BICB_X(IX,RG);   } }
void t11_device::bicb_ix_rgd(UINT16 op)    { m_icount -= 24+12; { BICB_M(IX,RGD);  } }
void t11_device::bicb_ix_in(UINT16 op)     { m_icount -= 24+12; { BICB_M(IX,IN);   } }
void t11_device::bicb_ix_ind(UINT16 op)    { m_icount -= 24+18; { BICB_M(IX,IND);  } }
void t11_device::bicb_ix_de(UINT16 op)     { m_icount -= 24+15; { BICB_M(IX,DE);   } }
void t11_device::bicb_ix_ded(UINT16 op)    { m_icount -= 24+21; { BICB_M(IX,DED);  } }
void t11_device::bicb_ix_ix(UINT16 op)     { m_icount -= 24+21; { BICB_M(IX,IX);   } }
void t11_device::bicb_ix_ixd(UINT16 op)    { m_icount -= 24+27; { BICB_M(IX,IXD);  } }
void t11_device::bicb_ixd_rg(UINT16 op)    { m_icount -= 30+ 3; { BICB_X(IXD,RG);  } }
void t11_device::bicb_ixd_rgd(UINT16 op)   { m_icount -= 30+12; { BICB_M(IXD,RGD); } }
void t11_device::bicb_ixd_in(UINT16 op)    { m_icount -= 30+12; { BICB_M(IXD,IN);  } }
void t11_device::bicb_ixd_ind(UINT16 op)   { m_icount -= 30+18; { BICB_M(IXD,IND); } }
void t11_device::bicb_ixd_de(UINT16 op)    { m_icount -= 30+15; { BICB_M(IXD,DE);  } }
void t11_device::bicb_ixd_ded(UINT16 op)   { m_icount -= 30+21; { BICB_M(IXD,DED); } }
void t11_device::bicb_ixd_ix(UINT16 op)    { m_icount -= 30+21; { BICB_M(IXD,IX);  } }
void t11_device::bicb_ixd_ixd(UINT16 op)   { m_icount -= 30+27; { BICB_M(IXD,IXD); } }

void t11_device::bisb_rg_rg(UINT16 op)     { m_icount -=  9+ 3; { BISB_R(RG,RG);   } }
void t11_device::bisb_rg_rgd(UINT16 op)    { m_icount -=  9+12; { BISB_M(RG,RGD);  } }
void t11_device::bisb_rg_in(UINT16 op)     { m_icount -=  9+12; { BISB_M(RG,IN);   } }
void t11_device::bisb_rg_ind(UINT16 op)    { m_icount -=  9+18; { BISB_M(RG,IND);  } }
void t11_device::bisb_rg_de(UINT16 op)     { m_icount -=  9+15; { BISB_M(RG,DE);   } }
void t11_device::bisb_rg_ded(UINT16 op)    { m_icount -=  9+21; { BISB_M(RG,DED);  } }
void t11_device::bisb_rg_ix(UINT16 op)     { m_icount -=  9+21; { BISB_M(RG,IX);   } }
void t11_device::bisb_rg_ixd(UINT16 op)    { m_icount -=  9+27; { BISB_M(RG,IXD);  } }
void t11_device::bisb_rgd_rg(UINT16 op)    { m_icount -= 15+ 3; { BISB_X(RGD,RG);  } }
void t11_device::bisb_rgd_rgd(UINT16 op)   { m_icount -= 15+12; { BISB_M(RGD,RGD); } }
void t11_device::bisb_rgd_in(UINT16 op)    { m_icount -= 15+12; { BISB_M(RGD,IN);  } }
void t11_device::bisb_rgd_ind(UINT16 op)   { m_icount -= 15+18; { BISB_M(RGD,IND); } }
void t11_device::bisb_rgd_de(UINT16 op)    { m_icount -= 15+15; { BISB_M(RGD,DE);  } }
void t11_device::bisb_rgd_ded(UINT16 op)   { m_icount -= 15+21; { BISB_M(RGD,DED); } }
void t11_device::bisb_rgd_ix(UINT16 op)    { m_icount -= 15+21; { BISB_M(RGD,IX);  } }
void t11_device::bisb_rgd_ixd(UINT16 op)   { m_icount -= 15+27; { BISB_M(RGD,IXD); } }
void t11_device::bisb_in_rg(UINT16 op)     { m_icount -= 15+ 3; { BISB_X(IN,RG);   } }
void t11_device::bisb_in_rgd(UINT16 op)    { m_icount -= 15+12; { BISB_M(IN,RGD);  } }
void t11_device::bisb_in_in(UINT16 op)     { m_icount -= 15+12; { BISB_M(IN,IN);   } }
void t11_device::bisb_in_ind(UINT16 op)    { m_icount -= 15+18; { BISB_M(IN,IND);  } }
void t11_device::bisb_in_de(UINT16 op)     { m_icount -= 15+15; { BISB_M(IN,DE);   } }
void t11_device::bisb_in_ded(UINT16 op)    { m_icount -= 15+21; { BISB_M(IN,DED);  } }
void t11_device::bisb_in_ix(UINT16 op)     { m_icount -= 15+21; { BISB_M(IN,IX);   } }
void t11_device::bisb_in_ixd(UINT16 op)    { m_icount -= 15+27; { BISB_M(IN,IXD);  } }
void t11_device::bisb_ind_rg(UINT16 op)    { m_icount -= 21+ 3; { BISB_X(IND,RG);  } }
void t11_device::bisb_ind_rgd(UINT16 op)   { m_icount -= 21+12; { BISB_M(IND,RGD); } }
void t11_device::bisb_ind_in(UINT16 op)    { m_icount -= 21+12; { BISB_M(IND,IN);  } }
void t11_device::bisb_ind_ind(UINT16 op)   { m_icount -= 21+18; { BISB_M(IND,IND); } }
void t11_device::bisb_ind_de(UINT16 op)    { m_icount -= 21+15; { BISB_M(IND,DE);  } }
void t11_device::bisb_ind_ded(UINT16 op)   { m_icount -= 21+21; { BISB_M(IND,DED); } }
void t11_device::bisb_ind_ix(UINT16 op)    { m_icount -= 21+21; { BISB_M(IND,IX);  } }
void t11_device::bisb_ind_ixd(UINT16 op)   { m_icount -= 21+27; { BISB_M(IND,IXD); } }
void t11_device::bisb_de_rg(UINT16 op)     { m_icount -= 18+ 3; { BISB_X(DE,RG);   } }
void t11_device::bisb_de_rgd(UINT16 op)    { m_icount -= 18+12; { BISB_M(DE,RGD);  } }
void t11_device::bisb_de_in(UINT16 op)     { m_icount -= 18+12; { BISB_M(DE,IN);   } }
void t11_device::bisb_de_ind(UINT16 op)    { m_icount -= 18+18; { BISB_M(DE,IND);  } }
void t11_device::bisb_de_de(UINT16 op)     { m_icount -= 18+15; { BISB_M(DE,DE);   } }
void t11_device::bisb_de_ded(UINT16 op)    { m_icount -= 18+21; { BISB_M(DE,DED);  } }
void t11_device::bisb_de_ix(UINT16 op)     { m_icount -= 18+21; { BISB_M(DE,IX);   } }
void t11_device::bisb_de_ixd(UINT16 op)    { m_icount -= 18+27; { BISB_M(DE,IXD);  } }
void t11_device::bisb_ded_rg(UINT16 op)    { m_icount -= 24+ 3; { BISB_X(DED,RG);  } }
void t11_device::bisb_ded_rgd(UINT16 op)   { m_icount -= 24+12; { BISB_M(DED,RGD); } }
void t11_device::bisb_ded_in(UINT16 op)    { m_icount -= 24+12; { BISB_M(DED,IN);  } }
void t11_device::bisb_ded_ind(UINT16 op)   { m_icount -= 24+18; { BISB_M(DED,IND); } }
void t11_device::bisb_ded_de(UINT16 op)    { m_icount -= 24+15; { BISB_M(DED,DE);  } }
void t11_device::bisb_ded_ded(UINT16 op)   { m_icount -= 24+21; { BISB_M(DED,DED); } }
void t11_device::bisb_ded_ix(UINT16 op)    { m_icount -= 24+21; { BISB_M(DED,IX);  } }
void t11_device::bisb_ded_ixd(UINT16 op)   { m_icount -= 24+27; { BISB_M(DED,IXD); } }
void t11_device::bisb_ix_rg(UINT16 op)     { m_icount -= 24+ 3; { BISB_X(IX,RG);   } }
void t11_device::bisb_ix_rgd(UINT16 op)    { m_icount -= 24+12; { BISB_M(IX,RGD);  } }
void t11_device::bisb_ix_in(UINT16 op)     { m_icount -= 24+12; { BISB_M(IX,IN);   } }
void t11_device::bisb_ix_ind(UINT16 op)    { m_icount -= 24+18; { BISB_M(IX,IND);  } }
void t11_device::bisb_ix_de(UINT16 op)     { m_icount -= 24+15; { BISB_M(IX,DE);   } }
void t11_device::bisb_ix_ded(UINT16 op)    { m_icount -= 24+21; { BISB_M(IX,DED);  } }
void t11_device::bisb_ix_ix(UINT16 op)     { m_icount -= 24+21; { BISB_M(IX,IX);   } }
void t11_device::bisb_ix_ixd(UINT16 op)    { m_icount -= 24+27; { BISB_M(IX,IXD);  } }
void t11_device::bisb_ixd_rg(UINT16 op)    { m_icount -= 30+ 3; { BISB_X(IXD,RG);  } }
void t11_device::bisb_ixd_rgd(UINT16 op)   { m_icount -= 30+12; { BISB_M(IXD,RGD); } }
void t11_device::bisb_ixd_in(UINT16 op)    { m_icount -= 30+12; { BISB_M(IXD,IN);  } }
void t11_device::bisb_ixd_ind(UINT16 op)   { m_icount -= 30+18; { BISB_M(IXD,IND); } }
void t11_device::bisb_ixd_de(UINT16 op)    { m_icount -= 30+15; { BISB_M(IXD,DE);  } }
void t11_device::bisb_ixd_ded(UINT16 op)   { m_icount -= 30+21; { BISB_M(IXD,DED); } }
void t11_device::bisb_ixd_ix(UINT16 op)    { m_icount -= 30+21; { BISB_M(IXD,IX);  } }
void t11_device::bisb_ixd_ixd(UINT16 op)   { m_icount -= 30+27; { BISB_M(IXD,IXD); } }

void t11_device::sub_rg_rg(UINT16 op)     { m_icount -=  9+ 3; { SUB_R(RG,RG);   } }
void t11_device::sub_rg_rgd(UINT16 op)    { m_icount -=  9+12; { SUB_M(RG,RGD);  } }
void t11_device::sub_rg_in(UINT16 op)     { m_icount -=  9+12; { SUB_M(RG,IN);   } }
void t11_device::sub_rg_ind(UINT16 op)    { m_icount -=  9+18; { SUB_M(RG,IND);  } }
void t11_device::sub_rg_de(UINT16 op)     { m_icount -=  9+15; { SUB_M(RG,DE);   } }
void t11_device::sub_rg_ded(UINT16 op)    { m_icount -=  9+21; { SUB_M(RG,DED);  } }
void t11_device::sub_rg_ix(UINT16 op)     { m_icount -=  9+21; { SUB_M(RG,IX);   } }
void t11_device::sub_rg_ixd(UINT16 op)    { m_icount -=  9+27; { SUB_M(RG,IXD);  } }
void t11_device::sub_rgd_rg(UINT16 op)    { m_icount -= 15+ 3; { SUB_X(RGD,RG);  } }
void t11_device::sub_rgd_rgd(UINT16 op)   { m_icount -= 15+12; { SUB_M(RGD,RGD); } }
void t11_device::sub_rgd_in(UINT16 op)    { m_icount -= 15+12; { SUB_M(RGD,IN);  } }
void t11_device::sub_rgd_ind(UINT16 op)   { m_icount -= 15+18; { SUB_M(RGD,IND); } }
void t11_device::sub_rgd_de(UINT16 op)    { m_icount -= 15+15; { SUB_M(RGD,DE);  } }
void t11_device::sub_rgd_ded(UINT16 op)   { m_icount -= 15+21; { SUB_M(RGD,DED); } }
void t11_device::sub_rgd_ix(UINT16 op)    { m_icount -= 15+21; { SUB_M(RGD,IX);  } }
void t11_device::sub_rgd_ixd(UINT16 op)   { m_icount -= 15+27; { SUB_M(RGD,IXD); } }
void t11_device::sub_in_rg(UINT16 op)     { m_icount -= 15+ 3; { SUB_X(IN,RG);   } }
void t11_device::sub_in_rgd(UINT16 op)    { m_icount -= 15+12; { SUB_M(IN,RGD);  } }
void t11_device::sub_in_in(UINT16 op)     { m_icount -= 15+12; { SUB_M(IN,IN);   } }
void t11_device::sub_in_ind(UINT16 op)    { m_icount -= 15+18; { SUB_M(IN,IND);  } }
void t11_device::sub_in_de(UINT16 op)     { m_icount -= 15+15; { SUB_M(IN,DE);   } }
void t11_device::sub_in_ded(UINT16 op)    { m_icount -= 15+21; { SUB_M(IN,DED);  } }
void t11_device::sub_in_ix(UINT16 op)     { m_icount -= 15+21; { SUB_M(IN,IX);   } }
void t11_device::sub_in_ixd(UINT16 op)    { m_icount -= 15+27; { SUB_M(IN,IXD);  } }
void t11_device::sub_ind_rg(UINT16 op)    { m_icount -= 21+ 3; { SUB_X(IND,RG);  } }
void t11_device::sub_ind_rgd(UINT16 op)   { m_icount -= 21+12; { SUB_M(IND,RGD); } }
void t11_device::sub_ind_in(UINT16 op)    { m_icount -= 21+12; { SUB_M(IND,IN);  } }
void t11_device::sub_ind_ind(UINT16 op)   { m_icount -= 21+18; { SUB_M(IND,IND); } }
void t11_device::sub_ind_de(UINT16 op)    { m_icount -= 21+15; { SUB_M(IND,DE);  } }
void t11_device::sub_ind_ded(UINT16 op)   { m_icount -= 21+21; { SUB_M(IND,DED); } }
void t11_device::sub_ind_ix(UINT16 op)    { m_icount -= 21+21; { SUB_M(IND,IX);  } }
void t11_device::sub_ind_ixd(UINT16 op)   { m_icount -= 21+27; { SUB_M(IND,IXD); } }
void t11_device::sub_de_rg(UINT16 op)     { m_icount -= 18+ 3; { SUB_X(DE,RG);   } }
void t11_device::sub_de_rgd(UINT16 op)    { m_icount -= 18+12; { SUB_M(DE,RGD);  } }
void t11_device::sub_de_in(UINT16 op)     { m_icount -= 18+12; { SUB_M(DE,IN);   } }
void t11_device::sub_de_ind(UINT16 op)    { m_icount -= 18+18; { SUB_M(DE,IND);  } }
void t11_device::sub_de_de(UINT16 op)     { m_icount -= 18+15; { SUB_M(DE,DE);   } }
void t11_device::sub_de_ded(UINT16 op)    { m_icount -= 18+21; { SUB_M(DE,DED);  } }
void t11_device::sub_de_ix(UINT16 op)     { m_icount -= 18+21; { SUB_M(DE,IX);   } }
void t11_device::sub_de_ixd(UINT16 op)    { m_icount -= 18+27; { SUB_M(DE,IXD);  } }
void t11_device::sub_ded_rg(UINT16 op)    { m_icount -= 24+ 3; { SUB_X(DED,RG);  } }
void t11_device::sub_ded_rgd(UINT16 op)   { m_icount -= 24+12; { SUB_M(DED,RGD); } }
void t11_device::sub_ded_in(UINT16 op)    { m_icount -= 24+12; { SUB_M(DED,IN);  } }
void t11_device::sub_ded_ind(UINT16 op)   { m_icount -= 24+18; { SUB_M(DED,IND); } }
void t11_device::sub_ded_de(UINT16 op)    { m_icount -= 24+15; { SUB_M(DED,DE);  } }
void t11_device::sub_ded_ded(UINT16 op)   { m_icount -= 24+21; { SUB_M(DED,DED); } }
void t11_device::sub_ded_ix(UINT16 op)    { m_icount -= 24+21; { SUB_M(DED,IX);  } }
void t11_device::sub_ded_ixd(UINT16 op)   { m_icount -= 24+27; { SUB_M(DED,IXD); } }
void t11_device::sub_ix_rg(UINT16 op)     { m_icount -= 24+ 3; { SUB_X(IX,RG);   } }
void t11_device::sub_ix_rgd(UINT16 op)    { m_icount -= 24+12; { SUB_M(IX,RGD);  } }
void t11_device::sub_ix_in(UINT16 op)     { m_icount -= 24+12; { SUB_M(IX,IN);   } }
void t11_device::sub_ix_ind(UINT16 op)    { m_icount -= 24+18; { SUB_M(IX,IND);  } }
void t11_device::sub_ix_de(UINT16 op)     { m_icount -= 24+15; { SUB_M(IX,DE);   } }
void t11_device::sub_ix_ded(UINT16 op)    { m_icount -= 24+21; { SUB_M(IX,DED);  } }
void t11_device::sub_ix_ix(UINT16 op)     { m_icount -= 24+21; { SUB_M(IX,IX);   } }
void t11_device::sub_ix_ixd(UINT16 op)    { m_icount -= 24+27; { SUB_M(IX,IXD);  } }
void t11_device::sub_ixd_rg(UINT16 op)    { m_icount -= 30+ 3; { SUB_X(IXD,RG);  } }
void t11_device::sub_ixd_rgd(UINT16 op)   { m_icount -= 30+12; { SUB_M(IXD,RGD); } }
void t11_device::sub_ixd_in(UINT16 op)    { m_icount -= 30+12; { SUB_M(IXD,IN);  } }
void t11_device::sub_ixd_ind(UINT16 op)   { m_icount -= 30+18; { SUB_M(IXD,IND); } }
void t11_device::sub_ixd_de(UINT16 op)    { m_icount -= 30+15; { SUB_M(IXD,DE);  } }
void t11_device::sub_ixd_ded(UINT16 op)   { m_icount -= 30+21; { SUB_M(IXD,DED); } }
void t11_device::sub_ixd_ix(UINT16 op)    { m_icount -= 30+21; { SUB_M(IXD,IX);  } }
void t11_device::sub_ixd_ixd(UINT16 op)   { m_icount -= 30+27; { SUB_M(IXD,IXD); } }
