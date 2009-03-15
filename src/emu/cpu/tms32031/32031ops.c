/***************************************************************************

    32031ops.c
    Core implementation for the portable TMS32C031 emulator.
    Written by Aaron Giles

***************************************************************************/


/***************************************************************************
    COMPILE-TIME OPTIONS
***************************************************************************/

#define USE_FP				0



/***************************************************************************
    MACROS
***************************************************************************/

#define IREG(T,rnum)		((T)->r[rnum].i32[0])
#define FREGEXP(T,rnum)		(EXPONENT(&(T)->r[rnum]))
#define FREGMAN(T,rnum)		(MANTISSA(&(T)->r[rnum]))

#define FP2LONG(T,rnum)		((FREGEXP(T,rnum) << 24) | ((UINT32)FREGMAN(T,rnum) >> 8))
#define LONG2FP(T,rnum,v)	do { SET_MANTISSA(&(T)->r[rnum], (v) << 8); SET_EXPONENT(&(T)->r[rnum], (INT32)(v) >> 24); } while (0)
#define SHORT2FP(T,rnum,v)	do { \
								if ((UINT16)(v) == 0x8000) { SET_MANTISSA(&(T)->r[rnum], 0); SET_EXPONENT(&(T)->r[rnum], -128); } \
								else { SET_MANTISSA(&(T)->r[rnum], (v) << 20); SET_EXPONENT(&(T)->r[rnum], (INT16)(v) >> 12); } \
							} while (0)

#define DIRECT(T,op)			(((IREG(T,TMR_DP) & 0xff) << 16) | ((UINT16)op))
#define INDIRECT_D(T,op,o)		((*indirect_d[((o) >> 3) & 31])(T,op,o))
#define INDIRECT_1(T,op,o)		((*indirect_1[((o) >> 3) & 31])(T,op,o))
#define INDIRECT_1_DEF(T,op,o)	((*indirect_1_def[((o) >> 3) & 31])(T,op,o,&defptr))

#define SIGN(val)			((val) & 0x80000000)

#define OVERFLOW_SUB(a,b,r)	((INT32)(((a) ^ (b)) & ((a) ^ (r))) < 0)
#define OVERFLOW_ADD(a,b,r)	((INT32)(~((a) ^ (b)) & ((a) ^ (r))) < 0)

#define CLR_FLAGS(T,f)		do { IREG(T,TMR_ST) &= ~(f); } while (0)
#define CLR_NVUF(T)			CLR_FLAGS(T, NFLAG | VFLAG | UFFLAG)
#define CLR_NZVUF(T)		CLR_FLAGS(T, NFLAG | ZFLAG | VFLAG | UFFLAG)
#define CLR_NZCVUF(T)		CLR_FLAGS(T, NFLAG | ZFLAG | VFLAG | CFLAG | UFFLAG)

#define OR_C(T,flag)		do { IREG(T,TMR_ST) |= flag & CFLAG; } while (0)
#define OR_NZ(T,val)		do { IREG(T, TMR_ST) |= (((val) >> 28) & NFLAG) | (((val) == 0) << 2); } while (0)
#define OR_NZF(T,reg)		do { IREG(T, TMR_ST) |= ((MANTISSA(reg) >> 28) & NFLAG) | ((EXPONENT(reg) == -128) << 2); } while (0)
#define OR_NUF(T,reg)		do { int temp = (EXPONENT(reg) == -128) << 4; IREG(T,TMR_ST) |= ((MANTISSA(reg) >> 28) & NFLAG) | (temp) | (temp << 2); } while (0)
#define OR_V_SUB(T,a,b,r)	do { UINT32 temp = ((((a) ^ (b)) & ((a) ^ (r))) >> 30) & VFLAG; IREG(T,TMR_ST) |= temp | (temp << 4); } while (0)
#define OR_V_ADD(T,a,b,r)	do { UINT32 temp = ((~((a) ^ (b)) & ((a) ^ (r))) >> 30) & VFLAG; IREG(T,TMR_ST) |= temp | (temp << 4); } while (0)
#define OR_C_SUB(T,a,b,r)	do { IREG(T,TMR_ST) |= ((UINT32)(b) > (UINT32)(a)); } while (0)
#define OR_C_ADD(T,a,b,r)	do { IREG(T,TMR_ST) |= ((UINT32)(~(a)) < (UINT32)(b)); } while (0)
#define OR_NZCV_SUB(T,a,b,r) do { OR_V_SUB(T,a,b,r); OR_C_SUB(T,a,b,r); OR_NZ(T,r); } while (0)
#define OR_NZCV_ADD(T,a,b,r) do { OR_V_ADD(T,a,b,r); OR_C_ADD(T,a,b,r); OR_NZ(T,r); } while (0)

#define OVM(T)				(IREG(T,TMR_ST) & OVMFLAG)

#define DECLARE_DEF			UINT32 defval; UINT32 *defptr = &defval
#define UPDATE_DEF()		*defptr = defval



/***************************************************************************
    FORWARD DECLARATIONS
***************************************************************************/

extern void (*const tms32031ops[])(tms32031_state *tms, UINT32 op);



/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

static void illegal(tms32031_state *tms, UINT32 op)
{
	if ((tms->device->machine->debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		logerror("Illegal op @ %06X: %08X (tbl=%03X)\n", tms->pc - 1, op, op >> 21);
		debugger_break(tms->device->machine);
	}
}


static void unimplemented(tms32031_state *tms, UINT32 op)
{
	fatalerror("Unimplemented op @ %06X: %08X (tbl=%03X)", tms->pc - 1, op, op >> 21);
}


INLINE void execute_one(tms32031_state *tms)
{
	UINT32 op = ROPCODE(tms, tms->pc);
	tms->icount -= 2;	/* 2 clocks per cycle */
	tms->pc++;
#if (LOG_OPCODE_USAGE)
	hits[op >> 21]++;
#endif
	(*tms32031ops[op >> 21])(tms, op);
}


static void update_special(tms32031_state *tms, int dreg)
{
	if (dreg == TMR_BK)
	{
		UINT32 temp = IREG(tms, TMR_BK);
		tms->bkmask = temp;
		while (temp >>= 1)
			tms->bkmask |= temp;
	}
	else if (dreg == TMR_IOF)
	{
		if (tms->xf0_w != NULL && IREG(tms, TMR_IOF) & 0x002)
			(*tms->xf0_w)(tms->device, (IREG(tms, TMR_IOF) >> 2) & 1);
		if (tms->xf1_w != NULL && IREG(tms, TMR_IOF) & 0x020)
			(*tms->xf1_w)(tms->device, (IREG(tms, TMR_IOF) >> 6) & 1);
	}
	else if (dreg == TMR_ST || dreg == TMR_IF || dreg == TMR_IE)
		check_irqs(tms);
}



/***************************************************************************
    CONDITION CODES
***************************************************************************/

#define	CONDITION_LO(T)		(IREG(T,TMR_ST) & CFLAG)
#define CONDITION_LS(T)		(IREG(T,TMR_ST) & (CFLAG | ZFLAG))
#define CONDITION_HI(T)		(!(IREG(T,TMR_ST) & (CFLAG | ZFLAG)))
#define CONDITION_HS(T)		(!(IREG(T,TMR_ST) & CFLAG))
#define CONDITION_EQ(T)		(IREG(T,TMR_ST) & ZFLAG)
#define CONDITION_NE(T)		(!(IREG(T,TMR_ST) & ZFLAG))
#define CONDITION_LT(T)		(IREG(T,TMR_ST) & NFLAG)
#define CONDITION_LE(T)		(IREG(T,TMR_ST) & (NFLAG | ZFLAG))
#define CONDITION_GT(T)		(!(IREG(T,TMR_ST) & (NFLAG | ZFLAG)))
#define CONDITION_GE(T)		(!(IREG(T,TMR_ST) & NFLAG))
#define CONDITION_NV(T)		(!(IREG(T,TMR_ST) & VFLAG))
#define CONDITION_V(T)		(IREG(T,TMR_ST) & VFLAG)
#define CONDITION_NUF(T)	(!(IREG(T,TMR_ST) & UFFLAG))
#define CONDITION_UF(T)		(IREG(T,TMR_ST) & UFFLAG)
#define CONDITION_NLV(T)	(!(IREG(T,TMR_ST) & LVFLAG))
#define CONDITION_LV(T)		(IREG(T,TMR_ST) & LVFLAG)
#define CONDITION_NLUF(T)	(!(IREG(T,TMR_ST) & LUFFLAG))
#define CONDITION_LUF(T)	(IREG(T,TMR_ST) & LUFFLAG)
#define CONDITION_ZUF(T)	(IREG(T,TMR_ST) & (UFFLAG | ZFLAG))

static int condition(tms32031_state *tms, int which)
{
	switch (which & 0x1f)
	{
		case 0:		return 1;
		case 1:		return CONDITION_LO(tms);
		case 2:		return CONDITION_LS(tms);
		case 3:		return CONDITION_HI(tms);
		case 4:		return CONDITION_HS(tms);
		case 5:		return CONDITION_EQ(tms);
		case 6:		return CONDITION_NE(tms);
		case 7:		return CONDITION_LT(tms);
		case 8:		return CONDITION_LE(tms);
		case 9:		return CONDITION_GT(tms);
		case 10:	return CONDITION_GE(tms);
		case 12:	return CONDITION_NV(tms);
		case 13:	return CONDITION_V(tms);
		case 14:	return CONDITION_NUF(tms);
		case 15:	return CONDITION_UF(tms);
		case 16:	return CONDITION_NLV(tms);
		case 17:	return CONDITION_LV(tms);
		case 18:	return CONDITION_NLUF(tms);
		case 19:	return CONDITION_LUF(tms);
		case 20:	return CONDITION_ZUF(tms);
		default:	illegal(tms, 0); return 1;
	}
}



/***************************************************************************
    FLOATING POINT HELPERS
***************************************************************************/

#if USE_FP
void double_to_dsp_with_flags(tms32031_state *tms, double val, tmsreg *result)
{
	int mantissa, exponent;
	int_double id;
	id.d = val;

	CLR_NZVUF(tms);

	mantissa = ((id.i[BYTE_XOR_BE(0)] & 0x000fffff) << 11) | ((id.i[BYTE_XOR_BE(1)] & 0xffe00000) >> 21);
	exponent = ((id.i[BYTE_XOR_BE(0)] & 0x7ff00000) >> 20) - 1023;
	if (exponent <= -128)
	{
		SET_MANTISSA(result, 0);
		SET_EXPONENT(result, -128);
		IREG(tms, TMR_ST) |= UFFLAG | LUFFLAG | ZFLAG;
	}
	else if (exponent > 127)
	{
		if ((INT32)id.i[BYTE_XOR_BE(0)] >= 0)
			SET_MANTISSA(result, 0x7fffffff);
		else
		{
			SET_MANTISSA(result, 0x80000001);
			IREG(tms, TMR_ST) |= NFLAG;
		}
		SET_EXPONENT(result, 127);
		IREG(tms, TMR_ST) |= VFLAG | LVFLAG;
	}
	else if (val == 0)
	{
		SET_MANTISSA(result, 0);
		SET_EXPONENT(result, -128);
		IREG(tms, TMR_ST) |= ZFLAG;
	}
	else if ((INT32)id.i[BYTE_XOR_BE(0)] >= 0)
	{
		SET_MANTISSA(result, mantissa);
		SET_EXPONENT(result, exponent);
	}
	else if (mantissa != 0)
	{
		SET_MANTISSA(result, 0x80000000 | -mantissa);
		SET_EXPONENT(result, exponent);
		IREG(tms, TMR_ST) |= NFLAG;
	}
	else
	{
		SET_MANTISSA(result, 0x80000000);
		SET_EXPONENT(result, exponent - 1);
		IREG(tms, TMR_ST) |= NFLAG;
	}
}
#endif

/* integer to floating point conversion */
#if USE_FP
static void int2float(tms32031_state *tms, tmsreg *srcdst)
{
	double val = MANTISSA(srcdst);
	double_to_dsp_with_flags(tms, val, srcdst);
}
#else
static void int2float(tms32031_state *tms, tmsreg *srcdst)
{
	UINT32 man = MANTISSA(srcdst);
	int exp, cnt;

	/* never overflows or underflows */
	CLR_NZVUF(tms);

	/* 0 always has exponent of -128 */
	if (man == 0)
	{
		man = 0x80000000;
		exp = -128;
	}

	/* check for -1 here because count_leading_ones will infinite loop */
	else if (man == (UINT32)-1)
	{
		man = 0;
		exp = -1;
	}

	/* positive values; count leading zeros and shift */
	else if ((INT32)man > 0)
	{
		cnt = count_leading_zeros(man);
		man <<= cnt;
		exp = 31 - cnt;
	}

	/* negative values; count leading ones and shift */
	else
	{
		cnt = count_leading_ones(man);
		man <<= cnt;
		exp = 31 - cnt;
	}

	/* set the final results and compute NZ */
	SET_MANTISSA(srcdst, man ^ 0x80000000);
	SET_EXPONENT(srcdst, exp);
	OR_NZF(tms, srcdst);
}
#endif


/* floating point to integer conversion */
#if USE_FP
static void float2int(tms32031_state *tms, tmsreg *srcdst, int setflags)
{
	INT32 val;

	if (setflags) CLR_NZVUF(tms);
	if (EXPONENT(srcdst) > 30)
	{
		if ((INT32)MANTISSA(srcdst) >= 0)
			val = 0x7fffffff;
		else
			val = 0x80000000;
		if (setflags) IREG(tms, TMR_ST) |= VFLAG | LVFLAG;
	}
	else
		val = floor(dsp_to_double(srcdst));
	SET_MANTISSA(srcdst, val);
	if (setflags) OR_NZ(tms, val);
}
#else
static void float2int(tms32031_state *tms, tmsreg *srcdst, int setflags)
{
	INT32 man = MANTISSA(srcdst);
	int shift = 31 - EXPONENT(srcdst);

	/* never underflows */
	if (setflags) CLR_NZVUF(tms);

	/* if we've got too much to handle, overflow */
	if (shift <= 0)
	{
		SET_MANTISSA(srcdst, (man >= 0) ? 0x7fffffff : 0x80000000);
		if (setflags) IREG(tms, TMR_ST) |= VFLAG | LVFLAG;
	}

	/* if we're too small, go to 0 or -1 */
	else if (shift > 31)
		SET_MANTISSA(srcdst, man >> 31);

	/* we're in the middle; shift it */
	else
		SET_MANTISSA(srcdst, (man >> shift) ^ (1 << (31 - shift)));

	/* set the NZ flags */
	if (setflags) OR_NZ(tms, MANTISSA(srcdst));
}
#endif


/* compute the negative of a floating point value */
#if USE_FP
static void negf(tms32031_state *tms, tmsreg *dst, tmsreg *src)
{
	double val = -dsp_to_double(src);
	double_to_dsp_with_flags(tms, val, dst);
}
#else
static void negf(tms32031_state *tms, tmsreg *dst, tmsreg *src)
{
	INT32 man = MANTISSA(src);

	CLR_NZVUF(tms);

	if (EXPONENT(src) == -128)
	{
		SET_MANTISSA(dst, 0);
		SET_EXPONENT(dst, -128);
	}
	else if ((man & 0x7fffffff) != 0)
	{
		SET_MANTISSA(dst, -man);
		SET_EXPONENT(dst, EXPONENT(src));
	}
	else
	{
		SET_MANTISSA(dst, man ^ 0x80000000);
		if (man == 0)
			SET_EXPONENT(dst, EXPONENT(src) - 1);
		else
			SET_EXPONENT(dst, EXPONENT(src) + 1);
	}
	OR_NZF(tms, dst);
}
#endif



/* add two floating point values */
#if USE_FP
static void addf(tms32031_state *tms, tmsreg *dst, tmsreg *src1, tmsreg *src2)
{
	double val = dsp_to_double(src1) + dsp_to_double(src2);
	double_to_dsp_with_flags(tms, val, dst);
}
#else
static void addf(tms32031_state *tms, tmsreg *dst, tmsreg *src1, tmsreg *src2)
{
	INT64 man;
	INT64 m1, m2;
	int exp, cnt;

	/* reset over/underflow conditions */
	CLR_NZVUF(tms);

	/* first check for 0 operands */
	if (EXPONENT(src1) == -128)
	{
		*dst = *src2;
		OR_NZF(tms, dst);
		return;
	}
	if (EXPONENT(src2) == -128)
	{
		*dst = *src1;
		OR_NZF(tms, dst);
		return;
	}

	/* extract mantissas from 1.0.31 values to 1.1.31 values */
	m1 = (INT64)MANTISSA(src1) ^ 0x80000000;
	m2 = (INT64)MANTISSA(src2) ^ 0x80000000;

	/* normalize based on the exponent */
	if (EXPONENT(src1) > EXPONENT(src2))
	{
		exp = EXPONENT(src1);
		cnt = exp - EXPONENT(src2);
		if (cnt >= 32)
		{
			*dst = *src1;
			OR_NZF(tms, dst);
			return;
		}
		m2 >>= cnt;
	}
	else
	{
		exp = EXPONENT(src2);
		cnt = exp - EXPONENT(src1);
		if (cnt >= 32)
		{
			*dst = *src2;
			OR_NZF(tms, dst);
			return;
		}
		m1 >>= cnt;
	}

	/* add */
	man = m1 + m2;

	/* if the mantissa is zero, set the exponent appropriately */
	if (man == 0 || exp == -128)
	{
		exp = -128;
		man = 0x80000000;
	}

	/* if the mantissa is >= 2.0 or < -2.0, normalize */
	else if (man >= ((INT64)2 << 31) || man < ((INT64)-2 << 31))
	{
		man >>= 1;
		exp++;
	}

	/* if the mantissa is < 1.0 and > -1.0, normalize */
	else if (man < ((INT64)1 << 31) && man >= ((INT64)-1 << 31))
	{
		if (man > 0)
		{
			cnt = count_leading_zeros((UINT32)man);
			man <<= cnt;
			exp -= cnt;
		}
		else
		{
			cnt = count_leading_ones((UINT32)man);
			man <<= cnt;
			exp -= cnt;
		}
	}

	/* check for underflow */
	if (exp <= -128)
	{
		man = 0x80000000;
		exp = -128;
		IREG(tms, TMR_ST) |= UFFLAG | LUFFLAG;
	}

	/* check for overflow */
	else if (exp > 127)
	{
		man = (man < 0) ? 0x00000000 : 0xffffffff;
		exp = 127;
		IREG(tms, TMR_ST) |= VFLAG | LVFLAG;
	}

	/* store the result back, removing the implicit one and putting */
	/* back the sign bit */
	SET_MANTISSA(dst, (UINT32)man ^ 0x80000000);
	SET_EXPONENT(dst, exp);
	OR_NZF(tms, dst);
}
#endif


/* subtract two floating point values */
#if USE_FP
static void subf(tms32031_state *tms, tmsreg *dst, tmsreg *src1, tmsreg *src2)
{
	double val = dsp_to_double(src1) - dsp_to_double(src2);
	double_to_dsp_with_flags(tms, val, dst);
}
#else
static void subf(tms32031_state *tms, tmsreg *dst, tmsreg *src1, tmsreg *src2)
{
	INT64 man;
	INT64 m1, m2;
	int exp, cnt;

	/* reset over/underflow conditions */
	CLR_NZVUF(tms);

	/* first check for 0 operands */
	if (EXPONENT(src2) == -128)
	{
		*dst = *src1;
		OR_NZF(tms, dst);
		return;
	}

	/* extract mantissas from 1.0.31 values to 1.1.31 values */
	m1 = (INT64)MANTISSA(src1) ^ 0x80000000;
	m2 = (INT64)MANTISSA(src2) ^ 0x80000000;

	/* normalize based on the exponent */
	if (EXPONENT(src1) > EXPONENT(src2))
	{
		exp = EXPONENT(src1);
		cnt = exp - EXPONENT(src2);
		if (cnt >= 32)
		{
			*dst = *src1;
			OR_NZF(tms, dst);
			return;
		}
		m2 >>= cnt;
	}
	else
	{
		exp = EXPONENT(src2);
		cnt = exp - EXPONENT(src1);
		if (cnt >= 32)
		{
			negf(tms, dst, src2);
			return;
		}
		m1 >>= cnt;
	}

	/* subtract */
	man = m1 - m2;

	/* if the mantissa is zero, set the exponent appropriately */
	if (man == 0 || exp == -128)
	{
		exp = -128;
		man = 0x80000000;
	}

	/* if the mantissa is >= 2.0 or < -2.0, normalize */
	else if (man >= ((INT64)2 << 31) || man < ((INT64)-2 << 31))
	{
		man >>= 1;
		exp++;
	}

	/* if the mantissa is < 1.0 and > -1.0, normalize */
	else if (man < ((INT64)1 << 31) && man >= ((INT64)-1 << 31))
	{
		if (man > 0)
		{
			cnt = count_leading_zeros((UINT32)man);
			man <<= cnt;
			exp -= cnt;
		}
		else
		{
			cnt = count_leading_ones((UINT32)man);
			man <<= cnt;
			exp -= cnt;
		}
	}

	/* check for underflow */
	if (exp <= -128)
	{
		man = 0x80000000;
		exp = -128;
		IREG(tms, TMR_ST) |= UFFLAG | LUFFLAG;
	}

	/* check for overflow */
	else if (exp > 127)
	{
		man = (man < 0) ? 0x00000000 : 0xffffffff;
		exp = 127;
		IREG(tms, TMR_ST) |= VFLAG | LVFLAG;
	}

	/* store the result back, removing the implicit one and putting */
	/* back the sign bit */
	SET_MANTISSA(dst, (UINT32)man ^ 0x80000000);
	SET_EXPONENT(dst, exp);
	OR_NZF(tms, dst);
}
#endif


/* multiply two floating point values */
#if USE_FP
static void mpyf(tms32031_state *tms, tmsreg *dst, tmsreg *src1, tmsreg *src2)
{
	double val = (double)dsp_to_float(src1) * (double)dsp_to_float(src2);
	double_to_dsp_with_flags(tms, val, dst);
}
#else
static void mpyf(tms32031_state *tms, tmsreg *dst, tmsreg *src1, tmsreg *src2)
{
	INT64 man;
	INT32 m1, m2;
	int exp;

	/* reset over/underflow conditions */
	CLR_NZVUF(tms);

	/* first check for 0 multipliers and return 0 in any case */
	if (EXPONENT(src1) == -128 || EXPONENT(src2) == -128)
	{
		SET_MANTISSA(dst, 0);
		SET_EXPONENT(dst, -128);
		OR_NZF(tms, dst);
		return;
	}

	/* convert the mantissas from 1.0.31 numbers to 1.1.23 numbers */
	m1 = (MANTISSA(src1) >> 8) ^ 0x800000;
	m2 = (MANTISSA(src2) >> 8) ^ 0x800000;

	/* multiply the mantissas and add the exponents */
	man = (INT64)m1 * (INT64)m2;
	exp = EXPONENT(src1) + EXPONENT(src2);

	/* chop off the low bits, going from 1.2.46 down to 1.2.31 */
	man >>= 46 - 31;

	/* if the mantissa is zero, set the exponent appropriately */
	if (man == 0)
	{
		exp = -128;
		man = 0x80000000;
	}

	/* if the mantissa is >= 2.0 or <= -2.0, normalize */
	else if (man >= ((INT64)2 << 31))
	{
		man >>= 1;
		exp++;
		if (man >= ((INT64)2 << 31))
		{
			man >>= 1;
			exp++;
		}
	}

	/* if the mantissa is >= 2.0 or <= -2.0, normalize */
	else if (man < ((INT64)-2 << 31))
	{
		man >>= 1;
		exp++;
	}

	/* check for underflow */
	if (exp <= -128)
	{
		man = 0x80000000;
		exp = -128;
		IREG(tms, TMR_ST) |= UFFLAG | LUFFLAG;
	}

	/* check for overflow */
	else if (exp > 127)
	{
		man = (man < 0) ? 0x00000000 : 0xffffffff;
		exp = 127;
		IREG(tms, TMR_ST) |= VFLAG | LVFLAG;
	}

	/* store the result back, removing the implicit one and putting */
	/* back the sign bit */
	SET_MANTISSA(dst, (UINT32)man ^ 0x80000000);
	SET_EXPONENT(dst, exp);
	OR_NZF(tms, dst);
}
#endif


/* normalize a floating point value */
#if USE_FP
static void norm(tms32031_state *tms, tmsreg *dst, tmsreg *src)
{
	fatalerror("norm not implemented");
}
#else
static void norm(tms32031_state *tms, tmsreg *dst, tmsreg *src)
{
	INT32 man = MANTISSA(src);
	int exp = EXPONENT(src);

	CLR_NZVUF(tms);

	if (exp == -128 || man == 0)
	{
		SET_MANTISSA(dst, 0);
		SET_EXPONENT(dst, -128);
		if (man != 0)
			IREG(tms, TMR_ST) |= UFFLAG | LUFFLAG;
	}
	else
	{
		int cnt;
		if (man > 0)
		{
			cnt = count_leading_zeros((UINT32)man);
			man <<= cnt;
			exp -= cnt;
		}
		else
		{
			cnt = count_leading_ones((UINT32)man);
			man <<= cnt;
			exp -= cnt;
		}

		/* check for underflow */
		if (exp <= -128)
		{
			man = 0x00000000;
			exp = -128;
			IREG(tms, TMR_ST) |= UFFLAG | LUFFLAG;
		}
	}

	SET_MANTISSA(dst, man);
	SET_EXPONENT(dst, exp);
	OR_NZF(tms, dst);
}
#endif




/***************************************************************************
    INDIRECT MEMORY REFS
***************************************************************************/

/* immediate displacement variants */

static UINT32 mod00_d(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(tms, reg) + (UINT8)op;
}

static UINT32 mod01_d(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(tms, reg) - (UINT8)op;
}

static UINT32 mod02_d(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	IREG(tms, reg) += (UINT8)op;
	return IREG(tms, reg);
}

static UINT32 mod03_d(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	IREG(tms, reg) -= (UINT8)op;
	return IREG(tms, reg);
}

static UINT32 mod04_d(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(tms, reg);
	IREG(tms, reg) += (UINT8)op;
	return result;
}

static UINT32 mod05_d(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(tms, reg);
	IREG(tms, reg) -= (UINT8)op;
	return result;
}

static UINT32 mod06_d(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(tms, reg);
	INT32 temp = (result & tms->bkmask) + (UINT8)op;
	if (temp >= IREG(tms, TMR_BK))
		temp -= IREG(tms, TMR_BK);
	IREG(tms, reg) = (IREG(tms, reg) & ~tms->bkmask) | (temp & tms->bkmask);
	return result;
}

static UINT32 mod07_d(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(tms, reg);
	INT32 temp = (result & tms->bkmask) - (UINT8)op;
	if (temp < 0)
		temp += IREG(tms, TMR_BK);
	IREG(tms, reg) = (IREG(tms, reg) & ~tms->bkmask) | (temp & tms->bkmask);
	return result;
}


/* immediate displacement variants (implied 1) */

static UINT32 mod00_1(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(tms, reg) + 1;
}

static UINT32 mod01_1(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(tms, reg) - 1;
}

static UINT32 mod02_1(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return ++IREG(tms, reg);
}

static UINT32 mod03_1(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return --IREG(tms, reg);
}

static UINT32 mod04_1(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(tms, reg)++;
}

static UINT32 mod05_1(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(tms, reg)--;
}

static UINT32 mod06_1(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(tms, reg);
	INT32 temp = (result & tms->bkmask) + 1;
	if (temp >= IREG(tms, TMR_BK))
		temp -= IREG(tms, TMR_BK);
	IREG(tms, reg) = (IREG(tms, reg) & ~tms->bkmask) | (temp & tms->bkmask);
	return result;
}

static UINT32 mod07_1(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(tms, reg);
	INT32 temp = (result & tms->bkmask) - 1;
	if (temp < 0)
		temp += IREG(tms, TMR_BK);
	IREG(tms, reg) = (IREG(tms, reg) & ~tms->bkmask) | (temp & tms->bkmask);
	return result;
}


/* IR0 displacement variants */

static UINT32 mod08(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(tms, reg) + IREG(tms, TMR_IR0);
}

static UINT32 mod09(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(tms, reg) - IREG(tms, TMR_IR0);
}

static UINT32 mod0a(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	IREG(tms, reg) += IREG(tms, TMR_IR0);
	return IREG(tms, reg);
}

static UINT32 mod0b(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	IREG(tms, reg) -= IREG(tms, TMR_IR0);
	return IREG(tms, reg);
}

static UINT32 mod0c(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(tms, reg);
	IREG(tms, reg) += IREG(tms, TMR_IR0);
	return result;
}

static UINT32 mod0d(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(tms, reg);
	IREG(tms, reg) -= IREG(tms, TMR_IR0);
	return result;
}

static UINT32 mod0e(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(tms, reg);
	INT32 temp = (result & tms->bkmask) + IREG(tms, TMR_IR0);
	if (temp >= IREG(tms, TMR_BK))
		temp -= IREG(tms, TMR_BK);
	IREG(tms, reg) = (IREG(tms, reg) & ~tms->bkmask) | (temp & tms->bkmask);
	return result;
}

static UINT32 mod0f(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(tms, reg);
	INT32 temp = (result & tms->bkmask) - IREG(tms, TMR_IR0);
	if (temp < 0)
		temp += IREG(tms, TMR_BK);
	IREG(tms, reg) = (IREG(tms, reg) & ~tms->bkmask) | (temp & tms->bkmask);
	return result;
}


/* IR1 displacement variants */

static UINT32 mod10(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(tms, reg) + IREG(tms, TMR_IR1);
}

static UINT32 mod11(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(tms, reg) - IREG(tms, TMR_IR1);
}

static UINT32 mod12(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	IREG(tms, reg) += IREG(tms, TMR_IR1);
	return IREG(tms, reg);
}

static UINT32 mod13(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	IREG(tms, reg) -= IREG(tms, TMR_IR1);
	return IREG(tms, reg);
}

static UINT32 mod14(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(tms, reg);
	IREG(tms, reg) += IREG(tms, TMR_IR1);
	return result;
}

static UINT32 mod15(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(tms, reg);
	IREG(tms, reg) -= IREG(tms, TMR_IR1);
	return result;
}

static UINT32 mod16(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(tms, reg);
	INT32 temp = (result & tms->bkmask) + IREG(tms, TMR_IR1);
	if (temp >= IREG(tms, TMR_BK))
		temp -= IREG(tms, TMR_BK);
	IREG(tms, reg) = (IREG(tms, reg) & ~tms->bkmask) | (temp & tms->bkmask);
	return result;
}

static UINT32 mod17(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(tms, reg);
	INT32 temp = (result & tms->bkmask) - IREG(tms, TMR_IR1);
	if (temp < 0)
		temp += IREG(tms, TMR_BK);
	IREG(tms, reg) = (IREG(tms, reg) & ~tms->bkmask) | (temp & tms->bkmask);
	return result;
}


/* special variants */

static UINT32 mod18(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(tms, reg);
}

static UINT32 mod19(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	unimplemented(tms, op);
	return 0;
}

static UINT32 modillegal(tms32031_state *tms, UINT32 op, UINT8 ar)
{
	illegal(tms, op);
	return 0;
}


/* immediate displacement variants (implied 1) */

static UINT32 mod00_1_def(tms32031_state *tms, UINT32 op, UINT8 ar, UINT32 **defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(tms, reg) + 1;
}

static UINT32 mod01_1_def(tms32031_state *tms, UINT32 op, UINT8 ar, UINT32 **defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(tms, reg) - 1;
}

static UINT32 mod02_1_def(tms32031_state *tms, UINT32 op, UINT8 ar, UINT32 **defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 defval = IREG(tms, reg) + 1;
	**defptrptr = defval;
	*defptrptr = &IREG(tms, reg);
	return defval;
}

static UINT32 mod03_1_def(tms32031_state *tms, UINT32 op, UINT8 ar, UINT32 **defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 defval = IREG(tms, reg) - 1;
	**defptrptr = defval;
	*defptrptr = &IREG(tms, reg);
	return defval;
}

static UINT32 mod04_1_def(tms32031_state *tms, UINT32 op, UINT8 ar, UINT32 **defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	**defptrptr = IREG(tms, reg) + 1;
	*defptrptr = &IREG(tms, reg);
	return IREG(tms, reg);
}

static UINT32 mod05_1_def(tms32031_state *tms, UINT32 op, UINT8 ar, UINT32 **defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	**defptrptr = IREG(tms, reg) - 1;
	*defptrptr = &IREG(tms, reg);
	return IREG(tms, reg);
}

static UINT32 mod06_1_def(tms32031_state *tms, UINT32 op, UINT8 ar, UINT32 **defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(tms, reg);
	INT32 temp = (result & tms->bkmask) + 1;
	if (temp >= IREG(tms, TMR_BK))
		temp -= IREG(tms, TMR_BK);
	**defptrptr = (IREG(tms, reg) & ~tms->bkmask) | (temp & tms->bkmask);
	*defptrptr = &IREG(tms, reg);
	return result;
}

static UINT32 mod07_1_def(tms32031_state *tms, UINT32 op, UINT8 ar, UINT32 **defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(tms, reg);
	INT32 temp = (result & tms->bkmask) - 1;
	if (temp < 0)
		temp += IREG(tms, TMR_BK);
	**defptrptr = (IREG(tms, reg) & ~tms->bkmask) | (temp & tms->bkmask);
	*defptrptr = &IREG(tms, reg);
	return result;
}


/* IR0 displacement variants */

static UINT32 mod08_def(tms32031_state *tms, UINT32 op, UINT8 ar, UINT32 **defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(tms, reg) + IREG(tms, TMR_IR0);
}

static UINT32 mod09_def(tms32031_state *tms, UINT32 op, UINT8 ar, UINT32 **defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(tms, reg) - IREG(tms, TMR_IR0);
}

static UINT32 mod0a_def(tms32031_state *tms, UINT32 op, UINT8 ar, UINT32 **defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 defval = IREG(tms, reg) + IREG(tms, TMR_IR0);
	**defptrptr = defval;
	*defptrptr = &IREG(tms, reg);
	return defval;
}

static UINT32 mod0b_def(tms32031_state *tms, UINT32 op, UINT8 ar, UINT32 **defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 defval = IREG(tms, reg) - IREG(tms, TMR_IR0);
	**defptrptr = defval;
	*defptrptr = &IREG(tms, reg);
	return defval;
}

static UINT32 mod0c_def(tms32031_state *tms, UINT32 op, UINT8 ar, UINT32 **defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	**defptrptr = IREG(tms, reg) + IREG(tms, TMR_IR0);
	*defptrptr = &IREG(tms, reg);
	return IREG(tms, reg);
}

static UINT32 mod0d_def(tms32031_state *tms, UINT32 op, UINT8 ar, UINT32 **defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	**defptrptr = IREG(tms, reg) - IREG(tms, TMR_IR0);
	*defptrptr = &IREG(tms, reg);
	return IREG(tms, reg);
}

static UINT32 mod0e_def(tms32031_state *tms, UINT32 op, UINT8 ar, UINT32 **defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(tms, reg);
	INT32 temp = (result & tms->bkmask) + IREG(tms, TMR_IR0);
	if (temp >= IREG(tms, TMR_BK))
		temp -= IREG(tms, TMR_BK);
	**defptrptr = (IREG(tms, reg) & ~tms->bkmask) | (temp & tms->bkmask);
	*defptrptr = &IREG(tms, reg);
	return result;
}

static UINT32 mod0f_def(tms32031_state *tms, UINT32 op, UINT8 ar, UINT32 **defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(tms, reg);
	INT32 temp = (result & tms->bkmask) - IREG(tms, TMR_IR0);
	if (temp < 0)
		temp += IREG(tms, TMR_BK);
	**defptrptr = (IREG(tms, reg) & ~tms->bkmask) | (temp & tms->bkmask);
	*defptrptr = &IREG(tms, reg);
	return result;
}


/* IR1 displacement variants */

static UINT32 mod10_def(tms32031_state *tms, UINT32 op, UINT8 ar, UINT32 **defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(tms, reg) + IREG(tms, TMR_IR1);
}

static UINT32 mod11_def(tms32031_state *tms, UINT32 op, UINT8 ar, UINT32 **defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(tms, reg) - IREG(tms, TMR_IR1);
}

static UINT32 mod12_def(tms32031_state *tms, UINT32 op, UINT8 ar, UINT32 **defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 defval = IREG(tms, reg) + IREG(tms, TMR_IR1);
	**defptrptr = defval;
	*defptrptr = &IREG(tms, reg);
	return defval;
}

static UINT32 mod13_def(tms32031_state *tms, UINT32 op, UINT8 ar, UINT32 **defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 defval = IREG(tms, reg) - IREG(tms, TMR_IR1);
	**defptrptr = defval;
	*defptrptr = &IREG(tms, reg);
	return defval;
}

static UINT32 mod14_def(tms32031_state *tms, UINT32 op, UINT8 ar, UINT32 **defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	**defptrptr = IREG(tms, reg) + IREG(tms, TMR_IR1);
	*defptrptr = &IREG(tms, reg);
	return IREG(tms, reg);
}

static UINT32 mod15_def(tms32031_state *tms, UINT32 op, UINT8 ar, UINT32 **defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	**defptrptr = IREG(tms, reg) - IREG(tms, TMR_IR1);
	*defptrptr = &IREG(tms, reg);
	return IREG(tms, reg);
}

static UINT32 mod16_def(tms32031_state *tms, UINT32 op, UINT8 ar, UINT32 **defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(tms, reg);
	INT32 temp = (result & tms->bkmask) + IREG(tms, TMR_IR1);
	if (temp >= IREG(tms, TMR_BK))
		temp -= IREG(tms, TMR_BK);
	**defptrptr = (IREG(tms, reg) & ~tms->bkmask) | (temp & tms->bkmask);
	*defptrptr = &IREG(tms, reg);
	return result;
}

static UINT32 mod17_def(tms32031_state *tms, UINT32 op, UINT8 ar, UINT32 **defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(tms, reg);
	INT32 temp = (result & tms->bkmask) - IREG(tms, TMR_IR1);
	if (temp < 0)
		temp += IREG(tms, TMR_BK);
	**defptrptr = (IREG(tms, reg) & ~tms->bkmask) | (temp & tms->bkmask);
	*defptrptr = &IREG(tms, reg);
	return result;
}

static UINT32 mod18_def(tms32031_state *tms, UINT32 op, UINT8 ar, UINT32 **defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(tms, reg);
}

static UINT32 mod19_def(tms32031_state *tms, UINT32 op, UINT8 ar, UINT32 **defptrptr)
{
	unimplemented(tms, op);
	return 0;
}

static UINT32 modillegal_def(tms32031_state *tms, UINT32 op, UINT8 ar, UINT32 **defptrptr)
{
	illegal(tms, op);
	return 0;
}

/* tables */

static UINT32 (*const indirect_d[0x20])(tms32031_state *, UINT32, UINT8) =
{
	mod00_d,	mod01_d,	mod02_d,	mod03_d,	mod04_d,	mod05_d,	mod06_d,	mod07_d,
	mod08,		mod09,		mod0a,		mod0b,		mod0c,		mod0d,		mod0e,		mod0f,
	mod10,		mod11,		mod12,		mod13,		mod14,		mod15,		mod16,		mod17,
	mod18,		mod19,		modillegal,	modillegal,	modillegal,	modillegal,	modillegal,	modillegal
};


static UINT32 (*const indirect_1[0x20])(tms32031_state *, UINT32, UINT8) =
{
	mod00_1,	mod01_1,	mod02_1,	mod03_1,	mod04_1,	mod05_1,	mod06_1,	mod07_1,
	mod08,		mod09,		mod0a,		mod0b,		mod0c,		mod0d,		mod0e,		mod0f,
	mod10,		mod11,		mod12,		mod13,		mod14,		mod15,		mod16,		mod17,
	mod18,		mod19,		modillegal,	modillegal,	modillegal,	modillegal,	modillegal,	modillegal
};


static UINT32 (*const indirect_1_def[0x20])(tms32031_state *, UINT32, UINT8, UINT32 **) =
{
	mod00_1_def,mod01_1_def,mod02_1_def,mod03_1_def,mod04_1_def,mod05_1_def,mod06_1_def,mod07_1_def,
	mod08_def,	mod09_def,	mod0a_def,	mod0b_def,	mod0c_def,	mod0d_def,	mod0e_def,	mod0f_def,
	mod10_def,	mod11_def,	mod12_def,	mod13_def,	mod14_def,	mod15_def,	mod16_def,	mod17_def,
	mod18_def,	mod19_def,	modillegal_def,	modillegal_def,	modillegal_def,	modillegal_def,	modillegal_def,	modillegal_def
};



/*-----------------------------------------------------*/

#define ABSF(dreg, sreg)												\
{																		\
	INT32 man = FREGMAN(tms, sreg);											\
	CLR_NZVUF(tms);														\
	tms->r[dreg] = tms->r[sreg];								\
	if (man < 0)														\
	{																	\
		SET_MANTISSA(&tms->r[dreg], ~man);							\
		if (man == (INT32)0x80000000 && FREGEXP(tms, sreg) == 127)			\
			IREG(tms, TMR_ST) |= VFLAG | LVFLAG;								\
	}																	\
	OR_NZF(tms, &tms->r[dreg]);											\
}

static void absf_reg(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 7;
	int sreg = op & 7;
	ABSF(dreg, sreg);
}

static void absf_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 res = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 7;
	LONG2FP(tms, TMR_TEMP1, res);
	ABSF(dreg, TMR_TEMP1);
}

static void absf_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 7;
	LONG2FP(tms, TMR_TEMP1, res);
	ABSF(dreg, TMR_TEMP1);
}

static void absf_imm(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 7;
	SHORT2FP(tms, TMR_TEMP1, op);
	ABSF(dreg, TMR_TEMP1);
}

/*-----------------------------------------------------*/

#define ABSI(dreg, src)												\
{																	\
	UINT32 _res = ((INT32)src < 0) ? -src : src;					\
	if (!OVM(tms) || _res != 0x80000000)									\
		IREG(tms, dreg) = _res;											\
	else															\
		IREG(tms, dreg) = 0x7fffffff;									\
	if (dreg < 8)													\
	{																\
		CLR_NZVUF(tms);												\
		OR_NZ(tms, _res);												\
		if (_res == 0x80000000) 									\
			IREG(tms, TMR_ST) |= VFLAG | LVFLAG;							\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(tms, dreg);										\
}

static void absi_reg(tms32031_state *tms, UINT32 op)
{
	UINT32 src = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	ABSI(dreg, src);
}

static void absi_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 31;
	ABSI(dreg, src);
}

static void absi_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 31;
	ABSI(dreg, src);
}

static void absi_imm(tms32031_state *tms, UINT32 op)
{
	UINT32 src = (INT16)op;
	int dreg = (op >> 16) & 31;
	ABSI(dreg, src);
}

/*-----------------------------------------------------*/

#define ADDC(dreg, src1, src2)										\
{																	\
	UINT32 _res = src1 + src2 + (IREG(tms, TMR_ST) & CFLAG);				\
	if (!OVM(tms) || !OVERFLOW_ADD(src1,src2,_res))						\
		IREG(tms, dreg) = _res;											\
	else															\
		IREG(tms, dreg) = ((INT32)src1 < 0) ? 0x80000000 : 0x7fffffff;	\
	if (dreg < 8)													\
	{																\
		UINT32 tempc = src2 + (IREG(tms, TMR_ST) & CFLAG);				\
		CLR_NZCVUF(tms);												\
		OR_NZCV_ADD(tms,src1,tempc,_res);								\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(tms, dreg);										\
}

static void addc_reg(tms32031_state *tms, UINT32 op)
{
	UINT32 src = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	ADDC(dreg, dst, src);
}

static void addc_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	ADDC(dreg, dst, src);
}

static void addc_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	ADDC(dreg, dst, src);
}

static void addc_imm(tms32031_state *tms, UINT32 op)
{
	UINT32 src = (INT16)op;
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	ADDC(dreg, dst, src);
}

/*-----------------------------------------------------*/

static void addf_reg(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 7;
	addf(tms, &tms->r[dreg], &tms->r[dreg], &tms->r[op & 7]);
}

static void addf_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 res = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 7;
	LONG2FP(tms, TMR_TEMP1, res);
	addf(tms, &tms->r[dreg], &tms->r[dreg], &tms->r[TMR_TEMP1]);
}

static void addf_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 7;
	LONG2FP(tms, TMR_TEMP1, res);
	addf(tms, &tms->r[dreg], &tms->r[dreg], &tms->r[TMR_TEMP1]);
}

static void addf_imm(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 7;
	SHORT2FP(tms, TMR_TEMP1, op);
	addf(tms, &tms->r[dreg], &tms->r[dreg], &tms->r[TMR_TEMP1]);
}

/*-----------------------------------------------------*/

#define ADDI(dreg, src1, src2)										\
{																	\
	UINT32 _res = src1 + src2;										\
	if (!OVM(tms) || !OVERFLOW_ADD(src1,src2,_res))						\
		IREG(tms, dreg) = _res;											\
	else															\
		IREG(tms, dreg) = ((INT32)src1 < 0) ? 0x80000000 : 0x7fffffff;	\
	if (dreg < 8)													\
	{																\
		CLR_NZCVUF(tms);												\
		OR_NZCV_ADD(tms,src1,src2,_res);								\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(tms, dreg);										\
}

static void addi_reg(tms32031_state *tms, UINT32 op)
{
	UINT32 src = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	ADDI(dreg, dst, src);
}

static void addi_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	ADDI(dreg, dst, src);
}

static void addi_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	ADDI(dreg, dst, src);
}

static void addi_imm(tms32031_state *tms, UINT32 op)
{
	UINT32 src = (INT16)op;
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	ADDI(dreg, dst, src);
}

/*-----------------------------------------------------*/

#define AND(dreg, src1, src2)										\
{																	\
	UINT32 _res = (src1) & (src2);									\
	IREG(tms, dreg) = _res;												\
	if (dreg < 8)													\
	{																\
		CLR_NZVUF(tms);												\
		OR_NZ(tms, _res);												\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(tms, dreg);										\
}

static void and_reg(tms32031_state *tms, UINT32 op)
{
	UINT32 src = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	AND(dreg, dst, src);
}

static void and_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	AND(dreg, dst, src);
}

static void and_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	AND(dreg, dst, src);
}

static void and_imm(tms32031_state *tms, UINT32 op)
{
	UINT32 src = (UINT16)op;
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	AND(dreg, dst, src);
}

/*-----------------------------------------------------*/

#define ANDN(dreg, src1, src2)										\
{																	\
	UINT32 _res = (src1) & ~(src2);									\
	IREG(tms, dreg) = _res;												\
	if (dreg < 8)													\
	{																\
		CLR_NZVUF(tms);												\
		OR_NZ(tms, _res);												\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(tms, dreg);										\
}

static void andn_reg(tms32031_state *tms, UINT32 op)
{
	UINT32 src = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	ANDN(dreg, dst, src);
}

static void andn_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	ANDN(dreg, dst, src);
}

static void andn_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	ANDN(dreg, dst, src);
}

static void andn_imm(tms32031_state *tms, UINT32 op)
{
	UINT32 src = (UINT16)op;
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	ANDN(dreg, dst, src);
}

/*-----------------------------------------------------*/

#define ASH(dreg, src, count)										\
{																	\
	UINT32 _res;													\
	INT32 _count = (INT16)(count << 9) >> 9;	/* 7 LSBs */		\
	if (_count < 0)													\
	{																\
		if (_count >= -31)											\
			_res = (INT32)src >> -_count;							\
		else														\
			_res = (INT32)src >> 31;								\
	}																\
	else															\
	{																\
		if (_count <= 31)											\
			_res = (INT32)src << _count;							\
		else														\
			_res = 0;												\
	}																\
	IREG(tms, dreg) = _res;												\
	if (dreg < 8)													\
	{																\
		CLR_NZCVUF(tms);												\
		OR_NZ(tms, _res);												\
		if (_count < 0)												\
		{															\
			if (_count >= -32)										\
				OR_C(tms, ((INT32)src >> (-_count - 1)) & 1);			\
			else													\
				OR_C(tms, ((INT32)src >> 31) & 1);						\
		}															\
		else if (_count > 0)										\
		{															\
			if (_count <= 32)										\
				OR_C(tms, ((UINT32)src << (_count - 1)) >> 31);			\
		}															\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(tms, dreg);										\
}

static void ash_reg(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 31;
	int count = IREG(tms, op & 31);
	UINT32 src = IREG(tms, dreg);
	ASH(dreg, src, count);
}

static void ash_dir(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 31;
	int count = RMEM(tms, DIRECT(tms, op));
	UINT32 src = IREG(tms, dreg);
	ASH(dreg, src, count);
}

static void ash_ind(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 31;
	int count = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	UINT32 src = IREG(tms, dreg);
	ASH(dreg, src, count);
}

static void ash_imm(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 31;
	int count = op;
	UINT32 src = IREG(tms, dreg);
	ASH(dreg, src, count);
}

/*-----------------------------------------------------*/

static void cmpf_reg(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 7;
	subf(tms, &tms->r[TMR_TEMP2], &tms->r[dreg], &tms->r[op & 7]);
}

static void cmpf_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 res = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 7;
	LONG2FP(tms, TMR_TEMP1, res);
	subf(tms, &tms->r[TMR_TEMP2], &tms->r[dreg], &tms->r[TMR_TEMP1]);
}

static void cmpf_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 7;
	LONG2FP(tms, TMR_TEMP1, res);
	subf(tms, &tms->r[TMR_TEMP2], &tms->r[dreg], &tms->r[TMR_TEMP1]);
}

static void cmpf_imm(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 7;
	SHORT2FP(tms, TMR_TEMP1, op);
	subf(tms, &tms->r[TMR_TEMP2], &tms->r[dreg], &tms->r[TMR_TEMP1]);
}

/*-----------------------------------------------------*/

#define CMPI(src1, src2)											\
{																	\
	UINT32 _res = src1 - src2;										\
	CLR_NZCVUF(tms);													\
	OR_NZCV_SUB(tms,src1,src2,_res);									\
}

static void cmpi_reg(tms32031_state *tms, UINT32 op)
{
	UINT32 src = IREG(tms, op & 31);
	UINT32 dst = IREG(tms, (op >> 16) & 31);
	CMPI(dst, src);
}

static void cmpi_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, DIRECT(tms, op));
	UINT32 dst = IREG(tms, (op >> 16) & 31);
	CMPI(dst, src);
}

static void cmpi_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	UINT32 dst = IREG(tms, (op >> 16) & 31);
	CMPI(dst, src);
}

static void cmpi_imm(tms32031_state *tms, UINT32 op)
{
	UINT32 src = (INT16)op;
	UINT32 dst = IREG(tms, (op >> 16) & 31);
	CMPI(dst, src);
}

/*-----------------------------------------------------*/

static void fix_reg(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 31;
	tms->r[dreg] = tms->r[op & 7];
	float2int(tms, &tms->r[dreg], dreg < 8);
}

static void fix_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 res = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 31;
	LONG2FP(tms, dreg, res);
	float2int(tms, &tms->r[dreg], dreg < 8);
}

static void fix_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 31;
	LONG2FP(tms, dreg, res);
	float2int(tms, &tms->r[dreg], dreg < 8);
}

static void fix_imm(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 31;
	SHORT2FP(tms, dreg, op);
	float2int(tms, &tms->r[dreg], dreg < 8);
}

/*-----------------------------------------------------*/

#define FLOAT(dreg, src)											\
{																	\
	IREG(tms, dreg) = src;												\
	int2float(tms, &tms->r[dreg]);									\
}

static void float_reg(tms32031_state *tms, UINT32 op)
{
	UINT32 src = IREG(tms, op & 31);
	int dreg = (op >> 16) & 7;
	FLOAT(dreg, src);
}

static void float_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 7;
	FLOAT(dreg, src);
}

static void float_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 7;
	FLOAT(dreg, src);
}

static void float_imm(tms32031_state *tms, UINT32 op)
{
	UINT32 src = (INT16)op;
	int dreg = (op >> 16) & 7;
	FLOAT(dreg, src);
}

/*-----------------------------------------------------*/

static void idle(tms32031_state *tms, UINT32 op)
{
	tms->is_idling = TRUE;
	IREG(tms, TMR_ST) |= GIEFLAG;
	check_irqs(tms);
	if (tms->is_idling)
		tms->icount = 0;
}

/*-----------------------------------------------------*/

static void lde_reg(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 7;
	SET_EXPONENT(&tms->r[dreg], EXPONENT(&tms->r[op & 7]));
	if (EXPONENT(&tms->r[dreg]) == -128)
		SET_MANTISSA(&tms->r[dreg], 0);
}

static void lde_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 res = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 7;
	LONG2FP(tms, TMR_TEMP1, res);
	SET_EXPONENT(&tms->r[dreg], EXPONENT(&tms->r[TMR_TEMP1]));
	if (EXPONENT(&tms->r[dreg]) == -128)
		SET_MANTISSA(&tms->r[dreg], 0);
}

static void lde_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 7;
	LONG2FP(tms, TMR_TEMP1, res);
	SET_EXPONENT(&tms->r[dreg], EXPONENT(&tms->r[TMR_TEMP1]));
	if (EXPONENT(&tms->r[dreg]) == -128)
		SET_MANTISSA(&tms->r[dreg], 0);
}

static void lde_imm(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 7;
	SHORT2FP(tms, TMR_TEMP1, op);
	SET_EXPONENT(&tms->r[dreg], EXPONENT(&tms->r[TMR_TEMP1]));
	if (EXPONENT(&tms->r[dreg]) == -128)
		SET_MANTISSA(&tms->r[dreg], 0);
}

/*-----------------------------------------------------*/

static void ldf_reg(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 7;
	tms->r[dreg] = tms->r[op & 7];
	CLR_NZVUF(tms);
	OR_NZF(tms, &tms->r[dreg]);
}

static void ldf_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 res = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 7;
	LONG2FP(tms, dreg, res);
	CLR_NZVUF(tms);
	OR_NZF(tms, &tms->r[dreg]);
}

static void ldf_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 7;
	LONG2FP(tms, dreg, res);
	CLR_NZVUF(tms);
	OR_NZF(tms, &tms->r[dreg]);
}

static void ldf_imm(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 7;
	SHORT2FP(tms, dreg, op);
	CLR_NZVUF(tms);
	OR_NZF(tms, &tms->r[dreg]);
}

/*-----------------------------------------------------*/

static void ldfi_dir(tms32031_state *tms, UINT32 op) { unimplemented(tms, op); }
static void ldfi_ind(tms32031_state *tms, UINT32 op) { unimplemented(tms, op); }

/*-----------------------------------------------------*/

#define LDI(dreg, src)												\
{																	\
	IREG(tms, dreg) = src;												\
	if (dreg < 8)													\
	{																\
		CLR_NZVUF(tms);												\
		OR_NZ(tms, src);													\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(tms, dreg);										\
}

static void ldi_reg(tms32031_state *tms, UINT32 op)
{
	UINT32 src = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	LDI(dreg, src);
}

static void ldi_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 31;
	LDI(dreg, src);
}

static void ldi_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 31;
	LDI(dreg, src);
}

static void ldi_imm(tms32031_state *tms, UINT32 op)
{
	UINT32 src = (INT16)op;
	int dreg = (op >> 16) & 31;
	LDI(dreg, src);
}

/*-----------------------------------------------------*/

static void ldii_dir(tms32031_state *tms, UINT32 op) { unimplemented(tms, op); }
static void ldii_ind(tms32031_state *tms, UINT32 op) { unimplemented(tms, op); }

/*-----------------------------------------------------*/

static void ldm_reg(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 7;
	SET_MANTISSA(&tms->r[dreg], MANTISSA(&tms->r[op & 7]));
}

static void ldm_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 res = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 7;
	SET_MANTISSA(&tms->r[dreg], res);
}

static void ldm_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 7;
	SET_MANTISSA(&tms->r[dreg], res);
}

static void ldm_imm(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 7;
	SHORT2FP(tms, TMR_TEMP1, op);
	SET_MANTISSA(&tms->r[dreg], MANTISSA(&tms->r[TMR_TEMP1]));
}

/*-----------------------------------------------------*/

#define LSH(dreg, src, count)										\
{																	\
	UINT32 _res;													\
	INT32 _count = (INT16)(count << 9) >> 9;	/* 7 LSBs */		\
	if (_count < 0)													\
	{																\
		if (_count >= -31)											\
			_res = (UINT32)src >> -_count;							\
		else														\
			_res = 0;												\
	}																\
	else															\
	{																\
		if (_count <= 31)											\
			_res = (UINT32)src << _count;							\
		else														\
			_res = 0;												\
	}																\
	IREG(tms, dreg) = _res;												\
	if (dreg < 8)													\
	{																\
		CLR_NZCVUF(tms);												\
		OR_NZ(tms, _res);												\
		if (_count < 0)												\
		{															\
			if (_count >= -32)										\
				OR_C(tms, ((UINT32)src >> (-_count - 1)) & 1);			\
		}															\
		else if (_count > 0)										\
		{															\
			if (_count <= 32)										\
				OR_C(tms, ((UINT32)src << (_count - 1)) >> 31);			\
		}															\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(tms, dreg);										\
}

static void lsh_reg(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 31;
	int count = IREG(tms, op & 31);
	UINT32 src = IREG(tms, dreg);
	LSH(dreg, src, count);
}

static void lsh_dir(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 31;
	int count = RMEM(tms, DIRECT(tms, op));
	UINT32 src = IREG(tms, dreg);
	LSH(dreg, src, count);
}

static void lsh_ind(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 31;
	int count = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	UINT32 src = IREG(tms, dreg);
	LSH(dreg, src, count);
}

static void lsh_imm(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 31;
	int count = op;
	UINT32 src = IREG(tms, dreg);
	LSH(dreg, src, count);
}

/*-----------------------------------------------------*/

static void mpyf_reg(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 31;
	mpyf(tms, &tms->r[dreg], &tms->r[dreg], &tms->r[op & 31]);
}

static void mpyf_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 res = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 31;
	LONG2FP(tms, TMR_TEMP1, res);
	mpyf(tms, &tms->r[dreg], &tms->r[dreg], &tms->r[TMR_TEMP1]);
}

static void mpyf_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 31;
	LONG2FP(tms, TMR_TEMP1, res);
	mpyf(tms, &tms->r[dreg], &tms->r[dreg], &tms->r[TMR_TEMP1]);
}

static void mpyf_imm(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 31;
	SHORT2FP(tms, TMR_TEMP1, op);
	mpyf(tms, &tms->r[dreg], &tms->r[dreg], &tms->r[TMR_TEMP1]);
}

/*-----------------------------------------------------*/

#define MPYI(dreg, src1, src2)										\
{																	\
	INT64 _res = (INT64)((INT32)(src1 << 8) >> 8) * (INT64)((INT32)(src2 << 8) >> 8);\
	if (!OVM(tms) || (_res >= -0x80000000 && _res <= 0x7fffffff))		\
		IREG(tms, dreg) = _res;											\
	else															\
		IREG(tms, dreg) = (_res < 0) ? 0x80000000 : 0x7fffffff;			\
	if (dreg < 8)													\
	{																\
		CLR_NZVUF(tms);												\
		OR_NZ(tms, (UINT32)_res);										\
		if (_res < -(INT64)0x80000000 || _res > (INT64)0x7fffffff)	\
			IREG(tms, TMR_ST) |= VFLAG | LVFLAG;							\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(tms, dreg);										\
}

static void mpyi_reg(tms32031_state *tms, UINT32 op)
{
	UINT32 src = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	MPYI(dreg, dst, src);
}

static void mpyi_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	MPYI(dreg, dst, src);
}

static void mpyi_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	MPYI(dreg, dst, src);
}

static void mpyi_imm(tms32031_state *tms, UINT32 op)
{
	UINT32 src = (INT16)op;
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	MPYI(dreg, dst, src);
}

/*-----------------------------------------------------*/

#define NEGB(dreg, src)												\
{																	\
	UINT32 temps = 0 - (IREG(tms, TMR_ST) & CFLAG);						\
	UINT32 _res = temps - src;										\
	if (!OVM(tms) || !OVERFLOW_SUB(temps,src,_res))						\
		IREG(tms, dreg) = _res;											\
	else															\
		IREG(tms, dreg) = ((INT32)src < 0) ? 0x80000000 : 0x7fffffff;	\
	if (dreg < 8)													\
	{																\
		CLR_NZCVUF(tms);												\
		OR_NZCV_SUB(tms,temps,src,_res);								\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(tms, dreg);										\
}

static void negb_reg(tms32031_state *tms, UINT32 op)
{
	UINT32 src = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	NEGB(dreg, src);
}

static void negb_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 31;
	NEGB(dreg, src);
}

static void negb_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 31;
	NEGB(dreg, src);
}

static void negb_imm(tms32031_state *tms, UINT32 op)
{
	UINT32 src = (INT16)op;
	int dreg = (op >> 16) & 31;
	NEGB(dreg, src);
}

/*-----------------------------------------------------*/

static void negf_reg(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 7;
	negf(tms, &tms->r[dreg], &tms->r[op & 7]);
}

static void negf_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 res = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 7;
	LONG2FP(tms, TMR_TEMP1, res);
	negf(tms, &tms->r[dreg], &tms->r[TMR_TEMP1]);
}

static void negf_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 7;
	LONG2FP(tms, TMR_TEMP1, res);
	negf(tms, &tms->r[dreg], &tms->r[TMR_TEMP1]);
}

static void negf_imm(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 7;
	SHORT2FP(tms, TMR_TEMP1, op);
	negf(tms, &tms->r[dreg], &tms->r[TMR_TEMP1]);
}

/*-----------------------------------------------------*/

#define NEGI(dreg, src)												\
{																	\
	UINT32 _res = 0 - src;											\
	if (!OVM(tms) || !OVERFLOW_SUB(0,src,_res))							\
		IREG(tms, dreg) = _res;											\
	else															\
		IREG(tms, dreg) = ((INT32)src < 0) ? 0x80000000 : 0x7fffffff;	\
	if (dreg < 8)													\
	{																\
		CLR_NZCVUF(tms);												\
		OR_NZCV_SUB(tms,0,src,_res);									\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(tms, dreg);										\
}

static void negi_reg(tms32031_state *tms, UINT32 op)
{
	UINT32 src = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	NEGI(dreg, src);
}

static void negi_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 31;
	NEGI(dreg, src);
}

static void negi_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 31;
	NEGI(dreg, src);
}

static void negi_imm(tms32031_state *tms, UINT32 op)
{
	UINT32 src = (INT16)op;
	int dreg = (op >> 16) & 31;
	NEGI(dreg, src);
}

/*-----------------------------------------------------*/

static void nop_reg(tms32031_state *tms, UINT32 op)
{
}

static void nop_ind(tms32031_state *tms, UINT32 op)
{
	RMEM(tms, INDIRECT_D(tms, op, op >> 8));
}

/*-----------------------------------------------------*/

static void norm_reg(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 7;
	norm(tms, &tms->r[dreg], &tms->r[op & 7]);
}

static void norm_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 res = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 7;
	LONG2FP(tms, TMR_TEMP1, res);
	norm(tms, &tms->r[dreg], &tms->r[TMR_TEMP1]);
}

static void norm_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 7;
	LONG2FP(tms, TMR_TEMP1, res);
	norm(tms, &tms->r[dreg], &tms->r[TMR_TEMP1]);
}

static void norm_imm(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 7;
	SHORT2FP(tms, TMR_TEMP1, op);
	norm(tms, &tms->r[dreg], &tms->r[TMR_TEMP1]);
}

/*-----------------------------------------------------*/

#define NOT(dreg, src)												\
{																	\
	UINT32 _res = ~(src);											\
	IREG(tms, dreg) = _res;												\
	if (dreg < 8)													\
	{																\
		CLR_NZVUF(tms);												\
		OR_NZ(tms, _res);												\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(tms, dreg);										\
}

static void not_reg(tms32031_state *tms, UINT32 op)
{
	UINT32 src = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	NOT(dreg, src);
}

static void not_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 31;
	NOT(dreg, src);
}

static void not_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 31;
	NOT(dreg, src);
}

static void not_imm(tms32031_state *tms, UINT32 op)
{
	UINT32 src = (UINT16)op;
	int dreg = (op >> 16) & 31;
	NOT(dreg, src);
}

/*-----------------------------------------------------*/

static void pop(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 31;
	UINT32 val = RMEM(tms, IREG(tms, TMR_SP)--);
	IREG(tms, dreg) = val;
	if (dreg < 8)
	{
		CLR_NZVUF(tms);
		OR_NZ(tms, val);
	}
	else if (dreg >= TMR_BK)
		update_special(tms, dreg);
}

static void popf(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 7;
	UINT32 val = RMEM(tms, IREG(tms, TMR_SP)--);
	LONG2FP(tms, dreg, val);
	CLR_NZVUF(tms);
	OR_NZF(tms, &tms->r[dreg]);
}

static void push(tms32031_state *tms, UINT32 op)
{
	WMEM(tms, ++IREG(tms, TMR_SP), IREG(tms, (op >> 16) & 31));
}

static void pushf(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 7;
	WMEM(tms, ++IREG(tms, TMR_SP), FP2LONG(tms, dreg));
}

/*-----------------------------------------------------*/

#define OR(dreg, src1, src2)										\
{																	\
	UINT32 _res = (src1) | (src2);									\
	IREG(tms, dreg) = _res;												\
	if (dreg < 8)													\
	{																\
		CLR_NZVUF(tms);												\
		OR_NZ(tms, _res);												\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(tms, dreg);										\
}

static void or_reg(tms32031_state *tms, UINT32 op)
{
	UINT32 src = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	OR(dreg, dst, src);
}

static void or_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	OR(dreg, dst, src);
}

static void or_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	OR(dreg, dst, src);
}

static void or_imm(tms32031_state *tms, UINT32 op)
{
	UINT32 src = (UINT16)op;
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	OR(dreg, dst, src);
}

/*-----------------------------------------------------*/

static void maxspeed(tms32031_state *tms, UINT32 op) { unimplemented(tms, op); }

/*-----------------------------------------------------*/

#define RND(dreg)													\
{																	\
	INT32 man = FREGMAN(tms, dreg);										\
	CLR_NVUF(tms);														\
	if (man < 0x7fffff80)											\
	{																\
		SET_MANTISSA(&tms->r[dreg], ((UINT32)man + 0x80) & 0xffffff00);	\
		OR_NUF(tms, &tms->r[dreg]);									\
	}																\
	else if (FREGEXP(tms, dreg) < 127)									\
	{																\
		SET_MANTISSA(&tms->r[dreg], ((UINT32)man + 0x80) & 0x7fffff00);	\
		SET_EXPONENT(&tms->r[dreg], FREGEXP(tms, dreg) + 1);			\
		OR_NUF(tms, &tms->r[dreg]);									\
	}																\
	else															\
	{																\
		SET_MANTISSA(&tms->r[dreg], 0x7fffff00);				\
		IREG(tms, TMR_ST) |= VFLAG | LVFLAG;								\
	}																\
}

static void rnd_reg(tms32031_state *tms, UINT32 op)
{
	int sreg = op & 7;
	int dreg = (op >> 16) & 7;
	tms->r[dreg] = tms->r[sreg];
	RND(dreg);
}

static void rnd_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 res = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 7;
	LONG2FP(tms, dreg, res);
	RND(dreg);
}

static void rnd_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 7;
	LONG2FP(tms, dreg, res);
	RND(dreg);
}

static void rnd_imm(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 7;
	SHORT2FP(tms, dreg, op);
	RND(dreg);
}

/*-----------------------------------------------------*/

static void rol(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 31;
	UINT32 res = IREG(tms, dreg);
	int newcflag = res >> 31;
	res = (res << 1) | newcflag;
	IREG(tms, dreg) = res;
	if (dreg < 8)
	{
		CLR_NZCVUF(tms);
		OR_NZ(tms, res);
		OR_C(tms, newcflag);
	}
	else if (dreg >= TMR_BK)
		update_special(tms, dreg);
}

static void rolc(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 31;
	UINT32 res = IREG(tms, dreg);
	int newcflag = res >> 31;
	res = (res << 1) | (IREG(tms, TMR_ST) & CFLAG);
	IREG(tms, dreg) = res;
	if (dreg < 8)
	{
		CLR_NZCVUF(tms);
		OR_NZ(tms, res);
		OR_C(tms, newcflag);
	}
	else if (dreg >= TMR_BK)
		update_special(tms, dreg);
}

static void ror(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 31;
	UINT32 res = IREG(tms, dreg);
	int newcflag = res & 1;
	res = (res >> 1) | (newcflag << 31);
	IREG(tms, dreg) = res;
	if (dreg < 8)
	{
		CLR_NZCVUF(tms);
		OR_NZ(tms, res);
		OR_C(tms, newcflag);
	}
	else if (dreg >= TMR_BK)
		update_special(tms, dreg);
}

static void rorc(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 31;
	UINT32 res = IREG(tms, dreg);
	int newcflag = res & 1;
	res = (res >> 1) | ((IREG(tms, TMR_ST) & CFLAG) << 31);
	IREG(tms, dreg) = res;
	if (dreg < 8)
	{
		CLR_NZCVUF(tms);
		OR_NZ(tms, res);
		OR_C(tms, newcflag);
	}
	else if (dreg >= TMR_BK)
		update_special(tms, dreg);
}

/*-----------------------------------------------------*/

static void rtps_reg(tms32031_state *tms, UINT32 op)
{
	IREG(tms, TMR_RC) = IREG(tms, op & 31);
	IREG(tms, TMR_RS) = tms->pc;
	IREG(tms, TMR_RE) = tms->pc;
	IREG(tms, TMR_ST) |= RMFLAG;
	tms->icount -= 3*2;
	tms->delayed = TRUE;
}

static void rtps_dir(tms32031_state *tms, UINT32 op)
{
	IREG(tms, TMR_RC) = RMEM(tms, DIRECT(tms, op));
	IREG(tms, TMR_RS) = tms->pc;
	IREG(tms, TMR_RE) = tms->pc;
	IREG(tms, TMR_ST) |= RMFLAG;
	tms->icount -= 3*2;
	tms->delayed = TRUE;
}

static void rtps_ind(tms32031_state *tms, UINT32 op)
{
	IREG(tms, TMR_RC) = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	IREG(tms, TMR_RS) = tms->pc;
	IREG(tms, TMR_RE) = tms->pc;
	IREG(tms, TMR_ST) |= RMFLAG;
	tms->icount -= 3*2;
	tms->delayed = TRUE;
}

static void rtps_imm(tms32031_state *tms, UINT32 op)
{
	IREG(tms, TMR_RC) = (UINT16)op;
	IREG(tms, TMR_RS) = tms->pc;
	IREG(tms, TMR_RE) = tms->pc;
	IREG(tms, TMR_ST) |= RMFLAG;
	tms->icount -= 3*2;
	tms->delayed = TRUE;
}

/*-----------------------------------------------------*/

static void stf_dir(tms32031_state *tms, UINT32 op)
{
	WMEM(tms, DIRECT(tms, op), FP2LONG(tms, (op >> 16) & 7));
}

static void stf_ind(tms32031_state *tms, UINT32 op)
{
	WMEM(tms, INDIRECT_D(tms, op, op >> 8), FP2LONG(tms, (op >> 16) & 7));
}

/*-----------------------------------------------------*/

static void stfi_dir(tms32031_state *tms, UINT32 op) { unimplemented(tms, op); }
static void stfi_ind(tms32031_state *tms, UINT32 op) { unimplemented(tms, op); }

/*-----------------------------------------------------*/

static void sti_dir(tms32031_state *tms, UINT32 op)
{
	WMEM(tms, DIRECT(tms, op), IREG(tms, (op >> 16) & 31));
}

static void sti_ind(tms32031_state *tms, UINT32 op)
{
	WMEM(tms, INDIRECT_D(tms, op, op >> 8), IREG(tms, (op >> 16) & 31));
}

/*-----------------------------------------------------*/

static void stii_dir(tms32031_state *tms, UINT32 op) { unimplemented(tms, op); }
static void stii_ind(tms32031_state *tms, UINT32 op) { unimplemented(tms, op); }

/*-----------------------------------------------------*/

static void sigi(tms32031_state *tms, UINT32 op) { unimplemented(tms, op); }

/*-----------------------------------------------------*/

#define SUBB(dreg, src1, src2)										\
{																	\
	UINT32 temps = src1 - (IREG(tms, TMR_ST) & CFLAG);					\
	UINT32 _res = temps - src2;										\
	if (!OVM(tms) || !OVERFLOW_SUB(temps,src2,_res))						\
		IREG(tms, dreg) = _res;											\
	else															\
		IREG(tms, dreg) = ((INT32)src1 < 0) ? 0x80000000 : 0x7fffffff;	\
	if (dreg < 8)													\
	{																\
		CLR_NZCVUF(tms);												\
		OR_NZCV_SUB(tms,temps,src2,_res);								\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(tms, dreg);										\
}

static void subb_reg(tms32031_state *tms, UINT32 op)
{
	UINT32 src = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	SUBB(dreg, dst, src);
}

static void subb_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	SUBB(dreg, dst, src);
}

static void subb_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	SUBB(dreg, dst, src);
}

static void subb_imm(tms32031_state *tms, UINT32 op)
{
	UINT32 src = (INT16)op;
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	SUBB(dreg, dst, src);
}

/*-----------------------------------------------------*/

#define SUBC(dreg, src)												\
{																	\
	UINT32 dst = IREG(tms, dreg);										\
	if (dst >= src)													\
		IREG(tms, dreg) = ((dst - src) << 1) | 1;						\
	else															\
		IREG(tms, dreg) = dst << 1;										\
	if (dreg >= TMR_BK)												\
		update_special(tms, dreg);										\
}

static void subc_reg(tms32031_state *tms, UINT32 op)
{
	UINT32 src = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	SUBC(dreg, src);
}

static void subc_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 31;
	SUBC(dreg, src);
}

static void subc_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 31;
	SUBC(dreg, src);
}

static void subc_imm(tms32031_state *tms, UINT32 op)
{
	UINT32 src = (INT16)op;
	int dreg = (op >> 16) & 31;
	SUBC(dreg, src);
}

/*-----------------------------------------------------*/

static void subf_reg(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 7;
	subf(tms, &tms->r[dreg], &tms->r[dreg], &tms->r[op & 7]);
}

static void subf_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 res = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 7;
	LONG2FP(tms, TMR_TEMP1, res);
	subf(tms, &tms->r[dreg], &tms->r[dreg], &tms->r[TMR_TEMP1]);
}

static void subf_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 7;
	LONG2FP(tms, TMR_TEMP1, res);
	subf(tms, &tms->r[dreg], &tms->r[dreg], &tms->r[TMR_TEMP1]);
}

static void subf_imm(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 7;
	SHORT2FP(tms, TMR_TEMP1, op);
	subf(tms, &tms->r[dreg], &tms->r[dreg], &tms->r[TMR_TEMP1]);
}

/*-----------------------------------------------------*/

#define SUBI(dreg, src1, src2)										\
{																	\
	UINT32 _res = src1 - src2;										\
	if (!OVM(tms) || !OVERFLOW_SUB(src1,src2,_res))						\
		IREG(tms, dreg) = _res;											\
	else															\
		IREG(tms, dreg) = ((INT32)src1 < 0) ? 0x80000000 : 0x7fffffff;	\
	if (dreg < 8)													\
	{																\
		CLR_NZCVUF(tms);												\
		OR_NZCV_SUB(tms,src1,src2,_res);								\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(tms, dreg);										\
}

static void subi_reg(tms32031_state *tms, UINT32 op)
{
	UINT32 src = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	SUBI(dreg, dst, src);
}

static void subi_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	SUBI(dreg, dst, src);
}

static void subi_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	SUBI(dreg, dst, src);
}

static void subi_imm(tms32031_state *tms, UINT32 op)
{
	UINT32 src = (INT16)op;
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	SUBI(dreg, dst, src);
}

/*-----------------------------------------------------*/

static void subrb_reg(tms32031_state *tms, UINT32 op)
{
	UINT32 src = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	SUBB(dreg, src, dst);
}

static void subrb_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	SUBB(dreg, src, dst);
}

static void subrb_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	SUBB(dreg, src, dst);
}

static void subrb_imm(tms32031_state *tms, UINT32 op)
{
	UINT32 src = (INT16)op;
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	SUBB(dreg, src, dst);
}

/*-----------------------------------------------------*/

static void subrf_reg(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 7;
	subf(tms, &tms->r[dreg], &tms->r[op & 7], &tms->r[dreg]);
}

static void subrf_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 res = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 7;
	LONG2FP(tms, TMR_TEMP1, res);
	subf(tms, &tms->r[dreg], &tms->r[TMR_TEMP1], &tms->r[dreg]);
}

static void subrf_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 7;
	LONG2FP(tms, TMR_TEMP1, res);
	subf(tms, &tms->r[dreg], &tms->r[TMR_TEMP1], &tms->r[dreg]);
}

static void subrf_imm(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 7;
	SHORT2FP(tms, TMR_TEMP1, op);
	subf(tms, &tms->r[dreg], &tms->r[TMR_TEMP1], &tms->r[dreg]);
}

/*-----------------------------------------------------*/

static void subri_reg(tms32031_state *tms, UINT32 op)
{
	UINT32 src = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	SUBI(dreg, src, dst);
}

static void subri_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	SUBI(dreg, src, dst);
}

static void subri_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	SUBI(dreg, src, dst);
}

static void subri_imm(tms32031_state *tms, UINT32 op)
{
	UINT32 src = (INT16)op;
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	SUBI(dreg, src, dst);
}

/*-----------------------------------------------------*/

#define TSTB(src1, src2)											\
{																	\
	UINT32 _res = (src1) & (src2);									\
	CLR_NZVUF(tms);													\
	OR_NZ(tms, _res);													\
}

static void tstb_reg(tms32031_state *tms, UINT32 op)
{
	UINT32 src = IREG(tms, op & 31);
	UINT32 dst = IREG(tms, (op >> 16) & 31);
	TSTB(dst, src);
}

static void tstb_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, DIRECT(tms, op));
	UINT32 dst = IREG(tms, (op >> 16) & 31);
	TSTB(dst, src);
}

static void tstb_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	UINT32 dst = IREG(tms, (op >> 16) & 31);
	TSTB(dst, src);
}

static void tstb_imm(tms32031_state *tms, UINT32 op)
{
	UINT32 src = (UINT16)op;
	UINT32 dst = IREG(tms, (op >> 16) & 31);
	TSTB(dst, src);
}

/*-----------------------------------------------------*/

#define XOR(dreg, src1, src2)										\
{																	\
	UINT32 _res = (src1) ^ (src2);									\
	IREG(tms, dreg) = _res;												\
	if (dreg < 8)													\
	{																\
		CLR_NZVUF(tms);												\
		OR_NZ(tms, _res);												\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(tms, dreg);										\
}

static void xor_reg(tms32031_state *tms, UINT32 op)
{
	UINT32 src = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	XOR(dreg, dst, src);
}

static void xor_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	XOR(dreg, dst, src);
}

static void xor_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 src = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	XOR(dreg, dst, src);
}

static void xor_imm(tms32031_state *tms, UINT32 op)
{
	UINT32 src = (UINT16)op;
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(tms, dreg);
	XOR(dreg, dst, src);
}

/*-----------------------------------------------------*/

static void iack_dir(tms32031_state *tms, UINT32 op)
{
	offs_t addr = DIRECT(tms, op);
	if (tms->iack_w)
		(*tms->iack_w)(tms->device, ASSERT_LINE, addr);
	RMEM(tms, addr);
	if (tms->iack_w)
		(*tms->iack_w)(tms->device, CLEAR_LINE, addr);
}

static void iack_ind(tms32031_state *tms, UINT32 op)
{
	offs_t addr = INDIRECT_D(tms, op, op >> 8);
	if (tms->iack_w)
		(*tms->iack_w)(tms->device, ASSERT_LINE, addr);
	RMEM(tms, addr);
	if (tms->iack_w)
		(*tms->iack_w)(tms->device, CLEAR_LINE, addr);
}

/*-----------------------------------------------------*/

static void addc3_regreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = IREG(tms, (op >> 8) & 31);
	UINT32 src2 = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	ADDC(dreg, src1, src2);
}

static void addc3_indreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = RMEM(tms, INDIRECT_1(tms, op, op >> 8));
	UINT32 src2 = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	ADDC(dreg, src1, src2);
}

static void addc3_regind(tms32031_state *tms, UINT32 op)
{
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	UINT32 src1 = IREG(tms, (op >> 8) & 31);
	int dreg = (op >> 16) & 31;
	ADDC(dreg, src1, src2);
}

static void addc3_indind(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	int dreg = (op >> 16) & 31;
	UPDATE_DEF();
	ADDC(dreg, src1, src2);
}

/*-----------------------------------------------------*/

static void addf3_regreg(tms32031_state *tms, UINT32 op)
{
	int sreg1 = (op >> 8) & 7;
	int sreg2 = op & 7;
	int dreg = (op >> 16) & 7;
	addf(tms, &tms->r[dreg], &tms->r[sreg1], &tms->r[sreg2]);
}

static void addf3_indreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = RMEM(tms, INDIRECT_1(tms, op, op >> 8));
	int sreg2 = op & 7;
	int dreg = (op >> 16) & 7;
	LONG2FP(tms, TMR_TEMP1, src1);
	addf(tms, &tms->r[dreg], &tms->r[TMR_TEMP1], &tms->r[sreg2]);
}

static void addf3_regind(tms32031_state *tms, UINT32 op)
{
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	int sreg1 = (op >> 8) & 7;
	int dreg = (op >> 16) & 7;
	LONG2FP(tms, TMR_TEMP2, src2);
	addf(tms, &tms->r[dreg], &tms->r[sreg1], &tms->r[TMR_TEMP2]);
}

static void addf3_indind(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	int dreg = (op >> 16) & 7;
	UPDATE_DEF();
	LONG2FP(tms, TMR_TEMP1, src1);
	LONG2FP(tms, TMR_TEMP2, src2);
	addf(tms, &tms->r[dreg], &tms->r[TMR_TEMP1], &tms->r[TMR_TEMP2]);
}

/*-----------------------------------------------------*/

static void addi3_regreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = IREG(tms, (op >> 8) & 31);
	UINT32 src2 = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	ADDI(dreg, src1, src2);
}

static void addi3_indreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = RMEM(tms, INDIRECT_1(tms, op, op >> 8));
	UINT32 src2 = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	ADDI(dreg, src1, src2);
}

static void addi3_regind(tms32031_state *tms, UINT32 op)
{
	/* Radikal Bikers confirms via ADDI3 AR3,*AR3++(1),R2 / SUB $0001,R2 sequence */
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	UINT32 src1 = IREG(tms, (op >> 8) & 31);
	int dreg = (op >> 16) & 31;
	ADDI(dreg, src1, src2);
}

static void addi3_indind(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	int dreg = (op >> 16) & 31;
	UPDATE_DEF();
	ADDI(dreg, src1, src2);
}

/*-----------------------------------------------------*/

static void and3_regreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = IREG(tms, (op >> 8) & 31);
	UINT32 src2 = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	AND(dreg, src1, src2);
}

static void and3_indreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = RMEM(tms, INDIRECT_1(tms, op, op >> 8));
	UINT32 src2 = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	AND(dreg, src1, src2);
}

static void and3_regind(tms32031_state *tms, UINT32 op)
{
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	UINT32 src1 = IREG(tms, (op >> 8) & 31);
	int dreg = (op >> 16) & 31;
	AND(dreg, src1, src2);
}

static void and3_indind(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	int dreg = (op >> 16) & 31;
	UPDATE_DEF();
	AND(dreg, src1, src2);
}

/*-----------------------------------------------------*/

static void andn3_regreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = IREG(tms, (op >> 8) & 31);
	UINT32 src2 = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	ANDN(dreg, src1, src2);
}

static void andn3_indreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = RMEM(tms, INDIRECT_1(tms, op, op >> 8));
	UINT32 src2 = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	ANDN(dreg, src1, src2);
}

static void andn3_regind(tms32031_state *tms, UINT32 op)
{
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	UINT32 src1 = IREG(tms, (op >> 8) & 31);
	int dreg = (op >> 16) & 31;
	ANDN(dreg, src1, src2);
}

static void andn3_indind(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	int dreg = (op >> 16) & 31;
	UPDATE_DEF();
	ANDN(dreg, src1, src2);
}

/*-----------------------------------------------------*/

static void ash3_regreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = IREG(tms, (op >> 8) & 31);
	UINT32 src2 = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	ASH(dreg, src1, src2);
}

static void ash3_indreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = RMEM(tms, INDIRECT_1(tms, op, op >> 8));
	UINT32 src2 = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	ASH(dreg, src1, src2);
}

static void ash3_regind(tms32031_state *tms, UINT32 op)
{
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	UINT32 src1 = IREG(tms, (op >> 8) & 31);
	int dreg = (op >> 16) & 31;
	ASH(dreg, src1, src2);
}

static void ash3_indind(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	int dreg = (op >> 16) & 31;
	UPDATE_DEF();
	ASH(dreg, src1, src2);
}

/*-----------------------------------------------------*/

static void cmpf3_regreg(tms32031_state *tms, UINT32 op)
{
	int sreg1 = (op >> 8) & 7;
	int sreg2 = op & 7;
	subf(tms, &tms->r[TMR_TEMP1], &tms->r[sreg1], &tms->r[sreg2]);
}

static void cmpf3_indreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = RMEM(tms, INDIRECT_1(tms, op, op >> 8));
	int sreg2 = op & 7;
	LONG2FP(tms, TMR_TEMP1, src1);
	subf(tms, &tms->r[TMR_TEMP1], &tms->r[TMR_TEMP1], &tms->r[sreg2]);
}

static void cmpf3_regind(tms32031_state *tms, UINT32 op)
{
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	int sreg1 = (op >> 8) & 7;
	LONG2FP(tms, TMR_TEMP2, src2);
	subf(tms, &tms->r[TMR_TEMP1], &tms->r[sreg1], &tms->r[TMR_TEMP2]);
}

static void cmpf3_indind(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	UPDATE_DEF();
	LONG2FP(tms, TMR_TEMP1, src1);
	LONG2FP(tms, TMR_TEMP2, src2);
	subf(tms, &tms->r[TMR_TEMP1], &tms->r[TMR_TEMP1], &tms->r[TMR_TEMP2]);
}

/*-----------------------------------------------------*/

static void cmpi3_regreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = IREG(tms, (op >> 8) & 31);
	UINT32 src2 = IREG(tms, op & 31);
	CMPI(src1, src2);
}

static void cmpi3_indreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = RMEM(tms, INDIRECT_1(tms, op, op >> 8));
	UINT32 src2 = IREG(tms, op & 31);
	CMPI(src1, src2);
}

static void cmpi3_regind(tms32031_state *tms, UINT32 op)
{
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	UINT32 src1 = IREG(tms, (op >> 8) & 31);
	CMPI(src1, src2);
}

static void cmpi3_indind(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	UPDATE_DEF();
	CMPI(src1, src2);
}

/*-----------------------------------------------------*/

static void lsh3_regreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = IREG(tms, (op >> 8) & 31);
	UINT32 src2 = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	LSH(dreg, src1, src2);
}

static void lsh3_indreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = RMEM(tms, INDIRECT_1(tms, op, op >> 8));
	UINT32 src2 = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	LSH(dreg, src1, src2);
}

static void lsh3_regind(tms32031_state *tms, UINT32 op)
{
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	UINT32 src1 = IREG(tms, (op >> 8) & 31);
	int dreg = (op >> 16) & 31;
	LSH(dreg, src1, src2);
}

static void lsh3_indind(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	int dreg = (op >> 16) & 31;
	UPDATE_DEF();
	LSH(dreg, src1, src2);
}

/*-----------------------------------------------------*/

static void mpyf3_regreg(tms32031_state *tms, UINT32 op)
{
	int sreg1 = (op >> 8) & 7;
	int sreg2 = op & 7;
	int dreg = (op >> 16) & 7;
	mpyf(tms, &tms->r[dreg], &tms->r[sreg1], &tms->r[sreg2]);
}

static void mpyf3_indreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = RMEM(tms, INDIRECT_1(tms, op, op >> 8));
	int sreg2 = op & 7;
	int dreg = (op >> 16) & 7;
	LONG2FP(tms, TMR_TEMP1, src1);
	mpyf(tms, &tms->r[dreg], &tms->r[TMR_TEMP1], &tms->r[sreg2]);
}

static void mpyf3_regind(tms32031_state *tms, UINT32 op)
{
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	int sreg1 = (op >> 8) & 7;
	int dreg = (op >> 16) & 7;
	LONG2FP(tms, TMR_TEMP2, src2);
	mpyf(tms, &tms->r[dreg], &tms->r[sreg1], &tms->r[TMR_TEMP2]);
}

static void mpyf3_indind(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	int dreg = (op >> 16) & 7;
	UPDATE_DEF();
	LONG2FP(tms, TMR_TEMP1, src1);
	LONG2FP(tms, TMR_TEMP2, src2);
	mpyf(tms, &tms->r[dreg], &tms->r[TMR_TEMP1], &tms->r[TMR_TEMP2]);
}

/*-----------------------------------------------------*/

static void mpyi3_regreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = IREG(tms, (op >> 8) & 31);
	UINT32 src2 = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	MPYI(dreg, src1, src2);
}

static void mpyi3_indreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = RMEM(tms, INDIRECT_1(tms, op, op >> 8));
	UINT32 src2 = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	MPYI(dreg, src1, src2);
}

static void mpyi3_regind(tms32031_state *tms, UINT32 op)
{
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	UINT32 src1 = IREG(tms, (op >> 8) & 31);
	int dreg = (op >> 16) & 31;
	MPYI(dreg, src1, src2);
}

static void mpyi3_indind(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	int dreg = (op >> 16) & 31;
	UPDATE_DEF();
	MPYI(dreg, src1, src2);
}

/*-----------------------------------------------------*/

static void or3_regreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = IREG(tms, (op >> 8) & 31);
	UINT32 src2 = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	OR(dreg, src1, src2);
}

static void or3_indreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = RMEM(tms, INDIRECT_1(tms, op, op >> 8));
	UINT32 src2 = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	OR(dreg, src1, src2);
}

static void or3_regind(tms32031_state *tms, UINT32 op)
{
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	UINT32 src1 = IREG(tms, (op >> 8) & 31);
	int dreg = (op >> 16) & 31;
	OR(dreg, src1, src2);
}

static void or3_indind(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	int dreg = (op >> 16) & 31;
	UPDATE_DEF();
	OR(dreg, src1, src2);
}

/*-----------------------------------------------------*/

static void subb3_regreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = IREG(tms, (op >> 8) & 31);
	UINT32 src2 = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	SUBB(dreg, src1, src2);
}

static void subb3_indreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = RMEM(tms, INDIRECT_1(tms, op, op >> 8));
	UINT32 src2 = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	SUBB(dreg, src1, src2);
}

static void subb3_regind(tms32031_state *tms, UINT32 op)
{
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	UINT32 src1 = IREG(tms, (op >> 8) & 31);
	int dreg = (op >> 16) & 31;
	SUBB(dreg, src1, src2);
}

static void subb3_indind(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	int dreg = (op >> 16) & 31;
	UPDATE_DEF();
	SUBB(dreg, src1, src2);
}

/*-----------------------------------------------------*/

static void subf3_regreg(tms32031_state *tms, UINT32 op)
{
	int sreg1 = (op >> 8) & 7;
	int sreg2 = op & 7;
	int dreg = (op >> 16) & 7;
	subf(tms, &tms->r[dreg], &tms->r[sreg1], &tms->r[sreg2]);
}

static void subf3_indreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = RMEM(tms, INDIRECT_1(tms, op, op >> 8));
	int sreg2 = op & 7;
	int dreg = (op >> 16) & 7;
	LONG2FP(tms, TMR_TEMP1, src1);
	subf(tms, &tms->r[dreg], &tms->r[TMR_TEMP1], &tms->r[sreg2]);
}

static void subf3_regind(tms32031_state *tms, UINT32 op)
{
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	int sreg1 = (op >> 8) & 7;
	int dreg = (op >> 16) & 7;
	LONG2FP(tms, TMR_TEMP2, src2);
	subf(tms, &tms->r[dreg], &tms->r[sreg1], &tms->r[TMR_TEMP2]);
}

static void subf3_indind(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	int dreg = (op >> 16) & 7;
	UPDATE_DEF();
	LONG2FP(tms, TMR_TEMP1, src1);
	LONG2FP(tms, TMR_TEMP2, src2);
	subf(tms, &tms->r[dreg], &tms->r[TMR_TEMP1], &tms->r[TMR_TEMP2]);
}

/*-----------------------------------------------------*/

static void subi3_regreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = IREG(tms, (op >> 8) & 31);
	UINT32 src2 = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	SUBI(dreg, src1, src2);
}

static void subi3_indreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = RMEM(tms, INDIRECT_1(tms, op, op >> 8));
	UINT32 src2 = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	SUBI(dreg, src1, src2);
}

static void subi3_regind(tms32031_state *tms, UINT32 op)
{
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	UINT32 src1 = IREG(tms, (op >> 8) & 31);
	int dreg = (op >> 16) & 31;
	SUBI(dreg, src1, src2);
}

static void subi3_indind(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	int dreg = (op >> 16) & 31;
	UPDATE_DEF();
	SUBI(dreg, src1, src2);
}

/*-----------------------------------------------------*/

static void tstb3_regreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = IREG(tms, (op >> 8) & 31);
	UINT32 src2 = IREG(tms, op & 31);
	TSTB(src1, src2);
}

static void tstb3_indreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = RMEM(tms, INDIRECT_1(tms, op, op >> 8));
	UINT32 src2 = IREG(tms, op & 31);
	TSTB(src1, src2);
}

static void tstb3_regind(tms32031_state *tms, UINT32 op)
{
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	UINT32 src1 = IREG(tms, (op >> 8) & 31);
	TSTB(src1, src2);
}

static void tstb3_indind(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	UPDATE_DEF();
	TSTB(src1, src2);
}

/*-----------------------------------------------------*/

static void xor3_regreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = IREG(tms, (op >> 8) & 31);
	UINT32 src2 = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	XOR(dreg, src1, src2);
}

static void xor3_indreg(tms32031_state *tms, UINT32 op)
{
	UINT32 src1 = RMEM(tms, INDIRECT_1(tms, op, op >> 8));
	UINT32 src2 = IREG(tms, op & 31);
	int dreg = (op >> 16) & 31;
	XOR(dreg, src1, src2);
}

static void xor3_regind(tms32031_state *tms, UINT32 op)
{
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	UINT32 src1 = IREG(tms, (op >> 8) & 31);
	int dreg = (op >> 16) & 31;
	XOR(dreg, src1, src2);
}

static void xor3_indind(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src2 = RMEM(tms, INDIRECT_1(tms, op, op));
	int dreg = (op >> 16) & 31;
	UPDATE_DEF();
	XOR(dreg, src1, src2);
}

/*-----------------------------------------------------*/

static void ldfu_reg(tms32031_state *tms, UINT32 op)
{
	tms->r[(op >> 16) & 7] = tms->r[op & 7];
}

static void ldfu_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 res = RMEM(tms, DIRECT(tms, op));
	int dreg = (op >> 16) & 7;
	LONG2FP(tms, dreg, res);
}

static void ldfu_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	int dreg = (op >> 16) & 7;
	LONG2FP(tms, dreg, res);
}

static void ldfu_imm(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 7;
	SHORT2FP(tms, dreg, op);
}

/*-----------------------------------------------------*/

static void ldflo_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LO(tms))
		tms->r[(op >> 16) & 7] = tms->r[op & 7];
}

static void ldflo_dir(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LO(tms))
	{
		UINT32 res = RMEM(tms, DIRECT(tms, op));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
}

static void ldflo_ind(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LO(tms))
	{
		UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
	else
		INDIRECT_D(tms, op, op >> 8);
}

static void ldflo_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LO(tms))
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(tms, dreg, op);
	}
}

/*-----------------------------------------------------*/

static void ldfls_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LS(tms))
		tms->r[(op >> 16) & 7] = tms->r[op & 7];
}

static void ldfls_dir(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LS(tms))
	{
		UINT32 res = RMEM(tms, DIRECT(tms, op));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
}

static void ldfls_ind(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LS(tms))
	{
		UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
	else
		INDIRECT_D(tms, op, op >> 8);
}

static void ldfls_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LS(tms))
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(tms, dreg, op);
	}
}

/*-----------------------------------------------------*/

static void ldfhi_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_HI(tms))
		tms->r[(op >> 16) & 7] = tms->r[op & 7];
}

static void ldfhi_dir(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_HI(tms))
	{
		UINT32 res = RMEM(tms, DIRECT(tms, op));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
}

static void ldfhi_ind(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_HI(tms))
	{
		UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
	else
		INDIRECT_D(tms, op, op >> 8);
}

static void ldfhi_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_HI(tms))
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(tms, dreg, op);
	}
}

/*-----------------------------------------------------*/

static void ldfhs_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_HS(tms))
		tms->r[(op >> 16) & 7] = tms->r[op & 7];
}

static void ldfhs_dir(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_HS(tms))
	{
		UINT32 res = RMEM(tms, DIRECT(tms, op));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
}

static void ldfhs_ind(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_HS(tms))
	{
		UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
	else
		INDIRECT_D(tms, op, op >> 8);
}

static void ldfhs_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_HS(tms))
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(tms, dreg, op);
	}
}

/*-----------------------------------------------------*/

static void ldfeq_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_EQ(tms))
		tms->r[(op >> 16) & 7] = tms->r[op & 7];
}

static void ldfeq_dir(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_EQ(tms))
	{
		UINT32 res = RMEM(tms, DIRECT(tms, op));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
}

static void ldfeq_ind(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_EQ(tms))
	{
		UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
	else
		INDIRECT_D(tms, op, op >> 8);
}

static void ldfeq_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_EQ(tms))
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(tms, dreg, op);
	}
}

/*-----------------------------------------------------*/

static void ldfne_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NE(tms))
		tms->r[(op >> 16) & 7] = tms->r[op & 7];
}

static void ldfne_dir(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NE(tms))
	{
		UINT32 res = RMEM(tms, DIRECT(tms, op));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
}

static void ldfne_ind(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NE(tms))
	{
		UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
	else
		INDIRECT_D(tms, op, op >> 8);
}

static void ldfne_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NE(tms))
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(tms, dreg, op);
	}
}

/*-----------------------------------------------------*/

static void ldflt_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LT(tms))
		tms->r[(op >> 16) & 7] = tms->r[op & 7];
}

static void ldflt_dir(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LT(tms))
	{
		UINT32 res = RMEM(tms, DIRECT(tms, op));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
}

static void ldflt_ind(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LT(tms))
	{
		UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
	else
		INDIRECT_D(tms, op, op >> 8);
}

static void ldflt_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LT(tms))
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(tms, dreg, op);
	}
}

/*-----------------------------------------------------*/

static void ldfle_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LE(tms))
		tms->r[(op >> 16) & 7] = tms->r[op & 7];
}

static void ldfle_dir(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LE(tms))
	{
		UINT32 res = RMEM(tms, DIRECT(tms, op));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
}

static void ldfle_ind(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LE(tms))
	{
		UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
	else
		INDIRECT_D(tms, op, op >> 8);
}

static void ldfle_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LE(tms))
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(tms, dreg, op);
	}
}

/*-----------------------------------------------------*/

static void ldfgt_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_GT(tms))
		tms->r[(op >> 16) & 7] = tms->r[op & 7];
}

static void ldfgt_dir(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_GT(tms))
	{
		UINT32 res = RMEM(tms, DIRECT(tms, op));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
}

static void ldfgt_ind(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_GT(tms))
	{
		UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
	else
		INDIRECT_D(tms, op, op >> 8);
}

static void ldfgt_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_GT(tms))
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(tms, dreg, op);
	}
}

/*-----------------------------------------------------*/

static void ldfge_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_GE(tms))
		tms->r[(op >> 16) & 7] = tms->r[op & 7];
}

static void ldfge_dir(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_GE(tms))
	{
		UINT32 res = RMEM(tms, DIRECT(tms, op));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
}

static void ldfge_ind(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_GE(tms))
	{
		UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
	else
		INDIRECT_D(tms, op, op >> 8);
}

static void ldfge_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_GE(tms))
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(tms, dreg, op);
	}
}

/*-----------------------------------------------------*/

static void ldfnv_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NV(tms))
		tms->r[(op >> 16) & 7] = tms->r[op & 7];
}

static void ldfnv_dir(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NV(tms))
	{
		UINT32 res = RMEM(tms, DIRECT(tms, op));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
}

static void ldfnv_ind(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NV(tms))
	{
		UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
	else
		INDIRECT_D(tms, op, op >> 8);
}

static void ldfnv_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NV(tms))
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(tms, dreg, op);
	}
}

/*-----------------------------------------------------*/

static void ldfv_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_V(tms))
		tms->r[(op >> 16) & 7] = tms->r[op & 7];
}

static void ldfv_dir(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_V(tms))
	{
		UINT32 res = RMEM(tms, DIRECT(tms, op));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
}

static void ldfv_ind(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_V(tms))
	{
		UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
	else
		INDIRECT_D(tms, op, op >> 8);
}

static void ldfv_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_V(tms))
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(tms, dreg, op);
	}
}

/*-----------------------------------------------------*/

static void ldfnuf_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NUF(tms))
		tms->r[(op >> 16) & 7] = tms->r[op & 7];
}

static void ldfnuf_dir(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NUF(tms))
	{
		UINT32 res = RMEM(tms, DIRECT(tms, op));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
}

static void ldfnuf_ind(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NUF(tms))
	{
		UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
	else
		INDIRECT_D(tms, op, op >> 8);
}

static void ldfnuf_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NUF(tms))
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(tms, dreg, op);
	}
}

/*-----------------------------------------------------*/

static void ldfuf_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_UF(tms))
		tms->r[(op >> 16) & 7] = tms->r[op & 7];
}

static void ldfuf_dir(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_UF(tms))
	{
		UINT32 res = RMEM(tms, DIRECT(tms, op));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
}

static void ldfuf_ind(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_UF(tms))
	{
		UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
	else
		INDIRECT_D(tms, op, op >> 8);
}

static void ldfuf_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_UF(tms))
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(tms, dreg, op);
	}
}

/*-----------------------------------------------------*/

static void ldfnlv_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NLV(tms))
		tms->r[(op >> 16) & 7] = tms->r[op & 7];
}

static void ldfnlv_dir(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NLV(tms))
	{
		UINT32 res = RMEM(tms, DIRECT(tms, op));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
}

static void ldfnlv_ind(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NLV(tms))
	{
		UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
	else
		INDIRECT_D(tms, op, op >> 8);
}

static void ldfnlv_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NLV(tms))
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(tms, dreg, op);
	}
}

/*-----------------------------------------------------*/

static void ldflv_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LV(tms))
		tms->r[(op >> 16) & 7] = tms->r[op & 7];
}

static void ldflv_dir(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LV(tms))
	{
		UINT32 res = RMEM(tms, DIRECT(tms, op));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
}

static void ldflv_ind(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LV(tms))
	{
		UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
	else
		INDIRECT_D(tms, op, op >> 8);
}

static void ldflv_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LV(tms))
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(tms, dreg, op);
	}
}

/*-----------------------------------------------------*/

static void ldfnluf_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NLUF(tms))
		tms->r[(op >> 16) & 7] = tms->r[op & 7];
}

static void ldfnluf_dir(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NLUF(tms))
	{
		UINT32 res = RMEM(tms, DIRECT(tms, op));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
}

static void ldfnluf_ind(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NLUF(tms))
	{
		UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
	else
		INDIRECT_D(tms, op, op >> 8);
}

static void ldfnluf_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NLUF(tms))
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(tms, dreg, op);
	}
}

/*-----------------------------------------------------*/

static void ldfluf_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LUF(tms))
		tms->r[(op >> 16) & 7] = tms->r[op & 7];
}

static void ldfluf_dir(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LUF(tms))
	{
		UINT32 res = RMEM(tms, DIRECT(tms, op));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
}

static void ldfluf_ind(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LUF(tms))
	{
		UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
	else
		INDIRECT_D(tms, op, op >> 8);
}

static void ldfluf_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LUF(tms))
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(tms, dreg, op);
	}
}

/*-----------------------------------------------------*/

static void ldfzuf_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_ZUF(tms))
		tms->r[(op >> 16) & 7] = tms->r[op & 7];
}

static void ldfzuf_dir(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_ZUF(tms))
	{
		UINT32 res = RMEM(tms, DIRECT(tms, op));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
}

static void ldfzuf_ind(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_ZUF(tms))
	{
		UINT32 res = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(tms, dreg, res);
	}
	else
		INDIRECT_D(tms, op, op >> 8);
}

static void ldfzuf_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_ZUF(tms))
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(tms, dreg, op);
	}
}

/*-----------------------------------------------------*/

static void ldiu_reg(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 31;
	IREG(tms, dreg) = IREG(tms, op & 31);
	if (dreg >= TMR_BK)
		update_special(tms, dreg);
}

static void ldiu_dir(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 31;
	IREG(tms, dreg) = RMEM(tms, DIRECT(tms, op));
	if (dreg >= TMR_BK)
		update_special(tms, dreg);
}

static void ldiu_ind(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 31;
	IREG(tms, dreg) = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	if (dreg >= TMR_BK)
		update_special(tms, dreg);
}

static void ldiu_imm(tms32031_state *tms, UINT32 op)
{
	int dreg = (op >> 16) & 31;
	IREG(tms, dreg) = (INT16)op;
	if (dreg >= TMR_BK)
		update_special(tms, dreg);
}

/*-----------------------------------------------------*/

static void ldilo_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LO(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = IREG(tms, op & 31);
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldilo_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, DIRECT(tms, op));
	if (CONDITION_LO(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldilo_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	if (CONDITION_LO(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldilo_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LO(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

/*-----------------------------------------------------*/

static void ldils_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LS(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = IREG(tms, op & 31);
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldils_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, DIRECT(tms, op));
	if (CONDITION_LS(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldils_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	if (CONDITION_LS(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldils_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LS(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

/*-----------------------------------------------------*/

static void ldihi_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_HI(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = IREG(tms, op & 31);
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldihi_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, DIRECT(tms, op));
	if (CONDITION_HI(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldihi_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	if (CONDITION_HI(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldihi_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_HI(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

/*-----------------------------------------------------*/

static void ldihs_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_HS(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = IREG(tms, op & 31);
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldihs_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, DIRECT(tms, op));
	if (CONDITION_HS(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldihs_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	if (CONDITION_HS(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldihs_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_HS(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

/*-----------------------------------------------------*/

static void ldieq_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_EQ(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = IREG(tms, op & 31);
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldieq_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, DIRECT(tms, op));
	if (CONDITION_EQ(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldieq_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	if (CONDITION_EQ(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldieq_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_EQ(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

/*-----------------------------------------------------*/

static void ldine_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NE(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = IREG(tms, op & 31);
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldine_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, DIRECT(tms, op));
	if (CONDITION_NE(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldine_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	if (CONDITION_NE(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldine_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NE(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

/*-----------------------------------------------------*/

static void ldilt_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LT(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = IREG(tms, op & 31);
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldilt_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, DIRECT(tms, op));
	if (CONDITION_LT(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldilt_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	if (CONDITION_LT(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldilt_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LT(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

/*-----------------------------------------------------*/

static void ldile_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LE(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = IREG(tms, op & 31);
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldile_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, DIRECT(tms, op));
	if (CONDITION_LE(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldile_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	if (CONDITION_LE(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldile_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LE(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

/*-----------------------------------------------------*/

static void ldigt_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_GT(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = IREG(tms, op & 31);
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldigt_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, DIRECT(tms, op));
	if (CONDITION_GT(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldigt_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	if (CONDITION_GT(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldigt_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_GT(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

/*-----------------------------------------------------*/

static void ldige_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_GE(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = IREG(tms, op & 31);
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldige_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, DIRECT(tms, op));
	if (CONDITION_GE(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldige_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	if (CONDITION_GE(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldige_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_GE(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

/*-----------------------------------------------------*/

static void ldinv_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NV(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = IREG(tms, op & 31);
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldinv_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, DIRECT(tms, op));
	if (CONDITION_NV(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldinv_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	if (CONDITION_NV(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldinv_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NV(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

/*-----------------------------------------------------*/

static void ldiuf_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_UF(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = IREG(tms, op & 31);
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldiuf_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, DIRECT(tms, op));
	if (CONDITION_UF(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldiuf_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	if (CONDITION_UF(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldiuf_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_UF(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

/*-----------------------------------------------------*/

static void ldinuf_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NUF(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = IREG(tms, op & 31);
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldinuf_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, DIRECT(tms, op));
	if (CONDITION_NUF(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldinuf_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	if (CONDITION_NUF(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldinuf_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NUF(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

/*-----------------------------------------------------*/

static void ldiv_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_V(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = IREG(tms, op & 31);
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldiv_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, DIRECT(tms, op));
	if (CONDITION_V(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldiv_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	if (CONDITION_V(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldiv_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_V(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

/*-----------------------------------------------------*/

static void ldinlv_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NLV(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = IREG(tms, op & 31);
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldinlv_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, DIRECT(tms, op));
	if (CONDITION_NLV(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldinlv_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	if (CONDITION_NLV(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldinlv_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NLV(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

/*-----------------------------------------------------*/

static void ldilv_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LV(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = IREG(tms, op & 31);
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldilv_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, DIRECT(tms, op));
	if (CONDITION_LV(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldilv_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	if (CONDITION_LV(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldilv_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LV(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

/*-----------------------------------------------------*/

static void ldinluf_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NLUF(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = IREG(tms, op & 31);
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldinluf_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, DIRECT(tms, op));
	if (CONDITION_NLUF(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldinluf_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	if (CONDITION_NLUF(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldinluf_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_NLUF(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

/*-----------------------------------------------------*/

static void ldiluf_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LUF(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = IREG(tms, op & 31);
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldiluf_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, DIRECT(tms, op));
	if (CONDITION_LUF(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldiluf_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	if (CONDITION_LUF(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldiluf_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_LUF(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

/*-----------------------------------------------------*/

static void ldizuf_reg(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_ZUF(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = IREG(tms, op & 31);
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldizuf_dir(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, DIRECT(tms, op));
	if (CONDITION_ZUF(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldizuf_ind(tms32031_state *tms, UINT32 op)
{
	UINT32 val = RMEM(tms, INDIRECT_D(tms, op, op >> 8));
	if (CONDITION_ZUF(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = val;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

static void ldizuf_imm(tms32031_state *tms, UINT32 op)
{
	if (CONDITION_ZUF(tms))
	{
		int dreg = (op >> 16) & 31;
		IREG(tms, dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(tms, dreg);
	}
}

/*-----------------------------------------------------*/

INLINE void execute_delayed(tms32031_state *tms, UINT32 newpc)
{
	tms->delayed = TRUE;

	if ((tms->device->machine->debug_flags & DEBUG_FLAG_ENABLED) == 0)
	{
		execute_one(tms);
		execute_one(tms);
		execute_one(tms);
	}
	else
	{
		debugger_instruction_hook(tms->device, tms->pc);
		execute_one(tms);
		debugger_instruction_hook(tms->device, tms->pc);
		execute_one(tms);
		debugger_instruction_hook(tms->device, tms->pc);
		execute_one(tms);
	}

	tms->pc = newpc;

	tms->delayed = FALSE;
	if (tms->irq_pending)
	{
		tms->irq_pending = FALSE;
		check_irqs(tms);
	}
}

/*-----------------------------------------------------*/

static void br_imm(tms32031_state *tms, UINT32 op)
{
	tms->pc = op & 0xffffff;
	tms->icount -= 3*2;
}

static void brd_imm(tms32031_state *tms, UINT32 op)
{
	execute_delayed(tms, op & 0xffffff);
}

/*-----------------------------------------------------*/

static void call_imm(tms32031_state *tms, UINT32 op)
{
	WMEM(tms, ++IREG(tms, TMR_SP), tms->pc);
	tms->pc = op & 0xffffff;
	tms->icount -= 3*2;
}

/*-----------------------------------------------------*/

static void rptb_imm(tms32031_state *tms, UINT32 op)
{
	IREG(tms, TMR_RS) = tms->pc;
	IREG(tms, TMR_RE) = op & 0xffffff;
	IREG(tms, TMR_ST) |= RMFLAG;
	tms->icount -= 3*2;
}

/*-----------------------------------------------------*/

static void swi(tms32031_state *tms, UINT32 op) { unimplemented(tms, op); }

/*-----------------------------------------------------*/

static void brc_reg(tms32031_state *tms, UINT32 op)
{
	if (condition(tms, op >> 16))
	{
		tms->pc = IREG(tms, op & 31);
		tms->icount -= 3*2;
	}
}

static void brcd_reg(tms32031_state *tms, UINT32 op)
{
	if (condition(tms, op >> 16))
		execute_delayed(tms, IREG(tms, op & 31));
}

static void brc_imm(tms32031_state *tms, UINT32 op)
{
	if (condition(tms, op >> 16))
	{
		tms->pc += (INT16)op;
		tms->icount -= 3*2;
	}
}

static void brcd_imm(tms32031_state *tms, UINT32 op)
{
	if (condition(tms, op >> 16))
		execute_delayed(tms, tms->pc + 2 + (INT16)op);
}

/*-----------------------------------------------------*/

static void dbc_reg(tms32031_state *tms, UINT32 op)
{
	int reg = TMR_AR0 + ((op >> 22) & 7);
	int res = (IREG(tms, reg) - 1) & 0xffffff;
	IREG(tms, reg) = res | (IREG(tms, reg) & 0xff000000);
	if (condition(tms, op >> 16) && !(res & 0x800000))
	{
		tms->pc = IREG(tms, op & 31);
		tms->icount -= 3*2;
	}
}

static void dbcd_reg(tms32031_state *tms, UINT32 op)
{
	int reg = TMR_AR0 + ((op >> 22) & 7);
	int res = (IREG(tms, reg) - 1) & 0xffffff;
	IREG(tms, reg) = res | (IREG(tms, reg) & 0xff000000);
	if (condition(tms, op >> 16) && !(res & 0x800000))
		execute_delayed(tms, IREG(tms, op & 31));
}

static void dbc_imm(tms32031_state *tms, UINT32 op)
{
	int reg = TMR_AR0 + ((op >> 22) & 7);
	int res = (IREG(tms, reg) - 1) & 0xffffff;
	IREG(tms, reg) = res | (IREG(tms, reg) & 0xff000000);
	if (condition(tms, op >> 16) && !(res & 0x800000))
	{
		tms->pc += (INT16)op;
		tms->icount -= 3*2;
	}
}

static void dbcd_imm(tms32031_state *tms, UINT32 op)
{
	int reg = TMR_AR0 + ((op >> 22) & 7);
	int res = (IREG(tms, reg) - 1) & 0xffffff;
	IREG(tms, reg) = res | (IREG(tms, reg) & 0xff000000);
	if (condition(tms, op >> 16) && !(res & 0x800000))
		execute_delayed(tms, tms->pc + 2 + (INT16)op);
}

/*-----------------------------------------------------*/

static void callc_reg(tms32031_state *tms, UINT32 op)
{
	if (condition(tms, op >> 16))
	{
		WMEM(tms, ++IREG(tms, TMR_SP), tms->pc);
		tms->pc = IREG(tms, op & 31);
		tms->icount -= 3*2;
	}
}

static void callc_imm(tms32031_state *tms, UINT32 op)
{
	if (condition(tms, op >> 16))
	{
		WMEM(tms, ++IREG(tms, TMR_SP), tms->pc);
		tms->pc += (INT16)op;
		tms->icount -= 3*2;
	}
}

/*-----------------------------------------------------*/

static void trap(tms32031_state *tms, int trapnum)
{
	WMEM(tms, ++IREG(tms, TMR_SP), tms->pc);
	IREG(tms, TMR_ST) &= ~GIEFLAG;
	if (tms->is_32032)
		tms->pc = RMEM(tms, ((IREG(tms, TMR_IF) >> 16) << 8) + trapnum);
	else if (tms->mcu_mode)
		tms->pc = 0x809fc0 + trapnum;
	else
		tms->pc = RMEM(tms, trapnum);
	tms->icount -= 4*2;
}

static void trapc(tms32031_state *tms, UINT32 op)
{
	if (condition(tms, op >> 16))
		trap(tms, op & 0x3f);
}

/*-----------------------------------------------------*/

static void retic_reg(tms32031_state *tms, UINT32 op)
{
	if (condition(tms, op >> 16))
	{
		tms->pc = RMEM(tms, IREG(tms, TMR_SP)--);
		IREG(tms, TMR_ST) |= GIEFLAG;
		tms->icount -= 3*2;
		check_irqs(tms);
	}
}

static void retsc_reg(tms32031_state *tms, UINT32 op)
{
	if (condition(tms, op >> 16))
	{
		tms->pc = RMEM(tms, IREG(tms, TMR_SP)--);
		tms->icount -= 3*2;
	}
}

/*-----------------------------------------------------*/

static void mpyaddf_0(tms32031_state *tms, UINT32 op)
{
	/* src3 * src4, src1 + src2 */
	DECLARE_DEF;
	UINT32 src3 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src4 = RMEM(tms, INDIRECT_1(tms, op, op));
	LONG2FP(tms, TMR_TEMP1, src3);
	LONG2FP(tms, TMR_TEMP2, src4);
	mpyf(tms, &tms->r[TMR_TEMP3], &tms->r[TMR_TEMP1], &tms->r[TMR_TEMP2]);
	addf(tms, &tms->r[((op >> 22) & 1) | 2], &tms->r[(op >> 19) & 7], &tms->r[(op >> 16) & 7]);
	tms->r[(op >> 23) & 1] = tms->r[TMR_TEMP3];
	UPDATE_DEF();
}

static void mpyaddf_1(tms32031_state *tms, UINT32 op)
{
	/* src3 * src1, src4 + src2 */
	DECLARE_DEF;
	UINT32 src3 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src4 = RMEM(tms, INDIRECT_1(tms, op, op));
	LONG2FP(tms, TMR_TEMP1, src3);
	LONG2FP(tms, TMR_TEMP2, src4);
	mpyf(tms, &tms->r[TMR_TEMP3], &tms->r[TMR_TEMP1], &tms->r[(op >> 19) & 7]);
	addf(tms, &tms->r[((op >> 22) & 1) | 2], &tms->r[TMR_TEMP2], &tms->r[(op >> 16) & 7]);
	tms->r[(op >> 23) & 1] = tms->r[TMR_TEMP3];
	UPDATE_DEF();
}

static void mpyaddf_2(tms32031_state *tms, UINT32 op)
{
	/* src1 * src2, src3 + src4 */
	DECLARE_DEF;
	UINT32 src3 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src4 = RMEM(tms, INDIRECT_1(tms, op, op));
	LONG2FP(tms, TMR_TEMP1, src3);
	LONG2FP(tms, TMR_TEMP2, src4);
	mpyf(tms, &tms->r[TMR_TEMP3], &tms->r[(op >> 19) & 7], &tms->r[(op >> 16) & 7]);
	addf(tms, &tms->r[((op >> 22) & 1) | 2], &tms->r[TMR_TEMP1], &tms->r[TMR_TEMP2]);
	tms->r[(op >> 23) & 1] = tms->r[TMR_TEMP3];
	UPDATE_DEF();
}

static void mpyaddf_3(tms32031_state *tms, UINT32 op)
{
	/* src3 * src1, src2 + src4 */
	DECLARE_DEF;
	UINT32 src3 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src4 = RMEM(tms, INDIRECT_1(tms, op, op));
	LONG2FP(tms, TMR_TEMP1, src3);
	LONG2FP(tms, TMR_TEMP2, src4);
	mpyf(tms, &tms->r[TMR_TEMP3], &tms->r[TMR_TEMP1], &tms->r[(op >> 19) & 7]);
	addf(tms, &tms->r[((op >> 22) & 1) | 2], &tms->r[(op >> 16) & 7], &tms->r[TMR_TEMP2]);
	tms->r[(op >> 23) & 1] = tms->r[TMR_TEMP3];
	UPDATE_DEF();
}

/*-----------------------------------------------------*/

static void mpysubf_0(tms32031_state *tms, UINT32 op)
{
	/* src3 * src4, src1 - src2 */
	DECLARE_DEF;
	UINT32 src3 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src4 = RMEM(tms, INDIRECT_1(tms, op, op));
	LONG2FP(tms, TMR_TEMP1, src3);
	LONG2FP(tms, TMR_TEMP2, src4);
	mpyf(tms, &tms->r[TMR_TEMP3], &tms->r[TMR_TEMP1], &tms->r[TMR_TEMP2]);
	subf(tms, &tms->r[((op >> 22) & 1) | 2], &tms->r[(op >> 19) & 7], &tms->r[(op >> 16) & 7]);
	tms->r[(op >> 23) & 1] = tms->r[TMR_TEMP3];
	UPDATE_DEF();
}

static void mpysubf_1(tms32031_state *tms, UINT32 op)
{
	/* src3 * src1, src4 - src2 */
	DECLARE_DEF;
	UINT32 src3 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src4 = RMEM(tms, INDIRECT_1(tms, op, op));
	LONG2FP(tms, TMR_TEMP1, src3);
	LONG2FP(tms, TMR_TEMP2, src4);
	mpyf(tms, &tms->r[TMR_TEMP3], &tms->r[TMR_TEMP1], &tms->r[(op >> 19) & 7]);
	subf(tms, &tms->r[((op >> 22) & 1) | 2], &tms->r[TMR_TEMP2], &tms->r[(op >> 16) & 7]);
	tms->r[(op >> 23) & 1] = tms->r[TMR_TEMP3];
	UPDATE_DEF();
}

static void mpysubf_2(tms32031_state *tms, UINT32 op)
{
	/* src1 * src2, src3 - src4 */
	DECLARE_DEF;
	UINT32 src3 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src4 = RMEM(tms, INDIRECT_1(tms, op, op));
	LONG2FP(tms, TMR_TEMP1, src3);
	LONG2FP(tms, TMR_TEMP2, src4);
	mpyf(tms, &tms->r[TMR_TEMP3], &tms->r[(op >> 19) & 7], &tms->r[(op >> 16) & 7]);
	subf(tms, &tms->r[((op >> 22) & 1) | 2], &tms->r[TMR_TEMP1], &tms->r[TMR_TEMP2]);
	tms->r[(op >> 23) & 1] = tms->r[TMR_TEMP3];
	UPDATE_DEF();
}

static void mpysubf_3(tms32031_state *tms, UINT32 op)
{
	/* src3 * src1, src2 - src4 */
	DECLARE_DEF;
	UINT32 src3 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src4 = RMEM(tms, INDIRECT_1(tms, op, op));
	LONG2FP(tms, TMR_TEMP1, src3);
	LONG2FP(tms, TMR_TEMP2, src4);
	mpyf(tms, &tms->r[TMR_TEMP3], &tms->r[TMR_TEMP1], &tms->r[(op >> 19) & 7]);
	subf(tms, &tms->r[((op >> 22) & 1) | 2], &tms->r[(op >> 16) & 7], &tms->r[TMR_TEMP2]);
	tms->r[(op >> 23) & 1] = tms->r[TMR_TEMP3];
	UPDATE_DEF();
}

/*-----------------------------------------------------*/

static void mpyaddi_0(tms32031_state *tms, UINT32 op)
{
	/* src3 * src4, src1 + src2 */
	DECLARE_DEF;
	UINT32 src1 = IREG(tms, (op >> 19) & 7);
	UINT32 src2 = IREG(tms, (op >> 16) & 7);
	UINT32 src3 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src4 = RMEM(tms, INDIRECT_1(tms, op, op));
	INT64 mres = (INT64)((INT32)(src3 << 8) >> 8) * (INT64)((INT32)(src4 << 8) >> 8);
	UINT32 ares = src1 + src2;

	CLR_NZVUF(tms);
	if (OVM(tms))
	{
		if (mres < -(INT64)0x80000000 || mres > (INT64)0x7fffffff)
			mres = (mres < 0) ? 0x80000000 : 0x7fffffff;
		if (OVERFLOW_ADD(src1,src2,ares))
			ares = ((INT32)src1 < 0) ? 0x80000000 : 0x7fffffff;
	}
	IREG(tms, (op >> 23) & 1) = mres;
	IREG(tms, ((op >> 22) & 1) | 2) = ares;
	UPDATE_DEF();
}

static void mpyaddi_1(tms32031_state *tms, UINT32 op)
{
	/* src3 * src1, src4 + src2 */
	DECLARE_DEF;
	UINT32 src1 = IREG(tms, (op >> 19) & 7);
	UINT32 src2 = IREG(tms, (op >> 16) & 7);
	UINT32 src3 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src4 = RMEM(tms, INDIRECT_1(tms, op, op));
	INT64 mres = (INT64)((INT32)(src3 << 8) >> 8) * (INT64)((INT32)(src1 << 8) >> 8);
	UINT32 ares = src4 + src2;

	CLR_NZVUF(tms);
	if (OVM(tms))
	{
		if (mres < -(INT64)0x80000000 || mres > (INT64)0x7fffffff)
			mres = (mres < 0) ? 0x80000000 : 0x7fffffff;
		if (OVERFLOW_ADD(src4,src2,ares))
			ares = ((INT32)src4 < 0) ? 0x80000000 : 0x7fffffff;
	}
	IREG(tms, (op >> 23) & 1) = mres;
	IREG(tms, ((op >> 22) & 1) | 2) = ares;
	UPDATE_DEF();
}

static void mpyaddi_2(tms32031_state *tms, UINT32 op)
{
	/* src1 * src2, src3 + src4 */
	DECLARE_DEF;
	UINT32 src1 = IREG(tms, (op >> 19) & 7);
	UINT32 src2 = IREG(tms, (op >> 16) & 7);
	UINT32 src3 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src4 = RMEM(tms, INDIRECT_1(tms, op, op));
	INT64 mres = (INT64)((INT32)(src1 << 8) >> 8) * (INT64)((INT32)(src2 << 8) >> 8);
	UINT32 ares = src3 + src4;

	CLR_NZVUF(tms);
	if (OVM(tms))
	{
		if (mres < -(INT64)0x80000000 || mres > (INT64)0x7fffffff)
			mres = (mres < 0) ? 0x80000000 : 0x7fffffff;
		if (OVERFLOW_ADD(src3,src4,ares))
			ares = ((INT32)src3 < 0) ? 0x80000000 : 0x7fffffff;
	}
	IREG(tms, (op >> 23) & 1) = mres;
	IREG(tms, ((op >> 22) & 1) | 2) = ares;
	UPDATE_DEF();
}

static void mpyaddi_3(tms32031_state *tms, UINT32 op)
{
	/* src3 * src1, src2 + src4 */
	DECLARE_DEF;
	UINT32 src1 = IREG(tms, (op >> 19) & 7);
	UINT32 src2 = IREG(tms, (op >> 16) & 7);
	UINT32 src3 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src4 = RMEM(tms, INDIRECT_1(tms, op, op));
	INT64 mres = (INT64)((INT32)(src3 << 8) >> 8) * (INT64)((INT32)(src1 << 8) >> 8);
	UINT32 ares = src2 + src4;

	CLR_NZVUF(tms);
	if (OVM(tms))
	{
		if (mres < -(INT64)0x80000000 || mres > (INT64)0x7fffffff)
			mres = (mres < 0) ? 0x80000000 : 0x7fffffff;
		if (OVERFLOW_ADD(src2,src4,ares))
			ares = ((INT32)src2 < 0) ? 0x80000000 : 0x7fffffff;
	}
	IREG(tms, (op >> 23) & 1) = mres;
	IREG(tms, ((op >> 22) & 1) | 2) = ares;
	UPDATE_DEF();
}

/*-----------------------------------------------------*/

static void mpysubi_0(tms32031_state *tms, UINT32 op)
{
	/* src3 * src4, src1 - src2 */
	DECLARE_DEF;
	UINT32 src1 = IREG(tms, (op >> 19) & 7);
	UINT32 src2 = IREG(tms, (op >> 16) & 7);
	UINT32 src3 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src4 = RMEM(tms, INDIRECT_1(tms, op, op));
	INT64 mres = (INT64)((INT32)(src3 << 8) >> 8) * (INT64)((INT32)(src4 << 8) >> 8);
	UINT32 ares = src1 - src2;

	CLR_NZVUF(tms);
	if (OVM(tms))
	{
		if (mres < -(INT64)0x80000000 || mres > (INT64)0x7fffffff)
			mres = (mres < 0) ? 0x80000000 : 0x7fffffff;
		if (OVERFLOW_SUB(src1,src2,ares))
			ares = ((INT32)src1 < 0) ? 0x80000000 : 0x7fffffff;
	}
	IREG(tms, (op >> 23) & 1) = mres;
	IREG(tms, ((op >> 22) & 1) | 2) = ares;
	UPDATE_DEF();
}

static void mpysubi_1(tms32031_state *tms, UINT32 op)
{
	/* src3 * src1, src4 - src2 */
	DECLARE_DEF;
	UINT32 src1 = IREG(tms, (op >> 19) & 7);
	UINT32 src2 = IREG(tms, (op >> 16) & 7);
	UINT32 src3 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src4 = RMEM(tms, INDIRECT_1(tms, op, op));
	INT64 mres = (INT64)((INT32)(src3 << 8) >> 8) * (INT64)((INT32)(src1 << 8) >> 8);
	UINT32 ares = src4 - src2;

	CLR_NZVUF(tms);
	if (OVM(tms))
	{
		if (mres < -(INT64)0x80000000 || mres > (INT64)0x7fffffff)
			mres = (mres < 0) ? 0x80000000 : 0x7fffffff;
		if (OVERFLOW_SUB(src4,src2,ares))
			ares = ((INT32)src4 < 0) ? 0x80000000 : 0x7fffffff;
	}
	IREG(tms, (op >> 23) & 1) = mres;
	IREG(tms, ((op >> 22) & 1) | 2) = ares;
	UPDATE_DEF();
}

static void mpysubi_2(tms32031_state *tms, UINT32 op)
{
	/* src1 * src2, src3 - src4 */
	DECLARE_DEF;
	UINT32 src1 = IREG(tms, (op >> 19) & 7);
	UINT32 src2 = IREG(tms, (op >> 16) & 7);
	UINT32 src3 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src4 = RMEM(tms, INDIRECT_1(tms, op, op));
	INT64 mres = (INT64)((INT32)(src1 << 8) >> 8) * (INT64)((INT32)(src2 << 8) >> 8);
	UINT32 ares = src3 - src4;

	CLR_NZVUF(tms);
	if (OVM(tms))
	{
		if (mres < -(INT64)0x80000000 || mres > (INT64)0x7fffffff)
			mres = (mres < 0) ? 0x80000000 : 0x7fffffff;
		if (OVERFLOW_SUB(src3,src4,ares))
			ares = ((INT32)src3 < 0) ? 0x80000000 : 0x7fffffff;
	}
	IREG(tms, (op >> 23) & 1) = mres;
	IREG(tms, ((op >> 22) & 1) | 2) = ares;
	UPDATE_DEF();
}

static void mpysubi_3(tms32031_state *tms, UINT32 op)
{
	/* src3 * src1, src2 - src4 */
	DECLARE_DEF;
	UINT32 src1 = IREG(tms, (op >> 19) & 7);
	UINT32 src2 = IREG(tms, (op >> 16) & 7);
	UINT32 src3 = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	UINT32 src4 = RMEM(tms, INDIRECT_1(tms, op, op));
	INT64 mres = (INT64)((INT32)(src3 << 8) >> 8) * (INT64)((INT32)(src1 << 8) >> 8);
	UINT32 ares = src2 - src4;

	CLR_NZVUF(tms);
	if (OVM(tms))
	{
		if (mres < -(INT64)0x80000000 || mres > (INT64)0x7fffffff)
			mres = (mres < 0) ? 0x80000000 : 0x7fffffff;
		if (OVERFLOW_SUB(src2,src4,ares))
			ares = ((INT32)src2 < 0) ? 0x80000000 : 0x7fffffff;
	}
	IREG(tms, (op >> 23) & 1) = mres;
	IREG(tms, ((op >> 22) & 1) | 2) = ares;
	UPDATE_DEF();
}

/*-----------------------------------------------------*/

static void stfstf(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	WMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8), FP2LONG(tms, (op >> 16) & 7));
	WMEM(tms, INDIRECT_1(tms, op, op), FP2LONG(tms, (op >> 22) & 7));
	UPDATE_DEF();
}

static void stisti(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	WMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8), IREG(tms, (op >> 16) & 7));
	WMEM(tms, INDIRECT_1(tms, op, op), IREG(tms, (op >> 22) & 7));
	UPDATE_DEF();
}

/*-----------------------------------------------------*/

static void ldfldf(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 res;
	int dreg;

	res = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	dreg = (op >> 19) & 7;
	LONG2FP(tms, dreg, res);
	res = RMEM(tms, INDIRECT_1(tms, op, op));
	dreg = (op >> 22) & 7;
	LONG2FP(tms, dreg, res);
	UPDATE_DEF();
}

static void ldildi(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	IREG(tms, (op >> 19) & 7) = RMEM(tms, INDIRECT_1_DEF(tms, op, op >> 8));
	IREG(tms, (op >> 22) & 7) = RMEM(tms, INDIRECT_1(tms, op, op));
	UPDATE_DEF();
}

/*-----------------------------------------------------*/

//  src2 = ind(op)
//  dst2 = ind(op >> 8)
//  sreg3 = ((op >> 16) & 7)
//  sreg1 = ((op >> 19) & 7)
//  dreg1 = ((op >> 22) & 7)

static void absfstf(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = FP2LONG(tms, (op >> 16) & 7);
	UINT32 src2 = RMEM(tms, INDIRECT_1_DEF(tms, op, op));
	{
		int dreg = (op >> 22) & 7;
		LONG2FP(tms, TMR_TEMP1, src2);
		ABSF(dreg, TMR_TEMP1);
	}
	WMEM(tms, INDIRECT_1(tms, op, op >> 8), src3);
	UPDATE_DEF();
}

static void absisti(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = IREG(tms, (op >> 16) & 7);
	UINT32 src2 = RMEM(tms, INDIRECT_1_DEF(tms, op, op));
	{
		int dreg = (op >> 22) & 7;
		ABSI(dreg, src2);
	}
	WMEM(tms, INDIRECT_1(tms, op, op >> 8), src3);
	UPDATE_DEF();
}

static void addf3stf(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = FP2LONG(tms, (op >> 16) & 7);
	UINT32 src2 = RMEM(tms, INDIRECT_1_DEF(tms, op, op));
	{
		LONG2FP(tms, TMR_TEMP1, src2);
		addf(tms, &tms->r[(op >> 22) & 7], &tms->r[(op >> 19) & 7], &tms->r[TMR_TEMP1]);
	}
	WMEM(tms, INDIRECT_1(tms, op, op >> 8), src3);
	UPDATE_DEF();
}

static void addi3sti(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = IREG(tms, (op >> 16) & 7);
	UINT32 src2 = RMEM(tms, INDIRECT_1_DEF(tms, op, op));
	{
		int dreg = (op >> 22) & 7;
		UINT32 src1 = IREG(tms, (op >> 19) & 7);
		ADDI(dreg, src1, src2);
	}
	WMEM(tms, INDIRECT_1(tms, op, op >> 8), src3);
	UPDATE_DEF();
}

static void and3sti(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = IREG(tms, (op >> 16) & 7);
	UINT32 src2 = RMEM(tms, INDIRECT_1_DEF(tms, op, op));
	{
		int dreg = (op >> 22) & 7;
		UINT32 src1 = IREG(tms, (op >> 19) & 7);
		AND(dreg, src1, src2);
	}
	WMEM(tms, INDIRECT_1(tms, op, op >> 8), src3);
	UPDATE_DEF();
}

static void ash3sti(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = IREG(tms, (op >> 16) & 7);
	UINT32 src2 = RMEM(tms, INDIRECT_1_DEF(tms, op, op));
	{
		int dreg = (op >> 22) & 7;
		UINT32 count = IREG(tms, (op >> 19) & 7);
		ASH(dreg, src2, count);
	}
	WMEM(tms, INDIRECT_1(tms, op, op >> 8), src3);
	UPDATE_DEF();
}

static void fixsti(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = IREG(tms, (op >> 16) & 7);
	UINT32 src2 = RMEM(tms, INDIRECT_1_DEF(tms, op, op));
	{
		int dreg = (op >> 22) & 7;
		LONG2FP(tms, dreg, src2);
		float2int(tms, &tms->r[dreg], 1);
	}
	WMEM(tms, INDIRECT_1(tms, op, op >> 8), src3);
	UPDATE_DEF();
}

static void floatstf(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = FP2LONG(tms, (op >> 16) & 7);
	UINT32 src2 = RMEM(tms, INDIRECT_1_DEF(tms, op, op));
	{
		int dreg = (op >> 22) & 7;
		IREG(tms, dreg) = src2;
		int2float(tms, &tms->r[dreg]);
	}
	WMEM(tms, INDIRECT_1(tms, op, op >> 8), src3);
	UPDATE_DEF();
}

static void ldfstf(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = FP2LONG(tms, (op >> 16) & 7);
	UINT32 src2 = RMEM(tms, INDIRECT_1_DEF(tms, op, op));
	{
		int dreg = (op >> 22) & 7;
		LONG2FP(tms, dreg, src2);
	}
	WMEM(tms, INDIRECT_1(tms, op, op >> 8), src3);
	UPDATE_DEF();
}

static void ldisti(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = IREG(tms, (op >> 16) & 7);
	UINT32 src2 = RMEM(tms, INDIRECT_1_DEF(tms, op, op));
	IREG(tms, (op >> 22) & 7) = src2;
	WMEM(tms, INDIRECT_1(tms, op, op >> 8), src3);
	UPDATE_DEF();
}

static void lsh3sti(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = IREG(tms, (op >> 16) & 7);
	UINT32 src2 = RMEM(tms, INDIRECT_1_DEF(tms, op, op));
	{
		int dreg = (op >> 22) & 7;
		UINT32 count = IREG(tms, (op >> 19) & 7);
		LSH(dreg, src2, count);
	}
	WMEM(tms, INDIRECT_1(tms, op, op >> 8), src3);
	UPDATE_DEF();
}

static void mpyf3stf(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = FP2LONG(tms, (op >> 16) & 7);
	UINT32 src2 = RMEM(tms, INDIRECT_1_DEF(tms, op, op));
	{
		LONG2FP(tms, TMR_TEMP1, src2);
		mpyf(tms, &tms->r[(op >> 22) & 7], &tms->r[(op >> 19) & 7], &tms->r[TMR_TEMP1]);
	}
	WMEM(tms, INDIRECT_1(tms, op, op >> 8), src3);
	UPDATE_DEF();
}

static void mpyi3sti(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = IREG(tms, (op >> 16) & 7);
	UINT32 src2 = RMEM(tms, INDIRECT_1_DEF(tms, op, op));
	{
		int dreg = (op >> 22) & 7;
		UINT32 src1 = IREG(tms, (op >> 19) & 7);
		MPYI(dreg, src1, src2);
	}
	WMEM(tms, INDIRECT_1(tms, op, op >> 8), src3);
	UPDATE_DEF();
}

static void negfstf(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = FP2LONG(tms, (op >> 16) & 7);
	UINT32 src2 = RMEM(tms, INDIRECT_1_DEF(tms, op, op));
	{
		LONG2FP(tms, TMR_TEMP1, src2);
		negf(tms, &tms->r[(op >> 22) & 7], &tms->r[TMR_TEMP1]);
	}
	WMEM(tms, INDIRECT_1(tms, op, op >> 8), src3);
	UPDATE_DEF();
}

static void negisti(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = IREG(tms, (op >> 16) & 7);
	UINT32 src2 = RMEM(tms, INDIRECT_1_DEF(tms, op, op));
	{
		int dreg = (op >> 22) & 7;
		NEGI(dreg, src2);
	}
	WMEM(tms, INDIRECT_1(tms, op, op >> 8), src3);
	UPDATE_DEF();
}

static void notsti(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = IREG(tms, (op >> 16) & 7);
	UINT32 src2 = RMEM(tms, INDIRECT_1_DEF(tms, op, op));
	{
		int dreg = (op >> 22) & 7;
		NOT(dreg, src2);
	}
	WMEM(tms, INDIRECT_1(tms, op, op >> 8), src3);
	UPDATE_DEF();
}

static void or3sti(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = IREG(tms, (op >> 16) & 7);
	UINT32 src2 = RMEM(tms, INDIRECT_1_DEF(tms, op, op));
	{
		int dreg = (op >> 22) & 7;
		UINT32 src1 = IREG(tms, (op >> 19) & 7);
		OR(dreg, src1, src2);
	}
	WMEM(tms, INDIRECT_1(tms, op, op >> 8), src3);
	UPDATE_DEF();
}

static void subf3stf(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = FP2LONG(tms, (op >> 16) & 7);
	UINT32 src2 = RMEM(tms, INDIRECT_1_DEF(tms, op, op));
	{
		LONG2FP(tms, TMR_TEMP1, src2);
		subf(tms, &tms->r[(op >> 22) & 7], &tms->r[TMR_TEMP1], &tms->r[(op >> 19) & 7]);
	}
	WMEM(tms, INDIRECT_1(tms, op, op >> 8), src3);
	UPDATE_DEF();
}

static void subi3sti(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = IREG(tms, (op >> 16) & 7);
	UINT32 src2 = RMEM(tms, INDIRECT_1_DEF(tms, op, op));
	{
		int dreg = (op >> 22) & 7;
		UINT32 src1 = IREG(tms, (op >> 19) & 7);
		SUBI(dreg, src2, src1);
	}
	WMEM(tms, INDIRECT_1(tms, op, op >> 8), src3);
	UPDATE_DEF();
}

static void xor3sti(tms32031_state *tms, UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = IREG(tms, (op >> 16) & 7);
	UINT32 src2 = RMEM(tms, INDIRECT_1_DEF(tms, op, op));
	{
		int dreg = (op >> 22) & 7;
		UINT32 src1 = IREG(tms, (op >> 19) & 7);
		XOR(dreg, src1, src2);
	}
	WMEM(tms, INDIRECT_1(tms, op, op >> 8), src3);
	UPDATE_DEF();
}


/***************************************************************************
    FUNCTION TABLE
***************************************************************************/

void (*const tms32031ops[])(tms32031_state *tms, UINT32 op) =
{
	absf_reg,		absf_dir,		absf_ind,		absf_imm,		/* 0x00 */
	absi_reg,		absi_dir,		absi_ind,		absi_imm,
	addc_reg,		addc_dir,		addc_ind,		addc_imm,
	addf_reg,		addf_dir,		addf_ind,		addf_imm,
	addi_reg,		addi_dir,		addi_ind,		addi_imm,
	and_reg,		and_dir,		and_ind,		and_imm,
	andn_reg,		andn_dir,		andn_ind,		andn_imm,
	ash_reg,		ash_dir,		ash_ind,		ash_imm,
	cmpf_reg,		cmpf_dir,		cmpf_ind,		cmpf_imm,		/* 0x08 */
	cmpi_reg,		cmpi_dir,		cmpi_ind,		cmpi_imm,
	fix_reg,		fix_dir,		fix_ind,		fix_imm,
	float_reg,		float_dir,		float_ind,		float_imm,
	idle,			idle,			idle,			idle,
	lde_reg,		lde_dir,		lde_ind,		lde_imm,
	ldf_reg,		ldf_dir,		ldf_ind,		ldf_imm,
	illegal,		ldfi_dir,		ldfi_ind,		illegal,
	ldi_reg,		ldi_dir,		ldi_ind,		ldi_imm,		/* 0x10 */
	illegal,		ldii_dir,		ldii_ind,		illegal,
	ldm_reg,		ldm_dir,		ldm_ind,		ldm_imm,
	lsh_reg,		lsh_dir,		lsh_ind,		lsh_imm,
	mpyf_reg,		mpyf_dir,		mpyf_ind,		mpyf_imm,
	mpyi_reg,		mpyi_dir,		mpyi_ind,		mpyi_imm,
	negb_reg,		negb_dir,		negb_ind,		negb_imm,
	negf_reg,		negf_dir,		negf_ind,		negf_imm,
	negi_reg,		negi_dir,		negi_ind,		negi_imm,		/* 0x18 */
	nop_reg,		illegal,		nop_ind,		illegal,
	norm_reg,		norm_dir,		norm_ind,		norm_imm,
	not_reg,		not_dir,		not_ind,		not_imm,
	illegal,		pop,			illegal,		illegal,
	illegal,		popf,			illegal,		illegal,
	illegal,		push,			illegal,		illegal,
	illegal,		pushf,			illegal,		illegal,
	or_reg,			or_dir,			or_ind,			or_imm,			/* 0x20 */
	maxspeed,		maxspeed,		maxspeed,		maxspeed,
	rnd_reg,		rnd_dir,		rnd_ind,		rnd_imm,
	illegal,		illegal,		illegal,		rol,
	illegal,		illegal,		illegal,		rolc,
	illegal,		illegal,		illegal,		ror,
	illegal,		illegal,		illegal,		rorc,
	rtps_reg,		rtps_dir,		rtps_ind,		rtps_imm,
	illegal,		stf_dir,		stf_ind,		illegal,		/* 0x28 */
	illegal,		stfi_dir,		stfi_ind,		illegal,
	illegal,		sti_dir,		sti_ind,		illegal,
	illegal,		stii_dir,		stii_ind,		illegal,
	sigi,			illegal,		illegal,		illegal,
	subb_reg,		subb_dir,		subb_ind,		subb_imm,
	subc_reg,		subc_dir,		subc_ind,		subc_imm,
	subf_reg,		subf_dir,		subf_ind,		subf_imm,
	subi_reg,		subi_dir,		subi_ind,		subi_imm,		/* 0x30 */
	subrb_reg,		subrb_dir,		subrb_ind,		subrb_imm,
	subrf_reg,		subrf_dir,		subrf_ind,		subrf_imm,
	subri_reg,		subri_dir,		subri_ind,		subri_imm,
	tstb_reg,		tstb_dir,		tstb_ind,		tstb_imm,
	xor_reg,		xor_dir,		xor_ind,		xor_imm,
	illegal,		iack_dir,		iack_ind,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,		/* 0x38 */
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,

	addc3_regreg,	addc3_indreg,	addc3_regind,	addc3_indind,	/* 0x40 */
	addf3_regreg,	addf3_indreg,	addf3_regind,	addf3_indind,
	addi3_regreg,	addi3_indreg,	addi3_regind,	addi3_indind,
	and3_regreg,	and3_indreg,	and3_regind,	and3_indind,
	andn3_regreg,	andn3_indreg,	andn3_regind,	andn3_indind,
	ash3_regreg,	ash3_indreg,	ash3_regind,	ash3_indind,
	cmpf3_regreg,	cmpf3_indreg,	cmpf3_regind,	cmpf3_indind,
	cmpi3_regreg,	cmpi3_indreg,	cmpi3_regind,	cmpi3_indind,
	lsh3_regreg,	lsh3_indreg,	lsh3_regind,	lsh3_indind,	/* 0x48 */
	mpyf3_regreg,	mpyf3_indreg,	mpyf3_regind,	mpyf3_indind,
	mpyi3_regreg,	mpyi3_indreg,	mpyi3_regind,	mpyi3_indind,
	or3_regreg,		or3_indreg,		or3_regind,		or3_indind,
	subb3_regreg,	subb3_indreg,	subb3_regind,	subb3_indind,
	subf3_regreg,	subf3_indreg,	subf3_regind,	subf3_indind,
	subi3_regreg,	subi3_indreg,	subi3_regind,	subi3_indind,
	tstb3_regreg,	tstb3_indreg,	tstb3_regind,	tstb3_indind,
	xor3_regreg,	xor3_indreg,	xor3_regind,	xor3_indind,	/* 0x50 */
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,		/* 0x58 */
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,		/* 0x60 */
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,		/* 0x68 */
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,		/* 0x70 */
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,		/* 0x78 */
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,

	ldfu_reg,		ldfu_dir,		ldfu_ind,		ldfu_imm,		/* 0x80 */
	ldflo_reg,		ldflo_dir,		ldflo_ind,		ldflo_imm,
	ldfls_reg,		ldfls_dir,		ldfls_ind,		ldfls_imm,
	ldfhi_reg,		ldfhi_dir,		ldfhi_ind,		ldfhi_imm,
	ldfhs_reg,		ldfhs_dir,		ldfhs_ind,		ldfhs_imm,
	ldfeq_reg,		ldfeq_dir,		ldfeq_ind,		ldfeq_imm,
	ldfne_reg,		ldfne_dir,		ldfne_ind,		ldfne_imm,
	ldflt_reg,		ldflt_dir,		ldflt_ind,		ldflt_imm,
	ldfle_reg,		ldfle_dir,		ldfle_ind,		ldfle_imm,		/* 0x88 */
	ldfgt_reg,		ldfgt_dir,		ldfgt_ind,		ldfgt_imm,
	ldfge_reg,		ldfge_dir,		ldfge_ind,		ldfge_imm,
	illegal,		illegal,		illegal,		illegal,
	ldfnv_reg,		ldfnv_dir,		ldfnv_ind,		ldfnv_imm,
	ldfv_reg,		ldfv_dir,		ldfv_ind,		ldfv_imm,
	ldfnuf_reg,		ldfnuf_dir,		ldfnuf_ind,		ldfnuf_imm,
	ldfuf_reg,		ldfuf_dir,		ldfuf_ind,		ldfuf_imm,
	ldfnlv_reg,		ldfnlv_dir,		ldfnlv_ind,		ldfnlv_imm,		/* 0x90 */
	ldflv_reg,		ldflv_dir,		ldflv_ind,		ldflv_imm,
	ldfnluf_reg,	ldfnluf_dir,	ldfnluf_ind,	ldfnluf_imm,
	ldfluf_reg,		ldfluf_dir,		ldfluf_ind,		ldfluf_imm,
	ldfzuf_reg,		ldfzuf_dir,		ldfzuf_ind,		ldfzuf_imm,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,		/* 0x98 */
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	ldiu_reg,		ldiu_dir,		ldiu_ind,		ldiu_imm,		/* 0xa0 */
	ldilo_reg,		ldilo_dir,		ldilo_ind,		ldilo_imm,
	ldils_reg,		ldils_dir,		ldils_ind,		ldils_imm,
	ldihi_reg,		ldihi_dir,		ldihi_ind,		ldihi_imm,
	ldihs_reg,		ldihs_dir,		ldihs_ind,		ldihs_imm,
	ldieq_reg,		ldieq_dir,		ldieq_ind,		ldieq_imm,
	ldine_reg,		ldine_dir,		ldine_ind,		ldine_imm,
	ldilt_reg,		ldilt_dir,		ldilt_ind,		ldilt_imm,
	ldile_reg,		ldile_dir,		ldile_ind,		ldile_imm,		/* 0xa8 */
	ldigt_reg,		ldigt_dir,		ldigt_ind,		ldigt_imm,
	ldige_reg,		ldige_dir,		ldige_ind,		ldige_imm,
	illegal,		illegal,		illegal,		illegal,
	ldinv_reg,		ldinv_dir,		ldinv_ind,		ldinv_imm,
	ldiv_reg,		ldiv_dir,		ldiv_ind,		ldiv_imm,
	ldinuf_reg,		ldinuf_dir,		ldinuf_ind,		ldinuf_imm,
	ldiuf_reg,		ldiuf_dir,		ldiuf_ind,		ldiuf_imm,
	ldinlv_reg,		ldinlv_dir,		ldinlv_ind,		ldinlv_imm,		/* 0xb0 */
	ldilv_reg,		ldilv_dir,		ldilv_ind,		ldilv_imm,
	ldinluf_reg,	ldinluf_dir,	ldinluf_ind,	ldinluf_imm,
	ldiluf_reg,		ldiluf_dir,		ldiluf_ind,		ldiluf_imm,
	ldizuf_reg,		ldizuf_dir,		ldizuf_ind,		ldizuf_imm,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,		/* 0xb8 */
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,

	br_imm,			br_imm,			br_imm,			br_imm,			/* 0xc0 */
	br_imm,			br_imm,			br_imm,			br_imm,
	brd_imm,		brd_imm,		brd_imm,		brd_imm,
	brd_imm,		brd_imm,		brd_imm,		brd_imm,
	call_imm,		call_imm,		call_imm,		call_imm,
	call_imm,		call_imm,		call_imm,		call_imm,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	rptb_imm,		rptb_imm,		rptb_imm,		rptb_imm,		/* 0xc8 */
	rptb_imm,		rptb_imm,		rptb_imm,		rptb_imm,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	swi,			illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	brc_reg,		brcd_reg,		illegal,		illegal,		/* 0xd0 */
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	brc_imm,		brcd_imm,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	dbc_reg,		dbcd_reg,		dbc_reg,		dbcd_reg,		/* 0xd8 */
	dbc_reg,		dbcd_reg,		dbc_reg,		dbcd_reg,
	dbc_reg,		dbcd_reg,		dbc_reg,		dbcd_reg,
	dbc_reg,		dbcd_reg,		dbc_reg,		dbcd_reg,
	dbc_imm,		dbcd_imm,		dbc_imm,		dbcd_imm,
	dbc_imm,		dbcd_imm,		dbc_imm,		dbcd_imm,
	dbc_imm,		dbcd_imm,		dbc_imm,		dbcd_imm,
	dbc_imm,		dbcd_imm,		dbc_imm,		dbcd_imm,
	callc_reg,		illegal,		illegal,		illegal,		/* 0xe0 */
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	callc_imm,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	trapc,			illegal,		illegal,		illegal,		/* 0xe8 */
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	retic_reg,		illegal,		illegal,		illegal,		/* 0xf0 */
	retsc_reg,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,		/* 0xf8 */
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,

	mpyaddf_0,		mpyaddf_0,		mpyaddf_0,		mpyaddf_0,		/* 0x100 */
	mpyaddf_0,		mpyaddf_0,		mpyaddf_0,		mpyaddf_0,
	mpyaddf_1,		mpyaddf_1,		mpyaddf_1,		mpyaddf_1,
	mpyaddf_1,		mpyaddf_1,		mpyaddf_1,		mpyaddf_1,
	mpyaddf_2,		mpyaddf_2,		mpyaddf_2,		mpyaddf_2,
	mpyaddf_2,		mpyaddf_2,		mpyaddf_2,		mpyaddf_2,
	mpyaddf_3,		mpyaddf_3,		mpyaddf_3,		mpyaddf_3,
	mpyaddf_3,		mpyaddf_3,		mpyaddf_3,		mpyaddf_3,
	mpysubf_0,		mpysubf_0,		mpysubf_0,		mpysubf_0,		/* 0x108 */
	mpysubf_0,		mpysubf_0,		mpysubf_0,		mpysubf_0,
	mpysubf_1,		mpysubf_1,		mpysubf_1,		mpysubf_1,
	mpysubf_1,		mpysubf_1,		mpysubf_1,		mpysubf_1,
	mpysubf_2,		mpysubf_2,		mpysubf_2,		mpysubf_2,
	mpysubf_2,		mpysubf_2,		mpysubf_2,		mpysubf_2,
	mpysubf_3,		mpysubf_3,		mpysubf_3,		mpysubf_3,
	mpysubf_3,		mpysubf_3,		mpysubf_3,		mpysubf_3,
	mpyaddi_0,		mpyaddi_0,		mpyaddi_0,		mpyaddi_0,		/* 0x110 */
	mpyaddi_0,		mpyaddi_0,		mpyaddi_0,		mpyaddi_0,
	mpyaddi_1,		mpyaddi_1,		mpyaddi_1,		mpyaddi_1,
	mpyaddi_1,		mpyaddi_1,		mpyaddi_1,		mpyaddi_1,
	mpyaddi_2,		mpyaddi_2,		mpyaddi_2,		mpyaddi_2,
	mpyaddi_2,		mpyaddi_2,		mpyaddi_2,		mpyaddi_2,
	mpyaddi_3,		mpyaddi_3,		mpyaddi_3,		mpyaddi_3,
	mpyaddi_3,		mpyaddi_3,		mpyaddi_3,		mpyaddi_3,
	mpysubi_0,		mpysubi_0,		mpysubi_0,		mpysubi_0,		/* 0x118 */
	mpysubi_0,		mpysubi_0,		mpysubi_0,		mpysubi_0,
	mpysubi_1,		mpysubi_1,		mpysubi_1,		mpysubi_1,
	mpysubi_1,		mpysubi_1,		mpysubi_1,		mpysubi_1,
	mpysubi_2,		mpysubi_2,		mpysubi_2,		mpysubi_2,
	mpysubi_2,		mpysubi_2,		mpysubi_2,		mpysubi_2,
	mpysubi_3,		mpysubi_3,		mpysubi_3,		mpysubi_3,
	mpysubi_3,		mpysubi_3,		mpysubi_3,		mpysubi_3,
	illegal,		illegal,		illegal,		illegal,		/* 0x120 */
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,		/* 0x128 */
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,		/* 0x130 */
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,		/* 0x138 */
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,

	illegal,		illegal,		illegal,		illegal,		/* 0x140 */
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,		/* 0x148 */
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,		/* 0x150 */
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,		/* 0x158 */
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,		/* 0x160 */
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,		/* 0x168 */
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,		/* 0x170 */
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,		/* 0x178 */
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,

	stfstf,			stfstf,			stfstf,			stfstf,			/* 0x180 */
	stfstf,			stfstf,			stfstf,			stfstf,
	stfstf,			stfstf,			stfstf,			stfstf,
	stfstf,			stfstf,			stfstf,			stfstf,
	stisti,			stisti,			stisti,			stisti,
	stisti,			stisti,			stisti,			stisti,
	stisti,			stisti,			stisti,			stisti,
	stisti,			stisti,			stisti,			stisti,
	ldfldf,			ldfldf,			ldfldf,			ldfldf,			/* 0x188 */
	ldfldf,			ldfldf,			ldfldf,			ldfldf,
	ldfldf,			ldfldf,			ldfldf,			ldfldf,
	ldfldf,			ldfldf,			ldfldf,			ldfldf,
	ldildi,			ldildi,			ldildi,			ldildi,
	ldildi,			ldildi,			ldildi,			ldildi,
	ldildi,			ldildi,			ldildi,			ldildi,
	ldildi,			ldildi,			ldildi,			ldildi,
	absfstf,		absfstf,		absfstf,		absfstf,		/* 0x190 */
	absfstf,		absfstf,		absfstf,		absfstf,
	absfstf,		absfstf,		absfstf,		absfstf,
	absfstf,		absfstf,		absfstf,		absfstf,
	absisti,		absisti,		absisti,		absisti,
	absisti,		absisti,		absisti,		absisti,
	absisti,		absisti,		absisti,		absisti,
	absisti,		absisti,		absisti,		absisti,
	addf3stf,		addf3stf,		addf3stf,		addf3stf,		/* 0x198 */
	addf3stf,		addf3stf,		addf3stf,		addf3stf,
	addf3stf,		addf3stf,		addf3stf,		addf3stf,
	addf3stf,		addf3stf,		addf3stf,		addf3stf,
	addi3sti,		addi3sti,		addi3sti,		addi3sti,
	addi3sti,		addi3sti,		addi3sti,		addi3sti,
	addi3sti,		addi3sti,		addi3sti,		addi3sti,
	addi3sti,		addi3sti,		addi3sti,		addi3sti,
	and3sti,		and3sti,		and3sti,		and3sti,		/* 0x1a0 */
	and3sti,		and3sti,		and3sti,		and3sti,
	and3sti,		and3sti,		and3sti,		and3sti,
	and3sti,		and3sti,		and3sti,		and3sti,
	ash3sti,		ash3sti,		ash3sti,		ash3sti,
	ash3sti,		ash3sti,		ash3sti,		ash3sti,
	ash3sti,		ash3sti,		ash3sti,		ash3sti,
	ash3sti,		ash3sti,		ash3sti,		ash3sti,
	fixsti,			fixsti,			fixsti,			fixsti,			/* 0x1a8 */
	fixsti,			fixsti,			fixsti,			fixsti,
	fixsti,			fixsti,			fixsti,			fixsti,
	fixsti,			fixsti,			fixsti,			fixsti,
	floatstf,		floatstf,		floatstf,		floatstf,
	floatstf,		floatstf,		floatstf,		floatstf,
	floatstf,		floatstf,		floatstf,		floatstf,
	floatstf,		floatstf,		floatstf,		floatstf,
	ldfstf,			ldfstf,			ldfstf,			ldfstf,			/* 0x1b0 */
	ldfstf,			ldfstf,			ldfstf,			ldfstf,
	ldfstf,			ldfstf,			ldfstf,			ldfstf,
	ldfstf,			ldfstf,			ldfstf,			ldfstf,
	ldisti,			ldisti,			ldisti,			ldisti,
	ldisti,			ldisti,			ldisti,			ldisti,
	ldisti,			ldisti,			ldisti,			ldisti,
	ldisti,			ldisti,			ldisti,			ldisti,
	lsh3sti,		lsh3sti,		lsh3sti,		lsh3sti,		/* 0x1b8 */
	lsh3sti,		lsh3sti,		lsh3sti,		lsh3sti,
	lsh3sti,		lsh3sti,		lsh3sti,		lsh3sti,
	lsh3sti,		lsh3sti,		lsh3sti,		lsh3sti,
	mpyf3stf,		mpyf3stf,		mpyf3stf,		mpyf3stf,
	mpyf3stf,		mpyf3stf,		mpyf3stf,		mpyf3stf,
	mpyf3stf,		mpyf3stf,		mpyf3stf,		mpyf3stf,
	mpyf3stf,		mpyf3stf,		mpyf3stf,		mpyf3stf,

	mpyi3sti,		mpyi3sti,		mpyi3sti,		mpyi3sti,		/* 0x1c0 */
	mpyi3sti,		mpyi3sti,		mpyi3sti,		mpyi3sti,
	mpyi3sti,		mpyi3sti,		mpyi3sti,		mpyi3sti,
	mpyi3sti,		mpyi3sti,		mpyi3sti,		mpyi3sti,
	negfstf,		negfstf,		negfstf,		negfstf,
	negfstf,		negfstf,		negfstf,		negfstf,
	negfstf,		negfstf,		negfstf,		negfstf,
	negfstf,		negfstf,		negfstf,		negfstf,
	negisti,		negisti,		negisti,		negisti,		/* 0x1c8 */
	negisti,		negisti,		negisti,		negisti,
	negisti,		negisti,		negisti,		negisti,
	negisti,		negisti,		negisti,		negisti,
	notsti,			notsti,			notsti,			notsti,
	notsti,			notsti,			notsti,			notsti,
	notsti,			notsti,			notsti,			notsti,
	notsti,			notsti,			notsti,			notsti,
	or3sti,			or3sti,			or3sti,			or3sti,			/* 0x1d0 */
	or3sti,			or3sti,			or3sti,			or3sti,
	or3sti,			or3sti,			or3sti,			or3sti,
	or3sti,			or3sti,			or3sti,			or3sti,
	subf3stf,		subf3stf,		subf3stf,		subf3stf,
	subf3stf,		subf3stf,		subf3stf,		subf3stf,
	subf3stf,		subf3stf,		subf3stf,		subf3stf,
	subf3stf,		subf3stf,		subf3stf,		subf3stf,
	subi3sti,		subi3sti,		subi3sti,		subi3sti,		/* 0x1d8 */
	subi3sti,		subi3sti,		subi3sti,		subi3sti,
	subi3sti,		subi3sti,		subi3sti,		subi3sti,
	subi3sti,		subi3sti,		subi3sti,		subi3sti,
	xor3sti,		xor3sti,		xor3sti,		xor3sti,
	xor3sti,		xor3sti,		xor3sti,		xor3sti,
	xor3sti,		xor3sti,		xor3sti,		xor3sti,
	xor3sti,		xor3sti,		xor3sti,		xor3sti,
	illegal,		illegal,		illegal,		illegal,		/* 0x1e0 */
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,		/* 0x1e8 */
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,		/* 0x1f0 */
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,		/* 0x1f8 */
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal,
	illegal,		illegal,		illegal,		illegal
};
