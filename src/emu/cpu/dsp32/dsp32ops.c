/***************************************************************************

    dsp32ops.c
    Core implementation for the portable DSP32 emulator.
    Written by Aaron Giles

***************************************************************************/



/***************************************************************************
    COMPILE-TIME OPTIONS
***************************************************************************/

/* these defined latencies are a pain to implement, but are necessary */
#define EMULATE_MEMORY_LATENCY		(1)
#define EMULATE_MULTIPLIER_LATENCY	(1)
#define EMULATE_AFLAGS_LATENCY		(1)

/* these optimizations should have some effect, but they don't really, so */
/* leave them off */
#define IGNORE_DAU_UV_FLAGS			(0)
#define ASSUME_WRITEABLE			(0)
#define ASSUME_UNCONDITIONAL_CAU	(0)



/***************************************************************************
    MACROS
***************************************************************************/

#define SET_V_16(a,b,r)		dsp32.vflags = (((a) ^ (b) ^ (r) ^ ((r) >> 1)) << 8)
#define SET_NZC_16(r)		dsp32.nzcflags = ((r) << 8)
#define SET_NZCV_16(a,b,r)	SET_NZC_16(r); SET_V_16(a,b,r)
#define SET_NZ00_16(r)		dsp32.nzcflags = (((r) << 8) & 0xffffff); dsp32.vflags = 0

#define SET_V_24(a,b,r)		dsp32.vflags = ((a) ^ (b) ^ (r) ^ ((r) >> 1))
#define SET_NZC_24(r)		dsp32.nzcflags = (r)
#define SET_NZCV_24(a,b,r)	SET_NZC_24(r); SET_V_24(a,b,r)
#define SET_NZ00_24(r)		dsp32.nzcflags = ((r) & 0xffffff); dsp32.vflags = 0

#define TRUNCATE24(a)		((a) & 0xffffff)
#define EXTEND16_TO_24(a)	TRUNCATE24((INT32)(INT16)(a))
#define REG16(a)			((UINT16)dsp32.r[a])
#define REG24(a)			(dsp32.r[a])

#define WRITEABLE_REGS		(0x6f3efffe)
#if ASSUME_WRITEABLE
#define IS_WRITEABLE(r) 	(1)
#else
#define IS_WRITEABLE(r)		(WRITEABLE_REGS & (1 << (r)))
#endif

#if ASSUME_UNCONDITIONAL_CAU
#define CONDITION_IS_TRUE	(1)
#else
#define CONDITION_IS_TRUE	(!(OP & 0x400) || (condition((OP >> 12) & 15)))
#endif

#if EMULATE_MEMORY_LATENCY
#define WWORD_DEFERRED(a,v)	do { int bufidx = dsp32.mbuf_index & 3; dsp32.mbufaddr[bufidx] = -(a); dsp32.mbufdata[bufidx] = (v); } while (0)
#define WLONG_DEFERRED(a,v)	do { int bufidx = dsp32.mbuf_index & 3; dsp32.mbufaddr[bufidx] = (a); dsp32.mbufdata[bufidx] = (v); } while (0)
#define PROCESS_DEFERRED_MEMORY() 									\
	if (dsp32.mbufaddr[++dsp32.mbuf_index & 3] != 1) 				\
	{																\
		int bufidx = dsp32.mbuf_index & 3;							\
		if (dsp32.mbufaddr[bufidx] >= 0)							\
			WLONG(dsp32.mbufaddr[bufidx], dsp32.mbufdata[bufidx]);	\
		else														\
			WWORD(-dsp32.mbufaddr[bufidx], dsp32.mbufdata[bufidx]);	\
		dsp32.mbufaddr[bufidx] = 1;									\
	}
#else
#define WWORD_DEFERRED(a,v)	WWORD(a,v)
#define WLONG_DEFERRED(a,v)	WLONG(a,v)
#define PROCESS_DEFERRED_MEMORY()
#endif

#if EMULATE_MULTIPLIER_LATENCY
#define DEFERRED_MULTIPLIER(x)	dau_get_amult(x)
#else
#define DEFERRED_MULTIPLIER(x)	dsp32.a[x]
#endif

#if EMULATE_AFLAGS_LATENCY
#define DEFERRED_NZFLAGS		dau_get_anzflags()
#define DEFERRED_VUFLAGS		dau_get_avuflags()
#else
#define DEFERRED_NZFLAGS		dsp32.NZflags
#define DEFERRED_VUFLAGS		dsp32.VUflags
#endif



/***************************************************************************
    FORWARD DECLARATIONS
***************************************************************************/

extern void (*dsp32ops[])(void);



/***************************************************************************
    TYPEDEFS
***************************************************************************/

typedef union int_double
{
	double d;
	UINT32 i[2];
} int_double;



/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

static void illegal(void)
{
}


static void unimplemented(void)
{
    fatalerror("Unimplemented op @ %06X: %08X (dis=%02X, tbl=%03X)", dsp32.PC - 4, OP, OP >> 25, OP >> 21);
}


INLINE void execute_one(void)
{
	PROCESS_DEFERRED_MEMORY();
	CALL_MAME_DEBUG;
	OP = ROPCODE(dsp32.PC);
	dsp32_icount -= 4;	/* 4 clocks per cycle */
	dsp32.PC += 4;
	if (OP)
		(*dsp32ops[OP >> 21])();
}



/***************************************************************************
    CAU HELPERS
***************************************************************************/

static UINT32 cau_read_pi_special(UINT8 i)
{
	switch (i)
	{
		case 4:		return dsp32.ibuf;
		case 5:		return dsp32.obuf;
		case 6:		update_pcr(dsp32.pcr & ~PCR_PDFs); return dsp32.pdr;
		case 14:	return dsp32.piop;
		case 20:	return dsp32.pdr2;
		case 22:	update_pcr(dsp32.pcr & ~PCR_PIFs); return dsp32.pir;
		case 30:	return dsp32.pcw;
		default:	fprintf(stderr, "Unimplemented CAU PI read = %X\n", i);
	}
	return 0;
}


static void cau_write_pi_special(UINT8 i, UINT32 val)
{
	switch (i)
	{
		case 4:		dsp32.ibuf = val;	break;
		case 5:		dsp32.obuf = val;	break;
		case 6:		dsp32.pdr = val; update_pcr(dsp32.pcr | PCR_PDFs); break;
		case 14:	dsp32.piop = val;	break;
		case 20:	dsp32.pdr2 = val; 	break;
		case 22:	dsp32.pir = val; update_pcr(dsp32.pcr | PCR_PIFs); break;
		case 30:	dsp32.pcw = val; 	break;
		default:	fprintf(stderr, "Unimplemented CAU PI write = %X\n", i);
	}
}


INLINE UINT8 cau_read_pi_1byte(int pi)
{
	int p = (pi >> 5) & 0x1f;
	int i = (pi >> 0) & 0x1f;
	if (p)
	{
		UINT32 result = RBYTE(dsp32.r[p]);
		dsp32.r[p] = TRUNCATE24(dsp32.r[p] + dsp32.r[i]);
		return result;
	}
	else
		return cau_read_pi_special(i);
}


INLINE UINT16 cau_read_pi_2byte(int pi)
{
	int p = (pi >> 5) & 0x1f;
	int i = (pi >> 0) & 0x1f;
	if (p)
	{
		UINT32 result = RWORD(dsp32.r[p]);
		if (i < 22 || i > 23)
			dsp32.r[p] = TRUNCATE24(dsp32.r[p] + dsp32.r[i]);
		else
			dsp32.r[p] = TRUNCATE24(dsp32.r[p] + dsp32.r[i] * 2);
		return result;
	}
	else
		return cau_read_pi_special(i);
}


INLINE UINT32 cau_read_pi_4byte(int pi)
{
	int p = (pi >> 5) & 0x1f;
	int i = (pi >> 0) & 0x1f;
	if (p)
	{
		UINT32 result = RLONG(dsp32.r[p]);
		if (i < 22 || i > 23)
			dsp32.r[p] = TRUNCATE24(dsp32.r[p] + dsp32.r[i]);
		else
			dsp32.r[p] = TRUNCATE24(dsp32.r[p] + dsp32.r[i] * 4);
		return result;
	}
	else
		return cau_read_pi_special(i);
}


INLINE void cau_write_pi_1byte(int pi, UINT8 val)
{
	int p = (pi >> 5) & 0x1f;
	int i = (pi >> 0) & 0x1f;
	if (p)
	{
		WBYTE(dsp32.r[p], val);
		dsp32.r[p] = TRUNCATE24(dsp32.r[p] + dsp32.r[i]);
	}
	else
		cau_write_pi_special(i, val);
}


INLINE void cau_write_pi_2byte(int pi, UINT16 val)
{
	int p = (pi >> 5) & 0x1f;
	int i = (pi >> 0) & 0x1f;
	if (p)
	{
		WWORD(dsp32.r[p], val);
		if (i < 22 || i > 23)
			dsp32.r[p] = TRUNCATE24(dsp32.r[p] + dsp32.r[i]);
		else
			dsp32.r[p] = TRUNCATE24(dsp32.r[p] + dsp32.r[i] * 2);
	}
	else
		cau_write_pi_special(i, val);
}


INLINE void cau_write_pi_4byte(int pi, UINT32 val)
{
	int p = (pi >> 5) & 0x1f;
	int i = (pi >> 0) & 0x1f;
	if (p)
	{
		WLONG(dsp32.r[p], (INT32)(val << 8) >> 8);
		if (i < 22 || i > 23)
			dsp32.r[p] = TRUNCATE24(dsp32.r[p] + dsp32.r[i]);
		else
			dsp32.r[p] = TRUNCATE24(dsp32.r[p] + dsp32.r[i] * 4);
	}
	else
		cau_write_pi_special(i, val);
}



/***************************************************************************
    DAU HELPERS
***************************************************************************/

INLINE double dau_get_amult(int aidx)
{
	int bufidx = (dsp32.abuf_index - 1) & 3;
	double val = dsp32.a[aidx];
	while (dsp32_icount >= dsp32.abufcycle[bufidx] - 2 * 4)
	{
		if (dsp32.abufreg[bufidx] == aidx)
			val = dsp32.abuf[bufidx];
		bufidx = (bufidx - 1) & 3;
	}
	return val;
}


INLINE double dau_get_anzflags(void)
{
	int bufidx = (dsp32.abuf_index - 1) & 3;
	double nzflags = dsp32.NZflags;
	while (dsp32_icount >= dsp32.abufcycle[bufidx] - 3 * 4)
	{
		nzflags = dsp32.abufNZflags[bufidx];
		bufidx = (bufidx - 1) & 3;
	}
	return nzflags;
}


INLINE UINT8 dau_get_avuflags(void)
{
#if (!IGNORE_DAU_UV_FLAGS)
	int bufidx = (dsp32.abuf_index - 1) & 3;
	UINT8 vuflags = dsp32.VUflags;
	while (dsp32_icount >= dsp32.abufcycle[bufidx] - 3 * 4)
	{
		vuflags = dsp32.abufVUflags[bufidx];
		bufidx = (bufidx - 1) & 3;
	}
	return vuflags;
#else
	return 0;
#endif
}


INLINE void remember_last_dau(int aidx)
{
#if (EMULATE_MULTIPLIER_LATENCY || EMULATE_AFLAGS_LATENCY)
	int bufidx = dsp32.abuf_index++ & 3;
	dsp32.abuf[bufidx] = dsp32.a[aidx];
	dsp32.abufreg[bufidx] = aidx;
	dsp32.abufNZflags[bufidx] = dsp32.NZflags;
#if (!IGNORE_DAU_UV_FLAGS)
	dsp32.abufVUflags[bufidx] = dsp32.VUflags;
#endif
	dsp32.abufcycle[bufidx] = dsp32_icount;
#endif
}


INLINE void dau_set_val_noflags(int aidx, double res)
{
	remember_last_dau(aidx);
	dsp32.a[aidx] = res;
}


INLINE void dau_set_val_flags(int aidx, double res)
{
	remember_last_dau(aidx);
#if (!IGNORE_DAU_UV_FLAGS)
{
	double absres = (res < 0) ? -res : res;
	dsp32.VUflags = 0;
	if (absres < 5.87747e-39)
	{
		if (absres != 0)
			dsp32.VUflags = UFLAGBIT;
		res = 0.0;
	}
	else if (absres > 3.40282e38)
	{
		dsp32.VUflags = VFLAGBIT;
//      DEBUGGER_BREAK;
//      fprintf(stderr, "Result = %g\n", absres);
		res = (res < 0) ? -3.40282e38 : 3.40282e38;
	}
}
#endif
	dsp32.NZflags = res;
	dsp32.a[aidx] = res;
}


INLINE double dsp_to_double(UINT32 val)
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


INLINE UINT32 double_to_dsp(double val)
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
//      DEBUGGER_BREAK;
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


static double dau_read_pi_special(int i)
{
    fatalerror("Unimplemented dau_read_pi_special(%d)", i);
	return 0;
}


static void dau_write_pi_special(int i, double val)
{
    fatalerror("Unimplemented dau_write_pi_special(%d)", i);
}


static int lastp;

INLINE double dau_read_pi_double_1st(int pi, int multiplier)
{
	int p = (pi >> 3) & 15;
	int i = (pi >> 0) & 7;

	lastp = p;
	if (p)
	{
		UINT32 result = RLONG(dsp32.r[p]);
		if (i < 6)
			dsp32.r[p] = TRUNCATE24(dsp32.r[p] + dsp32.r[i+16]);
		else
			dsp32.r[p] = TRUNCATE24(dsp32.r[p] + dsp32.r[i+16] * 4);
		return dsp_to_double(result);
	}
	else if (i < 4)
		return multiplier ? DEFERRED_MULTIPLIER(i) : dsp32.a[i];
	else
		return dau_read_pi_special(i);
}


INLINE double dau_read_pi_double_2nd(int pi, int multiplier, double xval)
{
	int p = (pi >> 3) & 15;
	int i = (pi >> 0) & 7;

	if (p == 15) p = lastp;		/* P=15 means Z inherits from Y, Y inherits from X */
	lastp = p;
	if (p)
	{
		UINT32 result;
		result = RLONG(dsp32.r[p]);
		if (i < 6)
			dsp32.r[p] = TRUNCATE24(dsp32.r[p] + dsp32.r[i+16]);
		else
			dsp32.r[p] = TRUNCATE24(dsp32.r[p] + dsp32.r[i+16] * 4);
		return dsp_to_double(result);
	}
	else if (i < 4)
		return multiplier ? DEFERRED_MULTIPLIER(i) : dsp32.a[i];
	else
		return dau_read_pi_special(i);
}


INLINE UINT32 dau_read_pi_4bytes(int pi)
{
	int p = (pi >> 3) & 15;
	int i = (pi >> 0) & 7;

	lastp = p;
	if (p)
	{
		UINT32 result = RLONG(dsp32.r[p]);
		if (i < 6)
			dsp32.r[p] = TRUNCATE24(dsp32.r[p] + dsp32.r[i+16]);
		else
			dsp32.r[p] = TRUNCATE24(dsp32.r[p] + dsp32.r[i+16] * 4);
		return result;
	}
	else if (i < 4)
		return double_to_dsp(dsp32.a[i]);
	else
		return dau_read_pi_special(i);
}


INLINE UINT16 dau_read_pi_2bytes(int pi)
{
	int p = (pi >> 3) & 15;
	int i = (pi >> 0) & 7;

	lastp = p;
	if (p)
	{
		UINT32 result = RWORD(dsp32.r[p]);
		if (i < 6)
			dsp32.r[p] = TRUNCATE24(dsp32.r[p] + dsp32.r[i+16]);
		else
			dsp32.r[p] = TRUNCATE24(dsp32.r[p] + dsp32.r[i+16] * 2);
		return result;
	}
	else if (i < 4)
		return double_to_dsp(dsp32.a[i]);
	else
		return dau_read_pi_special(i);
}


INLINE void dau_write_pi_double(int pi, double val)
{
	int p = (pi >> 3) & 15;
	int i = (pi >> 0) & 7;

	if (p == 15) p = lastp;		/* P=15 means Z inherits from Y, Y inherits from X */
	if (p)
	{
		WLONG_DEFERRED(dsp32.r[p], double_to_dsp(val));
		if (i < 6)
			dsp32.r[p] = TRUNCATE24(dsp32.r[p] + dsp32.r[i+16]);
		else
			dsp32.r[p] = TRUNCATE24(dsp32.r[p] + dsp32.r[i+16] * 4);
	}
	else if (i < 4)
		dau_set_val_noflags(i, val);
	else
		dau_write_pi_special(i, val);
}


INLINE void dau_write_pi_4bytes(int pi, UINT32 val)
{
	int p = (pi >> 3) & 15;
	int i = (pi >> 0) & 7;

	if (p == 15) p = lastp;		/* P=15 means Z inherits from Y, Y inherits from X */
	if (p)
	{
		lastp = p;
		WLONG_DEFERRED(dsp32.r[p], val);
		if (i < 6)
			dsp32.r[p] = TRUNCATE24(dsp32.r[p] + dsp32.r[i+16]);
		else
			dsp32.r[p] = TRUNCATE24(dsp32.r[p] + dsp32.r[i+16] * 4);
	}
	else if (i < 4)
		dau_set_val_noflags(i, dsp_to_double(val));
	else
		dau_write_pi_special(i, val);
}


INLINE void dau_write_pi_2bytes(int pi, UINT16 val)
{
	int p = (pi >> 3) & 15;
	int i = (pi >> 0) & 7;

	if (p == 15) p = lastp;		/* P=15 means Z inherits from Y, Y inherits from X */
	if (p)
	{
		lastp = p;
		WWORD_DEFERRED(dsp32.r[p], val);
		if (i < 6)
			dsp32.r[p] = TRUNCATE24(dsp32.r[p] + dsp32.r[i+16]);
		else
			dsp32.r[p] = TRUNCATE24(dsp32.r[p] + dsp32.r[i+16] * 2);
	}
	else if (i < 4)
		dau_set_val_noflags(i, dsp_to_double(val << 16));
	else
		dau_write_pi_special(i, val);
}



/***************************************************************************
    COMMON CONDITION ROUTINE
***************************************************************************/

#if (!ASSUME_UNCONDITIONAL_CAU)
static int condition(int cond)
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
			return !(DEFERRED_VUFLAGS & UFLAGBIT);
		case 17:
			return (DEFERRED_VUFLAGS & UFLAGBIT);
		case 18:
			return !(DEFERRED_NZFLAGS < 0);
		case 19:
			return (DEFERRED_NZFLAGS < 0);
		case 20:
			return !(DEFERRED_NZFLAGS == 0);
		case 21:
			return (DEFERRED_NZFLAGS == 0);
		case 22:
			return !(DEFERRED_VUFLAGS & VFLAGBIT);
		case 23:
			return (DEFERRED_VUFLAGS & VFLAGBIT);
		case 24:
			return !(DEFERRED_NZFLAGS <= 0);
		case 25:
			return (DEFERRED_NZFLAGS <= 0);

		case 32:	/* !ibf */
		case 33:	/* ibf */
		case 34:	/* !obe */
		case 35:	/* obe */
		case 36:	/* !pdf */
		case 37:	/* pdf */
		case 38:	/* !pif */
		case 39:	/* pif */
		case 40:	/* !sy */
		case 41:	/* sy */
		case 42:	/* !fb */
		case 43:	/* fb */
		case 44:	/* !ireq1 */
		case 45:	/* ireq1 */
		case 46:	/* !ireq2 */
		case 47:	/* ireq2 */
		default:
		    fatalerror("Unimplemented condition: %X", cond);
	}
}
#endif



/***************************************************************************
    CAU BRANCH INSTRUCTION IMPLEMENTATION
***************************************************************************/

static void nop(void)
{
	UINT32 op = OP;
	if (op == 0)
		return;
	execute_one();
	dsp32.PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	memory_set_opbase(dsp32.PC);

}


static void goto_t(void)
{
	UINT32 op = OP;
	execute_one();
	dsp32.PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	memory_set_opbase(dsp32.PC);
}


static void goto_pl(void)
{
	if (!nFLAG)
	{
		UINT32 op = OP;
		execute_one();
		dsp32.PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
		memory_set_opbase(dsp32.PC);
	}
}


static void goto_mi(void)
{
	if (nFLAG)
	{
		UINT32 op = OP;
		execute_one();
		dsp32.PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
		memory_set_opbase(dsp32.PC);
	}
}


static void goto_ne(void)
{
	if (!zFLAG)
	{
		UINT32 op = OP;
		execute_one();
		dsp32.PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
		memory_set_opbase(dsp32.PC);
	}
}


static void goto_eq(void)
{
	if (zFLAG)
	{
		UINT32 op = OP;
		execute_one();
		dsp32.PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
		memory_set_opbase(dsp32.PC);
	}
}


static void goto_vc(void)
{
	if (!vFLAG)
	{
		UINT32 op = OP;
		execute_one();
		dsp32.PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
		memory_set_opbase(dsp32.PC);
	}
}


static void goto_vs(void)
{
	if (vFLAG)
	{
		UINT32 op = OP;
		execute_one();
		dsp32.PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
		memory_set_opbase(dsp32.PC);
	}
}


static void goto_cc(void)
{
	if (!cFLAG)
	{
		UINT32 op = OP;
		execute_one();
		dsp32.PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
		memory_set_opbase(dsp32.PC);
	}
}


static void goto_cs(void)
{
	if (cFLAG)
	{
		UINT32 op = OP;
		execute_one();
		dsp32.PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
		memory_set_opbase(dsp32.PC);
	}
}


static void goto_ge(void)
{
	if (!(nFLAG ^ vFLAG))
	{
		UINT32 op = OP;
		execute_one();
		dsp32.PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
		memory_set_opbase(dsp32.PC);
	}
}


static void goto_lt(void)
{
	if (nFLAG ^ vFLAG)
	{
		UINT32 op = OP;
		execute_one();
		dsp32.PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
		memory_set_opbase(dsp32.PC);
	}
}


static void goto_gt(void)
{
	if (!(zFLAG | (nFLAG ^ vFLAG)))
	{
		UINT32 op = OP;
		execute_one();
		dsp32.PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
		memory_set_opbase(dsp32.PC);
	}
}


static void goto_le(void)
{
	if (zFLAG | (nFLAG ^ vFLAG))
	{
		UINT32 op = OP;
		execute_one();
		dsp32.PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
		memory_set_opbase(dsp32.PC);
	}
}


static void goto_hi(void)
{
	if (!cFLAG && !zFLAG)
	{
		UINT32 op = OP;
		execute_one();
		dsp32.PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
		memory_set_opbase(dsp32.PC);
	}
}


static void goto_ls(void)
{
	if (cFLAG || zFLAG)
	{
		UINT32 op = OP;
		execute_one();
		dsp32.PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
		memory_set_opbase(dsp32.PC);
	}
}


static void goto_auc(void)
{
	if (!(DEFERRED_VUFLAGS & UFLAGBIT))
	{
		UINT32 op = OP;
		execute_one();
		dsp32.PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
		memory_set_opbase(dsp32.PC);
	}
}


static void goto_aus(void)
{
	if (DEFERRED_VUFLAGS & UFLAGBIT)
	{
		UINT32 op = OP;
		execute_one();
		dsp32.PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
		memory_set_opbase(dsp32.PC);
	}
}


static void goto_age(void)
{
	if (DEFERRED_NZFLAGS >= 0)
	{
		UINT32 op = OP;
		execute_one();
		dsp32.PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
		memory_set_opbase(dsp32.PC);
	}
}


static void goto_alt(void)
{
	if (DEFERRED_NZFLAGS < 0)
	{
		UINT32 op = OP;
		execute_one();
		dsp32.PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
		memory_set_opbase(dsp32.PC);
	}
}


static void goto_ane(void)
{
	if (DEFERRED_NZFLAGS != 0)
	{
		UINT32 op = OP;
		execute_one();
		dsp32.PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
		memory_set_opbase(dsp32.PC);
	}
}


static void goto_aeq(void)
{
	if (DEFERRED_NZFLAGS == 0)
	{
		UINT32 op = OP;
		execute_one();
		dsp32.PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
		memory_set_opbase(dsp32.PC);
	}
}


static void goto_avc(void)
{
	if (!(DEFERRED_VUFLAGS & VFLAGBIT))
	{
		UINT32 op = OP;
		execute_one();
		dsp32.PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
		memory_set_opbase(dsp32.PC);
	}
}


static void goto_avs(void)
{
	if (DEFERRED_VUFLAGS & VFLAGBIT)
	{
		UINT32 op = OP;
		execute_one();
		dsp32.PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
		memory_set_opbase(dsp32.PC);
	}
}


static void goto_agt(void)
{
	if (DEFERRED_NZFLAGS > 0)
	{
		UINT32 op = OP;
		execute_one();
		dsp32.PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
		memory_set_opbase(dsp32.PC);
	}
}


static void goto_ale(void)
{
	if (DEFERRED_NZFLAGS <= 0)
	{
		UINT32 op = OP;
		execute_one();
		dsp32.PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
		memory_set_opbase(dsp32.PC);
	}
}


static void goto_ibe(void)
{
	unimplemented();
}


static void goto_ibf(void)
{
	unimplemented();
}


static void goto_obf(void)
{
	unimplemented();
}


static void goto_obe(void)
{
	unimplemented();
}


static void goto_pde(void)
{
	unimplemented();
}


static void goto_pdf(void)
{
	unimplemented();
}


static void goto_pie(void)
{
	unimplemented();
}


static void goto_pif(void)
{
	unimplemented();
}


static void goto_syc(void)
{
	unimplemented();
}


static void goto_sys(void)
{
	unimplemented();
}


static void goto_fbc(void)
{
	unimplemented();
}


static void goto_fbs(void)
{
	unimplemented();
}


static void goto_irq1lo(void)
{
	unimplemented();
}


static void goto_irq1hi(void)
{
	unimplemented();
}


static void goto_irq2lo(void)
{
	unimplemented();
}


static void goto_irq2hi(void)
{
	unimplemented();
}


static void dec_goto(void)
{
	int hr = (OP >> 21) & 0x1f;
	int old = (INT16)dsp32.r[hr];
	dsp32.r[hr] = EXTEND16_TO_24(dsp32.r[hr] - 1);
	if (old >= 0)
	{
		UINT32 op = OP;
		execute_one();
		dsp32.PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
		memory_set_opbase(dsp32.PC);
	}
}


static void call(void)
{
	UINT32 op = OP;
	int mr = (op >> 21) & 0x1f;
	if (IS_WRITEABLE(mr))
		dsp32.r[mr] = dsp32.PC + 4;
	execute_one();
	dsp32.PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (INT16)op);
	memory_set_opbase(dsp32.PC);
}


static void goto24(void)
{
	UINT32 op = OP;
	execute_one();
	dsp32.PC = TRUNCATE24(REG24((op >> 16) & 0x1f) + (op & 0xffff) + ((op >> 5) & 0xff0000));
	memory_set_opbase(dsp32.PC);
}


static void call24(void)
{
	UINT32 op = OP;
	int mr = (op >> 16) & 0x1f;
	if (IS_WRITEABLE(mr))
		dsp32.r[mr] = dsp32.PC + 4;
	execute_one();
	dsp32.PC = (op & 0xffff) + ((op >> 5) & 0xff0000);
	memory_set_opbase(dsp32.PC);
}


static void do_i(void)
{
	unimplemented();
}


static void do_r(void)
{
	unimplemented();
}



/***************************************************************************
    CAU 16-BIT ARITHMETIC IMPLEMENTATION
***************************************************************************/

static void add_si(void)
{
	int dr = (OP >> 21) & 0x1f;
	int hrval = REG16((OP >> 16) & 0x1f);
	int res = hrval + (UINT16)OP;
	if (IS_WRITEABLE(dr))
		dsp32.r[dr] = EXTEND16_TO_24(res);
	SET_NZCV_16(hrval, OP, res);
}


static void add_ss(void)
{
	if (CONDITION_IS_TRUE)
	{
		int dr = (OP >> 16) & 0x1f;
		int s1rval = REG16((OP >> 5) & 0x1f);
		int s2rval = (OP & 0x800) ? REG16((OP >> 0) & 0x1f) : REG16(dr);
		int res = s2rval + s1rval;
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = EXTEND16_TO_24(res);
		SET_NZCV_16(s1rval, s2rval, res);
	}
}


static void mul2_s(void)
{
	if (CONDITION_IS_TRUE)
	{
		int dr = (OP >> 16) & 0x1f;
		int s1rval = REG16((OP >> 5) & 0x1f);
		int res = s1rval * 2;
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = EXTEND16_TO_24(res);
		SET_NZCV_16(s1rval, 0, res);
	}
}


static void subr_ss(void)
{
	if (CONDITION_IS_TRUE)
	{
		int dr = (OP >> 16) & 0x1f;
		int s1rval = REG16((OP >> 5) & 0x1f);
		int s2rval = (OP & 0x800) ? REG16((OP >> 0) & 0x1f) : REG16(dr);
		int res = s1rval - s2rval;
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = EXTEND16_TO_24(res);
		SET_NZCV_16(s1rval, s2rval, res);
	}
}


static void addr_ss(void)
{
	unimplemented();
}


static void sub_ss(void)
{
	if (CONDITION_IS_TRUE)
	{
		int dr = (OP >> 16) & 0x1f;
		int s1rval = REG16((OP >> 5) & 0x1f);
		int s2rval = (OP & 0x800) ? REG16((OP >> 0) & 0x1f) : REG16(dr);
		int res = s2rval - s1rval;
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = EXTEND16_TO_24(res);
		SET_NZCV_16(s1rval, s2rval, res);
	}
}


static void neg_s(void)
{
	if (CONDITION_IS_TRUE)
	{
		int dr = (OP >> 16) & 0x1f;
		int s1rval = REG16((OP >> 5) & 0x1f);
		int res = -s1rval;
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = EXTEND16_TO_24(res);
		SET_NZCV_16(s1rval, 0, res);
	}
}


static void andc_ss(void)
{
	if (CONDITION_IS_TRUE)
	{
		int dr = (OP >> 16) & 0x1f;
		int s1rval = REG16((OP >> 5) & 0x1f);
		int s2rval = (OP & 0x800) ? REG16((OP >> 0) & 0x1f) : REG16(dr);
		int res = s2rval & ~s1rval;
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = EXTEND16_TO_24(res);
		SET_NZ00_16(res);
	}
}


static void cmp_ss(void)
{
	if (CONDITION_IS_TRUE)
	{
		int drval = REG16((OP >> 16) & 0x1f);
		int s1rval = REG16((OP >> 5) & 0x1f);
		int res = drval - s1rval;
		SET_NZCV_16(drval, s1rval, res);
	}
}


static void xor_ss(void)
{
	if (CONDITION_IS_TRUE)
	{
		int dr = (OP >> 16) & 0x1f;
		int s1rval = REG16((OP >> 5) & 0x1f);
		int s2rval = (OP & 0x800) ? REG16((OP >> 0) & 0x1f) : REG16(dr);
		int res = s2rval ^ s1rval;
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = EXTEND16_TO_24(res);
		SET_NZ00_16(res);
	}
}


static void rcr_s(void)
{
	if (CONDITION_IS_TRUE)
	{
		int dr = (OP >> 16) & 0x1f;
		int s1rval = REG16((OP >> 5) & 0x1f);
		int res = ((dsp32.nzcflags >> 9) & 0x8000) | (s1rval >> 1);
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = EXTEND16_TO_24(res);
		dsp32.nzcflags = ((res & 0xffff) << 8) | ((s1rval & 1) << 24);
		dsp32.vflags = 0;
	}
}


static void or_ss(void)
{
	if (CONDITION_IS_TRUE)
	{
		int dr = (OP >> 16) & 0x1f;
		int s1rval = REG16((OP >> 5) & 0x1f);
		int s2rval = (OP & 0x800) ? REG16((OP >> 0) & 0x1f) : REG16(dr);
		int res = s2rval | s1rval;
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = EXTEND16_TO_24(res);
		SET_NZ00_16(res);
	}
}


static void rcl_s(void)
{
	if (CONDITION_IS_TRUE)
	{
		int dr = (OP >> 16) & 0x1f;
		int s1rval = REG16((OP >> 5) & 0x1f);
		int res = ((dsp32.nzcflags >> 24) & 0x0001) | (s1rval << 1);
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = EXTEND16_TO_24(res);
		dsp32.nzcflags = ((res & 0xffff) << 8) | ((s1rval & 0x8000) << 9);
		dsp32.vflags = 0;
	}
}


static void shr_s(void)
{
	if (CONDITION_IS_TRUE)
	{
		int dr = (OP >> 16) & 0x1f;
		int s1rval = REG16((OP >> 5) & 0x1f);
		int res = s1rval >> 1;
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = EXTEND16_TO_24(res);
		dsp32.nzcflags = ((res & 0xffff) << 8) | ((s1rval & 1) << 24);
		dsp32.vflags = 0;
	}
}


static void div2_s(void)
{
	if (CONDITION_IS_TRUE)
	{
		int dr = (OP >> 16) & 0x1f;
		int s1rval = REG16((OP >> 5) & 0x1f);
		int res = (s1rval & 0x8000) | (s1rval >> 1);
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = EXTEND16_TO_24(res);
		dsp32.nzcflags = ((res & 0xffff) << 8) | ((s1rval & 1) << 24);
		dsp32.vflags = 0;
	}
}


static void and_ss(void)
{
	if (CONDITION_IS_TRUE)
	{
		int dr = (OP >> 16) & 0x1f;
		int s1rval = REG16((OP >> 5) & 0x1f);
		int s2rval = (OP & 0x800) ? REG16((OP >> 0) & 0x1f) : REG16(dr);
		int res = s2rval & s1rval;
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = EXTEND16_TO_24(res);
		SET_NZ00_16(res);
	}
}


static void test_ss(void)
{
	if (CONDITION_IS_TRUE)
	{
		int drval = REG16((OP >> 16) & 0x1f);
		int s1rval = REG16((OP >> 5) & 0x1f);
		int res = drval & s1rval;
		SET_NZ00_16(res);
	}
}


static void add_di(void)
{
	int dr = (OP >> 16) & 0x1f;
	int drval = REG16(dr);
	int res = drval + (UINT16)OP;
	if (IS_WRITEABLE(dr))
		dsp32.r[dr] = EXTEND16_TO_24(res);
	SET_NZCV_16(drval, OP, res);
}


static void subr_di(void)
{
	int dr = (OP >> 16) & 0x1f;
	int drval = REG16(dr);
	int res = (UINT16)OP - drval;
	if (IS_WRITEABLE(dr))
		dsp32.r[dr] = EXTEND16_TO_24(res);
	SET_NZCV_16(drval, OP, res);
}


static void addr_di(void)
{
	unimplemented();
}


static void sub_di(void)
{
	int dr = (OP >> 16) & 0x1f;
	int drval = REG16(dr);
	int res = drval - (UINT16)OP;
	if (IS_WRITEABLE(dr))
		dsp32.r[dr] = EXTEND16_TO_24(res);
	SET_NZCV_16(drval, OP, res);
}


static void andc_di(void)
{
	int dr = (OP >> 16) & 0x1f;
	int drval = REG16(dr);
	int res = drval & ~(UINT16)OP;
	if (IS_WRITEABLE(dr))
		dsp32.r[dr] = EXTEND16_TO_24(res);
	SET_NZ00_16(res);
}


static void cmp_di(void)
{
	int drval = REG16((OP >> 16) & 0x1f);
	int res = drval - (UINT16)OP;
	SET_NZCV_16(drval, OP, res);
}


static void xor_di(void)
{
	int dr = (OP >> 16) & 0x1f;
	int drval = REG16(dr);
	int res = drval ^ (UINT16)OP;
	if (IS_WRITEABLE(dr))
		dsp32.r[dr] = EXTEND16_TO_24(res);
	SET_NZ00_16(res);
}


static void or_di(void)
{
	int dr = (OP >> 16) & 0x1f;
	int drval = REG16(dr);
	int res = drval | (UINT16)OP;
	if (IS_WRITEABLE(dr))
		dsp32.r[dr] = EXTEND16_TO_24(res);
	SET_NZ00_16(res);
}


static void and_di(void)
{
	int dr = (OP >> 16) & 0x1f;
	int drval = REG16(dr);
	int res = drval & (UINT16)OP;
	if (IS_WRITEABLE(dr))
		dsp32.r[dr] = EXTEND16_TO_24(res);
	SET_NZ00_16(res);
}


static void test_di(void)
{
	int drval = REG16((OP >> 16) & 0x1f);
	int res = drval & (UINT16)OP;
	SET_NZ00_16(res);
}



/***************************************************************************
    CAU 24-BIT ARITHMETIC IMPLEMENTATION
***************************************************************************/

static void adde_si(void)
{
	int dr = (OP >> 21) & 0x1f;
	int hrval = REG24((OP >> 16) & 0x1f);
	int res = hrval + EXTEND16_TO_24(OP);
	if (IS_WRITEABLE(dr))
		dsp32.r[dr] = TRUNCATE24(res);
	SET_NZCV_24(hrval, OP << 8, res);
}


static void adde_ss(void)
{
	if (CONDITION_IS_TRUE)
	{
		int dr = (OP >> 16) & 0x1f;
		int s1rval = REG24((OP >> 5) & 0x1f);
		int s2rval = (OP & 0x800) ? REG24((OP >> 0) & 0x1f) : REG24(dr);
		int res = s2rval + s1rval;
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = TRUNCATE24(res);
		SET_NZCV_24(s1rval, s2rval, res);
	}
}


static void mul2e_s(void)
{
	if (CONDITION_IS_TRUE)
	{
		int dr = (OP >> 16) & 0x1f;
		int s1rval = REG24((OP >> 5) & 0x1f);
		int res = s1rval * 2;
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = TRUNCATE24(res);
		SET_NZCV_24(s1rval, 0, res);
	}
}


static void subre_ss(void)
{
	if (CONDITION_IS_TRUE)
	{
		int dr = (OP >> 16) & 0x1f;
		int s1rval = REG24((OP >> 5) & 0x1f);
		int s2rval = (OP & 0x800) ? REG24((OP >> 0) & 0x1f) : REG24(dr);
		int res = s1rval - s2rval;
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = TRUNCATE24(res);
		SET_NZCV_24(s1rval, s2rval, res);
	}
}


static void addre_ss(void)
{
	unimplemented();
}


static void sube_ss(void)
{
	if (CONDITION_IS_TRUE)
	{
		int dr = (OP >> 16) & 0x1f;
		int s1rval = REG24((OP >> 5) & 0x1f);
		int s2rval = (OP & 0x800) ? REG24((OP >> 0) & 0x1f) : REG24(dr);
		int res = s2rval - s1rval;
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = TRUNCATE24(res);
		SET_NZCV_24(s1rval, s2rval, res);
	}
}


static void nege_s(void)
{
	if (CONDITION_IS_TRUE)
	{
		int dr = (OP >> 16) & 0x1f;
		int s1rval = REG24((OP >> 5) & 0x1f);
		int res = -s1rval;
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = TRUNCATE24(res);
		SET_NZCV_24(s1rval, 0, res);
	}
}


static void andce_ss(void)
{
	if (CONDITION_IS_TRUE)
	{
		int dr = (OP >> 16) & 0x1f;
		int s1rval = REG24((OP >> 5) & 0x1f);
		int s2rval = (OP & 0x800) ? REG24((OP >> 0) & 0x1f) : REG24(dr);
		int res = s2rval & ~s1rval;
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = res;
		SET_NZ00_24(res);
	}
}


static void cmpe_ss(void)
{
	if (CONDITION_IS_TRUE)
	{
		int drval = REG24((OP >> 16) & 0x1f);
		int s1rval = REG24((OP >> 5) & 0x1f);
		int res = drval - s1rval;
		SET_NZCV_24(drval, s1rval, res);
	}
}


static void xore_ss(void)
{
	if (CONDITION_IS_TRUE)
	{
		int dr = (OP >> 16) & 0x1f;
		int s1rval = REG24((OP >> 5) & 0x1f);
		int s2rval = (OP & 0x800) ? REG24((OP >> 0) & 0x1f) : REG24(dr);
		int res = s2rval ^ s1rval;
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = res;
		SET_NZ00_24(res);
	}
}


static void rcre_s(void)
{
	if (CONDITION_IS_TRUE)
	{
		int dr = (OP >> 16) & 0x1f;
		int s1rval = REG24((OP >> 5) & 0x1f);
		int res = ((dsp32.nzcflags >> 1) & 0x800000) | (s1rval >> 1);
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = TRUNCATE24(res);
		dsp32.nzcflags = res | ((s1rval & 1) << 24);
		dsp32.vflags = 0;
	}
}


static void ore_ss(void)
{
	if (CONDITION_IS_TRUE)
	{
		int dr = (OP >> 16) & 0x1f;
		int s1rval = REG24((OP >> 5) & 0x1f);
		int s2rval = (OP & 0x800) ? REG24((OP >> 0) & 0x1f) : REG24(dr);
		int res = s2rval | s1rval;
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = res;
		SET_NZ00_24(res);
	}
}


static void rcle_s(void)
{
	if (CONDITION_IS_TRUE)
	{
		int dr = (OP >> 16) & 0x1f;
		int s1rval = REG24((OP >> 5) & 0x1f);
		int res = ((dsp32.nzcflags >> 24) & 0x000001) | (s1rval << 1);
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = TRUNCATE24(res);
		dsp32.nzcflags = res | ((s1rval & 0x800000) << 1);
		dsp32.vflags = 0;
	}
}


static void shre_s(void)
{
	if (CONDITION_IS_TRUE)
	{
		int dr = (OP >> 16) & 0x1f;
		int s1rval = REG24((OP >> 5) & 0x1f);
		int res = s1rval >> 1;
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = res;
		dsp32.nzcflags = res | ((s1rval & 1) << 24);
		dsp32.vflags = 0;
	}
}


static void div2e_s(void)
{
	if (CONDITION_IS_TRUE)
	{
		int dr = (OP >> 16) & 0x1f;
		int s1rval = REG24((OP >> 5) & 0x1f);
		int res = (s1rval & 0x800000) | (s1rval >> 1);
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = TRUNCATE24(res);
		dsp32.nzcflags = res | ((s1rval & 1) << 24);
		dsp32.vflags = 0;
	}
}


static void ande_ss(void)
{
	if (CONDITION_IS_TRUE)
	{
		int dr = (OP >> 16) & 0x1f;
		int s1rval = REG24((OP >> 5) & 0x1f);
		int s2rval = (OP & 0x800) ? REG24((OP >> 0) & 0x1f) : REG24(dr);
		int res = s2rval & s1rval;
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = res;
		SET_NZ00_24(res);
	}
}


static void teste_ss(void)
{
	if (CONDITION_IS_TRUE)
	{
		int drval = REG24((OP >> 16) & 0x1f);
		int s1rval = REG24((OP >> 5) & 0x1f);
		int res = drval & s1rval;
		SET_NZ00_24(res);
	}
}


static void adde_di(void)
{
	int dr = (OP >> 16) & 0x1f;
	int drval = REG24(dr);
	int res = drval + EXTEND16_TO_24(OP);
	if (IS_WRITEABLE(dr))
		dsp32.r[dr] = TRUNCATE24(res);
	SET_NZCV_24(drval, OP << 8, res);
}


static void subre_di(void)
{
	int dr = (OP >> 16) & 0x1f;
	int drval = REG24(dr);
	int res = EXTEND16_TO_24(OP) - drval;
	if (IS_WRITEABLE(dr))
		dsp32.r[dr] = TRUNCATE24(res);
	SET_NZCV_24(drval, OP << 8, res);
}


static void addre_di(void)
{
	unimplemented();
}


static void sube_di(void)
{
	int dr = (OP >> 16) & 0x1f;
	int drval = REG24(dr);
	int res = drval - EXTEND16_TO_24(OP);
	if (IS_WRITEABLE(dr))
		dsp32.r[dr] = TRUNCATE24(res);
	SET_NZCV_24(drval, OP << 8, res);
}


static void andce_di(void)
{
	int dr = (OP >> 16) & 0x1f;
	int drval = REG24(dr);
	int res = drval & ~EXTEND16_TO_24(OP);
	if (IS_WRITEABLE(dr))
		dsp32.r[dr] = res;
	SET_NZ00_24(res);
}


static void cmpe_di(void)
{
	int drval = REG24((OP >> 16) & 0x1f);
	int res = drval - EXTEND16_TO_24(OP);
	SET_NZCV_24(drval, OP << 8, res);
}


static void xore_di(void)
{
	int dr = (OP >> 16) & 0x1f;
	int drval = REG24(dr);
	int res = drval ^ EXTEND16_TO_24(OP);
	if (IS_WRITEABLE(dr))
		dsp32.r[dr] = res;
	SET_NZ00_24(res);
}


static void ore_di(void)
{
	int dr = (OP >> 16) & 0x1f;
	int drval = REG24(dr);
	int res = drval | EXTEND16_TO_24(OP);
	if (IS_WRITEABLE(dr))
		dsp32.r[dr] = res;
	SET_NZ00_24(res);
}


static void ande_di(void)
{
	int dr = (OP >> 16) & 0x1f;
	int drval = REG24(dr);
	int res = drval & EXTEND16_TO_24(OP);
	if (IS_WRITEABLE(dr))
		dsp32.r[dr] = res;
	SET_NZ00_24(res);
}


static void teste_di(void)
{
	int drval = REG24((OP >> 16) & 0x1f);
	int res = drval & EXTEND16_TO_24(OP);
	SET_NZ00_24(res);
}



/***************************************************************************
    CAU LOAD/STORE IMPLEMENTATION
***************************************************************************/

static void load_hi(void)
{
	int dr = (OP >> 16) & 0x1f;
	UINT32 res = RBYTE(EXTEND16_TO_24(OP));
	if (IS_WRITEABLE(dr))
		dsp32.r[dr] = EXTEND16_TO_24(res);
	dsp32.nzcflags = res << 8;
	dsp32.vflags = 0;
}


static void load_li(void)
{
	int dr = (OP >> 16) & 0x1f;
	UINT32 res = RBYTE(EXTEND16_TO_24(OP));
	if (IS_WRITEABLE(dr))
		dsp32.r[dr] = res;
	dsp32.nzcflags = res << 8;
	dsp32.vflags = 0;
}


static void load_i(void)
{
	UINT32 res = RWORD(EXTEND16_TO_24(OP));
	int dr = (OP >> 16) & 0x1f;
	if (IS_WRITEABLE(dr))
		dsp32.r[dr] = EXTEND16_TO_24(res);
	dsp32.nzcflags = res << 8;
	dsp32.vflags = 0;
}


static void load_ei(void)
{
	UINT32 res = TRUNCATE24(RLONG(EXTEND16_TO_24(OP)));
	int dr = (OP >> 16) & 0x1f;
	if (IS_WRITEABLE(dr))
		dsp32.r[dr] = res;
	dsp32.nzcflags = res;
	dsp32.vflags = 0;
}


static void store_hi(void)
{
	WBYTE(EXTEND16_TO_24(OP), dsp32.r[(OP >> 16) & 0x1f] >> 8);
}


static void store_li(void)
{
	WBYTE(EXTEND16_TO_24(OP), dsp32.r[(OP >> 16) & 0x1f]);
}


static void store_i(void)
{
	WWORD(EXTEND16_TO_24(OP), REG16((OP >> 16) & 0x1f));
}


static void store_ei(void)
{
	WLONG(EXTEND16_TO_24(OP), (INT32)(REG24((OP >> 16) & 0x1f) << 8) >> 8);
}


static void load_hr(void)
{
	if (!(OP & 0x400))
	{
		int dr = (OP >> 16) & 0x1f;
		UINT32 res = cau_read_pi_1byte(OP) << 8;
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = EXTEND16_TO_24(res);
		dsp32.nzcflags = res << 8;
		dsp32.vflags = 0;
	}
	else
		unimplemented();
}


static void load_lr(void)
{
	if (!(OP & 0x400))
	{
		int dr = (OP >> 16) & 0x1f;
		UINT32 res = cau_read_pi_1byte(OP);
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = res;
		dsp32.nzcflags = res << 8;
		dsp32.vflags = 0;
	}
	else
		unimplemented();
}


static void load_r(void)
{
	if (!(OP & 0x400))
	{
		UINT32 res = cau_read_pi_2byte(OP);
		int dr = (OP >> 16) & 0x1f;
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = EXTEND16_TO_24(res);
		dsp32.nzcflags = res << 8;
		dsp32.vflags = 0;
	}
	else
		unimplemented();
}


static void load_er(void)
{
	if (!(OP & 0x400))
	{
		UINT32 res = TRUNCATE24(cau_read_pi_4byte(OP));
		int dr = (OP >> 16) & 0x1f;
		if (IS_WRITEABLE(dr))
			dsp32.r[dr] = res;
		dsp32.nzcflags = res;
		dsp32.vflags = 0;
	}
	else
		unimplemented();
}


static void store_hr(void)
{
	if (!(OP & 0x400))
		cau_write_pi_1byte(OP, dsp32.r[(OP >> 16) & 0x1f] >> 8);
	else
		unimplemented();
}


static void store_lr(void)
{
	if (!(OP & 0x400))
		cau_write_pi_1byte(OP, dsp32.r[(OP >> 16) & 0x1f]);
	else
		unimplemented();
}


static void store_r(void)
{
	if (!(OP & 0x400))
		cau_write_pi_2byte(OP, REG16((OP >> 16) & 0x1f));
	else
		unimplemented();
}


static void store_er(void)
{
	if (!(OP & 0x400))
		cau_write_pi_4byte(OP, REG24((OP >> 16) & 0x1f));
	else
		unimplemented();
}


static void load24(void)
{
	int dr = (OP >> 16) & 0x1f;
	UINT32 res = (OP & 0xffff) + ((OP >> 5) & 0xff0000);
	if (IS_WRITEABLE(dr))
		dsp32.r[dr] = res;
}



/***************************************************************************
    DAU FORM 1 IMPLEMENTATION
***************************************************************************/

static void d1_aMpp(void)
{
	double xval = dau_read_pi_double_1st(OP >> 14, 1);
	double yval = dau_read_pi_double_2nd(OP >> 7, 0, xval);
	double res = yval + DEFERRED_MULTIPLIER((OP >> 26) & 7) * xval;
	int zpi = (OP >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((OP >> 21) & 3, res);
}


static void d1_aMpm(void)
{
	double xval = dau_read_pi_double_1st(OP >> 14, 1);
	double yval = dau_read_pi_double_2nd(OP >> 7, 0, xval);
	double res = yval - DEFERRED_MULTIPLIER((OP >> 26) & 7) * xval;
	int zpi = (OP >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((OP >> 21) & 3, res);
}


static void d1_aMmp(void)
{
	double xval = dau_read_pi_double_1st(OP >> 14, 1);
	double yval = dau_read_pi_double_2nd(OP >> 7, 0, xval);
	double res = -yval + DEFERRED_MULTIPLIER((OP >> 26) & 7) * xval;
	int zpi = (OP >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((OP >> 21) & 3, res);
}


static void d1_aMmm(void)
{
	double xval = dau_read_pi_double_1st(OP >> 14, 1);
	double yval = dau_read_pi_double_2nd(OP >> 7, 0, xval);
	double res = -yval - DEFERRED_MULTIPLIER((OP >> 26) & 7) * xval;
	int zpi = (OP >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((OP >> 21) & 3, res);
}


static void d1_0px(void)
{
	double xval = dau_read_pi_double_1st(OP >> 14, 1);
	double yval = dau_read_pi_double_2nd(OP >> 7, 0, xval);
	double res = yval;
	int zpi = (OP >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((OP >> 21) & 3, res);
	(void)xval;
}


static void d1_0mx(void)
{
	double xval = dau_read_pi_double_1st(OP >> 14, 1);
	double yval = dau_read_pi_double_2nd(OP >> 7, 0, xval);
	double res = -yval;
	int zpi = (OP >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((OP >> 21) & 3, res);
	(void)xval;
}


static void d1_1pp(void)
{
	double xval = dau_read_pi_double_1st(OP >> 14, 1);
	double yval = dau_read_pi_double_2nd(OP >> 7, 0, xval);
	double res = yval + xval;
	int zpi = (OP >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((OP >> 21) & 3, res);
}


static void d1_1pm(void)
{
	double xval = dau_read_pi_double_1st(OP >> 14, 1);
	double yval = dau_read_pi_double_2nd(OP >> 7, 0, xval);
	double res = yval - xval;
	int zpi = (OP >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((OP >> 21) & 3, res);
}


static void d1_1mp(void)
{
	double xval = dau_read_pi_double_1st(OP >> 14, 1);
	double yval = dau_read_pi_double_2nd(OP >> 7, 0, xval);
	double res = -yval + xval;
	int zpi = (OP >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((OP >> 21) & 3, res);
}


static void d1_1mm(void)
{
	double xval = dau_read_pi_double_1st(OP >> 14, 1);
	double yval = dau_read_pi_double_2nd(OP >> 7, 0, xval);
	double res = -yval - xval;
	int zpi = (OP >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((OP >> 21) & 3, res);
}


static void d1_aMppr(void)
{
	unimplemented();
}


static void d1_aMpmr(void)
{
	unimplemented();
}


static void d1_aMmpr(void)
{
	unimplemented();
}


static void d1_aMmmr(void)
{
	unimplemented();
}



/***************************************************************************
    DAU FORM 2 IMPLEMENTATION
***************************************************************************/

static void d2_aMpp(void)
{
	double xval = dau_read_pi_double_1st(OP >> 14, 1);
	double yval = dau_read_pi_double_2nd(OP >> 7, 1, xval);
	double res = dsp32.a[(OP >> 26) & 7] + yval * xval;
	int zpi = (OP >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, yval);
	dau_set_val_flags((OP >> 21) & 3, res);
}


static void d2_aMpm(void)
{
	double xval = dau_read_pi_double_1st(OP >> 14, 1);
	double yval = dau_read_pi_double_2nd(OP >> 7, 1, xval);
	double res = dsp32.a[(OP >> 26) & 7] - yval * xval;
	int zpi = (OP >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, yval);
	dau_set_val_flags((OP >> 21) & 3, res);
}


static void d2_aMmp(void)
{
	double xval = dau_read_pi_double_1st(OP >> 14, 1);
	double yval = dau_read_pi_double_2nd(OP >> 7, 1, xval);
	double res = -dsp32.a[(OP >> 26) & 7] + yval * xval;
	int zpi = (OP >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, yval);
	dau_set_val_flags((OP >> 21) & 3, res);
}


static void d2_aMmm(void)
{
	double xval = dau_read_pi_double_1st(OP >> 14, 1);
	double yval = dau_read_pi_double_2nd(OP >> 7, 1, xval);
	double res = -dsp32.a[(OP >> 26) & 7] - yval * xval;
	int zpi = (OP >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, yval);
	dau_set_val_flags((OP >> 21) & 3, res);
}


static void d2_aMppr(void)
{
	unimplemented();
}


static void d2_aMpmr(void)
{
	unimplemented();
}


static void d2_aMmpr(void)
{
	unimplemented();
}


static void d2_aMmmr(void)
{
	unimplemented();
}



/***************************************************************************
    DAU FORM 3 IMPLEMENTATION
***************************************************************************/

static void d3_aMpp(void)
{
	double xval = dau_read_pi_double_1st(OP >> 14, 1);
	double yval = dau_read_pi_double_2nd(OP >> 7, 1, xval);
	double res = dsp32.a[(OP >> 26) & 7] + yval * xval;
	int zpi = (OP >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((OP >> 21) & 3, res);
}


static void d3_aMpm(void)
{
	double xval = dau_read_pi_double_1st(OP >> 14, 1);
	double yval = dau_read_pi_double_2nd(OP >> 7, 1, xval);
	double res = dsp32.a[(OP >> 26) & 7] - yval * xval;
	int zpi = (OP >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((OP >> 21) & 3, res);
}


static void d3_aMmp(void)
{
	double xval = dau_read_pi_double_1st(OP >> 14, 1);
	double yval = dau_read_pi_double_2nd(OP >> 7, 1, xval);
	double res = -dsp32.a[(OP >> 26) & 7] + yval * xval;
	int zpi = (OP >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((OP >> 21) & 3, res);
}


static void d3_aMmm(void)
{
	double xval = dau_read_pi_double_1st(OP >> 14, 1);
	double yval = dau_read_pi_double_2nd(OP >> 7, 1, xval);
	double res = -dsp32.a[(OP >> 26) & 7] - yval * xval;
	int zpi = (OP >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((OP >> 21) & 3, res);
}


static void d3_aMppr(void)
{
	unimplemented();
}


static void d3_aMpmr(void)
{
	unimplemented();
}


static void d3_aMmpr(void)
{
	unimplemented();
}


static void d3_aMmmr(void)
{
	unimplemented();
}



/***************************************************************************
    DAU FORM 4 IMPLEMENTATION
***************************************************************************/

static void d4_pp(void)
{
	double xval = dau_read_pi_double_1st(OP >> 14, 1);
	double yval = dau_read_pi_double_2nd(OP >> 7, 0, xval);
	double res = yval + xval;
	int zpi = (OP >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, yval);
	dau_set_val_flags((OP >> 21) & 3, res);
}


static void d4_pm(void)
{
	double xval = dau_read_pi_double_1st(OP >> 14, 1);
	double yval = dau_read_pi_double_2nd(OP >> 7, 0, xval);
	double res = yval - xval;
	int zpi = (OP >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, yval);
	dau_set_val_flags((OP >> 21) & 3, res);
}


static void d4_mp(void)
{
	double xval = dau_read_pi_double_1st(OP >> 14, 1);
	double yval = dau_read_pi_double_2nd(OP >> 7, 0, xval);
	double res = -yval + xval;
	int zpi = (OP >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, yval);
	dau_set_val_flags((OP >> 21) & 3, res);
}


static void d4_mm(void)
{
	double xval = dau_read_pi_double_1st(OP >> 14, 1);
	double yval = dau_read_pi_double_2nd(OP >> 7, 0, xval);
	double res = -yval - xval;
	int zpi = (OP >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, yval);
	dau_set_val_flags((OP >> 21) & 3, res);
}


static void d4_ppr(void)
{
	unimplemented();
}


static void d4_pmr(void)
{
	unimplemented();
}


static void d4_mpr(void)
{
	unimplemented();
}


static void d4_mmr(void)
{
	unimplemented();
}



/***************************************************************************
    DAU FORM 5 IMPLEMENTATION
***************************************************************************/

static void d5_ic(void)
{
	unimplemented();
}


static void d5_oc(void)
{
	unimplemented();
}


static void d5_float(void)
{
	double res = (double)(INT16)dau_read_pi_2bytes(OP >> 7);
	int zpi = (OP >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((OP >> 21) & 3, res);
}


static void d5_int(void)
{
	double val = dau_read_pi_double_1st(OP >> 7, 0);
	int zpi = (OP >> 0) & 0x7f;
	INT16 res;
	if (!(dsp32.DAUC & 0x10)) val = floor(val + 0.5);
	else val = ceil(val - 0.5);
	res = (INT16)val;
	if (zpi != 7)
		dau_write_pi_2bytes(zpi, res);
	dau_set_val_noflags((OP >> 21) & 3, dsp_to_double(res << 16));
}


static void d5_round(void)
{
	double res = (double)(float)dau_read_pi_double_1st(OP >> 7, 0);
	int zpi = (OP >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((OP >> 21) & 3, res);
}


static void d5_ifalt(void)
{
	int ar = (OP >> 21) & 3;
	double res = dsp32.a[ar];
	int zpi = (OP >> 0) & 0x7f;
	if (NFLAG)
		res = dau_read_pi_double_1st(OP >> 7, 0);
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_noflags(ar, res);
}


static void d5_ifaeq(void)
{
	int ar = (OP >> 21) & 3;
	double res = dsp32.a[ar];
	int zpi = (OP >> 0) & 0x7f;
	if (ZFLAG)
		res = dau_read_pi_double_1st(OP >> 7, 0);
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_noflags(ar, res);
}


static void d5_ifagt(void)
{
	int ar = (OP >> 21) & 3;
	double res = dsp32.a[ar];
	int zpi = (OP >> 0) & 0x7f;
	if (!NFLAG && !ZFLAG)
		res = dau_read_pi_double_1st(OP >> 7, 0);
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_noflags(ar, res);
}


static void d5_float24(void)
{
	double res = (double)((INT32)(dau_read_pi_4bytes(OP >> 7) << 8) >> 8);
	int zpi = (OP >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(zpi, res);
	dau_set_val_flags((OP >> 21) & 3, res);
}


static void d5_int24(void)
{
	double val = dau_read_pi_double_1st(OP >> 7, 0);
	int zpi = (OP >> 0) & 0x7f;
	INT32 res;
	if (!(dsp32.DAUC & 0x10)) val = floor(val + 0.5);
	else val = ceil(val - 0.5);
	res = (INT32)val;
	if (res > 0x7fffff) res = 0x7fffff;
	else if (res < -0x800000) res = -0x800000;
	if (zpi != 7)
		dau_write_pi_4bytes(zpi, (INT32)(res << 8) >> 8);
	dau_set_val_noflags((OP >> 21) & 3, dsp_to_double(res << 8));
}


static void d5_ieee(void)
{
	unimplemented();
}


static void d5_dsp(void)
{
	unimplemented();
}


static void d5_seed(void)
{
	UINT32 val = dau_read_pi_4bytes(OP >> 7);
	INT32 res = val ^ 0x7fffffff;
	int zpi = (OP >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_4bytes(zpi, res);
	dau_set_val_flags((OP >> 21) & 3, dsp_to_double((INT32)res));
}



/***************************************************************************
    FUNCTION TABLE
***************************************************************************/

void (*dsp32ops[])(void) =
{
	nop,		goto_t,		goto_pl,	goto_mi,	goto_ne,	goto_eq,	goto_vc,	goto_vs,	/* 00 */
	goto_cc,	goto_cs,	goto_ge,	goto_lt,	goto_gt,	goto_le,	goto_hi,	goto_ls,
	goto_auc,	goto_aus,	goto_age,	goto_alt,	goto_ane,	goto_aeq,	goto_avc,	goto_avs,	/* 01 */
	goto_agt,	goto_ale,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	goto_ibe,	goto_ibf,	goto_obf,	goto_obe,	goto_pde,	goto_pdf,	goto_pie,	goto_pif,	/* 02 */
	goto_syc,	goto_sys,	goto_fbc,	goto_fbs,	goto_irq1lo,goto_irq1hi,goto_irq2lo,goto_irq2hi,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	/* 03 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,

	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	/* 04 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	/* 05 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	dec_goto,	dec_goto,	dec_goto,	dec_goto,	dec_goto,	dec_goto,	dec_goto,	dec_goto,	/* 06 */
	dec_goto,	dec_goto,	dec_goto,	dec_goto,	dec_goto,	dec_goto,	dec_goto,	dec_goto,
	dec_goto,	dec_goto,	dec_goto,	dec_goto,	dec_goto,	dec_goto,	dec_goto,	dec_goto,	/* 07 */
	dec_goto,	dec_goto,	dec_goto,	dec_goto,	dec_goto,	dec_goto,	dec_goto,	dec_goto,

	call,		call,		call,		call,		call,		call,		call,		call,		/* 08 */
	call,		call,		call,		call,		call,		call,		call,		call,
	call,		call,		call,		call,		call,		call,		call,		call,		/* 09 */
	call,		call,		call,		call,		call,		call,		call,		call,
	add_si,		add_si,		add_si,		add_si,		add_si,		add_si,		add_si,		add_si,		/* 0a */
	add_si,		add_si,		add_si,		add_si,		add_si,		add_si,		add_si,		add_si,
	add_si,		add_si,		add_si,		add_si,		add_si,		add_si,		add_si,		add_si,		/* 0b */
	add_si,		add_si,		add_si,		add_si,		add_si,		add_si,		add_si,		add_si,

	add_ss,		mul2_s,		subr_ss,	addr_ss,	sub_ss,		neg_s,		andc_ss,	cmp_ss,		/* 0c */
	xor_ss,		rcr_s,		or_ss,		rcl_s,		shr_s,		div2_s,		and_ss,		test_ss,
	add_di,		illegal,	subr_di,	addr_di,	sub_di,		illegal,	andc_di,	cmp_di,		/* 0d */
	xor_di,		illegal,	or_di,		illegal,	illegal,	illegal,	and_di,		test_di,
	load_hi,	load_hi,	load_li,	load_li,	load_i,		load_i,		load_ei,	load_ei,	/* 0e */
	store_hi,	store_hi,	store_li,	store_li,	store_i,	store_i,	store_ei,	store_ei,
	load_hr,	load_hr,	load_lr,	load_lr,	load_r,		load_r,		load_er,	load_er,	/* 0f */
	store_hr,	store_hr,	store_lr,	store_lr,	store_r,	store_r,	store_er,	store_er,

	d1_aMpp,	d1_aMpp,	d1_aMpp,	d1_aMpp,	d1_aMpm,	d1_aMpm,	d1_aMpm,	d1_aMpm,	/* 10 */
	d1_aMmp,	d1_aMmp,	d1_aMmp,	d1_aMmp,	d1_aMmm,	d1_aMmm,	d1_aMmm,	d1_aMmm,
	d1_aMppr,	d1_aMppr,	d1_aMppr,	d1_aMppr,	d1_aMpmr,	d1_aMpmr,	d1_aMpmr,	d1_aMpmr,	/* 11 */
	d1_aMmpr,	d1_aMmpr,	d1_aMmpr,	d1_aMmpr,	d1_aMmmr,	d1_aMmmr,	d1_aMmmr,	d1_aMmmr,
	d1_aMpp,	d1_aMpp,	d1_aMpp,	d1_aMpp,	d1_aMpm,	d1_aMpm,	d1_aMpm,	d1_aMpm,	/* 12 */
	d1_aMmp,	d1_aMmp,	d1_aMmp,	d1_aMmp,	d1_aMmm,	d1_aMmm,	d1_aMmm,	d1_aMmm,
	d1_aMppr,	d1_aMppr,	d1_aMppr,	d1_aMppr,	d1_aMpmr,	d1_aMpmr,	d1_aMpmr,	d1_aMpmr,	/* 13 */
	d1_aMmpr,	d1_aMmpr,	d1_aMmpr,	d1_aMmpr,	d1_aMmmr,	d1_aMmmr,	d1_aMmmr,	d1_aMmmr,

	d1_aMpp,	d1_aMpp,	d1_aMpp,	d1_aMpp,	d1_aMpm,	d1_aMpm,	d1_aMpm,	d1_aMpm,	/* 14 */
	d1_aMmp,	d1_aMmp,	d1_aMmp,	d1_aMmp,	d1_aMmm,	d1_aMmm,	d1_aMmm,	d1_aMmm,
	d1_aMppr,	d1_aMppr,	d1_aMppr,	d1_aMppr,	d1_aMpmr,	d1_aMpmr,	d1_aMpmr,	d1_aMpmr,	/* 15 */
	d1_aMmpr,	d1_aMmpr,	d1_aMmpr,	d1_aMmpr,	d1_aMmmr,	d1_aMmmr,	d1_aMmmr,	d1_aMmmr,
	d1_aMpp,	d1_aMpp,	d1_aMpp,	d1_aMpp,	d1_aMpm,	d1_aMpm,	d1_aMpm,	d1_aMpm,	/* 16 */
	d1_aMmp,	d1_aMmp,	d1_aMmp,	d1_aMmp,	d1_aMmm,	d1_aMmm,	d1_aMmm,	d1_aMmm,
	d1_aMppr,	d1_aMppr,	d1_aMppr,	d1_aMppr,	d1_aMpmr,	d1_aMpmr,	d1_aMpmr,	d1_aMpmr,	/* 17 */
	d1_aMmpr,	d1_aMmpr,	d1_aMmpr,	d1_aMmpr,	d1_aMmmr,	d1_aMmmr,	d1_aMmmr,	d1_aMmmr,

	d1_0px,		d1_0px,		d1_0px,		d1_0px,		d1_0px,		d1_0px,		d1_0px,		d1_0px,		/* 18 */
	d1_0mx,		d1_0mx,		d1_0mx,		d1_0mx,		d1_0mx,		d1_0mx,		d1_0mx,		d1_0mx,
	d1_aMppr,	d1_aMppr,	d1_aMppr,	d1_aMppr,	d1_aMpmr,	d1_aMpmr,	d1_aMpmr,	d1_aMpmr,	/* 19 */
	d1_aMmpr,	d1_aMmpr,	d1_aMmpr,	d1_aMmpr,	d1_aMmmr,	d1_aMmmr,	d1_aMmmr,	d1_aMmmr,
	d1_1pp,		d1_1pp,		d1_1pp,		d1_1pp,		d1_1pm,		d1_1pm,		d1_1pm,		d1_1pm,		/* 1a */
	d1_1mp,		d1_1mp,		d1_1mp,		d1_1mp,		d1_1mm,		d1_1mm,		d1_1mm,		d1_1mm,
	d1_aMppr,	d1_aMppr,	d1_aMppr,	d1_aMppr,	d1_aMpmr,	d1_aMpmr,	d1_aMpmr,	d1_aMpmr,	/* 1b */
	d1_aMmpr,	d1_aMmpr,	d1_aMmpr,	d1_aMmpr,	d1_aMmmr,	d1_aMmmr,	d1_aMmmr,	d1_aMmmr,

	d4_pp,		d4_pp,		d4_pp,		d4_pp,		d4_pm,		d4_pm,		d4_pm,		d4_pm,		/* 1c */
	d4_mp,		d4_mp,		d4_mp,		d4_mp,		d4_mm,		d4_mm,		d4_mm,		d4_mm,
	d4_ppr,		d4_ppr,		d4_ppr,		d4_ppr,		d4_pmr,		d4_pmr,		d4_pmr,		d4_pmr,		/* 1d */
	d4_mpr,		d4_mpr,		d4_mpr,		d4_mpr,		d4_mmr,		d4_mmr,		d4_mmr,		d4_mmr,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	/* 1e */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	/* 1f */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,

	d2_aMpp,	d2_aMpp,	d2_aMpp,	d2_aMpp,	d2_aMpm,	d2_aMpm,	d2_aMpm,	d2_aMpm,	/* 20 */
	d2_aMmp,	d2_aMmp,	d2_aMmp,	d2_aMmp,	d2_aMmm,	d2_aMmm,	d2_aMmm,	d2_aMmm,
	d2_aMppr,	d2_aMppr,	d2_aMppr,	d2_aMppr,	d2_aMpmr,	d2_aMpmr,	d2_aMpmr,	d2_aMpmr,	/* 21 */
	d2_aMmpr,	d2_aMmpr,	d2_aMmpr,	d2_aMmpr,	d2_aMmmr,	d2_aMmmr,	d2_aMmmr,	d2_aMmmr,
	d2_aMpp,	d2_aMpp,	d2_aMpp,	d2_aMpp,	d2_aMpm,	d2_aMpm,	d2_aMpm,	d2_aMpm,	/* 22 */
	d2_aMmp,	d2_aMmp,	d2_aMmp,	d2_aMmp,	d2_aMmm,	d2_aMmm,	d2_aMmm,	d2_aMmm,
	d2_aMppr,	d2_aMppr,	d2_aMppr,	d2_aMppr,	d2_aMpmr,	d2_aMpmr,	d2_aMpmr,	d2_aMpmr,	/* 23 */
	d2_aMmpr,	d2_aMmpr,	d2_aMmpr,	d2_aMmpr,	d2_aMmmr,	d2_aMmmr,	d2_aMmmr,	d2_aMmmr,

	d2_aMpp,	d2_aMpp,	d2_aMpp,	d2_aMpp,	d2_aMpm,	d2_aMpm,	d2_aMpm,	d2_aMpm,	/* 24 */
	d2_aMmp,	d2_aMmp,	d2_aMmp,	d2_aMmp,	d2_aMmm,	d2_aMmm,	d2_aMmm,	d2_aMmm,
	d2_aMppr,	d2_aMppr,	d2_aMppr,	d2_aMppr,	d2_aMpmr,	d2_aMpmr,	d2_aMpmr,	d2_aMpmr,	/* 25 */
	d2_aMmpr,	d2_aMmpr,	d2_aMmpr,	d2_aMmpr,	d2_aMmmr,	d2_aMmmr,	d2_aMmmr,	d2_aMmmr,
	d2_aMpp,	d2_aMpp,	d2_aMpp,	d2_aMpp,	d2_aMpm,	d2_aMpm,	d2_aMpm,	d2_aMpm,	/* 26 */
	d2_aMmp,	d2_aMmp,	d2_aMmp,	d2_aMmp,	d2_aMmm,	d2_aMmm,	d2_aMmm,	d2_aMmm,
	d2_aMppr,	d2_aMppr,	d2_aMppr,	d2_aMppr,	d2_aMpmr,	d2_aMpmr,	d2_aMpmr,	d2_aMpmr,	/* 27 */
	d2_aMmpr,	d2_aMmpr,	d2_aMmpr,	d2_aMmpr,	d2_aMmmr,	d2_aMmmr,	d2_aMmmr,	d2_aMmmr,

	d2_aMpp,	d2_aMpp,	d2_aMpp,	d2_aMpp,	d2_aMpm,	d2_aMpm,	d2_aMpm,	d2_aMpm,	/* 28 */
	d2_aMmp,	d2_aMmp,	d2_aMmp,	d2_aMmp,	d2_aMmm,	d2_aMmm,	d2_aMmm,	d2_aMmm,
	d2_aMppr,	d2_aMppr,	d2_aMppr,	d2_aMppr,	d2_aMpmr,	d2_aMpmr,	d2_aMpmr,	d2_aMpmr,	/* 29 */
	d2_aMmpr,	d2_aMmpr,	d2_aMmpr,	d2_aMmpr,	d2_aMmmr,	d2_aMmmr,	d2_aMmmr,	d2_aMmmr,
	d2_aMpp,	d2_aMpp,	d2_aMpp,	d2_aMpp,	d2_aMpm,	d2_aMpm,	d2_aMpm,	d2_aMpm,	/* 2a */
	d2_aMmp,	d2_aMmp,	d2_aMmp,	d2_aMmp,	d2_aMmm,	d2_aMmm,	d2_aMmm,	d2_aMmm,
	d2_aMppr,	d2_aMppr,	d2_aMppr,	d2_aMppr,	d2_aMpmr,	d2_aMpmr,	d2_aMpmr,	d2_aMpmr,	/* 2b */
	d2_aMmpr,	d2_aMmpr,	d2_aMmpr,	d2_aMmpr,	d2_aMmmr,	d2_aMmmr,	d2_aMmmr,	d2_aMmmr,

	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	/* 2c */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	/* 2d */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	/* 2e */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	/* 2f */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,

	d3_aMpp,	d3_aMpp,	d3_aMpp,	d3_aMpp,	d3_aMpm,	d3_aMpm,	d3_aMpm,	d3_aMpm,	/* 30 */
	d3_aMmp,	d3_aMmp,	d3_aMmp,	d3_aMmp,	d3_aMmm,	d3_aMmm,	d3_aMmm,	d3_aMmm,
	d3_aMppr,	d3_aMppr,	d3_aMppr,	d3_aMppr,	d3_aMpmr,	d3_aMpmr,	d3_aMpmr,	d3_aMpmr,	/* 31 */
	d3_aMmpr,	d3_aMmpr,	d3_aMmpr,	d3_aMmpr,	d3_aMmmr,	d3_aMmmr,	d3_aMmmr,	d3_aMmmr,
	d3_aMpp,	d3_aMpp,	d3_aMpp,	d3_aMpp,	d3_aMpm,	d3_aMpm,	d3_aMpm,	d3_aMpm,	/* 32 */
	d3_aMmp,	d3_aMmp,	d3_aMmp,	d3_aMmp,	d3_aMmm,	d3_aMmm,	d3_aMmm,	d3_aMmm,
	d3_aMppr,	d3_aMppr,	d3_aMppr,	d3_aMppr,	d3_aMpmr,	d3_aMpmr,	d3_aMpmr,	d3_aMpmr,	/* 33 */
	d3_aMmpr,	d3_aMmpr,	d3_aMmpr,	d3_aMmpr,	d3_aMmmr,	d3_aMmmr,	d3_aMmmr,	d3_aMmmr,

	d3_aMpp,	d3_aMpp,	d3_aMpp,	d3_aMpp,	d3_aMpm,	d3_aMpm,	d3_aMpm,	d3_aMpm,	/* 34 */
	d3_aMmp,	d3_aMmp,	d3_aMmp,	d3_aMmp,	d3_aMmm,	d3_aMmm,	d3_aMmm,	d3_aMmm,
	d3_aMppr,	d3_aMppr,	d3_aMppr,	d3_aMppr,	d3_aMpmr,	d3_aMpmr,	d3_aMpmr,	d3_aMpmr,	/* 35 */
	d3_aMmpr,	d3_aMmpr,	d3_aMmpr,	d3_aMmpr,	d3_aMmmr,	d3_aMmmr,	d3_aMmmr,	d3_aMmmr,
	d3_aMpp,	d3_aMpp,	d3_aMpp,	d3_aMpp,	d3_aMpm,	d3_aMpm,	d3_aMpm,	d3_aMpm,	/* 36 */
	d3_aMmp,	d3_aMmp,	d3_aMmp,	d3_aMmp,	d3_aMmm,	d3_aMmm,	d3_aMmm,	d3_aMmm,
	d3_aMppr,	d3_aMppr,	d3_aMppr,	d3_aMppr,	d3_aMpmr,	d3_aMpmr,	d3_aMpmr,	d3_aMpmr,	/* 37 */
	d3_aMmpr,	d3_aMmpr,	d3_aMmpr,	d3_aMmpr,	d3_aMmmr,	d3_aMmmr,	d3_aMmmr,	d3_aMmmr,

	d3_aMpp,	d3_aMpp,	d3_aMpp,	d3_aMpp,	d3_aMpm,	d3_aMpm,	d3_aMpm,	d3_aMpm,	/* 38 */
	d3_aMmp,	d3_aMmp,	d3_aMmp,	d3_aMmp,	d3_aMmm,	d3_aMmm,	d3_aMmm,	d3_aMmm,
	d3_aMppr,	d3_aMppr,	d3_aMppr,	d3_aMppr,	d3_aMpmr,	d3_aMpmr,	d3_aMpmr,	d3_aMpmr,	/* 39 */
	d3_aMmpr,	d3_aMmpr,	d3_aMmpr,	d3_aMmpr,	d3_aMmmr,	d3_aMmmr,	d3_aMmmr,	d3_aMmmr,
	d3_aMpp,	d3_aMpp,	d3_aMpp,	d3_aMpp,	d3_aMpm,	d3_aMpm,	d3_aMpm,	d3_aMpm,	/* 3a */
	d3_aMmp,	d3_aMmp,	d3_aMmp,	d3_aMmp,	d3_aMmm,	d3_aMmm,	d3_aMmm,	d3_aMmm,
	d3_aMppr,	d3_aMppr,	d3_aMppr,	d3_aMppr,	d3_aMpmr,	d3_aMpmr,	d3_aMpmr,	d3_aMpmr,	/* 3b */
	d3_aMmpr,	d3_aMmpr,	d3_aMmpr,	d3_aMmpr,	d3_aMmmr,	d3_aMmmr,	d3_aMmmr,	d3_aMmmr,

	d5_ic,		d5_ic,		d5_ic,		d5_ic,		d5_oc,		d5_oc,		d5_oc,		d5_oc,		/* 3c */
	d5_float,	d5_float,	d5_float,	d5_float,	d5_int,		d5_int,		d5_int,		d5_int,
	d5_round,	d5_round,	d5_round,	d5_round,	d5_ifalt,	d5_ifalt,	d5_ifalt,	d5_ifalt,	/* 3d */
	d5_ifaeq,	d5_ifaeq,	d5_ifaeq,	d5_ifaeq,	d5_ifagt,	d5_ifagt,	d5_ifagt,	d5_ifagt,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	/* 3e */
	d5_float24,	d5_float24,	d5_float24,	d5_float24,	d5_int24,	d5_int24,	d5_int24,	d5_int24,
	d5_ieee,	d5_ieee,	d5_ieee,	d5_ieee,	d5_dsp,		d5_dsp,		d5_dsp,		d5_dsp,		/* 3f */
	d5_seed,	d5_seed,	d5_seed,	d5_seed,	illegal,	illegal,	illegal,	illegal,

	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	/* 40 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	/* 41 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	/* 42 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	/* 43 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,

	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	/* 44 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	/* 45 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	do_i,		do_r,		illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	/* 46 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	/* 47 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,

	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	/* 48 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	/* 49 */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	adde_si,	adde_si,	adde_si,	adde_si,	adde_si,	adde_si,	adde_si,	adde_si,	/* 4a */
	adde_si,	adde_si,	adde_si,	adde_si,	adde_si,	adde_si,	adde_si,	adde_si,
	adde_si,	adde_si,	adde_si,	adde_si,	adde_si,	adde_si,	adde_si,	adde_si,	/* 4b */
	adde_si,	adde_si,	adde_si,	adde_si,	adde_si,	adde_si,	adde_si,	adde_si,

	adde_ss,	mul2e_s,	subre_ss,	addre_ss,	sube_ss,	nege_s,		andce_ss,	cmpe_ss,	/* 4c */
	xore_ss,	rcre_s,		ore_ss,		rcle_s,		shre_s,		div2e_s,	ande_ss,	teste_ss,
	adde_di,	illegal,	subre_di,	addre_di,	sube_di,	illegal,	andce_di,	cmpe_di,	/* 4d */
	xore_di,	illegal,	ore_di,		illegal,	illegal,	illegal,	ande_di,	teste_di,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	/* 4e */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	/* 4f */
	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,

	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		/* 50 */
	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,
	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		/* 51 */
	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,
	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		/* 52 */
	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,
	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		/* 53 */
	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,

	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		/* 54 */
	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,
	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		/* 55 */
	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,
	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		/* 56 */
	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,
	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		/* 57 */
	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,

	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		/* 58 */
	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,
	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		/* 59 */
	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,
	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		/* 5a */
	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,
	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		/* 5b */
	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,

	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		/* 5c */
	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,
	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		/* 5d */
	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,
	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		/* 5e */
	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,
	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		/* 5f */
	goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,		goto24,

	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,		/* 60 */
	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,
	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,		/* 61 */
	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,
	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,		/* 62 */
	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,
	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,		/* 63 */
	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,

	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,		/* 64 */
	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,
	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,		/* 65 */
	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,
	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,		/* 66 */
	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,
	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,		/* 67 */
	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,

	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,		/* 68 */
	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,
	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,		/* 69 */
	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,
	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,		/* 6a */
	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,
	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,		/* 6b */
	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,

	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,		/* 6c */
	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,
	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,		/* 6d */
	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,
	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,		/* 6e */
	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,
	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,		/* 6f */
	load24,		load24,		load24,		load24,		load24,		load24,		load24,		load24,

	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,		/* 70 */
	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,
	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,		/* 71 */
	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,
	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,		/* 72 */
	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,
	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,		/* 73 */
	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,

	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,		/* 74 */
	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,
	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,		/* 75 */
	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,
	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,		/* 76 */
	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,
	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,		/* 77 */
	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,

	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,		/* 78 */
	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,
	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,		/* 79 */
	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,
	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,		/* 7a */
	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,
	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,		/* 7b */
	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,

	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,		/* 7c */
	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,
	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,		/* 7d */
	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,
	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,		/* 7e */
	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,
	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24,		/* 7f */
	call24,		call24,		call24,		call24,		call24,		call24,		call24,		call24
};


/*

    Most common OPs in Race Drivin':

301681217 - OP   0 - nop
164890391 - OP 4A1 - adde_si
99210113 - OP 661 - load24
86010010 - OP  F7 - load_er
61148739 - OP 4D4 - sube_di
52693763 - OP 180 - d1_0px
41525754 - OP  FF - store_er
35033321 - OP 380 - d3_aMpp
31621151 - OP 4C0 - adde_ss
28076244 - OP 660 - load24
19190505 - OP 4C1 - mul2e_s
13270852 - OP  F5 - load_r
12535169 - OP 1A4 - d1_1pm
12265141 - OP 4C4 - sube_ss
10748211 - OP 4CD - div2e_s
10493660 - OP  FD - store_r
 9721263 - OP 189
 9415685 - OP 3C8
 9294148 - OP 3D5
 8887846 - OP 1A1
 8788648 - OP 381
 8185239 - OP 300
 7241256 - OP 383
 6877349 - OP 4A3
 6832295 - OP 181
 6601270 - OP 3E8
 6562483 - OP 4A4
 6553514 - OP 3C9
 6270430 - OP 280
 6041485 - OP 1A0
 5299529 - OP 304
 5110926 - OP 382
 4922253 - OP 363
 4603670 - OP 4D7
 4164327 - OP 4AE
 3980085 - OP 3EC
 3599198 - OP 3CC
 3543878 - OP 3D0
 3489158 - OP   4
 3463235 - OP 321
 3335995 - OP 3F9
 3001546 - OP 4CE
 2882940 - OP 129
 2882940 - OP 1A5
 2882940 - OP 342
 2841981 - OP 360
 2663417 - OP  FB
 2059640 - OP 3ED
 1867166 - OP 1A8
 1830789 - OP 305
 1753312 - OP 301
 1726866 - OP   5
 1594991 - OP  12
 1571286 - OP  19
 1507644 - OP  A2
 1418846 - OP 3CD
 1273134 - OP  F3
 1177914 - OP 4C7
 1175720 - OP 188
 1091848 - OP 3E9
 1088206 - OP 6FF
 1088204 - OP 4CA
 1012639 - OP 101
  939617 - OP 4C5

*/
