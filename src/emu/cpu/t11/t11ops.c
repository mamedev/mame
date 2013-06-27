/*** t11: Portable DEC T-11 emulator ******************************************

    Copyright Aaron Giles

    Actual opcode implementation.  Excuse the excessive use of macros, it
    was the only way I could bear to type all this in!

*****************************************************************************/


/* given a register index 'r', this computes the effective address for a byte-sized operation
   and puts the result in 'ea' */
#define MAKE_EAB_RGD(r) ea = cpustate->REGD(r)
#define MAKE_EAB_IN(r)  ea = cpustate->REGD(r); cpustate->REGW(r) += ((r) < 6 ? 1 : 2)
#define MAKE_EAB_INS(r) ea = cpustate->REGD(r); cpustate->REGW(r) += ((r) < 6 ? 1 : 2)
#define MAKE_EAB_IND(r) ea = cpustate->REGD(r); cpustate->REGW(r) += 2; ea = RWORD(cpustate, ea)
#define MAKE_EAB_DE(r)  cpustate->REGW(r) -= ((r) < 6 ? 1 : 2); ea = cpustate->REGD(r)
#define MAKE_EAB_DED(r) cpustate->REGW(r) -= 2; ea = cpustate->REGD(r); ea = RWORD(cpustate, ea)
#define MAKE_EAB_IX(r)  ea = ROPCODE(cpustate); ea = (ea + cpustate->REGD(r)) & 0xffff
#define MAKE_EAB_IXD(r) ea = ROPCODE(cpustate); ea = (ea + cpustate->REGD(r)) & 0xffff; ea = RWORD(cpustate, ea)

/* given a register index 'r', this computes the effective address for a word-sized operation
   and puts the result in 'ea' */
/* note that word accesses ignore the low bit!! this fixes APB! */
#define MAKE_EAW_RGD(r) MAKE_EAB_RGD(r)
#define MAKE_EAW_IN(r)  ea = cpustate->REGD(r); cpustate->REGW(r) += 2
#define MAKE_EAW_IND(r) MAKE_EAB_IND(r)
#define MAKE_EAW_DE(r)  cpustate->REGW(r) -= 2; ea = cpustate->REGD(r)
#define MAKE_EAW_DED(r) MAKE_EAB_DED(r)
#define MAKE_EAW_IX(r)  MAKE_EAB_IX(r)
#define MAKE_EAW_IXD(r) MAKE_EAB_IXD(r)

/* extracts the source/destination register index from the opcode into 'sreg' or 'dreg' */
#define GET_SREG sreg = (op >> 6) & 7
#define GET_DREG dreg = op & 7

/* for a byte-sized source operand: extracts 'sreg', computes 'ea', and loads the value into 'source' */
#define GET_SB_RG  GET_SREG; source = cpustate->REGB(sreg)
#define GET_SB_RGD GET_SREG; MAKE_EAB_RGD(sreg); source = RBYTE(cpustate, ea)
#define GET_SB_IN  GET_SREG; if (sreg == 7) { source = ROPCODE(cpustate); } else { MAKE_EAB_IN(sreg); source = RBYTE(cpustate, ea); }
#define GET_SB_IND GET_SREG; if (sreg == 7) { ea = ROPCODE(cpustate); } else { MAKE_EAB_IND(sreg); } source = RBYTE(cpustate, ea)
#define GET_SB_DE  GET_SREG; MAKE_EAB_DE(sreg); source = RBYTE(cpustate, ea)
#define GET_SB_DED GET_SREG; MAKE_EAB_DED(sreg); source = RBYTE(cpustate, ea)
#define GET_SB_IX  GET_SREG; MAKE_EAB_IX(sreg); source = RBYTE(cpustate, ea)
#define GET_SB_IXD GET_SREG; MAKE_EAB_IXD(sreg); source = RBYTE(cpustate, ea)

/* for a word-sized source operand: extracts 'sreg', computes 'ea', and loads the value into 'source' */
#define GET_SW_RG  GET_SREG; source = cpustate->REGD(sreg)
#define GET_SW_RGD GET_SREG; MAKE_EAW_RGD(sreg); source = RWORD(cpustate, ea)
#define GET_SW_IN  GET_SREG; if (sreg == 7) { source = ROPCODE(cpustate); } else { MAKE_EAW_IN(sreg); source = RWORD(cpustate, ea); }
#define GET_SW_IND GET_SREG; if (sreg == 7) { ea = ROPCODE(cpustate); } else { MAKE_EAW_IND(sreg); } source = RWORD(cpustate, ea)
#define GET_SW_DE  GET_SREG; MAKE_EAW_DE(sreg); source = RWORD(cpustate, ea)
#define GET_SW_DED GET_SREG; MAKE_EAW_DED(sreg); source = RWORD(cpustate, ea)
#define GET_SW_IX  GET_SREG; MAKE_EAW_IX(sreg); source = RWORD(cpustate, ea)
#define GET_SW_IXD GET_SREG; MAKE_EAW_IXD(sreg); source = RWORD(cpustate, ea)

/* for a byte-sized destination operand: extracts 'dreg', computes 'ea', and loads the value into 'dest' */
#define GET_DB_RG  GET_DREG; dest = cpustate->REGB(dreg)
#define GET_DB_RGD GET_DREG; MAKE_EAB_RGD(dreg); dest = RBYTE(cpustate, ea)
#define GET_DB_IN  GET_DREG; MAKE_EAB_IN(dreg); dest = RBYTE(cpustate, ea)
#define GET_DB_IND GET_DREG; if (dreg == 7) { ea = ROPCODE(cpustate); } else { MAKE_EAB_IND(dreg); } dest = RBYTE(cpustate, ea)
#define GET_DB_DE  GET_DREG; MAKE_EAB_DE(dreg); dest = RBYTE(cpustate, ea)
#define GET_DB_DED GET_DREG; MAKE_EAB_DED(dreg); dest = RBYTE(cpustate, ea)
#define GET_DB_IX  GET_DREG; MAKE_EAB_IX(dreg); dest = RBYTE(cpustate, ea)
#define GET_DB_IXD GET_DREG; MAKE_EAB_IXD(dreg); dest = RBYTE(cpustate, ea)

/* for a word-sized destination operand: extracts 'dreg', computes 'ea', and loads the value into 'dest' */
#define GET_DW_RG  GET_DREG; dest = cpustate->REGD(dreg)
#define GET_DW_RGD GET_DREG; MAKE_EAW_RGD(dreg); dest = RWORD(cpustate, ea)
#define GET_DW_IN  GET_DREG; MAKE_EAW_IN(dreg); dest = RWORD(cpustate, ea)
#define GET_DW_IND GET_DREG; if (dreg == 7) { ea = ROPCODE(cpustate); } else { MAKE_EAW_IND(dreg); } dest = RWORD(cpustate, ea)
#define GET_DW_DE  GET_DREG; MAKE_EAW_DE(dreg); dest = RWORD(cpustate, ea)
#define GET_DW_DED GET_DREG; MAKE_EAW_DED(dreg); dest = RWORD(cpustate, ea)
#define GET_DW_IX  GET_DREG; MAKE_EAW_IX(dreg); dest = RWORD(cpustate, ea)
#define GET_DW_IXD GET_DREG; MAKE_EAW_IXD(dreg); dest = RWORD(cpustate, ea)

/* writes a value to a previously computed 'ea' */
#define PUT_DB_EA(v) WBYTE(cpustate, ea, (v))
#define PUT_DW_EA(v) WWORD(cpustate, ea, (v))

/* writes a value to a previously computed 'dreg' register */
#define PUT_DB_DREG(v) cpustate->REGB(dreg) = (v)
#define PUT_DW_DREG(v) cpustate->REGW(dreg) = (v)

/* for a byte-sized destination operand: extracts 'dreg', computes 'ea', and writes 'v' to it */
#define PUT_DB_RG(v)  GET_DREG; cpustate->REGB(dreg) = (v)
#define PUT_DB_RGD(v) GET_DREG; MAKE_EAB_RGD(dreg); WBYTE(cpustate, ea, (v))
#define PUT_DB_IN(v)  GET_DREG; MAKE_EAB_IN(dreg); WBYTE(cpustate, ea, (v))
#define PUT_DB_IND(v) GET_DREG; if (dreg == 7) { ea = ROPCODE(cpustate); } else { MAKE_EAB_IND(dreg); } WBYTE(cpustate, ea, (v))
#define PUT_DB_DE(v)  GET_DREG; MAKE_EAB_DE(dreg); WBYTE(cpustate, ea, (v))
#define PUT_DB_DED(v) GET_DREG; MAKE_EAB_DED(dreg); WBYTE(cpustate, ea, (v))
#define PUT_DB_IX(v)  GET_DREG; MAKE_EAB_IX(dreg); WBYTE(cpustate, ea, (v))
#define PUT_DB_IXD(v) GET_DREG; MAKE_EAB_IXD(dreg); WBYTE(cpustate, ea, (v))

/* for a word-sized destination operand: extracts 'dreg', computes 'ea', and writes 'v' to it */
#define PUT_DW_RG(v)  GET_DREG; cpustate->REGW(dreg) = (v)
#define PUT_DW_RGD(v) GET_DREG; MAKE_EAW_RGD(dreg); WWORD(cpustate, ea, (v))
#define PUT_DW_IN(v)  GET_DREG; MAKE_EAW_IN(dreg); WWORD(cpustate, ea, (v))
#define PUT_DW_IND(v) GET_DREG; if (dreg == 7) { ea = ROPCODE(cpustate); } else { MAKE_EAW_IND(dreg); } WWORD(cpustate, ea, (v))
#define PUT_DW_DE(v)  GET_DREG; MAKE_EAW_DE(dreg); WWORD(cpustate, ea, (v))
#define PUT_DW_DED(v) GET_DREG; MAKE_EAW_DED(dreg); WWORD(cpustate, ea, (v))
#define PUT_DW_IX(v)  GET_DREG; MAKE_EAW_IX(dreg); WWORD(cpustate, ea, (v))
#define PUT_DW_IXD(v) GET_DREG; MAKE_EAW_IXD(dreg); WWORD(cpustate, ea, (v))

/* flag clearing; must be done before setting */
#define CLR_ZV   (cpustate->PSW &= ~(ZFLAG | VFLAG))
#define CLR_NZV  (cpustate->PSW &= ~(NFLAG | ZFLAG | VFLAG))
#define CLR_NZVC (cpustate->PSW &= ~(NFLAG | ZFLAG | VFLAG | CFLAG))

/* set individual flags byte-sized */
#define SETB_N (cpustate->PSW |= (result >> 4) & 0x08)
#define SETB_Z (cpustate->PSW |= ((result & 0xff) == 0) << 2)
#define SETB_V (cpustate->PSW |= ((source ^ dest ^ result ^ (result >> 1)) >> 6) & 0x02)
#define SETB_C (cpustate->PSW |= (result >> 8) & 0x01)
#define SETB_NZ SETB_N; SETB_Z
#define SETB_NZV SETB_N; SETB_Z; SETB_V
#define SETB_NZVC SETB_N; SETB_Z; SETB_V; SETB_C

/* set individual flags word-sized */
#define SETW_N (cpustate->PSW |= (result >> 12) & 0x08)
#define SETW_Z (cpustate->PSW |= ((result & 0xffff) == 0) << 2)
#define SETW_V (cpustate->PSW |= ((source ^ dest ^ result ^ (result >> 1)) >> 14) & 0x02)
#define SETW_C (cpustate->PSW |= (result >> 16) & 0x01)
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
#define ASL_R(d)    int dreg, dest, result;     GET_DW_##d; CLR_NZVC; result = dest << 1; SETW_NZ; cpustate->PSW |= (dest >> 15) & 1; cpustate->PSW |= ((cpustate->PSW << 1) ^ (cpustate->PSW >> 2)) & 2; PUT_DW_DREG(result)
#define ASL_M(d)    int dreg, dest, result, ea; GET_DW_##d; CLR_NZVC; result = dest << 1; SETW_NZ; cpustate->PSW |= (dest >> 15) & 1; cpustate->PSW |= ((cpustate->PSW << 1) ^ (cpustate->PSW >> 2)) & 2; PUT_DW_EA(result)
#define ASLB_R(d)   int dreg, dest, result;     GET_DB_##d; CLR_NZVC; result = dest << 1; SETB_NZ; cpustate->PSW |= (dest >> 7) & 1;  cpustate->PSW |= ((cpustate->PSW << 1) ^ (cpustate->PSW >> 2)) & 2; PUT_DB_DREG(result)
#define ASLB_M(d)   int dreg, dest, result, ea; GET_DB_##d; CLR_NZVC; result = dest << 1; SETB_NZ; cpustate->PSW |= (dest >> 7) & 1;  cpustate->PSW |= ((cpustate->PSW << 1) ^ (cpustate->PSW >> 2)) & 2; PUT_DB_EA(result)
/* ASR: dst = (dst << 1); C = (dst >> 7) */
#define ASR_R(d)    int dreg, dest, result;     GET_DW_##d; CLR_NZVC; result = (dest >> 1) | (dest & 0x8000); SETW_NZ; cpustate->PSW |= dest & 1; cpustate->PSW |= ((cpustate->PSW << 1) ^ (cpustate->PSW >> 2)) & 2; PUT_DW_DREG(result)
#define ASR_M(d)    int dreg, dest, result, ea; GET_DW_##d; CLR_NZVC; result = (dest >> 1) | (dest & 0x8000); SETW_NZ; cpustate->PSW |= dest & 1; cpustate->PSW |= ((cpustate->PSW << 1) ^ (cpustate->PSW >> 2)) & 2; PUT_DW_EA(result)
#define ASRB_R(d)   int dreg, dest, result;     GET_DB_##d; CLR_NZVC; result = (dest >> 1) | (dest & 0x80);   SETB_NZ; cpustate->PSW |= dest & 1; cpustate->PSW |= ((cpustate->PSW << 1) ^ (cpustate->PSW >> 2)) & 2; PUT_DB_DREG(result)
#define ASRB_M(d)   int dreg, dest, result, ea; GET_DB_##d; CLR_NZVC; result = (dest >> 1) | (dest & 0x80);   SETB_NZ; cpustate->PSW |= dest & 1; cpustate->PSW |= ((cpustate->PSW << 1) ^ (cpustate->PSW >> 2)) & 2; PUT_DB_EA(result)
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
#define BR(c)       if (c) { cpustate->PC += 2 * (signed char)(op & 0xff); }
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
#define JMP(d)      int dreg, ea; GET_DREG; MAKE_EAW_##d(dreg); cpustate->PC = ea
/* JSR: PUSH src, src = PC, PC = ea */
#define JSR(d)      int sreg, dreg, ea; GET_SREG; GET_DREG; MAKE_EAW_##d(dreg); PUSH(cpustate, cpustate->REGW(sreg)); cpustate->REGW(sreg) = cpustate->PC; cpustate->PC = ea
/* MFPS: dst = flags */
#define MFPS_R(d)   int dreg, result;     result = cpustate->PSW; CLR_NZV; SETB_NZ; PUT_DW_##d((signed char)result)
#define MFPS_M(d)   int dreg, result, ea; result = cpustate->PSW; CLR_NZV; SETB_NZ; PUT_DB_##d(result)
/* MOV: dst = src */
#define MOV_R(s,d)  int sreg, dreg, source, result;     GET_SW_##s; CLR_NZV; result = source; SETW_NZ; PUT_DW_##d(result)
#define MOV_M(s,d)  int sreg, dreg, source, result, ea; GET_SW_##s; CLR_NZV; result = source; SETW_NZ; PUT_DW_##d(result)
#define MOVB_R(s,d) int sreg, dreg, source, result;     GET_SB_##s; CLR_NZV; result = source; SETB_NZ; PUT_DW_##d((signed char)result)
#define MOVB_X(s,d) int sreg, dreg, source, result, ea; GET_SB_##s; CLR_NZV; result = source; SETB_NZ; PUT_DW_##d((signed char)result)
#define MOVB_M(s,d) int sreg, dreg, source, result, ea; GET_SB_##s; CLR_NZV; result = source; SETB_NZ; PUT_DB_##d(result)
/* MTPS: flags = src */
#define MTPS_R(d)   int dreg, dest;     GET_DW_##d; cpustate->PSW = (cpustate->PSW & ~0xef) | (dest & 0xef); t11_check_irqs(cpustate)
#define MTPS_M(d)   int dreg, dest, ea; GET_DW_##d; cpustate->PSW = (cpustate->PSW & ~0xef) | (dest & 0xef); t11_check_irqs(cpustate)
/* NEG: dst = -dst */
#define NEG_R(d)    int dreg, dest, result;     GET_DW_##d; CLR_NZVC; result = -dest; SETW_NZ; if (dest == 0x8000) SET_V; if (result) SET_C; PUT_DW_DREG(result)
#define NEG_M(d)    int dreg, dest, result, ea; GET_DW_##d; CLR_NZVC; result = -dest; SETW_NZ; if (dest == 0x8000) SET_V; if (result) SET_C; PUT_DW_EA(result)
#define NEGB_R(d)   int dreg, dest, result;     GET_DB_##d; CLR_NZVC; result = -dest; SETB_NZ; if (dest == 0x80)   SET_V; if (result) SET_C; PUT_DB_DREG(result)
#define NEGB_M(d)   int dreg, dest, result, ea; GET_DB_##d; CLR_NZVC; result = -dest; SETB_NZ; if (dest == 0x80)   SET_V; if (result) SET_C; PUT_DB_EA(result)
/* ROL: dst = (dst << 1) | C; C = (dst >> 7) */
#define ROL_R(d)    int dreg, dest, result;     GET_DW_##d; result = (dest << 1) | GET_C; CLR_NZVC; SETW_NZ; cpustate->PSW |= (dest >> 15) & 1; cpustate->PSW |= ((cpustate->PSW << 1) ^ (cpustate->PSW >> 2)) & 2; PUT_DW_DREG(result)
#define ROL_M(d)    int dreg, dest, result, ea; GET_DW_##d; result = (dest << 1) | GET_C; CLR_NZVC; SETW_NZ; cpustate->PSW |= (dest >> 15) & 1; cpustate->PSW |= ((cpustate->PSW << 1) ^ (cpustate->PSW >> 2)) & 2; PUT_DW_EA(result)
#define ROLB_R(d)   int dreg, dest, result;     GET_DB_##d; result = (dest << 1) | GET_C; CLR_NZVC; SETB_NZ; cpustate->PSW |= (dest >> 7) & 1;  cpustate->PSW |= ((cpustate->PSW << 1) ^ (cpustate->PSW >> 2)) & 2; PUT_DB_DREG(result)
#define ROLB_M(d)   int dreg, dest, result, ea; GET_DB_##d; result = (dest << 1) | GET_C; CLR_NZVC; SETB_NZ; cpustate->PSW |= (dest >> 7) & 1;  cpustate->PSW |= ((cpustate->PSW << 1) ^ (cpustate->PSW >> 2)) & 2; PUT_DB_EA(result)
/* ROR: dst = (dst >> 1) | (C << 7); C = dst & 1 */
#define ROR_R(d)    int dreg, dest, result;     GET_DW_##d; result = (dest >> 1) | (GET_C << 15); CLR_NZVC; SETW_NZ; cpustate->PSW |= dest & 1; cpustate->PSW |= ((cpustate->PSW << 1) ^ (cpustate->PSW >> 2)) & 2; PUT_DW_DREG(result)
#define ROR_M(d)    int dreg, dest, result, ea; GET_DW_##d; result = (dest >> 1) | (GET_C << 15); CLR_NZVC; SETW_NZ; cpustate->PSW |= dest & 1; cpustate->PSW |= ((cpustate->PSW << 1) ^ (cpustate->PSW >> 2)) & 2; PUT_DW_EA(result)
#define RORB_R(d)   int dreg, dest, result;     GET_DB_##d; result = (dest >> 1) | (GET_C << 7);  CLR_NZVC; SETB_NZ; cpustate->PSW |= dest & 1; cpustate->PSW |= ((cpustate->PSW << 1) ^ (cpustate->PSW >> 2)) & 2; PUT_DB_DREG(result)
#define RORB_M(d)   int dreg, dest, result, ea; GET_DB_##d; result = (dest >> 1) | (GET_C << 7);  CLR_NZVC; SETB_NZ; cpustate->PSW |= dest & 1; cpustate->PSW |= ((cpustate->PSW << 1) ^ (cpustate->PSW >> 2)) & 2; PUT_DB_EA(result)
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
#define XOR_R(d)    int sreg, dreg, source, dest, result;     GET_SREG; source = cpustate->REGW(sreg); GET_DW_##d; CLR_NZV; result = dest ^ source; SETW_NZ; PUT_DW_DREG(result)
#define XOR_M(d)    int sreg, dreg, source, dest, result, ea; GET_SREG; source = cpustate->REGW(sreg); GET_DW_##d; CLR_NZV; result = dest ^ source; SETW_NZ; PUT_DW_EA(result)



static void op_0000(t11_state *cpustate, UINT16 op)
{
	switch (op & 0x3f)
	{
		case 0x00:  /* HALT  */ halt(cpustate, op); break;
		case 0x01:  /* WAIT  */ cpustate->icount = 0; cpustate->wait_state = 1; break;
		case 0x02:  /* RTI   */ cpustate->icount -= 24; cpustate->PC = POP(cpustate); cpustate->PSW = POP(cpustate); t11_check_irqs(cpustate); break;
		case 0x03:  /* BPT   */ cpustate->icount -= 48; PUSH(cpustate, cpustate->PSW); PUSH(cpustate, cpustate->PC); cpustate->PC = RWORD(cpustate, 0x0c); cpustate->PSW = RWORD(cpustate, 0x0e); t11_check_irqs(cpustate); break;
		case 0x04:  /* IOT   */ cpustate->icount -= 48; PUSH(cpustate, cpustate->PSW); PUSH(cpustate, cpustate->PC); cpustate->PC = RWORD(cpustate, 0x10); cpustate->PSW = RWORD(cpustate, 0x12); t11_check_irqs(cpustate); break;
		case 0x05:  /* RESET */ cpustate->icount -= 110; break;
		case 0x06:  /* RTT   */ cpustate->icount -= 33; cpustate->PC = POP(cpustate); cpustate->PSW = POP(cpustate); t11_check_irqs(cpustate); break;
		default:    illegal(cpustate, op); break;
	}
}

static void halt(t11_state *cpustate, UINT16 op)
{
	cpustate->icount -= 48;
	PUSH(cpustate, cpustate->PSW);
	PUSH(cpustate, cpustate->PC);
	cpustate->PC = RWORD(cpustate, 0x04);
	cpustate->PSW = RWORD(cpustate, 0x06);
	t11_check_irqs(cpustate);
}

static void illegal(t11_state *cpustate, UINT16 op)
{
	cpustate->icount -= 48;
	PUSH(cpustate, cpustate->PSW);
	PUSH(cpustate, cpustate->PC);
	cpustate->PC = RWORD(cpustate, 0x08);
	cpustate->PSW = RWORD(cpustate, 0x0a);
	t11_check_irqs(cpustate);
}

static void mark(t11_state *cpustate, UINT16 op)
{
	cpustate->icount -= 36;

	cpustate->SP = cpustate->SP + 2 * (op & 0x3f);
	cpustate->PC = cpustate->REGW(5);
	cpustate->REGW(5) = POP(cpustate);
}

static void jmp_rgd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 15; { JMP(RGD); } }
static void jmp_in(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 18; { JMP(IN);  } }
static void jmp_ind(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 18; { JMP(IND); } }
static void jmp_de(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 18; { JMP(DE);  } }
static void jmp_ded(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 21; { JMP(DED); } }
static void jmp_ix(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 21; { JMP(IX);  } }
static void jmp_ixd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 27; { JMP(IXD); } }

static void rts(t11_state *cpustate, UINT16 op)
{
	int dreg;
	cpustate->icount -= 21;
	GET_DREG;
	cpustate->PC = cpustate->REGD(dreg);
	cpustate->REGW(dreg) = POP(cpustate);
}

static void ccc(t11_state *cpustate, UINT16 op)         { cpustate->icount -= 18; { cpustate->PSW &= ~(op & 15); } }
static void scc(t11_state *cpustate, UINT16 op)         { cpustate->icount -= 18; { cpustate->PSW |=  (op & 15); } }

static void swab_rg(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 12; { SWAB_R(RG); } }
static void swab_rgd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 21; { SWAB_M(RGD); } }
static void swab_in(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 21; { SWAB_M(IN); } }
static void swab_ind(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 27; { SWAB_M(IND); } }
static void swab_de(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 24; { SWAB_M(DE); } }
static void swab_ded(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 30; { SWAB_M(DED); } }
static void swab_ix(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 30; { SWAB_M(IX); } }
static void swab_ixd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 36; { SWAB_M(IXD); } }

static void br(t11_state *cpustate, UINT16 op)            { cpustate->icount -= 12; { BR(1); } }
static void bne(t11_state *cpustate, UINT16 op)           { cpustate->icount -= 12; { BR(!GET_Z); } }
static void beq(t11_state *cpustate, UINT16 op)           { cpustate->icount -= 12; { BR( GET_Z); } }
static void bge(t11_state *cpustate, UINT16 op)           { cpustate->icount -= 12; { BR(!((GET_N >> 2) ^ GET_V)); } }
static void blt(t11_state *cpustate, UINT16 op)           { cpustate->icount -= 12; { BR(((GET_N >> 2) ^ GET_V)); } }
static void bgt(t11_state *cpustate, UINT16 op)           { cpustate->icount -= 12; { BR(!GET_Z && !((GET_N >> 2) ^ GET_V)); } }
static void ble(t11_state *cpustate, UINT16 op)           { cpustate->icount -= 12; { BR( GET_Z || ((GET_N >> 2) ^ GET_V)); } }

static void jsr_rgd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 27; { JSR(RGD); } }
static void jsr_in(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 30; { JSR(IN);  } }
static void jsr_ind(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 30; { JSR(IND); } }
static void jsr_de(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 30; { JSR(DE);  } }
static void jsr_ded(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 33; { JSR(DED); } }
static void jsr_ix(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 33; { JSR(IX);  } }
static void jsr_ixd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 39; { JSR(IXD); } }

static void clr_rg(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 12; { CLR_R(RG);  } }
static void clr_rgd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 21; { CLR_M(RGD); } }
static void clr_in(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 21; { CLR_M(IN);  } }
static void clr_ind(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 27; { CLR_M(IND); } }
static void clr_de(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 24; { CLR_M(DE);  } }
static void clr_ded(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 30; { CLR_M(DED); } }
static void clr_ix(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 30; { CLR_M(IX);  } }
static void clr_ixd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 36; { CLR_M(IXD); } }

static void com_rg(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 12; { COM_R(RG);  } }
static void com_rgd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 21; { COM_M(RGD); } }
static void com_in(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 21; { COM_M(IN);  } }
static void com_ind(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 27; { COM_M(IND); } }
static void com_de(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 24; { COM_M(DE);  } }
static void com_ded(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 30; { COM_M(DED); } }
static void com_ix(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 30; { COM_M(IX);  } }
static void com_ixd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 36; { COM_M(IXD); } }

static void inc_rg(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 12; { INC_R(RG);  } }
static void inc_rgd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 21; { INC_M(RGD); } }
static void inc_in(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 21; { INC_M(IN);  } }
static void inc_ind(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 27; { INC_M(IND); } }
static void inc_de(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 24; { INC_M(DE);  } }
static void inc_ded(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 30; { INC_M(DED); } }
static void inc_ix(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 30; { INC_M(IX);  } }
static void inc_ixd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 36; { INC_M(IXD); } }

static void dec_rg(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 12; { DEC_R(RG);  } }
static void dec_rgd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 21; { DEC_M(RGD); } }
static void dec_in(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 21; { DEC_M(IN);  } }
static void dec_ind(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 27; { DEC_M(IND); } }
static void dec_de(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 24; { DEC_M(DE);  } }
static void dec_ded(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 30; { DEC_M(DED); } }
static void dec_ix(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 30; { DEC_M(IX);  } }
static void dec_ixd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 36; { DEC_M(IXD); } }

static void neg_rg(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 12; { NEG_R(RG);  } }
static void neg_rgd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 21; { NEG_M(RGD); } }
static void neg_in(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 21; { NEG_M(IN);  } }
static void neg_ind(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 27; { NEG_M(IND); } }
static void neg_de(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 24; { NEG_M(DE);  } }
static void neg_ded(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 30; { NEG_M(DED); } }
static void neg_ix(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 30; { NEG_M(IX);  } }
static void neg_ixd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 36; { NEG_M(IXD); } }

static void adc_rg(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 12; { ADC_R(RG);  } }
static void adc_rgd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 21; { ADC_M(RGD); } }
static void adc_in(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 21; { ADC_M(IN);  } }
static void adc_ind(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 27; { ADC_M(IND); } }
static void adc_de(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 24; { ADC_M(DE);  } }
static void adc_ded(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 30; { ADC_M(DED); } }
static void adc_ix(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 30; { ADC_M(IX);  } }
static void adc_ixd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 36; { ADC_M(IXD); } }

static void sbc_rg(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 12; { SBC_R(RG);  } }
static void sbc_rgd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 21; { SBC_M(RGD); } }
static void sbc_in(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 21; { SBC_M(IN);  } }
static void sbc_ind(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 27; { SBC_M(IND); } }
static void sbc_de(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 24; { SBC_M(DE);  } }
static void sbc_ded(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 30; { SBC_M(DED); } }
static void sbc_ix(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 30; { SBC_M(IX);  } }
static void sbc_ixd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 36; { SBC_M(IXD); } }

static void tst_rg(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 12; { TST_R(RG);  } }
static void tst_rgd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 18; { TST_M(RGD); } }
static void tst_in(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 18; { TST_M(IN);  } }
static void tst_ind(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 24; { TST_M(IND); } }
static void tst_de(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 21; { TST_M(DE);  } }
static void tst_ded(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 27; { TST_M(DED); } }
static void tst_ix(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 27; { TST_M(IX);  } }
static void tst_ixd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 33; { TST_M(IXD); } }

static void ror_rg(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 12; { ROR_R(RG);  } }
static void ror_rgd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 21; { ROR_M(RGD); } }
static void ror_in(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 21; { ROR_M(IN);  } }
static void ror_ind(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 27; { ROR_M(IND); } }
static void ror_de(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 24; { ROR_M(DE);  } }
static void ror_ded(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 30; { ROR_M(DED); } }
static void ror_ix(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 30; { ROR_M(IX);  } }
static void ror_ixd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 36; { ROR_M(IXD); } }

static void rol_rg(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 12; { ROL_R(RG);  } }
static void rol_rgd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 21; { ROL_M(RGD); } }
static void rol_in(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 21; { ROL_M(IN);  } }
static void rol_ind(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 27; { ROL_M(IND); } }
static void rol_de(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 24; { ROL_M(DE);  } }
static void rol_ded(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 30; { ROL_M(DED); } }
static void rol_ix(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 30; { ROL_M(IX);  } }
static void rol_ixd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 36; { ROL_M(IXD); } }

static void asr_rg(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 12; { ASR_R(RG);  } }
static void asr_rgd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 21; { ASR_M(RGD); } }
static void asr_in(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 21; { ASR_M(IN);  } }
static void asr_ind(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 27; { ASR_M(IND); } }
static void asr_de(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 24; { ASR_M(DE);  } }
static void asr_ded(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 30; { ASR_M(DED); } }
static void asr_ix(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 30; { ASR_M(IX);  } }
static void asr_ixd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 36; { ASR_M(IXD); } }

static void asl_rg(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 12; { ASL_R(RG);  } }
static void asl_rgd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 21; { ASL_M(RGD); } }
static void asl_in(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 21; { ASL_M(IN);  } }
static void asl_ind(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 27; { ASL_M(IND); } }
static void asl_de(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 24; { ASL_M(DE);  } }
static void asl_ded(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 30; { ASL_M(DED); } }
static void asl_ix(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 30; { ASL_M(IX);  } }
static void asl_ixd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 36; { ASL_M(IXD); } }

static void sxt_rg(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 12; { SXT_R(RG);  } }
static void sxt_rgd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 21; { SXT_M(RGD); } }
static void sxt_in(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 21; { SXT_M(IN);  } }
static void sxt_ind(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 27; { SXT_M(IND); } }
static void sxt_de(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 24; { SXT_M(DE);  } }
static void sxt_ded(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 30; { SXT_M(DED); } }
static void sxt_ix(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 30; { SXT_M(IX);  } }
static void sxt_ixd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 36; { SXT_M(IXD); } }

static void mov_rg_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+ 3; { MOV_R(RG,RG);   } }
static void mov_rg_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+12; { MOV_M(RG,RGD);  } }
static void mov_rg_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+12; { MOV_M(RG,IN);   } }
static void mov_rg_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+18; { MOV_M(RG,IND);  } }
static void mov_rg_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+15; { MOV_M(RG,DE);   } }
static void mov_rg_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+21; { MOV_M(RG,DED);  } }
static void mov_rg_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+21; { MOV_M(RG,IX);   } }
static void mov_rg_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+27; { MOV_M(RG,IXD);  } }
static void mov_rgd_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+ 3; { MOV_M(RGD,RG);  } }
static void mov_rgd_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+12; { MOV_M(RGD,RGD); } }
static void mov_rgd_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+12; { MOV_M(RGD,IN);  } }
static void mov_rgd_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+18; { MOV_M(RGD,IND); } }
static void mov_rgd_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+15; { MOV_M(RGD,DE);  } }
static void mov_rgd_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+21; { MOV_M(RGD,DED); } }
static void mov_rgd_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+21; { MOV_M(RGD,IX);  } }
static void mov_rgd_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+27; { MOV_M(RGD,IXD); } }
static void mov_in_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+ 3; { MOV_M(IN,RG);   } }
static void mov_in_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+12; { MOV_M(IN,RGD);  } }
static void mov_in_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+12; { MOV_M(IN,IN);   } }
static void mov_in_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+18; { MOV_M(IN,IND);  } }
static void mov_in_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+15; { MOV_M(IN,DE);   } }
static void mov_in_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+21; { MOV_M(IN,DED);  } }
static void mov_in_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+21; { MOV_M(IN,IX);   } }
static void mov_in_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+27; { MOV_M(IN,IXD);  } }
static void mov_ind_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+ 3; { MOV_M(IND,RG);  } }
static void mov_ind_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+12; { MOV_M(IND,RGD); } }
static void mov_ind_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+12; { MOV_M(IND,IN);  } }
static void mov_ind_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+18; { MOV_M(IND,IND); } }
static void mov_ind_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+15; { MOV_M(IND,DE);  } }
static void mov_ind_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+21; { MOV_M(IND,DED); } }
static void mov_ind_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+21; { MOV_M(IND,IX);  } }
static void mov_ind_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+27; { MOV_M(IND,IXD); } }
static void mov_de_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+ 3; { MOV_M(DE,RG);   } }
static void mov_de_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+12; { MOV_M(DE,RGD);  } }
static void mov_de_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+12; { MOV_M(DE,IN);   } }
static void mov_de_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+18; { MOV_M(DE,IND);  } }
static void mov_de_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+15; { MOV_M(DE,DE);   } }
static void mov_de_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+21; { MOV_M(DE,DED);  } }
static void mov_de_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+21; { MOV_M(DE,IX);   } }
static void mov_de_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+27; { MOV_M(DE,IXD);  } }
static void mov_ded_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+ 3; { MOV_M(DED,RG);  } }
static void mov_ded_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+12; { MOV_M(DED,RGD); } }
static void mov_ded_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+12; { MOV_M(DED,IN);  } }
static void mov_ded_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+18; { MOV_M(DED,IND); } }
static void mov_ded_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+15; { MOV_M(DED,DE);  } }
static void mov_ded_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+21; { MOV_M(DED,DED); } }
static void mov_ded_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+21; { MOV_M(DED,IX);  } }
static void mov_ded_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+27; { MOV_M(DED,IXD); } }
static void mov_ix_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+ 3; { MOV_M(IX,RG);   } }
static void mov_ix_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+12; { MOV_M(IX,RGD);  } }
static void mov_ix_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+12; { MOV_M(IX,IN);   } }
static void mov_ix_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+18; { MOV_M(IX,IND);  } }
static void mov_ix_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+15; { MOV_M(IX,DE);   } }
static void mov_ix_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+21; { MOV_M(IX,DED);  } }
static void mov_ix_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+21; { MOV_M(IX,IX);   } }
static void mov_ix_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+27; { MOV_M(IX,IXD);  } }
static void mov_ixd_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+ 3; { MOV_M(IXD,RG);  } }
static void mov_ixd_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+12; { MOV_M(IXD,RGD); } }
static void mov_ixd_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+12; { MOV_M(IXD,IN);  } }
static void mov_ixd_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+18; { MOV_M(IXD,IND); } }
static void mov_ixd_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+15; { MOV_M(IXD,DE);  } }
static void mov_ixd_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+21; { MOV_M(IXD,DED); } }
static void mov_ixd_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+21; { MOV_M(IXD,IX);  } }
static void mov_ixd_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+27; { MOV_M(IXD,IXD); } }

static void cmp_rg_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+ 3; { CMP_R(RG,RG);   } }
static void cmp_rg_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+ 9; { CMP_M(RG,RGD);  } }
static void cmp_rg_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+ 9; { CMP_M(RG,IN);   } }
static void cmp_rg_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+15; { CMP_M(RG,IND);  } }
static void cmp_rg_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+12; { CMP_M(RG,DE);   } }
static void cmp_rg_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+18; { CMP_M(RG,DED);  } }
static void cmp_rg_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+18; { CMP_M(RG,IX);   } }
static void cmp_rg_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+24; { CMP_M(RG,IXD);  } }
static void cmp_rgd_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+ 3; { CMP_M(RGD,RG);  } }
static void cmp_rgd_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+ 9; { CMP_M(RGD,RGD); } }
static void cmp_rgd_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+ 9; { CMP_M(RGD,IN);  } }
static void cmp_rgd_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+15; { CMP_M(RGD,IND); } }
static void cmp_rgd_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+12; { CMP_M(RGD,DE);  } }
static void cmp_rgd_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+18; { CMP_M(RGD,DED); } }
static void cmp_rgd_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+18; { CMP_M(RGD,IX);  } }
static void cmp_rgd_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+24; { CMP_M(RGD,IXD); } }
static void cmp_in_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+ 3; { CMP_M(IN,RG);   } }
static void cmp_in_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+ 9; { CMP_M(IN,RGD);  } }
static void cmp_in_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+ 9; { CMP_M(IN,IN);   } }
static void cmp_in_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+15; { CMP_M(IN,IND);  } }
static void cmp_in_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+12; { CMP_M(IN,DE);   } }
static void cmp_in_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+18; { CMP_M(IN,DED);  } }
static void cmp_in_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+18; { CMP_M(IN,IX);   } }
static void cmp_in_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+24; { CMP_M(IN,IXD);  } }
static void cmp_ind_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+ 3; { CMP_M(IND,RG);  } }
static void cmp_ind_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+ 9; { CMP_M(IND,RGD); } }
static void cmp_ind_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+ 9; { CMP_M(IND,IN);  } }
static void cmp_ind_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+15; { CMP_M(IND,IND); } }
static void cmp_ind_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+12; { CMP_M(IND,DE);  } }
static void cmp_ind_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+18; { CMP_M(IND,DED); } }
static void cmp_ind_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+18; { CMP_M(IND,IX);  } }
static void cmp_ind_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+24; { CMP_M(IND,IXD); } }
static void cmp_de_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+ 3; { CMP_M(DE,RG);   } }
static void cmp_de_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+ 9; { CMP_M(DE,RGD);  } }
static void cmp_de_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+ 9; { CMP_M(DE,IN);   } }
static void cmp_de_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+15; { CMP_M(DE,IND);  } }
static void cmp_de_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+12; { CMP_M(DE,DE);   } }
static void cmp_de_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+18; { CMP_M(DE,DED);  } }
static void cmp_de_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+18; { CMP_M(DE,IX);   } }
static void cmp_de_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+24; { CMP_M(DE,IXD);  } }
static void cmp_ded_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+ 3; { CMP_M(DED,RG);  } }
static void cmp_ded_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+ 9; { CMP_M(DED,RGD); } }
static void cmp_ded_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+ 9; { CMP_M(DED,IN);  } }
static void cmp_ded_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+15; { CMP_M(DED,IND); } }
static void cmp_ded_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+12; { CMP_M(DED,DE);  } }
static void cmp_ded_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+18; { CMP_M(DED,DED); } }
static void cmp_ded_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+18; { CMP_M(DED,IX);  } }
static void cmp_ded_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+24; { CMP_M(DED,IXD); } }
static void cmp_ix_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+ 3; { CMP_M(IX,RG);   } }
static void cmp_ix_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+ 9; { CMP_M(IX,RGD);  } }
static void cmp_ix_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+ 9; { CMP_M(IX,IN);   } }
static void cmp_ix_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+15; { CMP_M(IX,IND);  } }
static void cmp_ix_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+12; { CMP_M(IX,DE);   } }
static void cmp_ix_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+18; { CMP_M(IX,DED);  } }
static void cmp_ix_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+18; { CMP_M(IX,IX);   } }
static void cmp_ix_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+24; { CMP_M(IX,IXD);  } }
static void cmp_ixd_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+ 3; { CMP_M(IXD,RG);  } }
static void cmp_ixd_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+ 9; { CMP_M(IXD,RGD); } }
static void cmp_ixd_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+ 9; { CMP_M(IXD,IN);  } }
static void cmp_ixd_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+15; { CMP_M(IXD,IND); } }
static void cmp_ixd_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+12; { CMP_M(IXD,DE);  } }
static void cmp_ixd_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+18; { CMP_M(IXD,DED); } }
static void cmp_ixd_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+18; { CMP_M(IXD,IX);  } }
static void cmp_ixd_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+24; { CMP_M(IXD,IXD); } }

static void bit_rg_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+ 3; { BIT_R(RG,RG);   } }
static void bit_rg_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+ 9; { BIT_M(RG,RGD);  } }
static void bit_rg_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+ 9; { BIT_M(RG,IN);   } }
static void bit_rg_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+15; { BIT_M(RG,IND);  } }
static void bit_rg_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+12; { BIT_M(RG,DE);   } }
static void bit_rg_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+18; { BIT_M(RG,DED);  } }
static void bit_rg_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+18; { BIT_M(RG,IX);   } }
static void bit_rg_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+24; { BIT_M(RG,IXD);  } }
static void bit_rgd_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+ 3; { BIT_M(RGD,RG);  } }
static void bit_rgd_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+ 9; { BIT_M(RGD,RGD); } }
static void bit_rgd_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+ 9; { BIT_M(RGD,IN);  } }
static void bit_rgd_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+15; { BIT_M(RGD,IND); } }
static void bit_rgd_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+12; { BIT_M(RGD,DE);  } }
static void bit_rgd_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+18; { BIT_M(RGD,DED); } }
static void bit_rgd_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+18; { BIT_M(RGD,IX);  } }
static void bit_rgd_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+24; { BIT_M(RGD,IXD); } }
static void bit_in_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+ 3; { BIT_M(IN,RG);   } }
static void bit_in_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+ 9; { BIT_M(IN,RGD);  } }
static void bit_in_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+ 9; { BIT_M(IN,IN);   } }
static void bit_in_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+15; { BIT_M(IN,IND);  } }
static void bit_in_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+12; { BIT_M(IN,DE);   } }
static void bit_in_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+18; { BIT_M(IN,DED);  } }
static void bit_in_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+18; { BIT_M(IN,IX);   } }
static void bit_in_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+24; { BIT_M(IN,IXD);  } }
static void bit_ind_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+ 3; { BIT_M(IND,RG);  } }
static void bit_ind_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+ 9; { BIT_M(IND,RGD); } }
static void bit_ind_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+ 9; { BIT_M(IND,IN);  } }
static void bit_ind_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+15; { BIT_M(IND,IND); } }
static void bit_ind_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+12; { BIT_M(IND,DE);  } }
static void bit_ind_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+18; { BIT_M(IND,DED); } }
static void bit_ind_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+18; { BIT_M(IND,IX);  } }
static void bit_ind_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+24; { BIT_M(IND,IXD); } }
static void bit_de_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+ 3; { BIT_M(DE,RG);   } }
static void bit_de_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+ 9; { BIT_M(DE,RGD);  } }
static void bit_de_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+ 9; { BIT_M(DE,IN);   } }
static void bit_de_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+15; { BIT_M(DE,IND);  } }
static void bit_de_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+12; { BIT_M(DE,DE);   } }
static void bit_de_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+18; { BIT_M(DE,DED);  } }
static void bit_de_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+18; { BIT_M(DE,IX);   } }
static void bit_de_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+24; { BIT_M(DE,IXD);  } }
static void bit_ded_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+ 3; { BIT_M(DED,RG);  } }
static void bit_ded_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+ 9; { BIT_M(DED,RGD); } }
static void bit_ded_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+ 9; { BIT_M(DED,IN);  } }
static void bit_ded_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+15; { BIT_M(DED,IND); } }
static void bit_ded_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+12; { BIT_M(DED,DE);  } }
static void bit_ded_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+18; { BIT_M(DED,DED); } }
static void bit_ded_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+18; { BIT_M(DED,IX);  } }
static void bit_ded_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+24; { BIT_M(DED,IXD); } }
static void bit_ix_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+ 3; { BIT_M(IX,RG);   } }
static void bit_ix_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+ 9; { BIT_M(IX,RGD);  } }
static void bit_ix_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+ 9; { BIT_M(IX,IN);   } }
static void bit_ix_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+15; { BIT_M(IX,IND);  } }
static void bit_ix_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+12; { BIT_M(IX,DE);   } }
static void bit_ix_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+18; { BIT_M(IX,DED);  } }
static void bit_ix_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+18; { BIT_M(IX,IX);   } }
static void bit_ix_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+24; { BIT_M(IX,IXD);  } }
static void bit_ixd_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+ 3; { BIT_M(IXD,RG);  } }
static void bit_ixd_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+ 9; { BIT_M(IXD,RGD); } }
static void bit_ixd_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+ 9; { BIT_M(IXD,IN);  } }
static void bit_ixd_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+15; { BIT_M(IXD,IND); } }
static void bit_ixd_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+12; { BIT_M(IXD,DE);  } }
static void bit_ixd_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+18; { BIT_M(IXD,DED); } }
static void bit_ixd_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+18; { BIT_M(IXD,IX);  } }
static void bit_ixd_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+24; { BIT_M(IXD,IXD); } }

static void bic_rg_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+ 3; { BIC_R(RG,RG);   } }
static void bic_rg_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+12; { BIC_M(RG,RGD);  } }
static void bic_rg_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+12; { BIC_M(RG,IN);   } }
static void bic_rg_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+18; { BIC_M(RG,IND);  } }
static void bic_rg_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+15; { BIC_M(RG,DE);   } }
static void bic_rg_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+21; { BIC_M(RG,DED);  } }
static void bic_rg_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+21; { BIC_M(RG,IX);   } }
static void bic_rg_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+27; { BIC_M(RG,IXD);  } }
static void bic_rgd_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+ 3; { BIC_X(RGD,RG);  } }
static void bic_rgd_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+12; { BIC_M(RGD,RGD); } }
static void bic_rgd_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+12; { BIC_M(RGD,IN);  } }
static void bic_rgd_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+18; { BIC_M(RGD,IND); } }
static void bic_rgd_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+15; { BIC_M(RGD,DE);  } }
static void bic_rgd_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+21; { BIC_M(RGD,DED); } }
static void bic_rgd_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+21; { BIC_M(RGD,IX);  } }
static void bic_rgd_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+27; { BIC_M(RGD,IXD); } }
static void bic_in_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+ 3; { BIC_X(IN,RG);   } }
static void bic_in_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+12; { BIC_M(IN,RGD);  } }
static void bic_in_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+12; { BIC_M(IN,IN);   } }
static void bic_in_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+18; { BIC_M(IN,IND);  } }
static void bic_in_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+15; { BIC_M(IN,DE);   } }
static void bic_in_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+21; { BIC_M(IN,DED);  } }
static void bic_in_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+21; { BIC_M(IN,IX);   } }
static void bic_in_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+27; { BIC_M(IN,IXD);  } }
static void bic_ind_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+ 3; { BIC_X(IND,RG);  } }
static void bic_ind_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+12; { BIC_M(IND,RGD); } }
static void bic_ind_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+12; { BIC_M(IND,IN);  } }
static void bic_ind_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+18; { BIC_M(IND,IND); } }
static void bic_ind_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+15; { BIC_M(IND,DE);  } }
static void bic_ind_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+21; { BIC_M(IND,DED); } }
static void bic_ind_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+21; { BIC_M(IND,IX);  } }
static void bic_ind_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+27; { BIC_M(IND,IXD); } }
static void bic_de_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+ 3; { BIC_X(DE,RG);   } }
static void bic_de_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+12; { BIC_M(DE,RGD);  } }
static void bic_de_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+12; { BIC_M(DE,IN);   } }
static void bic_de_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+18; { BIC_M(DE,IND);  } }
static void bic_de_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+15; { BIC_M(DE,DE);   } }
static void bic_de_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+21; { BIC_M(DE,DED);  } }
static void bic_de_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+21; { BIC_M(DE,IX);   } }
static void bic_de_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+27; { BIC_M(DE,IXD);  } }
static void bic_ded_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+ 3; { BIC_X(DED,RG);  } }
static void bic_ded_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+12; { BIC_M(DED,RGD); } }
static void bic_ded_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+12; { BIC_M(DED,IN);  } }
static void bic_ded_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+18; { BIC_M(DED,IND); } }
static void bic_ded_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+15; { BIC_M(DED,DE);  } }
static void bic_ded_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+21; { BIC_M(DED,DED); } }
static void bic_ded_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+21; { BIC_M(DED,IX);  } }
static void bic_ded_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+27; { BIC_M(DED,IXD); } }
static void bic_ix_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+ 3; { BIC_X(IX,RG);   } }
static void bic_ix_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+12; { BIC_M(IX,RGD);  } }
static void bic_ix_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+12; { BIC_M(IX,IN);   } }
static void bic_ix_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+18; { BIC_M(IX,IND);  } }
static void bic_ix_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+15; { BIC_M(IX,DE);   } }
static void bic_ix_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+21; { BIC_M(IX,DED);  } }
static void bic_ix_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+21; { BIC_M(IX,IX);   } }
static void bic_ix_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+27; { BIC_M(IX,IXD);  } }
static void bic_ixd_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+ 3; { BIC_X(IXD,RG);  } }
static void bic_ixd_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+12; { BIC_M(IXD,RGD); } }
static void bic_ixd_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+12; { BIC_M(IXD,IN);  } }
static void bic_ixd_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+18; { BIC_M(IXD,IND); } }
static void bic_ixd_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+15; { BIC_M(IXD,DE);  } }
static void bic_ixd_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+21; { BIC_M(IXD,DED); } }
static void bic_ixd_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+21; { BIC_M(IXD,IX);  } }
static void bic_ixd_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+27; { BIC_M(IXD,IXD); } }

static void bis_rg_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+ 3; { BIS_R(RG,RG);   } }
static void bis_rg_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+12; { BIS_M(RG,RGD);  } }
static void bis_rg_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+12; { BIS_M(RG,IN);   } }
static void bis_rg_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+18; { BIS_M(RG,IND);  } }
static void bis_rg_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+15; { BIS_M(RG,DE);   } }
static void bis_rg_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+21; { BIS_M(RG,DED);  } }
static void bis_rg_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+21; { BIS_M(RG,IX);   } }
static void bis_rg_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+27; { BIS_M(RG,IXD);  } }
static void bis_rgd_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+ 3; { BIS_X(RGD,RG);  } }
static void bis_rgd_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+12; { BIS_M(RGD,RGD); } }
static void bis_rgd_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+12; { BIS_M(RGD,IN);  } }
static void bis_rgd_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+18; { BIS_M(RGD,IND); } }
static void bis_rgd_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+15; { BIS_M(RGD,DE);  } }
static void bis_rgd_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+21; { BIS_M(RGD,DED); } }
static void bis_rgd_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+21; { BIS_M(RGD,IX);  } }
static void bis_rgd_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+27; { BIS_M(RGD,IXD); } }
static void bis_in_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+ 3; { BIS_X(IN,RG);   } }
static void bis_in_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+12; { BIS_M(IN,RGD);  } }
static void bis_in_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+12; { BIS_M(IN,IN);   } }
static void bis_in_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+18; { BIS_M(IN,IND);  } }
static void bis_in_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+15; { BIS_M(IN,DE);   } }
static void bis_in_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+21; { BIS_M(IN,DED);  } }
static void bis_in_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+21; { BIS_M(IN,IX);   } }
static void bis_in_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+27; { BIS_M(IN,IXD);  } }
static void bis_ind_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+ 3; { BIS_X(IND,RG);  } }
static void bis_ind_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+12; { BIS_M(IND,RGD); } }
static void bis_ind_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+12; { BIS_M(IND,IN);  } }
static void bis_ind_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+18; { BIS_M(IND,IND); } }
static void bis_ind_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+15; { BIS_M(IND,DE);  } }
static void bis_ind_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+21; { BIS_M(IND,DED); } }
static void bis_ind_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+21; { BIS_M(IND,IX);  } }
static void bis_ind_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+27; { BIS_M(IND,IXD); } }
static void bis_de_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+ 3; { BIS_X(DE,RG);   } }
static void bis_de_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+12; { BIS_M(DE,RGD);  } }
static void bis_de_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+12; { BIS_M(DE,IN);   } }
static void bis_de_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+18; { BIS_M(DE,IND);  } }
static void bis_de_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+15; { BIS_M(DE,DE);   } }
static void bis_de_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+21; { BIS_M(DE,DED);  } }
static void bis_de_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+21; { BIS_M(DE,IX);   } }
static void bis_de_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+27; { BIS_M(DE,IXD);  } }
static void bis_ded_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+ 3; { BIS_X(DED,RG);  } }
static void bis_ded_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+12; { BIS_M(DED,RGD); } }
static void bis_ded_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+12; { BIS_M(DED,IN);  } }
static void bis_ded_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+18; { BIS_M(DED,IND); } }
static void bis_ded_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+15; { BIS_M(DED,DE);  } }
static void bis_ded_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+21; { BIS_M(DED,DED); } }
static void bis_ded_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+21; { BIS_M(DED,IX);  } }
static void bis_ded_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+27; { BIS_M(DED,IXD); } }
static void bis_ix_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+ 3; { BIS_X(IX,RG);   } }
static void bis_ix_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+12; { BIS_M(IX,RGD);  } }
static void bis_ix_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+12; { BIS_M(IX,IN);   } }
static void bis_ix_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+18; { BIS_M(IX,IND);  } }
static void bis_ix_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+15; { BIS_M(IX,DE);   } }
static void bis_ix_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+21; { BIS_M(IX,DED);  } }
static void bis_ix_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+21; { BIS_M(IX,IX);   } }
static void bis_ix_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+27; { BIS_M(IX,IXD);  } }
static void bis_ixd_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+ 3; { BIS_X(IXD,RG);  } }
static void bis_ixd_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+12; { BIS_M(IXD,RGD); } }
static void bis_ixd_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+12; { BIS_M(IXD,IN);  } }
static void bis_ixd_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+18; { BIS_M(IXD,IND); } }
static void bis_ixd_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+15; { BIS_M(IXD,DE);  } }
static void bis_ixd_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+21; { BIS_M(IXD,DED); } }
static void bis_ixd_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+21; { BIS_M(IXD,IX);  } }
static void bis_ixd_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+27; { BIS_M(IXD,IXD); } }

static void add_rg_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+ 3; { ADD_R(RG,RG);   } }
static void add_rg_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+12; { ADD_M(RG,RGD);  } }
static void add_rg_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+12; { ADD_M(RG,IN);   } }
static void add_rg_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+18; { ADD_M(RG,IND);  } }
static void add_rg_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+15; { ADD_M(RG,DE);   } }
static void add_rg_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+21; { ADD_M(RG,DED);  } }
static void add_rg_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+21; { ADD_M(RG,IX);   } }
static void add_rg_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+27; { ADD_M(RG,IXD);  } }
static void add_rgd_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+ 3; { ADD_X(RGD,RG);  } }
static void add_rgd_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+12; { ADD_M(RGD,RGD); } }
static void add_rgd_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+12; { ADD_M(RGD,IN);  } }
static void add_rgd_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+18; { ADD_M(RGD,IND); } }
static void add_rgd_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+15; { ADD_M(RGD,DE);  } }
static void add_rgd_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+21; { ADD_M(RGD,DED); } }
static void add_rgd_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+21; { ADD_M(RGD,IX);  } }
static void add_rgd_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+27; { ADD_M(RGD,IXD); } }
static void add_in_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+ 3; { ADD_X(IN,RG);   } }
static void add_in_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+12; { ADD_M(IN,RGD);  } }
static void add_in_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+12; { ADD_M(IN,IN);   } }
static void add_in_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+18; { ADD_M(IN,IND);  } }
static void add_in_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+15; { ADD_M(IN,DE);   } }
static void add_in_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+21; { ADD_M(IN,DED);  } }
static void add_in_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+21; { ADD_M(IN,IX);   } }
static void add_in_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+27; { ADD_M(IN,IXD);  } }
static void add_ind_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+ 3; { ADD_X(IND,RG);  } }
static void add_ind_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+12; { ADD_M(IND,RGD); } }
static void add_ind_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+12; { ADD_M(IND,IN);  } }
static void add_ind_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+18; { ADD_M(IND,IND); } }
static void add_ind_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+15; { ADD_M(IND,DE);  } }
static void add_ind_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+21; { ADD_M(IND,DED); } }
static void add_ind_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+21; { ADD_M(IND,IX);  } }
static void add_ind_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+27; { ADD_M(IND,IXD); } }
static void add_de_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+ 3; { ADD_X(DE,RG);   } }
static void add_de_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+12; { ADD_M(DE,RGD);  } }
static void add_de_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+12; { ADD_M(DE,IN);   } }
static void add_de_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+18; { ADD_M(DE,IND);  } }
static void add_de_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+15; { ADD_M(DE,DE);   } }
static void add_de_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+21; { ADD_M(DE,DED);  } }
static void add_de_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+21; { ADD_M(DE,IX);   } }
static void add_de_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+27; { ADD_M(DE,IXD);  } }
static void add_ded_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+ 3; { ADD_X(DED,RG);  } }
static void add_ded_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+12; { ADD_M(DED,RGD); } }
static void add_ded_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+12; { ADD_M(DED,IN);  } }
static void add_ded_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+18; { ADD_M(DED,IND); } }
static void add_ded_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+15; { ADD_M(DED,DE);  } }
static void add_ded_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+21; { ADD_M(DED,DED); } }
static void add_ded_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+21; { ADD_M(DED,IX);  } }
static void add_ded_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+27; { ADD_M(DED,IXD); } }
static void add_ix_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+ 3; { ADD_X(IX,RG);   } }
static void add_ix_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+12; { ADD_M(IX,RGD);  } }
static void add_ix_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+12; { ADD_M(IX,IN);   } }
static void add_ix_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+18; { ADD_M(IX,IND);  } }
static void add_ix_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+15; { ADD_M(IX,DE);   } }
static void add_ix_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+21; { ADD_M(IX,DED);  } }
static void add_ix_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+21; { ADD_M(IX,IX);   } }
static void add_ix_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+27; { ADD_M(IX,IXD);  } }
static void add_ixd_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+ 3; { ADD_X(IXD,RG);  } }
static void add_ixd_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+12; { ADD_M(IXD,RGD); } }
static void add_ixd_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+12; { ADD_M(IXD,IN);  } }
static void add_ixd_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+18; { ADD_M(IXD,IND); } }
static void add_ixd_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+15; { ADD_M(IXD,DE);  } }
static void add_ixd_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+21; { ADD_M(IXD,DED); } }
static void add_ixd_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+21; { ADD_M(IXD,IX);  } }
static void add_ixd_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+27; { ADD_M(IXD,IXD); } }

static void xor_rg(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 12; { XOR_R(RG);  } }
static void xor_rgd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 21; { XOR_M(RGD); } }
static void xor_in(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 21; { XOR_M(IN);  } }
static void xor_ind(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 27; { XOR_M(IND); } }
static void xor_de(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 24; { XOR_M(DE);  } }
static void xor_ded(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 30; { XOR_M(DED); } }
static void xor_ix(t11_state *cpustate, UINT16 op)        { cpustate->icount -= 30; { XOR_M(IX);  } }
static void xor_ixd(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 36; { XOR_M(IXD); } }

static void sob(t11_state *cpustate, UINT16 op)
{
	int sreg, source;

	cpustate->icount -= 18;
	GET_SREG; source = cpustate->REGD(sreg);
	source -= 1;
	cpustate->REGW(sreg) = source;
	if (source)
		cpustate->PC -= 2 * (op & 0x3f);
}

static void bpl(t11_state *cpustate, UINT16 op)           { cpustate->icount -= 12; { BR(!GET_N); } }
static void bmi(t11_state *cpustate, UINT16 op)           { cpustate->icount -= 12; { BR( GET_N); } }
static void bhi(t11_state *cpustate, UINT16 op)           { cpustate->icount -= 12; { BR(!GET_C && !GET_Z); } }
static void blos(t11_state *cpustate, UINT16 op)          { cpustate->icount -= 12; { BR( GET_C ||  GET_Z); } }
static void bvc(t11_state *cpustate, UINT16 op)           { cpustate->icount -= 12; { BR(!GET_V); } }
static void bvs(t11_state *cpustate, UINT16 op)           { cpustate->icount -= 12; { BR( GET_V); } }
static void bcc(t11_state *cpustate, UINT16 op)           { cpustate->icount -= 12; { BR(!GET_C); } }
static void bcs(t11_state *cpustate, UINT16 op)           { cpustate->icount -= 12; { BR( GET_C); } }

static void emt(t11_state *cpustate, UINT16 op)
{
	cpustate->icount -= 48;
	PUSH(cpustate, cpustate->PSW);
	PUSH(cpustate, cpustate->PC);
	cpustate->PC = RWORD(cpustate, 0x18);
	cpustate->PSW = RWORD(cpustate, 0x1a);
	t11_check_irqs(cpustate);
}

static void trap(t11_state *cpustate, UINT16 op)
{
	cpustate->icount -= 48;
	PUSH(cpustate, cpustate->PSW);
	PUSH(cpustate, cpustate->PC);
	cpustate->PC = RWORD(cpustate, 0x1c);
	cpustate->PSW = RWORD(cpustate, 0x1e);
	t11_check_irqs(cpustate);
}

static void clrb_rg(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 12; { CLRB_R(RG);  } }
static void clrb_rgd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 21; { CLRB_M(RGD); } }
static void clrb_in(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 21; { CLRB_M(IN);  } }
static void clrb_ind(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 27; { CLRB_M(IND); } }
static void clrb_de(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 24; { CLRB_M(DE);  } }
static void clrb_ded(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 30; { CLRB_M(DED); } }
static void clrb_ix(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 30; { CLRB_M(IX);  } }
static void clrb_ixd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 36; { CLRB_M(IXD); } }

static void comb_rg(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 12; { COMB_R(RG);  } }
static void comb_rgd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 21; { COMB_M(RGD); } }
static void comb_in(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 21; { COMB_M(IN);  } }
static void comb_ind(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 27; { COMB_M(IND); } }
static void comb_de(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 24; { COMB_M(DE);  } }
static void comb_ded(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 30; { COMB_M(DED); } }
static void comb_ix(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 30; { COMB_M(IX);  } }
static void comb_ixd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 36; { COMB_M(IXD); } }

static void incb_rg(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 12; { INCB_R(RG);  } }
static void incb_rgd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 21; { INCB_M(RGD); } }
static void incb_in(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 21; { INCB_M(IN);  } }
static void incb_ind(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 27; { INCB_M(IND); } }
static void incb_de(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 24; { INCB_M(DE);  } }
static void incb_ded(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 30; { INCB_M(DED); } }
static void incb_ix(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 30; { INCB_M(IX);  } }
static void incb_ixd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 36; { INCB_M(IXD); } }

static void decb_rg(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 12; { DECB_R(RG);  } }
static void decb_rgd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 21; { DECB_M(RGD); } }
static void decb_in(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 21; { DECB_M(IN);  } }
static void decb_ind(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 27; { DECB_M(IND); } }
static void decb_de(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 24; { DECB_M(DE);  } }
static void decb_ded(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 30; { DECB_M(DED); } }
static void decb_ix(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 30; { DECB_M(IX);  } }
static void decb_ixd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 36; { DECB_M(IXD); } }

static void negb_rg(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 12; { NEGB_R(RG);  } }
static void negb_rgd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 21; { NEGB_M(RGD); } }
static void negb_in(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 21; { NEGB_M(IN);  } }
static void negb_ind(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 27; { NEGB_M(IND); } }
static void negb_de(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 24; { NEGB_M(DE);  } }
static void negb_ded(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 30; { NEGB_M(DED); } }
static void negb_ix(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 30; { NEGB_M(IX);  } }
static void negb_ixd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 36; { NEGB_M(IXD); } }

static void adcb_rg(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 12; { ADCB_R(RG);  } }
static void adcb_rgd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 21; { ADCB_M(RGD); } }
static void adcb_in(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 21; { ADCB_M(IN);  } }
static void adcb_ind(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 27; { ADCB_M(IND); } }
static void adcb_de(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 24; { ADCB_M(DE);  } }
static void adcb_ded(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 30; { ADCB_M(DED); } }
static void adcb_ix(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 30; { ADCB_M(IX);  } }
static void adcb_ixd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 36; { ADCB_M(IXD); } }

static void sbcb_rg(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 12; { SBCB_R(RG);  } }
static void sbcb_rgd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 21; { SBCB_M(RGD); } }
static void sbcb_in(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 21; { SBCB_M(IN);  } }
static void sbcb_ind(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 27; { SBCB_M(IND); } }
static void sbcb_de(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 24; { SBCB_M(DE);  } }
static void sbcb_ded(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 30; { SBCB_M(DED); } }
static void sbcb_ix(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 30; { SBCB_M(IX);  } }
static void sbcb_ixd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 36; { SBCB_M(IXD); } }

static void tstb_rg(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 12; { TSTB_R(RG);  } }
static void tstb_rgd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 18; { TSTB_M(RGD); } }
static void tstb_in(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 18; { TSTB_M(IN);  } }
static void tstb_ind(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 24; { TSTB_M(IND); } }
static void tstb_de(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 21; { TSTB_M(DE);  } }
static void tstb_ded(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 27; { TSTB_M(DED); } }
static void tstb_ix(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 27; { TSTB_M(IX);  } }
static void tstb_ixd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 33; { TSTB_M(IXD); } }

static void rorb_rg(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 12; { RORB_R(RG);  } }
static void rorb_rgd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 21; { RORB_M(RGD); } }
static void rorb_in(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 21; { RORB_M(IN);  } }
static void rorb_ind(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 27; { RORB_M(IND); } }
static void rorb_de(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 24; { RORB_M(DE);  } }
static void rorb_ded(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 30; { RORB_M(DED); } }
static void rorb_ix(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 30; { RORB_M(IX);  } }
static void rorb_ixd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 36; { RORB_M(IXD); } }

static void rolb_rg(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 12; { ROLB_R(RG);  } }
static void rolb_rgd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 21; { ROLB_M(RGD); } }
static void rolb_in(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 21; { ROLB_M(IN);  } }
static void rolb_ind(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 27; { ROLB_M(IND); } }
static void rolb_de(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 24; { ROLB_M(DE);  } }
static void rolb_ded(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 30; { ROLB_M(DED); } }
static void rolb_ix(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 30; { ROLB_M(IX);  } }
static void rolb_ixd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 36; { ROLB_M(IXD); } }

static void asrb_rg(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 12; { ASRB_R(RG);  } }
static void asrb_rgd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 21; { ASRB_M(RGD); } }
static void asrb_in(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 21; { ASRB_M(IN);  } }
static void asrb_ind(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 27; { ASRB_M(IND); } }
static void asrb_de(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 24; { ASRB_M(DE);  } }
static void asrb_ded(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 30; { ASRB_M(DED); } }
static void asrb_ix(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 30; { ASRB_M(IX);  } }
static void asrb_ixd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 36; { ASRB_M(IXD); } }

static void aslb_rg(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 12; { ASLB_R(RG);  } }
static void aslb_rgd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 21; { ASLB_M(RGD); } }
static void aslb_in(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 21; { ASLB_M(IN);  } }
static void aslb_ind(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 27; { ASLB_M(IND); } }
static void aslb_de(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 24; { ASLB_M(DE);  } }
static void aslb_ded(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 30; { ASLB_M(DED); } }
static void aslb_ix(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 30; { ASLB_M(IX);  } }
static void aslb_ixd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 36; { ASLB_M(IXD); } }

static void mtps_rg(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 24; { MTPS_R(RG);  } }
static void mtps_rgd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 30; { MTPS_M(RGD); } }
static void mtps_in(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 30; { MTPS_M(IN);  } }
static void mtps_ind(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 36; { MTPS_M(IND); } }
static void mtps_de(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 33; { MTPS_M(DE);  } }
static void mtps_ded(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 39; { MTPS_M(DED); } }
static void mtps_ix(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 39; { MTPS_M(IX);  } }
static void mtps_ixd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 45; { MTPS_M(IXD); } }

static void mfps_rg(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 12; { MFPS_R(RG);  } }
static void mfps_rgd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 21; { MFPS_M(RGD); } }
static void mfps_in(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 21; { MFPS_M(IN);  } }
static void mfps_ind(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 27; { MFPS_M(IND); } }
static void mfps_de(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 24; { MFPS_M(DE);  } }
static void mfps_ded(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 30; { MFPS_M(DED); } }
static void mfps_ix(t11_state *cpustate, UINT16 op)       { cpustate->icount -= 30; { MFPS_M(IX);  } }
static void mfps_ixd(t11_state *cpustate, UINT16 op)      { cpustate->icount -= 36; { MFPS_M(IXD); } }

static void movb_rg_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+ 3; { MOVB_R(RG,RG);   } }
static void movb_rg_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+12; { MOVB_M(RG,RGD);  } }
static void movb_rg_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+12; { MOVB_M(RG,IN);   } }
static void movb_rg_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+18; { MOVB_M(RG,IND);  } }
static void movb_rg_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+15; { MOVB_M(RG,DE);   } }
static void movb_rg_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+21; { MOVB_M(RG,DED);  } }
static void movb_rg_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+21; { MOVB_M(RG,IX);   } }
static void movb_rg_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+27; { MOVB_M(RG,IXD);  } }
static void movb_rgd_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+ 3; { MOVB_X(RGD,RG);  } }
static void movb_rgd_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+12; { MOVB_M(RGD,RGD); } }
static void movb_rgd_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+12; { MOVB_M(RGD,IN);  } }
static void movb_rgd_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+18; { MOVB_M(RGD,IND); } }
static void movb_rgd_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+15; { MOVB_M(RGD,DE);  } }
static void movb_rgd_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+21; { MOVB_M(RGD,DED); } }
static void movb_rgd_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+21; { MOVB_M(RGD,IX);  } }
static void movb_rgd_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+27; { MOVB_M(RGD,IXD); } }
static void movb_in_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+ 3; { MOVB_X(IN,RG);   } }
static void movb_in_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+12; { MOVB_M(IN,RGD);  } }
static void movb_in_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+12; { MOVB_M(IN,IN);   } }
static void movb_in_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+18; { MOVB_M(IN,IND);  } }
static void movb_in_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+15; { MOVB_M(IN,DE);   } }
static void movb_in_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+21; { MOVB_M(IN,DED);  } }
static void movb_in_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+21; { MOVB_M(IN,IX);   } }
static void movb_in_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+27; { MOVB_M(IN,IXD);  } }
static void movb_ind_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+ 3; { MOVB_X(IND,RG);  } }
static void movb_ind_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+12; { MOVB_M(IND,RGD); } }
static void movb_ind_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+12; { MOVB_M(IND,IN);  } }
static void movb_ind_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+18; { MOVB_M(IND,IND); } }
static void movb_ind_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+15; { MOVB_M(IND,DE);  } }
static void movb_ind_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+21; { MOVB_M(IND,DED); } }
static void movb_ind_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+21; { MOVB_M(IND,IX);  } }
static void movb_ind_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+27; { MOVB_M(IND,IXD); } }
static void movb_de_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+ 3; { MOVB_X(DE,RG);   } }
static void movb_de_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+12; { MOVB_M(DE,RGD);  } }
static void movb_de_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+12; { MOVB_M(DE,IN);   } }
static void movb_de_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+18; { MOVB_M(DE,IND);  } }
static void movb_de_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+15; { MOVB_M(DE,DE);   } }
static void movb_de_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+21; { MOVB_M(DE,DED);  } }
static void movb_de_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+21; { MOVB_M(DE,IX);   } }
static void movb_de_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+27; { MOVB_M(DE,IXD);  } }
static void movb_ded_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+ 3; { MOVB_X(DED,RG);  } }
static void movb_ded_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+12; { MOVB_M(DED,RGD); } }
static void movb_ded_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+12; { MOVB_M(DED,IN);  } }
static void movb_ded_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+18; { MOVB_M(DED,IND); } }
static void movb_ded_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+15; { MOVB_M(DED,DE);  } }
static void movb_ded_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+21; { MOVB_M(DED,DED); } }
static void movb_ded_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+21; { MOVB_M(DED,IX);  } }
static void movb_ded_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+27; { MOVB_M(DED,IXD); } }
static void movb_ix_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+ 3; { MOVB_X(IX,RG);   } }
static void movb_ix_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+12; { MOVB_M(IX,RGD);  } }
static void movb_ix_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+12; { MOVB_M(IX,IN);   } }
static void movb_ix_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+18; { MOVB_M(IX,IND);  } }
static void movb_ix_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+15; { MOVB_M(IX,DE);   } }
static void movb_ix_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+21; { MOVB_M(IX,DED);  } }
static void movb_ix_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+21; { MOVB_M(IX,IX);   } }
static void movb_ix_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+27; { MOVB_M(IX,IXD);  } }
static void movb_ixd_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+ 3; { MOVB_X(IXD,RG);  } }
static void movb_ixd_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+12; { MOVB_M(IXD,RGD); } }
static void movb_ixd_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+12; { MOVB_M(IXD,IN);  } }
static void movb_ixd_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+18; { MOVB_M(IXD,IND); } }
static void movb_ixd_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+15; { MOVB_M(IXD,DE);  } }
static void movb_ixd_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+21; { MOVB_M(IXD,DED); } }
static void movb_ixd_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+21; { MOVB_M(IXD,IX);  } }
static void movb_ixd_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+27; { MOVB_M(IXD,IXD); } }

static void cmpb_rg_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+ 3; { CMPB_R(RG,RG);   } }
static void cmpb_rg_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+ 9; { CMPB_M(RG,RGD);  } }
static void cmpb_rg_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+ 9; { CMPB_M(RG,IN);   } }
static void cmpb_rg_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+15; { CMPB_M(RG,IND);  } }
static void cmpb_rg_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+12; { CMPB_M(RG,DE);   } }
static void cmpb_rg_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+18; { CMPB_M(RG,DED);  } }
static void cmpb_rg_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+18; { CMPB_M(RG,IX);   } }
static void cmpb_rg_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+24; { CMPB_M(RG,IXD);  } }
static void cmpb_rgd_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+ 3; { CMPB_M(RGD,RG);  } }
static void cmpb_rgd_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+ 9; { CMPB_M(RGD,RGD); } }
static void cmpb_rgd_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+ 9; { CMPB_M(RGD,IN);  } }
static void cmpb_rgd_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+15; { CMPB_M(RGD,IND); } }
static void cmpb_rgd_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+12; { CMPB_M(RGD,DE);  } }
static void cmpb_rgd_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+18; { CMPB_M(RGD,DED); } }
static void cmpb_rgd_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+18; { CMPB_M(RGD,IX);  } }
static void cmpb_rgd_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+24; { CMPB_M(RGD,IXD); } }
static void cmpb_in_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+ 3; { CMPB_M(IN,RG);   } }
static void cmpb_in_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+ 9; { CMPB_M(IN,RGD);  } }
static void cmpb_in_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+ 9; { CMPB_M(IN,IN);   } }
static void cmpb_in_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+15; { CMPB_M(IN,IND);  } }
static void cmpb_in_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+12; { CMPB_M(IN,DE);   } }
static void cmpb_in_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+18; { CMPB_M(IN,DED);  } }
static void cmpb_in_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+18; { CMPB_M(IN,IX);   } }
static void cmpb_in_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+24; { CMPB_M(IN,IXD);  } }
static void cmpb_ind_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+ 3; { CMPB_M(IND,RG);  } }
static void cmpb_ind_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+ 9; { CMPB_M(IND,RGD); } }
static void cmpb_ind_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+ 9; { CMPB_M(IND,IN);  } }
static void cmpb_ind_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+15; { CMPB_M(IND,IND); } }
static void cmpb_ind_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+12; { CMPB_M(IND,DE);  } }
static void cmpb_ind_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+18; { CMPB_M(IND,DED); } }
static void cmpb_ind_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+18; { CMPB_M(IND,IX);  } }
static void cmpb_ind_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+24; { CMPB_M(IND,IXD); } }
static void cmpb_de_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+ 3; { CMPB_M(DE,RG);   } }
static void cmpb_de_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+ 9; { CMPB_M(DE,RGD);  } }
static void cmpb_de_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+ 9; { CMPB_M(DE,IN);   } }
static void cmpb_de_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+15; { CMPB_M(DE,IND);  } }
static void cmpb_de_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+12; { CMPB_M(DE,DE);   } }
static void cmpb_de_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+18; { CMPB_M(DE,DED);  } }
static void cmpb_de_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+18; { CMPB_M(DE,IX);   } }
static void cmpb_de_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+24; { CMPB_M(DE,IXD);  } }
static void cmpb_ded_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+ 3; { CMPB_M(DED,RG);  } }
static void cmpb_ded_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+ 9; { CMPB_M(DED,RGD); } }
static void cmpb_ded_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+ 9; { CMPB_M(DED,IN);  } }
static void cmpb_ded_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+15; { CMPB_M(DED,IND); } }
static void cmpb_ded_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+12; { CMPB_M(DED,DE);  } }
static void cmpb_ded_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+18; { CMPB_M(DED,DED); } }
static void cmpb_ded_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+18; { CMPB_M(DED,IX);  } }
static void cmpb_ded_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+24; { CMPB_M(DED,IXD); } }
static void cmpb_ix_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+ 3; { CMPB_M(IX,RG);   } }
static void cmpb_ix_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+ 9; { CMPB_M(IX,RGD);  } }
static void cmpb_ix_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+ 9; { CMPB_M(IX,IN);   } }
static void cmpb_ix_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+15; { CMPB_M(IX,IND);  } }
static void cmpb_ix_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+12; { CMPB_M(IX,DE);   } }
static void cmpb_ix_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+18; { CMPB_M(IX,DED);  } }
static void cmpb_ix_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+18; { CMPB_M(IX,IX);   } }
static void cmpb_ix_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+24; { CMPB_M(IX,IXD);  } }
static void cmpb_ixd_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+ 3; { CMPB_M(IXD,RG);  } }
static void cmpb_ixd_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+ 9; { CMPB_M(IXD,RGD); } }
static void cmpb_ixd_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+ 9; { CMPB_M(IXD,IN);  } }
static void cmpb_ixd_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+15; { CMPB_M(IXD,IND); } }
static void cmpb_ixd_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+12; { CMPB_M(IXD,DE);  } }
static void cmpb_ixd_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+18; { CMPB_M(IXD,DED); } }
static void cmpb_ixd_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+18; { CMPB_M(IXD,IX);  } }
static void cmpb_ixd_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+24; { CMPB_M(IXD,IXD); } }

static void bitb_rg_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+ 3; { BITB_R(RG,RG);   } }
static void bitb_rg_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+ 9; { BITB_M(RG,RGD);  } }
static void bitb_rg_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+ 9; { BITB_M(RG,IN);   } }
static void bitb_rg_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+15; { BITB_M(RG,IND);  } }
static void bitb_rg_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+12; { BITB_M(RG,DE);   } }
static void bitb_rg_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+18; { BITB_M(RG,DED);  } }
static void bitb_rg_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+18; { BITB_M(RG,IX);   } }
static void bitb_rg_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+24; { BITB_M(RG,IXD);  } }
static void bitb_rgd_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+ 3; { BITB_M(RGD,RG);  } }
static void bitb_rgd_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+ 9; { BITB_M(RGD,RGD); } }
static void bitb_rgd_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+ 9; { BITB_M(RGD,IN);  } }
static void bitb_rgd_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+15; { BITB_M(RGD,IND); } }
static void bitb_rgd_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+12; { BITB_M(RGD,DE);  } }
static void bitb_rgd_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+18; { BITB_M(RGD,DED); } }
static void bitb_rgd_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+18; { BITB_M(RGD,IX);  } }
static void bitb_rgd_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+24; { BITB_M(RGD,IXD); } }
static void bitb_in_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+ 3; { BITB_M(IN,RG);   } }
static void bitb_in_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+ 9; { BITB_M(IN,RGD);  } }
static void bitb_in_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+ 9; { BITB_M(IN,IN);   } }
static void bitb_in_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+15; { BITB_M(IN,IND);  } }
static void bitb_in_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+12; { BITB_M(IN,DE);   } }
static void bitb_in_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+18; { BITB_M(IN,DED);  } }
static void bitb_in_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+18; { BITB_M(IN,IX);   } }
static void bitb_in_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+24; { BITB_M(IN,IXD);  } }
static void bitb_ind_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+ 3; { BITB_M(IND,RG);  } }
static void bitb_ind_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+ 9; { BITB_M(IND,RGD); } }
static void bitb_ind_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+ 9; { BITB_M(IND,IN);  } }
static void bitb_ind_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+15; { BITB_M(IND,IND); } }
static void bitb_ind_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+12; { BITB_M(IND,DE);  } }
static void bitb_ind_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+18; { BITB_M(IND,DED); } }
static void bitb_ind_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+18; { BITB_M(IND,IX);  } }
static void bitb_ind_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+24; { BITB_M(IND,IXD); } }
static void bitb_de_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+ 3; { BITB_M(DE,RG);   } }
static void bitb_de_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+ 9; { BITB_M(DE,RGD);  } }
static void bitb_de_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+ 9; { BITB_M(DE,IN);   } }
static void bitb_de_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+15; { BITB_M(DE,IND);  } }
static void bitb_de_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+12; { BITB_M(DE,DE);   } }
static void bitb_de_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+18; { BITB_M(DE,DED);  } }
static void bitb_de_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+18; { BITB_M(DE,IX);   } }
static void bitb_de_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+24; { BITB_M(DE,IXD);  } }
static void bitb_ded_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+ 3; { BITB_M(DED,RG);  } }
static void bitb_ded_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+ 9; { BITB_M(DED,RGD); } }
static void bitb_ded_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+ 9; { BITB_M(DED,IN);  } }
static void bitb_ded_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+15; { BITB_M(DED,IND); } }
static void bitb_ded_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+12; { BITB_M(DED,DE);  } }
static void bitb_ded_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+18; { BITB_M(DED,DED); } }
static void bitb_ded_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+18; { BITB_M(DED,IX);  } }
static void bitb_ded_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+24; { BITB_M(DED,IXD); } }
static void bitb_ix_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+ 3; { BITB_M(IX,RG);   } }
static void bitb_ix_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+ 9; { BITB_M(IX,RGD);  } }
static void bitb_ix_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+ 9; { BITB_M(IX,IN);   } }
static void bitb_ix_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+15; { BITB_M(IX,IND);  } }
static void bitb_ix_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+12; { BITB_M(IX,DE);   } }
static void bitb_ix_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+18; { BITB_M(IX,DED);  } }
static void bitb_ix_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+18; { BITB_M(IX,IX);   } }
static void bitb_ix_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+24; { BITB_M(IX,IXD);  } }
static void bitb_ixd_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+ 3; { BITB_M(IXD,RG);  } }
static void bitb_ixd_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+ 9; { BITB_M(IXD,RGD); } }
static void bitb_ixd_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+ 9; { BITB_M(IXD,IN);  } }
static void bitb_ixd_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+15; { BITB_M(IXD,IND); } }
static void bitb_ixd_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+12; { BITB_M(IXD,DE);  } }
static void bitb_ixd_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+18; { BITB_M(IXD,DED); } }
static void bitb_ixd_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+18; { BITB_M(IXD,IX);  } }
static void bitb_ixd_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+24; { BITB_M(IXD,IXD); } }

static void bicb_rg_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+ 3; { BICB_R(RG,RG);   } }
static void bicb_rg_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+12; { BICB_M(RG,RGD);  } }
static void bicb_rg_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+12; { BICB_M(RG,IN);   } }
static void bicb_rg_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+18; { BICB_M(RG,IND);  } }
static void bicb_rg_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+15; { BICB_M(RG,DE);   } }
static void bicb_rg_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+21; { BICB_M(RG,DED);  } }
static void bicb_rg_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+21; { BICB_M(RG,IX);   } }
static void bicb_rg_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+27; { BICB_M(RG,IXD);  } }
static void bicb_rgd_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+ 3; { BICB_X(RGD,RG);  } }
static void bicb_rgd_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+12; { BICB_M(RGD,RGD); } }
static void bicb_rgd_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+12; { BICB_M(RGD,IN);  } }
static void bicb_rgd_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+18; { BICB_M(RGD,IND); } }
static void bicb_rgd_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+15; { BICB_M(RGD,DE);  } }
static void bicb_rgd_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+21; { BICB_M(RGD,DED); } }
static void bicb_rgd_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+21; { BICB_M(RGD,IX);  } }
static void bicb_rgd_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+27; { BICB_M(RGD,IXD); } }
static void bicb_in_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+ 3; { BICB_X(IN,RG);   } }
static void bicb_in_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+12; { BICB_M(IN,RGD);  } }
static void bicb_in_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+12; { BICB_M(IN,IN);   } }
static void bicb_in_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+18; { BICB_M(IN,IND);  } }
static void bicb_in_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+15; { BICB_M(IN,DE);   } }
static void bicb_in_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+21; { BICB_M(IN,DED);  } }
static void bicb_in_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+21; { BICB_M(IN,IX);   } }
static void bicb_in_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+27; { BICB_M(IN,IXD);  } }
static void bicb_ind_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+ 3; { BICB_X(IND,RG);  } }
static void bicb_ind_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+12; { BICB_M(IND,RGD); } }
static void bicb_ind_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+12; { BICB_M(IND,IN);  } }
static void bicb_ind_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+18; { BICB_M(IND,IND); } }
static void bicb_ind_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+15; { BICB_M(IND,DE);  } }
static void bicb_ind_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+21; { BICB_M(IND,DED); } }
static void bicb_ind_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+21; { BICB_M(IND,IX);  } }
static void bicb_ind_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+27; { BICB_M(IND,IXD); } }
static void bicb_de_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+ 3; { BICB_X(DE,RG);   } }
static void bicb_de_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+12; { BICB_M(DE,RGD);  } }
static void bicb_de_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+12; { BICB_M(DE,IN);   } }
static void bicb_de_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+18; { BICB_M(DE,IND);  } }
static void bicb_de_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+15; { BICB_M(DE,DE);   } }
static void bicb_de_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+21; { BICB_M(DE,DED);  } }
static void bicb_de_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+21; { BICB_M(DE,IX);   } }
static void bicb_de_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+27; { BICB_M(DE,IXD);  } }
static void bicb_ded_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+ 3; { BICB_X(DED,RG);  } }
static void bicb_ded_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+12; { BICB_M(DED,RGD); } }
static void bicb_ded_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+12; { BICB_M(DED,IN);  } }
static void bicb_ded_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+18; { BICB_M(DED,IND); } }
static void bicb_ded_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+15; { BICB_M(DED,DE);  } }
static void bicb_ded_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+21; { BICB_M(DED,DED); } }
static void bicb_ded_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+21; { BICB_M(DED,IX);  } }
static void bicb_ded_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+27; { BICB_M(DED,IXD); } }
static void bicb_ix_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+ 3; { BICB_X(IX,RG);   } }
static void bicb_ix_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+12; { BICB_M(IX,RGD);  } }
static void bicb_ix_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+12; { BICB_M(IX,IN);   } }
static void bicb_ix_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+18; { BICB_M(IX,IND);  } }
static void bicb_ix_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+15; { BICB_M(IX,DE);   } }
static void bicb_ix_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+21; { BICB_M(IX,DED);  } }
static void bicb_ix_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+21; { BICB_M(IX,IX);   } }
static void bicb_ix_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+27; { BICB_M(IX,IXD);  } }
static void bicb_ixd_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+ 3; { BICB_X(IXD,RG);  } }
static void bicb_ixd_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+12; { BICB_M(IXD,RGD); } }
static void bicb_ixd_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+12; { BICB_M(IXD,IN);  } }
static void bicb_ixd_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+18; { BICB_M(IXD,IND); } }
static void bicb_ixd_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+15; { BICB_M(IXD,DE);  } }
static void bicb_ixd_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+21; { BICB_M(IXD,DED); } }
static void bicb_ixd_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+21; { BICB_M(IXD,IX);  } }
static void bicb_ixd_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+27; { BICB_M(IXD,IXD); } }

static void bisb_rg_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+ 3; { BISB_R(RG,RG);   } }
static void bisb_rg_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+12; { BISB_M(RG,RGD);  } }
static void bisb_rg_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+12; { BISB_M(RG,IN);   } }
static void bisb_rg_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+18; { BISB_M(RG,IND);  } }
static void bisb_rg_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+15; { BISB_M(RG,DE);   } }
static void bisb_rg_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+21; { BISB_M(RG,DED);  } }
static void bisb_rg_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+21; { BISB_M(RG,IX);   } }
static void bisb_rg_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+27; { BISB_M(RG,IXD);  } }
static void bisb_rgd_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+ 3; { BISB_X(RGD,RG);  } }
static void bisb_rgd_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+12; { BISB_M(RGD,RGD); } }
static void bisb_rgd_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+12; { BISB_M(RGD,IN);  } }
static void bisb_rgd_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+18; { BISB_M(RGD,IND); } }
static void bisb_rgd_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+15; { BISB_M(RGD,DE);  } }
static void bisb_rgd_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+21; { BISB_M(RGD,DED); } }
static void bisb_rgd_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+21; { BISB_M(RGD,IX);  } }
static void bisb_rgd_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+27; { BISB_M(RGD,IXD); } }
static void bisb_in_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+ 3; { BISB_X(IN,RG);   } }
static void bisb_in_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+12; { BISB_M(IN,RGD);  } }
static void bisb_in_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+12; { BISB_M(IN,IN);   } }
static void bisb_in_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+18; { BISB_M(IN,IND);  } }
static void bisb_in_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+15; { BISB_M(IN,DE);   } }
static void bisb_in_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+21; { BISB_M(IN,DED);  } }
static void bisb_in_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+21; { BISB_M(IN,IX);   } }
static void bisb_in_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+27; { BISB_M(IN,IXD);  } }
static void bisb_ind_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+ 3; { BISB_X(IND,RG);  } }
static void bisb_ind_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+12; { BISB_M(IND,RGD); } }
static void bisb_ind_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+12; { BISB_M(IND,IN);  } }
static void bisb_ind_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+18; { BISB_M(IND,IND); } }
static void bisb_ind_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+15; { BISB_M(IND,DE);  } }
static void bisb_ind_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+21; { BISB_M(IND,DED); } }
static void bisb_ind_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+21; { BISB_M(IND,IX);  } }
static void bisb_ind_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+27; { BISB_M(IND,IXD); } }
static void bisb_de_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+ 3; { BISB_X(DE,RG);   } }
static void bisb_de_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+12; { BISB_M(DE,RGD);  } }
static void bisb_de_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+12; { BISB_M(DE,IN);   } }
static void bisb_de_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+18; { BISB_M(DE,IND);  } }
static void bisb_de_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+15; { BISB_M(DE,DE);   } }
static void bisb_de_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+21; { BISB_M(DE,DED);  } }
static void bisb_de_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+21; { BISB_M(DE,IX);   } }
static void bisb_de_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+27; { BISB_M(DE,IXD);  } }
static void bisb_ded_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+ 3; { BISB_X(DED,RG);  } }
static void bisb_ded_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+12; { BISB_M(DED,RGD); } }
static void bisb_ded_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+12; { BISB_M(DED,IN);  } }
static void bisb_ded_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+18; { BISB_M(DED,IND); } }
static void bisb_ded_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+15; { BISB_M(DED,DE);  } }
static void bisb_ded_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+21; { BISB_M(DED,DED); } }
static void bisb_ded_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+21; { BISB_M(DED,IX);  } }
static void bisb_ded_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+27; { BISB_M(DED,IXD); } }
static void bisb_ix_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+ 3; { BISB_X(IX,RG);   } }
static void bisb_ix_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+12; { BISB_M(IX,RGD);  } }
static void bisb_ix_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+12; { BISB_M(IX,IN);   } }
static void bisb_ix_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+18; { BISB_M(IX,IND);  } }
static void bisb_ix_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+15; { BISB_M(IX,DE);   } }
static void bisb_ix_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+21; { BISB_M(IX,DED);  } }
static void bisb_ix_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+21; { BISB_M(IX,IX);   } }
static void bisb_ix_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+27; { BISB_M(IX,IXD);  } }
static void bisb_ixd_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+ 3; { BISB_X(IXD,RG);  } }
static void bisb_ixd_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+12; { BISB_M(IXD,RGD); } }
static void bisb_ixd_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+12; { BISB_M(IXD,IN);  } }
static void bisb_ixd_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+18; { BISB_M(IXD,IND); } }
static void bisb_ixd_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+15; { BISB_M(IXD,DE);  } }
static void bisb_ixd_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+21; { BISB_M(IXD,DED); } }
static void bisb_ixd_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+21; { BISB_M(IXD,IX);  } }
static void bisb_ixd_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+27; { BISB_M(IXD,IXD); } }

static void sub_rg_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+ 3; { SUB_R(RG,RG);   } }
static void sub_rg_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+12; { SUB_M(RG,RGD);  } }
static void sub_rg_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+12; { SUB_M(RG,IN);   } }
static void sub_rg_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+18; { SUB_M(RG,IND);  } }
static void sub_rg_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+15; { SUB_M(RG,DE);   } }
static void sub_rg_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+21; { SUB_M(RG,DED);  } }
static void sub_rg_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -=  9+21; { SUB_M(RG,IX);   } }
static void sub_rg_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -=  9+27; { SUB_M(RG,IXD);  } }
static void sub_rgd_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+ 3; { SUB_X(RGD,RG);  } }
static void sub_rgd_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+12; { SUB_M(RGD,RGD); } }
static void sub_rgd_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+12; { SUB_M(RGD,IN);  } }
static void sub_rgd_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+18; { SUB_M(RGD,IND); } }
static void sub_rgd_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+15; { SUB_M(RGD,DE);  } }
static void sub_rgd_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+21; { SUB_M(RGD,DED); } }
static void sub_rgd_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+21; { SUB_M(RGD,IX);  } }
static void sub_rgd_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 15+27; { SUB_M(RGD,IXD); } }
static void sub_in_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+ 3; { SUB_X(IN,RG);   } }
static void sub_in_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+12; { SUB_M(IN,RGD);  } }
static void sub_in_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+12; { SUB_M(IN,IN);   } }
static void sub_in_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+18; { SUB_M(IN,IND);  } }
static void sub_in_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+15; { SUB_M(IN,DE);   } }
static void sub_in_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+21; { SUB_M(IN,DED);  } }
static void sub_in_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 15+21; { SUB_M(IN,IX);   } }
static void sub_in_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 15+27; { SUB_M(IN,IXD);  } }
static void sub_ind_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+ 3; { SUB_X(IND,RG);  } }
static void sub_ind_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+12; { SUB_M(IND,RGD); } }
static void sub_ind_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+12; { SUB_M(IND,IN);  } }
static void sub_ind_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+18; { SUB_M(IND,IND); } }
static void sub_ind_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+15; { SUB_M(IND,DE);  } }
static void sub_ind_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+21; { SUB_M(IND,DED); } }
static void sub_ind_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 21+21; { SUB_M(IND,IX);  } }
static void sub_ind_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 21+27; { SUB_M(IND,IXD); } }
static void sub_de_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+ 3; { SUB_X(DE,RG);   } }
static void sub_de_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+12; { SUB_M(DE,RGD);  } }
static void sub_de_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+12; { SUB_M(DE,IN);   } }
static void sub_de_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+18; { SUB_M(DE,IND);  } }
static void sub_de_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+15; { SUB_M(DE,DE);   } }
static void sub_de_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+21; { SUB_M(DE,DED);  } }
static void sub_de_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 18+21; { SUB_M(DE,IX);   } }
static void sub_de_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 18+27; { SUB_M(DE,IXD);  } }
static void sub_ded_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+ 3; { SUB_X(DED,RG);  } }
static void sub_ded_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+12; { SUB_M(DED,RGD); } }
static void sub_ded_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+12; { SUB_M(DED,IN);  } }
static void sub_ded_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+18; { SUB_M(DED,IND); } }
static void sub_ded_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+15; { SUB_M(DED,DE);  } }
static void sub_ded_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+21; { SUB_M(DED,DED); } }
static void sub_ded_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+21; { SUB_M(DED,IX);  } }
static void sub_ded_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 24+27; { SUB_M(DED,IXD); } }
static void sub_ix_rg(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+ 3; { SUB_X(IX,RG);   } }
static void sub_ix_rgd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+12; { SUB_M(IX,RGD);  } }
static void sub_ix_in(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+12; { SUB_M(IX,IN);   } }
static void sub_ix_ind(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+18; { SUB_M(IX,IND);  } }
static void sub_ix_de(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+15; { SUB_M(IX,DE);   } }
static void sub_ix_ded(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+21; { SUB_M(IX,DED);  } }
static void sub_ix_ix(t11_state *cpustate, UINT16 op)     { cpustate->icount -= 24+21; { SUB_M(IX,IX);   } }
static void sub_ix_ixd(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 24+27; { SUB_M(IX,IXD);  } }
static void sub_ixd_rg(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+ 3; { SUB_X(IXD,RG);  } }
static void sub_ixd_rgd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+12; { SUB_M(IXD,RGD); } }
static void sub_ixd_in(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+12; { SUB_M(IXD,IN);  } }
static void sub_ixd_ind(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+18; { SUB_M(IXD,IND); } }
static void sub_ixd_de(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+15; { SUB_M(IXD,DE);  } }
static void sub_ixd_ded(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+21; { SUB_M(IXD,DED); } }
static void sub_ixd_ix(t11_state *cpustate, UINT16 op)    { cpustate->icount -= 30+21; { SUB_M(IXD,IX);  } }
static void sub_ixd_ixd(t11_state *cpustate, UINT16 op)   { cpustate->icount -= 30+27; { SUB_M(IXD,IXD); } }
