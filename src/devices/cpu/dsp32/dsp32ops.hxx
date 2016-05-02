// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    dsp32ops.inc
    Core implementation for the portable DSP32 emulator.

****************************************************************************/



//**************************************************************************
//  COMPILE-TIME OPTIONS
//**************************************************************************

// these defined latencies are a pain to implement, but are necessary
#define EMULATE_MEMORY_LATENCY      (1)
#define EMULATE_MULTIPLIER_LATENCY  (1)
#define EMULATE_AFLAGS_LATENCY      (1)

// these optimizations should have some effect, but they don't really, so
// leave them off
#define IGNORE_DAU_UV_FLAGS         (0)
#define ASSUME_WRITEABLE            (0)
#define ASSUME_UNCONDITIONAL_CAU    (0)



//**************************************************************************
//  MACROS
//**************************************************************************

#define SET_V_16(a,b,r)         m_vflags = (((a) ^ (b) ^ (r) ^ ((r) >> 1)) << 8)
#define SET_NZC_16(r)           m_nzcflags = ((r) << 8)
#define SET_NZCV_16(a,b,r)      SET_NZC_16(r); SET_V_16(a,b,r)
#define SET_NZ00_16(r)          m_nzcflags = (((r) << 8) & 0xffffff); m_vflags = 0

#define SET_V_24(a,b,r)         m_vflags = ((a) ^ (b) ^ (r) ^ ((r) >> 1))
#define SET_NZC_24(r)           m_nzcflags = (r)
#define SET_NZCV_24(a,b,r)      SET_NZC_24(r); SET_V_24(a,b,r)
#define SET_NZ00_24(r)          m_nzcflags = ((r) & 0xffffff); m_vflags = 0

#define TRUNCATE24(a)           ((a) & 0xffffff)
#define EXTEND16_TO_24(a)       TRUNCATE24((INT32)(INT16)(a))
#define REG16(a)                ((UINT16)m_r[a])
#define REG24(a)                (m_r[a])

#define WRITEABLE_REGS          (0x6f3efffe)
#if ASSUME_WRITEABLE
#define IS_WRITEABLE(r)         (1)
#else
#define IS_WRITEABLE(r)         (WRITEABLE_REGS & (1 << (r)))
#endif

#if ASSUME_UNCONDITIONAL_CAU
#define CONDITION_IS_TRUE()     (1)
#else
#define CONDITION_IS_TRUE()     (!(op & 0x400) || (condition((op >> 12) & 15)))
#endif

#if EMULATE_MEMORY_LATENCY
#define WWORD_DEFERRED(a,v)     do { int bufidx = m_mbuf_index & 3; m_mbufaddr[bufidx] = -(a); m_mbufdata[bufidx] = (v); } while (0)
#define WLONG_DEFERRED(a,v)     do { int bufidx = m_mbuf_index & 3; m_mbufaddr[bufidx] = (a); m_mbufdata[bufidx] = (v); } while (0)
#define PROCESS_DEFERRED_MEMORY()                                   \
	if (m_mbufaddr[++m_mbuf_index & 3] != 1)                    \
	{                                                                   \
		int bufidx = m_mbuf_index & 3;                              \
		if (m_mbufaddr[bufidx] >= 0)                                \
			WLONG(m_mbufaddr[bufidx], m_mbufdata[bufidx]);  \
		else                                                            \
			WWORD(-m_mbufaddr[bufidx], m_mbufdata[bufidx]); \
		m_mbufaddr[bufidx] = 1;                                     \
	}
#else
#define WWORD_DEFERRED(a,v) WWORD(a,v)
#define WLONG_DEFERRED(a,v) WLONG(a,v)
#define PROCESS_DEFERRED_MEMORY()
#endif

#if EMULATE_MULTIPLIER_LATENCY
#define DEFERRED_MULTIPLIER(x)  dau_get_amult(x)
#else
#define DEFERRED_MULTIPLIER(x)  m_a[x]
#endif

#if EMULATE_AFLAGS_LATENCY
#define DEFERRED_NZFLAGS()  dau_get_anzflags()
#define DEFERRED_VUFLAGS()  dau_get_avuflags()
#else
#define DEFERRED_NZFLAGS()  m_NZflags
#define DEFERRED_VUFLAGS()  m_VUflags
#endif



//**************************************************************************
//  TYPEDEFS
//**************************************************************************

union int_double
{
	double d;
	UINT32 i[2];
};



//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void dsp32c_device::illegal(UINT32 op)
{
}


void dsp32c_device::unimplemented(UINT32 op)
{
	fatalerror("Unimplemented op @ %06X: %08X (dis=%02X, tbl=%03X)\n", PC - 4, op, op >> 25, op >> 21);
}


inline void dsp32c_device::execute_one()
{
	UINT32 op;

	PROCESS_DEFERRED_MEMORY();
	debugger_instruction_hook(this, PC);
	op = ROPCODE(PC);
	m_icount -= 4;  // 4 clocks per cycle
	PC += 4;
	if (op)
		(this->*s_dsp32ops[op >> 21])(op);
}



//**************************************************************************
//  CAU HELPERS
//**************************************************************************

UINT32 dsp32c_device::cau_read_pi_special(UINT8 i)
{
	switch (i)
	{
		case 4:     return m_ibuf;
		case 5:     return m_obuf;
		case 6:     update_pcr(m_pcr & ~PCR_PDFs); update_pins(); return m_pdr;
		case 14:    return m_piop;
		case 20:    return m_pdr2;
		case 22:    update_pcr(m_pcr & ~PCR_PIFs); update_pins(); return m_pir;
		case 30:    return m_pcw;
		default:    fprintf(stderr, "Unimplemented CAU PI read = %X\n", i);
	}
	return 0;
}


void dsp32c_device::cau_write_pi_special(UINT8 i, UINT32 val)
{
	switch (i)
	{
		case 4:     m_ibuf = val;   break;
		case 5:     m_obuf = val;   break;
		case 6:     m_pdr = val; update_pcr(m_pcr | PCR_PDFs); update_pins(); break;
		case 14:    m_piop = val;   break;
		case 20:    m_pdr2 = val;   break;
		case 22:    m_pir = val; update_pcr(m_pcr | PCR_PIFs); update_pins(); break;
		case 30:    m_pcw = val;    break;
		default:    fprintf(stderr, "Unimplemented CAU PI write = %X\n", i);
	}
}


inline UINT8 dsp32c_device::cau_read_pi_1byte(int pi)
{
	int p = (pi >> 5) & 0x1f;
	int i = (pi >> 0) & 0x1f;
	if (p)
	{
		UINT32 result = RBYTE(m_r[p]);
		m_r[p] = TRUNCATE24(m_r[p] + m_r[i]);
		return result;
	}
	else
		return cau_read_pi_special(i);
}


inline UINT16 dsp32c_device::cau_read_pi_2byte(int pi)
{
	int p = (pi >> 5) & 0x1f;
	int i = (pi >> 0) & 0x1f;
	if (p)
	{
		UINT32 result = RWORD(m_r[p]);
		if (i < 22 || i > 23)
			m_r[p] = TRUNCATE24(m_r[p] + m_r[i]);
		else
			m_r[p] = TRUNCATE24(m_r[p] + m_r[i] * 2);
		return result;
	}
	else
		return cau_read_pi_special(i);
}


inline UINT32 dsp32c_device::cau_read_pi_4byte(int pi)
{
	int p = (pi >> 5) & 0x1f;
	int i = (pi >> 0) & 0x1f;
	if (p)
	{
		UINT32 result = RLONG(m_r[p]);
		if (i < 22 || i > 23)
			m_r[p] = TRUNCATE24(m_r[p] + m_r[i]);
		else
			m_r[p] = TRUNCATE24(m_r[p] + m_r[i] * 4);
		return result;
	}
	else
		return cau_read_pi_special(i);
}


inline void dsp32c_device::cau_write_pi_1byte(int pi, UINT8 val)
{
	int p = (pi >> 5) & 0x1f;
	int i = (pi >> 0) & 0x1f;
	if (p)
	{
		WBYTE(m_r[p], val);
		m_r[p] = TRUNCATE24(m_r[p] + m_r[i]);
	}
	else
		cau_write_pi_special(i, val);
}


inline void dsp32c_device::cau_write_pi_2byte(int pi, UINT16 val)
{
	int p = (pi >> 5) & 0x1f;
	int i = (pi >> 0) & 0x1f;
	if (p)
	{
		WWORD(m_r[p], val);
		if (i < 22 || i > 23)
			m_r[p] = TRUNCATE24(m_r[p] + m_r[i]);
		else
			m_r[p] = TRUNCATE24(m_r[p] + m_r[i] * 2);
	}
	else
		cau_write_pi_special(i, val);
}


inline void dsp32c_device::cau_write_pi_4byte(int pi, UINT32 val)
{
	int p = (pi >> 5) & 0x1f;
	int i = (pi >> 0) & 0x1f;
	if (p)
	{
		WLONG(m_r[p], (INT32)(val << 8) >> 8);
		if (i < 22 || i > 23)
			m_r[p] = TRUNCATE24(m_r[p] + m_r[i]);
		else
			m_r[p] = TRUNCATE24(m_r[p] + m_r[i] * 4);
	}
	else
		cau_write_pi_special(i, val);
}



//**************************************************************************
//  DAU HELPERS
//**************************************************************************

inline double dsp32c_device::dau_get_amult(int aidx)
{
	int bufidx = (m_abuf_index - 1) & 3;
	double val = m_a[aidx];
	while (m_icount >= m_abufcycle[bufidx] - 2 * 4)
	{
		if (m_abufreg[bufidx] == aidx)
			val = m_abuf[bufidx];
		bufidx = (bufidx - 1) & 3;
	}
	return val;
}


inline double dsp32c_device::dau_get_anzflags()
{
	int bufidx = (m_abuf_index - 1) & 3;
	double nzflags = m_NZflags;
	while (m_icount >= m_abufcycle[bufidx] - 3 * 4)
	{
		nzflags = m_abufNZflags[bufidx];
		bufidx = (bufidx - 1) & 3;
	}
	return nzflags;
}


inline UINT8 dsp32c_device::dau_get_avuflags()
{
#if (!IGNORE_DAU_UV_FLAGS)
	int bufidx = (m_abuf_index - 1) & 3;
	UINT8 vuflags = m_VUflags;
	while (m_icount >= m_abufcycle[bufidx] - 3 * 4)
	{
		vuflags = m_abufVUflags[bufidx];
		bufidx = (bufidx - 1) & 3;
	}
	return vuflags;
#else
	return 0;
#endif
}


inline void dsp32c_device::remember_last_dau(int aidx)
{
#if (EMULATE_MULTIPLIER_LATENCY || EMULATE_AFLAGS_LATENCY)
	int bufidx = m_abuf_index++ & 3;
	m_abuf[bufidx] = m_a[aidx];
	m_abufreg[bufidx] = aidx;
	m_abufNZflags[bufidx] = m_NZflags;
#if (!IGNORE_DAU_UV_FLAGS)
	m_abufVUflags[bufidx] = m_VUflags;
#endif
	m_abufcycle[bufidx] = m_icount;
#endif
}


inline void dsp32c_device::dau_set_val_noflags(int aidx, double res)
{
	remember_last_dau(aidx);
	m_a[aidx] = res;
}


inline void dsp32c_device::dau_set_val_flags(int aidx, double res)
{
	remember_last_dau(aidx);
#if (!IGNORE_DAU_UV_FLAGS)
{
	double absres = (res < 0) ? -res : res;
	m_VUflags = 0;
	if (absres < 5.87747e-39)
	{
		if (absres != 0)
			m_VUflags = UFLAGBIT;
		res = 0.0;
	}
	else if (absres > 3.40282e38)
	{
		m_VUflags = VFLAGBIT;
//      debugger_break(Machine);
//      fprintf(stderr, "Result = %g\n", absres);
		res = (res < 0) ? -3.40282e38 : 3.40282e38;
	}
}
#endif
	m_NZflags = res;
	m_a[aidx] = res;
}


inline double dsp32c_device::dsp_to_double(UINT32 val)
{
	int_double id;

	if (val == 0)
		return 0;
	else if ((INT32)val > 0)
	{
		int exponent = ((val & 0xff) - 128 + 1023) << 20;
		id.i[BYTE_XOR_BE(0)] = exponent + (val >> 11);
		id.i[BYTE_XOR_BE(1)] = (val << 21) & 0xe0000000;
	}
	else
	{
		int exponent = ((val & 0xff) - 128 + 1023) << 20;
		val = -(val & 0xffffff00);
		id.i[BYTE_XOR_BE(0)] = 0x80000000 + exponent + ((val >> 11) & 0x001fffff);
		id.i[BYTE_XOR_BE(1)] = (val << 21) & 0xe0000000;
	}
	return id.d;
}


inline UINT32 dsp32c_device::double_to_dsp(double val)
{
	int mantissa, exponent;
	int_double id;
	id.d = val;
	mantissa = ((id.i[BYTE_XOR_BE(0)] & 0x000fffff) << 11) | ((id.i[BYTE_XOR_BE(1)] & 0xe0000000) >> 21);
	exponent = ((id.i[BYTE_XOR_BE(0)] & 0x7ff00000) >> 20) - 1023 + 128;
	if (exponent < 0)
		return 0x00000000;
	else if (exponent > 255)
	{
//      debugger_break(Machine);
//      fprintf(stderr, "Exponent = %d\n", exponent);
		return ((INT32)id.i[BYTE_XOR_BE(0)] >= 0) ? 0x7fffffff : 0x800000ff;
	}
	else if ((INT32)id.i[BYTE_XOR_BE(0)] >= 0)
		return exponent | mantissa;
	else
	{
		mantissa = -mantissa;
		if (mantissa == 0) { mantissa = 0x80000000; exponent--; }
		return 0x80000000 | exponent | (mantissa & 0xffffff00);
	}
}


double dsp32c_device::dau_read_pi_special(int i)
{
	fatalerror("Unimplemented dau_read_pi_special(%d)\n", i);
	return 0;
}


void dsp32c_device::dau_write_pi_special(int i, double val)
{
	fatalerror("Unimplemented dau_write_pi_special(%d)\n", i);
}


inline double dsp32c_device::dau_read_pi_double_1st(int pi, int multiplier)
{
	int p = (pi >> 3) & 15;
	int i = (pi >> 0) & 7;

	m_lastp = p;
	if (p)
	{
		UINT32 result = RLONG(m_r[p]);
		if (i < 6)
			m_r[p] = TRUNCATE24(m_r[p] + m_r[i+16]);
		else
			m_r[p] = TRUNCATE24(m_r[p] + m_r[i+16] * 4);
		return dsp_to_double(result);
	}
	else if (i < 4)
		return multiplier ? DEFERRED_MULTIPLIER(i) : m_a[i];
	else
		return dau_read_pi_special(i);
}


inline double dsp32c_device::dau_read_pi_double_2nd(int pi, int multiplier, double xval)
{
	int p = (pi >> 3) & 15;
	int i = (pi >> 0) & 7;

	if (p == 15) p = m_lastp;       // P=15 means Z inherits from Y, Y inherits from X
	m_lastp = p;
	if (p)
	{
		UINT32 result;
		result = RLONG(m_r[p]);
		if (i < 6)
			m_r[p] = TRUNCATE24(m_r[p] + m_r[i+16]);
		else
			m_r[p] = TRUNCATE24(m_r[p] + m_r[i+16] * 4);
		return dsp_to_double(result);
	}
	else if (i < 4)
		return multiplier ? DEFERRED_MULTIPLIER(i) : m_a[i];
	else
		return dau_read_pi_special(i);
}


inline UINT32 dsp32c_device::dau_read_pi_4bytes(int pi)
{
	int p = (pi >> 3) & 15;
	int i = (pi >> 0) & 7;

	m_lastp = p;
	if (p)
	{
		UINT32 result = RLONG(m_r[p]);
		if (i < 6)
			m_r[p] = TRUNCATE24(m_r[p] + m_r[i+16]);
		else
			m_r[p] = TRUNCATE24(m_r[p] + m_r[i+16] * 4);
		return result;
	}
	else if (i < 4)
		return double_to_dsp(m_a[i]);
	else
		return dau_read_pi_special(i);
}


inline UINT16 dsp32c_device::dau_read_pi_2bytes(int pi)
{
	int p = (pi >> 3) & 15;
	int i = (pi >> 0) & 7;

	m_lastp = p;
	if (p)
	{
		UINT32 result = RWORD(m_r[p]);
		if (i < 6)
			m_r[p] = TRUNCATE24(m_r[p] + m_r[i+16]);
		else
			m_r[p] = TRUNCATE24(m_r[p] + m_r[i+16] * 2);
		return result;
	}
	else if (i < 4)
		return double_to_dsp(m_a[i]);
	else
		return dau_read_pi_special(i);
}


inline void dsp32c_device::dau_write_pi_double(int pi, double val)
{
	int p = (pi >> 3) & 15;
	int i = (pi >> 0) & 7;

	if (p == 15) p = m_lastp;       // P=15 means Z inherits from Y, Y inherits from X
	if (p)
	{
		WLONG_DEFERRED(m_r[p], double_to_dsp(val));
		if (i < 6)
			m_r[p] = TRUNCATE24(m_r[p] + m_r[i+16]);
		else
			m_r[p] = TRUNCATE24(m_r[p] + m_r[i+16] * 4);
	}
	else if (i < 4)
		dau_set_val_noflags(i, val);
	else
		dau_write_pi_special(i, val);
}


inline void dsp32c_device::dau_write_pi_4bytes(int pi, UINT32 val)
{
	int p = (pi >> 3) & 15;
	int i = (pi >> 0) & 7;

	if (p == 15) p = m_lastp;       // P=15 means Z inherits from Y, Y inherits from X
	if (p)
	{
		m_lastp = p;
		WLONG_DEFERRED(m_r[p], val);
		if (i < 6)
			m_r[p] = TRUNCATE24(m_r[p] + m_r[i+16]);
		else
			m_r[p] = TRUNCATE24(m_r[p] + m_r[i+16] * 4);
	}
	else if (i < 4)
		dau_set_val_noflags(i, dsp_to_double(val));
	else
		dau_write_pi_special(i, val);
}


inline void dsp32c_device::dau_write_pi_2bytes(int pi, UINT16 val)
{
	int p = (pi >> 3) & 15;
	int i = (pi >> 0) & 7;

	if (p == 15) p = m_lastp;       // P=15 means Z inherits from Y, Y inherits from X
	if (p)
	{
		m_lastp = p;
		WWORD_DEFERRED(m_r[p], val);
		if (i < 6)
			m_r[p] = TRUNCATE24(m_r[p] + m_r[i+16]);
		else
			m_r[p] = TRUNCATE24(m_r[p] + m_r[i+16] * 2);
	}
	else if (i < 4)
		dau_set_val_noflags(i, dsp_to_double(val << 16));
	else
		dau_write_pi_special(i, val);
}



//**************************************************************************
//  COMMON CONDITION ROUTINE
//**************************************************************************

#if (!ASSUME_UNCONDITIONAL_CAU)
int dsp32c_device::condition(int cond)
{
	switch (cond)
	{
		case 0:
			return 0;
		case 1:
			return 1;
		case 2:
			return !nFLAG;
		case 3:
			return nFLAG;
		case 4:
			return !zFLAG;
		case 5:
			return zFLAG;
		case 6:
			return !vFLAG;
		case 7:
			return vFLAG;
		case 8:
			return !cFLAG;
		case 9:
			return cFLAG;
		case 10:
			return !(nFLAG ^ cFLAG);
		case 11:
			return (nFLAG ^ cFLAG);
		case 12:
			return !(zFLAG | (nFLAG ^ vFLAG));
		case 13:
			return (zFLAG | (nFLAG ^ vFLAG));
		case 14:
			return !(cFLAG | zFLAG);
		case 15:
			return (cFLAG | zFLAG);

		case 16:
			return !(DEFERRED_VUFLAGS() & UFLAGBIT);
		case 17:
			return (DEFERRED_VUFLAGS() & UFLAGBIT);
		case 18:
			return !(DEFERRED_NZFLAGS() < 0);
		case 19:
			return (DEFERRED_NZFLAGS() < 0);
		case 20:
			return !(DEFERRED_NZFLAGS() == 0);
		case 21:
			return (DEFERRED_NZFLAGS() == 0);
		case 22:
			return !(DEFERRED_VUFLAGS() & VFLAGBIT);
		case 23:
			return (DEFERRED_VUFLAGS() & VFLAGBIT);
		case 24:
			return !(DEFERRED_NZFLAGS() <= 0);
		case 25:
			return (DEFERRED_NZFLAGS() <= 0);

		case 32:    // !ibf
		case 33:    // ibf
		case 34:    // !obe
		case 35:    // obe
		case 36:    // !pdf
		case 37:    // pdf
		case 38:    // !pif
		case 39:    // pif
		case 40:    // !sy
		case 41:    // sy
		case 42:    // !fb
		case 43:    // fb
		case 44:    // !ireq1
		case 45:    // ireq1
		case 46:    // !ireq2
		case 47:    // ireq2
		default:
			fatalerror("Unimplemented condition: %X\n", cond);
	}
}
#endif



//**************************************************************************
//  CAU BRANCH INSTRUCTION IMPLEMENTATION
//**************************************************************************

void dsp32c_device::nop(UINT32 op)
{
	if (op == 0)
		return;
	execute_one();
	PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
}


void dsp32c_device::goto_t(UINT32 op)
{
	execute_one();
	PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
}


void dsp32c_device::goto_pl(UINT32 op)
{
	if (!nFLAG)
	{
		execute_one();
		PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	}
}


void dsp32c_device::goto_mi(UINT32 op)
{
	if (nFLAG)
	{
		execute_one();
		PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	}
}


void dsp32c_device::goto_ne(UINT32 op)
{
	if (!zFLAG)
	{
		execute_one();
		PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	}
}


void dsp32c_device::goto_eq(UINT32 op)
{
	if (zFLAG)
	{
		execute_one();
		PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	}
}


void dsp32c_device::goto_vc(UINT32 op)
{
	if (!vFLAG)
	{
		execute_one();
		PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	}
}


void dsp32c_device::goto_vs(UINT32 op)
{
	if (vFLAG)
	{
		execute_one();
		PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	}
}


void dsp32c_device::goto_cc(UINT32 op)
{
	if (!cFLAG)
	{
		execute_one();
		PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	}
}


void dsp32c_device::goto_cs(UINT32 op)
{
	if (cFLAG)
	{
		execute_one();
		PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	}
}


void dsp32c_device::goto_ge(UINT32 op)
{
	if (!(nFLAG ^ vFLAG))
	{
		execute_one();
		PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	}
}


void dsp32c_device::goto_lt(UINT32 op)
{
	if (nFLAG ^ vFLAG)
	{
		execute_one();
		PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	}
}


void dsp32c_device::goto_gt(UINT32 op)
{
	if (!(zFLAG | (nFLAG ^ vFLAG)))
	{
		execute_one();
		PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	}
}


void dsp32c_device::goto_le(UINT32 op)
{
	if (zFLAG | (nFLAG ^ vFLAG))
	{
		execute_one();
		PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	}
}


void dsp32c_device::goto_hi(UINT32 op)
{
	if (!cFLAG && !zFLAG)
	{
		execute_one();
		PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	}
}


void dsp32c_device::goto_ls(UINT32 op)
{
	if (cFLAG || zFLAG)
	{
		execute_one();
		PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	}
}


void dsp32c_device::goto_auc(UINT32 op)
{
	if (!(DEFERRED_VUFLAGS() & UFLAGBIT))
	{
		execute_one();
		PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	}
}


void dsp32c_device::goto_aus(UINT32 op)
{
	if (DEFERRED_VUFLAGS() & UFLAGBIT)
	{
		execute_one();
		PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	}
}


void dsp32c_device::goto_age(UINT32 op)
{
	if (DEFERRED_NZFLAGS() >= 0)
	{
		execute_one();
		PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	}
}


void dsp32c_device::goto_alt(UINT32 op)
{
	if (DEFERRED_NZFLAGS() < 0)
	{
		execute_one();
		PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	}
}


void dsp32c_device::goto_ane(UINT32 op)
{
	if (DEFERRED_NZFLAGS() != 0)
	{
		execute_one();
		PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	}
}


void dsp32c_device::goto_aeq(UINT32 op)
{
	if (DEFERRED_NZFLAGS() == 0)
	{
		execute_one();
		PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	}
}


void dsp32c_device::goto_avc(UINT32 op)
{
	if (!(DEFERRED_VUFLAGS() & VFLAGBIT))
	{
		execute_one();
		PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	}
}


void dsp32c_device::goto_avs(UINT32 op)
{
	if (DEFERRED_VUFLAGS() & VFLAGBIT)
	{
		execute_one();
		PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	}
}


void dsp32c_device::goto_agt(UINT32 op)
{
	if (DEFERRED_NZFLAGS() > 0)
	{
		execute_one();
		PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	}
}


void dsp32c_device::goto_ale(UINT32 op)
{
	if (DEFERRED_NZFLAGS() <= 0)
	{
		execute_one();
		PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	}
}


void dsp32c_device::goto_ibe(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::goto_ibf(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::goto_obf(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::goto_obe(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::goto_pde(UINT32 op)
{
	if (!(m_pcr & PCR_PDFs))
	{
		execute_one();
		PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	}
}


void dsp32c_device::goto_pdf(UINT32 op)
{
	if (m_pcr & PCR_PDFs)
	{
		execute_one();
		PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	}
}


void dsp32c_device::goto_pie(UINT32 op)
{
	if (!(m_pcr & PCR_PIFs))
	{
		execute_one();
		PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	}
}


void dsp32c_device::goto_pif(UINT32 op)
{
	if (m_pcr & PCR_PIFs)
	{
		execute_one();
		PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	}
}


void dsp32c_device::goto_syc(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::goto_sys(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::goto_fbc(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::goto_fbs(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::goto_irq1lo(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::goto_irq1hi(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::goto_irq2lo(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::goto_irq2hi(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::dec_goto(UINT32 op)
{
	int hr = (op >> 21) & 0x1f;
	int old = (INT16)m_r[hr];
	m_r[hr] = EXTEND16_TO_24(m_r[hr] - 1);
	if (old >= 0)
	{
		execute_one();
		PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	}
}


void dsp32c_device::call(UINT32 op)
{
	int mr = (op >> 21) & 0x1f;
	if (IS_WRITEABLE(mr))
		m_r[mr] = PC + 4;
	execute_one();
	PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
}


void dsp32c_device::goto24(UINT32 op)
{
	execute_one();
	PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (op & 0xffff) + ((op >> 5) & 0xff0000));
}


void dsp32c_device::call24(UINT32 op)
{
	int mr = (op >> 16) & 0x1f;
	if (IS_WRITEABLE(mr))
		m_r[mr] = PC + 4;
	execute_one();
	PC = (op & 0xffff) + ((op >> 5) & 0xff0000);
}


void dsp32c_device::do_i(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::do_r(UINT32 op)
{
	unimplemented(op);
}



//**************************************************************************
//  CAU 16-BIT ARITHMETIC IMPLEMENTATION
//**************************************************************************

void dsp32c_device::add_si(UINT32 op)
{
	int dr = (op >> 21) & 0x1f;
	int hrval = REG16((op >> 16) & 0x1f);
	int res = hrval + (UINT16)op;
	if (IS_WRITEABLE(dr))
		m_r[dr] = EXTEND16_TO_24(res);
	SET_NZCV_16(hrval, op, res);
}


void dsp32c_device::add_ss(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG16((op >> 5) & 0x1f);
		int s2rval = (op & 0x800) ? REG16((op >> 0) & 0x1f) : REG16(dr);
		int res = s2rval + s1rval;
		if (IS_WRITEABLE(dr))
			m_r[dr] = EXTEND16_TO_24(res);
		SET_NZCV_16(s1rval, s2rval, res);
	}
}


void dsp32c_device::mul2_s(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG16((op >> 5) & 0x1f);
		int res = s1rval * 2;
		if (IS_WRITEABLE(dr))
			m_r[dr] = EXTEND16_TO_24(res);
		SET_NZCV_16(s1rval, 0, res);
	}
}


void dsp32c_device::subr_ss(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG16((op >> 5) & 0x1f);
		int s2rval = (op & 0x800) ? REG16((op >> 0) & 0x1f) : REG16(dr);
		int res = s1rval - s2rval;
		if (IS_WRITEABLE(dr))
			m_r[dr] = EXTEND16_TO_24(res);
		SET_NZCV_16(s1rval, s2rval, res);
	}
}


void dsp32c_device::addr_ss(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::sub_ss(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG16((op >> 5) & 0x1f);
		int s2rval = (op & 0x800) ? REG16((op >> 0) & 0x1f) : REG16(dr);
		int res = s2rval - s1rval;
		if (IS_WRITEABLE(dr))
			m_r[dr] = EXTEND16_TO_24(res);
		SET_NZCV_16(s1rval, s2rval, res);
	}
}


void dsp32c_device::neg_s(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG16((op >> 5) & 0x1f);
		int res = -s1rval;
		if (IS_WRITEABLE(dr))
			m_r[dr] = EXTEND16_TO_24(res);
		SET_NZCV_16(s1rval, 0, res);
	}
}


void dsp32c_device::andc_ss(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG16((op >> 5) & 0x1f);
		int s2rval = (op & 0x800) ? REG16((op >> 0) & 0x1f) : REG16(dr);
		int res = s2rval & ~s1rval;
		if (IS_WRITEABLE(dr))
			m_r[dr] = EXTEND16_TO_24(res);
		SET_NZ00_16(res);
	}
}


void dsp32c_device::cmp_ss(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int drval = REG16((op >> 16) & 0x1f);
		int s1rval = REG16((op >> 5) & 0x1f);
		int res = drval - s1rval;
		SET_NZCV_16(drval, s1rval, res);
	}
}


void dsp32c_device::xor_ss(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG16((op >> 5) & 0x1f);
		int s2rval = (op & 0x800) ? REG16((op >> 0) & 0x1f) : REG16(dr);
		int res = s2rval ^ s1rval;
		if (IS_WRITEABLE(dr))
			m_r[dr] = EXTEND16_TO_24(res);
		SET_NZ00_16(res);
	}
}


void dsp32c_device::rcr_s(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG16((op >> 5) & 0x1f);
		int res = ((m_nzcflags >> 9) & 0x8000) | (s1rval >> 1);
		if (IS_WRITEABLE(dr))
			m_r[dr] = EXTEND16_TO_24(res);
		m_nzcflags = ((res & 0xffff) << 8) | ((s1rval & 1) << 24);
		m_vflags = 0;
	}
}


void dsp32c_device::or_ss(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG16((op >> 5) & 0x1f);
		int s2rval = (op & 0x800) ? REG16((op >> 0) & 0x1f) : REG16(dr);
		int res = s2rval | s1rval;
		if (IS_WRITEABLE(dr))
			m_r[dr] = EXTEND16_TO_24(res);
		SET_NZ00_16(res);
	}
}


void dsp32c_device::rcl_s(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG16((op >> 5) & 0x1f);
		int res = ((m_nzcflags >> 24) & 0x0001) | (s1rval << 1);
		if (IS_WRITEABLE(dr))
			m_r[dr] = EXTEND16_TO_24(res);
		m_nzcflags = ((res & 0xffff) << 8) | ((s1rval & 0x8000) << 9);
		m_vflags = 0;
	}
}


void dsp32c_device::shr_s(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG16((op >> 5) & 0x1f);
		int res = s1rval >> 1;
		if (IS_WRITEABLE(dr))
			m_r[dr] = EXTEND16_TO_24(res);
		m_nzcflags = ((res & 0xffff) << 8) | ((s1rval & 1) << 24);
		m_vflags = 0;
	}
}


void dsp32c_device::div2_s(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG16((op >> 5) & 0x1f);
		int res = (s1rval & 0x8000) | (s1rval >> 1);
		if (IS_WRITEABLE(dr))
			m_r[dr] = EXTEND16_TO_24(res);
		m_nzcflags = ((res & 0xffff) << 8) | ((s1rval & 1) << 24);
		m_vflags = 0;
	}
}


void dsp32c_device::and_ss(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG16((op >> 5) & 0x1f);
		int s2rval = (op & 0x800) ? REG16((op >> 0) & 0x1f) : REG16(dr);
		int res = s2rval & s1rval;
		if (IS_WRITEABLE(dr))
			m_r[dr] = EXTEND16_TO_24(res);
		SET_NZ00_16(res);
	}
}


void dsp32c_device::test_ss(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int drval = REG16((op >> 16) & 0x1f);
		int s1rval = REG16((op >> 5) & 0x1f);
		int res = drval & s1rval;
		SET_NZ00_16(res);
	}
}


void dsp32c_device::add_di(UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	int drval = REG16(dr);
	int res = drval + (UINT16)op;
	if (IS_WRITEABLE(dr))
		m_r[dr] = EXTEND16_TO_24(res);
	SET_NZCV_16(drval, op, res);
}


void dsp32c_device::subr_di(UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	int drval = REG16(dr);
	int res = (UINT16)op - drval;
	if (IS_WRITEABLE(dr))
		m_r[dr] = EXTEND16_TO_24(res);
	SET_NZCV_16(drval, op, res);
}


void dsp32c_device::addr_di(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::sub_di(UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	int drval = REG16(dr);
	int res = drval - (UINT16)op;
	if (IS_WRITEABLE(dr))
		m_r[dr] = EXTEND16_TO_24(res);
	SET_NZCV_16(drval, op, res);
}


void dsp32c_device::andc_di(UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	int drval = REG16(dr);
	int res = drval & ~(UINT16)op;
	if (IS_WRITEABLE(dr))
		m_r[dr] = EXTEND16_TO_24(res);
	SET_NZ00_16(res);
}


void dsp32c_device::cmp_di(UINT32 op)
{
	int drval = REG16((op >> 16) & 0x1f);
	int res = drval - (UINT16)op;
	SET_NZCV_16(drval, op, res);
}


void dsp32c_device::xor_di(UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	int drval = REG16(dr);
	int res = drval ^ (UINT16)op;
	if (IS_WRITEABLE(dr))
		m_r[dr] = EXTEND16_TO_24(res);
	SET_NZ00_16(res);
}


void dsp32c_device::or_di(UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	int drval = REG16(dr);
	int res = drval | (UINT16)op;
	if (IS_WRITEABLE(dr))
		m_r[dr] = EXTEND16_TO_24(res);
	SET_NZ00_16(res);
}


void dsp32c_device::and_di(UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	int drval = REG16(dr);
	int res = drval & (UINT16)op;
	if (IS_WRITEABLE(dr))
		m_r[dr] = EXTEND16_TO_24(res);
	SET_NZ00_16(res);
}


void dsp32c_device::test_di(UINT32 op)
{
	int drval = REG16((op >> 16) & 0x1f);
	int res = drval & (UINT16)op;
	SET_NZ00_16(res);
}



//**************************************************************************
//  CAU 24-BIT ARITHMETIC IMPLEMENTATION
//**************************************************************************

void dsp32c_device::adde_si(UINT32 op)
{
	int dr = (op >> 21) & 0x1f;
	int hrval = REG24((op >> 16) & 0x1f);
	int res = hrval + EXTEND16_TO_24(op);
	if (IS_WRITEABLE(dr))
		m_r[dr] = TRUNCATE24(res);
	SET_NZCV_24(hrval, op << 8, res);
}


void dsp32c_device::adde_ss(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG24((op >> 5) & 0x1f);
		int s2rval = (op & 0x800) ? REG24((op >> 0) & 0x1f) : REG24(dr);
		int res = s2rval + s1rval;
		if (IS_WRITEABLE(dr))
			m_r[dr] = TRUNCATE24(res);
		SET_NZCV_24(s1rval, s2rval, res);
	}
}


void dsp32c_device::mul2e_s(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG24((op >> 5) & 0x1f);
		int res = s1rval * 2;
		if (IS_WRITEABLE(dr))
			m_r[dr] = TRUNCATE24(res);
		SET_NZCV_24(s1rval, 0, res);
	}
}


void dsp32c_device::subre_ss(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG24((op >> 5) & 0x1f);
		int s2rval = (op & 0x800) ? REG24((op >> 0) & 0x1f) : REG24(dr);
		int res = s1rval - s2rval;
		if (IS_WRITEABLE(dr))
			m_r[dr] = TRUNCATE24(res);
		SET_NZCV_24(s1rval, s2rval, res);
	}
}


void dsp32c_device::addre_ss(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::sube_ss(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG24((op >> 5) & 0x1f);
		int s2rval = (op & 0x800) ? REG24((op >> 0) & 0x1f) : REG24(dr);
		int res = s2rval - s1rval;
		if (IS_WRITEABLE(dr))
			m_r[dr] = TRUNCATE24(res);
		SET_NZCV_24(s1rval, s2rval, res);
	}
}


void dsp32c_device::nege_s(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG24((op >> 5) & 0x1f);
		int res = -s1rval;
		if (IS_WRITEABLE(dr))
			m_r[dr] = TRUNCATE24(res);
		SET_NZCV_24(s1rval, 0, res);
	}
}


void dsp32c_device::andce_ss(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG24((op >> 5) & 0x1f);
		int s2rval = (op & 0x800) ? REG24((op >> 0) & 0x1f) : REG24(dr);
		int res = s2rval & ~s1rval;
		if (IS_WRITEABLE(dr))
			m_r[dr] = res;
		SET_NZ00_24(res);
	}
}


void dsp32c_device::cmpe_ss(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int drval = REG24((op >> 16) & 0x1f);
		int s1rval = REG24((op >> 5) & 0x1f);
		int res = drval - s1rval;
		SET_NZCV_24(drval, s1rval, res);
	}
}


void dsp32c_device::xore_ss(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG24((op >> 5) & 0x1f);
		int s2rval = (op & 0x800) ? REG24((op >> 0) & 0x1f) : REG24(dr);
		int res = s2rval ^ s1rval;
		if (IS_WRITEABLE(dr))
			m_r[dr] = res;
		SET_NZ00_24(res);
	}
}


void dsp32c_device::rcre_s(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG24((op >> 5) & 0x1f);
		int res = ((m_nzcflags >> 1) & 0x800000) | (s1rval >> 1);
		if (IS_WRITEABLE(dr))
			m_r[dr] = TRUNCATE24(res);
		m_nzcflags = res | ((s1rval & 1) << 24);
		m_vflags = 0;
	}
}


void dsp32c_device::ore_ss(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG24((op >> 5) & 0x1f);
		int s2rval = (op & 0x800) ? REG24((op >> 0) & 0x1f) : REG24(dr);
		int res = s2rval | s1rval;
		if (IS_WRITEABLE(dr))
			m_r[dr] = res;
		SET_NZ00_24(res);
	}
}


void dsp32c_device::rcle_s(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG24((op >> 5) & 0x1f);
		int res = ((m_nzcflags >> 24) & 0x000001) | (s1rval << 1);
		if (IS_WRITEABLE(dr))
			m_r[dr] = TRUNCATE24(res);
		m_nzcflags = res | ((s1rval & 0x800000) << 1);
		m_vflags = 0;
	}
}


void dsp32c_device::shre_s(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG24((op >> 5) & 0x1f);
		int res = s1rval >> 1;
		if (IS_WRITEABLE(dr))
			m_r[dr] = res;
		m_nzcflags = res | ((s1rval & 1) << 24);
		m_vflags = 0;
	}
}


void dsp32c_device::div2e_s(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG24((op >> 5) & 0x1f);
		int res = (s1rval & 0x800000) | (s1rval >> 1);
		if (IS_WRITEABLE(dr))
			m_r[dr] = TRUNCATE24(res);
		m_nzcflags = res | ((s1rval & 1) << 24);
		m_vflags = 0;
	}
}


void dsp32c_device::ande_ss(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG24((op >> 5) & 0x1f);
		int s2rval = (op & 0x800) ? REG24((op >> 0) & 0x1f) : REG24(dr);
		int res = s2rval & s1rval;
		if (IS_WRITEABLE(dr))
			m_r[dr] = res;
		SET_NZ00_24(res);
	}
}


void dsp32c_device::teste_ss(UINT32 op)
{
	if (CONDITION_IS_TRUE())
	{
		int drval = REG24((op >> 16) & 0x1f);
		int s1rval = REG24((op >> 5) & 0x1f);
		int res = drval & s1rval;
		SET_NZ00_24(res);
	}
}


void dsp32c_device::adde_di(UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	int drval = REG24(dr);
	int res = drval + EXTEND16_TO_24(op);
	if (IS_WRITEABLE(dr))
		m_r[dr] = TRUNCATE24(res);
	SET_NZCV_24(drval, op << 8, res);
}


void dsp32c_device::subre_di(UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	int drval = REG24(dr);
	int res = EXTEND16_TO_24(op) - drval;
	if (IS_WRITEABLE(dr))
		m_r[dr] = TRUNCATE24(res);
	SET_NZCV_24(drval, op << 8, res);
}


void dsp32c_device::addre_di(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::sube_di(UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	int drval = REG24(dr);
	int res = drval - EXTEND16_TO_24(op);
	if (IS_WRITEABLE(dr))
		m_r[dr] = TRUNCATE24(res);
	SET_NZCV_24(drval, op << 8, res);
}


void dsp32c_device::andce_di(UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	int drval = REG24(dr);
	int res = drval & ~EXTEND16_TO_24(op);
	if (IS_WRITEABLE(dr))
		m_r[dr] = res;
	SET_NZ00_24(res);
}


void dsp32c_device::cmpe_di(UINT32 op)
{
	int drval = REG24((op >> 16) & 0x1f);
	int res = drval - EXTEND16_TO_24(op);
	SET_NZCV_24(drval, op << 8, res);
}


void dsp32c_device::xore_di(UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	int drval = REG24(dr);
	int res = drval ^ EXTEND16_TO_24(op);
	if (IS_WRITEABLE(dr))
		m_r[dr] = res;
	SET_NZ00_24(res);
}


void dsp32c_device::ore_di(UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	int drval = REG24(dr);
	int res = drval | EXTEND16_TO_24(op);
	if (IS_WRITEABLE(dr))
		m_r[dr] = res;
	SET_NZ00_24(res);
}


void dsp32c_device::ande_di(UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	int drval = REG24(dr);
	int res = drval & EXTEND16_TO_24(op);
	if (IS_WRITEABLE(dr))
		m_r[dr] = res;
	SET_NZ00_24(res);
}


void dsp32c_device::teste_di(UINT32 op)
{
	int drval = REG24((op >> 16) & 0x1f);
	int res = drval & EXTEND16_TO_24(op);
	SET_NZ00_24(res);
}



//**************************************************************************
//  CAU LOAD/STORE IMPLEMENTATION
//**************************************************************************

void dsp32c_device::load_hi(UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	UINT32 res = RBYTE(EXTEND16_TO_24(op));
	if (IS_WRITEABLE(dr))
		m_r[dr] = EXTEND16_TO_24(res);
	m_nzcflags = res << 8;
	m_vflags = 0;
}


void dsp32c_device::load_li(UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	UINT32 res = RBYTE(EXTEND16_TO_24(op));
	if (IS_WRITEABLE(dr))
		m_r[dr] = res;
	m_nzcflags = res << 8;
	m_vflags = 0;
}


void dsp32c_device::load_i(UINT32 op)
{
	UINT32 res = RWORD(EXTEND16_TO_24(op));
	int dr = (op >> 16) & 0x1f;
	if (IS_WRITEABLE(dr))
		m_r[dr] = EXTEND16_TO_24(res);
	m_nzcflags = res << 8;
	m_vflags = 0;
}


void dsp32c_device::load_ei(UINT32 op)
{
	UINT32 res = TRUNCATE24(RLONG(EXTEND16_TO_24(op)));
	int dr = (op >> 16) & 0x1f;
	if (IS_WRITEABLE(dr))
		m_r[dr] = res;
	m_nzcflags = res;
	m_vflags = 0;
}


void dsp32c_device::store_hi(UINT32 op)
{
	WBYTE(EXTEND16_TO_24(op), m_r[(op >> 16) & 0x1f] >> 8);
}


void dsp32c_device::store_li(UINT32 op)
{
	WBYTE(EXTEND16_TO_24(op), m_r[(op >> 16) & 0x1f]);
}


void dsp32c_device::store_i(UINT32 op)
{
	WWORD(EXTEND16_TO_24(op), REG16((op >> 16) & 0x1f));
}


void dsp32c_device::store_ei(UINT32 op)
{
	WLONG(EXTEND16_TO_24(op), (INT32)(REG24((op >> 16) & 0x1f) << 8) >> 8);
}


void dsp32c_device::load_hr(UINT32 op)
{
	if (!(op & 0x400))
	{
		int dr = (op >> 16) & 0x1f;
		UINT32 res = cau_read_pi_1byte(op) << 8;
		if (IS_WRITEABLE(dr))
			m_r[dr] = EXTEND16_TO_24(res);
		m_nzcflags = res << 8;
		m_vflags = 0;
	}
	else
		unimplemented(op);
}


void dsp32c_device::load_lr(UINT32 op)
{
	if (!(op & 0x400))
	{
		int dr = (op >> 16) & 0x1f;
		UINT32 res = cau_read_pi_1byte(op);
		if (IS_WRITEABLE(dr))
			m_r[dr] = res;
		m_nzcflags = res << 8;
		m_vflags = 0;
	}
	else
		unimplemented(op);
}


void dsp32c_device::load_r(UINT32 op)
{
	if (!(op & 0x400))
	{
		UINT32 res = cau_read_pi_2byte(op);
		int dr = (op >> 16) & 0x1f;
		if (IS_WRITEABLE(dr))
			m_r[dr] = EXTEND16_TO_24(res);
		m_nzcflags = res << 8;
		m_vflags = 0;
	}
	else
		unimplemented(op);
}


void dsp32c_device::load_er(UINT32 op)
{
	if (!(op & 0x400))
	{
		UINT32 res = TRUNCATE24(cau_read_pi_4byte(op));
		int dr = (op >> 16) & 0x1f;
		if (IS_WRITEABLE(dr))
			m_r[dr] = res;
		m_nzcflags = res;
		m_vflags = 0;
	}
	else
		unimplemented(op);
}


void dsp32c_device::store_hr(UINT32 op)
{
	if (!(op & 0x400))
		cau_write_pi_1byte(op, m_r[(op >> 16) & 0x1f] >> 8);
	else
		unimplemented(op);
}


void dsp32c_device::store_lr(UINT32 op)
{
	if (!(op & 0x400))
		cau_write_pi_1byte(op, m_r[(op >> 16) & 0x1f]);
	else
		unimplemented(op);
}


void dsp32c_device::store_r(UINT32 op)
{
	if (!(op & 0x400))
		cau_write_pi_2byte(op, REG16((op >> 16) & 0x1f));
	else
		unimplemented(op);
}


void dsp32c_device::store_er(UINT32 op)
{
	if (!(op & 0x400))
		cau_write_pi_4byte(op, REG24((op >> 16) & 0x1f));
	else
		unimplemented(op);
}


void dsp32c_device::load24(UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	UINT32 res = (op & 0xffff) + ((op >> 5) & 0xff0000);
	if (IS_WRITEABLE(dr))
		m_r[dr] = res;
}



//**************************************************************************
//  DAU FORM 1 IMPLEMENTATION
//**************************************************************************

void dsp32c_device::d1_aMpp(UINT32 op)
{
	double xval = dau_read_pi_double_1st(op >> 14, 1);
	double yval = dau_read_pi_double_2nd(op >> 7, 0, xval);
	double res = yval + DEFERRED_MULTIPLIER((op >> 26) & 7) * xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((op >> 21) & 3, res);
}


void dsp32c_device::d1_aMpm(UINT32 op)
{
	double xval = dau_read_pi_double_1st(op >> 14, 1);
	double yval = dau_read_pi_double_2nd(op >> 7, 0, xval);
	double res = yval - DEFERRED_MULTIPLIER((op >> 26) & 7) * xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((op >> 21) & 3, res);
}


void dsp32c_device::d1_aMmp(UINT32 op)
{
	double xval = dau_read_pi_double_1st(op >> 14, 1);
	double yval = dau_read_pi_double_2nd(op >> 7, 0, xval);
	double res = -yval + DEFERRED_MULTIPLIER((op >> 26) & 7) * xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((op >> 21) & 3, res);
}


void dsp32c_device::d1_aMmm(UINT32 op)
{
	double xval = dau_read_pi_double_1st(op >> 14, 1);
	double yval = dau_read_pi_double_2nd(op >> 7, 0, xval);
	double res = -yval - DEFERRED_MULTIPLIER((op >> 26) & 7) * xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((op >> 21) & 3, res);
}


void dsp32c_device::d1_0px(UINT32 op)
{
	double xval = dau_read_pi_double_1st(op >> 14, 1);
	double yval = dau_read_pi_double_2nd(op >> 7, 0, xval);
	double res = yval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((op >> 21) & 3, res);
	(void)xval;
}


void dsp32c_device::d1_0mx(UINT32 op)
{
	double xval = dau_read_pi_double_1st(op >> 14, 1);
	double yval = dau_read_pi_double_2nd(op >> 7, 0, xval);
	double res = -yval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((op >> 21) & 3, res);
	(void)xval;
}


void dsp32c_device::d1_1pp(UINT32 op)
{
	double xval = dau_read_pi_double_1st(op >> 14, 1);
	double yval = dau_read_pi_double_2nd(op >> 7, 0, xval);
	double res = yval + xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((op >> 21) & 3, res);
}


void dsp32c_device::d1_1pm(UINT32 op)
{
	double xval = dau_read_pi_double_1st(op >> 14, 1);
	double yval = dau_read_pi_double_2nd(op >> 7, 0, xval);
	double res = yval - xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((op >> 21) & 3, res);
}


void dsp32c_device::d1_1mp(UINT32 op)
{
	double xval = dau_read_pi_double_1st(op >> 14, 1);
	double yval = dau_read_pi_double_2nd(op >> 7, 0, xval);
	double res = -yval + xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((op >> 21) & 3, res);
}


void dsp32c_device::d1_1mm(UINT32 op)
{
	double xval = dau_read_pi_double_1st(op >> 14, 1);
	double yval = dau_read_pi_double_2nd(op >> 7, 0, xval);
	double res = -yval - xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((op >> 21) & 3, res);
}


void dsp32c_device::d1_aMppr(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::d1_aMpmr(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::d1_aMmpr(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::d1_aMmmr(UINT32 op)
{
	unimplemented(op);
}



//**************************************************************************
//  DAU FORM 2 IMPLEMENTATION
//**************************************************************************

void dsp32c_device::d2_aMpp(UINT32 op)
{
	double xval = dau_read_pi_double_1st(op >> 14, 1);
	double yval = dau_read_pi_double_2nd(op >> 7, 1, xval);
	double res = m_a[(op >> 26) & 7] + yval * xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, yval);
	dau_set_val_flags((op >> 21) & 3, res);
}


void dsp32c_device::d2_aMpm(UINT32 op)
{
	double xval = dau_read_pi_double_1st(op >> 14, 1);
	double yval = dau_read_pi_double_2nd(op >> 7, 1, xval);
	double res = m_a[(op >> 26) & 7] - yval * xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, yval);
	dau_set_val_flags((op >> 21) & 3, res);
}


void dsp32c_device::d2_aMmp(UINT32 op)
{
	double xval = dau_read_pi_double_1st(op >> 14, 1);
	double yval = dau_read_pi_double_2nd(op >> 7, 1, xval);
	double res = -m_a[(op >> 26) & 7] + yval * xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, yval);
	dau_set_val_flags((op >> 21) & 3, res);
}


void dsp32c_device::d2_aMmm(UINT32 op)
{
	double xval = dau_read_pi_double_1st(op >> 14, 1);
	double yval = dau_read_pi_double_2nd(op >> 7, 1, xval);
	double res = -m_a[(op >> 26) & 7] - yval * xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, yval);
	dau_set_val_flags((op >> 21) & 3, res);
}


void dsp32c_device::d2_aMppr(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::d2_aMpmr(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::d2_aMmpr(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::d2_aMmmr(UINT32 op)
{
	unimplemented(op);
}



//**************************************************************************
//  DAU FORM 3 IMPLEMENTATION
//**************************************************************************

void dsp32c_device::d3_aMpp(UINT32 op)
{
	double xval = dau_read_pi_double_1st(op >> 14, 1);
	double yval = dau_read_pi_double_2nd(op >> 7, 1, xval);
	double res = m_a[(op >> 26) & 7] + yval * xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((op >> 21) & 3, res);
}


void dsp32c_device::d3_aMpm(UINT32 op)
{
	double xval = dau_read_pi_double_1st(op >> 14, 1);
	double yval = dau_read_pi_double_2nd(op >> 7, 1, xval);
	double res = m_a[(op >> 26) & 7] - yval * xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((op >> 21) & 3, res);
}


void dsp32c_device::d3_aMmp(UINT32 op)
{
	double xval = dau_read_pi_double_1st(op >> 14, 1);
	double yval = dau_read_pi_double_2nd(op >> 7, 1, xval);
	double res = -m_a[(op >> 26) & 7] + yval * xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((op >> 21) & 3, res);
}


void dsp32c_device::d3_aMmm(UINT32 op)
{
	double xval = dau_read_pi_double_1st(op >> 14, 1);
	double yval = dau_read_pi_double_2nd(op >> 7, 1, xval);
	double res = -m_a[(op >> 26) & 7] - yval * xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((op >> 21) & 3, res);
}


void dsp32c_device::d3_aMppr(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::d3_aMpmr(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::d3_aMmpr(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::d3_aMmmr(UINT32 op)
{
	unimplemented(op);
}



//**************************************************************************
//  DAU FORM 4 IMPLEMENTATION
//**************************************************************************

void dsp32c_device::d4_pp(UINT32 op)
{
	double xval = dau_read_pi_double_1st(op >> 14, 1);
	double yval = dau_read_pi_double_2nd(op >> 7, 0, xval);
	double res = yval + xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, yval);
	dau_set_val_flags((op >> 21) & 3, res);
}


void dsp32c_device::d4_pm(UINT32 op)
{
	double xval = dau_read_pi_double_1st(op >> 14, 1);
	double yval = dau_read_pi_double_2nd(op >> 7, 0, xval);
	double res = yval - xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, yval);
	dau_set_val_flags((op >> 21) & 3, res);
}


void dsp32c_device::d4_mp(UINT32 op)
{
	double xval = dau_read_pi_double_1st(op >> 14, 1);
	double yval = dau_read_pi_double_2nd(op >> 7, 0, xval);
	double res = -yval + xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, yval);
	dau_set_val_flags((op >> 21) & 3, res);
}


void dsp32c_device::d4_mm(UINT32 op)
{
	double xval = dau_read_pi_double_1st(op >> 14, 1);
	double yval = dau_read_pi_double_2nd(op >> 7, 0, xval);
	double res = -yval - xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, yval);
	dau_set_val_flags((op >> 21) & 3, res);
}


void dsp32c_device::d4_ppr(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::d4_pmr(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::d4_mpr(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::d4_mmr(UINT32 op)
{
	unimplemented(op);
}



//**************************************************************************
//  DAU FORM 5 IMPLEMENTATION
//**************************************************************************

void dsp32c_device::d5_ic(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::d5_oc(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::d5_float(UINT32 op)
{
	double res = (double)(INT16)dau_read_pi_2bytes(op >> 7);
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((op >> 21) & 3, res);
}


void dsp32c_device::d5_int(UINT32 op)
{
	double val = dau_read_pi_double_1st(op >> 7, 0);
	int zpi = (op >> 0) & 0x7f;
	INT16 res;
	if (!(DAUC & 0x10)) val = floor(val + 0.5);
	else val = ceil(val - 0.5);
	res = (INT16)val;
	if (zpi != 7)
		dau_write_pi_2bytes(zpi, res);
	dau_set_val_noflags((op >> 21) & 3, dsp_to_double(res << 16));
}


void dsp32c_device::d5_round(UINT32 op)
{
	double res = (double)(float)dau_read_pi_double_1st(op >> 7, 0);
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((op >> 21) & 3, res);
}


void dsp32c_device::d5_ifalt(UINT32 op)
{
	int ar = (op >> 21) & 3;
	double res = m_a[ar];
	int zpi = (op >> 0) & 0x7f;
	if (NFLAG)
		res = dau_read_pi_double_1st(op >> 7, 0);
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_noflags(ar, res);
}


void dsp32c_device::d5_ifaeq(UINT32 op)
{
	int ar = (op >> 21) & 3;
	double res = m_a[ar];
	int zpi = (op >> 0) & 0x7f;
	if (ZFLAG)
		res = dau_read_pi_double_1st(op >> 7, 0);
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_noflags(ar, res);
}


void dsp32c_device::d5_ifagt(UINT32 op)
{
	int ar = (op >> 21) & 3;
	double res = m_a[ar];
	int zpi = (op >> 0) & 0x7f;
	if (!NFLAG && !ZFLAG)
		res = dau_read_pi_double_1st(op >> 7, 0);
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_noflags(ar, res);
}


void dsp32c_device::d5_float24(UINT32 op)
{
	double res = (double)((INT32)(dau_read_pi_4bytes(op >> 7) << 8) >> 8);
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((op >> 21) & 3, res);
}


void dsp32c_device::d5_int24(UINT32 op)
{
	double val = dau_read_pi_double_1st(op >> 7, 0);
	int zpi = (op >> 0) & 0x7f;
	INT32 res;
	if (!(DAUC & 0x10)) val = floor(val + 0.5);
	else val = ceil(val - 0.5);
	res = (INT32)val;
	if (res > 0x7fffff) res = 0x7fffff;
	else if (res < -0x800000) res = -0x800000;
	if (zpi != 7)
		dau_write_pi_4bytes(zpi, (INT32)(res << 8) >> 8);
	dau_set_val_noflags((op >> 21) & 3, dsp_to_double(res << 8));
}


void dsp32c_device::d5_ieee(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::d5_dsp(UINT32 op)
{
	unimplemented(op);
}


void dsp32c_device::d5_seed(UINT32 op)
{
	UINT32 val = dau_read_pi_4bytes(op >> 7);
	INT32 res = val ^ 0x7fffffff;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_4bytes(zpi, res);
	dau_set_val_flags((op >> 21) & 3, dsp_to_double((INT32)res));
}



//**************************************************************************
//  FUNCTION TABLE
//**************************************************************************

void (dsp32c_device::*const dsp32c_device::s_dsp32ops[])(UINT32 op) =
{
	&dsp32c_device::nop,        &dsp32c_device::goto_t,     &dsp32c_device::goto_pl,    &dsp32c_device::goto_mi,    &dsp32c_device::goto_ne,    &dsp32c_device::goto_eq,    &dsp32c_device::goto_vc,    &dsp32c_device::goto_vs,    // 00
	&dsp32c_device::goto_cc,    &dsp32c_device::goto_cs,    &dsp32c_device::goto_ge,    &dsp32c_device::goto_lt,    &dsp32c_device::goto_gt,    &dsp32c_device::goto_le,    &dsp32c_device::goto_hi,    &dsp32c_device::goto_ls,
	&dsp32c_device::goto_auc,   &dsp32c_device::goto_aus,   &dsp32c_device::goto_age,   &dsp32c_device::goto_alt,   &dsp32c_device::goto_ane,   &dsp32c_device::goto_aeq,   &dsp32c_device::goto_avc,   &dsp32c_device::goto_avs,   // 01
	&dsp32c_device::goto_agt,   &dsp32c_device::goto_ale,   &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,
	&dsp32c_device::goto_ibe,   &dsp32c_device::goto_ibf,   &dsp32c_device::goto_obf,   &dsp32c_device::goto_obe,   &dsp32c_device::goto_pde,   &dsp32c_device::goto_pdf,   &dsp32c_device::goto_pie,   &dsp32c_device::goto_pif,   // 02
	&dsp32c_device::goto_syc,   &dsp32c_device::goto_sys,   &dsp32c_device::goto_fbc,   &dsp32c_device::goto_fbs,   &dsp32c_device::goto_irq1lo,&dsp32c_device::goto_irq1hi,&dsp32c_device::goto_irq2lo,&dsp32c_device::goto_irq2hi,
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    // 03
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,

	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    // 04
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    // 05
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,
	&dsp32c_device::dec_goto,   &dsp32c_device::dec_goto,   &dsp32c_device::dec_goto,   &dsp32c_device::dec_goto,   &dsp32c_device::dec_goto,   &dsp32c_device::dec_goto,   &dsp32c_device::dec_goto,   &dsp32c_device::dec_goto,   // 06
	&dsp32c_device::dec_goto,   &dsp32c_device::dec_goto,   &dsp32c_device::dec_goto,   &dsp32c_device::dec_goto,   &dsp32c_device::dec_goto,   &dsp32c_device::dec_goto,   &dsp32c_device::dec_goto,   &dsp32c_device::dec_goto,
	&dsp32c_device::dec_goto,   &dsp32c_device::dec_goto,   &dsp32c_device::dec_goto,   &dsp32c_device::dec_goto,   &dsp32c_device::dec_goto,   &dsp32c_device::dec_goto,   &dsp32c_device::dec_goto,   &dsp32c_device::dec_goto,   // 07
	&dsp32c_device::dec_goto,   &dsp32c_device::dec_goto,   &dsp32c_device::dec_goto,   &dsp32c_device::dec_goto,   &dsp32c_device::dec_goto,   &dsp32c_device::dec_goto,   &dsp32c_device::dec_goto,   &dsp32c_device::dec_goto,

	&dsp32c_device::call,       &dsp32c_device::call,       &dsp32c_device::call,       &dsp32c_device::call,       &dsp32c_device::call,       &dsp32c_device::call,       &dsp32c_device::call,       &dsp32c_device::call,       // 08
	&dsp32c_device::call,       &dsp32c_device::call,       &dsp32c_device::call,       &dsp32c_device::call,       &dsp32c_device::call,       &dsp32c_device::call,       &dsp32c_device::call,       &dsp32c_device::call,
	&dsp32c_device::call,       &dsp32c_device::call,       &dsp32c_device::call,       &dsp32c_device::call,       &dsp32c_device::call,       &dsp32c_device::call,       &dsp32c_device::call,       &dsp32c_device::call,       // 09
	&dsp32c_device::call,       &dsp32c_device::call,       &dsp32c_device::call,       &dsp32c_device::call,       &dsp32c_device::call,       &dsp32c_device::call,       &dsp32c_device::call,       &dsp32c_device::call,
	&dsp32c_device::add_si,     &dsp32c_device::add_si,     &dsp32c_device::add_si,     &dsp32c_device::add_si,     &dsp32c_device::add_si,     &dsp32c_device::add_si,     &dsp32c_device::add_si,     &dsp32c_device::add_si,     // 0a
	&dsp32c_device::add_si,     &dsp32c_device::add_si,     &dsp32c_device::add_si,     &dsp32c_device::add_si,     &dsp32c_device::add_si,     &dsp32c_device::add_si,     &dsp32c_device::add_si,     &dsp32c_device::add_si,
	&dsp32c_device::add_si,     &dsp32c_device::add_si,     &dsp32c_device::add_si,     &dsp32c_device::add_si,     &dsp32c_device::add_si,     &dsp32c_device::add_si,     &dsp32c_device::add_si,     &dsp32c_device::add_si,     // 0b
	&dsp32c_device::add_si,     &dsp32c_device::add_si,     &dsp32c_device::add_si,     &dsp32c_device::add_si,     &dsp32c_device::add_si,     &dsp32c_device::add_si,     &dsp32c_device::add_si,     &dsp32c_device::add_si,

	&dsp32c_device::add_ss,     &dsp32c_device::mul2_s,     &dsp32c_device::subr_ss,    &dsp32c_device::addr_ss,    &dsp32c_device::sub_ss,     &dsp32c_device::neg_s,      &dsp32c_device::andc_ss,    &dsp32c_device::cmp_ss,     // 0c
	&dsp32c_device::xor_ss,     &dsp32c_device::rcr_s,      &dsp32c_device::or_ss,      &dsp32c_device::rcl_s,      &dsp32c_device::shr_s,      &dsp32c_device::div2_s,     &dsp32c_device::and_ss,     &dsp32c_device::test_ss,
	&dsp32c_device::add_di,     &dsp32c_device::illegal,    &dsp32c_device::subr_di,    &dsp32c_device::addr_di,    &dsp32c_device::sub_di,     &dsp32c_device::illegal,    &dsp32c_device::andc_di,    &dsp32c_device::cmp_di,     // 0d
	&dsp32c_device::xor_di,     &dsp32c_device::illegal,    &dsp32c_device::or_di,      &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::and_di,     &dsp32c_device::test_di,
	&dsp32c_device::load_hi,    &dsp32c_device::load_hi,    &dsp32c_device::load_li,    &dsp32c_device::load_li,    &dsp32c_device::load_i,     &dsp32c_device::load_i,     &dsp32c_device::load_ei,    &dsp32c_device::load_ei,    // 0e
	&dsp32c_device::store_hi,   &dsp32c_device::store_hi,   &dsp32c_device::store_li,   &dsp32c_device::store_li,   &dsp32c_device::store_i,    &dsp32c_device::store_i,    &dsp32c_device::store_ei,   &dsp32c_device::store_ei,
	&dsp32c_device::load_hr,    &dsp32c_device::load_hr,    &dsp32c_device::load_lr,    &dsp32c_device::load_lr,    &dsp32c_device::load_r,     &dsp32c_device::load_r,     &dsp32c_device::load_er,    &dsp32c_device::load_er,    // 0f
	&dsp32c_device::store_hr,   &dsp32c_device::store_hr,   &dsp32c_device::store_lr,   &dsp32c_device::store_lr,   &dsp32c_device::store_r,    &dsp32c_device::store_r,    &dsp32c_device::store_er,   &dsp32c_device::store_er,

	&dsp32c_device::d1_aMpp,    &dsp32c_device::d1_aMpp,    &dsp32c_device::d1_aMpp,    &dsp32c_device::d1_aMpp,    &dsp32c_device::d1_aMpm,    &dsp32c_device::d1_aMpm,    &dsp32c_device::d1_aMpm,    &dsp32c_device::d1_aMpm,    // 10
	&dsp32c_device::d1_aMmp,    &dsp32c_device::d1_aMmp,    &dsp32c_device::d1_aMmp,    &dsp32c_device::d1_aMmp,    &dsp32c_device::d1_aMmm,    &dsp32c_device::d1_aMmm,    &dsp32c_device::d1_aMmm,    &dsp32c_device::d1_aMmm,
	&dsp32c_device::d1_aMppr,   &dsp32c_device::d1_aMppr,   &dsp32c_device::d1_aMppr,   &dsp32c_device::d1_aMppr,   &dsp32c_device::d1_aMpmr,   &dsp32c_device::d1_aMpmr,   &dsp32c_device::d1_aMpmr,   &dsp32c_device::d1_aMpmr,   // 11
	&dsp32c_device::d1_aMmpr,   &dsp32c_device::d1_aMmpr,   &dsp32c_device::d1_aMmpr,   &dsp32c_device::d1_aMmpr,   &dsp32c_device::d1_aMmmr,   &dsp32c_device::d1_aMmmr,   &dsp32c_device::d1_aMmmr,   &dsp32c_device::d1_aMmmr,
	&dsp32c_device::d1_aMpp,    &dsp32c_device::d1_aMpp,    &dsp32c_device::d1_aMpp,    &dsp32c_device::d1_aMpp,    &dsp32c_device::d1_aMpm,    &dsp32c_device::d1_aMpm,    &dsp32c_device::d1_aMpm,    &dsp32c_device::d1_aMpm,    // 12
	&dsp32c_device::d1_aMmp,    &dsp32c_device::d1_aMmp,    &dsp32c_device::d1_aMmp,    &dsp32c_device::d1_aMmp,    &dsp32c_device::d1_aMmm,    &dsp32c_device::d1_aMmm,    &dsp32c_device::d1_aMmm,    &dsp32c_device::d1_aMmm,
	&dsp32c_device::d1_aMppr,   &dsp32c_device::d1_aMppr,   &dsp32c_device::d1_aMppr,   &dsp32c_device::d1_aMppr,   &dsp32c_device::d1_aMpmr,   &dsp32c_device::d1_aMpmr,   &dsp32c_device::d1_aMpmr,   &dsp32c_device::d1_aMpmr,   // 13
	&dsp32c_device::d1_aMmpr,   &dsp32c_device::d1_aMmpr,   &dsp32c_device::d1_aMmpr,   &dsp32c_device::d1_aMmpr,   &dsp32c_device::d1_aMmmr,   &dsp32c_device::d1_aMmmr,   &dsp32c_device::d1_aMmmr,   &dsp32c_device::d1_aMmmr,

	&dsp32c_device::d1_aMpp,    &dsp32c_device::d1_aMpp,    &dsp32c_device::d1_aMpp,    &dsp32c_device::d1_aMpp,    &dsp32c_device::d1_aMpm,    &dsp32c_device::d1_aMpm,    &dsp32c_device::d1_aMpm,    &dsp32c_device::d1_aMpm,    // 14
	&dsp32c_device::d1_aMmp,    &dsp32c_device::d1_aMmp,    &dsp32c_device::d1_aMmp,    &dsp32c_device::d1_aMmp,    &dsp32c_device::d1_aMmm,    &dsp32c_device::d1_aMmm,    &dsp32c_device::d1_aMmm,    &dsp32c_device::d1_aMmm,
	&dsp32c_device::d1_aMppr,   &dsp32c_device::d1_aMppr,   &dsp32c_device::d1_aMppr,   &dsp32c_device::d1_aMppr,   &dsp32c_device::d1_aMpmr,   &dsp32c_device::d1_aMpmr,   &dsp32c_device::d1_aMpmr,   &dsp32c_device::d1_aMpmr,   // 15
	&dsp32c_device::d1_aMmpr,   &dsp32c_device::d1_aMmpr,   &dsp32c_device::d1_aMmpr,   &dsp32c_device::d1_aMmpr,   &dsp32c_device::d1_aMmmr,   &dsp32c_device::d1_aMmmr,   &dsp32c_device::d1_aMmmr,   &dsp32c_device::d1_aMmmr,
	&dsp32c_device::d1_aMpp,    &dsp32c_device::d1_aMpp,    &dsp32c_device::d1_aMpp,    &dsp32c_device::d1_aMpp,    &dsp32c_device::d1_aMpm,    &dsp32c_device::d1_aMpm,    &dsp32c_device::d1_aMpm,    &dsp32c_device::d1_aMpm,    // 16
	&dsp32c_device::d1_aMmp,    &dsp32c_device::d1_aMmp,    &dsp32c_device::d1_aMmp,    &dsp32c_device::d1_aMmp,    &dsp32c_device::d1_aMmm,    &dsp32c_device::d1_aMmm,    &dsp32c_device::d1_aMmm,    &dsp32c_device::d1_aMmm,
	&dsp32c_device::d1_aMppr,   &dsp32c_device::d1_aMppr,   &dsp32c_device::d1_aMppr,   &dsp32c_device::d1_aMppr,   &dsp32c_device::d1_aMpmr,   &dsp32c_device::d1_aMpmr,   &dsp32c_device::d1_aMpmr,   &dsp32c_device::d1_aMpmr,   // 17
	&dsp32c_device::d1_aMmpr,   &dsp32c_device::d1_aMmpr,   &dsp32c_device::d1_aMmpr,   &dsp32c_device::d1_aMmpr,   &dsp32c_device::d1_aMmmr,   &dsp32c_device::d1_aMmmr,   &dsp32c_device::d1_aMmmr,   &dsp32c_device::d1_aMmmr,

	&dsp32c_device::d1_0px,     &dsp32c_device::d1_0px,     &dsp32c_device::d1_0px,     &dsp32c_device::d1_0px,     &dsp32c_device::d1_0px,     &dsp32c_device::d1_0px,     &dsp32c_device::d1_0px,     &dsp32c_device::d1_0px,     // 18
	&dsp32c_device::d1_0mx,     &dsp32c_device::d1_0mx,     &dsp32c_device::d1_0mx,     &dsp32c_device::d1_0mx,     &dsp32c_device::d1_0mx,     &dsp32c_device::d1_0mx,     &dsp32c_device::d1_0mx,     &dsp32c_device::d1_0mx,
	&dsp32c_device::d1_aMppr,   &dsp32c_device::d1_aMppr,   &dsp32c_device::d1_aMppr,   &dsp32c_device::d1_aMppr,   &dsp32c_device::d1_aMpmr,   &dsp32c_device::d1_aMpmr,   &dsp32c_device::d1_aMpmr,   &dsp32c_device::d1_aMpmr,   // 19
	&dsp32c_device::d1_aMmpr,   &dsp32c_device::d1_aMmpr,   &dsp32c_device::d1_aMmpr,   &dsp32c_device::d1_aMmpr,   &dsp32c_device::d1_aMmmr,   &dsp32c_device::d1_aMmmr,   &dsp32c_device::d1_aMmmr,   &dsp32c_device::d1_aMmmr,
	&dsp32c_device::d1_1pp,     &dsp32c_device::d1_1pp,     &dsp32c_device::d1_1pp,     &dsp32c_device::d1_1pp,     &dsp32c_device::d1_1pm,     &dsp32c_device::d1_1pm,     &dsp32c_device::d1_1pm,     &dsp32c_device::d1_1pm,     // 1a
	&dsp32c_device::d1_1mp,     &dsp32c_device::d1_1mp,     &dsp32c_device::d1_1mp,     &dsp32c_device::d1_1mp,     &dsp32c_device::d1_1mm,     &dsp32c_device::d1_1mm,     &dsp32c_device::d1_1mm,     &dsp32c_device::d1_1mm,
	&dsp32c_device::d1_aMppr,   &dsp32c_device::d1_aMppr,   &dsp32c_device::d1_aMppr,   &dsp32c_device::d1_aMppr,   &dsp32c_device::d1_aMpmr,   &dsp32c_device::d1_aMpmr,   &dsp32c_device::d1_aMpmr,   &dsp32c_device::d1_aMpmr,   // 1b
	&dsp32c_device::d1_aMmpr,   &dsp32c_device::d1_aMmpr,   &dsp32c_device::d1_aMmpr,   &dsp32c_device::d1_aMmpr,   &dsp32c_device::d1_aMmmr,   &dsp32c_device::d1_aMmmr,   &dsp32c_device::d1_aMmmr,   &dsp32c_device::d1_aMmmr,

	&dsp32c_device::d4_pp,      &dsp32c_device::d4_pp,      &dsp32c_device::d4_pp,      &dsp32c_device::d4_pp,      &dsp32c_device::d4_pm,      &dsp32c_device::d4_pm,      &dsp32c_device::d4_pm,      &dsp32c_device::d4_pm,      // 1c
	&dsp32c_device::d4_mp,      &dsp32c_device::d4_mp,      &dsp32c_device::d4_mp,      &dsp32c_device::d4_mp,      &dsp32c_device::d4_mm,      &dsp32c_device::d4_mm,      &dsp32c_device::d4_mm,      &dsp32c_device::d4_mm,
	&dsp32c_device::d4_ppr,     &dsp32c_device::d4_ppr,     &dsp32c_device::d4_ppr,     &dsp32c_device::d4_ppr,     &dsp32c_device::d4_pmr,     &dsp32c_device::d4_pmr,     &dsp32c_device::d4_pmr,     &dsp32c_device::d4_pmr,     // 1d
	&dsp32c_device::d4_mpr,     &dsp32c_device::d4_mpr,     &dsp32c_device::d4_mpr,     &dsp32c_device::d4_mpr,     &dsp32c_device::d4_mmr,     &dsp32c_device::d4_mmr,     &dsp32c_device::d4_mmr,     &dsp32c_device::d4_mmr,
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    // 1e
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    // 1f
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,

	&dsp32c_device::d2_aMpp,    &dsp32c_device::d2_aMpp,    &dsp32c_device::d2_aMpp,    &dsp32c_device::d2_aMpp,    &dsp32c_device::d2_aMpm,    &dsp32c_device::d2_aMpm,    &dsp32c_device::d2_aMpm,    &dsp32c_device::d2_aMpm,    // 20
	&dsp32c_device::d2_aMmp,    &dsp32c_device::d2_aMmp,    &dsp32c_device::d2_aMmp,    &dsp32c_device::d2_aMmp,    &dsp32c_device::d2_aMmm,    &dsp32c_device::d2_aMmm,    &dsp32c_device::d2_aMmm,    &dsp32c_device::d2_aMmm,
	&dsp32c_device::d2_aMppr,   &dsp32c_device::d2_aMppr,   &dsp32c_device::d2_aMppr,   &dsp32c_device::d2_aMppr,   &dsp32c_device::d2_aMpmr,   &dsp32c_device::d2_aMpmr,   &dsp32c_device::d2_aMpmr,   &dsp32c_device::d2_aMpmr,   // 21
	&dsp32c_device::d2_aMmpr,   &dsp32c_device::d2_aMmpr,   &dsp32c_device::d2_aMmpr,   &dsp32c_device::d2_aMmpr,   &dsp32c_device::d2_aMmmr,   &dsp32c_device::d2_aMmmr,   &dsp32c_device::d2_aMmmr,   &dsp32c_device::d2_aMmmr,
	&dsp32c_device::d2_aMpp,    &dsp32c_device::d2_aMpp,    &dsp32c_device::d2_aMpp,    &dsp32c_device::d2_aMpp,    &dsp32c_device::d2_aMpm,    &dsp32c_device::d2_aMpm,    &dsp32c_device::d2_aMpm,    &dsp32c_device::d2_aMpm,    // 22
	&dsp32c_device::d2_aMmp,    &dsp32c_device::d2_aMmp,    &dsp32c_device::d2_aMmp,    &dsp32c_device::d2_aMmp,    &dsp32c_device::d2_aMmm,    &dsp32c_device::d2_aMmm,    &dsp32c_device::d2_aMmm,    &dsp32c_device::d2_aMmm,
	&dsp32c_device::d2_aMppr,   &dsp32c_device::d2_aMppr,   &dsp32c_device::d2_aMppr,   &dsp32c_device::d2_aMppr,   &dsp32c_device::d2_aMpmr,   &dsp32c_device::d2_aMpmr,   &dsp32c_device::d2_aMpmr,   &dsp32c_device::d2_aMpmr,   // 23
	&dsp32c_device::d2_aMmpr,   &dsp32c_device::d2_aMmpr,   &dsp32c_device::d2_aMmpr,   &dsp32c_device::d2_aMmpr,   &dsp32c_device::d2_aMmmr,   &dsp32c_device::d2_aMmmr,   &dsp32c_device::d2_aMmmr,   &dsp32c_device::d2_aMmmr,

	&dsp32c_device::d2_aMpp,    &dsp32c_device::d2_aMpp,    &dsp32c_device::d2_aMpp,    &dsp32c_device::d2_aMpp,    &dsp32c_device::d2_aMpm,    &dsp32c_device::d2_aMpm,    &dsp32c_device::d2_aMpm,    &dsp32c_device::d2_aMpm,    // 24
	&dsp32c_device::d2_aMmp,    &dsp32c_device::d2_aMmp,    &dsp32c_device::d2_aMmp,    &dsp32c_device::d2_aMmp,    &dsp32c_device::d2_aMmm,    &dsp32c_device::d2_aMmm,    &dsp32c_device::d2_aMmm,    &dsp32c_device::d2_aMmm,
	&dsp32c_device::d2_aMppr,   &dsp32c_device::d2_aMppr,   &dsp32c_device::d2_aMppr,   &dsp32c_device::d2_aMppr,   &dsp32c_device::d2_aMpmr,   &dsp32c_device::d2_aMpmr,   &dsp32c_device::d2_aMpmr,   &dsp32c_device::d2_aMpmr,   // 25
	&dsp32c_device::d2_aMmpr,   &dsp32c_device::d2_aMmpr,   &dsp32c_device::d2_aMmpr,   &dsp32c_device::d2_aMmpr,   &dsp32c_device::d2_aMmmr,   &dsp32c_device::d2_aMmmr,   &dsp32c_device::d2_aMmmr,   &dsp32c_device::d2_aMmmr,
	&dsp32c_device::d2_aMpp,    &dsp32c_device::d2_aMpp,    &dsp32c_device::d2_aMpp,    &dsp32c_device::d2_aMpp,    &dsp32c_device::d2_aMpm,    &dsp32c_device::d2_aMpm,    &dsp32c_device::d2_aMpm,    &dsp32c_device::d2_aMpm,    // 26
	&dsp32c_device::d2_aMmp,    &dsp32c_device::d2_aMmp,    &dsp32c_device::d2_aMmp,    &dsp32c_device::d2_aMmp,    &dsp32c_device::d2_aMmm,    &dsp32c_device::d2_aMmm,    &dsp32c_device::d2_aMmm,    &dsp32c_device::d2_aMmm,
	&dsp32c_device::d2_aMppr,   &dsp32c_device::d2_aMppr,   &dsp32c_device::d2_aMppr,   &dsp32c_device::d2_aMppr,   &dsp32c_device::d2_aMpmr,   &dsp32c_device::d2_aMpmr,   &dsp32c_device::d2_aMpmr,   &dsp32c_device::d2_aMpmr,   // 27
	&dsp32c_device::d2_aMmpr,   &dsp32c_device::d2_aMmpr,   &dsp32c_device::d2_aMmpr,   &dsp32c_device::d2_aMmpr,   &dsp32c_device::d2_aMmmr,   &dsp32c_device::d2_aMmmr,   &dsp32c_device::d2_aMmmr,   &dsp32c_device::d2_aMmmr,

	&dsp32c_device::d2_aMpp,    &dsp32c_device::d2_aMpp,    &dsp32c_device::d2_aMpp,    &dsp32c_device::d2_aMpp,    &dsp32c_device::d2_aMpm,    &dsp32c_device::d2_aMpm,    &dsp32c_device::d2_aMpm,    &dsp32c_device::d2_aMpm,    // 28
	&dsp32c_device::d2_aMmp,    &dsp32c_device::d2_aMmp,    &dsp32c_device::d2_aMmp,    &dsp32c_device::d2_aMmp,    &dsp32c_device::d2_aMmm,    &dsp32c_device::d2_aMmm,    &dsp32c_device::d2_aMmm,    &dsp32c_device::d2_aMmm,
	&dsp32c_device::d2_aMppr,   &dsp32c_device::d2_aMppr,   &dsp32c_device::d2_aMppr,   &dsp32c_device::d2_aMppr,   &dsp32c_device::d2_aMpmr,   &dsp32c_device::d2_aMpmr,   &dsp32c_device::d2_aMpmr,   &dsp32c_device::d2_aMpmr,   // 29
	&dsp32c_device::d2_aMmpr,   &dsp32c_device::d2_aMmpr,   &dsp32c_device::d2_aMmpr,   &dsp32c_device::d2_aMmpr,   &dsp32c_device::d2_aMmmr,   &dsp32c_device::d2_aMmmr,   &dsp32c_device::d2_aMmmr,   &dsp32c_device::d2_aMmmr,
	&dsp32c_device::d2_aMpp,    &dsp32c_device::d2_aMpp,    &dsp32c_device::d2_aMpp,    &dsp32c_device::d2_aMpp,    &dsp32c_device::d2_aMpm,    &dsp32c_device::d2_aMpm,    &dsp32c_device::d2_aMpm,    &dsp32c_device::d2_aMpm,    // 2a
	&dsp32c_device::d2_aMmp,    &dsp32c_device::d2_aMmp,    &dsp32c_device::d2_aMmp,    &dsp32c_device::d2_aMmp,    &dsp32c_device::d2_aMmm,    &dsp32c_device::d2_aMmm,    &dsp32c_device::d2_aMmm,    &dsp32c_device::d2_aMmm,
	&dsp32c_device::d2_aMppr,   &dsp32c_device::d2_aMppr,   &dsp32c_device::d2_aMppr,   &dsp32c_device::d2_aMppr,   &dsp32c_device::d2_aMpmr,   &dsp32c_device::d2_aMpmr,   &dsp32c_device::d2_aMpmr,   &dsp32c_device::d2_aMpmr,   // 2b
	&dsp32c_device::d2_aMmpr,   &dsp32c_device::d2_aMmpr,   &dsp32c_device::d2_aMmpr,   &dsp32c_device::d2_aMmpr,   &dsp32c_device::d2_aMmmr,   &dsp32c_device::d2_aMmmr,   &dsp32c_device::d2_aMmmr,   &dsp32c_device::d2_aMmmr,

	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    // 2c
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    // 2d
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    // 2e
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    // 2f
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,

	&dsp32c_device::d3_aMpp,    &dsp32c_device::d3_aMpp,    &dsp32c_device::d3_aMpp,    &dsp32c_device::d3_aMpp,    &dsp32c_device::d3_aMpm,    &dsp32c_device::d3_aMpm,    &dsp32c_device::d3_aMpm,    &dsp32c_device::d3_aMpm,    // 30
	&dsp32c_device::d3_aMmp,    &dsp32c_device::d3_aMmp,    &dsp32c_device::d3_aMmp,    &dsp32c_device::d3_aMmp,    &dsp32c_device::d3_aMmm,    &dsp32c_device::d3_aMmm,    &dsp32c_device::d3_aMmm,    &dsp32c_device::d3_aMmm,
	&dsp32c_device::d3_aMppr,   &dsp32c_device::d3_aMppr,   &dsp32c_device::d3_aMppr,   &dsp32c_device::d3_aMppr,   &dsp32c_device::d3_aMpmr,   &dsp32c_device::d3_aMpmr,   &dsp32c_device::d3_aMpmr,   &dsp32c_device::d3_aMpmr,   // 31
	&dsp32c_device::d3_aMmpr,   &dsp32c_device::d3_aMmpr,   &dsp32c_device::d3_aMmpr,   &dsp32c_device::d3_aMmpr,   &dsp32c_device::d3_aMmmr,   &dsp32c_device::d3_aMmmr,   &dsp32c_device::d3_aMmmr,   &dsp32c_device::d3_aMmmr,
	&dsp32c_device::d3_aMpp,    &dsp32c_device::d3_aMpp,    &dsp32c_device::d3_aMpp,    &dsp32c_device::d3_aMpp,    &dsp32c_device::d3_aMpm,    &dsp32c_device::d3_aMpm,    &dsp32c_device::d3_aMpm,    &dsp32c_device::d3_aMpm,    // 32
	&dsp32c_device::d3_aMmp,    &dsp32c_device::d3_aMmp,    &dsp32c_device::d3_aMmp,    &dsp32c_device::d3_aMmp,    &dsp32c_device::d3_aMmm,    &dsp32c_device::d3_aMmm,    &dsp32c_device::d3_aMmm,    &dsp32c_device::d3_aMmm,
	&dsp32c_device::d3_aMppr,   &dsp32c_device::d3_aMppr,   &dsp32c_device::d3_aMppr,   &dsp32c_device::d3_aMppr,   &dsp32c_device::d3_aMpmr,   &dsp32c_device::d3_aMpmr,   &dsp32c_device::d3_aMpmr,   &dsp32c_device::d3_aMpmr,   // 33
	&dsp32c_device::d3_aMmpr,   &dsp32c_device::d3_aMmpr,   &dsp32c_device::d3_aMmpr,   &dsp32c_device::d3_aMmpr,   &dsp32c_device::d3_aMmmr,   &dsp32c_device::d3_aMmmr,   &dsp32c_device::d3_aMmmr,   &dsp32c_device::d3_aMmmr,

	&dsp32c_device::d3_aMpp,    &dsp32c_device::d3_aMpp,    &dsp32c_device::d3_aMpp,    &dsp32c_device::d3_aMpp,    &dsp32c_device::d3_aMpm,    &dsp32c_device::d3_aMpm,    &dsp32c_device::d3_aMpm,    &dsp32c_device::d3_aMpm,    // 34
	&dsp32c_device::d3_aMmp,    &dsp32c_device::d3_aMmp,    &dsp32c_device::d3_aMmp,    &dsp32c_device::d3_aMmp,    &dsp32c_device::d3_aMmm,    &dsp32c_device::d3_aMmm,    &dsp32c_device::d3_aMmm,    &dsp32c_device::d3_aMmm,
	&dsp32c_device::d3_aMppr,   &dsp32c_device::d3_aMppr,   &dsp32c_device::d3_aMppr,   &dsp32c_device::d3_aMppr,   &dsp32c_device::d3_aMpmr,   &dsp32c_device::d3_aMpmr,   &dsp32c_device::d3_aMpmr,   &dsp32c_device::d3_aMpmr,   // 35
	&dsp32c_device::d3_aMmpr,   &dsp32c_device::d3_aMmpr,   &dsp32c_device::d3_aMmpr,   &dsp32c_device::d3_aMmpr,   &dsp32c_device::d3_aMmmr,   &dsp32c_device::d3_aMmmr,   &dsp32c_device::d3_aMmmr,   &dsp32c_device::d3_aMmmr,
	&dsp32c_device::d3_aMpp,    &dsp32c_device::d3_aMpp,    &dsp32c_device::d3_aMpp,    &dsp32c_device::d3_aMpp,    &dsp32c_device::d3_aMpm,    &dsp32c_device::d3_aMpm,    &dsp32c_device::d3_aMpm,    &dsp32c_device::d3_aMpm,    // 36
	&dsp32c_device::d3_aMmp,    &dsp32c_device::d3_aMmp,    &dsp32c_device::d3_aMmp,    &dsp32c_device::d3_aMmp,    &dsp32c_device::d3_aMmm,    &dsp32c_device::d3_aMmm,    &dsp32c_device::d3_aMmm,    &dsp32c_device::d3_aMmm,
	&dsp32c_device::d3_aMppr,   &dsp32c_device::d3_aMppr,   &dsp32c_device::d3_aMppr,   &dsp32c_device::d3_aMppr,   &dsp32c_device::d3_aMpmr,   &dsp32c_device::d3_aMpmr,   &dsp32c_device::d3_aMpmr,   &dsp32c_device::d3_aMpmr,   // 37
	&dsp32c_device::d3_aMmpr,   &dsp32c_device::d3_aMmpr,   &dsp32c_device::d3_aMmpr,   &dsp32c_device::d3_aMmpr,   &dsp32c_device::d3_aMmmr,   &dsp32c_device::d3_aMmmr,   &dsp32c_device::d3_aMmmr,   &dsp32c_device::d3_aMmmr,

	&dsp32c_device::d3_aMpp,    &dsp32c_device::d3_aMpp,    &dsp32c_device::d3_aMpp,    &dsp32c_device::d3_aMpp,    &dsp32c_device::d3_aMpm,    &dsp32c_device::d3_aMpm,    &dsp32c_device::d3_aMpm,    &dsp32c_device::d3_aMpm,    // 38
	&dsp32c_device::d3_aMmp,    &dsp32c_device::d3_aMmp,    &dsp32c_device::d3_aMmp,    &dsp32c_device::d3_aMmp,    &dsp32c_device::d3_aMmm,    &dsp32c_device::d3_aMmm,    &dsp32c_device::d3_aMmm,    &dsp32c_device::d3_aMmm,
	&dsp32c_device::d3_aMppr,   &dsp32c_device::d3_aMppr,   &dsp32c_device::d3_aMppr,   &dsp32c_device::d3_aMppr,   &dsp32c_device::d3_aMpmr,   &dsp32c_device::d3_aMpmr,   &dsp32c_device::d3_aMpmr,   &dsp32c_device::d3_aMpmr,   // 39
	&dsp32c_device::d3_aMmpr,   &dsp32c_device::d3_aMmpr,   &dsp32c_device::d3_aMmpr,   &dsp32c_device::d3_aMmpr,   &dsp32c_device::d3_aMmmr,   &dsp32c_device::d3_aMmmr,   &dsp32c_device::d3_aMmmr,   &dsp32c_device::d3_aMmmr,
	&dsp32c_device::d3_aMpp,    &dsp32c_device::d3_aMpp,    &dsp32c_device::d3_aMpp,    &dsp32c_device::d3_aMpp,    &dsp32c_device::d3_aMpm,    &dsp32c_device::d3_aMpm,    &dsp32c_device::d3_aMpm,    &dsp32c_device::d3_aMpm,    // 3a
	&dsp32c_device::d3_aMmp,    &dsp32c_device::d3_aMmp,    &dsp32c_device::d3_aMmp,    &dsp32c_device::d3_aMmp,    &dsp32c_device::d3_aMmm,    &dsp32c_device::d3_aMmm,    &dsp32c_device::d3_aMmm,    &dsp32c_device::d3_aMmm,
	&dsp32c_device::d3_aMppr,   &dsp32c_device::d3_aMppr,   &dsp32c_device::d3_aMppr,   &dsp32c_device::d3_aMppr,   &dsp32c_device::d3_aMpmr,   &dsp32c_device::d3_aMpmr,   &dsp32c_device::d3_aMpmr,   &dsp32c_device::d3_aMpmr,   // 3b
	&dsp32c_device::d3_aMmpr,   &dsp32c_device::d3_aMmpr,   &dsp32c_device::d3_aMmpr,   &dsp32c_device::d3_aMmpr,   &dsp32c_device::d3_aMmmr,   &dsp32c_device::d3_aMmmr,   &dsp32c_device::d3_aMmmr,   &dsp32c_device::d3_aMmmr,

	&dsp32c_device::d5_ic,      &dsp32c_device::d5_ic,      &dsp32c_device::d5_ic,      &dsp32c_device::d5_ic,      &dsp32c_device::d5_oc,      &dsp32c_device::d5_oc,      &dsp32c_device::d5_oc,      &dsp32c_device::d5_oc,      // 3c
	&dsp32c_device::d5_float,   &dsp32c_device::d5_float,   &dsp32c_device::d5_float,   &dsp32c_device::d5_float,   &dsp32c_device::d5_int,     &dsp32c_device::d5_int,     &dsp32c_device::d5_int,     &dsp32c_device::d5_int,
	&dsp32c_device::d5_round,   &dsp32c_device::d5_round,   &dsp32c_device::d5_round,   &dsp32c_device::d5_round,   &dsp32c_device::d5_ifalt,   &dsp32c_device::d5_ifalt,   &dsp32c_device::d5_ifalt,   &dsp32c_device::d5_ifalt,   // 3d
	&dsp32c_device::d5_ifaeq,   &dsp32c_device::d5_ifaeq,   &dsp32c_device::d5_ifaeq,   &dsp32c_device::d5_ifaeq,   &dsp32c_device::d5_ifagt,   &dsp32c_device::d5_ifagt,   &dsp32c_device::d5_ifagt,   &dsp32c_device::d5_ifagt,
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    // 3e
	&dsp32c_device::d5_float24, &dsp32c_device::d5_float24, &dsp32c_device::d5_float24, &dsp32c_device::d5_float24, &dsp32c_device::d5_int24,   &dsp32c_device::d5_int24,   &dsp32c_device::d5_int24,   &dsp32c_device::d5_int24,
	&dsp32c_device::d5_ieee,    &dsp32c_device::d5_ieee,    &dsp32c_device::d5_ieee,    &dsp32c_device::d5_ieee,    &dsp32c_device::d5_dsp,     &dsp32c_device::d5_dsp,     &dsp32c_device::d5_dsp,     &dsp32c_device::d5_dsp,     // 3f
	&dsp32c_device::d5_seed,    &dsp32c_device::d5_seed,    &dsp32c_device::d5_seed,    &dsp32c_device::d5_seed,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,

	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    // 40
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    // 41
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    // 42
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    // 43
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,

	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    // 44
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    // 45
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,
	&dsp32c_device::do_i,       &dsp32c_device::do_r,       &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    // 46
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    // 47
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,

	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    // 48
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    // 49
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,
	&dsp32c_device::adde_si,    &dsp32c_device::adde_si,    &dsp32c_device::adde_si,    &dsp32c_device::adde_si,    &dsp32c_device::adde_si,    &dsp32c_device::adde_si,    &dsp32c_device::adde_si,    &dsp32c_device::adde_si,    // 4a
	&dsp32c_device::adde_si,    &dsp32c_device::adde_si,    &dsp32c_device::adde_si,    &dsp32c_device::adde_si,    &dsp32c_device::adde_si,    &dsp32c_device::adde_si,    &dsp32c_device::adde_si,    &dsp32c_device::adde_si,
	&dsp32c_device::adde_si,    &dsp32c_device::adde_si,    &dsp32c_device::adde_si,    &dsp32c_device::adde_si,    &dsp32c_device::adde_si,    &dsp32c_device::adde_si,    &dsp32c_device::adde_si,    &dsp32c_device::adde_si,    // 4b
	&dsp32c_device::adde_si,    &dsp32c_device::adde_si,    &dsp32c_device::adde_si,    &dsp32c_device::adde_si,    &dsp32c_device::adde_si,    &dsp32c_device::adde_si,    &dsp32c_device::adde_si,    &dsp32c_device::adde_si,

	&dsp32c_device::adde_ss,    &dsp32c_device::mul2e_s,    &dsp32c_device::subre_ss,   &dsp32c_device::addre_ss,   &dsp32c_device::sube_ss,    &dsp32c_device::nege_s,     &dsp32c_device::andce_ss,   &dsp32c_device::cmpe_ss,    // 4c
	&dsp32c_device::xore_ss,    &dsp32c_device::rcre_s,     &dsp32c_device::ore_ss,     &dsp32c_device::rcle_s,     &dsp32c_device::shre_s,     &dsp32c_device::div2e_s,    &dsp32c_device::ande_ss,    &dsp32c_device::teste_ss,
	&dsp32c_device::adde_di,    &dsp32c_device::illegal,    &dsp32c_device::subre_di,   &dsp32c_device::addre_di,   &dsp32c_device::sube_di,    &dsp32c_device::illegal,    &dsp32c_device::andce_di,   &dsp32c_device::cmpe_di,    // 4d
	&dsp32c_device::xore_di,    &dsp32c_device::illegal,    &dsp32c_device::ore_di,     &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::ande_di,    &dsp32c_device::teste_di,
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    // 4e
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    // 4f
	&dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,    &dsp32c_device::illegal,

	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     // 50
	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,
	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     // 51
	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,
	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     // 52
	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,
	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     // 53
	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,

	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     // 54
	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,
	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     // 55
	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,
	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     // 56
	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,
	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     // 57
	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,

	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     // 58
	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,
	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     // 59
	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,
	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     // 5a
	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,
	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     // 5b
	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,

	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     // 5c
	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,
	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     // 5d
	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,
	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     // 5e
	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,
	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     // 5f
	&dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,     &dsp32c_device::goto24,

	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     // 60
	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,
	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     // 61
	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,
	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     // 62
	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,
	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     // 63
	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,

	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     // 64
	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,
	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     // 65
	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,
	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     // 66
	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,
	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     // 67
	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,

	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     // 68
	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,
	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     // 69
	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,
	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     // 6a
	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,
	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     // 6b
	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,

	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     // 6c
	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,
	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     // 6d
	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,
	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     // 6e
	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,
	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     // 6f
	&dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,     &dsp32c_device::load24,

	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     // 70
	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,
	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     // 71
	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,
	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     // 72
	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,
	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     // 73
	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,

	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     // 74
	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,
	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     // 75
	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,
	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     // 76
	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,
	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     // 77
	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,

	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     // 78
	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,
	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     // 79
	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,
	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     // 7a
	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,
	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     // 7b
	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,

	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     // 7c
	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,
	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     // 7d
	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,
	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     // 7e
	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,
	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     // 7f
	&dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24,     &dsp32c_device::call24
};


/*

    Most common OPs in Race Drivin':

301681217 - op   0 - nop
164890391 - op 4A1 - adde_si
99210113 - op 661 - load24
86010010 - op  F7 - load_er
61148739 - op 4D4 - sube_di
52693763 - op 180 - d1_0px
41525754 - op  FF - store_er
35033321 - op 380 - d3_aMpp
31621151 - op 4C0 - adde_ss
28076244 - op 660 - load24
19190505 - op 4C1 - mul2e_s
13270852 - op  F5 - load_r
12535169 - op 1A4 - d1_1pm
12265141 - op 4C4 - sube_ss
10748211 - op 4CD - div2e_s
10493660 - op  FD - store_r
 9721263 - op 189
 9415685 - op 3C8
 9294148 - op 3D5
 8887846 - op 1A1
 8788648 - op 381
 8185239 - op 300
 7241256 - op 383
 6877349 - op 4A3
 6832295 - op 181
 6601270 - op 3E8
 6562483 - op 4A4
 6553514 - op 3C9
 6270430 - op 280
 6041485 - op 1A0
 5299529 - op 304
 5110926 - op 382
 4922253 - op 363
 4603670 - op 4D7
 4164327 - op 4AE
 3980085 - op 3EC
 3599198 - op 3CC
 3543878 - op 3D0
 3489158 - op   4
 3463235 - op 321
 3335995 - op 3F9
 3001546 - op 4CE
 2882940 - op 129
 2882940 - op 1A5
 2882940 - op 342
 2841981 - op 360
 2663417 - op  FB
 2059640 - op 3ED
 1867166 - op 1A8
 1830789 - op 305
 1753312 - op 301
 1726866 - op   5
 1594991 - op  12
 1571286 - op  19
 1507644 - op  A2
 1418846 - op 3CD
 1273134 - op  F3
 1177914 - op 4C7
 1175720 - op 188
 1091848 - op 3E9
 1088206 - op 6FF
 1088204 - op 4CA
 1012639 - op 101
  939617 - op 4C5

*/
