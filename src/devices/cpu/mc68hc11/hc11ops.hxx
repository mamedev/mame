// license:BSD-3-Clause
// copyright-holders:Ville Linde, Angelo Salese, hap

#define SET_Z8(r)           (m_ccr |= ((UINT8)r == 0) ? CC_Z : 0)
#define SET_Z16(r)          (m_ccr |= ((UINT16)r == 0) ? CC_Z : 0)
#define SET_N8(r)           (m_ccr |= (r & 0x80) ? CC_N : 0)
#define SET_N16(r)          (m_ccr |= (r & 0x8000) ? CC_N : 0)
#define SET_V_ADD8(r,s,d)   (m_ccr |= (((r) ^ (s)) & ((r) ^ (d)) & 0x80) ? CC_V : 0)
#define SET_V_SUB8(r,s,d)   (m_ccr |= (((d) ^ (s)) & ((d) ^ (r)) & 0x80) ? CC_V : 0)
#define SET_V_ADD16(r,s,d)  (m_ccr |= (((r) ^ (s)) & ((r) ^ (d)) & 0x8000) ? CC_V : 0)
#define SET_V_SUB16(r,s,d)  (m_ccr |= (((d) ^ (s)) & ((d) ^ (r)) & 0x8000) ? CC_V : 0)
#define SET_H(r,s,d)        (m_ccr |= (((r) ^ (s) ^ (d)) & 0x10) ? CC_H : 0)
#define SET_C8(x)           (m_ccr |= ((x) & 0x100) ? CC_C : 0)
#define SET_C16(x)          (m_ccr |= ((x) & 0x10000) ? CC_C : 0)
#define CLEAR_Z()           (m_ccr &= ~(CC_Z))
#define CLEAR_C()           (m_ccr &= ~(CC_C))
#define CLEAR_NZV()         (m_ccr &= ~(CC_N | CC_Z | CC_V))
#define CLEAR_ZVC()         (m_ccr &= ~(CC_Z | CC_V | CC_C))
#define CLEAR_NZVC()        (m_ccr &= ~(CC_N | CC_Z | CC_V | CC_C))
#define CLEAR_HNZVC()       (m_ccr &= ~(CC_H | CC_N | CC_Z | CC_V | CC_C))

#define SET_ZFLAG()         (m_ccr |= CC_Z)
#define SET_NFLAG()         (m_ccr |= CC_N)
#define SET_VFLAG()         (m_ccr |= CC_V)

#define REG_A               m_d.d8.a
#define REG_B               m_d.d8.b
#define REG_D               m_d.d16

void mc68hc11_cpu_device::CYCLES(int cycles)
{
	m_icount -= cycles;
}

void mc68hc11_cpu_device::SET_PC(int pc)
{
	m_pc = pc;
}

void mc68hc11_cpu_device::PUSH8(UINT8 value)
{
	WRITE8(m_sp--, value);
}

void mc68hc11_cpu_device::PUSH16(UINT16 value)
{
	WRITE8(m_sp--, (value >> 0) & 0xff);
	WRITE8(m_sp--, (value >> 8) & 0xff);
}

UINT8 mc68hc11_cpu_device::POP8()
{
	return READ8(++m_sp);
}

UINT16 mc68hc11_cpu_device::POP16()
{
	UINT16 r = 0;
	r |= (READ8(++m_sp) << 8);
	r |= (READ8(++m_sp) << 0);
	return r;
}



/*****************************************************************************/

/* ABA              0x1B */
void HC11OP(aba)()
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
void HC11OP(abx)()
{
	m_ix += REG_B;
	CYCLES(3);
}


/* ABY              0x18, 0x3A */
void HC11OP(aby)()
{
	m_iy += REG_B;
	CYCLES(4);
}


/* ADCA IMM         0x89 */
void HC11OP(adca_imm)()
{
	UINT8 i = FETCH();
	UINT16 r = REG_A + i + ((m_ccr & CC_C) ? 1 : 0);
	CLEAR_HNZVC();
	SET_H(r, i, REG_A);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(2);
}

/* ADCA DIR         0x99 */
void HC11OP(adca_dir)()
{
	UINT8 d = FETCH();
	UINT8 i = READ8(d);
	UINT16 r = REG_A + i + ((m_ccr & CC_C) ? 1 : 0);
	CLEAR_HNZVC();
	SET_H(r, i, REG_A);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(3);
}

/* ADCA EXT         0xB9 */
void HC11OP(adca_ext)()
{
	UINT16 adr = FETCH16();
	UINT8 i = READ8(adr);
	UINT16 r = REG_A + i + ((m_ccr & CC_C) ? 1 : 0);
	CLEAR_HNZVC();
	SET_H(r, i, REG_A);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(4);
}

/* ADCA IND, X      0xA9 */
void HC11OP(adca_indx)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_ix + offset);
	UINT16 r = REG_A + i + ((m_ccr & CC_C) ? 1 : 0);
	CLEAR_HNZVC();
	SET_H(r, i, REG_A);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(4);
}

/* ADCA IND, Y      0x18, 0xA9 */
void HC11OP(adca_indy)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_iy + offset);
	UINT16 r = REG_A + i + ((m_ccr & CC_C) ? 1 : 0);
	CLEAR_HNZVC();
	SET_H(r, i, REG_A);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(5);
}


/* ADCB IMM         0xC9 */
void HC11OP(adcb_imm)()
{
	UINT8 i = FETCH();
	UINT16 r = REG_B + i + ((m_ccr & CC_C) ? 1 : 0);
	CLEAR_HNZVC();
	SET_H(r, i, REG_B);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(2);
}

/* ADCB DIR         0xD9 */
void HC11OP(adcb_dir)()
{
	UINT8 d = FETCH();
	UINT8 i = READ8(d);
	UINT16 r = REG_B + i + ((m_ccr & CC_C) ? 1 : 0);
	CLEAR_HNZVC();
	SET_H(r, i, REG_B);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(3);
}

/* ADCB EXT         0xF9 */
void HC11OP(adcb_ext)()
{
	UINT16 adr = FETCH16();
	UINT8 i = READ8(adr);
	UINT16 r = REG_B + i + ((m_ccr & CC_C) ? 1 : 0);
	CLEAR_HNZVC();
	SET_H(r, i, REG_B);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(4);
}

/* ADCB IND, X      0xE9 */
void HC11OP(adcb_indx)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_ix + offset);
	UINT16 r = REG_B + i + ((m_ccr & CC_C) ? 1 : 0);
	CLEAR_HNZVC();
	SET_H(r, i, REG_B);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(4);
}

/* ADCB IND, Y      0x18, 0xE9 */
void HC11OP(adcb_indy)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_iy + offset);
	UINT16 r = REG_B + i + ((m_ccr & CC_C) ? 1 : 0);
	CLEAR_HNZVC();
	SET_H(r, i, REG_B);
	SET_N8(r);
	SET_Z8(r);
	SET_V_ADD8(r, i, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(5);
}


/* ADDA IMM         0x8B */
void HC11OP(adda_imm)()
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
void HC11OP(adda_dir)()
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
void HC11OP(adda_ext)()
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
void HC11OP(adda_indx)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_ix + offset);
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
void HC11OP(adda_indy)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_iy + offset);
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
void HC11OP(addb_imm)()
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
void HC11OP(addb_dir)()
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
void HC11OP(addb_ext)()
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
void HC11OP(addb_indx)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_ix + offset);
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
void HC11OP(addb_indy)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_iy + offset);
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
void HC11OP(addd_imm)()
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
void HC11OP(addd_dir)()
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
void HC11OP(addd_ext)()
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
void HC11OP(addd_indx)()
{
	UINT8 offset = FETCH();
	UINT16 i = READ16(m_ix + offset);
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
void HC11OP(addd_indy)()
{
	UINT8 offset = FETCH();
	UINT16 i = READ16(m_iy + offset);
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
void HC11OP(anda_imm)()
{
	UINT8 i = FETCH();
	CLEAR_NZV();
	REG_A &= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(2);
}

/* ANDA DIR         0x94 */
void HC11OP(anda_dir)()
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
void HC11OP(anda_ext)()
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
void HC11OP(anda_indx)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_ix + offset);
	CLEAR_NZV();
	REG_A &= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(4);
}

/* ANDA IND, Y      0x18, 0xA4 */
void HC11OP(anda_indy)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_iy + offset);
	CLEAR_NZV();
	REG_A &= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(5);
}


/* ANDB IMM         0xC4 */
void HC11OP(andb_imm)()
{
	UINT8 i = FETCH();
	CLEAR_NZV();
	REG_B &= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(2);
}

/* ANDB DIR         0xD4 */
void HC11OP(andb_dir)()
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
void HC11OP(andb_ext)()
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
void HC11OP(andb_indx)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_ix + offset);
	CLEAR_NZV();
	REG_B &= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(4);
}

/* ANDB IND, Y      0x18, 0xE4 */
void HC11OP(andb_indy)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_iy + offset);
	CLEAR_NZV();
	REG_B &= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(5);
}

/* ASLA             0x48 */
void HC11OP(asla)()
{
	UINT16 r = REG_A << 1;
	CLEAR_NZVC();
	SET_C8(r);
	REG_A = (UINT16)(r);
	SET_N8(REG_A);
	SET_Z8(REG_A);

	if (((m_ccr & CC_N) && (m_ccr & CC_C) == 0) ||
		((m_ccr & CC_N) == 0 && (m_ccr & CC_C)))
	{
		m_ccr |= CC_V;
	}

	CYCLES(2);
}

/* ASLB             0x58 */
void HC11OP(aslb)()
{
	UINT16 r = REG_B << 1;
	CLEAR_NZVC();
	SET_C8(r);
	REG_B = (UINT16)(r);
	SET_N8(REG_B);
	SET_Z8(REG_B);

	if (((m_ccr & CC_N) && (m_ccr & CC_C) == 0) ||
		((m_ccr & CC_N) == 0 && (m_ccr & CC_C)))
	{
		m_ccr |= CC_V;
	}

	CYCLES(2);
}

/* ASL EXT             0x78 */
void HC11OP(asl_ext)()
{
	UINT16 adr = FETCH16();
	UINT8 i = READ8(adr);
	UINT16 r = i << 1;
	CLEAR_NZVC();
	SET_C8(r);
	WRITE8(adr, r);
	SET_N8(r);
	SET_Z8(r);

	if (((m_ccr & CC_N) && (m_ccr & CC_C) == 0) ||
		((m_ccr & CC_N) == 0 && (m_ccr & CC_C)))
	{
		m_ccr |= CC_V;
	}

	CYCLES(6);
}

/* BITA IMM         0x85 */
void HC11OP(bita_imm)()
{
	UINT8 i = FETCH();
	UINT8 r = REG_A & i;
	CLEAR_NZV();
	SET_N8(r);
	SET_Z8(r);
	CYCLES(2);
}

/* BITA DIR         0x95 */
void HC11OP(bita_dir)()
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
void HC11OP(bita_ext)()
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
void HC11OP(bita_indx)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_ix + offset);
	UINT8 r = REG_A & i;
	CLEAR_NZV();
	SET_N8(r);
	SET_Z8(r);
	CYCLES(4);
}

/* BITA IND, Y      0x18, 0xA5 */
void HC11OP(bita_indy)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_iy + offset);
	UINT8 r = REG_A & i;
	CLEAR_NZV();
	SET_N8(r);
	SET_Z8(r);
	CYCLES(5);
}


/* BITB IMM         0xC5 */
void HC11OP(bitb_imm)()
{
	UINT8 i = FETCH();
	UINT8 r = REG_B & i;
	CLEAR_NZV();
	SET_N8(r);
	SET_Z8(r);
	CYCLES(2);
}

/* BITB DIR         0xD5 */
void HC11OP(bitb_dir)()
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
void HC11OP(bitb_ext)()
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
void HC11OP(bitb_indx)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_ix + offset);
	UINT8 r = REG_B & i;
	CLEAR_NZV();
	SET_N8(r);
	SET_Z8(r);
	CYCLES(4);
}

/* BITB IND, Y      0x18, 0xE5 */
void HC11OP(bitb_indy)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_iy + offset);
	UINT8 r = REG_B & i;
	CLEAR_NZV();
	SET_N8(r);
	SET_Z8(r);
	CYCLES(5);
}

/* BCC              0x24 */
void HC11OP(bcc)()
{
	INT8 rel = FETCH();
	if ((m_ccr & CC_C) == 0)    /* Branch if C flag clear */
	{
		SET_PC(m_ppc + rel + 2);
	}
	CYCLES(3);
}

/* BCLR DIR       0x15 */
void HC11OP(bclr_dir)()
{
	UINT8 d = FETCH();
	UINT8 mask = FETCH();
	UINT8 r = READ8(d) & ~mask;
	WRITE8(d, r);
	CLEAR_NZV();
	SET_N8(r);
	SET_Z8(r);
	CYCLES(6);
}

/* BCLR INDX       0x1d */
void HC11OP(bclr_indx)()
{
	UINT8 offset = FETCH();
	UINT8 mask = FETCH();
	UINT8 r = READ8(m_ix + offset) & ~mask;
	WRITE8(m_ix + offset, r);
	CLEAR_NZV();
	SET_N8(r);
	SET_Z8(r);

	CYCLES(7);
}

/* BCS              0x25 */
void HC11OP(bcs)()
{
	INT8 rel = FETCH();
	if (m_ccr & CC_C)           /* Branch if C flag set */
	{
		SET_PC(m_ppc + rel + 2);
	}
	CYCLES(3);
}


/* BEQ              0x27 */
void HC11OP(beq)()
{
	INT8 rel = FETCH();
	if (m_ccr & CC_Z)           /* Branch if Z flag set */
	{
		SET_PC(m_ppc + rel + 2);
	}
	CYCLES(3);
}


/* BHI              0x22 */
void HC11OP(bhi)()
{
	INT8 rel = FETCH();
	if (((m_ccr & CC_C) == 0) && ((m_ccr & CC_Z) == 0)) /* Branch if C and Z flag clear */
	{
		SET_PC(m_ppc + rel + 2);
	}
	CYCLES(3);
}


/* BNE              0x26 */
void HC11OP(bne)()
{
	INT8 rel = FETCH();
	if ((m_ccr & CC_Z) == 0)        /* Branch if Z flag clear */
	{
		SET_PC(m_ppc + rel + 2);
	}
	CYCLES(3);
}


/* BLE              0x2F */
void HC11OP(ble)()
{
	UINT8 n = (m_ccr & CC_N) ? 1 : 0;
	UINT8 v = (m_ccr & CC_V) ? 1 : 0;
	INT8 rel = FETCH();
	if ((m_ccr & CC_Z) || (n ^ v))  /* Branch if Z flag set or (N ^ V) */
	{
		SET_PC(m_ppc + rel + 2);
	}
	CYCLES(3);
}

/* BLS              0x23 */
void HC11OP(bls)()
{
	INT8 rel = FETCH();
	if (m_ccr & CC_C || m_ccr & CC_Z)   /* Branch if C or Z flag set */
	{
		SET_PC(m_ppc + rel + 2);
	}
	CYCLES(3);
}

/* BMI              0x2B */
void HC11OP(bmi)()
{
	INT8 rel = FETCH();
	if (m_ccr & CC_N)       /* Branch if N flag set */
	{
		SET_PC(m_ppc + rel + 2);
	}
	CYCLES(3);
}

/* BPL              0x2A */
void HC11OP(bpl)()
{
	INT8 rel = FETCH();
	if ((m_ccr & CC_N) == 0)        /* Branch if N flag clear */
	{
		SET_PC(m_ppc + rel + 2);
	}
	CYCLES(3);
}


/* BRA              0x20 */
void HC11OP(bra)()
{
	INT8 rel = FETCH();
	SET_PC(m_ppc + rel + 2);
	CYCLES(3);
}

/* BRCLR DIR       0x13 */
void HC11OP(brclr_dir)()
{
	UINT8 d = FETCH();
	UINT8 mask = FETCH();
	INT8 rel = FETCH();
	UINT8 i = READ8(d);

	if ((i & mask) == 0)
	{
		SET_PC(m_ppc + rel + 4);
	}

	CYCLES(6);
}


/* BRCLR INDX       0x1F */
void HC11OP(brclr_indx)()
{
	UINT8 offset = FETCH();
	UINT8 mask = FETCH();
	INT8 rel = FETCH();
	UINT8 i = READ8(m_ix + offset);

	if ((i & mask) == 0)
	{
		SET_PC(m_ppc + rel + 4);
	}

	CYCLES(7);
}

/* BRSET DIR       0x12 */
void HC11OP(brset_dir)()
{
	UINT8 d = FETCH();
	UINT8 mask = FETCH();
	INT8 rel = FETCH();
	UINT8 i = READ8(d);

	if(i & mask)
	{
		SET_PC(m_ppc + rel + 4);
	}

	CYCLES(6);
}


/* BRSET INDX       0x1E */
void HC11OP(brset_indx)()
{
	UINT8 offset = FETCH();
	UINT8 mask = FETCH();
	INT8 rel = FETCH();
	UINT8 i = READ8(m_ix + offset);

	if ((~i & mask) == 0)
	{
		SET_PC(m_ppc + rel + 4);
	}

	CYCLES(7);
}


/* BRN              0x21 */
void HC11OP(brn)()
{
	/* with this opcode the branch condition is always false. */
	SET_PC(m_ppc + 2);
	CYCLES(3);
}

/* BSET DIR       0x14 */
void HC11OP(bset_dir)()
{
	UINT8 d = FETCH();
	UINT8 mask = FETCH();
	UINT8 r = READ8(d) | mask;
	WRITE8(d, r);
	CLEAR_NZV();
	SET_N8(r);
	SET_Z8(r);
	CYCLES(6);
}

/* BSET INDX       0x1c */
void HC11OP(bset_indx)()
{
	UINT8 offset = FETCH();
	UINT8 mask = FETCH();
	UINT8 r = READ8(m_ix + offset) | mask;
	WRITE8(m_ix + offset, r);
	CLEAR_NZV();
	SET_N8(r);
	SET_Z8(r);

	CYCLES(7);
}

/* BSR              0x8D */
void HC11OP(bsr)()
{
	INT8 rel = FETCH();
	UINT16 rt_adr = m_pc;
	PUSH16(rt_adr);
	SET_PC(m_ppc + rel + 2);
	CYCLES(6);
}

/* BVC              0x28 */
void HC11OP(bvc)()
{
	INT8 rel = FETCH();
	if ((m_ccr & CC_V) == 0)    /* Branch if V flag clear */
	{
		SET_PC(m_ppc + rel + 2);
	}
	CYCLES(3);
}

/* BVS              0x29 */
void HC11OP(bvs)()
{
	INT8 rel = FETCH();
	if (m_ccr & CC_V)   /* Branch if V flag set */
	{
		SET_PC(m_ppc + rel + 2);
	}
	CYCLES(3);
}

/* CBA              0x11 */
void HC11OP(cba)()
{
	UINT16 r = REG_A - REG_B;
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, REG_B, REG_A);
	SET_C8(r);
	CYCLES(2);
}

/* CLC              0x0C */
void HC11OP(clc)()
{
	m_ccr &= ~CC_C;
	CYCLES(2);
}


/* CLI              0x0E */
void HC11OP(cli)()
{
	m_ccr &= ~CC_I;
	CYCLES(2);
}


/* CLRA             0x4F */
void HC11OP(clra)()
{
	REG_A = 0;
	CLEAR_NZVC();
	SET_ZFLAG();
	CYCLES(2);
}

/* CLRB             0x5F */
void HC11OP(clrb)()
{
	REG_B = 0;
	CLEAR_NZVC();
	SET_ZFLAG();
	CYCLES(2);
}

/* CLR EXT          0x7F */
void HC11OP(clr_ext)()
{
	UINT16 adr = FETCH16();
	WRITE8(adr, 0);
	CLEAR_NZVC();
	SET_ZFLAG();
	CYCLES(6);
}

/* CLR IND, X       0x6F */
void HC11OP(clr_indx)()
{
	UINT8 offset = FETCH();
	WRITE8(m_ix + offset, 0);
	CLEAR_NZVC();
	SET_ZFLAG();
	CYCLES(6);
}

/* CLR IND, Y       0x18, 0x6F */
void HC11OP(clr_indy)()
{
	UINT8 offset = FETCH();
	WRITE8(m_iy + offset, 0);
	CLEAR_NZVC();
	SET_ZFLAG();
	CYCLES(7);
}


/* CLV              0x0A */
void HC11OP(clv)()
{
	m_ccr &= ~CC_V;
	CYCLES(2);
}


/* CMPA IMM         0x81 */
void HC11OP(cmpa_imm)()
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
void HC11OP(cmpa_dir)()
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
void HC11OP(cmpa_ext)()
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
void HC11OP(cmpa_indx)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_ix + offset);
	UINT16 r = REG_A - i;
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_A);
	SET_C8(r);
	CYCLES(4);
}

/* CMPA IND, Y      0x18, 0xA1 */
void HC11OP(cmpa_indy)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_iy + offset);
	UINT16 r = REG_A - i;
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_A);
	SET_C8(r);
	CYCLES(5);
}


/* CMPB IMM         0xC1 */
void HC11OP(cmpb_imm)()
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
void HC11OP(cmpb_dir)()
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
void HC11OP(cmpb_ext)()
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
void HC11OP(cmpb_indx)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_ix + offset);
	UINT16 r = REG_B - i;
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_B);
	SET_C8(r);
	CYCLES(4);
}

/* CMPB IND, Y      0x18, 0xE1 */
void HC11OP(cmpb_indy)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_iy + offset);
	UINT16 r = REG_B - i;
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_B);
	SET_C8(r);
	CYCLES(5);
}


/* COMA              , 0x43 */
void HC11OP(coma)()
{
	UINT16 r = 0xff - REG_A;
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	m_ccr |= CC_C; //always set for M6800 compatibility
	REG_A = r;
	CYCLES(2);
}


/* COMB              , 0x53 */
void HC11OP(comb)()
{
	UINT16 r = 0xff - REG_B;
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	m_ccr |= CC_C; //always set for M6800 compatibility
	REG_B = r;
	CYCLES(2);
}


/* CPD IMM          0x1A, 0x83 */
void HC11OP(cpd_imm)()
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
void HC11OP(cpd_dir)()
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
void HC11OP(cpd_ext)()
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
void HC11OP(cpd_indx)()
{
	UINT8 offset = FETCH();
	UINT16 i = READ16(m_ix + offset);
	UINT32 r = REG_D - i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, REG_D);
	SET_C16(r);
	CYCLES(7);
}

/* CPD IND, Y       0xCD, 0xA3 */
void HC11OP(cpd_indy)()
{
	UINT8 offset = FETCH();
	UINT16 i = READ16(m_iy + offset);
	UINT32 r = REG_D - i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, REG_D);
	SET_C16(r);
	CYCLES(7);
}


/* CPX IMM          0x8C */
void HC11OP(cpx_imm)()
{
	UINT16 i = FETCH16();
	UINT32 r = m_ix - i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, m_ix);
	SET_C16(r);
	CYCLES(4);
}

/* CPX DIR          0x9C */
void HC11OP(cpx_dir)()
{
	UINT8 d = FETCH();
	UINT16 i = READ16(d);
	UINT32 r = m_ix - i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, m_ix);
	SET_C16(r);
	CYCLES(5);
}

/* CPX EXT          0xBC */
void HC11OP(cpx_ext)()
{
	UINT16 adr = FETCH16();
	UINT16 i = READ16(adr);
	UINT32 r = m_ix - i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, m_ix);
	SET_C16(r);
	CYCLES(6);
}

/* CPX IND, X       0xAC */
void HC11OP(cpx_indx)()
{
	UINT8 offset = FETCH();
	UINT16 i = READ16(m_ix + offset);
	UINT32 r = m_ix - i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, m_ix);
	SET_C16(r);
	CYCLES(6);
}

/* CPX IND, Y       0xCD, 0xAC */
void HC11OP(cpx_indy)()
{
	UINT8 offset = FETCH();
	UINT16 i = READ16(m_iy + offset);
	UINT32 r = m_ix - i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, m_ix);
	SET_C16(r);
	CYCLES(7);
}

/* CPY IMM          0x18, 0x8C */
void HC11OP(cpy_imm)()
{
	UINT16 i = FETCH16();
	UINT32 r = m_iy - i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, m_iy);
	SET_C16(r);
	CYCLES(5);
}

/* CPY DIR          0x18 0x9C */
void HC11OP(cpy_dir)()
{
	UINT8 d = FETCH();
	UINT16 i = READ16(d);
	UINT32 r = m_iy - i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, m_iy);
	SET_C16(r);
	CYCLES(6);
}

/* CPY EXT          0x18 0xBC */
void HC11OP(cpy_ext)()
{
	UINT16 adr = FETCH16();
	UINT16 i = READ16(adr);
	UINT32 r = m_iy - i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, m_iy);
	SET_C16(r);
	CYCLES(7);
}

/* CPY IND, X       0x1A 0xAC */
void HC11OP(cpy_indx)()
{
	UINT8 offset = FETCH();
	UINT16 i = READ16(m_ix + offset);
	UINT32 r = m_iy - i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, m_iy);
	SET_C16(r);
	CYCLES(7);
}

/* CPY IND, Y       0x18 0xAC */
void HC11OP(cpy_indy)()
{
	UINT8 offset = FETCH();
	UINT16 i = READ16(m_iy + offset);
	UINT32 r = m_iy - i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, m_iy);
	SET_C16(r);
	CYCLES(7);
}

/* DECA             0x4A */
void HC11OP(deca)()
{
	CLEAR_NZV();
	if (REG_A == 0x80)
		SET_VFLAG();
	REG_A--;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(2);
}

/* DECB             0x5A */
void HC11OP(decb)()
{
	CLEAR_NZV();
	if (REG_B == 0x80)
		SET_VFLAG();
	REG_B--;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(2);
}

/* DEC EXT          0x7A */
void HC11OP(dec_ext)()
{
	UINT16 adr = FETCH16();
	UINT8 i = READ8(adr);

	CLEAR_NZV();
	if (i == 0x80)
		SET_VFLAG();
	i--;
	SET_N8(i);
	SET_Z8(i);
	WRITE8(adr, i);
	CYCLES(6);
}

/* DEC INDX          0x6A */
void HC11OP(dec_indx)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_ix + offset);

	CLEAR_NZV();
	if (i == 0x80)
		SET_VFLAG();
	i--;
	SET_N8(i);
	SET_Z8(i);
	WRITE8(m_ix + offset, i);
	CYCLES(6);
}

/* DEC INDY          0x18 0x6A */
void HC11OP(dec_indy)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_iy + offset);

	CLEAR_NZV();
	if (i == 0x80)
		SET_VFLAG();
	i--;
	SET_N8(i);
	SET_Z8(i);
	WRITE8(m_iy + offset, i);
	CYCLES(7);
}

/* DEX              0x09 */
void HC11OP(dex)()
{
	CLEAR_Z();
	m_ix--;
	SET_Z16(m_ix);
	CYCLES(3);
}


/* DEY              0x18, 0x09 */
void HC11OP(dey)()
{
	CLEAR_Z();
	m_iy--;
	SET_Z16(m_iy);
	CYCLES(4);
}


/* EORA IMM         0x88 */
void HC11OP(eora_imm)()
{
	UINT8 i = FETCH();
	CLEAR_NZV();
	REG_A ^= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(2);
}

/* EORA DIR         0x98 */
void HC11OP(eora_dir)()
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
void HC11OP(eora_ext)()
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
void HC11OP(eora_indx)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_ix + offset);
	CLEAR_NZV();
	REG_A ^= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(4);
}

/* EORA IND, Y      0x18, 0xA8 */
void HC11OP(eora_indy)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_iy + offset);
	CLEAR_NZV();
	REG_A ^= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(5);
}


/* EORB IMM         0xC8 */
void HC11OP(eorb_imm)()
{
	UINT8 i = FETCH();
	CLEAR_NZV();
	REG_B ^= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(2);
}

/* EORB DIR         0xD8 */
void HC11OP(eorb_dir)()
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
void HC11OP(eorb_ext)()
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
void HC11OP(eorb_indx)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_ix + offset);
	CLEAR_NZV();
	REG_B ^= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(4);
}

/* EORB IND, Y      0x18, 0xE8 */
void HC11OP(eorb_indy)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_iy + offset);
	CLEAR_NZV();
	REG_B ^= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(5);
}

/* IDIV             0x02 */
void HC11OP(idiv)()
{
	UINT16 numerator = REG_D;
	UINT16 denominator = m_ix;
	UINT16 remainder;
	UINT16 result;

	CLEAR_ZVC();
	if(denominator == 0) // divide by zero behaviour
	{
		remainder = 0xffff; // TODO: undefined behaviour according to the docs
		result = 0xffff;
		logerror("HC11: divide by zero at PC=%04x\n",m_pc-1);
		m_ccr |= CC_C;
	}
	else
	{
		remainder = numerator % denominator;
		result = numerator / denominator;
	}
	m_ix = result;
	REG_D = remainder;
	SET_Z16(result);

	CYCLES(41);
}

/* INCA             0x4C */
void HC11OP(inca)()
{
	CLEAR_NZV();
	if (REG_A == 0x7f)
		SET_VFLAG();
	REG_A++;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(2);
}

/* INCB             0x5C */
void HC11OP(incb)()
{
	CLEAR_NZV();
	if (REG_B == 0x7f)
		SET_VFLAG();
	REG_B++;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(2);
}

/* INC EXT          0x7C */
void HC11OP(inc_ext)()
{
	UINT16 adr = FETCH16();
	UINT8 i = READ8(adr);

	CLEAR_NZV();
	if (i == 0x7f)
		SET_VFLAG();
	i++;
	SET_N8(i);
	SET_Z8(i);
	WRITE8(adr, i);
	CYCLES(6);
}

/* INC INDX          0x6C */
void HC11OP(inc_indx)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_ix + offset);

	CLEAR_NZV();
	if (i == 0x7f)
		SET_VFLAG();
	i++;
	SET_N8(i);
	SET_Z8(i);
	WRITE8(m_ix + offset, i);
	CYCLES(6);
}


/* INC INDY          0x18 0x6C */
void HC11OP(inc_indy)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_iy + offset);

	CLEAR_NZV();
	if (i == 0x7f)
		SET_VFLAG();
	i++;
	SET_N8(i);
	SET_Z8(i);
	WRITE8(m_iy + offset, i);
	CYCLES(7);
}


/* INX              0x08 */
void HC11OP(inx)()
{
	CLEAR_Z();
	m_ix++;
	SET_Z16(m_ix);
	CYCLES(3);
}

/* INY              0x18, 0x08 */
void HC11OP(iny)()
{
	CLEAR_Z();
	m_iy++;
	SET_Z16(m_iy);
	CYCLES(4);
}

/* JMP IND X        0x6E */
void HC11OP(jmp_indx)()
{
	UINT16 adr = FETCH();
	SET_PC(m_ix + adr);
	CYCLES(3);
}

/* JMP IND Y        0x18 0x6E */
void HC11OP(jmp_indy)()
{
	UINT16 adr = FETCH();
	SET_PC(m_iy + adr);
	CYCLES(4);
}

/* JMP EXT          0x7E */
void HC11OP(jmp_ext)()
{
	UINT16 adr = FETCH16();
	SET_PC(adr);
	CYCLES(3);
}


/* JSR DIR          0x9D */
void HC11OP(jsr_dir)()
{
	UINT8 i = FETCH();
	UINT16 rt_adr = m_pc;
	PUSH16(rt_adr);
	SET_PC(i);
	CYCLES(5);
}

/* JSR EXT          0xBD */
void HC11OP(jsr_ext)()
{
	UINT16 adr = FETCH16();
	UINT16 rt_adr = m_pc;
	PUSH16(rt_adr);
	SET_PC(adr);
	CYCLES(6);
}

/* JSR IND, X       0xAD */
void HC11OP(jsr_indx)()
{
	UINT8 offset = FETCH();
	UINT16 rt_adr = m_pc;
	PUSH16(rt_adr);
	SET_PC(m_ix + offset);
	CYCLES(6);
}

/* JSR IND, Y       0x18, 0xAD */
void HC11OP(jsr_indy)()
{
	UINT8 offset = FETCH();
	UINT16 rt_adr = m_pc;
	PUSH16(rt_adr);
	SET_PC(m_iy + offset);
	CYCLES(6);
}


/* LDAA IMM         0x86 */
void HC11OP(ldaa_imm)()
{
	CLEAR_NZV();
	REG_A = FETCH();
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(2);
}

/* LDAA DIR         0x96 */
void HC11OP(ldaa_dir)()
{
	UINT8 d = FETCH();
	CLEAR_NZV();
	REG_A = READ8(d);
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(3);
}

/* LDAA EXT         0xB6 */
void HC11OP(ldaa_ext)()
{
	UINT16 adr = FETCH16();
	CLEAR_NZV();
	REG_A = READ8(adr);
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(4);
}

/* LDAA IND, X      0xA6 */
void HC11OP(ldaa_indx)()
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	REG_A = READ8(m_ix + offset);
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(4);
}

/* LDAA IND, Y      0x18, 0xA6 */
void HC11OP(ldaa_indy)()
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	REG_A = READ8(m_iy + offset);
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(5);
}

/* LDAB IMM         0xC6 */
void HC11OP(ldab_imm)()
{
	CLEAR_NZV();
	REG_B = FETCH();
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(2);
}

/* LDAB DIR         0xD6 */
void HC11OP(ldab_dir)()
{
	UINT8 d = FETCH();
	CLEAR_NZV();
	REG_B = READ8(d);
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(3);
}

/* LDAB EXT         0xF6 */
void HC11OP(ldab_ext)()
{
	UINT16 adr = FETCH16();
	CLEAR_NZV();
	REG_B = READ8(adr);
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(4);
}

/* LDAB IND, X      0xE6 */
void HC11OP(ldab_indx)()
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	REG_B = READ8(m_ix + offset);
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(4);
}

/* LDAB IND, Y      0x18, 0xE6 */
void HC11OP(ldab_indy)()
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	REG_B = READ8(m_iy + offset);
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(5);
}


/* LDD IMM          0xCC */
void HC11OP(ldd_imm)()
{
	CLEAR_NZV();
	REG_D = FETCH16();
	SET_N16(REG_D);
	SET_Z16(REG_D);
	CYCLES(3);
}

/* LDD DIR          0xDC */
void HC11OP(ldd_dir)()
{
	UINT8 d = FETCH();
	CLEAR_NZV();
	REG_D = READ16(d);
	SET_N16(REG_D);
	SET_Z16(REG_D);
	CYCLES(4);
}

/* LDD EXT          0xFC */
void HC11OP(ldd_ext)()
{
	UINT16 adr = FETCH16();
	CLEAR_NZV();
	REG_D = READ16(adr);
	SET_N16(REG_D);
	SET_Z16(REG_D);
	CYCLES(5);
}

/* LDD IND, X       0xEC */
void HC11OP(ldd_indx)()
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	REG_D = READ16(m_ix + offset);
	SET_N16(REG_D);
	SET_Z16(REG_D);
	CYCLES(5);
}

/* LDD IND, Y       0x18, 0xEC */
void HC11OP(ldd_indy)()
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	REG_D = READ16(m_iy + offset);
	SET_N16(REG_D);
	SET_Z16(REG_D);
	CYCLES(6);
}


/* LDS IMM          0x8E */
void HC11OP(lds_imm)()
{
	CLEAR_NZV();
	m_sp = FETCH16();
	SET_N16(m_sp);
	SET_Z16(m_sp);
	CYCLES(3);
}

/* LDS DIR          0x9E */
void HC11OP(lds_dir)()
{
	UINT8 i = FETCH();
	CLEAR_NZV();
	m_sp = READ16(i);
	SET_N16(m_sp);
	SET_Z16(m_sp);
	CYCLES(4);
}

/* LDS EXT          0xBE */
void HC11OP(lds_ext)()
{
	UINT16 adr = FETCH16();
	CLEAR_NZV();
	m_sp = READ16(adr);
	SET_N16(m_sp);
	SET_Z16(m_sp);
	CYCLES(5);
}

/* LDS IND, X       0xAE */
void HC11OP(lds_indx)()
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	m_sp = READ16(m_ix + offset);
	SET_N16(m_sp);
	SET_Z16(m_sp);
	CYCLES(5);
}

/* LDS IND, Y       0x18, 0xAE */
void HC11OP(lds_indy)()
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	m_sp = READ16(m_iy + offset);
	SET_N16(m_sp);
	SET_Z16(m_sp);
	CYCLES(6);
}


/* LDX IMM          0xCE */
void HC11OP(ldx_imm)()
{
	CLEAR_NZV();
	m_ix = FETCH16();
	SET_N16(m_ix);
	SET_Z16(m_ix);
	CYCLES(3);
}

/* LDX DIR          0xDE */
void HC11OP(ldx_dir)()
{
	UINT8 d = FETCH();
	CLEAR_NZV();
	m_ix = READ16(d);
	SET_N16(m_ix);
	SET_Z16(m_ix);
	CYCLES(4);
}

/* LDX EXT          0xFE */
void HC11OP(ldx_ext)()
{
	UINT16 adr = FETCH16();
	CLEAR_NZV();
	m_ix = READ16(adr);
	SET_N16(m_ix);
	SET_Z16(m_ix);
	CYCLES(5);
}

/* LDX IND, X       0xEE */
void HC11OP(ldx_indx)()
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	m_ix = READ16(m_ix + offset);
	SET_N16(m_ix);
	SET_Z16(m_ix);
	CYCLES(5);
}

/* LDX IND, Y       0xCD, 0xEE */
void HC11OP(ldx_indy)()
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	m_ix = READ16(m_iy + offset);
	SET_N16(m_ix);
	SET_Z16(m_ix);
	CYCLES(6);
}


/* LDY IMM          0x18, 0xCE */
void HC11OP(ldy_imm)()
{
	CLEAR_NZV();
	m_iy = FETCH16();
	SET_N16(m_iy);
	SET_Z16(m_iy);
	CYCLES(4);
}

/* LDY DIR          0x18, 0xDE */
void HC11OP(ldy_dir)()
{
	UINT8 d = FETCH();
	CLEAR_NZV();
	m_iy = READ16(d);
	SET_N16(m_iy);
	SET_Z16(m_iy);
	CYCLES(5);
}

/* LDY EXT          0x18, 0xFE */
void HC11OP(ldy_ext)()
{
	UINT16 adr = FETCH16();
	CLEAR_NZV();
	m_iy = READ16(adr);
	SET_N16(m_iy);
	SET_Z16(m_iy);
	CYCLES(6);
}

/* LDY IND, X       0x1A, 0xEE */
void HC11OP(ldy_indx)()
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	m_iy = READ16(m_ix + offset);
	SET_N16(m_iy);
	SET_Z16(m_iy);
	CYCLES(6);
}

/* LDY IND, Y       0x18, 0xEE */
void HC11OP(ldy_indy)()
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	m_iy = READ16(m_iy + offset);
	SET_N16(m_iy);
	SET_Z16(m_iy);
	CYCLES(6);
}

/* LSLD             0x05 */
void HC11OP(lsld)()
{
	UINT32 r = REG_D << 1;
	CLEAR_NZVC();
	SET_C16(r);
	REG_D = (UINT16)(r);
	SET_N16(REG_D);
	SET_Z16(REG_D);

	if (((m_ccr & CC_N) && (m_ccr & CC_C) == 0) ||
		((m_ccr & CC_N) == 0 && (m_ccr & CC_C)))
	{
		m_ccr |= CC_V;
	}

	CYCLES(3);
}

/* LSRA              0x44 */
void HC11OP(lsra)()
{
	UINT16 r = REG_A >> 1;
	CLEAR_NZVC();
	m_ccr |= (REG_A & 1) ? CC_C : 0;
	REG_A = (UINT8)(r);
	m_ccr |= ((m_ccr & CC_C)) ? CC_V : 0;
	SET_Z8(REG_A);

	CYCLES(2);
}

/* LSRB              0x54 */
void HC11OP(lsrb)()
{
	UINT16 r = REG_B >> 1;
	CLEAR_NZVC();
	m_ccr |= (REG_B & 1) ? CC_C : 0;
	REG_B = (UINT8)(r);
	m_ccr |= ((m_ccr & CC_C)) ? CC_V : 0;
	SET_Z8(REG_B);

	CYCLES(2);
}

/* LSRD             0x04 */
void HC11OP(lsrd)()
{
	UINT32 r = REG_D >> 1;
	CLEAR_NZVC();
	m_ccr |= (REG_D & 1) ? CC_C : 0;
	REG_D = (UINT16)(r);
	m_ccr |= ((m_ccr & CC_C)) ? CC_V : 0;

	SET_N16(REG_D);
	SET_Z16(REG_D);

	CYCLES(3);
}

/* MUL              0x3d */
void HC11OP(mul)()
{
	REG_D = REG_A * REG_B;
	CLEAR_C();
	m_ccr |= (REG_B & 0x80) ? CC_C : 0;
	CYCLES(10);
}

/* NEGA              0x40 */
void HC11OP(nega)()
{
	REG_A = 0x00 - REG_A;
	CLEAR_NZVC();
	SET_N8(REG_A);
	SET_Z8(REG_A);
	m_ccr |= (REG_A == 0x80) ? CC_V : 0;
	m_ccr |= (REG_A != 0x00) ? CC_C : 0;
	CYCLES(2);
}

/* NEGB              0x50 */
void HC11OP(negb)()
{
	REG_B = 0x00 - REG_B;
	CLEAR_NZVC();
	SET_N8(REG_B);
	SET_Z8(REG_B);
	m_ccr |= (REG_B == 0x80) ? CC_V : 0;
	m_ccr |= (REG_B != 0x00) ? CC_C : 0;
	CYCLES(2);
}


/* NEG EXT           0x70 */
void HC11OP(neg_ext)()
{
	UINT16 adr = FETCH16();
	UINT8 i = 0x00 - READ8(adr);
	CLEAR_NZVC();
	SET_N8(i);
	SET_Z8(i);
	m_ccr |= (i == 0x80) ? CC_V : 0;
	m_ccr |= (i != 0x00) ? CC_C : 0;
	WRITE8(adr, i);
	CYCLES(6);
}


/* NEG INDX           0x60 */
void HC11OP(neg_indx)()
{
	UINT16 offset = FETCH();
	UINT8 i = 0x00 - READ8(m_ix + offset);
	CLEAR_NZVC();
	SET_N8(i);
	SET_Z8(i);
	m_ccr |= (i == 0x80) ? CC_V : 0;
	m_ccr |= (i != 0x00) ? CC_C : 0;
	WRITE8(m_ix + offset, i);
	CYCLES(6);
}


/* NEG INDY           0x18 0x60 */
void HC11OP(neg_indy)()
{
	UINT16 offset = FETCH();
	UINT8 i = 0x00 - READ8(m_iy + offset);
	CLEAR_NZVC();
	SET_N8(i);
	SET_Z8(i);
	m_ccr |= (i == 0x80) ? CC_V : 0;
	m_ccr |= (i != 0x00) ? CC_C : 0;
	WRITE8(m_iy + offset, i);
	CYCLES(7);
}


/* NOP              0x01 */
void HC11OP(nop)()
{
	CYCLES(2);
}

/* PSHA             0x36 */
void HC11OP(psha)()
{
	PUSH8(REG_A);
	CYCLES(3);
}

/* ORAA IMM         0x8A */
void HC11OP(oraa_imm)()
{
	UINT8 i = FETCH();
	CLEAR_NZV();
	REG_A |= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(2);
}

/* ORAA DIR         0x9A */
void HC11OP(oraa_dir)()
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
void HC11OP(oraa_ext)()
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
void HC11OP(oraa_indx)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_ix + offset);
	CLEAR_NZV();
	REG_A |= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(4);
}

/* ORAA IND, Y      0x18, 0xAA */
void HC11OP(oraa_indy)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_iy + offset);
	CLEAR_NZV();
	REG_A |= i;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(5);
}


/* ORAB IMM         0xCA */
void HC11OP(orab_imm)()
{
	UINT8 i = FETCH();
	CLEAR_NZV();
	REG_B |= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(2);
}

/* ORAB DIR         0xDA */
void HC11OP(orab_dir)()
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
void HC11OP(orab_ext)()
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
void HC11OP(orab_indx)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_ix + offset);
	CLEAR_NZV();
	REG_B |= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(4);
}

/* ORAB IND, Y      0x18, 0xEA */
void HC11OP(orab_indy)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_iy + offset);
	CLEAR_NZV();
	REG_B |= i;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(5);
}


/* PSHB             0x37 */
void HC11OP(pshb)()
{
	PUSH8(REG_B);
	CYCLES(3);
}


/* PSHX             0x3C */
void HC11OP(pshx)()
{
	PUSH16(m_ix);
	CYCLES(4);
}


/* PSHY             0x18, 0x3C */
void HC11OP(pshy)()
{
	PUSH16(m_iy);
	CYCLES(5);
}


/* PULA             0x32 */
void HC11OP(pula)()
{
	REG_A = POP8();
	CYCLES(4);
}


/* PULB             0x33 */
void HC11OP(pulb)()
{
	REG_B = POP8();
	CYCLES(4);
}


/* PULX             0x38 */
void HC11OP(pulx)()
{
	m_ix = POP16();
	CYCLES(5);
}


/* PULY             0x18, 0x38 */
void HC11OP(puly)()
{
	m_iy = POP16();
	CYCLES(6);
}

/* ROLA             0x49 */
void HC11OP(rola)()
{
	UINT16 r = ((REG_A & 0x7f) << 1) | ((m_ccr & CC_C) ? 1 : 0);
	CLEAR_NZVC();
	m_ccr |= (REG_A & 0x80) ? CC_C : 0;
	REG_A = (UINT8)(r);
	SET_N8(REG_A);
	SET_Z8(REG_A);

	if (((m_ccr & CC_N) && (m_ccr & CC_C) == 0) ||
		((m_ccr & CC_N) == 0 && (m_ccr & CC_C)))
	{
		m_ccr |= CC_V;
	}

	CYCLES(2);
}

/* ROLB             0x59 */
void HC11OP(rolb)()
{
	UINT16 r = ((REG_B & 0x7f) << 1) | ((m_ccr & CC_C) ? 1 : 0);
	CLEAR_NZVC();
	m_ccr |= (REG_B & 0x80) ? CC_C : 0;
	REG_B = (UINT8)(r);
	SET_N8(REG_B);
	SET_Z8(REG_B);

	if (((m_ccr & CC_N) && (m_ccr & CC_C) == 0) ||
		((m_ccr & CC_N) == 0 && (m_ccr & CC_C)))
	{
		m_ccr |= CC_V;
	}

	CYCLES(2);
}

/* ROL EXT           0x79 */
void HC11OP(rol_ext)()
{
	UINT16 adr = FETCH16();
	UINT8 r = READ8(adr);
	UINT8 c = (r & 0x80);
	r = ((r & 0x7f) << 1) | ((m_ccr & CC_C) ? 1 : 0);
	CLEAR_NZVC();
	m_ccr |= (c & 0x80) ? CC_C : 0;
	SET_N8(r);
	SET_Z8(r);
	WRITE8(adr, r);

	if (((m_ccr & CC_N) && (m_ccr & CC_C) == 0) ||
		((m_ccr & CC_N) == 0 && (m_ccr & CC_C)))
	{
		m_ccr |= CC_V;
	}

	CYCLES(6);
}

/* ROL INDX           0x69 */
void HC11OP(rol_indx)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_ix + offset);
	UINT8 c = (i & 0x80);
	i = ((i & 0x7f) << 1) | ((m_ccr & CC_C) ? 1 : 0);
	CLEAR_NZVC();
	m_ccr |= (c & 0x80) ? CC_C : 0;
	SET_N8(i);
	SET_Z8(i);
	WRITE8(m_ix + offset, i);

	if (((m_ccr & CC_N) && (m_ccr & CC_C) == 0) ||
		((m_ccr & CC_N) == 0 && (m_ccr & CC_C)))
	{
		m_ccr |= CC_V;
	}

	CYCLES(6);
}

/* ROL INDY           0x18 0x69 */
void HC11OP(rol_indy)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_iy + offset);
	UINT8 c = (i & 0x80);
	i = ((i & 0x7f) << 1) | ((m_ccr & CC_C) ? 1 : 0);
	CLEAR_NZVC();
	m_ccr |= (c & 0x80) ? CC_C : 0;
	SET_N8(i);
	SET_Z8(i);
	WRITE8(m_iy + offset, i);

	if (((m_ccr & CC_N) && (m_ccr & CC_C) == 0) ||
		((m_ccr & CC_N) == 0 && (m_ccr & CC_C)))
	{
		m_ccr |= CC_V;
	}

	CYCLES(6);
}


/* RORA             0x46 */
void HC11OP(rora)()
{
	UINT16 r = ((REG_A & 0xfe) >> 1) | ((m_ccr & CC_C) ? 0x80 : 0);
	CLEAR_NZVC();
	m_ccr |= (REG_A & 1) ? CC_C : 0;
	REG_A = (UINT8)(r);
	SET_N8(REG_A);
	SET_Z8(REG_A);

	if (((m_ccr & CC_N) && (m_ccr & CC_C) == 0) ||
		((m_ccr & CC_N) == 0 && (m_ccr & CC_C)))
	{
		m_ccr |= CC_V;
	}

	CYCLES(2);
}

/* RORB             0x56 */
void HC11OP(rorb)()
{
	UINT16 r = ((REG_B & 0xfe) >> 1) | ((m_ccr & CC_C) ? 0x80 : 0);
	CLEAR_NZVC();
	m_ccr |= (REG_B & 1) ? CC_C : 0;
	REG_B = (UINT8)(r);
	SET_N8(REG_B);
	SET_Z8(REG_B);

	if (((m_ccr & CC_N) && (m_ccr & CC_C) == 0) ||
		((m_ccr & CC_N) == 0 && (m_ccr & CC_C)))
	{
		m_ccr |= CC_V;
	}

	CYCLES(2);
}

/* RTI              0x3B */
void HC11OP(rti)()
{
	UINT16 rt_adr;
	UINT8 x_flag = m_ccr & CC_X;
	m_ccr = POP8();
	if(x_flag == 0 && m_ccr & CC_X) //X flag cannot do a 0->1 transition with this instruction.
		m_ccr &= ~CC_X;
	REG_B = POP8();
	REG_A = POP8();
	m_ix = POP16();
	m_iy = POP16();
	rt_adr = POP16();
	SET_PC(rt_adr);
	CYCLES(12);
}

/* RTS              0x39 */
void HC11OP(rts)()
{
	UINT16 rt_adr = POP16();
	SET_PC(rt_adr);
	CYCLES(5);
}


/* SBA              0x10 */
void HC11OP(sba)()
{
	UINT16 r = REG_A - REG_B;
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, REG_B, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(2);
}


/* SBCA IMM         0x82 */
void HC11OP(sbca_imm)()
{
	UINT8 i = FETCH();
	UINT16 r = (REG_A - i) - ((m_ccr & CC_C) ? 1 : 0);
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(2);
}

/* SBCA IND, X      0xA2 */
void HC11OP(sbca_indx)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_ix + offset);
	UINT16 r = (REG_A - i) - ((m_ccr & CC_C) ? 1 : 0);
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(4);
}

/* SBCA IND, Y      0x18, 0xA2 */
void HC11OP(sbca_indy)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_iy + offset);
	UINT16 r = (REG_A - i) - ((m_ccr & CC_C) ? 1 : 0);
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(5);
}

/* SBCB IMM         0xC2 */
void HC11OP(sbcb_imm)()
{
	UINT8 i = FETCH();
	UINT16 r = (REG_B - i) - ((m_ccr & CC_C) ? 1 : 0);
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(2);
}

/* SBCB IND, X      0xE2 */
void HC11OP(sbcb_indx)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_ix + offset);
	UINT16 r = (REG_B - i) - ((m_ccr & CC_C) ? 1 : 0);
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(4);
}

/* SBCB IND, Y      0x18, 0xE2 */
void HC11OP(sbcb_indy)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_iy + offset);
	UINT16 r = (REG_B - i) - ((m_ccr & CC_C) ? 1 : 0);
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(5);
}

/* SEC              0x0D */
void HC11OP(sec)()
{
	m_ccr |= CC_C;
	CYCLES(2);
}

/* SEI              0x0F */
void HC11OP(sei)()
{
	m_ccr |= CC_I;
	CYCLES(2);
}

/* SEV              0x0B */
void HC11OP(sev)()
{
	m_ccr |= CC_V;
	CYCLES(2);
}

/* STAA DIR         0x97 */
void HC11OP(staa_dir)()
{
	UINT8 d = FETCH();
	CLEAR_NZV();
	SET_N8(REG_A);
	SET_Z8(REG_A);
	WRITE8(d, REG_A);
	CYCLES(3);
}

/* STAA EXT         0xB7 */
void HC11OP(staa_ext)()
{
	UINT16 adr = FETCH16();
	CLEAR_NZV();
	SET_N8(REG_A);
	SET_Z8(REG_A);
	WRITE8(adr, REG_A);
	CYCLES(4);
}

/* STAA IND, X      0xA7 */
void HC11OP(staa_indx)()
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	SET_N8(REG_A);
	SET_Z8(REG_A);
	WRITE8(m_ix + offset, REG_A);
	CYCLES(4);
}

/* STAA IND, Y      0x18, 0xA7 */
void HC11OP(staa_indy)()
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	SET_N8(REG_A);
	SET_Z8(REG_A);
	WRITE8(m_iy + offset, REG_A);
	CYCLES(5);
}

/* STAB DIR         0xD7 */
void HC11OP(stab_dir)()
{
	UINT8 d = FETCH();
	CLEAR_NZV();
	SET_N8(REG_B);
	SET_Z8(REG_B);
	WRITE8(d, REG_B);
	CYCLES(3);
}

/* STAB EXT         0xF7 */
void HC11OP(stab_ext)()
{
	UINT16 adr = FETCH16();
	CLEAR_NZV();
	SET_N8(REG_B);
	SET_Z8(REG_B);
	WRITE8(adr, REG_B);
	CYCLES(4);
}

/* STAB IND, X      0xE7 */
void HC11OP(stab_indx)()
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	SET_N8(REG_B);
	SET_Z8(REG_B);
	WRITE8(m_ix + offset, REG_B);
	CYCLES(4);
}

/* STAB IND, Y      0x18, 0xE7 */
void HC11OP(stab_indy)()
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	SET_N8(REG_B);
	SET_Z8(REG_B);
	WRITE8(m_iy + offset, REG_B);
	CYCLES(5);
}


/* STD DIR          0xDD */
void HC11OP(std_dir)()
{
	UINT8 d = FETCH();
	CLEAR_NZV();
	WRITE16(d, REG_D);
	SET_N16(REG_D);
	SET_Z16(REG_D);
	CYCLES(4);
}

/* STD EXT          0xFD */
void HC11OP(std_ext)()
{
	UINT16 adr = FETCH16();
	CLEAR_NZV();
	WRITE16(adr, REG_D);
	SET_N16(REG_D);
	SET_Z16(REG_D);
	CYCLES(5);
}

/* STD IND, X       0xED */
void HC11OP(std_indx)()
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	WRITE16(m_ix + offset, REG_D);
	SET_N16(REG_D);
	SET_Z16(REG_D);
	CYCLES(5);
}

/* STD IND, Y       0x18, 0xED */
void HC11OP(std_indy)()
{
	UINT8 offset = FETCH();
	CLEAR_NZV();
	WRITE16(m_iy + offset, REG_D);
	SET_N16(REG_D);
	SET_Z16(REG_D);
	CYCLES(6);
}

/* STS DIR          0x9F */
void HC11OP(sts_dir)()
{
	UINT8 d = FETCH();
	UINT16 r = m_sp;
	CLEAR_NZV();
	WRITE8(d, (r & 0xff00) >> 8);
	WRITE8(d + 1, (r & 0xff));
	SET_N16(r);
	SET_Z16(r);
	CYCLES(4);
}


/* STX DIR          0xDF */
void HC11OP(stx_dir)()
{
	UINT8 adr = FETCH();
	UINT16 r = m_ix;
	CLEAR_NZV();
	WRITE8(adr, (r & 0xff00) >> 8);
	WRITE8(adr + 1, (r & 0xff));
	SET_N16(r);
	SET_Z16(r);
	CYCLES(4);
}

/* STX EXT          0xFF */
void HC11OP(stx_ext)()
{
	UINT16 adr = FETCH16();
	UINT16 r = m_ix;
	CLEAR_NZV();
	WRITE8(adr, (r & 0xff00) >> 8);
	WRITE8(adr + 1, (r & 0xff));
	SET_N16(r);
	SET_Z16(r);
	CYCLES(5);
}


/* STX INDX         0xEF */
void HC11OP(stx_indx)()
{
	UINT16 adr = FETCH();
	UINT16 r = m_ix;
	CLEAR_NZV();
	WRITE8(m_ix + adr, (r & 0xff00) >> 8);
	WRITE8(m_ix + adr + 1, (r & 0xff));
	SET_N16(r);
	SET_Z16(r);
	CYCLES(5);
}


/* STX INDY         0xCD 0xEF */
void HC11OP(stx_indy)()
{
	UINT16 adr = FETCH();
	UINT16 r = m_ix;
	CLEAR_NZV();
	WRITE8(m_iy + adr, (r & 0xff00) >> 8);
	WRITE8(m_iy + adr + 1, (r & 0xff));
	SET_N16(r);
	SET_Z16(r);
	CYCLES(6);
}

/* STY DIR          0x18 0xDF */
void HC11OP(sty_dir)()
{
	UINT8 adr = FETCH();
	UINT16 r = m_iy;
	CLEAR_NZV();
	WRITE8(adr, (r & 0xff00) >> 8);
	WRITE8(adr + 1, (r & 0xff));
	SET_N16(r);
	SET_Z16(r);
	CYCLES(5);
}


/* STY EXT          0x18 0xFF */
void HC11OP(sty_ext)()
{
	UINT16 adr = FETCH16();
	UINT16 r = m_iy;
	CLEAR_NZV();
	WRITE8(adr, (r & 0xff00) >> 8);
	WRITE8(adr + 1, (r & 0xff));
	SET_N16(r);
	SET_Z16(r);
	CYCLES(6);
}

/* STY INDX         0x1A 0xEF */
void HC11OP(sty_indx)()
{
	UINT16 adr = FETCH();
	UINT16 r = m_iy;
	CLEAR_NZV();
	WRITE8(m_ix + adr, (r & 0xff00) >> 8);
	WRITE8(m_ix + adr + 1, (r & 0xff));
	SET_N16(r);
	SET_Z16(r);
	CYCLES(6);
}

/* STY INDY         0x18 0xEF */
void HC11OP(sty_indy)()
{
	UINT16 adr = FETCH();
	UINT16 r = m_iy;
	CLEAR_NZV();
	WRITE8(m_iy + adr, (r & 0xff00) >> 8);
	WRITE8(m_iy + adr + 1, (r & 0xff));
	SET_N16(r);
	SET_Z16(r);
	CYCLES(6);
}

/* STOP              0xCF */
void HC11OP(stop)()
{
	if(m_stop_state == 0 && ((m_ccr & CC_S) == 0))
	{
		m_stop_state = 1;
	}

	if(m_stop_state == 1)
	{
		SET_PC(m_ppc); // wait for an exception
	}

	if(m_stop_state == 2)
	{
		m_stop_state = 0;
	}

	CYCLES(2);
}


/* SUBA IMM         0x80 */
void HC11OP(suba_imm)()
{
	UINT8 i = FETCH();
	UINT16 r = REG_A - i;
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(2);
}


/* SUBA DIR         0xd0 */
void HC11OP(suba_dir)()
{
	UINT8 d = FETCH();
	UINT8 i = READ8(d);
	UINT16 r = REG_A - i;
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(3);
}


/* SUBA EXT         0xE0 */
void HC11OP(suba_ext)()
{
	UINT16 adr = FETCH16();
	UINT8 i = READ8(adr);
	UINT16 r = REG_A - i;
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(4);
}


/* SUBA INDX        0xA0 */
void HC11OP(suba_indx)()
{
	UINT16 adr = FETCH();
	UINT8 i = READ8(m_ix + adr);
	UINT16 r = REG_A - i;
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(4);
}


/* SUBA INDY        0x18 0xA0 */
void HC11OP(suba_indy)()
{
	UINT16 adr = FETCH();
	UINT8 i = READ8(m_iy + adr);
	UINT16 r = REG_A - i;
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_A);
	SET_C8(r);
	REG_A = (UINT8)r;
	CYCLES(5);
}


/* SUBB IMM         0xC0 */
void HC11OP(subb_imm)()
{
	UINT8 i = FETCH();
	UINT16 r = REG_B - i;
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(2);
}


/* SUBB DIR         0xD0 */
void HC11OP(subb_dir)()
{
	UINT8 d = FETCH();
	UINT8 i = READ8(d);
	UINT16 r = REG_B - i;
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(3);
}


/* SUBB EXT         0xF0 */
void HC11OP(subb_ext)()
{
	UINT16 adr = FETCH16();
	UINT8 i = READ8(adr);
	UINT16 r = REG_B - i;
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(4);
}


/* SUBB INDX        0xE0 */
void HC11OP(subb_indx)()
{
	UINT16 adr = FETCH();
	UINT8 i = READ8(m_ix + adr);
	UINT16 r = REG_B - i;
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(4);
}

/* SUBB INDY        0x18 0xE0 */
void HC11OP(subb_indy)()
{
	UINT16 adr = FETCH();
	UINT8 i = READ8(m_iy + adr);
	UINT16 r = REG_B - i;
	CLEAR_NZVC();
	SET_N8(r);
	SET_Z8(r);
	SET_V_SUB8(r, i, REG_B);
	SET_C8(r);
	REG_B = (UINT8)r;
	CYCLES(5);
}

/* SUBD IMM         0x83 */
void HC11OP(subd_imm)()
{
	UINT16 i = FETCH16();
	UINT32 r = REG_D - i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, REG_D);
	SET_C16(r);
	REG_D = (UINT16)r;
	CYCLES(4);
}

/* SUBD DIR        0x93 */
void HC11OP(subd_dir)()
{
	UINT8 d = FETCH();
	UINT16 i = READ16(d);
	UINT32 r = REG_D - i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, REG_D);
	SET_C16(r);
	REG_D = (UINT16)r;
	CYCLES(5);
}

/* SUBD EXT        0xB3 */
void HC11OP(subd_ext)()
{
	UINT16 addr = FETCH16();
	UINT16 i = READ16(addr);
	UINT32 r = REG_D - i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, REG_D);
	SET_C16(r);
	REG_D = (UINT16)r;
	CYCLES(6);
}

/* SUBD INDX        0xA3 */
void HC11OP(subd_indx)()
{
	UINT8 offset = FETCH();
	UINT16 i = READ16(m_ix + offset);
	UINT32 r = REG_D - i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, REG_D);
	SET_C16(r);
	REG_D = (UINT16)r;
	CYCLES(6);
}

/* SUBD INDY        0x18 0xA3 */
void HC11OP(subd_indy)()
{
	UINT8 offset = FETCH();
	UINT16 i = READ16(m_iy + offset);
	UINT32 r = REG_D - i;
	CLEAR_NZVC();
	SET_N16(r);
	SET_Z16(r);
	SET_V_SUB16(r, i, REG_D);
	SET_C16(r);
	REG_D = (UINT16)r;
	CYCLES(7);
}

/* SWI              0x3F */
void HC11OP(swi)()
{
	UINT16 pc_vector;
	//m_pc++;
	PUSH16(m_pc);
	PUSH16(m_iy);
	PUSH16(m_ix);
	PUSH8(REG_A);
	PUSH8(REG_B);
	PUSH8(m_ccr);
	pc_vector = READ16(0xfff6);
	SET_PC(pc_vector);
	m_ccr |= CC_I; //irq taken, mask the flag
	CYCLES(14);
}

/* TAB              0x16 */
void HC11OP(tab)()
{
	CLEAR_NZV();
	REG_B = REG_A;
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(2);
}

/* TAP              0x06 */
void HC11OP(tap)()
{
	UINT8 x_flag = m_ccr & CC_X;
	m_ccr = REG_A;
	if(x_flag == 0 && m_ccr & CC_X) //X flag cannot do a 0->1 transition with this instruction.
		m_ccr &= ~CC_X;

	CYCLES(2);
}

/* TBA              0x17 */
void HC11OP(tba)()
{
	CLEAR_NZV();
	REG_A = REG_B;
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(2);
}

/* TEST              0x00 */
void HC11OP(test)()
{
//  if(m_test_mode)
		SET_PC(m_ppc); // Note: docs says "incremented" but the behaviour makes me think that's actually "decremented".
//  else
//  {
//      TODO: execute an illegal opcode exception here (NMI)
//  }

	CYCLES(1);
}

/* TPA              0x07 */
void HC11OP(tpa)()
{
	REG_A = m_ccr;
	CYCLES(2);
}


/* TSTA             0x4D */
void HC11OP(tsta)()
{
	CLEAR_NZVC();
	SET_N8(REG_A);
	SET_Z8(REG_A);
	CYCLES(2);
}

/* TSTB             0x5D */
void HC11OP(tstb)()
{
	CLEAR_NZVC();
	SET_N8(REG_B);
	SET_Z8(REG_B);
	CYCLES(2);
}

/* TST EXT          0x7D */
void HC11OP(tst_ext)()
{
	UINT16 adr = FETCH16();
	UINT8 i = READ8(adr);
	CLEAR_NZVC();
	SET_N8(i);
	SET_Z8(i);
	CYCLES(6);
}

/* TST IND, X       0x6D */
void HC11OP(tst_indx)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_ix + offset);
	CLEAR_NZVC();
	SET_N8(i);
	SET_Z8(i);
	CYCLES(6);
}

/* TST IND, Y       0x18, 0x6D */
void HC11OP(tst_indy)()
{
	UINT8 offset = FETCH();
	UINT8 i = READ8(m_iy + offset);
	CLEAR_NZVC();
	SET_N8(i);
	SET_Z8(i);
	CYCLES(6);
}

/* TSX              0x30 */
void HC11OP(tsx)()
{
	m_ix = m_sp + 1;
	CYCLES(3);
}

/* TSY              0x18 0x30 */
void HC11OP(tsy)()
{
	m_iy = m_sp + 1;
	CYCLES(4);
}

/* TXS              0x35 */
void HC11OP(txs)()
{
	m_sp = m_ix - 1;
	CYCLES(3);
}

/* TYS              0x18 0x35 */
void HC11OP(tys)()
{
	m_sp = m_iy - 1;
	CYCLES(4);
}

/* WAI              0x3E */
void HC11OP(wai)()
{
	if(m_wait_state == 0)
	{
		/* TODO: the following is bogus, pushes regs HERE in an instruction that wants an irq to go out? */
		PUSH16(m_pc);
		PUSH16(m_iy);
		PUSH16(m_ix);
		PUSH8(REG_A);
		PUSH8(REG_B);
		PUSH8(m_ccr);
		CYCLES(14);
		m_wait_state = 1;
	}
	if(m_wait_state == 1)
	{
		SET_PC(m_ppc); // wait for an exception
		CYCLES(1);
	}
	if(m_wait_state == 2)
	{
		m_wait_state = 0;
		CYCLES(1);
	}
}

/* XGDX             0x8F */
void HC11OP(xgdx)()
{
	UINT16 tmp = REG_D;
	REG_D = m_ix;
	m_ix = tmp;
	CYCLES(3);
}


/* XGDY             0x18, 0x8F */
void HC11OP(xgdy)()
{
	UINT16 tmp = REG_D;
	REG_D = m_iy;
	m_iy = tmp;
	CYCLES(4);
}

/*****************************************************************************/

void HC11OP(page2)()
{
	UINT8 op2 = FETCH();
	(this->*hc11_optable_page2[op2])();
}

void HC11OP(page3)()
{
	UINT8 op2 = FETCH();
	(this->*hc11_optable_page3[op2])();
}

void HC11OP(page4)()
{
	UINT8 op2 = FETCH();
	(this->*hc11_optable_page4[op2])();
}

void HC11OP(invalid)()
{
	fatalerror("HC11: Invalid opcode 0x%02X at %04X\n", READ8(m_pc-1), m_pc-1);
}
