#define SET_Z8(r)			(cpustate->ccr |= ((UINT8)r == 0) ? CC_Z : 0)
#define SET_Z16(r)			(cpustate->ccr |= ((UINT16)r == 0) ? CC_Z : 0)
#define SET_N8(r)			(cpustate->ccr |= (r & 0x80) ? CC_N : 0)
#define SET_N16(r)			(cpustate->ccr |= (r & 0x80) ? CC_N : 0)
#define SET_V_ADD8(r,s,d)	(cpustate->ccr |= (((r) ^ (s)) & ((r) ^ (d)) & 0x80) ? CC_V : 0)
#define SET_V_SUB8(r,s,d)	(cpustate->ccr |= (((d) ^ (s)) & ((d) ^ (r)) & 0x80) ? CC_V : 0)
#define SET_V_ADD16(r,s,d)	(cpustate->ccr |= (((r) ^ (s)) & ((r) ^ (d)) & 0x8000) ? CC_V : 0)
#define SET_V_SUB16(r,s,d)	(cpustate->ccr |= (((d) ^ (s)) & ((d) ^ (r)) & 0x8000) ? CC_V : 0)
#define SET_H(r,s,d)		(cpustate->ccr |= ((((s) & (d)) | ((d) & (r)) | ((r) & (s))) & 0x10) ? CC_H : 0)
#define SET_C8(x)			(cpustate->ccr |= ((x) & 0x100) ? CC_C : 0)
#define SET_C16(x)			(cpustate->ccr |= ((x) & 0x10000) ? CC_C : 0)
#define CLEAR_Z(cpustate)			(cpustate->ccr &= ~(CC_Z))
#define CLEAR_NZV(cpustate)			(cpustate->ccr &= ~(CC_N | CC_Z | CC_V))
#define CLEAR_NZVC(cpustate)		(cpustate->ccr &= ~(CC_N | CC_Z | CC_V | CC_C))
#define CLEAR_HNZVC(cpustate)		(cpustate->ccr &= ~(CC_H | CC_N | CC_Z | CC_V | CC_C))

#define SET_ZFLAG(cpustate)			(cpustate->ccr |= CC_Z)
#define SET_NFLAG(cpustate)			(cpustate->ccr |= CC_N)
#define SET_VFLAG(cpustate)			(cpustate->ccr |= CC_V)

#define REG_A				cpustate->d.d8.a
#define REG_B				cpustate->d.d8.b
#define REG_D				cpustate->d.d16

INLINE void CYCLES(hc11_state *cpustate, int cycles)
{
	cpustate->icount -= cycles;
}

INLINE void SET_PC(hc11_state *cpustate, int pc)
{
	cpustate->pc = pc;
}

INLINE void PUSH8(hc11_state *cpustate, UINT8 value)
{
	WRITE8(cpustate, cpustate->sp--, value);
}

INLINE void PUSH16(hc11_state *cpustate, UINT16 value)
{
	WRITE8(cpustate, cpustate->sp--, (value >> 0) & 0xff);
	WRITE8(cpustate, cpustate->sp--, (value >> 8) & 0xff);
}

INLINE UINT8 POP8(hc11_state *cpustate)
{
	return READ8(cpustate, ++cpustate->sp);
}

INLINE UINT16 POP16(hc11_state *cpustate)
{
	UINT16 r = 0;
	r |= (READ8(cpustate, ++cpustate->sp) << 8);
	r |= (READ8(cpustate, ++cpustate->sp) << 0);
	return r;
}



/*****************************************************************************/

/* ABA              0x1B */
static void HC11OP(aba)(hc11_state *cpustate)
{
	UINT16 r = REG_A + REG_B;
	CLEAR_HNZVC(cpustate);
	SET_H(r, REG_B, REG_A);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, REG_B, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(cpustate, 2);
}


/* ABX              0x3A */
static void HC11OP(abx)(hc11_state *cpustate)
{
	cpustate->ix += REG_B;
	CYCLES(cpustate, 3);
}


/* ABY              0x18, 0x3A */
static void HC11OP(aby)(hc11_state *cpustate)
{
	cpustate->iy += REG_B;
	CYCLES(cpustate, 4);
}


/* ADCA IMM         0x89 */
static void HC11OP(adca_imm)(hc11_state *cpustate)
{
	int c = (cpustate->ccr & CC_C) ? 1 : 0;
	UINT8 i = FETCH(cpustate);
	UINT16 r = REG_A + i + c;
	CLEAR_HNZVC(cpustate);
	SET_H(r, i+c, REG_A);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i+c, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(cpustate, 2);
}

/* ADCA DIR         0x99 */
static void HC11OP(adca_dir)(hc11_state *cpustate)
{
	int c = (cpustate->ccr & CC_C) ? 1 : 0;
	UINT8 d = FETCH(cpustate);
	UINT8 i = READ8(cpustate, d);
	UINT16 r = REG_A + i + c;
	CLEAR_HNZVC(cpustate);
	SET_H(r, i+c, REG_A);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i+c, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(cpustate, 3);
}

/* ADCA EXT         0xB9 */
static void HC11OP(adca_ext)(hc11_state *cpustate)
{
	int c = (cpustate->ccr & CC_C) ? 1 : 0;
	UINT16 adr = FETCH16(cpustate);
	UINT8 i = READ8(cpustate, adr);
	UINT16 r = REG_A + i + c;
	CLEAR_HNZVC(cpustate);
	SET_H(r, i+c, REG_A);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i+c, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(cpustate, 4);
}

/* ADCA IND, X      0xA9 */
static void HC11OP(adca_indx)(hc11_state *cpustate)
{
	int c = (cpustate->ccr & CC_C) ? 1 : 0;
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->ix + offset);
	UINT16 r = REG_A + i + c;
	CLEAR_HNZVC(cpustate);
	SET_H(r, i+c, REG_A);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i+c, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(cpustate, 4);
}

/* ADCA IND, Y      0x18, 0xA9 */
static void HC11OP(adca_indy)(hc11_state *cpustate)
{
	int c = (cpustate->ccr & CC_C) ? 1 : 0;
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->iy + offset);
	UINT16 r = REG_A + i + c;
	CLEAR_HNZVC(cpustate);
	SET_H(r, i+c, REG_A);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i+c, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(cpustate, 5);
}


/* ADCB IMM         0xC9 */
static void HC11OP(adcb_imm)(hc11_state *cpustate)
{
	int c = (cpustate->ccr & CC_C) ? 1 : 0;
	UINT8 i = FETCH(cpustate);
	UINT16 r = REG_B + i + c;
	CLEAR_HNZVC(cpustate);
	SET_H(r, i+c, REG_B);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i+c, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(cpustate, 2);
}

/* ADCB DIR         0xD9 */
static void HC11OP(adcb_dir)(hc11_state *cpustate)
{
	int c = (cpustate->ccr & CC_C) ? 1 : 0;
	UINT8 d = FETCH(cpustate);
	UINT8 i = READ8(cpustate, d);
	UINT16 r = REG_B + i + c;
	CLEAR_HNZVC(cpustate);
	SET_H(r, i+c, REG_B);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i+c, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(cpustate, 3);
}

/* ADCB EXT         0xF9 */
static void HC11OP(adcb_ext)(hc11_state *cpustate)
{
	int c = (cpustate->ccr & CC_C) ? 1 : 0;
	UINT16 adr = FETCH16(cpustate);
	UINT8 i = READ8(cpustate, adr);
	UINT16 r = REG_B + i + c;
	CLEAR_HNZVC(cpustate);
	SET_H(r, i+c, REG_B);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i+c, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(cpustate, 4);
}

/* ADCB IND, X      0xE9 */
static void HC11OP(adcb_indx)(hc11_state *cpustate)
{
	int c = (cpustate->ccr & CC_C) ? 1 : 0;
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->ix + offset);
	UINT16 r = REG_B + i + c;
	CLEAR_HNZVC(cpustate);
	SET_H(r, i+c, REG_B);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i+c, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(cpustate, 4);
}

/* ADCB IND, Y      0x18, 0xE9 */
static void HC11OP(adcb_indy)(hc11_state *cpustate)
{
	int c = (cpustate->ccr & CC_C) ? 1 : 0;
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->iy + offset);
	UINT16 r = REG_B + i + c;
	CLEAR_HNZVC(cpustate);
	SET_H(r, i+c, REG_B);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i+c, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(cpustate, 5);
}


/* ADDA IMM         0x8B */
static void HC11OP(adda_imm)(hc11_state *cpustate)
{
	UINT8 i = FETCH(cpustate);
	UINT16 r = REG_A + i;
	CLEAR_HNZVC(cpustate);
	SET_H(r, i, REG_A);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(cpustate, 2);
}

/* ADDA DIR         0x9B */
static void HC11OP(adda_dir)(hc11_state *cpustate)
{
	UINT8 d = FETCH(cpustate);
	UINT8 i = READ8(cpustate, d);
	UINT16 r = REG_A + i;
	CLEAR_HNZVC(cpustate);
	SET_H(r, i, REG_A);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(cpustate, 3);
}

/* ADDA EXT         0xBB */
static void HC11OP(adda_ext)(hc11_state *cpustate)
{
	UINT16 adr = FETCH16(cpustate);
	UINT8 i = READ8(cpustate, adr);
	UINT16 r = REG_A + i;
	CLEAR_HNZVC(cpustate);
	SET_H(r, i, REG_A);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(cpustate, 4);
}

/* ADDA IND, X      0xAB */
static void HC11OP(adda_indx)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->ix + offset);
	UINT16 r = REG_A + i;
	CLEAR_HNZVC(cpustate);
	SET_H(r, i, REG_A);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(cpustate, 4);
}

/* ADDA IND, Y      0x18, 0xAB */
static void HC11OP(adda_indy)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->iy + offset);
	UINT16 r = REG_A + i;
	CLEAR_HNZVC(cpustate);
	SET_H(r, i, REG_A);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(cpustate, 5);
}


/* ADDB IMM         0xCB */
static void HC11OP(addb_imm)(hc11_state *cpustate)
{
	UINT8 i = FETCH(cpustate);
	UINT16 r = REG_B + i;
	CLEAR_HNZVC(cpustate);
	SET_H(r, i, REG_B);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(cpustate, 2);
}

/* ADDB DIR         0xDB */
static void HC11OP(addb_dir)(hc11_state *cpustate)
{
	UINT8 d = FETCH(cpustate);
	UINT8 i = READ8(cpustate, d);
	UINT16 r = REG_B + i;
	CLEAR_HNZVC(cpustate);
	SET_H(r, i, REG_B);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(cpustate, 3);
}

/* ADDB EXT         0xFB */
static void HC11OP(addb_ext)(hc11_state *cpustate)
{
	UINT16 adr = FETCH16(cpustate);
	UINT8 i = READ8(cpustate, adr);
	UINT16 r = REG_B + i;
	CLEAR_HNZVC(cpustate);
	SET_H(r, i, REG_B);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(cpustate, 4);
}

/* ADDB IND, X      0xEB */
static void HC11OP(addb_indx)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->ix + offset);
	UINT16 r = REG_B + i;
	CLEAR_HNZVC(cpustate);
	SET_H(r, i, REG_B);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(cpustate, 4);
}

/* ADDB IND, Y      0x18, 0xEB */
static void HC11OP(addb_indy)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->iy + offset);
	UINT16 r = REG_B + i;
	CLEAR_HNZVC(cpustate);
	SET_H(r, i, REG_B);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(cpustate, 5);
}


/* ADDD IMM         0xC3 */
static void HC11OP(addd_imm)(hc11_state *cpustate)
{
	UINT16 i = FETCH16(cpustate);
	UINT32 r = REG_D + i;
	CLEAR_NZVC(cpustate);
	SET_N16(r);
	SET_Z16(r);
	SET_V_ADD16(r, i, REG_D);
	SET_C16(r);
	REG_D = (UINT16)r;
	CYCLES(cpustate, 4);
}

/* ADDD DIR         0xD3 */
static void HC11OP(addd_dir)(hc11_state *cpustate)
{
	UINT8 d = FETCH(cpustate);
	UINT16 i = READ16(cpustate, d);
	UINT32 r = REG_D + i;
	CLEAR_NZVC(cpustate);
	SET_N16(r);
	SET_Z16(r);
	SET_V_ADD16(r, i, REG_D);
	SET_C16(r);
	REG_D = (UINT16)r;
	CYCLES(cpustate, 5);
}

/* ADDD EXT         0xF3 */
static void HC11OP(addd_ext)(hc11_state *cpustate)
{
	UINT16 adr = FETCH16(cpustate);
	UINT16 i = READ16(cpustate, adr);
	UINT32 r = REG_D + i;
	CLEAR_NZVC(cpustate);
	SET_N16(r);
	SET_Z16(r);
	SET_V_ADD16(r, i, REG_D);
	SET_C16(r);
	REG_D = (UINT16)r;
	CYCLES(cpustate, 6);
}

/* ADDD IND, X      0xE3 */
static void HC11OP(addd_indx)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT16 i = READ16(cpustate, cpustate->ix + offset);
	UINT32 r = REG_D + i;
	CLEAR_NZVC(cpustate);
	SET_N16(r);
	SET_Z16(r);
	SET_V_ADD16(r, i, REG_D);
	SET_C16(r);
	REG_D = (UINT16)r;
	CYCLES(cpustate, 6);
}

/* ADDD IND, Y      0x18, 0xE3 */
static void HC11OP(addd_indy)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT16 i = READ16(cpustate, cpustate->iy + offset);
	UINT32 r = REG_D + i;
	CLEAR_NZVC(cpustate);
	SET_N16(r);
	SET_Z16(r);
	SET_V_ADD16(r, i, REG_D);
	SET_C16(r);
	REG_D = (UINT16)r;
	CYCLES(cpustate, 7);
}


/* ANDA IMM         0x84 */
static void HC11OP(anda_imm)(hc11_state *cpustate)
{
	UINT8 i = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	REG_A &= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(cpustate, 2);
}

/* ANDA DIR         0x94 */
static void HC11OP(anda_dir)(hc11_state *cpustate)
{
	UINT8 d = FETCH(cpustate);
	UINT8 i = READ8(cpustate, d);
	CLEAR_NZV(cpustate);
	REG_A &= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(cpustate, 3);
}

/* ANDA EXT         0xB4 */
static void HC11OP(anda_ext)(hc11_state *cpustate)
{
	UINT16 adr = FETCH16(cpustate);
	UINT8 i = READ8(cpustate, adr);
	CLEAR_NZV(cpustate);
	REG_A &= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(cpustate, 4);
}

/* ANDA IND, X      0xA4 */
static void HC11OP(anda_indx)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->ix + offset);
	CLEAR_NZV(cpustate);
	REG_A &= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(cpustate, 4);
}

/* ANDA IND, Y      0x18, 0xA4 */
static void HC11OP(anda_indy)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->iy + offset);
	CLEAR_NZV(cpustate);
	REG_A &= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(cpustate, 5);
}


/* ANDB IMM         0xC4 */
static void HC11OP(andb_imm)(hc11_state *cpustate)
{
	UINT8 i = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	REG_B &= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(cpustate, 2);
}

/* ANDB DIR         0xD4 */
static void HC11OP(andb_dir)(hc11_state *cpustate)
{
	UINT8 d = FETCH(cpustate);
	UINT8 i = READ8(cpustate, d);
	CLEAR_NZV(cpustate);
	REG_B &= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(cpustate, 3);
}

/* ANDB EXT         0xF4 */
static void HC11OP(andb_ext)(hc11_state *cpustate)
{
	UINT16 adr = FETCH16(cpustate);
	UINT8 i = READ8(cpustate, adr);
	CLEAR_NZV(cpustate);
	REG_B &= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(cpustate, 4);
}

/* ANDB IND, X      0xE4 */
static void HC11OP(andb_indx)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->ix + offset);
	CLEAR_NZV(cpustate);
	REG_B &= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(cpustate, 4);
}

/* ANDB IND, Y      0x18, 0xE4 */
static void HC11OP(andb_indy)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->iy + offset);
	CLEAR_NZV(cpustate);
	REG_B &= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(cpustate, 5);
}


/* BITA IMM         0x85 */
static void HC11OP(bita_imm)(hc11_state *cpustate)
{
	UINT8 i = FETCH(cpustate);
	UINT8 r = REG_A & i;
	CLEAR_NZV(cpustate);
	SET_N8(r);
	SET_Z8(r);
	CYCLES(cpustate, 2);
}

/* BITA DIR         0x95 */
static void HC11OP(bita_dir)(hc11_state *cpustate)
{
	UINT8 d = FETCH(cpustate);
	UINT8 i = READ8(cpustate, d);
	UINT8 r = REG_A & i;
	CLEAR_NZV(cpustate);
	SET_N8(r);
	SET_Z8(r);
	CYCLES(cpustate, 3);
}

/* BITA EXT         0xB5 */
static void HC11OP(bita_ext)(hc11_state *cpustate)
{
	UINT16 adr = FETCH16(cpustate);
	UINT8 i = READ8(cpustate, adr);
	UINT8 r = REG_A & i;
	CLEAR_NZV(cpustate);
	SET_N8(r);
	SET_Z8(r);
	CYCLES(cpustate, 4);
}

/* BITA IND, X      0xA5 */
static void HC11OP(bita_indx)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->ix + offset);
	UINT8 r = REG_A & i;
	CLEAR_NZV(cpustate);
	SET_N8(r);
	SET_Z8(r);
	CYCLES(cpustate, 4);
}

/* BITA IND, Y      0x18, 0xA5 */
static void HC11OP(bita_indy)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->iy + offset);
	UINT8 r = REG_A & i;
	CLEAR_NZV(cpustate);
	SET_N8(r);
	SET_Z8(r);
	CYCLES(cpustate, 5);
}


/* BITB IMM         0xC5 */
static void HC11OP(bitb_imm)(hc11_state *cpustate)
{
	UINT8 i = FETCH(cpustate);
	UINT8 r = REG_B & i;
	CLEAR_NZV(cpustate);
	SET_N8(r);
	SET_Z8(r);
	CYCLES(cpustate, 2);
}

/* BITB DIR         0xD5 */
static void HC11OP(bitb_dir)(hc11_state *cpustate)
{
	UINT8 d = FETCH(cpustate);
	UINT8 i = READ8(cpustate, d);
	UINT8 r = REG_B & i;
	CLEAR_NZV(cpustate);
	SET_N8(r);
	SET_Z8(r);
	CYCLES(cpustate, 3);
}

/* BITB EXT         0xF5 */
static void HC11OP(bitb_ext)(hc11_state *cpustate)
{
	UINT16 adr = FETCH16(cpustate);
	UINT8 i = READ8(cpustate, adr);
	UINT8 r = REG_B & i;
	CLEAR_NZV(cpustate);
	SET_N8(r);
	SET_Z8(r);
	CYCLES(cpustate, 4);
}

/* BITB IND, X      0xE5 */
static void HC11OP(bitb_indx)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->ix + offset);
	UINT8 r = REG_B & i;
	CLEAR_NZV(cpustate);
	SET_N8(r);
	SET_Z8(r);
	CYCLES(cpustate, 4);
}

/* BITB IND, Y      0x18, 0xE5 */
static void HC11OP(bitb_indy)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->iy + offset);
	UINT8 r = REG_B & i;
	CLEAR_NZV(cpustate);
	SET_N8(r);
	SET_Z8(r);
	CYCLES(cpustate, 5);
}


/* BCS              0x25 */
static void HC11OP(bcs)(hc11_state *cpustate)
{
	INT8 rel = FETCH(cpustate);
	if (cpustate->ccr & CC_C)			/* Branch if C flag set */
	{
		SET_PC(cpustate, cpustate->ppc + rel + 2);
	}
	CYCLES(cpustate, 3);
}


/* BEQ              0x27 */
static void HC11OP(beq)(hc11_state *cpustate)
{
	INT8 rel = FETCH(cpustate);
	if (cpustate->ccr & CC_Z)			/* Branch if Z flag set */
	{
		SET_PC(cpustate, cpustate->ppc + rel + 2);
	}
	CYCLES(cpustate, 3);
}


/* BNE              0x26 */
static void HC11OP(bne)(hc11_state *cpustate)
{
	INT8 rel = FETCH(cpustate);
	if ((cpustate->ccr & CC_Z) == 0)		/* Branch if Z flag clear */
	{
		SET_PC(cpustate, cpustate->ppc + rel + 2);
	}
	CYCLES(cpustate, 3);
}


/* BLE              0x2F */
static void HC11OP(ble)(hc11_state *cpustate)
{
	UINT8 n = (cpustate->ccr & CC_N) ? 1 : 0;
	UINT8 v = (cpustate->ccr & CC_V) ? 1 : 0;
	INT8 rel = FETCH(cpustate);
	if ((cpustate->ccr & CC_Z) || (n ^ v))	/* Branch if Z flag set or (N ^ V) */
	{
		SET_PC(cpustate, cpustate->ppc + rel + 2);
	}
	CYCLES(cpustate, 3);
}


/* BPL              0x2A */
static void HC11OP(bpl)(hc11_state *cpustate)
{
	INT8 rel = FETCH(cpustate);
	if ((cpustate->ccr & CC_N) == 0)		/* Branch if N flag clear */
	{
		SET_PC(cpustate, cpustate->ppc + rel + 2);
	}
	CYCLES(cpustate, 3);
}


/* BRA              0x20 */
static void HC11OP(bra)(hc11_state *cpustate)
{
	INT8 rel = FETCH(cpustate);
	SET_PC(cpustate, cpustate->ppc + rel + 2);
	CYCLES(cpustate, 3);
}


/* BSR              0x8D */
static void HC11OP(bsr)(hc11_state *cpustate)
{
	INT8 rel = FETCH(cpustate);
	UINT16 rt_adr = cpustate->pc;
	PUSH16(cpustate, rt_adr);
	SET_PC(cpustate, cpustate->ppc + rel + 2);
	CYCLES(cpustate, 6);
}


/* CLI              0x0E */
static void HC11OP(cli)(hc11_state *cpustate)
{
	cpustate->ccr &= ~CC_I;
	CYCLES(cpustate, 2);
}


/* CLRA             0x4F */
static void HC11OP(clra)(hc11_state *cpustate)
{
	REG_A = 0;
	CLEAR_NZVC(cpustate);
	SET_ZFLAG(cpustate);
	CYCLES(cpustate, 2);
}

/* CLRB             0x5F */
static void HC11OP(clrb)(hc11_state *cpustate)
{
	REG_B = 0;
	CLEAR_NZVC(cpustate);
	SET_ZFLAG(cpustate);
	CYCLES(cpustate, 2);
}

/* CLR EXT          0x7F */
static void HC11OP(clr_ext)(hc11_state *cpustate)
{
	UINT16 adr = FETCH16(cpustate);
	WRITE8(cpustate, adr, 0);
	CLEAR_NZVC(cpustate);
	SET_ZFLAG(cpustate);
	CYCLES(cpustate, 6);
}

/* CLR IND, X       0x6F */
static void HC11OP(clr_indx)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	WRITE8(cpustate, cpustate->ix + offset, 0);
	CLEAR_NZVC(cpustate);
	SET_ZFLAG(cpustate);
	CYCLES(cpustate, 6);
}

/* CLR IND, Y       0x18, 0x6F */
static void HC11OP(clr_indy)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	WRITE8(cpustate, cpustate->iy + offset, 0);
	CLEAR_NZVC(cpustate);
	SET_ZFLAG(cpustate);
	CYCLES(cpustate, 7);
}


/* CMPA IMM         0x81 */
static void HC11OP(cmpa_imm)(hc11_state *cpustate)
{
	UINT8 i = FETCH(cpustate);
	UINT16 r = REG_A - i;
	CLEAR_NZVC(cpustate);
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_A);
	SET_C8(r);
	CYCLES(cpustate, 2);
}

/* CMPA DIR         0x91 */
static void HC11OP(cmpa_dir)(hc11_state *cpustate)
{
	UINT8 d = FETCH(cpustate);
	UINT8 i = READ8(cpustate, d);
	UINT16 r = REG_A - i;
	CLEAR_NZVC(cpustate);
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_A);
	SET_C8(r);
	CYCLES(cpustate, 3);
}

/* CMPA EXT         0xB1 */
static void HC11OP(cmpa_ext)(hc11_state *cpustate)
{
	UINT16 adr = FETCH16(cpustate);
	UINT8 i = READ8(cpustate, adr);
	UINT16 r = REG_A - i;
	CLEAR_NZVC(cpustate);
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_A);
	SET_C8(r);
	CYCLES(cpustate, 4);
}

/* CMPA IND, X      0xA1 */
static void HC11OP(cmpa_indx)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->ix + offset);
	UINT16 r = REG_A - i;
	CLEAR_NZVC(cpustate);
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_A);
	SET_C8(r);
	CYCLES(cpustate, 4);
}

/* CMPA IND, Y      0x18, 0xA1 */
static void HC11OP(cmpa_indy)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->iy + offset);
	UINT16 r = REG_A - i;
	CLEAR_NZVC(cpustate);
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_A);
	SET_C8(r);
	CYCLES(cpustate, 5);
}


/* CMPB IMM         0xC1 */
static void HC11OP(cmpb_imm)(hc11_state *cpustate)
{
	UINT8 i = FETCH(cpustate);
	UINT16 r = REG_B - i;
	CLEAR_NZVC(cpustate);
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_B);
	SET_C8(r);
	CYCLES(cpustate, 2);
}

/* CMPB DIR         0xD1 */
static void HC11OP(cmpb_dir)(hc11_state *cpustate)
{
	UINT8 d = FETCH(cpustate);
	UINT8 i = READ8(cpustate, d);
	UINT16 r = REG_B - i;
	CLEAR_NZVC(cpustate);
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_B);
	SET_C8(r);
	CYCLES(cpustate, 3);
}

/* CMPB EXT         0xF1 */
static void HC11OP(cmpb_ext)(hc11_state *cpustate)
{
	UINT16 adr = FETCH16(cpustate);
	UINT8 i = READ8(cpustate, adr);
	UINT16 r = REG_B - i;
	CLEAR_NZVC(cpustate);
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_B);
	SET_C8(r);
	CYCLES(cpustate, 4);
}

/* CMPB IND, X      0xE1 */
static void HC11OP(cmpb_indx)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->ix + offset);
	UINT16 r = REG_B - i;
	CLEAR_NZVC(cpustate);
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_B);
	SET_C8(r);
	CYCLES(cpustate, 4);
}

/* CMPB IND, Y      0x18, 0xE1 */
static void HC11OP(cmpb_indy)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->iy + offset);
	UINT16 r = REG_B - i;
	CLEAR_NZVC(cpustate);
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_B);
	SET_C8(r);
	CYCLES(cpustate, 5);
}


/* CPD IMM          0x1A, 0x83 */
static void HC11OP(cpd_imm)(hc11_state *cpustate)
{
	UINT16 i = FETCH16(cpustate);
	UINT32 r = REG_D - i;
	CLEAR_NZVC(cpustate);
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, REG_D);
	SET_C16(r);
	CYCLES(cpustate, 5);
}

/* CPD DIR          0x1A, 0x93 */
static void HC11OP(cpd_dir)(hc11_state *cpustate)
{
	UINT8 d = FETCH(cpustate);
	UINT16 i = READ16(cpustate, d);
	UINT32 r = REG_D - i;
	CLEAR_NZVC(cpustate);
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, REG_D);
	SET_C16(r);
	CYCLES(cpustate, 6);
}

/* CPD EXT          0x1A, 0xB3 */
static void HC11OP(cpd_ext)(hc11_state *cpustate)
{
	UINT16 adr = FETCH16(cpustate);
	UINT16 i = READ16(cpustate, adr);
	UINT32 r = REG_D - i;
	CLEAR_NZVC(cpustate);
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, REG_D);
	SET_C16(r);
	CYCLES(cpustate, 7);
}

/* CPD IND, X       0x1A, 0xA3 */
static void HC11OP(cpd_indx)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT16 i = READ16(cpustate, cpustate->ix + offset);
	UINT32 r = REG_D - i;
	CLEAR_NZVC(cpustate);
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, REG_D);
	SET_C16(r);
	CYCLES(cpustate, 7);
}

/* CPD IND, Y       0xCD, 0xA3 */
static void HC11OP(cpd_indy)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT16 i = READ16(cpustate, cpustate->iy + offset);
	UINT32 r = REG_D - i;
	CLEAR_NZVC(cpustate);
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, REG_D);
	SET_C16(r);
	CYCLES(cpustate, 7);
}


/* CPX IMM          0x8C */
static void HC11OP(cpx_imm)(hc11_state *cpustate)
{
	UINT16 i = FETCH16(cpustate);
	UINT32 r = cpustate->ix - i;
	CLEAR_NZVC(cpustate);
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, cpustate->ix);
	SET_C16(r);
	CYCLES(cpustate, 4);
}

/* CPX DIR          0x9C */
static void HC11OP(cpx_dir)(hc11_state *cpustate)
{
	UINT8 d = FETCH(cpustate);
	UINT16 i = READ16(cpustate, d);
	UINT32 r = cpustate->ix - i;
	CLEAR_NZVC(cpustate);
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, cpustate->ix);
	SET_C16(r);
	CYCLES(cpustate, 5);
}

/* CPX EXT          0xBC */
static void HC11OP(cpx_ext)(hc11_state *cpustate)
{
	UINT16 adr = FETCH16(cpustate);
	UINT16 i = READ16(cpustate, adr);
	UINT32 r = cpustate->ix - i;
	CLEAR_NZVC(cpustate);
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, cpustate->ix);
	SET_C16(r);
	CYCLES(cpustate, 6);
}

/* CPX IND, X       0xAC */
static void HC11OP(cpx_indx)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT16 i = READ16(cpustate, cpustate->ix + offset);
	UINT32 r = cpustate->ix - i;
	CLEAR_NZVC(cpustate);
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, cpustate->ix);
	SET_C16(r);
	CYCLES(cpustate, 6);
}

/* CPX IND, Y       0xCD, 0xAC */
static void HC11OP(cpx_indy)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT16 i = READ16(cpustate, cpustate->iy + offset);
	UINT32 r = cpustate->ix - i;
	CLEAR_NZVC(cpustate);
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, cpustate->ix);
	SET_C16(r);
	CYCLES(cpustate, 7);
}


/* CPY IMM          0x18, 0x8C */
static void HC11OP(cpy_imm)(hc11_state *cpustate)
{
	UINT16 i = FETCH16(cpustate);
	UINT32 r = cpustate->iy - i;
	CLEAR_NZVC(cpustate);
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, cpustate->iy);
	SET_C16(r);
	CYCLES(cpustate, 5);
}


/* DEX              0x09 */
static void HC11OP(dex)(hc11_state *cpustate)
{
	CLEAR_Z(cpustate);
	cpustate->ix--;
	SET_Z16(cpustate->ix);
	CYCLES(cpustate, 3);
}


/* DEY              0x18, 0x09 */
static void HC11OP(dey)(hc11_state *cpustate)
{
	CLEAR_Z(cpustate);
	cpustate->iy--;
	SET_Z16(cpustate->iy);
	CYCLES(cpustate, 4);
}


/* EORA IMM         0x88 */
static void HC11OP(eora_imm)(hc11_state *cpustate)
{
	UINT8 i = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	REG_A ^= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(cpustate, 2);
}

/* EORA DIR         0x98 */
static void HC11OP(eora_dir)(hc11_state *cpustate)
{
	UINT8 d = FETCH(cpustate);
	UINT8 i = READ8(cpustate, d);
	CLEAR_NZV(cpustate);
	REG_A ^= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(cpustate, 3);
}

/* EORA EXT         0xB8 */
static void HC11OP(eora_ext)(hc11_state *cpustate)
{
	UINT16 adr = FETCH16(cpustate);
	UINT8 i = READ8(cpustate, adr);
	CLEAR_NZV(cpustate);
	REG_A ^= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(cpustate, 4);
}

/* EORA IND, X      0xA8 */
static void HC11OP(eora_indx)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->ix + offset);
	CLEAR_NZV(cpustate);
	REG_A ^= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(cpustate, 4);
}

/* EORA IND, Y      0x18, 0xA8 */
static void HC11OP(eora_indy)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->iy + offset);
	CLEAR_NZV(cpustate);
	REG_A ^= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(cpustate, 5);
}


/* EORB IMM         0xC8 */
static void HC11OP(eorb_imm)(hc11_state *cpustate)
{
	UINT8 i = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	REG_B ^= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(cpustate, 2);
}

/* EORB DIR         0xD8 */
static void HC11OP(eorb_dir)(hc11_state *cpustate)
{
	UINT8 d = FETCH(cpustate);
	UINT8 i = READ8(cpustate, d);
	CLEAR_NZV(cpustate);
	REG_B ^= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(cpustate, 3);
}

/* EORB EXT         0xF8 */
static void HC11OP(eorb_ext)(hc11_state *cpustate)
{
	UINT16 adr = FETCH16(cpustate);
	UINT8 i = READ8(cpustate, adr);
	CLEAR_NZV(cpustate);
	REG_B ^= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(cpustate, 4);
}

/* EORB IND, X      0xE8 */
static void HC11OP(eorb_indx)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->ix + offset);
	CLEAR_NZV(cpustate);
	REG_B ^= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(cpustate, 4);
}

/* EORB IND, Y      0x18, 0xE8 */
static void HC11OP(eorb_indy)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->iy + offset);
	CLEAR_NZV(cpustate);
	REG_B ^= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(cpustate, 5);
}


/* INCA             0x4C */
static void HC11OP(inca)(hc11_state *cpustate)
{
	CLEAR_NZV(cpustate);
	if (REG_A == 0x7f)
		SET_VFLAG(cpustate);
	REG_A++;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(cpustate, 2);
}


/* INX              0x08 */
static void HC11OP(inx)(hc11_state *cpustate)
{
	CLEAR_Z(cpustate);
	cpustate->ix++;
	SET_Z16(cpustate->ix);
	CYCLES(cpustate, 3);
}

/* INY              0x18, 0x08 */
static void HC11OP(iny)(hc11_state *cpustate)
{
	CLEAR_Z(cpustate);
	cpustate->iy++;
	SET_Z16(cpustate->iy);
	CYCLES(cpustate, 4);
}


/* JMP EXT          0x7E */
static void HC11OP(jmp_ext)(hc11_state *cpustate)
{
	UINT16 adr = FETCH16(cpustate);
	SET_PC(cpustate, adr);
	CYCLES(cpustate, 3);
}


/* JSR DIR          0x9D */
static void HC11OP(jsr_dir)(hc11_state *cpustate)
{
	UINT8 i = FETCH(cpustate);
	UINT16 rt_adr = cpustate->pc;
	PUSH16(cpustate, rt_adr);
	SET_PC(cpustate, i);
	CYCLES(cpustate, 5);
}

/* JSR EXT          0xBD */
static void HC11OP(jsr_ext)(hc11_state *cpustate)
{
	UINT16 adr = FETCH16(cpustate);
	UINT16 rt_adr = cpustate->pc;
	PUSH16(cpustate, rt_adr);
	SET_PC(cpustate, adr);
	CYCLES(cpustate, 6);
}

/* JSR IND, X       0xAD */
static void HC11OP(jsr_indx)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT16 rt_adr = cpustate->pc;
	PUSH16(cpustate, rt_adr);
	SET_PC(cpustate, cpustate->ix + offset);
	CYCLES(cpustate, 6);
}

/* JSR IND, Y       0x18, 0xAD */
static void HC11OP(jsr_indy)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT16 rt_adr = cpustate->pc;
	PUSH16(cpustate, rt_adr);
	SET_PC(cpustate, cpustate->iy + offset);
	CYCLES(cpustate, 6);
}


/* LDAA IMM         0x86 */
static void HC11OP(ldaa_imm)(hc11_state *cpustate)
{
	CLEAR_NZV(cpustate);
	REG_A = FETCH(cpustate);
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(cpustate, 2);
}

/* LDAA DIR         0x96 */
static void HC11OP(ldaa_dir)(hc11_state *cpustate)
{
	UINT8 d = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	REG_A = READ8(cpustate, d);
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(cpustate, 3);
}

/* LDAA EXT         0xB6 */
static void HC11OP(ldaa_ext)(hc11_state *cpustate)
{
	UINT16 adr = FETCH16(cpustate);
	CLEAR_NZV(cpustate);
	REG_A = READ8(cpustate, adr);
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(cpustate, 4);
}

/* LDAA IND, X      0xA6 */
static void HC11OP(ldaa_indx)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	REG_A = READ8(cpustate, cpustate->ix + offset);
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(cpustate, 4);
}

/* LDAA IND, Y      0x18, 0xA6 */
static void HC11OP(ldaa_indy)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	REG_A = READ8(cpustate, cpustate->iy + offset);
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(cpustate, 5);
}

/* LDAB IMM         0xC6 */
static void HC11OP(ldab_imm)(hc11_state *cpustate)
{
	CLEAR_NZV(cpustate);
	REG_B = FETCH(cpustate);
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(cpustate, 2);
}

/* LDAB DIR         0xD6 */
static void HC11OP(ldab_dir)(hc11_state *cpustate)
{
	UINT8 d = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	REG_B = READ8(cpustate, d);
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(cpustate, 3);
}

/* LDAB EXT         0xF6 */
static void HC11OP(ldab_ext)(hc11_state *cpustate)
{
	UINT16 adr = FETCH16(cpustate);
	CLEAR_NZV(cpustate);
	REG_B = READ8(cpustate, adr);
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(cpustate, 4);
}

/* LDAB IND, X      0xE6 */
static void HC11OP(ldab_indx)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	REG_B = READ8(cpustate, cpustate->ix + offset);
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(cpustate, 4);
}

/* LDAB IND, Y      0x18, 0xE6 */
static void HC11OP(ldab_indy)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	REG_B = READ8(cpustate, cpustate->iy + offset);
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(cpustate, 5);
}


/* LDD IMM          0xCC */
static void HC11OP(ldd_imm)(hc11_state *cpustate)
{
	CLEAR_NZV(cpustate);
	REG_D = FETCH16(cpustate);
	SET_N16(REG_D);
	SET_Z16(REG_D);
	CYCLES(cpustate, 3);
}

/* LDD DIR          0xDC */
static void HC11OP(ldd_dir)(hc11_state *cpustate)
{
	UINT8 d = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	REG_D = READ16(cpustate, d);
	SET_N16(REG_D);
	SET_Z16(REG_D);
	CYCLES(cpustate, 4);
}

/* LDD EXT          0xFC */
static void HC11OP(ldd_ext)(hc11_state *cpustate)
{
	UINT16 adr = FETCH16(cpustate);
	CLEAR_NZV(cpustate);
	REG_D = READ16(cpustate, adr);
	SET_N16(REG_D);
	SET_Z16(REG_D);
	CYCLES(cpustate, 5);
}

/* LDD IND, X       0xEC */
static void HC11OP(ldd_indx)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	REG_D = READ16(cpustate, cpustate->ix + offset);
	SET_N16(REG_D);
	SET_Z16(REG_D);
	CYCLES(cpustate, 5);
}

/* LDD IND, Y       0x18, 0xEC */
static void HC11OP(ldd_indy)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	REG_D = READ16(cpustate, cpustate->iy + offset);
	SET_N16(REG_D);
	SET_Z16(REG_D);
	CYCLES(cpustate, 6);
}


/* LDS IMM          0x8E */
static void HC11OP(lds_imm)(hc11_state *cpustate)
{
	CLEAR_NZV(cpustate);
	cpustate->sp = FETCH16(cpustate);
	SET_N16(cpustate->sp);
	SET_Z16(cpustate->sp);
	CYCLES(cpustate, 3);
}

/* LDS DIR          0x9E */
static void HC11OP(lds_dir)(hc11_state *cpustate)
{
	UINT8 i = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	cpustate->sp = READ16(cpustate, i);
	SET_N16(cpustate->sp);
	SET_Z16(cpustate->sp);
	CYCLES(cpustate, 4);
}

/* LDS EXT          0xBE */
static void HC11OP(lds_ext)(hc11_state *cpustate)
{
	UINT16 adr = FETCH16(cpustate);
	CLEAR_NZV(cpustate);
	cpustate->sp = READ16(cpustate, adr);
	SET_N16(cpustate->sp);
	SET_Z16(cpustate->sp);
	CYCLES(cpustate, 5);
}

/* LDS IND, X       0xAE */
static void HC11OP(lds_indx)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	cpustate->sp = READ16(cpustate, cpustate->ix + offset);
	SET_N16(cpustate->sp);
	SET_Z16(cpustate->sp);
	CYCLES(cpustate, 5);
}

/* LDS IND, Y       0x18, 0xAE */
static void HC11OP(lds_indy)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	cpustate->sp = READ16(cpustate, cpustate->iy + offset);
	SET_N16(cpustate->sp);
	SET_Z16(cpustate->sp);
	CYCLES(cpustate, 6);
}


/* LDX IMM          0xCE */
static void HC11OP(ldx_imm)(hc11_state *cpustate)
{
	CLEAR_NZV(cpustate);
	cpustate->ix = FETCH16(cpustate);
	SET_N16(cpustate->ix);
	SET_Z16(cpustate->ix);
	CYCLES(cpustate, 3);
}

/* LDX DIR          0xDE */
static void HC11OP(ldx_dir)(hc11_state *cpustate)
{
	UINT8 d = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	cpustate->ix = READ16(cpustate, d);
	SET_N16(cpustate->ix);
	SET_Z16(cpustate->ix);
	CYCLES(cpustate, 4);
}

/* LDX EXT          0xFE */
static void HC11OP(ldx_ext)(hc11_state *cpustate)
{
	UINT16 adr = FETCH16(cpustate);
	CLEAR_NZV(cpustate);
	cpustate->ix = READ16(cpustate, adr);
	SET_N16(cpustate->ix);
	SET_Z16(cpustate->ix);
	CYCLES(cpustate, 5);
}

/* LDX IND, X       0xEE */
static void HC11OP(ldx_indx)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	cpustate->ix = READ16(cpustate, cpustate->ix + offset);
	SET_N16(cpustate->ix);
	SET_Z16(cpustate->ix);
	CYCLES(cpustate, 5);
}

/* LDX IND, Y       0xCD, 0xEE */
static void HC11OP(ldx_indy)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	cpustate->ix = READ16(cpustate, cpustate->iy + offset);
	SET_N16(cpustate->ix);
	SET_Z16(cpustate->ix);
	CYCLES(cpustate, 6);
}


/* LDY IMM          0x18, 0xCE */
static void HC11OP(ldy_imm)(hc11_state *cpustate)
{
	CLEAR_NZV(cpustate);
	cpustate->iy = FETCH16(cpustate);
	SET_N16(cpustate->iy);
	SET_Z16(cpustate->iy);
	CYCLES(cpustate, 4);
}

/* LDY DIR          0x18, 0xDE */
static void HC11OP(ldy_dir)(hc11_state *cpustate)
{
	UINT8 d = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	cpustate->iy = READ16(cpustate, d);
	SET_N16(cpustate->iy);
	SET_Z16(cpustate->iy);
	CYCLES(cpustate, 5);
}

/* LDY EXT          0x18, 0xFE */
static void HC11OP(ldy_ext)(hc11_state *cpustate)
{
	UINT16 adr = FETCH16(cpustate);
	CLEAR_NZV(cpustate);
	cpustate->iy = READ16(cpustate, adr);
	SET_N16(cpustate->iy);
	SET_Z16(cpustate->iy);
	CYCLES(cpustate, 6);
}

/* LDY IND, X       0x1A, 0xEE */
static void HC11OP(ldy_indx)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	cpustate->iy = READ16(cpustate, cpustate->ix + offset);
	SET_N16(cpustate->iy);
	SET_Z16(cpustate->iy);
	CYCLES(cpustate, 6);
}

/* LDY IND, Y       0x18, 0xEE */
static void HC11OP(ldy_indy)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	cpustate->iy = READ16(cpustate, cpustate->iy + offset);
	SET_N16(cpustate->iy);
	SET_Z16(cpustate->iy);
	CYCLES(cpustate, 6);
}


/* LSLD             0x05 */
static void HC11OP(lsld)(hc11_state *cpustate)
{
	UINT32 r = REG_D << 1;
	CLEAR_NZVC(cpustate);
	SET_C16(r);
	REG_D = (UINT16)(r);
	SET_N16(REG_D);
	SET_Z16(REG_D);

	if (((cpustate->ccr & CC_N) == CC_N && (cpustate->ccr & CC_C) == 0) ||
		((cpustate->ccr & CC_N) == 0 && (cpustate->ccr & CC_C) == CC_C))
	{
		cpustate->ccr |= CC_V;
	}

	CYCLES(cpustate, 3);
}


/* PSHA             0x36 */
static void HC11OP(psha)(hc11_state *cpustate)
{
	PUSH8(cpustate, REG_A);
	CYCLES(cpustate, 3);
}


/* ORAA IMM         0x8A */
static void HC11OP(oraa_imm)(hc11_state *cpustate)
{
	UINT8 i = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	REG_A |= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(cpustate, 2);
}

/* ORAA DIR         0x9A */
static void HC11OP(oraa_dir)(hc11_state *cpustate)
{
	UINT8 d = FETCH(cpustate);
	UINT8 i = READ8(cpustate, d);
	CLEAR_NZV(cpustate);
	REG_A |= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(cpustate, 3);
}

/* ORAA EXT         0xBA */
static void HC11OP(oraa_ext)(hc11_state *cpustate)
{
	UINT16 adr = FETCH16(cpustate);
	UINT8 i = READ8(cpustate, adr);
	CLEAR_NZV(cpustate);
	REG_A |= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(cpustate, 4);
}

/* ORAA IND, X      0xAA */
static void HC11OP(oraa_indx)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->ix + offset);
	CLEAR_NZV(cpustate);
	REG_A |= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(cpustate, 4);
}

/* ORAA IND, Y      0x18, 0xAA */
static void HC11OP(oraa_indy)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->iy + offset);
	CLEAR_NZV(cpustate);
	REG_A |= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(cpustate, 5);
}


/* ORAB IMM         0xCA */
static void HC11OP(orab_imm)(hc11_state *cpustate)
{
	UINT8 i = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	REG_B |= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(cpustate, 2);
}

/* ORAB DIR         0xDA */
static void HC11OP(orab_dir)(hc11_state *cpustate)
{
	UINT8 d = FETCH(cpustate);
	UINT8 i = READ8(cpustate, d);
	CLEAR_NZV(cpustate);
	REG_B |= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(cpustate, 3);
}

/* ORAB EXT         0xFA */
static void HC11OP(orab_ext)(hc11_state *cpustate)
{
	UINT16 adr = FETCH16(cpustate);
	UINT8 i = READ8(cpustate, adr);
	CLEAR_NZV(cpustate);
	REG_B |= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(cpustate, 4);
}

/* ORAB IND, X      0xEA */
static void HC11OP(orab_indx)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->ix + offset);
	CLEAR_NZV(cpustate);
	REG_B |= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(cpustate, 4);
}

/* ORAB IND, Y      0x18, 0xEA */
static void HC11OP(orab_indy)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->iy + offset);
	CLEAR_NZV(cpustate);
	REG_B |= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(cpustate, 5);
}


/* PSHB             0x37 */
static void HC11OP(pshb)(hc11_state *cpustate)
{
	PUSH8(cpustate, REG_B);
	CYCLES(cpustate, 3);
}


/* PSHX             0x3C */
static void HC11OP(pshx)(hc11_state *cpustate)
{
	PUSH16(cpustate, cpustate->ix);
	CYCLES(cpustate, 4);
}


/* PSHY             0x18, 0x3C */
static void HC11OP(pshy)(hc11_state *cpustate)
{
	PUSH16(cpustate, cpustate->iy);
	CYCLES(cpustate, 5);
}


/* PULA             0x32 */
static void HC11OP(pula)(hc11_state *cpustate)
{
	REG_A = POP8(cpustate);
	CYCLES(cpustate, 4);
}


/* PULB             0x33 */
static void HC11OP(pulb)(hc11_state *cpustate)
{
	REG_B = POP8(cpustate);
	CYCLES(cpustate, 4);
}


/* PULX             0x38 */
static void HC11OP(pulx)(hc11_state *cpustate)
{
	cpustate->ix = POP16(cpustate);
	CYCLES(cpustate, 5);
}


/* PULY             0x18, 0x38 */
static void HC11OP(puly)(hc11_state *cpustate)
{
	cpustate->iy = POP16(cpustate);
	CYCLES(cpustate, 6);
}


/* RTS              0x39 */
static void HC11OP(rts)(hc11_state *cpustate)
{
	UINT16 rt_adr = POP16(cpustate);
	SET_PC(cpustate, rt_adr);
	CYCLES(cpustate, 5);
}


/* SEI              0x0F */
static void HC11OP(sei)(hc11_state *cpustate)
{
	cpustate->ccr |= CC_I;
	CYCLES(cpustate, 2);
}


/* STAA DIR         0x97 */
static void HC11OP(staa_dir)(hc11_state *cpustate)
{
	UINT8 d = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	SET_N8(REG_A);
	SET_Z8(REG_A);
	WRITE8(cpustate, d, REG_A);
	CYCLES(cpustate, 3);
}

/* STAA EXT         0xB7 */
static void HC11OP(staa_ext)(hc11_state *cpustate)
{
	UINT16 adr = FETCH16(cpustate);
	CLEAR_NZV(cpustate);
	SET_N8(REG_A);
	SET_Z8(REG_A);
	WRITE8(cpustate, adr, REG_A);
	CYCLES(cpustate, 4);
}

/* STAA IND, X      0xA7 */
static void HC11OP(staa_indx)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	SET_N8(REG_A);
	SET_Z8(REG_A);
	WRITE8(cpustate, cpustate->ix + offset, REG_A);
	CYCLES(cpustate, 4);
}

/* STAA IND, Y      0x18, 0xA7 */
static void HC11OP(staa_indy)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	SET_N8(REG_A);
	SET_Z8(REG_A);
	WRITE8(cpustate, cpustate->iy + offset, REG_A);
	CYCLES(cpustate, 5);
}

/* STAB DIR         0xD7 */
static void HC11OP(stab_dir)(hc11_state *cpustate)
{
	UINT8 d = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	SET_N8(REG_B);
	SET_Z8(REG_B);
	WRITE8(cpustate, d, REG_B);
	CYCLES(cpustate, 3);
}

/* STAB EXT         0xF7 */
static void HC11OP(stab_ext)(hc11_state *cpustate)
{
	UINT16 adr = FETCH16(cpustate);
	CLEAR_NZV(cpustate);
	SET_N8(REG_B);
	SET_Z8(REG_B);
	WRITE8(cpustate, adr, REG_B);
	CYCLES(cpustate, 4);
}

/* STAB IND, X      0xE7 */
static void HC11OP(stab_indx)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	SET_N8(REG_B);
	SET_Z8(REG_B);
	WRITE8(cpustate, cpustate->ix + offset, REG_B);
	CYCLES(cpustate, 4);
}

/* STAB IND, Y      0x18, 0xE7 */
static void HC11OP(stab_indy)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	SET_N8(REG_B);
	SET_Z8(REG_B);
	WRITE8(cpustate, cpustate->iy + offset, REG_B);
	CYCLES(cpustate, 5);
}


/* STD DIR          0xDD */
static void HC11OP(std_dir)(hc11_state *cpustate)
{
	UINT8 d = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	WRITE16(cpustate, d, REG_D);
	SET_N16(REG_D);
	SET_Z16(REG_D);
	CYCLES(cpustate, 4);
}

/* STD EXT          0xFD */
static void HC11OP(std_ext)(hc11_state *cpustate)
{
	UINT16 adr = FETCH16(cpustate);
	CLEAR_NZV(cpustate);
	WRITE16(cpustate, adr, REG_D);
	SET_N16(REG_D);
	SET_Z16(REG_D);
	CYCLES(cpustate, 5);
}

/* STD IND, X       0xED */
static void HC11OP(std_indx)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	WRITE16(cpustate, cpustate->ix + offset, REG_D);
	SET_N16(REG_D);
	SET_Z16(REG_D);
	CYCLES(cpustate, 5);
}

/* STD IND, Y       0x18, 0xED */
static void HC11OP(std_indy)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	CLEAR_NZV(cpustate);
	WRITE16(cpustate, cpustate->iy + offset, REG_D);
	SET_N16(REG_D);
	SET_Z16(REG_D);
	CYCLES(cpustate, 6);
}


/* TAB              0x16 */
static void HC11OP(tab)(hc11_state *cpustate)
{
	CLEAR_NZV(cpustate);
	REG_B = REG_A;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(cpustate, 2);
}


/* TAP              0x06 */
static void HC11OP(tap)(hc11_state *cpustate)
{
	UINT8 ccr = cpustate->ccr;
	cpustate->ccr = (REG_A & 0xbf) | (ccr & 0x40);
	CYCLES(cpustate, 2);
}


/* TPA              0x07 */
static void HC11OP(tpa)(hc11_state *cpustate)
{
	REG_A = cpustate->ccr;
	CYCLES(cpustate, 2);
}


/* TSTA             0x4D */
static void HC11OP(tsta)(hc11_state *cpustate)
{
	CLEAR_NZVC(cpustate);
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(cpustate, 2);
}

/* TSTB             0x5D */
static void HC11OP(tstb)(hc11_state *cpustate)
{
	CLEAR_NZVC(cpustate);
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(cpustate, 2);
}

/* TST EXT          0x7D */
static void HC11OP(tst_ext)(hc11_state *cpustate)
{
	UINT16 adr = FETCH16(cpustate);
	UINT8 i = READ8(cpustate, adr);
	CLEAR_NZVC(cpustate);
	SET_N8(i);
	SET_Z8(i);
	CYCLES(cpustate, 6);
}

/* TST IND, X       0x6D */
static void HC11OP(tst_indx)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->ix + offset);
	CLEAR_NZVC(cpustate);
	SET_N8(i);
	SET_Z8(i);
	CYCLES(cpustate, 6);
}

/* TST IND, Y       0x18, 0x6D */
static void HC11OP(tst_indy)(hc11_state *cpustate)
{
	UINT8 offset = FETCH(cpustate);
	UINT8 i = READ8(cpustate, cpustate->iy + offset);
	CLEAR_NZVC(cpustate);
	SET_N8(i);
	SET_Z8(i);
	CYCLES(cpustate, 6);
}


/* XGDX             0x8F */
static void HC11OP(xgdx)(hc11_state *cpustate)
{
	UINT16 tmp = REG_D;
	REG_D = cpustate->ix;
	cpustate->ix = tmp;
	CYCLES(cpustate, 3);
}


/* XGDY             0x18, 0x8F */
static void HC11OP(xgdy)(hc11_state *cpustate)
{
	UINT16 tmp = REG_D;
	REG_D = cpustate->iy;
	cpustate->iy = tmp;
	CYCLES(cpustate, 4);
}

/*****************************************************************************/

static void HC11OP(page2)(hc11_state *cpustate)
{
	UINT8 op2 = FETCH(cpustate);
	hc11_optable_page2[op2](cpustate);
}

static void HC11OP(page3)(hc11_state *cpustate)
{
	UINT8 op2 = FETCH(cpustate);
	hc11_optable_page3[op2](cpustate);
}

static void HC11OP(page4)(hc11_state *cpustate)
{
	UINT8 op2 = FETCH(cpustate);
	hc11_optable_page4[op2](cpustate);
}

static void HC11OP(invalid)(hc11_state *cpustate)
{
	fatalerror("HC11: Invalid opcode 0x%02X at %04X", READ8(cpustate, cpustate->pc-1), cpustate->pc-1);
}
