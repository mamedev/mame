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

#define SET_V_16(cs,a,b,r)		(cs)->vflags = (((a) ^ (b) ^ (r) ^ ((r) >> 1)) << 8)
#define SET_NZC_16(cs,r)		(cs)->nzcflags = ((r) << 8)
#define SET_NZCV_16(cs,a,b,r)	SET_NZC_16(cs,r); SET_V_16(cs,a,b,r)
#define SET_NZ00_16(cs,r)		(cs)->nzcflags = (((r) << 8) & 0xffffff); (cs)->vflags = 0

#define SET_V_24(cs,a,b,r)		(cs)->vflags = ((a) ^ (b) ^ (r) ^ ((r) >> 1))
#define SET_NZC_24(cs,r)		(cs)->nzcflags = (r)
#define SET_NZCV_24(cs,a,b,r)	SET_NZC_24(cs,r); SET_V_24(cs,a,b,r)
#define SET_NZ00_24(cs,r)		(cs)->nzcflags = ((r) & 0xffffff); (cs)->vflags = 0

#define TRUNCATE24(a)			((a) & 0xffffff)
#define EXTEND16_TO_24(a)		TRUNCATE24((INT32)(INT16)(a))
#define REG16(cs,a)				((UINT16)(cs)->r[a])
#define REG24(cs,a)				((cs)->r[a])

#define WRITEABLE_REGS			(0x6f3efffe)
#if ASSUME_WRITEABLE
#define IS_WRITEABLE(r) 		(1)
#else
#define IS_WRITEABLE(r)			(WRITEABLE_REGS & (1 << (r)))
#endif

#if ASSUME_UNCONDITIONAL_CAU
#define CONDITION_IS_TRUE(cs)	(1)
#else
#define CONDITION_IS_TRUE(cs)	(!(op & 0x400) || (condition(cs, (op >> 12) & 15)))
#endif

#if EMULATE_MEMORY_LATENCY
#define WWORD_DEFERRED(cs,a,v)	do { int bufidx = (cs)->mbuf_index & 3; (cs)->mbufaddr[bufidx] = -(a); (cs)->mbufdata[bufidx] = (v); } while (0)
#define WLONG_DEFERRED(cs,a,v)	do { int bufidx = (cs)->mbuf_index & 3; (cs)->mbufaddr[bufidx] = (a); (cs)->mbufdata[bufidx] = (v); } while (0)
#define PROCESS_DEFERRED_MEMORY(cs) 									\
	if ((cs)->mbufaddr[++(cs)->mbuf_index & 3] != 1)					\
	{																	\
		int bufidx = (cs)->mbuf_index & 3;								\
		if ((cs)->mbufaddr[bufidx] >= 0)								\
			WLONG(cs, (cs)->mbufaddr[bufidx], (cs)->mbufdata[bufidx]);	\
		else															\
			WWORD(cs, -(cs)->mbufaddr[bufidx], (cs)->mbufdata[bufidx]);	\
		(cs)->mbufaddr[bufidx] = 1;										\
	}
#else
#define WWORD_DEFERRED(cs,a,v)	WWORD(cs,a,v)
#define WLONG_DEFERRED(cs,a,v)	WLONG(cs,a,v)
#define PROCESS_DEFERRED_MEMORY(cs)
#endif

#if EMULATE_MULTIPLIER_LATENCY
#define DEFERRED_MULTIPLIER(cs,x)	dau_get_amult(cs, x)
#else
#define DEFERRED_MULTIPLIER(cs,x)	(cs)->a[x]
#endif

#if EMULATE_AFLAGS_LATENCY
#define DEFERRED_NZFLAGS(cs)	dau_get_anzflags(cs)
#define DEFERRED_VUFLAGS(cs)	dau_get_avuflags(cs)
#else
#define DEFERRED_NZFLAGS(cs)	(cs)->NZflags
#define DEFERRED_VUFLAGS(cs)	(cs)->VUflags
#endif



/***************************************************************************
    FORWARD DECLARATIONS
***************************************************************************/

extern void (*const dsp32ops[])(dsp32_state *cpustate, UINT32 op);



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

static void illegal(dsp32_state *cpustate, UINT32 op)
{
}


static void unimplemented(dsp32_state *cpustate, UINT32 op)
{
    fatalerror("Unimplemented op @ %06X: %08X (dis=%02X, tbl=%03X)", cpustate->PC - 4, op, op >> 25, op >> 21);
}


INLINE void execute_one(dsp32_state *cpustate)
{
	UINT32 op;

	PROCESS_DEFERRED_MEMORY(cpustate);
	debugger_instruction_hook(cpustate->device, cpustate->PC);
	op = ROPCODE(cpustate, cpustate->PC);
	cpustate->icount -= 4;	/* 4 clocks per cycle */
	cpustate->PC += 4;
	if (op)
		(*dsp32ops[op >> 21])(cpustate, op);
}



/***************************************************************************
    CAU HELPERS
***************************************************************************/

static UINT32 cau_read_pi_special(dsp32_state *cpustate, UINT8 i)
{
	switch (i)
	{
		case 4:		return cpustate->ibuf;
		case 5:		return cpustate->obuf;
		case 6:		update_pcr(cpustate, cpustate->pcr & ~PCR_PDFs); return cpustate->pdr;
		case 14:	return cpustate->piop;
		case 20:	return cpustate->pdr2;
		case 22:	update_pcr(cpustate, cpustate->pcr & ~PCR_PIFs); return cpustate->pir;
		case 30:	return cpustate->pcw;
		default:	fprintf(stderr, "Unimplemented CAU PI read = %X\n", i);
	}
	return 0;
}


static void cau_write_pi_special(dsp32_state *cpustate, UINT8 i, UINT32 val)
{
	switch (i)
	{
		case 4:		cpustate->ibuf = val;	break;
		case 5:		cpustate->obuf = val;	break;
		case 6:		cpustate->pdr = val; update_pcr(cpustate, cpustate->pcr | PCR_PDFs); break;
		case 14:	cpustate->piop = val;	break;
		case 20:	cpustate->pdr2 = val;	break;
		case 22:	cpustate->pir = val; update_pcr(cpustate, cpustate->pcr | PCR_PIFs); break;
		case 30:	cpustate->pcw = val;	break;
		default:	fprintf(stderr, "Unimplemented CAU PI write = %X\n", i);
	}
}


INLINE UINT8 cau_read_pi_1byte(dsp32_state *cpustate, int pi)
{
	int p = (pi >> 5) & 0x1f;
	int i = (pi >> 0) & 0x1f;
	if (p)
	{
		UINT32 result = RBYTE(cpustate, cpustate->r[p]);
		cpustate->r[p] = TRUNCATE24(cpustate->r[p] + cpustate->r[i]);
		return result;
	}
	else
		return cau_read_pi_special(cpustate, i);
}


INLINE UINT16 cau_read_pi_2byte(dsp32_state *cpustate, int pi)
{
	int p = (pi >> 5) & 0x1f;
	int i = (pi >> 0) & 0x1f;
	if (p)
	{
		UINT32 result = RWORD(cpustate, cpustate->r[p]);
		if (i < 22 || i > 23)
			cpustate->r[p] = TRUNCATE24(cpustate->r[p] + cpustate->r[i]);
		else
			cpustate->r[p] = TRUNCATE24(cpustate->r[p] + cpustate->r[i] * 2);
		return result;
	}
	else
		return cau_read_pi_special(cpustate, i);
}


INLINE UINT32 cau_read_pi_4byte(dsp32_state *cpustate, int pi)
{
	int p = (pi >> 5) & 0x1f;
	int i = (pi >> 0) & 0x1f;
	if (p)
	{
		UINT32 result = RLONG(cpustate, cpustate->r[p]);
		if (i < 22 || i > 23)
			cpustate->r[p] = TRUNCATE24(cpustate->r[p] + cpustate->r[i]);
		else
			cpustate->r[p] = TRUNCATE24(cpustate->r[p] + cpustate->r[i] * 4);
		return result;
	}
	else
		return cau_read_pi_special(cpustate, i);
}


INLINE void cau_write_pi_1byte(dsp32_state *cpustate, int pi, UINT8 val)
{
	int p = (pi >> 5) & 0x1f;
	int i = (pi >> 0) & 0x1f;
	if (p)
	{
		WBYTE(cpustate, cpustate->r[p], val);
		cpustate->r[p] = TRUNCATE24(cpustate->r[p] + cpustate->r[i]);
	}
	else
		cau_write_pi_special(cpustate, i, val);
}


INLINE void cau_write_pi_2byte(dsp32_state *cpustate, int pi, UINT16 val)
{
	int p = (pi >> 5) & 0x1f;
	int i = (pi >> 0) & 0x1f;
	if (p)
	{
		WWORD(cpustate, cpustate->r[p], val);
		if (i < 22 || i > 23)
			cpustate->r[p] = TRUNCATE24(cpustate->r[p] + cpustate->r[i]);
		else
			cpustate->r[p] = TRUNCATE24(cpustate->r[p] + cpustate->r[i] * 2);
	}
	else
		cau_write_pi_special(cpustate, i, val);
}


INLINE void cau_write_pi_4byte(dsp32_state *cpustate, int pi, UINT32 val)
{
	int p = (pi >> 5) & 0x1f;
	int i = (pi >> 0) & 0x1f;
	if (p)
	{
		WLONG(cpustate, cpustate->r[p], (INT32)(val << 8) >> 8);
		if (i < 22 || i > 23)
			cpustate->r[p] = TRUNCATE24(cpustate->r[p] + cpustate->r[i]);
		else
			cpustate->r[p] = TRUNCATE24(cpustate->r[p] + cpustate->r[i] * 4);
	}
	else
		cau_write_pi_special(cpustate, i, val);
}



/***************************************************************************
    DAU HELPERS
***************************************************************************/

INLINE double dau_get_amult(dsp32_state *cpustate, int aidx)
{
	int bufidx = (cpustate->abuf_index - 1) & 3;
	double val = cpustate->a[aidx];
	while (cpustate->icount >= cpustate->abufcycle[bufidx] - 2 * 4)
	{
		if (cpustate->abufreg[bufidx] == aidx)
			val = cpustate->abuf[bufidx];
		bufidx = (bufidx - 1) & 3;
	}
	return val;
}


INLINE double dau_get_anzflags(dsp32_state *cpustate)
{
	int bufidx = (cpustate->abuf_index - 1) & 3;
	double nzflags = cpustate->NZflags;
	while (cpustate->icount >= cpustate->abufcycle[bufidx] - 3 * 4)
	{
		nzflags = cpustate->abufNZflags[bufidx];
		bufidx = (bufidx - 1) & 3;
	}
	return nzflags;
}


INLINE UINT8 dau_get_avuflags(dsp32_state *cpustate)
{
#if (!IGNORE_DAU_UV_FLAGS)
	int bufidx = (cpustate->abuf_index - 1) & 3;
	UINT8 vuflags = cpustate->VUflags;
	while (cpustate->icount >= cpustate->abufcycle[bufidx] - 3 * 4)
	{
		vuflags = cpustate->abufVUflags[bufidx];
		bufidx = (bufidx - 1) & 3;
	}
	return vuflags;
#else
	return 0;
#endif
}


INLINE void remember_last_dau(dsp32_state *cpustate, int aidx)
{
#if (EMULATE_MULTIPLIER_LATENCY || EMULATE_AFLAGS_LATENCY)
	int bufidx = cpustate->abuf_index++ & 3;
	cpustate->abuf[bufidx] = cpustate->a[aidx];
	cpustate->abufreg[bufidx] = aidx;
	cpustate->abufNZflags[bufidx] = cpustate->NZflags;
#if (!IGNORE_DAU_UV_FLAGS)
	cpustate->abufVUflags[bufidx] = cpustate->VUflags;
#endif
	cpustate->abufcycle[bufidx] = cpustate->icount;
#endif
}


INLINE void dau_set_val_noflags(dsp32_state *cpustate, int aidx, double res)
{
	remember_last_dau(cpustate, aidx);
	cpustate->a[aidx] = res;
}


INLINE void dau_set_val_flags(dsp32_state *cpustate, int aidx, double res)
{
	remember_last_dau(cpustate, aidx);
#if (!IGNORE_DAU_UV_FLAGS)
{
	double absres = (res < 0) ? -res : res;
	cpustate->VUflags = 0;
	if (absres < 5.87747e-39)
	{
		if (absres != 0)
			cpustate->VUflags = UFLAGBIT;
		res = 0.0;
	}
	else if (absres > 3.40282e38)
	{
		cpustate->VUflags = VFLAGBIT;
//      debugger_break(Machine);
//      fprintf(stderr, "Result = %g\n", absres);
		res = (res < 0) ? -3.40282e38 : 3.40282e38;
	}
}
#endif
	cpustate->NZflags = res;
	cpustate->a[aidx] = res;
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


static double dau_read_pi_special(dsp32_state *cpustate, int i)
{
    fatalerror("Unimplemented dau_read_pi_special(%d)", i);
	return 0;
}


static void dau_write_pi_special(dsp32_state *cpustate, int i, double val)
{
    fatalerror("Unimplemented dau_write_pi_special(%d)", i);
}


static int lastp;

INLINE double dau_read_pi_double_1st(dsp32_state *cpustate, int pi, int multiplier)
{
	int p = (pi >> 3) & 15;
	int i = (pi >> 0) & 7;

	lastp = p;
	if (p)
	{
		UINT32 result = RLONG(cpustate, cpustate->r[p]);
		if (i < 6)
			cpustate->r[p] = TRUNCATE24(cpustate->r[p] + cpustate->r[i+16]);
		else
			cpustate->r[p] = TRUNCATE24(cpustate->r[p] + cpustate->r[i+16] * 4);
		return dsp_to_double(result);
	}
	else if (i < 4)
		return multiplier ? DEFERRED_MULTIPLIER(cpustate, i) : cpustate->a[i];
	else
		return dau_read_pi_special(cpustate, i);
}


INLINE double dau_read_pi_double_2nd(dsp32_state *cpustate, int pi, int multiplier, double xval)
{
	int p = (pi >> 3) & 15;
	int i = (pi >> 0) & 7;

	if (p == 15) p = lastp;		/* P=15 means Z inherits from Y, Y inherits from X */
	lastp = p;
	if (p)
	{
		UINT32 result;
		result = RLONG(cpustate, cpustate->r[p]);
		if (i < 6)
			cpustate->r[p] = TRUNCATE24(cpustate->r[p] + cpustate->r[i+16]);
		else
			cpustate->r[p] = TRUNCATE24(cpustate->r[p] + cpustate->r[i+16] * 4);
		return dsp_to_double(result);
	}
	else if (i < 4)
		return multiplier ? DEFERRED_MULTIPLIER(cpustate, i) : cpustate->a[i];
	else
		return dau_read_pi_special(cpustate, i);
}


INLINE UINT32 dau_read_pi_4bytes(dsp32_state *cpustate, int pi)
{
	int p = (pi >> 3) & 15;
	int i = (pi >> 0) & 7;

	lastp = p;
	if (p)
	{
		UINT32 result = RLONG(cpustate, cpustate->r[p]);
		if (i < 6)
			cpustate->r[p] = TRUNCATE24(cpustate->r[p] + cpustate->r[i+16]);
		else
			cpustate->r[p] = TRUNCATE24(cpustate->r[p] + cpustate->r[i+16] * 4);
		return result;
	}
	else if (i < 4)
		return double_to_dsp(cpustate->a[i]);
	else
		return dau_read_pi_special(cpustate, i);
}


INLINE UINT16 dau_read_pi_2bytes(dsp32_state *cpustate, int pi)
{
	int p = (pi >> 3) & 15;
	int i = (pi >> 0) & 7;

	lastp = p;
	if (p)
	{
		UINT32 result = RWORD(cpustate, cpustate->r[p]);
		if (i < 6)
			cpustate->r[p] = TRUNCATE24(cpustate->r[p] + cpustate->r[i+16]);
		else
			cpustate->r[p] = TRUNCATE24(cpustate->r[p] + cpustate->r[i+16] * 2);
		return result;
	}
	else if (i < 4)
		return double_to_dsp(cpustate->a[i]);
	else
		return dau_read_pi_special(cpustate, i);
}


INLINE void dau_write_pi_double(dsp32_state *cpustate, int pi, double val)
{
	int p = (pi >> 3) & 15;
	int i = (pi >> 0) & 7;

	if (p == 15) p = lastp;		/* P=15 means Z inherits from Y, Y inherits from X */
	if (p)
	{
		WLONG_DEFERRED(cpustate, cpustate->r[p], double_to_dsp(val));
		if (i < 6)
			cpustate->r[p] = TRUNCATE24(cpustate->r[p] + cpustate->r[i+16]);
		else
			cpustate->r[p] = TRUNCATE24(cpustate->r[p] + cpustate->r[i+16] * 4);
	}
	else if (i < 4)
		dau_set_val_noflags(cpustate, i, val);
	else
		dau_write_pi_special(cpustate, i, val);
}


INLINE void dau_write_pi_4bytes(dsp32_state *cpustate, int pi, UINT32 val)
{
	int p = (pi >> 3) & 15;
	int i = (pi >> 0) & 7;

	if (p == 15) p = lastp;		/* P=15 means Z inherits from Y, Y inherits from X */
	if (p)
	{
		lastp = p;
		WLONG_DEFERRED(cpustate, cpustate->r[p], val);
		if (i < 6)
			cpustate->r[p] = TRUNCATE24(cpustate->r[p] + cpustate->r[i+16]);
		else
			cpustate->r[p] = TRUNCATE24(cpustate->r[p] + cpustate->r[i+16] * 4);
	}
	else if (i < 4)
		dau_set_val_noflags(cpustate, i, dsp_to_double(val));
	else
		dau_write_pi_special(cpustate, i, val);
}


INLINE void dau_write_pi_2bytes(dsp32_state *cpustate, int pi, UINT16 val)
{
	int p = (pi >> 3) & 15;
	int i = (pi >> 0) & 7;

	if (p == 15) p = lastp;		/* P=15 means Z inherits from Y, Y inherits from X */
	if (p)
	{
		lastp = p;
		WWORD_DEFERRED(cpustate, cpustate->r[p], val);
		if (i < 6)
			cpustate->r[p] = TRUNCATE24(cpustate->r[p] + cpustate->r[i+16]);
		else
			cpustate->r[p] = TRUNCATE24(cpustate->r[p] + cpustate->r[i+16] * 2);
	}
	else if (i < 4)
		dau_set_val_noflags(cpustate, i, dsp_to_double(val << 16));
	else
		dau_write_pi_special(cpustate, i, val);
}



/***************************************************************************
    COMMON CONDITION ROUTINE
***************************************************************************/

#if (!ASSUME_UNCONDITIONAL_CAU)
static int condition(dsp32_state *cpustate, int cond)
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
			return !(DEFERRED_VUFLAGS(cpustate) & UFLAGBIT);
		case 17:
			return (DEFERRED_VUFLAGS(cpustate) & UFLAGBIT);
		case 18:
			return !(DEFERRED_NZFLAGS(cpustate) < 0);
		case 19:
			return (DEFERRED_NZFLAGS(cpustate) < 0);
		case 20:
			return !(DEFERRED_NZFLAGS(cpustate) == 0);
		case 21:
			return (DEFERRED_NZFLAGS(cpustate) == 0);
		case 22:
			return !(DEFERRED_VUFLAGS(cpustate) & VFLAGBIT);
		case 23:
			return (DEFERRED_VUFLAGS(cpustate) & VFLAGBIT);
		case 24:
			return !(DEFERRED_NZFLAGS(cpustate) <= 0);
		case 25:
			return (DEFERRED_NZFLAGS(cpustate) <= 0);

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

static void nop(dsp32_state *cpustate, UINT32 op)
{
	if (op == 0)
		return;
	execute_one(cpustate);
	cpustate->PC = TRUNCATE24(REG24(cpustate, (op >> 16) & 0x1f) + (INT16)op);
}


static void goto_t(dsp32_state *cpustate, UINT32 op)
{
	execute_one(cpustate);
	cpustate->PC = TRUNCATE24(REG24(cpustate, (op >> 16) & 0x1f) + (INT16)op);
}


static void goto_pl(dsp32_state *cpustate, UINT32 op)
{
	if (!nFLAG)
	{
		execute_one(cpustate);
		cpustate->PC = TRUNCATE24(REG24(cpustate, (op >> 16) & 0x1f) + (INT16)op);
	}
}


static void goto_mi(dsp32_state *cpustate, UINT32 op)
{
	if (nFLAG)
	{
		execute_one(cpustate);
		cpustate->PC = TRUNCATE24(REG24(cpustate, (op >> 16) & 0x1f) + (INT16)op);
	}
}


static void goto_ne(dsp32_state *cpustate, UINT32 op)
{
	if (!zFLAG)
	{
		execute_one(cpustate);
		cpustate->PC = TRUNCATE24(REG24(cpustate, (op >> 16) & 0x1f) + (INT16)op);
	}
}


static void goto_eq(dsp32_state *cpustate, UINT32 op)
{
	if (zFLAG)
	{
		execute_one(cpustate);
		cpustate->PC = TRUNCATE24(REG24(cpustate, (op >> 16) & 0x1f) + (INT16)op);
	}
}


static void goto_vc(dsp32_state *cpustate, UINT32 op)
{
	if (!vFLAG)
	{
		execute_one(cpustate);
		cpustate->PC = TRUNCATE24(REG24(cpustate, (op >> 16) & 0x1f) + (INT16)op);
	}
}


static void goto_vs(dsp32_state *cpustate, UINT32 op)
{
	if (vFLAG)
	{
		execute_one(cpustate);
		cpustate->PC = TRUNCATE24(REG24(cpustate, (op >> 16) & 0x1f) + (INT16)op);
	}
}


static void goto_cc(dsp32_state *cpustate, UINT32 op)
{
	if (!cFLAG)
	{
		execute_one(cpustate);
		cpustate->PC = TRUNCATE24(REG24(cpustate, (op >> 16) & 0x1f) + (INT16)op);
	}
}


static void goto_cs(dsp32_state *cpustate, UINT32 op)
{
	if (cFLAG)
	{
		execute_one(cpustate);
		cpustate->PC = TRUNCATE24(REG24(cpustate, (op >> 16) & 0x1f) + (INT16)op);
	}
}


static void goto_ge(dsp32_state *cpustate, UINT32 op)
{
	if (!(nFLAG ^ vFLAG))
	{
		execute_one(cpustate);
		cpustate->PC = TRUNCATE24(REG24(cpustate, (op >> 16) & 0x1f) + (INT16)op);
	}
}


static void goto_lt(dsp32_state *cpustate, UINT32 op)
{
	if (nFLAG ^ vFLAG)
	{
		execute_one(cpustate);
		cpustate->PC = TRUNCATE24(REG24(cpustate, (op >> 16) & 0x1f) + (INT16)op);
	}
}


static void goto_gt(dsp32_state *cpustate, UINT32 op)
{
	if (!(zFLAG | (nFLAG ^ vFLAG)))
	{
		execute_one(cpustate);
		cpustate->PC = TRUNCATE24(REG24(cpustate, (op >> 16) & 0x1f) + (INT16)op);
	}
}


static void goto_le(dsp32_state *cpustate, UINT32 op)
{
	if (zFLAG | (nFLAG ^ vFLAG))
	{
		execute_one(cpustate);
		cpustate->PC = TRUNCATE24(REG24(cpustate, (op >> 16) & 0x1f) + (INT16)op);
	}
}


static void goto_hi(dsp32_state *cpustate, UINT32 op)
{
	if (!cFLAG && !zFLAG)
	{
		execute_one(cpustate);
		cpustate->PC = TRUNCATE24(REG24(cpustate, (op >> 16) & 0x1f) + (INT16)op);
	}
}


static void goto_ls(dsp32_state *cpustate, UINT32 op)
{
	if (cFLAG || zFLAG)
	{
		execute_one(cpustate);
		cpustate->PC = TRUNCATE24(REG24(cpustate, (op >> 16) & 0x1f) + (INT16)op);
	}
}


static void goto_auc(dsp32_state *cpustate, UINT32 op)
{
	if (!(DEFERRED_VUFLAGS(cpustate) & UFLAGBIT))
	{
		execute_one(cpustate);
		cpustate->PC = TRUNCATE24(REG24(cpustate, (op >> 16) & 0x1f) + (INT16)op);
	}
}


static void goto_aus(dsp32_state *cpustate, UINT32 op)
{
	if (DEFERRED_VUFLAGS(cpustate) & UFLAGBIT)
	{
		execute_one(cpustate);
		cpustate->PC = TRUNCATE24(REG24(cpustate, (op >> 16) & 0x1f) + (INT16)op);
	}
}


static void goto_age(dsp32_state *cpustate, UINT32 op)
{
	if (DEFERRED_NZFLAGS(cpustate) >= 0)
	{
		execute_one(cpustate);
		cpustate->PC = TRUNCATE24(REG24(cpustate, (op >> 16) & 0x1f) + (INT16)op);
	}
}


static void goto_alt(dsp32_state *cpustate, UINT32 op)
{
	if (DEFERRED_NZFLAGS(cpustate) < 0)
	{
		execute_one(cpustate);
		cpustate->PC = TRUNCATE24(REG24(cpustate, (op >> 16) & 0x1f) + (INT16)op);
	}
}


static void goto_ane(dsp32_state *cpustate, UINT32 op)
{
	if (DEFERRED_NZFLAGS(cpustate) != 0)
	{
		execute_one(cpustate);
		cpustate->PC = TRUNCATE24(REG24(cpustate, (op >> 16) & 0x1f) + (INT16)op);
	}
}


static void goto_aeq(dsp32_state *cpustate, UINT32 op)
{
	if (DEFERRED_NZFLAGS(cpustate) == 0)
	{
		execute_one(cpustate);
		cpustate->PC = TRUNCATE24(REG24(cpustate, (op >> 16) & 0x1f) + (INT16)op);
	}
}


static void goto_avc(dsp32_state *cpustate, UINT32 op)
{
	if (!(DEFERRED_VUFLAGS(cpustate) & VFLAGBIT))
	{
		execute_one(cpustate);
		cpustate->PC = TRUNCATE24(REG24(cpustate, (op >> 16) & 0x1f) + (INT16)op);
	}
}


static void goto_avs(dsp32_state *cpustate, UINT32 op)
{
	if (DEFERRED_VUFLAGS(cpustate) & VFLAGBIT)
	{
		execute_one(cpustate);
		cpustate->PC = TRUNCATE24(REG24(cpustate, (op >> 16) & 0x1f) + (INT16)op);
	}
}


static void goto_agt(dsp32_state *cpustate, UINT32 op)
{
	if (DEFERRED_NZFLAGS(cpustate) > 0)
	{
		execute_one(cpustate);
		cpustate->PC = TRUNCATE24(REG24(cpustate, (op >> 16) & 0x1f) + (INT16)op);
	}
}


static void goto_ale(dsp32_state *cpustate, UINT32 op)
{
	if (DEFERRED_NZFLAGS(cpustate) <= 0)
	{
		execute_one(cpustate);
		cpustate->PC = TRUNCATE24(REG24(cpustate, (op >> 16) & 0x1f) + (INT16)op);
	}
}


static void goto_ibe(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void goto_ibf(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void goto_obf(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void goto_obe(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void goto_pde(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void goto_pdf(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void goto_pie(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void goto_pif(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void goto_syc(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void goto_sys(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void goto_fbc(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void goto_fbs(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void goto_irq1lo(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void goto_irq1hi(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void goto_irq2lo(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void goto_irq2hi(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void dec_goto(dsp32_state *cpustate, UINT32 op)
{
	int hr = (op >> 21) & 0x1f;
	int old = (INT16)cpustate->r[hr];
	cpustate->r[hr] = EXTEND16_TO_24(cpustate->r[hr] - 1);
	if (old >= 0)
	{
		execute_one(cpustate);
		cpustate->PC = TRUNCATE24(REG24(cpustate, (op >> 16) & 0x1f) + (INT16)op);
	}
}


static void call(dsp32_state *cpustate, UINT32 op)
{
	int mr = (op >> 21) & 0x1f;
	if (IS_WRITEABLE(mr))
		cpustate->r[mr] = cpustate->PC + 4;
	execute_one(cpustate);
	cpustate->PC = TRUNCATE24(REG24(cpustate, (op >> 16) & 0x1f) + (INT16)op);
}


static void goto24(dsp32_state *cpustate, UINT32 op)
{
	execute_one(cpustate);
	cpustate->PC = TRUNCATE24(REG24(cpustate, (op >> 16) & 0x1f) + (op & 0xffff) + ((op >> 5) & 0xff0000));
}


static void call24(dsp32_state *cpustate, UINT32 op)
{
	int mr = (op >> 16) & 0x1f;
	if (IS_WRITEABLE(mr))
		cpustate->r[mr] = cpustate->PC + 4;
	execute_one(cpustate);
	cpustate->PC = (op & 0xffff) + ((op >> 5) & 0xff0000);
}


static void do_i(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void do_r(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}



/***************************************************************************
    CAU 16-BIT ARITHMETIC IMPLEMENTATION
***************************************************************************/

static void add_si(dsp32_state *cpustate, UINT32 op)
{
	int dr = (op >> 21) & 0x1f;
	int hrval = REG16(cpustate, (op >> 16) & 0x1f);
	int res = hrval + (UINT16)op;
	if (IS_WRITEABLE(dr))
		cpustate->r[dr] = EXTEND16_TO_24(res);
	SET_NZCV_16(cpustate, hrval, op, res);
}


static void add_ss(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG16(cpustate, (op >> 5) & 0x1f);
		int s2rval = (op & 0x800) ? REG16(cpustate, (op >> 0) & 0x1f) : REG16(cpustate, dr);
		int res = s2rval + s1rval;
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = EXTEND16_TO_24(res);
		SET_NZCV_16(cpustate, s1rval, s2rval, res);
	}
}


static void mul2_s(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG16(cpustate, (op >> 5) & 0x1f);
		int res = s1rval * 2;
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = EXTEND16_TO_24(res);
		SET_NZCV_16(cpustate, s1rval, 0, res);
	}
}


static void subr_ss(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG16(cpustate, (op >> 5) & 0x1f);
		int s2rval = (op & 0x800) ? REG16(cpustate, (op >> 0) & 0x1f) : REG16(cpustate, dr);
		int res = s1rval - s2rval;
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = EXTEND16_TO_24(res);
		SET_NZCV_16(cpustate, s1rval, s2rval, res);
	}
}


static void addr_ss(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void sub_ss(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG16(cpustate, (op >> 5) & 0x1f);
		int s2rval = (op & 0x800) ? REG16(cpustate, (op >> 0) & 0x1f) : REG16(cpustate, dr);
		int res = s2rval - s1rval;
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = EXTEND16_TO_24(res);
		SET_NZCV_16(cpustate, s1rval, s2rval, res);
	}
}


static void neg_s(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG16(cpustate, (op >> 5) & 0x1f);
		int res = -s1rval;
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = EXTEND16_TO_24(res);
		SET_NZCV_16(cpustate, s1rval, 0, res);
	}
}


static void andc_ss(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG16(cpustate, (op >> 5) & 0x1f);
		int s2rval = (op & 0x800) ? REG16(cpustate, (op >> 0) & 0x1f) : REG16(cpustate, dr);
		int res = s2rval & ~s1rval;
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = EXTEND16_TO_24(res);
		SET_NZ00_16(cpustate, res);
	}
}


static void cmp_ss(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int drval = REG16(cpustate, (op >> 16) & 0x1f);
		int s1rval = REG16(cpustate, (op >> 5) & 0x1f);
		int res = drval - s1rval;
		SET_NZCV_16(cpustate, drval, s1rval, res);
	}
}


static void xor_ss(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG16(cpustate, (op >> 5) & 0x1f);
		int s2rval = (op & 0x800) ? REG16(cpustate, (op >> 0) & 0x1f) : REG16(cpustate, dr);
		int res = s2rval ^ s1rval;
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = EXTEND16_TO_24(res);
		SET_NZ00_16(cpustate, res);
	}
}


static void rcr_s(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG16(cpustate, (op >> 5) & 0x1f);
		int res = ((cpustate->nzcflags >> 9) & 0x8000) | (s1rval >> 1);
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = EXTEND16_TO_24(res);
		cpustate->nzcflags = ((res & 0xffff) << 8) | ((s1rval & 1) << 24);
		cpustate->vflags = 0;
	}
}


static void or_ss(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG16(cpustate, (op >> 5) & 0x1f);
		int s2rval = (op & 0x800) ? REG16(cpustate, (op >> 0) & 0x1f) : REG16(cpustate, dr);
		int res = s2rval | s1rval;
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = EXTEND16_TO_24(res);
		SET_NZ00_16(cpustate, res);
	}
}


static void rcl_s(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG16(cpustate, (op >> 5) & 0x1f);
		int res = ((cpustate->nzcflags >> 24) & 0x0001) | (s1rval << 1);
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = EXTEND16_TO_24(res);
		cpustate->nzcflags = ((res & 0xffff) << 8) | ((s1rval & 0x8000) << 9);
		cpustate->vflags = 0;
	}
}


static void shr_s(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG16(cpustate, (op >> 5) & 0x1f);
		int res = s1rval >> 1;
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = EXTEND16_TO_24(res);
		cpustate->nzcflags = ((res & 0xffff) << 8) | ((s1rval & 1) << 24);
		cpustate->vflags = 0;
	}
}


static void div2_s(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG16(cpustate, (op >> 5) & 0x1f);
		int res = (s1rval & 0x8000) | (s1rval >> 1);
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = EXTEND16_TO_24(res);
		cpustate->nzcflags = ((res & 0xffff) << 8) | ((s1rval & 1) << 24);
		cpustate->vflags = 0;
	}
}


static void and_ss(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG16(cpustate, (op >> 5) & 0x1f);
		int s2rval = (op & 0x800) ? REG16(cpustate, (op >> 0) & 0x1f) : REG16(cpustate, dr);
		int res = s2rval & s1rval;
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = EXTEND16_TO_24(res);
		SET_NZ00_16(cpustate, res);
	}
}


static void test_ss(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int drval = REG16(cpustate, (op >> 16) & 0x1f);
		int s1rval = REG16(cpustate, (op >> 5) & 0x1f);
		int res = drval & s1rval;
		SET_NZ00_16(cpustate, res);
	}
}


static void add_di(dsp32_state *cpustate, UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	int drval = REG16(cpustate, dr);
	int res = drval + (UINT16)op;
	if (IS_WRITEABLE(dr))
		cpustate->r[dr] = EXTEND16_TO_24(res);
	SET_NZCV_16(cpustate, drval, op, res);
}


static void subr_di(dsp32_state *cpustate, UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	int drval = REG16(cpustate, dr);
	int res = (UINT16)op - drval;
	if (IS_WRITEABLE(dr))
		cpustate->r[dr] = EXTEND16_TO_24(res);
	SET_NZCV_16(cpustate, drval, op, res);
}


static void addr_di(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void sub_di(dsp32_state *cpustate, UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	int drval = REG16(cpustate, dr);
	int res = drval - (UINT16)op;
	if (IS_WRITEABLE(dr))
		cpustate->r[dr] = EXTEND16_TO_24(res);
	SET_NZCV_16(cpustate, drval, op, res);
}


static void andc_di(dsp32_state *cpustate, UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	int drval = REG16(cpustate, dr);
	int res = drval & ~(UINT16)op;
	if (IS_WRITEABLE(dr))
		cpustate->r[dr] = EXTEND16_TO_24(res);
	SET_NZ00_16(cpustate, res);
}


static void cmp_di(dsp32_state *cpustate, UINT32 op)
{
	int drval = REG16(cpustate, (op >> 16) & 0x1f);
	int res = drval - (UINT16)op;
	SET_NZCV_16(cpustate, drval, op, res);
}


static void xor_di(dsp32_state *cpustate, UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	int drval = REG16(cpustate, dr);
	int res = drval ^ (UINT16)op;
	if (IS_WRITEABLE(dr))
		cpustate->r[dr] = EXTEND16_TO_24(res);
	SET_NZ00_16(cpustate, res);
}


static void or_di(dsp32_state *cpustate, UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	int drval = REG16(cpustate, dr);
	int res = drval | (UINT16)op;
	if (IS_WRITEABLE(dr))
		cpustate->r[dr] = EXTEND16_TO_24(res);
	SET_NZ00_16(cpustate, res);
}


static void and_di(dsp32_state *cpustate, UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	int drval = REG16(cpustate, dr);
	int res = drval & (UINT16)op;
	if (IS_WRITEABLE(dr))
		cpustate->r[dr] = EXTEND16_TO_24(res);
	SET_NZ00_16(cpustate, res);
}


static void test_di(dsp32_state *cpustate, UINT32 op)
{
	int drval = REG16(cpustate, (op >> 16) & 0x1f);
	int res = drval & (UINT16)op;
	SET_NZ00_16(cpustate, res);
}



/***************************************************************************
    CAU 24-BIT ARITHMETIC IMPLEMENTATION
***************************************************************************/

static void adde_si(dsp32_state *cpustate, UINT32 op)
{
	int dr = (op >> 21) & 0x1f;
	int hrval = REG24(cpustate, (op >> 16) & 0x1f);
	int res = hrval + EXTEND16_TO_24(op);
	if (IS_WRITEABLE(dr))
		cpustate->r[dr] = TRUNCATE24(res);
	SET_NZCV_24(cpustate, hrval, op << 8, res);
}


static void adde_ss(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG24(cpustate, (op >> 5) & 0x1f);
		int s2rval = (op & 0x800) ? REG24(cpustate, (op >> 0) & 0x1f) : REG24(cpustate, dr);
		int res = s2rval + s1rval;
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = TRUNCATE24(res);
		SET_NZCV_24(cpustate, s1rval, s2rval, res);
	}
}


static void mul2e_s(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG24(cpustate, (op >> 5) & 0x1f);
		int res = s1rval * 2;
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = TRUNCATE24(res);
		SET_NZCV_24(cpustate, s1rval, 0, res);
	}
}


static void subre_ss(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG24(cpustate, (op >> 5) & 0x1f);
		int s2rval = (op & 0x800) ? REG24(cpustate, (op >> 0) & 0x1f) : REG24(cpustate, dr);
		int res = s1rval - s2rval;
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = TRUNCATE24(res);
		SET_NZCV_24(cpustate, s1rval, s2rval, res);
	}
}


static void addre_ss(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void sube_ss(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG24(cpustate, (op >> 5) & 0x1f);
		int s2rval = (op & 0x800) ? REG24(cpustate, (op >> 0) & 0x1f) : REG24(cpustate, dr);
		int res = s2rval - s1rval;
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = TRUNCATE24(res);
		SET_NZCV_24(cpustate, s1rval, s2rval, res);
	}
}


static void nege_s(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG24(cpustate, (op >> 5) & 0x1f);
		int res = -s1rval;
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = TRUNCATE24(res);
		SET_NZCV_24(cpustate, s1rval, 0, res);
	}
}


static void andce_ss(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG24(cpustate, (op >> 5) & 0x1f);
		int s2rval = (op & 0x800) ? REG24(cpustate, (op >> 0) & 0x1f) : REG24(cpustate, dr);
		int res = s2rval & ~s1rval;
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = res;
		SET_NZ00_24(cpustate, res);
	}
}


static void cmpe_ss(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int drval = REG24(cpustate, (op >> 16) & 0x1f);
		int s1rval = REG24(cpustate, (op >> 5) & 0x1f);
		int res = drval - s1rval;
		SET_NZCV_24(cpustate, drval, s1rval, res);
	}
}


static void xore_ss(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG24(cpustate, (op >> 5) & 0x1f);
		int s2rval = (op & 0x800) ? REG24(cpustate, (op >> 0) & 0x1f) : REG24(cpustate, dr);
		int res = s2rval ^ s1rval;
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = res;
		SET_NZ00_24(cpustate, res);
	}
}


static void rcre_s(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG24(cpustate, (op >> 5) & 0x1f);
		int res = ((cpustate->nzcflags >> 1) & 0x800000) | (s1rval >> 1);
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = TRUNCATE24(res);
		cpustate->nzcflags = res | ((s1rval & 1) << 24);
		cpustate->vflags = 0;
	}
}


static void ore_ss(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG24(cpustate, (op >> 5) & 0x1f);
		int s2rval = (op & 0x800) ? REG24(cpustate, (op >> 0) & 0x1f) : REG24(cpustate, dr);
		int res = s2rval | s1rval;
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = res;
		SET_NZ00_24(cpustate, res);
	}
}


static void rcle_s(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG24(cpustate, (op >> 5) & 0x1f);
		int res = ((cpustate->nzcflags >> 24) & 0x000001) | (s1rval << 1);
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = TRUNCATE24(res);
		cpustate->nzcflags = res | ((s1rval & 0x800000) << 1);
		cpustate->vflags = 0;
	}
}


static void shre_s(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG24(cpustate, (op >> 5) & 0x1f);
		int res = s1rval >> 1;
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = res;
		cpustate->nzcflags = res | ((s1rval & 1) << 24);
		cpustate->vflags = 0;
	}
}


static void div2e_s(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG24(cpustate, (op >> 5) & 0x1f);
		int res = (s1rval & 0x800000) | (s1rval >> 1);
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = TRUNCATE24(res);
		cpustate->nzcflags = res | ((s1rval & 1) << 24);
		cpustate->vflags = 0;
	}
}


static void ande_ss(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int dr = (op >> 16) & 0x1f;
		int s1rval = REG24(cpustate, (op >> 5) & 0x1f);
		int s2rval = (op & 0x800) ? REG24(cpustate, (op >> 0) & 0x1f) : REG24(cpustate, dr);
		int res = s2rval & s1rval;
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = res;
		SET_NZ00_24(cpustate, res);
	}
}


static void teste_ss(dsp32_state *cpustate, UINT32 op)
{
	if (CONDITION_IS_TRUE(cpustate))
	{
		int drval = REG24(cpustate, (op >> 16) & 0x1f);
		int s1rval = REG24(cpustate, (op >> 5) & 0x1f);
		int res = drval & s1rval;
		SET_NZ00_24(cpustate, res);
	}
}


static void adde_di(dsp32_state *cpustate, UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	int drval = REG24(cpustate, dr);
	int res = drval + EXTEND16_TO_24(op);
	if (IS_WRITEABLE(dr))
		cpustate->r[dr] = TRUNCATE24(res);
	SET_NZCV_24(cpustate, drval, op << 8, res);
}


static void subre_di(dsp32_state *cpustate, UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	int drval = REG24(cpustate, dr);
	int res = EXTEND16_TO_24(op) - drval;
	if (IS_WRITEABLE(dr))
		cpustate->r[dr] = TRUNCATE24(res);
	SET_NZCV_24(cpustate, drval, op << 8, res);
}


static void addre_di(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void sube_di(dsp32_state *cpustate, UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	int drval = REG24(cpustate, dr);
	int res = drval - EXTEND16_TO_24(op);
	if (IS_WRITEABLE(dr))
		cpustate->r[dr] = TRUNCATE24(res);
	SET_NZCV_24(cpustate, drval, op << 8, res);
}


static void andce_di(dsp32_state *cpustate, UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	int drval = REG24(cpustate, dr);
	int res = drval & ~EXTEND16_TO_24(op);
	if (IS_WRITEABLE(dr))
		cpustate->r[dr] = res;
	SET_NZ00_24(cpustate, res);
}


static void cmpe_di(dsp32_state *cpustate, UINT32 op)
{
	int drval = REG24(cpustate, (op >> 16) & 0x1f);
	int res = drval - EXTEND16_TO_24(op);
	SET_NZCV_24(cpustate, drval, op << 8, res);
}


static void xore_di(dsp32_state *cpustate, UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	int drval = REG24(cpustate, dr);
	int res = drval ^ EXTEND16_TO_24(op);
	if (IS_WRITEABLE(dr))
		cpustate->r[dr] = res;
	SET_NZ00_24(cpustate, res);
}


static void ore_di(dsp32_state *cpustate, UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	int drval = REG24(cpustate, dr);
	int res = drval | EXTEND16_TO_24(op);
	if (IS_WRITEABLE(dr))
		cpustate->r[dr] = res;
	SET_NZ00_24(cpustate, res);
}


static void ande_di(dsp32_state *cpustate, UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	int drval = REG24(cpustate, dr);
	int res = drval & EXTEND16_TO_24(op);
	if (IS_WRITEABLE(dr))
		cpustate->r[dr] = res;
	SET_NZ00_24(cpustate, res);
}


static void teste_di(dsp32_state *cpustate, UINT32 op)
{
	int drval = REG24(cpustate, (op >> 16) & 0x1f);
	int res = drval & EXTEND16_TO_24(op);
	SET_NZ00_24(cpustate, res);
}



/***************************************************************************
    CAU LOAD/STORE IMPLEMENTATION
***************************************************************************/

static void load_hi(dsp32_state *cpustate, UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	UINT32 res = RBYTE(cpustate, EXTEND16_TO_24(op));
	if (IS_WRITEABLE(dr))
		cpustate->r[dr] = EXTEND16_TO_24(res);
	cpustate->nzcflags = res << 8;
	cpustate->vflags = 0;
}


static void load_li(dsp32_state *cpustate, UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	UINT32 res = RBYTE(cpustate, EXTEND16_TO_24(op));
	if (IS_WRITEABLE(dr))
		cpustate->r[dr] = res;
	cpustate->nzcflags = res << 8;
	cpustate->vflags = 0;
}


static void load_i(dsp32_state *cpustate, UINT32 op)
{
	UINT32 res = RWORD(cpustate, EXTEND16_TO_24(op));
	int dr = (op >> 16) & 0x1f;
	if (IS_WRITEABLE(dr))
		cpustate->r[dr] = EXTEND16_TO_24(res);
	cpustate->nzcflags = res << 8;
	cpustate->vflags = 0;
}


static void load_ei(dsp32_state *cpustate, UINT32 op)
{
	UINT32 res = TRUNCATE24(RLONG(cpustate, EXTEND16_TO_24(op)));
	int dr = (op >> 16) & 0x1f;
	if (IS_WRITEABLE(dr))
		cpustate->r[dr] = res;
	cpustate->nzcflags = res;
	cpustate->vflags = 0;
}


static void store_hi(dsp32_state *cpustate, UINT32 op)
{
	WBYTE(cpustate, EXTEND16_TO_24(op), cpustate->r[(op >> 16) & 0x1f] >> 8);
}


static void store_li(dsp32_state *cpustate, UINT32 op)
{
	WBYTE(cpustate, EXTEND16_TO_24(op), cpustate->r[(op >> 16) & 0x1f]);
}


static void store_i(dsp32_state *cpustate, UINT32 op)
{
	WWORD(cpustate, EXTEND16_TO_24(op), REG16(cpustate, (op >> 16) & 0x1f));
}


static void store_ei(dsp32_state *cpustate, UINT32 op)
{
	WLONG(cpustate, EXTEND16_TO_24(op), (INT32)(REG24(cpustate, (op >> 16) & 0x1f) << 8) >> 8);
}


static void load_hr(dsp32_state *cpustate, UINT32 op)
{
	if (!(op & 0x400))
	{
		int dr = (op >> 16) & 0x1f;
		UINT32 res = cau_read_pi_1byte(cpustate, op) << 8;
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = EXTEND16_TO_24(res);
		cpustate->nzcflags = res << 8;
		cpustate->vflags = 0;
	}
	else
		unimplemented(cpustate, op);
}


static void load_lr(dsp32_state *cpustate, UINT32 op)
{
	if (!(op & 0x400))
	{
		int dr = (op >> 16) & 0x1f;
		UINT32 res = cau_read_pi_1byte(cpustate, op);
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = res;
		cpustate->nzcflags = res << 8;
		cpustate->vflags = 0;
	}
	else
		unimplemented(cpustate, op);
}


static void load_r(dsp32_state *cpustate, UINT32 op)
{
	if (!(op & 0x400))
	{
		UINT32 res = cau_read_pi_2byte(cpustate, op);
		int dr = (op >> 16) & 0x1f;
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = EXTEND16_TO_24(res);
		cpustate->nzcflags = res << 8;
		cpustate->vflags = 0;
	}
	else
		unimplemented(cpustate, op);
}


static void load_er(dsp32_state *cpustate, UINT32 op)
{
	if (!(op & 0x400))
	{
		UINT32 res = TRUNCATE24(cau_read_pi_4byte(cpustate, op));
		int dr = (op >> 16) & 0x1f;
		if (IS_WRITEABLE(dr))
			cpustate->r[dr] = res;
		cpustate->nzcflags = res;
		cpustate->vflags = 0;
	}
	else
		unimplemented(cpustate, op);
}


static void store_hr(dsp32_state *cpustate, UINT32 op)
{
	if (!(op & 0x400))
		cau_write_pi_1byte(cpustate, op, cpustate->r[(op >> 16) & 0x1f] >> 8);
	else
		unimplemented(cpustate, op);
}


static void store_lr(dsp32_state *cpustate, UINT32 op)
{
	if (!(op & 0x400))
		cau_write_pi_1byte(cpustate, op, cpustate->r[(op >> 16) & 0x1f]);
	else
		unimplemented(cpustate, op);
}


static void store_r(dsp32_state *cpustate, UINT32 op)
{
	if (!(op & 0x400))
		cau_write_pi_2byte(cpustate, op, REG16(cpustate, (op >> 16) & 0x1f));
	else
		unimplemented(cpustate, op);
}


static void store_er(dsp32_state *cpustate, UINT32 op)
{
	if (!(op & 0x400))
		cau_write_pi_4byte(cpustate, op, REG24(cpustate, (op >> 16) & 0x1f));
	else
		unimplemented(cpustate, op);
}


static void load24(dsp32_state *cpustate, UINT32 op)
{
	int dr = (op >> 16) & 0x1f;
	UINT32 res = (op & 0xffff) + ((op >> 5) & 0xff0000);
	if (IS_WRITEABLE(dr))
		cpustate->r[dr] = res;
}



/***************************************************************************
    DAU FORM 1 IMPLEMENTATION
***************************************************************************/

static void d1_aMpp(dsp32_state *cpustate, UINT32 op)
{
	double xval = dau_read_pi_double_1st(cpustate, op >> 14, 1);
	double yval = dau_read_pi_double_2nd(cpustate, op >> 7, 0, xval);
	double res = yval + DEFERRED_MULTIPLIER(cpustate, (op >> 26) & 7) * xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(cpustate, zpi, res);
	dau_set_val_flags(cpustate, (op >> 21) & 3, res);
}


static void d1_aMpm(dsp32_state *cpustate, UINT32 op)
{
	double xval = dau_read_pi_double_1st(cpustate, op >> 14, 1);
	double yval = dau_read_pi_double_2nd(cpustate, op >> 7, 0, xval);
	double res = yval - DEFERRED_MULTIPLIER(cpustate, (op >> 26) & 7) * xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(cpustate, zpi, res);
	dau_set_val_flags(cpustate, (op >> 21) & 3, res);
}


static void d1_aMmp(dsp32_state *cpustate, UINT32 op)
{
	double xval = dau_read_pi_double_1st(cpustate, op >> 14, 1);
	double yval = dau_read_pi_double_2nd(cpustate, op >> 7, 0, xval);
	double res = -yval + DEFERRED_MULTIPLIER(cpustate, (op >> 26) & 7) * xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(cpustate, zpi, res);
	dau_set_val_flags(cpustate, (op >> 21) & 3, res);
}


static void d1_aMmm(dsp32_state *cpustate, UINT32 op)
{
	double xval = dau_read_pi_double_1st(cpustate, op >> 14, 1);
	double yval = dau_read_pi_double_2nd(cpustate, op >> 7, 0, xval);
	double res = -yval - DEFERRED_MULTIPLIER(cpustate, (op >> 26) & 7) * xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(cpustate, zpi, res);
	dau_set_val_flags(cpustate, (op >> 21) & 3, res);
}


static void d1_0px(dsp32_state *cpustate, UINT32 op)
{
	double xval = dau_read_pi_double_1st(cpustate, op >> 14, 1);
	double yval = dau_read_pi_double_2nd(cpustate, op >> 7, 0, xval);
	double res = yval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(cpustate, zpi, res);
	dau_set_val_flags(cpustate, (op >> 21) & 3, res);
	(void)xval;
}


static void d1_0mx(dsp32_state *cpustate, UINT32 op)
{
	double xval = dau_read_pi_double_1st(cpustate, op >> 14, 1);
	double yval = dau_read_pi_double_2nd(cpustate, op >> 7, 0, xval);
	double res = -yval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(cpustate, zpi, res);
	dau_set_val_flags(cpustate, (op >> 21) & 3, res);
	(void)xval;
}


static void d1_1pp(dsp32_state *cpustate, UINT32 op)
{
	double xval = dau_read_pi_double_1st(cpustate, op >> 14, 1);
	double yval = dau_read_pi_double_2nd(cpustate, op >> 7, 0, xval);
	double res = yval + xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(cpustate, zpi, res);
	dau_set_val_flags(cpustate, (op >> 21) & 3, res);
}


static void d1_1pm(dsp32_state *cpustate, UINT32 op)
{
	double xval = dau_read_pi_double_1st(cpustate, op >> 14, 1);
	double yval = dau_read_pi_double_2nd(cpustate, op >> 7, 0, xval);
	double res = yval - xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(cpustate, zpi, res);
	dau_set_val_flags(cpustate, (op >> 21) & 3, res);
}


static void d1_1mp(dsp32_state *cpustate, UINT32 op)
{
	double xval = dau_read_pi_double_1st(cpustate, op >> 14, 1);
	double yval = dau_read_pi_double_2nd(cpustate, op >> 7, 0, xval);
	double res = -yval + xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(cpustate, zpi, res);
	dau_set_val_flags(cpustate, (op >> 21) & 3, res);
}


static void d1_1mm(dsp32_state *cpustate, UINT32 op)
{
	double xval = dau_read_pi_double_1st(cpustate, op >> 14, 1);
	double yval = dau_read_pi_double_2nd(cpustate, op >> 7, 0, xval);
	double res = -yval - xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(cpustate, zpi, res);
	dau_set_val_flags(cpustate, (op >> 21) & 3, res);
}


static void d1_aMppr(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void d1_aMpmr(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void d1_aMmpr(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void d1_aMmmr(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}



/***************************************************************************
    DAU FORM 2 IMPLEMENTATION
***************************************************************************/

static void d2_aMpp(dsp32_state *cpustate, UINT32 op)
{
	double xval = dau_read_pi_double_1st(cpustate, op >> 14, 1);
	double yval = dau_read_pi_double_2nd(cpustate, op >> 7, 1, xval);
	double res = cpustate->a[(op >> 26) & 7] + yval * xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(cpustate, zpi, yval);
	dau_set_val_flags(cpustate, (op >> 21) & 3, res);
}


static void d2_aMpm(dsp32_state *cpustate, UINT32 op)
{
	double xval = dau_read_pi_double_1st(cpustate, op >> 14, 1);
	double yval = dau_read_pi_double_2nd(cpustate, op >> 7, 1, xval);
	double res = cpustate->a[(op >> 26) & 7] - yval * xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(cpustate, zpi, yval);
	dau_set_val_flags(cpustate, (op >> 21) & 3, res);
}


static void d2_aMmp(dsp32_state *cpustate, UINT32 op)
{
	double xval = dau_read_pi_double_1st(cpustate, op >> 14, 1);
	double yval = dau_read_pi_double_2nd(cpustate, op >> 7, 1, xval);
	double res = -cpustate->a[(op >> 26) & 7] + yval * xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(cpustate, zpi, yval);
	dau_set_val_flags(cpustate, (op >> 21) & 3, res);
}


static void d2_aMmm(dsp32_state *cpustate, UINT32 op)
{
	double xval = dau_read_pi_double_1st(cpustate, op >> 14, 1);
	double yval = dau_read_pi_double_2nd(cpustate, op >> 7, 1, xval);
	double res = -cpustate->a[(op >> 26) & 7] - yval * xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(cpustate, zpi, yval);
	dau_set_val_flags(cpustate, (op >> 21) & 3, res);
}


static void d2_aMppr(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void d2_aMpmr(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void d2_aMmpr(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void d2_aMmmr(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}



/***************************************************************************
    DAU FORM 3 IMPLEMENTATION
***************************************************************************/

static void d3_aMpp(dsp32_state *cpustate, UINT32 op)
{
	double xval = dau_read_pi_double_1st(cpustate, op >> 14, 1);
	double yval = dau_read_pi_double_2nd(cpustate, op >> 7, 1, xval);
	double res = cpustate->a[(op >> 26) & 7] + yval * xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(cpustate, zpi, res);
	dau_set_val_flags(cpustate, (op >> 21) & 3, res);
}


static void d3_aMpm(dsp32_state *cpustate, UINT32 op)
{
	double xval = dau_read_pi_double_1st(cpustate, op >> 14, 1);
	double yval = dau_read_pi_double_2nd(cpustate, op >> 7, 1, xval);
	double res = cpustate->a[(op >> 26) & 7] - yval * xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(cpustate, zpi, res);
	dau_set_val_flags(cpustate, (op >> 21) & 3, res);
}


static void d3_aMmp(dsp32_state *cpustate, UINT32 op)
{
	double xval = dau_read_pi_double_1st(cpustate, op >> 14, 1);
	double yval = dau_read_pi_double_2nd(cpustate, op >> 7, 1, xval);
	double res = -cpustate->a[(op >> 26) & 7] + yval * xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(cpustate, zpi, res);
	dau_set_val_flags(cpustate, (op >> 21) & 3, res);
}


static void d3_aMmm(dsp32_state *cpustate, UINT32 op)
{
	double xval = dau_read_pi_double_1st(cpustate, op >> 14, 1);
	double yval = dau_read_pi_double_2nd(cpustate, op >> 7, 1, xval);
	double res = -cpustate->a[(op >> 26) & 7] - yval * xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(cpustate, zpi, res);
	dau_set_val_flags(cpustate, (op >> 21) & 3, res);
}


static void d3_aMppr(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void d3_aMpmr(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void d3_aMmpr(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void d3_aMmmr(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}



/***************************************************************************
    DAU FORM 4 IMPLEMENTATION
***************************************************************************/

static void d4_pp(dsp32_state *cpustate, UINT32 op)
{
	double xval = dau_read_pi_double_1st(cpustate, op >> 14, 1);
	double yval = dau_read_pi_double_2nd(cpustate, op >> 7, 0, xval);
	double res = yval + xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(cpustate, zpi, yval);
	dau_set_val_flags(cpustate, (op >> 21) & 3, res);
}


static void d4_pm(dsp32_state *cpustate, UINT32 op)
{
	double xval = dau_read_pi_double_1st(cpustate, op >> 14, 1);
	double yval = dau_read_pi_double_2nd(cpustate, op >> 7, 0, xval);
	double res = yval - xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(cpustate, zpi, yval);
	dau_set_val_flags(cpustate, (op >> 21) & 3, res);
}


static void d4_mp(dsp32_state *cpustate, UINT32 op)
{
	double xval = dau_read_pi_double_1st(cpustate, op >> 14, 1);
	double yval = dau_read_pi_double_2nd(cpustate, op >> 7, 0, xval);
	double res = -yval + xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(cpustate, zpi, yval);
	dau_set_val_flags(cpustate, (op >> 21) & 3, res);
}


static void d4_mm(dsp32_state *cpustate, UINT32 op)
{
	double xval = dau_read_pi_double_1st(cpustate, op >> 14, 1);
	double yval = dau_read_pi_double_2nd(cpustate, op >> 7, 0, xval);
	double res = -yval - xval;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(cpustate, zpi, yval);
	dau_set_val_flags(cpustate, (op >> 21) & 3, res);
}


static void d4_ppr(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void d4_pmr(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void d4_mpr(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void d4_mmr(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}



/***************************************************************************
    DAU FORM 5 IMPLEMENTATION
***************************************************************************/

static void d5_ic(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void d5_oc(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void d5_float(dsp32_state *cpustate, UINT32 op)
{
	double res = (double)(INT16)dau_read_pi_2bytes(cpustate, op >> 7);
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(cpustate, zpi, res);
	dau_set_val_flags(cpustate, (op >> 21) & 3, res);
}


static void d5_int(dsp32_state *cpustate, UINT32 op)
{
	double val = dau_read_pi_double_1st(cpustate, op >> 7, 0);
	int zpi = (op >> 0) & 0x7f;
	INT16 res;
	if (!(cpustate->DAUC & 0x10)) val = floor(val + 0.5);
	else val = ceil(val - 0.5);
	res = (INT16)val;
	if (zpi != 7)
		dau_write_pi_2bytes(cpustate, zpi, res);
	dau_set_val_noflags(cpustate, (op >> 21) & 3, dsp_to_double(res << 16));
}


static void d5_round(dsp32_state *cpustate, UINT32 op)
{
	double res = (double)(float)dau_read_pi_double_1st(cpustate, op >> 7, 0);
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(cpustate, zpi, res);
	dau_set_val_flags(cpustate, (op >> 21) & 3, res);
}


static void d5_ifalt(dsp32_state *cpustate, UINT32 op)
{
	int ar = (op >> 21) & 3;
	double res = cpustate->a[ar];
	int zpi = (op >> 0) & 0x7f;
	if (NFLAG)
		res = dau_read_pi_double_1st(cpustate, op >> 7, 0);
	if (zpi != 7)
		dau_write_pi_double(cpustate, zpi, res);
	dau_set_val_noflags(cpustate, ar, res);
}


static void d5_ifaeq(dsp32_state *cpustate, UINT32 op)
{
	int ar = (op >> 21) & 3;
	double res = cpustate->a[ar];
	int zpi = (op >> 0) & 0x7f;
	if (ZFLAG)
		res = dau_read_pi_double_1st(cpustate, op >> 7, 0);
	if (zpi != 7)
		dau_write_pi_double(cpustate, zpi, res);
	dau_set_val_noflags(cpustate, ar, res);
}


static void d5_ifagt(dsp32_state *cpustate, UINT32 op)
{
	int ar = (op >> 21) & 3;
	double res = cpustate->a[ar];
	int zpi = (op >> 0) & 0x7f;
	if (!NFLAG && !ZFLAG)
		res = dau_read_pi_double_1st(cpustate, op >> 7, 0);
	if (zpi != 7)
		dau_write_pi_double(cpustate, zpi, res);
	dau_set_val_noflags(cpustate, ar, res);
}


static void d5_float24(dsp32_state *cpustate, UINT32 op)
{
	double res = (double)((INT32)(dau_read_pi_4bytes(cpustate, op >> 7) << 8) >> 8);
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_double(cpustate, zpi, res);
	dau_set_val_flags(cpustate, (op >> 21) & 3, res);
}


static void d5_int24(dsp32_state *cpustate, UINT32 op)
{
	double val = dau_read_pi_double_1st(cpustate, op >> 7, 0);
	int zpi = (op >> 0) & 0x7f;
	INT32 res;
	if (!(cpustate->DAUC & 0x10)) val = floor(val + 0.5);
	else val = ceil(val - 0.5);
	res = (INT32)val;
	if (res > 0x7fffff) res = 0x7fffff;
	else if (res < -0x800000) res = -0x800000;
	if (zpi != 7)
		dau_write_pi_4bytes(cpustate, zpi, (INT32)(res << 8) >> 8);
	dau_set_val_noflags(cpustate, (op >> 21) & 3, dsp_to_double(res << 8));
}


static void d5_ieee(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void d5_dsp(dsp32_state *cpustate, UINT32 op)
{
	unimplemented(cpustate, op);
}


static void d5_seed(dsp32_state *cpustate, UINT32 op)
{
	UINT32 val = dau_read_pi_4bytes(cpustate, op >> 7);
	INT32 res = val ^ 0x7fffffff;
	int zpi = (op >> 0) & 0x7f;
	if (zpi != 7)
		dau_write_pi_4bytes(cpustate, zpi, res);
	dau_set_val_flags(cpustate, (op >> 21) & 3, dsp_to_double((INT32)res));
}



/***************************************************************************
    FUNCTION TABLE
***************************************************************************/

void (*const dsp32ops[])(dsp32_state *cpustate, UINT32 op) =
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
