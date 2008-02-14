#define SET_Z8(r)			(hc11.ccr |= ((UINT8)r == 0) ? CC_Z : 0)
#define SET_Z16(r)			(hc11.ccr |= ((UINT16)r == 0) ? CC_Z : 0)
#define SET_N8(r)			(hc11.ccr |= (r & 0x80) ? CC_N : 0)
#define SET_N16(r)			(hc11.ccr |= (r & 0x80) ? CC_N : 0)
#define SET_V_ADD8(r,s,d)	(hc11.ccr |= (((r) ^ (s)) & ((r) ^ (d)) & 0x80) ? CC_V : 0)
#define SET_V_SUB8(r,s,d)	(hc11.ccr |= (((d) ^ (s)) & ((d) ^ (r)) & 0x80) ? CC_V : 0)
#define SET_V_ADD16(r,s,d)	(hc11.ccr |= (((r) ^ (s)) & ((r) ^ (d)) & 0x8000) ? CC_V : 0)
#define SET_V_SUB16(r,s,d)	(hc11.ccr |= (((d) ^ (s)) & ((d) ^ (r)) & 0x8000) ? CC_V : 0)
#define SET_H(r,s,d)		(hc11.ccr |= ((((s) & (d)) | ((d) & (r)) | ((r) & (s))) & 0x10) ? CC_H : 0)
#define SET_C8(x)			(hc11.ccr |= ((x) & 0x100) ? CC_C : 0)
#define SET_C16(x)			(hc11.ccr |= ((x) & 0x10000) ? CC_C : 0)
#define CLEAR_Z()			(hc11.ccr &= ~(CC_Z))
#define CLEAR_NZV()			(hc11.ccr &= ~(CC_N | CC_Z | CC_V))
#define CLEAR_NZVC()		(hc11.ccr &= ~(CC_N | CC_Z | CC_V | CC_C))
#define CLEAR_HNZVC()		(hc11.ccr &= ~(CC_H | CC_N | CC_Z | CC_V | CC_C))

#define SET_ZFLAG()			(hc11.ccr |= CC_Z)
#define SET_NFLAG()			(hc11.ccr |= CC_N)
#define SET_VFLAG()			(hc11.ccr |= CC_V)

#define REG_A				hc11.d.d8.a
#define REG_B				hc11.d.d8.b
#define REG_D				hc11.d.d16

INLINE void CYCLES(int cycles)
{
	hc11.icount -= cycles;
}

INLINE void SET_PC(int pc)
{
	hc11.pc = pc;
	change_pc(hc11.pc);
}

INLINE void PUSH8(UINT8 value)
{
	WRITE8(hc11.sp--, value);
}

INLINE void PUSH16(UINT16 value)
{
	WRITE8(hc11.sp--, (value >> 0) & 0xff);
	WRITE8(hc11.sp--, (value >> 8) & 0xff);
}

INLINE UINT8 POP8(void)
{
	return READ8(++hc11.sp);
}

INLINE UINT16 POP16(void)
{
	UINT16 r = 0;
	r |= (READ8(++hc11.sp) << 8);
	r |= (READ8(++hc11.sp) << 0);
	return r;
}



/*****************************************************************************/

/* ABA              0x1B */
static void HC11OP(aba)(void)
{
	UINT16 r = REG_A + REG_B;
	CLEAR_HNZVC();
	SET_H(r, REG_B, REG_A);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, REG_B, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(2);
}


/* ABX              0x3A */
static void HC11OP(abx)(void)
{
	hc11.ix += REG_B;
	CYCLES(3);
}


/* ABY              0x18, 0x3A */
static void HC11OP(aby)(void)
{
	hc11.iy += REG_B;
	CYCLES(4);
}


/* ADCA IMM         0x89 */
static void HC11OP(adca_imm)(void)
{
	int c = (hc11.ccr & CC_C) ? 1 : 0;
	UINT8 i = FETCH();
	UINT16 r = REG_A + i + c;
	CLEAR_HNZVC();
	SET_H(r, i+c, REG_A);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i+c, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(2);
}

/* ADCA DIR         0x99 */
static void HC11OP(adca_dir)(void)
{
	int c = (hc11.ccr & CC_C) ? 1 : 0;
	UINT8 d = FETCH();
	UINT8 i = READ8(d);
	UINT16 r = REG_A + i + c;
	CLEAR_HNZVC();
	SET_H(r, i+c, REG_A);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i+c, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(3);
}

/* ADCA EXT         0xB9 */
static void HC11OP(adca_ext)(void)
{
	int c = (hc11.ccr & CC_C) ? 1 : 0;
	UINT16 adr = FETCH16();
	UINT8 i = READ8(adr);
	UINT16 r = REG_A + i + c;
	CLEAR_HNZVC();
	SET_H(r, i+c, REG_A);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i+c, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(4);
}

/* ADCA IND, X      0xA9 */
static void HC11OP(adca_indx)(void)
{
	int c = (hc11.ccr & CC_C) ? 1 : 0;
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.ix + offset);
	UINT16 r = REG_A + i + c;
	CLEAR_HNZVC();
	SET_H(r, i+c, REG_A);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i+c, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(4);
}

/* ADCA IND, Y      0x18, 0xA9 */
static void HC11OP(adca_indy)(void)
{
	int c = (hc11.ccr & CC_C) ? 1 : 0;
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.iy + offset);
	UINT16 r = REG_A + i + c;
	CLEAR_HNZVC();
	SET_H(r, i+c, REG_A);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i+c, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(5);
}


/* ADCB IMM         0xC9 */
static void HC11OP(adcb_imm)(void)
{
	int c = (hc11.ccr & CC_C) ? 1 : 0;
	UINT8 i = FETCH();
	UINT16 r = REG_B + i + c;
	CLEAR_HNZVC();
	SET_H(r, i+c, REG_B);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i+c, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(2);
}

/* ADCB DIR         0xD9 */
static void HC11OP(adcb_dir)(void)
{
	int c = (hc11.ccr & CC_C) ? 1 : 0;
	UINT8 d = FETCH();
	UINT8 i = READ8(d);
	UINT16 r = REG_B + i + c;
	CLEAR_HNZVC();
	SET_H(r, i+c, REG_B);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i+c, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(3);
}

/* ADCB EXT         0xF9 */
static void HC11OP(adcb_ext)(void)
{
	int c = (hc11.ccr & CC_C) ? 1 : 0;
	UINT16 adr = FETCH16();
	UINT8 i = READ8(adr);
	UINT16 r = REG_B + i + c;
	CLEAR_HNZVC();
	SET_H(r, i+c, REG_B);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i+c, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(4);
}

/* ADCB IND, X      0xE9 */
static void HC11OP(adcb_indx)(void)
{
	int c = (hc11.ccr & CC_C) ? 1 : 0;
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.ix + offset);
	UINT16 r = REG_B + i + c;
	CLEAR_HNZVC();
	SET_H(r, i+c, REG_B);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i+c, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(4);
}

/* ADCB IND, Y      0x18, 0xE9 */
static void HC11OP(adcb_indy)(void)
{
	int c = (hc11.ccr & CC_C) ? 1 : 0;
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.iy + offset);
	UINT16 r = REG_B + i + c;
	CLEAR_HNZVC();
	SET_H(r, i+c, REG_B);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i+c, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(5);
}


/* ADDA IMM         0x8B */
static void HC11OP(adda_imm)(void)
{
	UINT8 i = FETCH();
	UINT16 r = REG_A + i;
	CLEAR_HNZVC();
	SET_H(r, i, REG_A);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(2);
}

/* ADDA DIR         0x9B */
static void HC11OP(adda_dir)(void)
{
	UINT8 d = FETCH();
	UINT8 i = READ8(d);
	UINT16 r = REG_A + i;
	CLEAR_HNZVC();
	SET_H(r, i, REG_A);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(3);
}

/* ADDA EXT         0xBB */
static void HC11OP(adda_ext)(void)
{
	UINT16 adr = FETCH16();
	UINT8 i = READ8(adr);
	UINT16 r = REG_A + i;
	CLEAR_HNZVC();
	SET_H(r, i, REG_A);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(4);
}

/* ADDA IND, X      0xAB */
static void HC11OP(adda_indx)(void)
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.ix + offset);
	UINT16 r = REG_A + i;
	CLEAR_HNZVC();
	SET_H(r, i, REG_A);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(4);
}

/* ADDA IND, Y      0x18, 0xAB */
static void HC11OP(adda_indy)(void)
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.iy + offset);
	UINT16 r = REG_A + i;
	CLEAR_HNZVC();
	SET_H(r, i, REG_A);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(5);
}


/* ADDB IMM         0xCB */
static void HC11OP(addb_imm)(void)
{
	UINT8 i = FETCH();
	UINT16 r = REG_B + i;
	CLEAR_HNZVC();
	SET_H(r, i, REG_B);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(2);
}

/* ADDB DIR         0xDB */
static void HC11OP(addb_dir)(void)
{
	UINT8 d = FETCH();
	UINT8 i = READ8(d);
	UINT16 r = REG_B + i;
	CLEAR_HNZVC();
	SET_H(r, i, REG_B);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(3);
}

/* ADDB EXT         0xFB */
static void HC11OP(addb_ext)(void)
{
	UINT16 adr = FETCH16();
	UINT8 i = READ8(adr);
	UINT16 r = REG_B + i;
	CLEAR_HNZVC();
	SET_H(r, i, REG_B);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(4);
}

/* ADDB IND, X      0xEB */
static void HC11OP(addb_indx)(void)
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.ix + offset);
	UINT16 r = REG_B + i;
	CLEAR_HNZVC();
	SET_H(r, i, REG_B);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(4);
}

/* ADDB IND, Y      0x18, 0xEB */
static void HC11OP(addb_indy)(void)
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.iy + offset);
	UINT16 r = REG_B + i;
	CLEAR_HNZVC();
	SET_H(r, i, REG_B);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(5);
}


/* ADDD IMM         0xC3 */
static void HC11OP(addd_imm)(void)
{
	UINT16 i = FETCH16();
	UINT32 r = REG_D + i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_ADD16(r, i, REG_D);
	SET_C16(r);
	REG_D = (UINT16)r;
	CYCLES(4);
}

/* ADDD DIR         0xD3 */
static void HC11OP(addd_dir)(void)
{
	UINT8 d = FETCH();
	UINT16 i = READ16(d);
	UINT32 r = REG_D + i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_ADD16(r, i, REG_D);
	SET_C16(r);
	REG_D = (UINT16)r;
	CYCLES(5);
}

/* ADDD EXT         0xF3 */
static void HC11OP(addd_ext)(void)
{
	UINT16 adr = FETCH16();
	UINT16 i = READ16(adr);
	UINT32 r = REG_D + i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_ADD16(r, i, REG_D);
	SET_C16(r);
	REG_D = (UINT16)r;
	CYCLES(6);
}

/* ADDD IND, X      0xE3 */
static void HC11OP(addd_indx)(void)
{
	UINT8 offset = FETCH();
	UINT16 i = READ16(hc11.ix + offset);
	UINT32 r = REG_D + i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_ADD16(r, i, REG_D);
	SET_C16(r);
	REG_D = (UINT16)r;
	CYCLES(6);
}

/* ADDD IND, Y      0x18, 0xE3 */
static void HC11OP(addd_indy)(void)
{
	UINT8 offset = FETCH();
	UINT16 i = READ16(hc11.iy + offset);
	UINT32 r = REG_D + i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_ADD16(r, i, REG_D);
	SET_C16(r);
	REG_D = (UINT16)r;
	CYCLES(7);
}


/* ANDA IMM         0x84 */
static void HC11OP(anda_imm)(void)
{
	UINT8 i = FETCH();
	CLEAR_NZV();
	REG_A &= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(2);
}

/* ANDA DIR         0x94 */
static void HC11OP(anda_dir)(void)
{
	UINT8 d = FETCH();
	UINT8 i = READ8(d);
	CLEAR_NZV();
	REG_A &= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(3);
}

/* ANDA EXT         0xB4 */
static void HC11OP(anda_ext)(void)
{
	UINT16 adr = FETCH16();
	UINT8 i = READ8(adr);
	CLEAR_NZV();
	REG_A &= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(4);
}

/* ANDA IND, X      0xA4 */
static void HC11OP(anda_indx)(void)
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.ix + offset);
	CLEAR_NZV();
	REG_A &= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(4);
}

/* ANDA IND, Y      0x18, 0xA4 */
static void HC11OP(anda_indy)(void)
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.iy + offset);
	CLEAR_NZV();
	REG_A &= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(5);
}


/* ANDB IMM         0xC4 */
static void HC11OP(andb_imm)(void)
{
	UINT8 i = FETCH();
	CLEAR_NZV();
	REG_B &= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(2);
}

/* ANDB DIR         0xD4 */
static void HC11OP(andb_dir)(void)
{
	UINT8 d = FETCH();
	UINT8 i = READ8(d);
	CLEAR_NZV();
	REG_B &= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(3);
}

/* ANDB EXT         0xF4 */
static void HC11OP(andb_ext)(void)
{
	UINT16 adr = FETCH16();
	UINT8 i = READ8(adr);
	CLEAR_NZV();
	REG_B &= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(4);
}

/* ANDB IND, X      0xE4 */
static void HC11OP(andb_indx)(void)
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.ix + offset);
	CLEAR_NZV();
	REG_B &= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(4);
}

/* ANDB IND, Y      0x18, 0xE4 */
static void HC11OP(andb_indy)(void)
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.iy + offset);
	CLEAR_NZV();
	REG_B &= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(5);
}


/* BITA IMM         0x85 */
static void HC11OP(bita_imm)(void)
{
	UINT8 i = FETCH();
	UINT8 r = REG_A & i;
	CLEAR_NZV();
	SET_N8(r);
	SET_Z8(r);
	CYCLES(2);
}

/* BITA DIR         0x95 */
static void HC11OP(bita_dir)(void)
{
	UINT8 d = FETCH();
	UINT8 i = READ8(d);
	UINT8 r = REG_A & i;
	CLEAR_NZV();
	SET_N8(r);
	SET_Z8(r);
	CYCLES(3);
}

/* BITA EXT         0xB5 */
static void HC11OP(bita_ext)(void)
{
	UINT16 adr = FETCH16();
	UINT8 i = READ8(adr);
	UINT8 r = REG_A & i;
	CLEAR_NZV();
	SET_N8(r);
	SET_Z8(r);
	CYCLES(4);
}

/* BITA IND, X      0xA5 */
static void HC11OP(bita_indx)(void)
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.ix + offset);
	UINT8 r = REG_A & i;
	CLEAR_NZV();
	SET_N8(r);
	SET_Z8(r);
	CYCLES(4);
}

/* BITA IND, Y      0x18, 0xA5 */
static void HC11OP(bita_indy)(void)
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.iy + offset);
	UINT8 r = REG_A & i;
	CLEAR_NZV();
	SET_N8(r);
	SET_Z8(r);
	CYCLES(5);
}


/* BITB IMM         0xC5 */
static void HC11OP(bitb_imm)(void)
{
	UINT8 i = FETCH();
	UINT8 r = REG_B & i;
	CLEAR_NZV();
	SET_N8(r);
	SET_Z8(r);
	CYCLES(2);
}

/* BITB DIR         0xD5 */
static void HC11OP(bitb_dir)(void)
{
	UINT8 d = FETCH();
	UINT8 i = READ8(d);
	UINT8 r = REG_B & i;
	CLEAR_NZV();
	SET_N8(r);
	SET_Z8(r);
	CYCLES(3);
}

/* BITB EXT         0xF5 */
static void HC11OP(bitb_ext)(void)
{
	UINT16 adr = FETCH16();
	UINT8 i = READ8(adr);
	UINT8 r = REG_B & i;
	CLEAR_NZV();
	SET_N8(r);
	SET_Z8(r);
	CYCLES(4);
}

/* BITB IND, X      0xE5 */
static void HC11OP(bitb_indx)(void)
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.ix + offset);
	UINT8 r = REG_B & i;
	CLEAR_NZV();
	SET_N8(r);
	SET_Z8(r);
	CYCLES(4);
}

/* BITB IND, Y      0x18, 0xE5 */
static void HC11OP(bitb_indy)(void)
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.iy + offset);
	UINT8 r = REG_B & i;
	CLEAR_NZV();
	SET_N8(r);
	SET_Z8(r);
	CYCLES(5);
}


/* BCS              0x25 */
static void HC11OP(bcs)(void)
{
	INT8 rel = FETCH();
	if (hc11.ccr & CC_C)			/* Branch if C flag set */
	{
		SET_PC(hc11.ppc + rel + 2);
	}
	CYCLES(3);
}


/* BEQ              0x27 */
static void HC11OP(beq)(void)
{
	INT8 rel = FETCH();
	if (hc11.ccr & CC_Z)			/* Branch if Z flag set */
	{
		SET_PC(hc11.ppc + rel + 2);
	}
	CYCLES(3);
}


/* BNE              0x26 */
static void HC11OP(bne)(void)
{
	INT8 rel = FETCH();
	if ((hc11.ccr & CC_Z) == 0)		/* Branch if Z flag clear */
	{
		SET_PC(hc11.ppc + rel + 2);
	}
	CYCLES(3);
}


/* BLE              0x2F */
static void HC11OP(ble)(void)
{
	UINT8 n = (hc11.ccr & CC_N) ? 1 : 0;
	UINT8 v = (hc11.ccr & CC_V) ? 1 : 0;
	INT8 rel = FETCH();
	if ((hc11.ccr & CC_Z) || (n ^ v))	/* Branch if Z flag set or (N ^ V) */
	{
		SET_PC(hc11.ppc + rel + 2);
	}
	CYCLES(3);
}


/* BPL              0x2A */
static void HC11OP(bpl)(void)
{
	INT8 rel = FETCH();
	if ((hc11.ccr & CC_N) == 0)		/* Branch if N flag clear */
	{
		SET_PC(hc11.ppc + rel + 2);
	}
	CYCLES(3);
}


/* BRA              0x20 */
static void HC11OP(bra)(void)
{
	INT8 rel = FETCH();
	SET_PC(hc11.ppc + rel + 2);
	CYCLES(3);
}


/* BSR              0x8D */
static void HC11OP(bsr)(void)
{
	INT8 rel = FETCH();
	UINT16 rt_adr = hc11.pc;
	PUSH16(rt_adr);
	SET_PC(hc11.ppc + rel + 2);
	CYCLES(6);
}


/* CLI              0x0E */
static void HC11OP(cli)(void)
{
	hc11.ccr &= ~CC_I;
	CYCLES(2);
}


/* CLRA             0x4F */
static void HC11OP(clra)(void)
{
	REG_A = 0;
	CLEAR_NZVC();
	SET_ZFLAG();
	CYCLES(2);
}

/* CLRB             0x5F */
static void HC11OP(clrb)(void)
{
	REG_B = 0;
	CLEAR_NZVC();
	SET_ZFLAG();
	CYCLES(2);
}

/* CLR EXT          0x7F */
static void HC11OP(clr_ext)(void)
{
	UINT16 adr = FETCH16();
	WRITE8(adr, 0);
	CLEAR_NZVC();
	SET_ZFLAG();
	CYCLES(6);
}

/* CLR IND, X       0x6F */
static void HC11OP(clr_indx)(void)
{
	UINT8 offset = FETCH();
	WRITE8(hc11.ix + offset, 0);
	CLEAR_NZVC();
	SET_ZFLAG();
	CYCLES(6);
}

/* CLR IND, Y       0x18, 0x6F */
static void HC11OP(clr_indy)(void)
{
	UINT8 offset = FETCH();
	WRITE8(hc11.iy + offset, 0);
	CLEAR_NZVC();
	SET_ZFLAG();
	CYCLES(7);
}


/* CMPA IMM         0x81 */
static void HC11OP(cmpa_imm)(void)
{
	UINT8 i = FETCH();
	UINT16 r = REG_A - i;
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_A);
	SET_C8(r);
	CYCLES(2);
}

/* CMPA DIR         0x91 */
static void HC11OP(cmpa_dir)(void)
{
	UINT8 d = FETCH();
	UINT8 i = READ8(d);
	UINT16 r = REG_A - i;
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_A);
	SET_C8(r);
	CYCLES(3);
}

/* CMPA EXT         0xB1 */
static void HC11OP(cmpa_ext)(void)
{
	UINT16 adr = FETCH16();
	UINT8 i = READ8(adr);
	UINT16 r = REG_A - i;
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_A);
	SET_C8(r);
	CYCLES(4);
}

/* CMPA IND, X      0xA1 */
static void HC11OP(cmpa_indx)(void)
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.ix + offset);
	UINT16 r = REG_A - i;
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_A);
	SET_C8(r);
	CYCLES(4);
}

/* CMPA IND, Y      0x18, 0xA1 */
static void HC11OP(cmpa_indy)(void)
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.iy + offset);
	UINT16 r = REG_A - i;
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_A);
	SET_C8(r);
	CYCLES(5);
}


/* CMPB IMM         0xC1 */
static void HC11OP(cmpb_imm)(void)
{
	UINT8 i = FETCH();
	UINT16 r = REG_B - i;
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_B);
	SET_C8(r);
	CYCLES(2);
}

/* CMPB DIR         0xD1 */
static void HC11OP(cmpb_dir)(void)
{
	UINT8 d = FETCH();
	UINT8 i = READ8(d);
	UINT16 r = REG_B - i;
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_B);
	SET_C8(r);
	CYCLES(3);
}

/* CMPB EXT         0xF1 */
static void HC11OP(cmpb_ext)(void)
{
	UINT16 adr = FETCH16();
	UINT8 i = READ8(adr);
	UINT16 r = REG_B - i;
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_B);
	SET_C8(r);
	CYCLES(4);
}

/* CMPB IND, X      0xE1 */
static void HC11OP(cmpb_indx)(void)
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.ix + offset);
	UINT16 r = REG_B - i;
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_B);
	SET_C8(r);
	CYCLES(4);
}

/* CMPB IND, Y      0x18, 0xE1 */
static void HC11OP(cmpb_indy)(void)
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.iy + offset);
	UINT16 r = REG_B - i;
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_B);
	SET_C8(r);
	CYCLES(5);
}


/* CPD IMM          0x1A, 0x83 */
static void HC11OP(cpd_imm)(void)
{
	UINT16 i = FETCH16();
	UINT32 r = REG_D - i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, REG_D);
	SET_C16(r);
	CYCLES(5);
}

/* CPD DIR          0x1A, 0x93 */
static void HC11OP(cpd_dir)(void)
{
	UINT8 d = FETCH();
	UINT16 i = READ16(d);
	UINT32 r = REG_D - i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, REG_D);
	SET_C16(r);
	CYCLES(6);
}

/* CPD EXT          0x1A, 0xB3 */
static void HC11OP(cpd_ext)(void)
{
	UINT16 adr = FETCH16();
	UINT16 i = READ16(adr);
	UINT32 r = REG_D - i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, REG_D);
	SET_C16(r);
	CYCLES(7);
}

/* CPD IND, X       0x1A, 0xA3 */
static void HC11OP(cpd_indx)(void)
{
	UINT8 offset = FETCH();
	UINT16 i = READ16(hc11.ix + offset);
	UINT32 r = REG_D - i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, REG_D);
	SET_C16(r);
	CYCLES(7);
}

/* CPD IND, Y       0xCD, 0xA3 */
static void HC11OP(cpd_indy)(void)
{
	UINT8 offset = FETCH();
	UINT16 i = READ16(hc11.iy + offset);
	UINT32 r = REG_D - i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, REG_D);
	SET_C16(r);
	CYCLES(7);
}


/* CPX IMM          0x8C */
static void HC11OP(cpx_imm)(void)
{
	UINT16 i = FETCH16();
	UINT32 r = hc11.ix - i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, hc11.ix);
	SET_C16(r);
	CYCLES(4);
}

/* CPX DIR          0x9C */
static void HC11OP(cpx_dir)(void)
{
	UINT8 d = FETCH();
	UINT16 i = READ16(d);
	UINT32 r = hc11.ix - i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, hc11.ix);
	SET_C16(r);
	CYCLES(5);
}

/* CPX EXT          0xBC */
static void HC11OP(cpx_ext)(void)
{
	UINT16 adr = FETCH16();
	UINT16 i = READ16(adr);
	UINT32 r = hc11.ix - i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, hc11.ix);
	SET_C16(r);
	CYCLES(6);
}

/* CPX IND, X       0xAC */
static void HC11OP(cpx_indx)(void)
{
	UINT8 offset = FETCH();
	UINT16 i = READ16(hc11.ix + offset);
	UINT32 r = hc11.ix - i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, hc11.ix);
	SET_C16(r);
	CYCLES(6);
}

/* CPX IND, Y       0xCD, 0xAC */
static void HC11OP(cpx_indy)(void)
{
	UINT8 offset = FETCH();
	UINT16 i = READ16(hc11.iy + offset);
	UINT32 r = hc11.ix - i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, hc11.ix);
	SET_C16(r);
	CYCLES(7);
}


/* CPY IMM          0x18, 0x8C */
static void HC11OP(cpy_imm)(void)
{
	UINT16 i = FETCH16();
	UINT32 r = hc11.iy - i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, hc11.iy);
	SET_C16(r);
	CYCLES(5);
}


/* DEX              0x09 */
static void HC11OP(dex)(void)
{
	CLEAR_Z();
	hc11.ix--;
	SET_Z16(hc11.ix);
	CYCLES(3);
}


/* DEY              0x18, 0x09 */
static void HC11OP(dey)(void)
{
	CLEAR_Z();
	hc11.iy--;
	SET_Z16(hc11.iy);
	CYCLES(4);
}


/* EORA IMM         0x88 */
static void HC11OP(eora_imm)(void)
{
	UINT8 i = FETCH();
	CLEAR_NZV();
	REG_A ^= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(2);
}

/* EORA DIR         0x98 */
static void HC11OP(eora_dir)(void)
{
	UINT8 d = FETCH();
	UINT8 i = READ8(d);
	CLEAR_NZV();
	REG_A ^= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(3);
}

/* EORA EXT         0xB8 */
static void HC11OP(eora_ext)(void)
{
	UINT16 adr = FETCH16();
	UINT8 i = READ8(adr);
	CLEAR_NZV();
	REG_A ^= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(4);
}

/* EORA IND, X      0xA8 */
static void HC11OP(eora_indx)(void)
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.ix + offset);
	CLEAR_NZV();
	REG_A ^= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(4);
}

/* EORA IND, Y      0x18, 0xA8 */
static void HC11OP(eora_indy)(void)
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.iy + offset);
	CLEAR_NZV();
	REG_A ^= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(5);
}


/* EORB IMM         0xC8 */
static void HC11OP(eorb_imm)(void)
{
	UINT8 i = FETCH();
	CLEAR_NZV();
	REG_B ^= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(2);
}

/* EORB DIR         0xD8 */
static void HC11OP(eorb_dir)(void)
{
	UINT8 d = FETCH();
	UINT8 i = READ8(d);
	CLEAR_NZV();
	REG_B ^= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(3);
}

/* EORB EXT         0xF8 */
static void HC11OP(eorb_ext)(void)
{
	UINT16 adr = FETCH16();
	UINT8 i = READ8(adr);
	CLEAR_NZV();
	REG_B ^= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(4);
}

/* EORB IND, X      0xE8 */
static void HC11OP(eorb_indx)(void)
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.ix + offset);
	CLEAR_NZV();
	REG_B ^= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(4);
}

/* EORB IND, Y      0x18, 0xE8 */
static void HC11OP(eorb_indy)(void)
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.iy + offset);
	CLEAR_NZV();
	REG_B ^= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(5);
}


/* INCA             0x4C */
static void HC11OP(inca)(void)
{
	CLEAR_NZV();
	if (REG_A == 0x7f)
		SET_VFLAG();
	REG_A++;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(2);
}


/* INX              0x08 */
static void HC11OP(inx)(void)
{
	CLEAR_Z();
	hc11.ix++;
	SET_Z16(hc11.ix);
	CYCLES(3);
}

/* INY              0x18, 0x08 */
static void HC11OP(iny)(void)
{
	CLEAR_Z();
	hc11.iy++;
	SET_Z16(hc11.iy);
	CYCLES(4);
}


/* JMP EXT          0x7E */
static void HC11OP(jmp_ext)(void)
{
	UINT16 adr = FETCH16();
	SET_PC(adr);
	CYCLES(3);
}


/* JSR DIR          0x9D */
static void HC11OP(jsr_dir)(void)
{
	UINT8 i = FETCH();
	UINT16 rt_adr = hc11.pc;
	PUSH16(rt_adr);
	SET_PC(i);
	CYCLES(5);
}

/* JSR EXT          0xBD */
static void HC11OP(jsr_ext)(void)
{
	UINT16 adr = FETCH16();
	UINT16 rt_adr = hc11.pc;
	PUSH16(rt_adr);
	SET_PC(adr);
	CYCLES(6);
}

/* JSR IND, X       0xAD */
static void HC11OP(jsr_indx)(void)
{
	UINT8 offset = FETCH();
	UINT16 rt_adr = hc11.pc;
	PUSH16(rt_adr);
	SET_PC(hc11.ix + offset);
	CYCLES(6);
}

/* JSR IND, Y       0x18, 0xAD */
static void HC11OP(jsr_indy)(void)
{
	UINT8 offset = FETCH();
	UINT16 rt_adr = hc11.pc;
	PUSH16(rt_adr);
	SET_PC(hc11.iy + offset);
	CYCLES(6);
}


/* LDAA IMM         0x86 */
static void HC11OP(ldaa_imm)(void)
{
	CLEAR_NZV();
	REG_A = FETCH();
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(2);
}

/* LDAA DIR         0x96 */
static void HC11OP(ldaa_dir)(void)
{
	UINT8 d = FETCH();
	CLEAR_NZV();
	REG_A = READ8(d);
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(3);
}

/* LDAA EXT         0xB6 */
static void HC11OP(ldaa_ext)(void)
{
	UINT16 adr = FETCH16();
	CLEAR_NZV();
	REG_A = READ8(adr);
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(4);
}

/* LDAA IND, X      0xA6 */
static void HC11OP(ldaa_indx)(void)
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	REG_A = READ8(hc11.ix + offset);
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(4);
}

/* LDAA IND, Y      0x18, 0xA6 */
static void HC11OP(ldaa_indy)(void)
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	REG_A = READ8(hc11.iy + offset);
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(5);
}

/* LDAB IMM         0xC6 */
static void HC11OP(ldab_imm)(void)
{
	CLEAR_NZV();
	REG_B = FETCH();
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(2);
}

/* LDAB DIR         0xD6 */
static void HC11OP(ldab_dir)(void)
{
	UINT8 d = FETCH();
	CLEAR_NZV();
	REG_B = READ8(d);
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(3);
}

/* LDAB EXT         0xF6 */
static void HC11OP(ldab_ext)(void)
{
	UINT16 adr = FETCH16();
	CLEAR_NZV();
	REG_B = READ8(adr);
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(4);
}

/* LDAB IND, X      0xE6 */
static void HC11OP(ldab_indx)(void)
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	REG_B = READ8(hc11.ix + offset);
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(4);
}

/* LDAB IND, Y      0x18, 0xE6 */
static void HC11OP(ldab_indy)(void)
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	REG_B = READ8(hc11.iy + offset);
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(5);
}


/* LDD IMM          0xCC */
static void HC11OP(ldd_imm)(void)
{
	CLEAR_NZV();
	REG_D = FETCH16();
	SET_N16(REG_D);
	SET_Z16(REG_D);
	CYCLES(3);
}

/* LDD DIR          0xDC */
static void HC11OP(ldd_dir)(void)
{
	UINT8 d = FETCH();
	CLEAR_NZV();
	REG_D = READ16(d);
	SET_N16(REG_D);
	SET_Z16(REG_D);
	CYCLES(4);
}

/* LDD EXT          0xFC */
static void HC11OP(ldd_ext)(void)
{
	UINT16 adr = FETCH16();
	CLEAR_NZV();
	REG_D = READ16(adr);
	SET_N16(REG_D);
	SET_Z16(REG_D);
	CYCLES(5);
}

/* LDD IND, X       0xEC */
static void HC11OP(ldd_indx)(void)
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	REG_D = READ16(hc11.ix + offset);
	SET_N16(REG_D);
	SET_Z16(REG_D);
	CYCLES(5);
}

/* LDD IND, Y       0x18, 0xEC */
static void HC11OP(ldd_indy)(void)
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	REG_D = READ16(hc11.iy + offset);
	SET_N16(REG_D);
	SET_Z16(REG_D);
	CYCLES(6);
}


/* LDS IMM          0x8E */
static void HC11OP(lds_imm)(void)
{
	CLEAR_NZV();
	hc11.sp = FETCH16();
	SET_N16(hc11.sp);
	SET_Z16(hc11.sp);
	CYCLES(3);
}

/* LDS DIR          0x9E */
static void HC11OP(lds_dir)(void)
{
	UINT8 i = FETCH();
	CLEAR_NZV();
	hc11.sp = READ16(i);
	SET_N16(hc11.sp);
	SET_Z16(hc11.sp);
	CYCLES(4);
}

/* LDS EXT          0xBE */
static void HC11OP(lds_ext)(void)
{
	UINT16 adr = FETCH16();
	CLEAR_NZV();
	hc11.sp = READ16(adr);
	SET_N16(hc11.sp);
	SET_Z16(hc11.sp);
	CYCLES(5);
}

/* LDS IND, X       0xAE */
static void HC11OP(lds_indx)(void)
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	hc11.sp = READ16(hc11.ix + offset);
	SET_N16(hc11.sp);
	SET_Z16(hc11.sp);
	CYCLES(5);
}

/* LDS IND, Y       0x18, 0xAE */
static void HC11OP(lds_indy)(void)
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	hc11.sp = READ16(hc11.iy + offset);
	SET_N16(hc11.sp);
	SET_Z16(hc11.sp);
	CYCLES(6);
}


/* LDX IMM          0xCE */
static void HC11OP(ldx_imm)(void)
{
	CLEAR_NZV();
	hc11.ix = FETCH16();
	SET_N16(hc11.ix);
	SET_Z16(hc11.ix);
	CYCLES(3);
}

/* LDX DIR          0xDE */
static void HC11OP(ldx_dir)(void)
{
	UINT8 d = FETCH();
	CLEAR_NZV();
	hc11.ix = READ16(d);
	SET_N16(hc11.ix);
	SET_Z16(hc11.ix);
	CYCLES(4);
}

/* LDX EXT          0xFE */
static void HC11OP(ldx_ext)(void)
{
	UINT16 adr = FETCH16();
	CLEAR_NZV();
	hc11.ix = READ16(adr);
	SET_N16(hc11.ix);
	SET_Z16(hc11.ix);
	CYCLES(5);
}

/* LDX IND, X       0xEE */
static void HC11OP(ldx_indx)(void)
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	hc11.ix = READ16(hc11.ix + offset);
	SET_N16(hc11.ix);
	SET_Z16(hc11.ix);
	CYCLES(5);
}

/* LDX IND, Y       0xCD, 0xEE */
static void HC11OP(ldx_indy)(void)
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	hc11.ix = READ16(hc11.iy + offset);
	SET_N16(hc11.ix);
	SET_Z16(hc11.ix);
	CYCLES(6);
}


/* LDY IMM          0x18, 0xCE */
static void HC11OP(ldy_imm)(void)
{
	CLEAR_NZV();
	hc11.iy = FETCH16();
	SET_N16(hc11.iy);
	SET_Z16(hc11.iy);
	CYCLES(4);
}

/* LDY DIR          0x18, 0xDE */
static void HC11OP(ldy_dir)(void)
{
	UINT8 d = FETCH();
	CLEAR_NZV();
	hc11.iy = READ16(d);
	SET_N16(hc11.iy);
	SET_Z16(hc11.iy);
	CYCLES(5);
}

/* LDY EXT          0x18, 0xFE */
static void HC11OP(ldy_ext)(void)
{
	UINT16 adr = FETCH16();
	CLEAR_NZV();
	hc11.iy = READ16(adr);
	SET_N16(hc11.iy);
	SET_Z16(hc11.iy);
	CYCLES(6);
}

/* LDY IND, X       0x1A, 0xEE */
static void HC11OP(ldy_indx)(void)
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	hc11.iy = READ16(hc11.ix + offset);
	SET_N16(hc11.iy);
	SET_Z16(hc11.iy);
	CYCLES(6);
}

/* LDY IND, Y       0x18, 0xEE */
static void HC11OP(ldy_indy)(void)
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	hc11.iy = READ16(hc11.iy + offset);
	SET_N16(hc11.iy);
	SET_Z16(hc11.iy);
	CYCLES(6);
}


/* LSLD             0x05 */
static void HC11OP(lsld)(void)
{
	UINT32 r = REG_D << 1;
	CLEAR_NZVC();
	SET_C16(r);
	REG_D = (UINT16)(r);
	SET_N16(REG_D);
	SET_Z16(REG_D);

	if (((hc11.ccr & CC_N) == CC_N && (hc11.ccr & CC_C) == 0) ||
		((hc11.ccr & CC_N) == 0 && (hc11.ccr & CC_C) == CC_C))
	{
		hc11.ccr |= CC_V;
	}

	CYCLES(3);
}


/* PSHA             0x36 */
static void HC11OP(psha)(void)
{
	PUSH8(REG_A);
	CYCLES(3);
}


/* ORAA IMM         0x8A */
static void HC11OP(oraa_imm)(void)
{
	UINT8 i = FETCH();
	CLEAR_NZV();
	REG_A |= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(2);
}

/* ORAA DIR         0x9A */
static void HC11OP(oraa_dir)(void)
{
	UINT8 d = FETCH();
	UINT8 i = READ8(d);
	CLEAR_NZV();
	REG_A |= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(3);
}

/* ORAA EXT         0xBA */
static void HC11OP(oraa_ext)(void)
{
	UINT16 adr = FETCH16();
	UINT8 i = READ8(adr);
	CLEAR_NZV();
	REG_A |= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(4);
}

/* ORAA IND, X      0xAA */
static void HC11OP(oraa_indx)(void)
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.ix + offset);
	CLEAR_NZV();
	REG_A |= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(4);
}

/* ORAA IND, Y      0x18, 0xAA */
static void HC11OP(oraa_indy)(void)
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.iy + offset);
	CLEAR_NZV();
	REG_A |= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(5);
}


/* ORAB IMM         0xCA */
static void HC11OP(orab_imm)(void)
{
	UINT8 i = FETCH();
	CLEAR_NZV();
	REG_B |= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(2);
}

/* ORAB DIR         0xDA */
static void HC11OP(orab_dir)(void)
{
	UINT8 d = FETCH();
	UINT8 i = READ8(d);
	CLEAR_NZV();
	REG_B |= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(3);
}

/* ORAB EXT         0xFA */
static void HC11OP(orab_ext)(void)
{
	UINT16 adr = FETCH16();
	UINT8 i = READ8(adr);
	CLEAR_NZV();
	REG_B |= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(4);
}

/* ORAB IND, X      0xEA */
static void HC11OP(orab_indx)(void)
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.ix + offset);
	CLEAR_NZV();
	REG_B |= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(4);
}

/* ORAB IND, Y      0x18, 0xEA */
static void HC11OP(orab_indy)(void)
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.iy + offset);
	CLEAR_NZV();
	REG_B |= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(5);
}


/* PSHB             0x37 */
static void HC11OP(pshb)(void)
{
	PUSH8(REG_B);
	CYCLES(3);
}


/* PSHX             0x3C */
static void HC11OP(pshx)(void)
{
	PUSH16(hc11.ix);
	CYCLES(4);
}


/* PSHY             0x18, 0x3C */
static void HC11OP(pshy)(void)
{
	PUSH16(hc11.iy);
	CYCLES(5);
}


/* PULA             0x32 */
static void HC11OP(pula)(void)
{
	REG_A = POP8();
	CYCLES(4);
}


/* PULB             0x33 */
static void HC11OP(pulb)(void)
{
	REG_B = POP8();
	CYCLES(4);
}


/* PULX             0x38 */
static void HC11OP(pulx)(void)
{
	hc11.ix = POP16();
	CYCLES(5);
}


/* PULY             0x18, 0x38 */
static void HC11OP(puly)(void)
{
	hc11.iy = POP16();
	CYCLES(6);
}


/* RTS              0x39 */
static void HC11OP(rts)(void)
{
	UINT16 rt_adr = POP16();
	SET_PC(rt_adr);
	CYCLES(5);
}


/* SEI              0x0F */
static void HC11OP(sei)(void)
{
	hc11.ccr |= CC_I;
	CYCLES(2);
}


/* STAA DIR         0x97 */
static void HC11OP(staa_dir)(void)
{
	UINT8 d = FETCH();
	CLEAR_NZV();
	SET_N8(REG_A);
	SET_Z8(REG_A);
	WRITE8(d, REG_A);
	CYCLES(3);
}

/* STAA EXT         0xB7 */
static void HC11OP(staa_ext)(void)
{
	UINT16 adr = FETCH16();
	CLEAR_NZV();
	SET_N8(REG_A);
	SET_Z8(REG_A);
	WRITE8(adr, REG_A);
	CYCLES(4);
}

/* STAA IND, X      0xA7 */
static void HC11OP(staa_indx)(void)
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	SET_N8(REG_A);
	SET_Z8(REG_A);
	WRITE8(hc11.ix + offset, REG_A);
	CYCLES(4);
}

/* STAA IND, Y      0x18, 0xA7 */
static void HC11OP(staa_indy)(void)
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	SET_N8(REG_A);
	SET_Z8(REG_A);
	WRITE8(hc11.iy + offset, REG_A);
	CYCLES(5);
}

/* STAB DIR         0xD7 */
static void HC11OP(stab_dir)(void)
{
	UINT8 d = FETCH();
	CLEAR_NZV();
	SET_N8(REG_B);
	SET_Z8(REG_B);
	WRITE8(d, REG_B);
	CYCLES(3);
}

/* STAB EXT         0xF7 */
static void HC11OP(stab_ext)(void)
{
	UINT16 adr = FETCH16();
	CLEAR_NZV();
	SET_N8(REG_B);
	SET_Z8(REG_B);
	WRITE8(adr, REG_B);
	CYCLES(4);
}

/* STAB IND, X      0xE7 */
static void HC11OP(stab_indx)(void)
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	SET_N8(REG_B);
	SET_Z8(REG_B);
	WRITE8(hc11.ix + offset, REG_B);
	CYCLES(4);
}

/* STAB IND, Y      0x18, 0xE7 */
static void HC11OP(stab_indy)(void)
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	SET_N8(REG_B);
	SET_Z8(REG_B);
	WRITE8(hc11.iy + offset, REG_B);
	CYCLES(5);
}


/* STD DIR          0xDD */
static void HC11OP(std_dir)(void)
{
	UINT8 d = FETCH();
	CLEAR_NZV();
	WRITE16(d, REG_D);
	SET_N16(REG_D);
	SET_Z16(REG_D);
	CYCLES(4);
}

/* STD EXT          0xFD */
static void HC11OP(std_ext)(void)
{
	UINT16 adr = FETCH16();
	CLEAR_NZV();
	WRITE16(adr, REG_D);
	SET_N16(REG_D);
	SET_Z16(REG_D);
	CYCLES(5);
}

/* STD IND, X       0xED */
static void HC11OP(std_indx)(void)
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	WRITE16(hc11.ix + offset, REG_D);
	SET_N16(REG_D);
	SET_Z16(REG_D);
	CYCLES(5);
}

/* STD IND, Y       0x18, 0xED */
static void HC11OP(std_indy)(void)
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	WRITE16(hc11.iy + offset, REG_D);
	SET_N16(REG_D);
	SET_Z16(REG_D);
	CYCLES(6);
}


/* TAB              0x16 */
static void HC11OP(tab)(void)
{
	CLEAR_NZV();
	REG_B = REG_A;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(2);
}


/* TAP              0x06 */
static void HC11OP(tap)(void)
{
	UINT8 ccr = hc11.ccr;
	hc11.ccr = (REG_A & 0xbf) | (ccr & 0x40);
	CYCLES(2);
}


/* TPA              0x07 */
static void HC11OP(tpa)(void)
{
	REG_A = hc11.ccr;
	CYCLES(2);
}


/* TSTA             0x4D */
static void HC11OP(tsta)(void)
{
	CLEAR_NZVC();
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(2);
}

/* TSTB             0x5D */
static void HC11OP(tstb)(void)
{
	CLEAR_NZVC();
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(2);
}

/* TST EXT          0x7D */
static void HC11OP(tst_ext)(void)
{
	UINT16 adr = FETCH16();
	UINT8 i = READ8(adr);
	CLEAR_NZVC();
	SET_N8(i);
	SET_Z8(i);
	CYCLES(6);
}

/* TST IND, X       0x6D */
static void HC11OP(tst_indx)(void)
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.ix + offset);
	CLEAR_NZVC();
	SET_N8(i);
	SET_Z8(i);
	CYCLES(6);
}

/* TST IND, Y       0x18, 0x6D */
static void HC11OP(tst_indy)(void)
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(hc11.iy + offset);
	CLEAR_NZVC();
	SET_N8(i);
	SET_Z8(i);
	CYCLES(6);
}


/* XGDX             0x8F */
static void HC11OP(xgdx)(void)
{
	UINT16 tmp = REG_D;
	REG_D = hc11.ix;
	hc11.ix = tmp;
	CYCLES(3);
}


/* XGDY             0x18, 0x8F */
static void HC11OP(xgdy)(void)
{
	UINT16 tmp = REG_D;
	REG_D = hc11.iy;
	hc11.iy = tmp;
	CYCLES(4);
}

/*****************************************************************************/

static void HC11OP(page2)(void)
{
	UINT8 op2 = FETCH();
	hc11_optable_page2[op2]();
}

static void HC11OP(page3)(void)
{
	UINT8 op2 = FETCH();
	hc11_optable_page3[op2]();
}

static void HC11OP(page4)(void)
{
	UINT8 op2 = FETCH();
	hc11_optable_page4[op2]();
}

static void HC11OP(invalid)(void)
{
	fatalerror("HC11: Invalid opcode 0x%02X at %04X", READ8(hc11.pc-1), hc11.pc-1);
}
