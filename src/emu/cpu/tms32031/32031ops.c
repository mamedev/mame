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

#define IREG(rnum)			(tms32031.r[rnum].i32[0])
#define FREGEXP(rnum)		(EXPONENT(&tms32031.r[rnum]))
#define FREGMAN(rnum)		(MANTISSA(&tms32031.r[rnum]))

#define FP2LONG(rnum)		((FREGEXP(rnum) << 24) | ((UINT32)FREGMAN(rnum) >> 8))
#define LONG2FP(rnum,v)		do { SET_MANTISSA(&tms32031.r[rnum], (v) << 8); SET_EXPONENT(&tms32031.r[rnum], (INT32)(v) >> 24); } while (0)
#define SHORT2FP(rnum,v)	do { \
								if ((UINT16)(v) == 0x8000) { SET_MANTISSA(&tms32031.r[rnum], 0); SET_EXPONENT(&tms32031.r[rnum], -128); } \
								else { SET_MANTISSA(&tms32031.r[rnum], (v) << 20); SET_EXPONENT(&tms32031.r[rnum], (INT16)(v) >> 12); } \
							} while (0)

#define DIRECT()			(((IREG(TMR_DP) & 0xff) << 16) | ((UINT16)OP))
#define INDIRECT_D(o)		((*indirect_d[((o) >> 3) & 31])(o))
#define INDIRECT_1(o)		((*indirect_1[((o) >> 3) & 31])(o))
#define INDIRECT_1_DEF(o)	((*indirect_1_def[((o) >> 3) & 31])(o))

#define SIGN(val)			((val) & 0x80000000)

#define OVERFLOW_SUB(a,b,r)	((INT32)(((a) ^ (b)) & ((a) ^ (r))) < 0)
#define OVERFLOW_ADD(a,b,r)	((INT32)(~((a) ^ (b)) & ((a) ^ (r))) < 0)

#define CLR_FLAGS(f)		do { IREG(TMR_ST) &= ~(f); } while (0)
#define CLR_NVUF()			CLR_FLAGS(NFLAG | VFLAG | UFFLAG)
#define CLR_NZVUF()			CLR_FLAGS(NFLAG | ZFLAG | VFLAG | UFFLAG)
#define CLR_NZCVUF()		CLR_FLAGS(NFLAG | ZFLAG | VFLAG | CFLAG | UFFLAG)

#define OR_C(flag)			do { IREG(TMR_ST) |= flag & CFLAG; } while (0)
#define OR_NZ(val)			do { IREG(TMR_ST) |= (((val) >> 28) & NFLAG) | (((val) == 0) << 2); } while (0)
#define OR_NZF(reg)			do { IREG(TMR_ST) |= ((MANTISSA(reg) >> 28) & NFLAG) | ((EXPONENT(reg) == -128) << 2); } while (0)
#define OR_NUF(reg)			do { int temp = (EXPONENT(reg) == -128) << 4; IREG(TMR_ST) |= ((MANTISSA(reg) >> 28) & NFLAG) | (temp) | (temp << 2); } while (0)
#define OR_V_SUB(a,b,r)		do { UINT32 temp = ((((a) ^ (b)) & ((a) ^ (r))) >> 30) & VFLAG; IREG(TMR_ST) |= temp | (temp << 4); } while (0)
#define OR_V_ADD(a,b,r)		do { UINT32 temp = ((~((a) ^ (b)) & ((a) ^ (r))) >> 30) & VFLAG; IREG(TMR_ST) |= temp | (temp << 4); } while (0)
#define OR_C_SUB(a,b,r)		do { IREG(TMR_ST) |= ((UINT32)(b) > (UINT32)(a)); } while (0)
#define OR_C_ADD(a,b,r)		do { IREG(TMR_ST) |= ((UINT32)(~(a)) < (UINT32)(b)); } while (0)
#define OR_NZCV_SUB(a,b,r)	do { OR_V_SUB(a,b,r); OR_C_SUB(a,b,r); OR_NZ(r); } while (0)
#define OR_NZCV_ADD(a,b,r)	do { OR_V_ADD(a,b,r); OR_C_ADD(a,b,r); OR_NZ(r); } while (0)

#define OVM					(IREG(TMR_ST) & OVMFLAG)

#define UPDATE_DEF()		if (defptr) { *defptr = defval; defptr = NULL; }



/***************************************************************************
    FORWARD DECLARATIONS
***************************************************************************/

void (*const tms32031ops[])(void);



/***************************************************************************
    LOCAL VARIABLES
***************************************************************************/

static UINT32 *defptr;
static UINT32 defval;



/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

static void illegal(void)
{
	if ((Machine->debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		logerror("Illegal op @ %06X: %08X (tbl=%03X)\n", tms32031.pc - 1, OP, OP >> 21);
		debugger_break(Machine);
	}
}


static void unimplemented(void)
{
	fatalerror("Unimplemented op @ %06X: %08X (tbl=%03X)", tms32031.pc - 1, OP, OP >> 21);
}


INLINE void execute_one(void)
{
	debugger_instruction_hook(Machine, tms32031.pc);
	OP = ROPCODE(tms32031.pc);
	tms32031_icount -= 2;	/* 2 clocks per cycle */
	tms32031.pc++;
#if (LOG_OPCODE_USAGE)
	hits[OP >> 21]++;
#endif
	(*tms32031ops[OP >> 21])();
}


static void update_special(int dreg)
{
	if (dreg == TMR_BK)
	{
		UINT32 temp = IREG(TMR_BK);
		tms32031.bkmask = temp;
		while (temp >>= 1)
			tms32031.bkmask |= temp;
	}
	else if (dreg == TMR_IOF)
	{
		if (tms32031.xf0_w && IREG(TMR_IOF) & 0x002)
			(*tms32031.xf0_w)((IREG(TMR_IOF) >> 2) & 1);
		if (tms32031.xf1_w && IREG(TMR_IOF) & 0x020)
			(*tms32031.xf1_w)((IREG(TMR_IOF) >> 6) & 1);
	}
	else if (dreg == TMR_ST || dreg == TMR_IF || dreg == TMR_IE)
		check_irqs();
}



/***************************************************************************
    CONDITION CODES
***************************************************************************/

#define	CONDITION_LO	(IREG(TMR_ST) & CFLAG)
#define CONDITION_LS	(IREG(TMR_ST) & (CFLAG | ZFLAG))
#define CONDITION_HI	(!(IREG(TMR_ST) & (CFLAG | ZFLAG)))
#define CONDITION_HS	(!(IREG(TMR_ST) & CFLAG))
#define CONDITION_EQ	(IREG(TMR_ST) & ZFLAG)
#define CONDITION_NE	(!(IREG(TMR_ST) & ZFLAG))
#define CONDITION_LT	(IREG(TMR_ST) & NFLAG)
#define CONDITION_LE	(IREG(TMR_ST) & (NFLAG | ZFLAG))
#define CONDITION_GT	(!(IREG(TMR_ST) & (NFLAG | ZFLAG)))
#define CONDITION_GE	(!(IREG(TMR_ST) & NFLAG))
#define CONDITION_NV	(!(IREG(TMR_ST) & VFLAG))
#define CONDITION_V		(IREG(TMR_ST) & VFLAG)
#define CONDITION_NUF	(!(IREG(TMR_ST) & UFFLAG))
#define CONDITION_UF	(IREG(TMR_ST) & UFFLAG)
#define CONDITION_NLV	(!(IREG(TMR_ST) & LVFLAG))
#define CONDITION_LV	(IREG(TMR_ST) & LVFLAG)
#define CONDITION_NLUF	(!(IREG(TMR_ST) & LUFFLAG))
#define CONDITION_LUF	(IREG(TMR_ST) & LUFFLAG)
#define CONDITION_ZUF	(IREG(TMR_ST) & (UFFLAG | ZFLAG))

static int condition(int which)
{
	switch (which & 0x1f)
	{
		case 0:		return 1;
		case 1:		return CONDITION_LO;
		case 2:		return CONDITION_LS;
		case 3:		return CONDITION_HI;
		case 4:		return CONDITION_HS;
		case 5:		return CONDITION_EQ;
		case 6:		return CONDITION_NE;
		case 7:		return CONDITION_LT;
		case 8:		return CONDITION_LE;
		case 9:		return CONDITION_GT;
		case 10:	return CONDITION_GE;
		case 12:	return CONDITION_NV;
		case 13:	return CONDITION_V;
		case 14:	return CONDITION_NUF;
		case 15:	return CONDITION_UF;
		case 16:	return CONDITION_NLV;
		case 17:	return CONDITION_LV;
		case 18:	return CONDITION_NLUF;
		case 19:	return CONDITION_LUF;
		case 20:	return CONDITION_ZUF;
		default:	illegal(); return 1;
	}
}



/***************************************************************************
    FLOATING POINT HELPERS
***************************************************************************/

#if USE_FP
void double_to_dsp_with_flags(double val, union genreg *result)
{
	int mantissa, exponent;
	int_double id;
	id.d = val;

	CLR_NZVUF();

	mantissa = ((id.i[BYTE_XOR_BE(0)] & 0x000fffff) << 11) | ((id.i[BYTE_XOR_BE(1)] & 0xffe00000) >> 21);
	exponent = ((id.i[BYTE_XOR_BE(0)] & 0x7ff00000) >> 20) - 1023;
	if (exponent <= -128)
	{
		SET_MANTISSA(result, 0);
		SET_EXPONENT(result, -128);
		IREG(TMR_ST) |= UFFLAG | LUFFLAG | ZFLAG;
	}
	else if (exponent > 127)
	{
		if ((INT32)id.i[BYTE_XOR_BE(0)] >= 0)
			SET_MANTISSA(result, 0x7fffffff);
		else
		{
			SET_MANTISSA(result, 0x80000001);
			IREG(TMR_ST) |= NFLAG;
		}
		SET_EXPONENT(result, 127);
		IREG(TMR_ST) |= VFLAG | LVFLAG;
	}
	else if (val == 0)
	{
		SET_MANTISSA(result, 0);
		SET_EXPONENT(result, -128);
		IREG(TMR_ST) |= ZFLAG;
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
		IREG(TMR_ST) |= NFLAG;
	}
	else
	{
		SET_MANTISSA(result, 0x80000000);
		SET_EXPONENT(result, exponent - 1);
		IREG(TMR_ST) |= NFLAG;
	}
}
#endif

/* integer to floating point conversion */
#if USE_FP
static void int2float(union genreg *srcdst)
{
	double val = MANTISSA(srcdst);
	double_to_dsp_with_flags(val, srcdst);
}
#else
static void int2float(union genreg *srcdst)
{
	UINT32 man = MANTISSA(srcdst);
	int exp, cnt;

	/* never overflows or underflows */
	CLR_NZVUF();

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
	OR_NZF(srcdst);
}
#endif


/* floating point to integer conversion */
#if USE_FP
static void float2int(union genreg *srcdst, int setflags)
{
	INT32 val;

	if (setflags) CLR_NZVUF();
	if (EXPONENT(srcdst) > 30)
	{
		if ((INT32)MANTISSA(srcdst) >= 0)
			val = 0x7fffffff;
		else
			val = 0x80000000;
		if (setflags) IREG(TMR_ST) |= VFLAG | LVFLAG;
	}
	else
		val = floor(dsp_to_double(srcdst));
	SET_MANTISSA(srcdst, val);
	if (setflags) OR_NZ(val);
}
#else
static void float2int(union genreg *srcdst, int setflags)
{
	INT32 man = MANTISSA(srcdst);
	int shift = 31 - EXPONENT(srcdst);

	/* never underflows */
	if (setflags) CLR_NZVUF();

	/* if we've got too much to handle, overflow */
	if (shift <= 0)
	{
		SET_MANTISSA(srcdst, (man >= 0) ? 0x7fffffff : 0x80000000);
		if (setflags) IREG(TMR_ST) |= VFLAG | LVFLAG;
	}

	/* if we're too small, go to 0 or -1 */
	else if (shift > 31)
		SET_MANTISSA(srcdst, man >> 31);

	/* we're in the middle; shift it */
	else
		SET_MANTISSA(srcdst, (man >> shift) ^ (1 << (31 - shift)));

	/* set the NZ flags */
	if (setflags) OR_NZ(MANTISSA(srcdst));
}
#endif


/* compute the negative of a floating point value */
#if USE_FP
static void negf(union genreg *dst, union genreg *src)
{
	double val = -dsp_to_double(src);
	double_to_dsp_with_flags(val, dst);
}
#else
static void negf(union genreg *dst, union genreg *src)
{
	INT32 man = MANTISSA(src);

	CLR_NZVUF();

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
	OR_NZF(dst);
}
#endif



/* add two floating point values */
#if USE_FP
static void addf(union genreg *dst, union genreg *src1, union genreg *src2)
{
	double val = dsp_to_double(src1) + dsp_to_double(src2);
	double_to_dsp_with_flags(val, dst);
}
#else
static void addf(union genreg *dst, union genreg *src1, union genreg *src2)
{
	INT64 man;
	INT64 m1, m2;
	int exp, cnt;

	/* reset over/underflow conditions */
	CLR_NZVUF();

	/* first check for 0 operands */
	if (EXPONENT(src1) == -128)
	{
		*dst = *src2;
		OR_NZF(dst);
		return;
	}
	if (EXPONENT(src2) == -128)
	{
		*dst = *src1;
		OR_NZF(dst);
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
			OR_NZF(dst);
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
			OR_NZF(dst);
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
		IREG(TMR_ST) |= UFFLAG | LUFFLAG;
	}

	/* check for overflow */
	else if (exp > 127)
	{
		man = (man < 0) ? 0x00000000 : 0xffffffff;
		exp = 127;
		IREG(TMR_ST) |= VFLAG | LVFLAG;
	}

	/* store the result back, removing the implicit one and putting */
	/* back the sign bit */
	SET_MANTISSA(dst, (UINT32)man ^ 0x80000000);
	SET_EXPONENT(dst, exp);
	OR_NZF(dst);
}
#endif


/* subtract two floating point values */
#if USE_FP
static void subf(union genreg *dst, union genreg *src1, union genreg *src2)
{
	double val = dsp_to_double(src1) - dsp_to_double(src2);
	double_to_dsp_with_flags(val, dst);
}
#else
static void subf(union genreg *dst, union genreg *src1, union genreg *src2)
{
	INT64 man;
	INT64 m1, m2;
	int exp, cnt;

	/* reset over/underflow conditions */
	CLR_NZVUF();

	/* first check for 0 operands */
	if (EXPONENT(src2) == -128)
	{
		*dst = *src1;
		OR_NZF(dst);
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
			OR_NZF(dst);
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
			negf(dst, src2);
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
		IREG(TMR_ST) |= UFFLAG | LUFFLAG;
	}

	/* check for overflow */
	else if (exp > 127)
	{
		man = (man < 0) ? 0x00000000 : 0xffffffff;
		exp = 127;
		IREG(TMR_ST) |= VFLAG | LVFLAG;
	}

	/* store the result back, removing the implicit one and putting */
	/* back the sign bit */
	SET_MANTISSA(dst, (UINT32)man ^ 0x80000000);
	SET_EXPONENT(dst, exp);
	OR_NZF(dst);
}
#endif


/* multiply two floating point values */
#if USE_FP
static void mpyf(union genreg *dst, union genreg *src1, union genreg *src2)
{
	double val = (double)dsp_to_float(src1) * (double)dsp_to_float(src2);
	double_to_dsp_with_flags(val, dst);
}
#else
static void mpyf(union genreg *dst, union genreg *src1, union genreg *src2)
{
	INT64 man;
	INT32 m1, m2;
	int exp;

	/* reset over/underflow conditions */
	CLR_NZVUF();

	/* first check for 0 multipliers and return 0 in any case */
	if (EXPONENT(src1) == -128 || EXPONENT(src2) == -128)
	{
		SET_MANTISSA(dst, 0);
		SET_EXPONENT(dst, -128);
		OR_NZF(dst);
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
		IREG(TMR_ST) |= UFFLAG | LUFFLAG;
	}

	/* check for overflow */
	else if (exp > 127)
	{
		man = (man < 0) ? 0x00000000 : 0xffffffff;
		exp = 127;
		IREG(TMR_ST) |= VFLAG | LVFLAG;
	}

	/* store the result back, removing the implicit one and putting */
	/* back the sign bit */
	SET_MANTISSA(dst, (UINT32)man ^ 0x80000000);
	SET_EXPONENT(dst, exp);
	OR_NZF(dst);
}
#endif


/* normalize a floating point value */
#if USE_FP
static void norm(union genreg *dst, union genreg *src)
{
	fatalerror("norm not implemented");
}
#else
static void norm(union genreg *dst, union genreg *src)
{
	INT32 man = MANTISSA(src);
	int exp = EXPONENT(src);

	CLR_NZVUF();

	if (exp == -128 || man == 0)
	{
		SET_MANTISSA(dst, 0);
		SET_EXPONENT(dst, -128);
		if (man != 0)
			IREG(TMR_ST) |= UFFLAG | LUFFLAG;
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
			IREG(TMR_ST) |= UFFLAG | LUFFLAG;
		}
	}

	SET_MANTISSA(dst, man);
	SET_EXPONENT(dst, exp);
	OR_NZF(dst);
}
#endif




/***************************************************************************
    INDIRECT MEMORY REFS
***************************************************************************/

/* immediate displacement variants */

static UINT32 mod00_d(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(reg) + (UINT8)OP;
}

static UINT32 mod01_d(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(reg) - (UINT8)OP;
}

static UINT32 mod02_d(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	IREG(reg) += (UINT8)OP;
	return IREG(reg);
}

static UINT32 mod03_d(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	IREG(reg) -= (UINT8)OP;
	return IREG(reg);
}

static UINT32 mod04_d(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	IREG(reg) += (UINT8)OP;
	return result;
}

static UINT32 mod05_d(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	IREG(reg) -= (UINT8)OP;
	return result;
}

static UINT32 mod06_d(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	INT32 temp = (result & tms32031.bkmask) + (UINT8)OP;
	if (temp >= IREG(TMR_BK))
		temp -= IREG(TMR_BK);
	IREG(reg) = (IREG(reg) & ~tms32031.bkmask) | (temp & tms32031.bkmask);
	return result;
}

static UINT32 mod07_d(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	INT32 temp = (result & tms32031.bkmask) - (UINT8)OP;
	if (temp < 0)
		temp += IREG(TMR_BK);
	IREG(reg) = (IREG(reg) & ~tms32031.bkmask) | (temp & tms32031.bkmask);
	return result;
}


/* immediate displacement variants (implied 1) */

static UINT32 mod00_1(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(reg) + 1;
}

static UINT32 mod01_1(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(reg) - 1;
}

static UINT32 mod02_1(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return ++IREG(reg);
}

static UINT32 mod03_1(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return --IREG(reg);
}

static UINT32 mod04_1(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(reg)++;
}

static UINT32 mod05_1(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(reg)--;
}

static UINT32 mod06_1(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	INT32 temp = (result & tms32031.bkmask) + 1;
	if (temp >= IREG(TMR_BK))
		temp -= IREG(TMR_BK);
	IREG(reg) = (IREG(reg) & ~tms32031.bkmask) | (temp & tms32031.bkmask);
	return result;
}

static UINT32 mod07_1(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	INT32 temp = (result & tms32031.bkmask) - 1;
	if (temp < 0)
		temp += IREG(TMR_BK);
	IREG(reg) = (IREG(reg) & ~tms32031.bkmask) | (temp & tms32031.bkmask);
	return result;
}


/* IR0 displacement variants */

static UINT32 mod08(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(reg) + IREG(TMR_IR0);
}

static UINT32 mod09(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(reg) - IREG(TMR_IR0);
}

static UINT32 mod0a(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	IREG(reg) += IREG(TMR_IR0);
	return IREG(reg);
}

static UINT32 mod0b(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	IREG(reg) -= IREG(TMR_IR0);
	return IREG(reg);
}

static UINT32 mod0c(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	IREG(reg) += IREG(TMR_IR0);
	return result;
}

static UINT32 mod0d(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	IREG(reg) -= IREG(TMR_IR0);
	return result;
}

static UINT32 mod0e(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	INT32 temp = (result & tms32031.bkmask) + IREG(TMR_IR0);
	if (temp >= IREG(TMR_BK))
		temp -= IREG(TMR_BK);
	IREG(reg) = (IREG(reg) & ~tms32031.bkmask) | (temp & tms32031.bkmask);
	return result;
}

static UINT32 mod0f(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	INT32 temp = (result & tms32031.bkmask) - IREG(TMR_IR0);
	if (temp < 0)
		temp += IREG(TMR_BK);
	IREG(reg) = (IREG(reg) & ~tms32031.bkmask) | (temp & tms32031.bkmask);
	return result;
}


/* IR1 displacement variants */

static UINT32 mod10(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(reg) + IREG(TMR_IR1);
}

static UINT32 mod11(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(reg) - IREG(TMR_IR1);
}

static UINT32 mod12(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	IREG(reg) += IREG(TMR_IR1);
	return IREG(reg);
}

static UINT32 mod13(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	IREG(reg) -= IREG(TMR_IR1);
	return IREG(reg);
}

static UINT32 mod14(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	IREG(reg) += IREG(TMR_IR1);
	return result;
}

static UINT32 mod15(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	IREG(reg) -= IREG(TMR_IR1);
	return result;
}

static UINT32 mod16(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	INT32 temp = (result & tms32031.bkmask) + IREG(TMR_IR1);
	if (temp >= IREG(TMR_BK))
		temp -= IREG(TMR_BK);
	IREG(reg) = (IREG(reg) & ~tms32031.bkmask) | (temp & tms32031.bkmask);
	return result;
}

static UINT32 mod17(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	INT32 temp = (result & tms32031.bkmask) - IREG(TMR_IR1);
	if (temp < 0)
		temp += IREG(TMR_BK);
	IREG(reg) = (IREG(reg) & ~tms32031.bkmask) | (temp & tms32031.bkmask);
	return result;
}


/* special variants */

static UINT32 mod18(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(reg);
}

static UINT32 mod19(UINT8 ar)
{
	unimplemented();
	return 0;
}

static UINT32 modillegal(UINT8 ar)
{
	illegal();
	return 0;
}


/* immediate displacement variants (implied 1) */

static UINT32 mod02_1_def(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	defptr = &IREG(reg);
	defval = IREG(reg) + 1;
	return defval;
}

static UINT32 mod03_1_def(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	defptr = &IREG(reg);
	defval = IREG(reg) - 1;
	return defval;
}

static UINT32 mod04_1_def(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	defptr = &IREG(reg);
	defval = IREG(reg) + 1;
	return IREG(reg);
}

static UINT32 mod05_1_def(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	defptr = &IREG(reg);
	defval = IREG(reg) - 1;
	return IREG(reg);
}

static UINT32 mod06_1_def(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	INT32 temp = (result & tms32031.bkmask) + 1;
	if (temp >= IREG(TMR_BK))
		temp -= IREG(TMR_BK);
	defptr = &IREG(reg);
	defval = (IREG(reg) & ~tms32031.bkmask) | (temp & tms32031.bkmask);
	return result;
}

static UINT32 mod07_1_def(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	INT32 temp = (result & tms32031.bkmask) - 1;
	if (temp < 0)
		temp += IREG(TMR_BK);
	defptr = &IREG(reg);
	defval = (IREG(reg) & ~tms32031.bkmask) | (temp & tms32031.bkmask);
	return result;
}


/* IR0 displacement variants */

static UINT32 mod0a_def(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	defptr = &IREG(reg);
	defval = IREG(reg) + IREG(TMR_IR0);
	return defval;
}

static UINT32 mod0b_def(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	defptr = &IREG(reg);
	defval = IREG(reg) - IREG(TMR_IR0);
	return defval;
}

static UINT32 mod0c_def(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	defptr = &IREG(reg);
	defval = IREG(reg) + IREG(TMR_IR0);
	return IREG(reg);
}

static UINT32 mod0d_def(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	defptr = &IREG(reg);
	defval = IREG(reg) - IREG(TMR_IR0);
	return IREG(reg);
}

static UINT32 mod0e_def(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	INT32 temp = (result & tms32031.bkmask) + IREG(TMR_IR0);
	if (temp >= IREG(TMR_BK))
		temp -= IREG(TMR_BK);
	defptr = &IREG(reg);
	defval = (IREG(reg) & ~tms32031.bkmask) | (temp & tms32031.bkmask);
	return result;
}

static UINT32 mod0f_def(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	INT32 temp = (result & tms32031.bkmask) - IREG(TMR_IR0);
	if (temp < 0)
		temp += IREG(TMR_BK);
	defptr = &IREG(reg);
	defval = (IREG(reg) & ~tms32031.bkmask) | (temp & tms32031.bkmask);
	return result;
}


/* IR1 displacement variants */

static UINT32 mod12_def(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	defptr = &IREG(reg);
	defval = IREG(reg) + IREG(TMR_IR1);
	return defval;
}

static UINT32 mod13_def(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	defptr = &IREG(reg);
	defval = IREG(reg) - IREG(TMR_IR1);
	return defval;
}

static UINT32 mod14_def(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	defptr = &IREG(reg);
	defval = IREG(reg) + IREG(TMR_IR1);
	return IREG(reg);
}

static UINT32 mod15_def(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	defptr = &IREG(reg);
	defval = IREG(reg) - IREG(TMR_IR1);
	return IREG(reg);
}

static UINT32 mod16_def(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	INT32 temp = (result & tms32031.bkmask) + IREG(TMR_IR1);
	if (temp >= IREG(TMR_BK))
		temp -= IREG(TMR_BK);
	defptr = &IREG(reg);
	defval = (IREG(reg) & ~tms32031.bkmask) | (temp & tms32031.bkmask);
	return result;
}

static UINT32 mod17_def(UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	INT32 temp = (result & tms32031.bkmask) - IREG(TMR_IR1);
	if (temp < 0)
		temp += IREG(TMR_BK);
	defptr = &IREG(reg);
	defval = (IREG(reg) & ~tms32031.bkmask) | (temp & tms32031.bkmask);
	return result;
}


/* tables */

static UINT32 (*const indirect_d[0x20])(UINT8) =
{
	mod00_d,	mod01_d,	mod02_d,	mod03_d,	mod04_d,	mod05_d,	mod06_d,	mod07_d,
	mod08,		mod09,		mod0a,		mod0b,		mod0c,		mod0d,		mod0e,		mod0f,
	mod10,		mod11,		mod12,		mod13,		mod14,		mod15,		mod16,		mod17,
	mod18,		mod19,		modillegal,	modillegal,	modillegal,	modillegal,	modillegal,	modillegal
};


static UINT32 (*const indirect_1[0x20])(UINT8) =
{
	mod00_1,	mod01_1,	mod02_1,	mod03_1,	mod04_1,	mod05_1,	mod06_1,	mod07_1,
	mod08,		mod09,		mod0a,		mod0b,		mod0c,		mod0d,		mod0e,		mod0f,
	mod10,		mod11,		mod12,		mod13,		mod14,		mod15,		mod16,		mod17,
	mod18,		mod19,		modillegal,	modillegal,	modillegal,	modillegal,	modillegal,	modillegal
};


static UINT32 (*const indirect_1_def[0x20])(UINT8) =
{
	mod00_1,	mod01_1,	mod02_1_def,mod03_1_def,mod04_1_def,mod05_1_def,mod06_1_def,mod07_1_def,
	mod08,		mod09,		mod0a_def,	mod0b_def,	mod0c_def,	mod0d_def,	mod0e_def,	mod0f_def,
	mod10,		mod11,		mod12_def,	mod13_def,	mod14_def,	mod15_def,	mod16_def,	mod17_def,
	mod18,		mod19,		modillegal,	modillegal,	modillegal,	modillegal,	modillegal,	modillegal
};



/*-----------------------------------------------------*/

#define ABSF(dreg, sreg)												\
{																		\
	INT32 man = FREGMAN(sreg);											\
	CLR_NZVUF();														\
	tms32031.r[dreg] = tms32031.r[sreg];								\
	if (man < 0)														\
	{																	\
		SET_MANTISSA(&tms32031.r[dreg], ~man);							\
		if (man == (INT32)0x80000000 && FREGEXP(sreg) == 127)			\
			IREG(TMR_ST) |= VFLAG | LVFLAG;								\
	}																	\
	OR_NZF(&tms32031.r[dreg]);											\
}

static void absf_reg(void)
{
	int dreg = (OP >> 16) & 7;
	int sreg = OP & 7;
	ABSF(dreg, sreg);
}

static void absf_dir(void)
{
	UINT32 res = RMEM(DIRECT());
	int dreg = (OP >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	ABSF(dreg, TMR_TEMP1);
}

static void absf_ind(void)
{
	UINT32 res = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	ABSF(dreg, TMR_TEMP1);
}

static void absf_imm(void)
{
	int dreg = (OP >> 16) & 7;
	SHORT2FP(TMR_TEMP1, OP);
	ABSF(dreg, TMR_TEMP1);
}

/*-----------------------------------------------------*/

#define ABSI(dreg, src)												\
{																	\
	UINT32 _res = ((INT32)src < 0) ? -src : src;					\
	if (!OVM || _res != 0x80000000)									\
		IREG(dreg) = _res;											\
	else															\
		IREG(dreg) = 0x7fffffff;									\
	if (dreg < 8)													\
	{																\
		CLR_NZVUF();												\
		OR_NZ(_res);												\
		if (_res == 0x80000000) 									\
			IREG(TMR_ST) |= VFLAG | LVFLAG;							\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(dreg);										\
}

static void absi_reg(void)
{
	UINT32 src = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	ABSI(dreg, src);
}

static void absi_dir(void)
{
	UINT32 src = RMEM(DIRECT());
	int dreg = (OP >> 16) & 31;
	ABSI(dreg, src);
}

static void absi_ind(void)
{
	UINT32 src = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 31;
	ABSI(dreg, src);
}

static void absi_imm(void)
{
	UINT32 src = (INT16)OP;
	int dreg = (OP >> 16) & 31;
	ABSI(dreg, src);
}

/*-----------------------------------------------------*/

#define ADDC(dreg, src1, src2)										\
{																	\
	UINT32 _res = src1 + src2 + (IREG(TMR_ST) & CFLAG);				\
	if (!OVM || !OVERFLOW_ADD(src1,src2,_res))						\
		IREG(dreg) = _res;											\
	else															\
		IREG(dreg) = ((INT32)src1 < 0) ? 0x80000000 : 0x7fffffff;	\
	if (dreg < 8)													\
	{																\
		UINT32 tempc = src2 + (IREG(TMR_ST) & CFLAG);				\
		CLR_NZCVUF();												\
		OR_NZCV_ADD(src1,tempc,_res);								\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(dreg);										\
}

static void addc_reg(void)
{
	UINT32 src = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	ADDC(dreg, dst, src);
}

static void addc_dir(void)
{
	UINT32 src = RMEM(DIRECT());
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	ADDC(dreg, dst, src);
}

static void addc_ind(void)
{
	UINT32 src = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	ADDC(dreg, dst, src);
}

static void addc_imm(void)
{
	UINT32 src = (INT16)OP;
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	ADDC(dreg, dst, src);
}

/*-----------------------------------------------------*/

static void addf_reg(void)
{
	int dreg = (OP >> 16) & 7;
	addf(&tms32031.r[dreg], &tms32031.r[dreg], &tms32031.r[OP & 7]);
}

static void addf_dir(void)
{
	UINT32 res = RMEM(DIRECT());
	int dreg = (OP >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	addf(&tms32031.r[dreg], &tms32031.r[dreg], &tms32031.r[TMR_TEMP1]);
}

static void addf_ind(void)
{
	UINT32 res = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	addf(&tms32031.r[dreg], &tms32031.r[dreg], &tms32031.r[TMR_TEMP1]);
}

static void addf_imm(void)
{
	int dreg = (OP >> 16) & 7;
	SHORT2FP(TMR_TEMP1, OP);
	addf(&tms32031.r[dreg], &tms32031.r[dreg], &tms32031.r[TMR_TEMP1]);
}

/*-----------------------------------------------------*/

#define ADDI(dreg, src1, src2)										\
{																	\
	UINT32 _res = src1 + src2;										\
	if (!OVM || !OVERFLOW_ADD(src1,src2,_res))						\
		IREG(dreg) = _res;											\
	else															\
		IREG(dreg) = ((INT32)src1 < 0) ? 0x80000000 : 0x7fffffff;	\
	if (dreg < 8)													\
	{																\
		CLR_NZCVUF();												\
		OR_NZCV_ADD(src1,src2,_res);								\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(dreg);										\
}

static void addi_reg(void)
{
	UINT32 src = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	ADDI(dreg, dst, src);
}

static void addi_dir(void)
{
	UINT32 src = RMEM(DIRECT());
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	ADDI(dreg, dst, src);
}

static void addi_ind(void)
{
	UINT32 src = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	ADDI(dreg, dst, src);
}

static void addi_imm(void)
{
	UINT32 src = (INT16)OP;
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	ADDI(dreg, dst, src);
}

/*-----------------------------------------------------*/

#define AND(dreg, src1, src2)										\
{																	\
	UINT32 _res = (src1) & (src2);									\
	IREG(dreg) = _res;												\
	if (dreg < 8)													\
	{																\
		CLR_NZVUF();												\
		OR_NZ(_res);												\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(dreg);										\
}

static void and_reg(void)
{
	UINT32 src = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	AND(dreg, dst, src);
}

static void and_dir(void)
{
	UINT32 src = RMEM(DIRECT());
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	AND(dreg, dst, src);
}

static void and_ind(void)
{
	UINT32 src = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	AND(dreg, dst, src);
}

static void and_imm(void)
{
	UINT32 src = (UINT16)OP;
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	AND(dreg, dst, src);
}

/*-----------------------------------------------------*/

#define ANDN(dreg, src1, src2)										\
{																	\
	UINT32 _res = (src1) & ~(src2);									\
	IREG(dreg) = _res;												\
	if (dreg < 8)													\
	{																\
		CLR_NZVUF();												\
		OR_NZ(_res);												\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(dreg);										\
}

static void andn_reg(void)
{
	UINT32 src = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	ANDN(dreg, dst, src);
}

static void andn_dir(void)
{
	UINT32 src = RMEM(DIRECT());
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	ANDN(dreg, dst, src);
}

static void andn_ind(void)
{
	UINT32 src = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	ANDN(dreg, dst, src);
}

static void andn_imm(void)
{
	UINT32 src = (UINT16)OP;
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
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
	IREG(dreg) = _res;												\
	if (dreg < 8)													\
	{																\
		CLR_NZCVUF();												\
		OR_NZ(_res);												\
		if (_count < 0)												\
		{															\
			if (_count >= -32)										\
				OR_C(((INT32)src >> (-_count - 1)) & 1);			\
			else													\
				OR_C(((INT32)src >> 31) & 1);						\
		}															\
		else if (_count > 0)										\
		{															\
			if (_count <= 32)										\
				OR_C(((UINT32)src << (_count - 1)) >> 31);			\
		}															\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(dreg);										\
}

static void ash_reg(void)
{
	int dreg = (OP >> 16) & 31;
	int count = IREG(OP & 31);
	UINT32 src = IREG(dreg);
	ASH(dreg, src, count);
}

static void ash_dir(void)
{
	int dreg = (OP >> 16) & 31;
	int count = RMEM(DIRECT());
	UINT32 src = IREG(dreg);
	ASH(dreg, src, count);
}

static void ash_ind(void)
{
	int dreg = (OP >> 16) & 31;
	int count = RMEM(INDIRECT_D(OP >> 8));
	UINT32 src = IREG(dreg);
	ASH(dreg, src, count);
}

static void ash_imm(void)
{
	int dreg = (OP >> 16) & 31;
	int count = OP;
	UINT32 src = IREG(dreg);
	ASH(dreg, src, count);
}

/*-----------------------------------------------------*/

static void cmpf_reg(void)
{
	int dreg = (OP >> 16) & 7;
	subf(&tms32031.r[TMR_TEMP2], &tms32031.r[dreg], &tms32031.r[OP & 7]);
}

static void cmpf_dir(void)
{
	UINT32 res = RMEM(DIRECT());
	int dreg = (OP >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	subf(&tms32031.r[TMR_TEMP2], &tms32031.r[dreg], &tms32031.r[TMR_TEMP1]);
}

static void cmpf_ind(void)
{
	UINT32 res = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	subf(&tms32031.r[TMR_TEMP2], &tms32031.r[dreg], &tms32031.r[TMR_TEMP1]);
}

static void cmpf_imm(void)
{
	int dreg = (OP >> 16) & 7;
	SHORT2FP(TMR_TEMP1, OP);
	subf(&tms32031.r[TMR_TEMP2], &tms32031.r[dreg], &tms32031.r[TMR_TEMP1]);
}

/*-----------------------------------------------------*/

#define CMPI(src1, src2)											\
{																	\
	UINT32 _res = src1 - src2;										\
	CLR_NZCVUF();													\
	OR_NZCV_SUB(src1,src2,_res);									\
}

static void cmpi_reg(void)
{
	UINT32 src = IREG(OP & 31);
	UINT32 dst = IREG((OP >> 16) & 31);
	CMPI(dst, src);
}

static void cmpi_dir(void)
{
	UINT32 src = RMEM(DIRECT());
	UINT32 dst = IREG((OP >> 16) & 31);
	CMPI(dst, src);
}

static void cmpi_ind(void)
{
	UINT32 src = RMEM(INDIRECT_D(OP >> 8));
	UINT32 dst = IREG((OP >> 16) & 31);
	CMPI(dst, src);
}

static void cmpi_imm(void)
{
	UINT32 src = (INT16)OP;
	UINT32 dst = IREG((OP >> 16) & 31);
	CMPI(dst, src);
}

/*-----------------------------------------------------*/

static void fix_reg(void)
{
	int dreg = (OP >> 16) & 31;
	tms32031.r[dreg] = tms32031.r[OP & 7];
	float2int(&tms32031.r[dreg], dreg < 8);
}

static void fix_dir(void)
{
	UINT32 res = RMEM(DIRECT());
	int dreg = (OP >> 16) & 31;
	LONG2FP(dreg, res);
	float2int(&tms32031.r[dreg], dreg < 8);
}

static void fix_ind(void)
{
	UINT32 res = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 31;
	LONG2FP(dreg, res);
	float2int(&tms32031.r[dreg], dreg < 8);
}

static void fix_imm(void)
{
	int dreg = (OP >> 16) & 31;
	SHORT2FP(dreg, OP);
	float2int(&tms32031.r[dreg], dreg < 8);
}

/*-----------------------------------------------------*/

#define FLOAT(dreg, src)											\
{																	\
	IREG(dreg) = src;												\
	int2float(&tms32031.r[dreg]);									\
}

static void float_reg(void)
{
	UINT32 src = IREG(OP & 31);
	int dreg = (OP >> 16) & 7;
	FLOAT(dreg, src);
}

static void float_dir(void)
{
	UINT32 src = RMEM(DIRECT());
	int dreg = (OP >> 16) & 7;
	FLOAT(dreg, src);
}

static void float_ind(void)
{
	UINT32 src = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 7;
	FLOAT(dreg, src);
}

static void float_imm(void)
{
	UINT32 src = (INT16)OP;
	int dreg = (OP >> 16) & 7;
	FLOAT(dreg, src);
}

/*-----------------------------------------------------*/

static void idle(void)
{
	tms32031.is_idling = TRUE;
	IREG(TMR_ST) |= GIEFLAG;
	check_irqs();
	if (tms32031.is_idling)
		tms32031_icount = 0;
}

/*-----------------------------------------------------*/

static void lde_reg(void)
{
	int dreg = (OP >> 16) & 7;
	SET_EXPONENT(&tms32031.r[dreg], EXPONENT(&tms32031.r[OP & 7]));
	if (EXPONENT(&tms32031.r[dreg]) == -128)
		SET_MANTISSA(&tms32031.r[dreg], 0);
}

static void lde_dir(void)
{
	UINT32 res = RMEM(DIRECT());
	int dreg = (OP >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	SET_EXPONENT(&tms32031.r[dreg], EXPONENT(&tms32031.r[TMR_TEMP1]));
	if (EXPONENT(&tms32031.r[dreg]) == -128)
		SET_MANTISSA(&tms32031.r[dreg], 0);
}

static void lde_ind(void)
{
	UINT32 res = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	SET_EXPONENT(&tms32031.r[dreg], EXPONENT(&tms32031.r[TMR_TEMP1]));
	if (EXPONENT(&tms32031.r[dreg]) == -128)
		SET_MANTISSA(&tms32031.r[dreg], 0);
}

static void lde_imm(void)
{
	int dreg = (OP >> 16) & 7;
	SHORT2FP(TMR_TEMP1, OP);
	SET_EXPONENT(&tms32031.r[dreg], EXPONENT(&tms32031.r[TMR_TEMP1]));
	if (EXPONENT(&tms32031.r[dreg]) == -128)
		SET_MANTISSA(&tms32031.r[dreg], 0);
}

/*-----------------------------------------------------*/

static void ldf_reg(void)
{
	int dreg = (OP >> 16) & 7;
	tms32031.r[dreg] = tms32031.r[OP & 7];
	CLR_NZVUF();
	OR_NZF(&tms32031.r[dreg]);
}

static void ldf_dir(void)
{
	UINT32 res = RMEM(DIRECT());
	int dreg = (OP >> 16) & 7;
	LONG2FP(dreg, res);
	CLR_NZVUF();
	OR_NZF(&tms32031.r[dreg]);
}

static void ldf_ind(void)
{
	UINT32 res = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 7;
	LONG2FP(dreg, res);
	CLR_NZVUF();
	OR_NZF(&tms32031.r[dreg]);
}

static void ldf_imm(void)
{
	int dreg = (OP >> 16) & 7;
	SHORT2FP(dreg, OP);
	CLR_NZVUF();
	OR_NZF(&tms32031.r[dreg]);
}

/*-----------------------------------------------------*/

static void ldfi_dir(void) { unimplemented(); }
static void ldfi_ind(void) { unimplemented(); }

/*-----------------------------------------------------*/

#define LDI(dreg, src)												\
{																	\
	IREG(dreg) = src;												\
	if (dreg < 8)													\
	{																\
		CLR_NZVUF();												\
		OR_NZ(src);													\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(dreg);										\
}

static void ldi_reg(void)
{
	UINT32 src = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	LDI(dreg, src);
}

static void ldi_dir(void)
{
	UINT32 src = RMEM(DIRECT());
	int dreg = (OP >> 16) & 31;
	LDI(dreg, src);
}

static void ldi_ind(void)
{
	UINT32 src = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 31;
	LDI(dreg, src);
}

static void ldi_imm(void)
{
	UINT32 src = (INT16)OP;
	int dreg = (OP >> 16) & 31;
	LDI(dreg, src);
}

/*-----------------------------------------------------*/

static void ldii_dir(void) { unimplemented(); }
static void ldii_ind(void) { unimplemented(); }

/*-----------------------------------------------------*/

static void ldm_reg(void)
{
	int dreg = (OP >> 16) & 7;
	SET_MANTISSA(&tms32031.r[dreg], MANTISSA(&tms32031.r[OP & 7]));
}

static void ldm_dir(void)
{
	UINT32 res = RMEM(DIRECT());
	int dreg = (OP >> 16) & 7;
	SET_MANTISSA(&tms32031.r[dreg], res);
}

static void ldm_ind(void)
{
	UINT32 res = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 7;
	SET_MANTISSA(&tms32031.r[dreg], res);
}

static void ldm_imm(void)
{
	int dreg = (OP >> 16) & 7;
	SHORT2FP(TMR_TEMP1, OP);
	SET_MANTISSA(&tms32031.r[dreg], MANTISSA(&tms32031.r[TMR_TEMP1]));
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
	IREG(dreg) = _res;												\
	if (dreg < 8)													\
	{																\
		CLR_NZCVUF();												\
		OR_NZ(_res);												\
		if (_count < 0)												\
		{															\
			if (_count >= -32)										\
				OR_C(((UINT32)src >> (-_count - 1)) & 1);			\
		}															\
		else if (_count > 0)										\
		{															\
			if (_count <= 32)										\
				OR_C(((UINT32)src << (_count - 1)) >> 31);			\
		}															\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(dreg);										\
}

static void lsh_reg(void)
{
	int dreg = (OP >> 16) & 31;
	int count = IREG(OP & 31);
	UINT32 src = IREG(dreg);
	LSH(dreg, src, count);
}

static void lsh_dir(void)
{
	int dreg = (OP >> 16) & 31;
	int count = RMEM(DIRECT());
	UINT32 src = IREG(dreg);
	LSH(dreg, src, count);
}

static void lsh_ind(void)
{
	int dreg = (OP >> 16) & 31;
	int count = RMEM(INDIRECT_D(OP >> 8));
	UINT32 src = IREG(dreg);
	LSH(dreg, src, count);
}

static void lsh_imm(void)
{
	int dreg = (OP >> 16) & 31;
	int count = OP;
	UINT32 src = IREG(dreg);
	LSH(dreg, src, count);
}

/*-----------------------------------------------------*/

static void mpyf_reg(void)
{
	int dreg = (OP >> 16) & 31;
	mpyf(&tms32031.r[dreg], &tms32031.r[dreg], &tms32031.r[OP & 31]);
}

static void mpyf_dir(void)
{
	UINT32 res = RMEM(DIRECT());
	int dreg = (OP >> 16) & 31;
	LONG2FP(TMR_TEMP1, res);
	mpyf(&tms32031.r[dreg], &tms32031.r[dreg], &tms32031.r[TMR_TEMP1]);
}

static void mpyf_ind(void)
{
	UINT32 res = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 31;
	LONG2FP(TMR_TEMP1, res);
	mpyf(&tms32031.r[dreg], &tms32031.r[dreg], &tms32031.r[TMR_TEMP1]);
}

static void mpyf_imm(void)
{
	int dreg = (OP >> 16) & 31;
	SHORT2FP(TMR_TEMP1, OP);
	mpyf(&tms32031.r[dreg], &tms32031.r[dreg], &tms32031.r[TMR_TEMP1]);
}

/*-----------------------------------------------------*/

#define MPYI(dreg, src1, src2)										\
{																	\
	INT64 _res = (INT64)((INT32)(src1 << 8) >> 8) * (INT64)((INT32)(src2 << 8) >> 8);\
	if (!OVM || (_res >= -0x80000000 && _res <= 0x7fffffff))		\
		IREG(dreg) = _res;											\
	else															\
		IREG(dreg) = (_res < 0) ? 0x80000000 : 0x7fffffff;			\
	if (dreg < 8)													\
	{																\
		CLR_NZVUF();												\
		OR_NZ((UINT32)_res);										\
		if (_res < -(INT64)0x80000000 || _res > (INT64)0x7fffffff)	\
			IREG(TMR_ST) |= VFLAG | LVFLAG;							\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(dreg);										\
}

static void mpyi_reg(void)
{
	UINT32 src = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	MPYI(dreg, dst, src);
}

static void mpyi_dir(void)
{
	UINT32 src = RMEM(DIRECT());
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	MPYI(dreg, dst, src);
}

static void mpyi_ind(void)
{
	UINT32 src = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	MPYI(dreg, dst, src);
}

static void mpyi_imm(void)
{
	UINT32 src = (INT16)OP;
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	MPYI(dreg, dst, src);
}

/*-----------------------------------------------------*/

#define NEGB(dreg, src)												\
{																	\
	UINT32 temps = 0 - (IREG(TMR_ST) & CFLAG);						\
	UINT32 _res = temps - src;										\
	if (!OVM || !OVERFLOW_SUB(temps,src,_res))						\
		IREG(dreg) = _res;											\
	else															\
		IREG(dreg) = ((INT32)src < 0) ? 0x80000000 : 0x7fffffff;	\
	if (dreg < 8)													\
	{																\
		CLR_NZCVUF();												\
		OR_NZCV_SUB(temps,src,_res);								\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(dreg);										\
}

static void negb_reg(void)
{
	UINT32 src = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	NEGB(dreg, src);
}

static void negb_dir(void)
{
	UINT32 src = RMEM(DIRECT());
	int dreg = (OP >> 16) & 31;
	NEGB(dreg, src);
}

static void negb_ind(void)
{
	UINT32 src = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 31;
	NEGB(dreg, src);
}

static void negb_imm(void)
{
	UINT32 src = (INT16)OP;
	int dreg = (OP >> 16) & 31;
	NEGB(dreg, src);
}

/*-----------------------------------------------------*/

static void negf_reg(void)
{
	int dreg = (OP >> 16) & 7;
	negf(&tms32031.r[dreg], &tms32031.r[OP & 7]);
}

static void negf_dir(void)
{
	UINT32 res = RMEM(DIRECT());
	int dreg = (OP >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	negf(&tms32031.r[dreg], &tms32031.r[TMR_TEMP1]);
}

static void negf_ind(void)
{
	UINT32 res = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	negf(&tms32031.r[dreg], &tms32031.r[TMR_TEMP1]);
}

static void negf_imm(void)
{
	int dreg = (OP >> 16) & 7;
	SHORT2FP(TMR_TEMP1, OP);
	negf(&tms32031.r[dreg], &tms32031.r[TMR_TEMP1]);
}

/*-----------------------------------------------------*/

#define NEGI(dreg, src)												\
{																	\
	UINT32 _res = 0 - src;											\
	if (!OVM || !OVERFLOW_SUB(0,src,_res))							\
		IREG(dreg) = _res;											\
	else															\
		IREG(dreg) = ((INT32)src < 0) ? 0x80000000 : 0x7fffffff;	\
	if (dreg < 8)													\
	{																\
		CLR_NZCVUF();												\
		OR_NZCV_SUB(0,src,_res);									\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(dreg);										\
}

static void negi_reg(void)
{
	UINT32 src = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	NEGI(dreg, src);
}

static void negi_dir(void)
{
	UINT32 src = RMEM(DIRECT());
	int dreg = (OP >> 16) & 31;
	NEGI(dreg, src);
}

static void negi_ind(void)
{
	UINT32 src = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 31;
	NEGI(dreg, src);
}

static void negi_imm(void)
{
	UINT32 src = (INT16)OP;
	int dreg = (OP >> 16) & 31;
	NEGI(dreg, src);
}

/*-----------------------------------------------------*/

static void nop_reg(void)
{
}

static void nop_ind(void)
{
	RMEM(INDIRECT_D(OP >> 8));
}

/*-----------------------------------------------------*/

static void norm_reg(void)
{
	int dreg = (OP >> 16) & 7;
	norm(&tms32031.r[dreg], &tms32031.r[OP & 7]);
}

static void norm_dir(void)
{
	UINT32 res = RMEM(DIRECT());
	int dreg = (OP >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	norm(&tms32031.r[dreg], &tms32031.r[TMR_TEMP1]);
}

static void norm_ind(void)
{
	UINT32 res = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	norm(&tms32031.r[dreg], &tms32031.r[TMR_TEMP1]);
}

static void norm_imm(void)
{
	int dreg = (OP >> 16) & 7;
	SHORT2FP(TMR_TEMP1, OP);
	norm(&tms32031.r[dreg], &tms32031.r[TMR_TEMP1]);
}

/*-----------------------------------------------------*/

#define NOT(dreg, src)												\
{																	\
	UINT32 _res = ~(src);											\
	IREG(dreg) = _res;												\
	if (dreg < 8)													\
	{																\
		CLR_NZVUF();												\
		OR_NZ(_res);												\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(dreg);										\
}

static void not_reg(void)
{
	UINT32 src = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	NOT(dreg, src);
}

static void not_dir(void)
{
	UINT32 src = RMEM(DIRECT());
	int dreg = (OP >> 16) & 31;
	NOT(dreg, src);
}

static void not_ind(void)
{
	UINT32 src = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 31;
	NOT(dreg, src);
}

static void not_imm(void)
{
	UINT32 src = (UINT16)OP;
	int dreg = (OP >> 16) & 31;
	NOT(dreg, src);
}

/*-----------------------------------------------------*/

static void pop(void)
{
	int dreg = (OP >> 16) & 31;
	UINT32 val = RMEM(IREG(TMR_SP)--);
	IREG(dreg) = val;
	if (dreg < 8)
	{
		CLR_NZVUF();
		OR_NZ(val);
	}
	else if (dreg >= TMR_BK)
		update_special(dreg);
}

static void popf(void)
{
	int dreg = (OP >> 16) & 7;
	UINT32 val = RMEM(IREG(TMR_SP)--);
	LONG2FP(dreg, val);
	CLR_NZVUF();
	OR_NZF(&tms32031.r[dreg]);
}

static void push(void)
{
	WMEM(++IREG(TMR_SP), IREG((OP >> 16) & 31));
}

static void pushf(void)
{
	int dreg = (OP >> 16) & 7;
	WMEM(++IREG(TMR_SP), FP2LONG(dreg));
}

/*-----------------------------------------------------*/

#define OR(dreg, src1, src2)										\
{																	\
	UINT32 _res = (src1) | (src2);									\
	IREG(dreg) = _res;												\
	if (dreg < 8)													\
	{																\
		CLR_NZVUF();												\
		OR_NZ(_res);												\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(dreg);										\
}

static void or_reg(void)
{
	UINT32 src = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	OR(dreg, dst, src);
}

static void or_dir(void)
{
	UINT32 src = RMEM(DIRECT());
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	OR(dreg, dst, src);
}

static void or_ind(void)
{
	UINT32 src = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	OR(dreg, dst, src);
}

static void or_imm(void)
{
	UINT32 src = (UINT16)OP;
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	OR(dreg, dst, src);
}

/*-----------------------------------------------------*/

static void maxspeed(void) { unimplemented(); }

/*-----------------------------------------------------*/

#define RND(dreg)													\
{																	\
	INT32 man = FREGMAN(dreg);										\
	CLR_NVUF();														\
	if (man < 0x7fffff80)											\
	{																\
		SET_MANTISSA(&tms32031.r[dreg], ((UINT32)man + 0x80) & 0xffffff00);	\
		OR_NUF(&tms32031.r[dreg]);									\
	}																\
	else if (FREGEXP(dreg) < 127)									\
	{																\
		SET_MANTISSA(&tms32031.r[dreg], ((UINT32)man + 0x80) & 0x7fffff00);	\
		SET_EXPONENT(&tms32031.r[dreg], FREGEXP(dreg) + 1);			\
		OR_NUF(&tms32031.r[dreg]);									\
	}																\
	else															\
	{																\
		SET_MANTISSA(&tms32031.r[dreg], 0x7fffff00);				\
		IREG(TMR_ST) |= VFLAG | LVFLAG;								\
	}																\
}

static void rnd_reg(void)
{
	int sreg = OP & 7;
	int dreg = (OP >> 16) & 7;
	tms32031.r[dreg] = tms32031.r[sreg];
	RND(dreg);
}

static void rnd_dir(void)
{
	UINT32 res = RMEM(DIRECT());
	int dreg = (OP >> 16) & 7;
	LONG2FP(dreg, res);
	RND(dreg);
}

static void rnd_ind(void)
{
	UINT32 res = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 7;
	LONG2FP(dreg, res);
	RND(dreg);
}

static void rnd_imm(void)
{
	int dreg = (OP >> 16) & 7;
	SHORT2FP(dreg, OP);
	RND(dreg);
}

/*-----------------------------------------------------*/

static void rol(void)
{
	int dreg = (OP >> 16) & 31;
	UINT32 res = IREG(dreg);
	int newcflag = res >> 31;
	res = (res << 1) | newcflag;
	IREG(dreg) = res;
	if (dreg < 8)
	{
		CLR_NZCVUF();
		OR_NZ(res);
		OR_C(newcflag);
	}
	else if (dreg >= TMR_BK)
		update_special(dreg);
}

static void rolc(void)
{
	int dreg = (OP >> 16) & 31;
	UINT32 res = IREG(dreg);
	int newcflag = res >> 31;
	res = (res << 1) | (IREG(TMR_ST) & CFLAG);
	IREG(dreg) = res;
	if (dreg < 8)
	{
		CLR_NZCVUF();
		OR_NZ(res);
		OR_C(newcflag);
	}
	else if (dreg >= TMR_BK)
		update_special(dreg);
}

static void ror(void)
{
	int dreg = (OP >> 16) & 31;
	UINT32 res = IREG(dreg);
	int newcflag = res & 1;
	res = (res >> 1) | (newcflag << 31);
	IREG(dreg) = res;
	if (dreg < 8)
	{
		CLR_NZCVUF();
		OR_NZ(res);
		OR_C(newcflag);
	}
	else if (dreg >= TMR_BK)
		update_special(dreg);
}

static void rorc(void)
{
	int dreg = (OP >> 16) & 31;
	UINT32 res = IREG(dreg);
	int newcflag = res & 1;
	res = (res >> 1) | ((IREG(TMR_ST) & CFLAG) << 31);
	IREG(dreg) = res;
	if (dreg < 8)
	{
		CLR_NZCVUF();
		OR_NZ(res);
		OR_C(newcflag);
	}
	else if (dreg >= TMR_BK)
		update_special(dreg);
}

/*-----------------------------------------------------*/

static void rtps_reg(void)
{
	IREG(TMR_RC) = IREG(OP & 31);
	IREG(TMR_RS) = tms32031.pc;
	IREG(TMR_RE) = tms32031.pc;
	IREG(TMR_ST) |= RMFLAG;
	tms32031_icount -= 3*2;
	tms32031.delayed = TRUE;
}

static void rtps_dir(void)
{
	IREG(TMR_RC) = RMEM(DIRECT());
	IREG(TMR_RS) = tms32031.pc;
	IREG(TMR_RE) = tms32031.pc;
	IREG(TMR_ST) |= RMFLAG;
	tms32031_icount -= 3*2;
	tms32031.delayed = TRUE;
}

static void rtps_ind(void)
{
	IREG(TMR_RC) = RMEM(INDIRECT_D(OP >> 8));
	IREG(TMR_RS) = tms32031.pc;
	IREG(TMR_RE) = tms32031.pc;
	IREG(TMR_ST) |= RMFLAG;
	tms32031_icount -= 3*2;
	tms32031.delayed = TRUE;
}

static void rtps_imm(void)
{
	IREG(TMR_RC) = (UINT16)OP;
	IREG(TMR_RS) = tms32031.pc;
	IREG(TMR_RE) = tms32031.pc;
	IREG(TMR_ST) |= RMFLAG;
	tms32031_icount -= 3*2;
	tms32031.delayed = TRUE;
}

/*-----------------------------------------------------*/

static void stf_dir(void)
{
	WMEM(DIRECT(), FP2LONG((OP >> 16) & 7));
}

static void stf_ind(void)
{
	WMEM(INDIRECT_D(OP >> 8), FP2LONG((OP >> 16) & 7));
}

/*-----------------------------------------------------*/

static void stfi_dir(void) { unimplemented(); }
static void stfi_ind(void) { unimplemented(); }

/*-----------------------------------------------------*/

static void sti_dir(void)
{
	WMEM(DIRECT(), IREG((OP >> 16) & 31));
}

static void sti_ind(void)
{
	WMEM(INDIRECT_D(OP >> 8), IREG((OP >> 16) & 31));
}

/*-----------------------------------------------------*/

static void stii_dir(void) { unimplemented(); }
static void stii_ind(void) { unimplemented(); }

/*-----------------------------------------------------*/

static void sigi(void) { unimplemented(); }

/*-----------------------------------------------------*/

#define SUBB(dreg, src1, src2)										\
{																	\
	UINT32 temps = src1 - (IREG(TMR_ST) & CFLAG);					\
	UINT32 _res = temps - src2;										\
	if (!OVM || !OVERFLOW_SUB(temps,src2,_res))						\
		IREG(dreg) = _res;											\
	else															\
		IREG(dreg) = ((INT32)src1 < 0) ? 0x80000000 : 0x7fffffff;	\
	if (dreg < 8)													\
	{																\
		CLR_NZCVUF();												\
		OR_NZCV_SUB(temps,src2,_res);								\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(dreg);										\
}

static void subb_reg(void)
{
	UINT32 src = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBB(dreg, dst, src);
}

static void subb_dir(void)
{
	UINT32 src = RMEM(DIRECT());
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBB(dreg, dst, src);
}

static void subb_ind(void)
{
	UINT32 src = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBB(dreg, dst, src);
}

static void subb_imm(void)
{
	UINT32 src = (INT16)OP;
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBB(dreg, dst, src);
}

/*-----------------------------------------------------*/

#define SUBC(dreg, src)												\
{																	\
	UINT32 dst = IREG(dreg);										\
	if (dst >= src)													\
		IREG(dreg) = ((dst - src) << 1) | 1;						\
	else															\
		IREG(dreg) = dst << 1;										\
	if (dreg >= TMR_BK)												\
		update_special(dreg);										\
}

static void subc_reg(void)
{
	UINT32 src = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	SUBC(dreg, src);
}

static void subc_dir(void)
{
	UINT32 src = RMEM(DIRECT());
	int dreg = (OP >> 16) & 31;
	SUBC(dreg, src);
}

static void subc_ind(void)
{
	UINT32 src = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 31;
	SUBC(dreg, src);
}

static void subc_imm(void)
{
	UINT32 src = (INT16)OP;
	int dreg = (OP >> 16) & 31;
	SUBC(dreg, src);
}

/*-----------------------------------------------------*/

static void subf_reg(void)
{
	int dreg = (OP >> 16) & 7;
	subf(&tms32031.r[dreg], &tms32031.r[dreg], &tms32031.r[OP & 7]);
}

static void subf_dir(void)
{
	UINT32 res = RMEM(DIRECT());
	int dreg = (OP >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	subf(&tms32031.r[dreg], &tms32031.r[dreg], &tms32031.r[TMR_TEMP1]);
}

static void subf_ind(void)
{
	UINT32 res = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	subf(&tms32031.r[dreg], &tms32031.r[dreg], &tms32031.r[TMR_TEMP1]);
}

static void subf_imm(void)
{
	int dreg = (OP >> 16) & 7;
	SHORT2FP(TMR_TEMP1, OP);
	subf(&tms32031.r[dreg], &tms32031.r[dreg], &tms32031.r[TMR_TEMP1]);
}

/*-----------------------------------------------------*/

#define SUBI(dreg, src1, src2)										\
{																	\
	UINT32 _res = src1 - src2;										\
	if (!OVM || !OVERFLOW_SUB(src1,src2,_res))						\
		IREG(dreg) = _res;											\
	else															\
		IREG(dreg) = ((INT32)src1 < 0) ? 0x80000000 : 0x7fffffff;	\
	if (dreg < 8)													\
	{																\
		CLR_NZCVUF();												\
		OR_NZCV_SUB(src1,src2,_res);								\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(dreg);										\
}

static void subi_reg(void)
{
	UINT32 src = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBI(dreg, dst, src);
}

static void subi_dir(void)
{
	UINT32 src = RMEM(DIRECT());
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBI(dreg, dst, src);
}

static void subi_ind(void)
{
	UINT32 src = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBI(dreg, dst, src);
}

static void subi_imm(void)
{
	UINT32 src = (INT16)OP;
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBI(dreg, dst, src);
}

/*-----------------------------------------------------*/

static void subrb_reg(void)
{
	UINT32 src = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBB(dreg, src, dst);
}

static void subrb_dir(void)
{
	UINT32 src = RMEM(DIRECT());
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBB(dreg, src, dst);
}

static void subrb_ind(void)
{
	UINT32 src = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBB(dreg, src, dst);
}

static void subrb_imm(void)
{
	UINT32 src = (INT16)OP;
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBB(dreg, src, dst);
}

/*-----------------------------------------------------*/

static void subrf_reg(void)
{
	int dreg = (OP >> 16) & 7;
	subf(&tms32031.r[dreg], &tms32031.r[OP & 7], &tms32031.r[dreg]);
}

static void subrf_dir(void)
{
	UINT32 res = RMEM(DIRECT());
	int dreg = (OP >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	subf(&tms32031.r[dreg], &tms32031.r[TMR_TEMP1], &tms32031.r[dreg]);
}

static void subrf_ind(void)
{
	UINT32 res = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	subf(&tms32031.r[dreg], &tms32031.r[TMR_TEMP1], &tms32031.r[dreg]);
}

static void subrf_imm(void)
{
	int dreg = (OP >> 16) & 7;
	SHORT2FP(TMR_TEMP1, OP);
	subf(&tms32031.r[dreg], &tms32031.r[TMR_TEMP1], &tms32031.r[dreg]);
}

/*-----------------------------------------------------*/

static void subri_reg(void)
{
	UINT32 src = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBI(dreg, src, dst);
}

static void subri_dir(void)
{
	UINT32 src = RMEM(DIRECT());
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBI(dreg, src, dst);
}

static void subri_ind(void)
{
	UINT32 src = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBI(dreg, src, dst);
}

static void subri_imm(void)
{
	UINT32 src = (INT16)OP;
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBI(dreg, src, dst);
}

/*-----------------------------------------------------*/

#define TSTB(src1, src2)											\
{																	\
	UINT32 _res = (src1) & (src2);									\
	CLR_NZVUF();													\
	OR_NZ(_res);													\
}

static void tstb_reg(void)
{
	UINT32 src = IREG(OP & 31);
	UINT32 dst = IREG((OP >> 16) & 31);
	TSTB(dst, src);
}

static void tstb_dir(void)
{
	UINT32 src = RMEM(DIRECT());
	UINT32 dst = IREG((OP >> 16) & 31);
	TSTB(dst, src);
}

static void tstb_ind(void)
{
	UINT32 src = RMEM(INDIRECT_D(OP >> 8));
	UINT32 dst = IREG((OP >> 16) & 31);
	TSTB(dst, src);
}

static void tstb_imm(void)
{
	UINT32 src = (UINT16)OP;
	UINT32 dst = IREG((OP >> 16) & 31);
	TSTB(dst, src);
}

/*-----------------------------------------------------*/

#define XOR(dreg, src1, src2)										\
{																	\
	UINT32 _res = (src1) ^ (src2);									\
	IREG(dreg) = _res;												\
	if (dreg < 8)													\
	{																\
		CLR_NZVUF();												\
		OR_NZ(_res);												\
	}																\
	else if (dreg >= TMR_BK)										\
		update_special(dreg);										\
}

static void xor_reg(void)
{
	UINT32 src = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	XOR(dreg, dst, src);
}

static void xor_dir(void)
{
	UINT32 src = RMEM(DIRECT());
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	XOR(dreg, dst, src);
}

static void xor_ind(void)
{
	UINT32 src = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	XOR(dreg, dst, src);
}

static void xor_imm(void)
{
	UINT32 src = (UINT16)OP;
	int dreg = (OP >> 16) & 31;
	UINT32 dst = IREG(dreg);
	XOR(dreg, dst, src);
}

/*-----------------------------------------------------*/

static void iack_dir(void)
{
	offs_t addr = DIRECT();
	if (tms32031.iack_w)
		(*tms32031.iack_w)(ASSERT_LINE, addr);
	RMEM(addr);
	if (tms32031.iack_w)
		(*tms32031.iack_w)(CLEAR_LINE, addr);
}

static void iack_ind(void)
{
	offs_t addr = INDIRECT_D(OP >> 8);
	if (tms32031.iack_w)
		(*tms32031.iack_w)(ASSERT_LINE, addr);
	RMEM(addr);
	if (tms32031.iack_w)
		(*tms32031.iack_w)(CLEAR_LINE, addr);
}

/*-----------------------------------------------------*/

static void addc3_regreg(void)
{
	UINT32 src1 = IREG((OP >> 8) & 31);
	UINT32 src2 = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	ADDC(dreg, src1, src2);
}

static void addc3_indreg(void)
{
	UINT32 src1 = RMEM(INDIRECT_1(OP >> 8));
	UINT32 src2 = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	ADDC(dreg, src1, src2);
}

static void addc3_regind(void)
{
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	UINT32 src1 = IREG((OP >> 8) & 31);
	int dreg = (OP >> 16) & 31;
	ADDC(dreg, src1, src2);
}

static void addc3_indind(void)
{
	UINT32 src1 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	int dreg = (OP >> 16) & 31;
	UPDATE_DEF();
	ADDC(dreg, src1, src2);
}

/*-----------------------------------------------------*/

static void addf3_regreg(void)
{
	int sreg1 = (OP >> 8) & 7;
	int sreg2 = OP & 7;
	int dreg = (OP >> 16) & 7;
	addf(&tms32031.r[dreg], &tms32031.r[sreg1], &tms32031.r[sreg2]);
}

static void addf3_indreg(void)
{
	UINT32 src1 = RMEM(INDIRECT_1(OP >> 8));
	int sreg2 = OP & 7;
	int dreg = (OP >> 16) & 7;
	LONG2FP(TMR_TEMP1, src1);
	addf(&tms32031.r[dreg], &tms32031.r[TMR_TEMP1], &tms32031.r[sreg2]);
}

static void addf3_regind(void)
{
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	int sreg1 = (OP >> 8) & 7;
	int dreg = (OP >> 16) & 7;
	LONG2FP(TMR_TEMP2, src2);
	addf(&tms32031.r[dreg], &tms32031.r[sreg1], &tms32031.r[TMR_TEMP2]);
}

static void addf3_indind(void)
{
	UINT32 src1 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	int dreg = (OP >> 16) & 7;
	UPDATE_DEF();
	LONG2FP(TMR_TEMP1, src1);
	LONG2FP(TMR_TEMP2, src2);
	addf(&tms32031.r[dreg], &tms32031.r[TMR_TEMP1], &tms32031.r[TMR_TEMP2]);
}

/*-----------------------------------------------------*/

static void addi3_regreg(void)
{
	UINT32 src1 = IREG((OP >> 8) & 31);
	UINT32 src2 = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	ADDI(dreg, src1, src2);
}

static void addi3_indreg(void)
{
	UINT32 src1 = RMEM(INDIRECT_1(OP >> 8));
	UINT32 src2 = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	ADDI(dreg, src1, src2);
}

static void addi3_regind(void)
{
	/* Radikal Bikers confirms via ADDI3 AR3,*AR3++(1),R2 / SUB $0001,R2 sequence */
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	UINT32 src1 = IREG((OP >> 8) & 31);
	int dreg = (OP >> 16) & 31;
	ADDI(dreg, src1, src2);
}

static void addi3_indind(void)
{
	UINT32 src1 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	int dreg = (OP >> 16) & 31;
	UPDATE_DEF();
	ADDI(dreg, src1, src2);
}

/*-----------------------------------------------------*/

static void and3_regreg(void)
{
	UINT32 src1 = IREG((OP >> 8) & 31);
	UINT32 src2 = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	AND(dreg, src1, src2);
}

static void and3_indreg(void)
{
	UINT32 src1 = RMEM(INDIRECT_1(OP >> 8));
	UINT32 src2 = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	AND(dreg, src1, src2);
}

static void and3_regind(void)
{
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	UINT32 src1 = IREG((OP >> 8) & 31);
	int dreg = (OP >> 16) & 31;
	AND(dreg, src1, src2);
}

static void and3_indind(void)
{
	UINT32 src1 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	int dreg = (OP >> 16) & 31;
	UPDATE_DEF();
	AND(dreg, src1, src2);
}

/*-----------------------------------------------------*/

static void andn3_regreg(void)
{
	UINT32 src1 = IREG((OP >> 8) & 31);
	UINT32 src2 = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	ANDN(dreg, src1, src2);
}

static void andn3_indreg(void)
{
	UINT32 src1 = RMEM(INDIRECT_1(OP >> 8));
	UINT32 src2 = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	ANDN(dreg, src1, src2);
}

static void andn3_regind(void)
{
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	UINT32 src1 = IREG((OP >> 8) & 31);
	int dreg = (OP >> 16) & 31;
	ANDN(dreg, src1, src2);
}

static void andn3_indind(void)
{
	UINT32 src1 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	int dreg = (OP >> 16) & 31;
	UPDATE_DEF();
	ANDN(dreg, src1, src2);
}

/*-----------------------------------------------------*/

static void ash3_regreg(void)
{
	UINT32 src1 = IREG((OP >> 8) & 31);
	UINT32 src2 = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	ASH(dreg, src1, src2);
}

static void ash3_indreg(void)
{
	UINT32 src1 = RMEM(INDIRECT_1(OP >> 8));
	UINT32 src2 = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	ASH(dreg, src1, src2);
}

static void ash3_regind(void)
{
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	UINT32 src1 = IREG((OP >> 8) & 31);
	int dreg = (OP >> 16) & 31;
	ASH(dreg, src1, src2);
}

static void ash3_indind(void)
{
	UINT32 src1 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	int dreg = (OP >> 16) & 31;
	UPDATE_DEF();
	ASH(dreg, src1, src2);
}

/*-----------------------------------------------------*/

static void cmpf3_regreg(void)
{
	int sreg1 = (OP >> 8) & 7;
	int sreg2 = OP & 7;
	subf(&tms32031.r[TMR_TEMP1], &tms32031.r[sreg1], &tms32031.r[sreg2]);
}

static void cmpf3_indreg(void)
{
	UINT32 src1 = RMEM(INDIRECT_1(OP >> 8));
	int sreg2 = OP & 7;
	LONG2FP(TMR_TEMP1, src1);
	subf(&tms32031.r[TMR_TEMP1], &tms32031.r[TMR_TEMP1], &tms32031.r[sreg2]);
}

static void cmpf3_regind(void)
{
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	int sreg1 = (OP >> 8) & 7;
	LONG2FP(TMR_TEMP2, src2);
	subf(&tms32031.r[TMR_TEMP1], &tms32031.r[sreg1], &tms32031.r[TMR_TEMP2]);
}

static void cmpf3_indind(void)
{
	UINT32 src1 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	UPDATE_DEF();
	LONG2FP(TMR_TEMP1, src1);
	LONG2FP(TMR_TEMP2, src2);
	subf(&tms32031.r[TMR_TEMP1], &tms32031.r[TMR_TEMP1], &tms32031.r[TMR_TEMP2]);
}

/*-----------------------------------------------------*/

static void cmpi3_regreg(void)
{
	UINT32 src1 = IREG((OP >> 8) & 31);
	UINT32 src2 = IREG(OP & 31);
	CMPI(src1, src2);
}

static void cmpi3_indreg(void)
{
	UINT32 src1 = RMEM(INDIRECT_1(OP >> 8));
	UINT32 src2 = IREG(OP & 31);
	CMPI(src1, src2);
}

static void cmpi3_regind(void)
{
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	UINT32 src1 = IREG((OP >> 8) & 31);
	CMPI(src1, src2);
}

static void cmpi3_indind(void)
{
	UINT32 src1 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	UPDATE_DEF();
	CMPI(src1, src2);
}

/*-----------------------------------------------------*/

static void lsh3_regreg(void)
{
	UINT32 src1 = IREG((OP >> 8) & 31);
	UINT32 src2 = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	LSH(dreg, src1, src2);
}

static void lsh3_indreg(void)
{
	UINT32 src1 = RMEM(INDIRECT_1(OP >> 8));
	UINT32 src2 = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	LSH(dreg, src1, src2);
}

static void lsh3_regind(void)
{
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	UINT32 src1 = IREG((OP >> 8) & 31);
	int dreg = (OP >> 16) & 31;
	LSH(dreg, src1, src2);
}

static void lsh3_indind(void)
{
	UINT32 src1 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	int dreg = (OP >> 16) & 31;
	UPDATE_DEF();
	LSH(dreg, src1, src2);
}

/*-----------------------------------------------------*/

static void mpyf3_regreg(void)
{
	int sreg1 = (OP >> 8) & 7;
	int sreg2 = OP & 7;
	int dreg = (OP >> 16) & 7;
	mpyf(&tms32031.r[dreg], &tms32031.r[sreg1], &tms32031.r[sreg2]);
}

static void mpyf3_indreg(void)
{
	UINT32 src1 = RMEM(INDIRECT_1(OP >> 8));
	int sreg2 = OP & 7;
	int dreg = (OP >> 16) & 7;
	LONG2FP(TMR_TEMP1, src1);
	mpyf(&tms32031.r[dreg], &tms32031.r[TMR_TEMP1], &tms32031.r[sreg2]);
}

static void mpyf3_regind(void)
{
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	int sreg1 = (OP >> 8) & 7;
	int dreg = (OP >> 16) & 7;
	LONG2FP(TMR_TEMP2, src2);
	mpyf(&tms32031.r[dreg], &tms32031.r[sreg1], &tms32031.r[TMR_TEMP2]);
}

static void mpyf3_indind(void)
{
	UINT32 src1 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	int dreg = (OP >> 16) & 7;
	UPDATE_DEF();
	LONG2FP(TMR_TEMP1, src1);
	LONG2FP(TMR_TEMP2, src2);
	mpyf(&tms32031.r[dreg], &tms32031.r[TMR_TEMP1], &tms32031.r[TMR_TEMP2]);
}

/*-----------------------------------------------------*/

static void mpyi3_regreg(void)
{
	UINT32 src1 = IREG((OP >> 8) & 31);
	UINT32 src2 = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	MPYI(dreg, src1, src2);
}

static void mpyi3_indreg(void)
{
	UINT32 src1 = RMEM(INDIRECT_1(OP >> 8));
	UINT32 src2 = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	MPYI(dreg, src1, src2);
}

static void mpyi3_regind(void)
{
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	UINT32 src1 = IREG((OP >> 8) & 31);
	int dreg = (OP >> 16) & 31;
	MPYI(dreg, src1, src2);
}

static void mpyi3_indind(void)
{
	UINT32 src1 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	int dreg = (OP >> 16) & 31;
	UPDATE_DEF();
	MPYI(dreg, src1, src2);
}

/*-----------------------------------------------------*/

static void or3_regreg(void)
{
	UINT32 src1 = IREG((OP >> 8) & 31);
	UINT32 src2 = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	OR(dreg, src1, src2);
}

static void or3_indreg(void)
{
	UINT32 src1 = RMEM(INDIRECT_1(OP >> 8));
	UINT32 src2 = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	OR(dreg, src1, src2);
}

static void or3_regind(void)
{
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	UINT32 src1 = IREG((OP >> 8) & 31);
	int dreg = (OP >> 16) & 31;
	OR(dreg, src1, src2);
}

static void or3_indind(void)
{
	UINT32 src1 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	int dreg = (OP >> 16) & 31;
	UPDATE_DEF();
	OR(dreg, src1, src2);
}

/*-----------------------------------------------------*/

static void subb3_regreg(void)
{
	UINT32 src1 = IREG((OP >> 8) & 31);
	UINT32 src2 = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	SUBB(dreg, src1, src2);
}

static void subb3_indreg(void)
{
	UINT32 src1 = RMEM(INDIRECT_1(OP >> 8));
	UINT32 src2 = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	SUBB(dreg, src1, src2);
}

static void subb3_regind(void)
{
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	UINT32 src1 = IREG((OP >> 8) & 31);
	int dreg = (OP >> 16) & 31;
	SUBB(dreg, src1, src2);
}

static void subb3_indind(void)
{
	UINT32 src1 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	int dreg = (OP >> 16) & 31;
	UPDATE_DEF();
	SUBB(dreg, src1, src2);
}

/*-----------------------------------------------------*/

static void subf3_regreg(void)
{
	int sreg1 = (OP >> 8) & 7;
	int sreg2 = OP & 7;
	int dreg = (OP >> 16) & 7;
	subf(&tms32031.r[dreg], &tms32031.r[sreg1], &tms32031.r[sreg2]);
}

static void subf3_indreg(void)
{
	UINT32 src1 = RMEM(INDIRECT_1(OP >> 8));
	int sreg2 = OP & 7;
	int dreg = (OP >> 16) & 7;
	LONG2FP(TMR_TEMP1, src1);
	subf(&tms32031.r[dreg], &tms32031.r[TMR_TEMP1], &tms32031.r[sreg2]);
}

static void subf3_regind(void)
{
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	int sreg1 = (OP >> 8) & 7;
	int dreg = (OP >> 16) & 7;
	LONG2FP(TMR_TEMP2, src2);
	subf(&tms32031.r[dreg], &tms32031.r[sreg1], &tms32031.r[TMR_TEMP2]);
}

static void subf3_indind(void)
{
	UINT32 src1 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	int dreg = (OP >> 16) & 7;
	UPDATE_DEF();
	LONG2FP(TMR_TEMP1, src1);
	LONG2FP(TMR_TEMP2, src2);
	subf(&tms32031.r[dreg], &tms32031.r[TMR_TEMP1], &tms32031.r[TMR_TEMP2]);
}

/*-----------------------------------------------------*/

static void subi3_regreg(void)
{
	UINT32 src1 = IREG((OP >> 8) & 31);
	UINT32 src2 = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	SUBI(dreg, src1, src2);
}

static void subi3_indreg(void)
{
	UINT32 src1 = RMEM(INDIRECT_1(OP >> 8));
	UINT32 src2 = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	SUBI(dreg, src1, src2);
}

static void subi3_regind(void)
{
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	UINT32 src1 = IREG((OP >> 8) & 31);
	int dreg = (OP >> 16) & 31;
	SUBI(dreg, src1, src2);
}

static void subi3_indind(void)
{
	UINT32 src1 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	int dreg = (OP >> 16) & 31;
	UPDATE_DEF();
	SUBI(dreg, src1, src2);
}

/*-----------------------------------------------------*/

static void tstb3_regreg(void)
{
	UINT32 src1 = IREG((OP >> 8) & 31);
	UINT32 src2 = IREG(OP & 31);
	TSTB(src1, src2);
}

static void tstb3_indreg(void)
{
	UINT32 src1 = RMEM(INDIRECT_1(OP >> 8));
	UINT32 src2 = IREG(OP & 31);
	TSTB(src1, src2);
}

static void tstb3_regind(void)
{
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	UINT32 src1 = IREG((OP >> 8) & 31);
	TSTB(src1, src2);
}

static void tstb3_indind(void)
{
	UINT32 src1 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	UPDATE_DEF();
	TSTB(src1, src2);
}

/*-----------------------------------------------------*/

static void xor3_regreg(void)
{
	UINT32 src1 = IREG((OP >> 8) & 31);
	UINT32 src2 = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	XOR(dreg, src1, src2);
}

static void xor3_indreg(void)
{
	UINT32 src1 = RMEM(INDIRECT_1(OP >> 8));
	UINT32 src2 = IREG(OP & 31);
	int dreg = (OP >> 16) & 31;
	XOR(dreg, src1, src2);
}

static void xor3_regind(void)
{
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	UINT32 src1 = IREG((OP >> 8) & 31);
	int dreg = (OP >> 16) & 31;
	XOR(dreg, src1, src2);
}

static void xor3_indind(void)
{
	UINT32 src1 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(OP));
	int dreg = (OP >> 16) & 31;
	UPDATE_DEF();
	XOR(dreg, src1, src2);
}

/*-----------------------------------------------------*/

static void ldfu_reg(void)
{
	tms32031.r[(OP >> 16) & 7] = tms32031.r[OP & 7];
}

static void ldfu_dir(void)
{
	UINT32 res = RMEM(DIRECT());
	int dreg = (OP >> 16) & 7;
	LONG2FP(dreg, res);
}

static void ldfu_ind(void)
{
	UINT32 res = RMEM(INDIRECT_D(OP >> 8));
	int dreg = (OP >> 16) & 7;
	LONG2FP(dreg, res);
}

static void ldfu_imm(void)
{
	int dreg = (OP >> 16) & 7;
	SHORT2FP(dreg, OP);
}

/*-----------------------------------------------------*/

static void ldflo_reg(void)
{
	if (CONDITION_LO)
		tms32031.r[(OP >> 16) & 7] = tms32031.r[OP & 7];
}

static void ldflo_dir(void)
{
	if (CONDITION_LO)
	{
		UINT32 res = RMEM(DIRECT());
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

static void ldflo_ind(void)
{
	if (CONDITION_LO)
	{
		UINT32 res = RMEM(INDIRECT_D(OP >> 8));
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(OP >> 8);
}

static void ldflo_imm(void)
{
	if (CONDITION_LO)
	{
		int dreg = (OP >> 16) & 7;
		SHORT2FP(dreg, OP);
	}
}

/*-----------------------------------------------------*/

static void ldfls_reg(void)
{
	if (CONDITION_LS)
		tms32031.r[(OP >> 16) & 7] = tms32031.r[OP & 7];
}

static void ldfls_dir(void)
{
	if (CONDITION_LS)
	{
		UINT32 res = RMEM(DIRECT());
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

static void ldfls_ind(void)
{
	if (CONDITION_LS)
	{
		UINT32 res = RMEM(INDIRECT_D(OP >> 8));
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(OP >> 8);
}

static void ldfls_imm(void)
{
	if (CONDITION_LS)
	{
		int dreg = (OP >> 16) & 7;
		SHORT2FP(dreg, OP);
	}
}

/*-----------------------------------------------------*/

static void ldfhi_reg(void)
{
	if (CONDITION_HI)
		tms32031.r[(OP >> 16) & 7] = tms32031.r[OP & 7];
}

static void ldfhi_dir(void)
{
	if (CONDITION_HI)
	{
		UINT32 res = RMEM(DIRECT());
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

static void ldfhi_ind(void)
{
	if (CONDITION_HI)
	{
		UINT32 res = RMEM(INDIRECT_D(OP >> 8));
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(OP >> 8);
}

static void ldfhi_imm(void)
{
	if (CONDITION_HI)
	{
		int dreg = (OP >> 16) & 7;
		SHORT2FP(dreg, OP);
	}
}

/*-----------------------------------------------------*/

static void ldfhs_reg(void)
{
	if (CONDITION_HS)
		tms32031.r[(OP >> 16) & 7] = tms32031.r[OP & 7];
}

static void ldfhs_dir(void)
{
	if (CONDITION_HS)
	{
		UINT32 res = RMEM(DIRECT());
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

static void ldfhs_ind(void)
{
	if (CONDITION_HS)
	{
		UINT32 res = RMEM(INDIRECT_D(OP >> 8));
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(OP >> 8);
}

static void ldfhs_imm(void)
{
	if (CONDITION_HS)
	{
		int dreg = (OP >> 16) & 7;
		SHORT2FP(dreg, OP);
	}
}

/*-----------------------------------------------------*/

static void ldfeq_reg(void)
{
	if (CONDITION_EQ)
		tms32031.r[(OP >> 16) & 7] = tms32031.r[OP & 7];
}

static void ldfeq_dir(void)
{
	if (CONDITION_EQ)
	{
		UINT32 res = RMEM(DIRECT());
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

static void ldfeq_ind(void)
{
	if (CONDITION_EQ)
	{
		UINT32 res = RMEM(INDIRECT_D(OP >> 8));
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(OP >> 8);
}

static void ldfeq_imm(void)
{
	if (CONDITION_EQ)
	{
		int dreg = (OP >> 16) & 7;
		SHORT2FP(dreg, OP);
	}
}

/*-----------------------------------------------------*/

static void ldfne_reg(void)
{
	if (CONDITION_NE)
		tms32031.r[(OP >> 16) & 7] = tms32031.r[OP & 7];
}

static void ldfne_dir(void)
{
	if (CONDITION_NE)
	{
		UINT32 res = RMEM(DIRECT());
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

static void ldfne_ind(void)
{
	if (CONDITION_NE)
	{
		UINT32 res = RMEM(INDIRECT_D(OP >> 8));
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(OP >> 8);
}

static void ldfne_imm(void)
{
	if (CONDITION_NE)
	{
		int dreg = (OP >> 16) & 7;
		SHORT2FP(dreg, OP);
	}
}

/*-----------------------------------------------------*/

static void ldflt_reg(void)
{
	if (CONDITION_LT)
		tms32031.r[(OP >> 16) & 7] = tms32031.r[OP & 7];
}

static void ldflt_dir(void)
{
	if (CONDITION_LT)
	{
		UINT32 res = RMEM(DIRECT());
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

static void ldflt_ind(void)
{
	if (CONDITION_LT)
	{
		UINT32 res = RMEM(INDIRECT_D(OP >> 8));
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(OP >> 8);
}

static void ldflt_imm(void)
{
	if (CONDITION_LT)
	{
		int dreg = (OP >> 16) & 7;
		SHORT2FP(dreg, OP);
	}
}

/*-----------------------------------------------------*/

static void ldfle_reg(void)
{
	if (CONDITION_LE)
		tms32031.r[(OP >> 16) & 7] = tms32031.r[OP & 7];
}

static void ldfle_dir(void)
{
	if (CONDITION_LE)
	{
		UINT32 res = RMEM(DIRECT());
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

static void ldfle_ind(void)
{
	if (CONDITION_LE)
	{
		UINT32 res = RMEM(INDIRECT_D(OP >> 8));
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(OP >> 8);
}

static void ldfle_imm(void)
{
	if (CONDITION_LE)
	{
		int dreg = (OP >> 16) & 7;
		SHORT2FP(dreg, OP);
	}
}

/*-----------------------------------------------------*/

static void ldfgt_reg(void)
{
	if (CONDITION_GT)
		tms32031.r[(OP >> 16) & 7] = tms32031.r[OP & 7];
}

static void ldfgt_dir(void)
{
	if (CONDITION_GT)
	{
		UINT32 res = RMEM(DIRECT());
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

static void ldfgt_ind(void)
{
	if (CONDITION_GT)
	{
		UINT32 res = RMEM(INDIRECT_D(OP >> 8));
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(OP >> 8);
}

static void ldfgt_imm(void)
{
	if (CONDITION_GT)
	{
		int dreg = (OP >> 16) & 7;
		SHORT2FP(dreg, OP);
	}
}

/*-----------------------------------------------------*/

static void ldfge_reg(void)
{
	if (CONDITION_GE)
		tms32031.r[(OP >> 16) & 7] = tms32031.r[OP & 7];
}

static void ldfge_dir(void)
{
	if (CONDITION_GE)
	{
		UINT32 res = RMEM(DIRECT());
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

static void ldfge_ind(void)
{
	if (CONDITION_GE)
	{
		UINT32 res = RMEM(INDIRECT_D(OP >> 8));
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(OP >> 8);
}

static void ldfge_imm(void)
{
	if (CONDITION_GE)
	{
		int dreg = (OP >> 16) & 7;
		SHORT2FP(dreg, OP);
	}
}

/*-----------------------------------------------------*/

static void ldfnv_reg(void)
{
	if (CONDITION_NV)
		tms32031.r[(OP >> 16) & 7] = tms32031.r[OP & 7];
}

static void ldfnv_dir(void)
{
	if (CONDITION_NV)
	{
		UINT32 res = RMEM(DIRECT());
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

static void ldfnv_ind(void)
{
	if (CONDITION_NV)
	{
		UINT32 res = RMEM(INDIRECT_D(OP >> 8));
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(OP >> 8);
}

static void ldfnv_imm(void)
{
	if (CONDITION_NV)
	{
		int dreg = (OP >> 16) & 7;
		SHORT2FP(dreg, OP);
	}
}

/*-----------------------------------------------------*/

static void ldfv_reg(void)
{
	if (CONDITION_V)
		tms32031.r[(OP >> 16) & 7] = tms32031.r[OP & 7];
}

static void ldfv_dir(void)
{
	if (CONDITION_V)
	{
		UINT32 res = RMEM(DIRECT());
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

static void ldfv_ind(void)
{
	if (CONDITION_V)
	{
		UINT32 res = RMEM(INDIRECT_D(OP >> 8));
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(OP >> 8);
}

static void ldfv_imm(void)
{
	if (CONDITION_V)
	{
		int dreg = (OP >> 16) & 7;
		SHORT2FP(dreg, OP);
	}
}

/*-----------------------------------------------------*/

static void ldfnuf_reg(void)
{
	if (CONDITION_NUF)
		tms32031.r[(OP >> 16) & 7] = tms32031.r[OP & 7];
}

static void ldfnuf_dir(void)
{
	if (CONDITION_NUF)
	{
		UINT32 res = RMEM(DIRECT());
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

static void ldfnuf_ind(void)
{
	if (CONDITION_NUF)
	{
		UINT32 res = RMEM(INDIRECT_D(OP >> 8));
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(OP >> 8);
}

static void ldfnuf_imm(void)
{
	if (CONDITION_NUF)
	{
		int dreg = (OP >> 16) & 7;
		SHORT2FP(dreg, OP);
	}
}

/*-----------------------------------------------------*/

static void ldfuf_reg(void)
{
	if (CONDITION_UF)
		tms32031.r[(OP >> 16) & 7] = tms32031.r[OP & 7];
}

static void ldfuf_dir(void)
{
	if (CONDITION_UF)
	{
		UINT32 res = RMEM(DIRECT());
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

static void ldfuf_ind(void)
{
	if (CONDITION_UF)
	{
		UINT32 res = RMEM(INDIRECT_D(OP >> 8));
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(OP >> 8);
}

static void ldfuf_imm(void)
{
	if (CONDITION_UF)
	{
		int dreg = (OP >> 16) & 7;
		SHORT2FP(dreg, OP);
	}
}

/*-----------------------------------------------------*/

static void ldfnlv_reg(void)
{
	if (CONDITION_NLV)
		tms32031.r[(OP >> 16) & 7] = tms32031.r[OP & 7];
}

static void ldfnlv_dir(void)
{
	if (CONDITION_NLV)
	{
		UINT32 res = RMEM(DIRECT());
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

static void ldfnlv_ind(void)
{
	if (CONDITION_NLV)
	{
		UINT32 res = RMEM(INDIRECT_D(OP >> 8));
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(OP >> 8);
}

static void ldfnlv_imm(void)
{
	if (CONDITION_NLV)
	{
		int dreg = (OP >> 16) & 7;
		SHORT2FP(dreg, OP);
	}
}

/*-----------------------------------------------------*/

static void ldflv_reg(void)
{
	if (CONDITION_LV)
		tms32031.r[(OP >> 16) & 7] = tms32031.r[OP & 7];
}

static void ldflv_dir(void)
{
	if (CONDITION_LV)
	{
		UINT32 res = RMEM(DIRECT());
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

static void ldflv_ind(void)
{
	if (CONDITION_LV)
	{
		UINT32 res = RMEM(INDIRECT_D(OP >> 8));
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(OP >> 8);
}

static void ldflv_imm(void)
{
	if (CONDITION_LV)
	{
		int dreg = (OP >> 16) & 7;
		SHORT2FP(dreg, OP);
	}
}

/*-----------------------------------------------------*/

static void ldfnluf_reg(void)
{
	if (CONDITION_NLUF)
		tms32031.r[(OP >> 16) & 7] = tms32031.r[OP & 7];
}

static void ldfnluf_dir(void)
{
	if (CONDITION_NLUF)
	{
		UINT32 res = RMEM(DIRECT());
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

static void ldfnluf_ind(void)
{
	if (CONDITION_NLUF)
	{
		UINT32 res = RMEM(INDIRECT_D(OP >> 8));
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(OP >> 8);
}

static void ldfnluf_imm(void)
{
	if (CONDITION_NLUF)
	{
		int dreg = (OP >> 16) & 7;
		SHORT2FP(dreg, OP);
	}
}

/*-----------------------------------------------------*/

static void ldfluf_reg(void)
{
	if (CONDITION_LUF)
		tms32031.r[(OP >> 16) & 7] = tms32031.r[OP & 7];
}

static void ldfluf_dir(void)
{
	if (CONDITION_LUF)
	{
		UINT32 res = RMEM(DIRECT());
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

static void ldfluf_ind(void)
{
	if (CONDITION_LUF)
	{
		UINT32 res = RMEM(INDIRECT_D(OP >> 8));
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(OP >> 8);
}

static void ldfluf_imm(void)
{
	if (CONDITION_LUF)
	{
		int dreg = (OP >> 16) & 7;
		SHORT2FP(dreg, OP);
	}
}

/*-----------------------------------------------------*/

static void ldfzuf_reg(void)
{
	if (CONDITION_ZUF)
		tms32031.r[(OP >> 16) & 7] = tms32031.r[OP & 7];
}

static void ldfzuf_dir(void)
{
	if (CONDITION_ZUF)
	{
		UINT32 res = RMEM(DIRECT());
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

static void ldfzuf_ind(void)
{
	if (CONDITION_ZUF)
	{
		UINT32 res = RMEM(INDIRECT_D(OP >> 8));
		int dreg = (OP >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(OP >> 8);
}

static void ldfzuf_imm(void)
{
	if (CONDITION_ZUF)
	{
		int dreg = (OP >> 16) & 7;
		SHORT2FP(dreg, OP);
	}
}

/*-----------------------------------------------------*/

static void ldiu_reg(void)
{
	int dreg = (OP >> 16) & 31;
	IREG(dreg) = IREG(OP & 31);
	if (dreg >= TMR_BK)
		update_special(dreg);
}

static void ldiu_dir(void)
{
	int dreg = (OP >> 16) & 31;
	IREG(dreg) = RMEM(DIRECT());
	if (dreg >= TMR_BK)
		update_special(dreg);
}

static void ldiu_ind(void)
{
	int dreg = (OP >> 16) & 31;
	IREG(dreg) = RMEM(INDIRECT_D(OP >> 8));
	if (dreg >= TMR_BK)
		update_special(dreg);
}

static void ldiu_imm(void)
{
	int dreg = (OP >> 16) & 31;
	IREG(dreg) = (INT16)OP;
	if (dreg >= TMR_BK)
		update_special(dreg);
}

/*-----------------------------------------------------*/

static void ldilo_reg(void)
{
	if (CONDITION_LO)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = IREG(OP & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldilo_dir(void)
{
	UINT32 val = RMEM(DIRECT());
	if (CONDITION_LO)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldilo_ind(void)
{
	UINT32 val = RMEM(INDIRECT_D(OP >> 8));
	if (CONDITION_LO)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldilo_imm(void)
{
	if (CONDITION_LO)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = (INT16)OP;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

static void ldils_reg(void)
{
	if (CONDITION_LS)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = IREG(OP & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldils_dir(void)
{
	UINT32 val = RMEM(DIRECT());
	if (CONDITION_LS)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldils_ind(void)
{
	UINT32 val = RMEM(INDIRECT_D(OP >> 8));
	if (CONDITION_LS)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldils_imm(void)
{
	if (CONDITION_LS)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = (INT16)OP;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

static void ldihi_reg(void)
{
	if (CONDITION_HI)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = IREG(OP & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldihi_dir(void)
{
	UINT32 val = RMEM(DIRECT());
	if (CONDITION_HI)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldihi_ind(void)
{
	UINT32 val = RMEM(INDIRECT_D(OP >> 8));
	if (CONDITION_HI)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldihi_imm(void)
{
	if (CONDITION_HI)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = (INT16)OP;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

static void ldihs_reg(void)
{
	if (CONDITION_HS)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = IREG(OP & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldihs_dir(void)
{
	UINT32 val = RMEM(DIRECT());
	if (CONDITION_HS)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldihs_ind(void)
{
	UINT32 val = RMEM(INDIRECT_D(OP >> 8));
	if (CONDITION_HS)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldihs_imm(void)
{
	if (CONDITION_HS)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = (INT16)OP;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

static void ldieq_reg(void)
{
	if (CONDITION_EQ)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = IREG(OP & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldieq_dir(void)
{
	UINT32 val = RMEM(DIRECT());
	if (CONDITION_EQ)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldieq_ind(void)
{
	UINT32 val = RMEM(INDIRECT_D(OP >> 8));
	if (CONDITION_EQ)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldieq_imm(void)
{
	if (CONDITION_EQ)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = (INT16)OP;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

static void ldine_reg(void)
{
	if (CONDITION_NE)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = IREG(OP & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldine_dir(void)
{
	UINT32 val = RMEM(DIRECT());
	if (CONDITION_NE)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldine_ind(void)
{
	UINT32 val = RMEM(INDIRECT_D(OP >> 8));
	if (CONDITION_NE)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldine_imm(void)
{
	if (CONDITION_NE)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = (INT16)OP;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

static void ldilt_reg(void)
{
	if (CONDITION_LT)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = IREG(OP & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldilt_dir(void)
{
	UINT32 val = RMEM(DIRECT());
	if (CONDITION_LT)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldilt_ind(void)
{
	UINT32 val = RMEM(INDIRECT_D(OP >> 8));
	if (CONDITION_LT)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldilt_imm(void)
{
	if (CONDITION_LT)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = (INT16)OP;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

static void ldile_reg(void)
{
	if (CONDITION_LE)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = IREG(OP & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldile_dir(void)
{
	UINT32 val = RMEM(DIRECT());
	if (CONDITION_LE)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldile_ind(void)
{
	UINT32 val = RMEM(INDIRECT_D(OP >> 8));
	if (CONDITION_LE)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldile_imm(void)
{
	if (CONDITION_LE)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = (INT16)OP;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

static void ldigt_reg(void)
{
	if (CONDITION_GT)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = IREG(OP & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldigt_dir(void)
{
	UINT32 val = RMEM(DIRECT());
	if (CONDITION_GT)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldigt_ind(void)
{
	UINT32 val = RMEM(INDIRECT_D(OP >> 8));
	if (CONDITION_GT)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldigt_imm(void)
{
	if (CONDITION_GT)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = (INT16)OP;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

static void ldige_reg(void)
{
	if (CONDITION_GE)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = IREG(OP & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldige_dir(void)
{
	UINT32 val = RMEM(DIRECT());
	if (CONDITION_GE)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldige_ind(void)
{
	UINT32 val = RMEM(INDIRECT_D(OP >> 8));
	if (CONDITION_GE)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldige_imm(void)
{
	if (CONDITION_GE)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = (INT16)OP;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

static void ldinv_reg(void)
{
	if (CONDITION_NV)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = IREG(OP & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldinv_dir(void)
{
	UINT32 val = RMEM(DIRECT());
	if (CONDITION_NV)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldinv_ind(void)
{
	UINT32 val = RMEM(INDIRECT_D(OP >> 8));
	if (CONDITION_NV)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldinv_imm(void)
{
	if (CONDITION_NV)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = (INT16)OP;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

static void ldiuf_reg(void)
{
	if (CONDITION_UF)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = IREG(OP & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldiuf_dir(void)
{
	UINT32 val = RMEM(DIRECT());
	if (CONDITION_UF)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldiuf_ind(void)
{
	UINT32 val = RMEM(INDIRECT_D(OP >> 8));
	if (CONDITION_UF)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldiuf_imm(void)
{
	if (CONDITION_UF)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = (INT16)OP;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

static void ldinuf_reg(void)
{
	if (CONDITION_NUF)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = IREG(OP & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldinuf_dir(void)
{
	UINT32 val = RMEM(DIRECT());
	if (CONDITION_NUF)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldinuf_ind(void)
{
	UINT32 val = RMEM(INDIRECT_D(OP >> 8));
	if (CONDITION_NUF)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldinuf_imm(void)
{
	if (CONDITION_NUF)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = (INT16)OP;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

static void ldiv_reg(void)
{
	if (CONDITION_V)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = IREG(OP & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldiv_dir(void)
{
	UINT32 val = RMEM(DIRECT());
	if (CONDITION_V)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldiv_ind(void)
{
	UINT32 val = RMEM(INDIRECT_D(OP >> 8));
	if (CONDITION_V)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldiv_imm(void)
{
	if (CONDITION_V)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = (INT16)OP;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

static void ldinlv_reg(void)
{
	if (CONDITION_NLV)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = IREG(OP & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldinlv_dir(void)
{
	UINT32 val = RMEM(DIRECT());
	if (CONDITION_NLV)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldinlv_ind(void)
{
	UINT32 val = RMEM(INDIRECT_D(OP >> 8));
	if (CONDITION_NLV)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldinlv_imm(void)
{
	if (CONDITION_NLV)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = (INT16)OP;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

static void ldilv_reg(void)
{
	if (CONDITION_LV)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = IREG(OP & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldilv_dir(void)
{
	UINT32 val = RMEM(DIRECT());
	if (CONDITION_LV)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldilv_ind(void)
{
	UINT32 val = RMEM(INDIRECT_D(OP >> 8));
	if (CONDITION_LV)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldilv_imm(void)
{
	if (CONDITION_LV)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = (INT16)OP;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

static void ldinluf_reg(void)
{
	if (CONDITION_NLUF)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = IREG(OP & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldinluf_dir(void)
{
	UINT32 val = RMEM(DIRECT());
	if (CONDITION_NLUF)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldinluf_ind(void)
{
	UINT32 val = RMEM(INDIRECT_D(OP >> 8));
	if (CONDITION_NLUF)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldinluf_imm(void)
{
	if (CONDITION_NLUF)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = (INT16)OP;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

static void ldiluf_reg(void)
{
	if (CONDITION_LUF)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = IREG(OP & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldiluf_dir(void)
{
	UINT32 val = RMEM(DIRECT());
	if (CONDITION_LUF)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldiluf_ind(void)
{
	UINT32 val = RMEM(INDIRECT_D(OP >> 8));
	if (CONDITION_LUF)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldiluf_imm(void)
{
	if (CONDITION_LUF)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = (INT16)OP;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

static void ldizuf_reg(void)
{
	if (CONDITION_ZUF)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = IREG(OP & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldizuf_dir(void)
{
	UINT32 val = RMEM(DIRECT());
	if (CONDITION_ZUF)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldizuf_ind(void)
{
	UINT32 val = RMEM(INDIRECT_D(OP >> 8));
	if (CONDITION_ZUF)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

static void ldizuf_imm(void)
{
	if (CONDITION_ZUF)
	{
		int dreg = (OP >> 16) & 31;
		IREG(dreg) = (INT16)OP;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

INLINE void execute_delayed(UINT32 newpc)
{
	tms32031.delayed = TRUE;

	execute_one();
	execute_one();
	execute_one();

	tms32031.pc = newpc;
	UPDATEPC(tms32031.pc);

	tms32031.delayed = FALSE;
	if (tms32031.irq_pending)
	{
		tms32031.irq_pending = FALSE;
		check_irqs();
	}
}

/*-----------------------------------------------------*/

static void br_imm(void)
{
	tms32031.pc = OP & 0xffffff;
	UPDATEPC(tms32031.pc);
	tms32031_icount -= 3*2;
}

static void brd_imm(void)
{
	execute_delayed(OP & 0xffffff);
}

/*-----------------------------------------------------*/

static void call_imm(void)
{
	WMEM(++IREG(TMR_SP), tms32031.pc);
	tms32031.pc = OP & 0xffffff;
	UPDATEPC(tms32031.pc);
	tms32031_icount -= 3*2;
}

/*-----------------------------------------------------*/

static void rptb_imm(void)
{
	IREG(TMR_RS) = tms32031.pc;
	IREG(TMR_RE) = OP & 0xffffff;
	IREG(TMR_ST) |= RMFLAG;
	tms32031_icount -= 3*2;
}

/*-----------------------------------------------------*/

static void swi(void) { unimplemented(); }

/*-----------------------------------------------------*/

static void brc_reg(void)
{
	if (condition(OP >> 16))
	{
		tms32031.pc = IREG(OP & 31);
		UPDATEPC(tms32031.pc);
		tms32031_icount -= 3*2;
	}
}

static void brcd_reg(void)
{
	if (condition(OP >> 16))
		execute_delayed(IREG(OP & 31));
}

static void brc_imm(void)
{
	if (condition(OP >> 16))
	{
		tms32031.pc += (INT16)OP;
		UPDATEPC(tms32031.pc);
		tms32031_icount -= 3*2;
	}
}

static void brcd_imm(void)
{
	if (condition(OP >> 16))
		execute_delayed(tms32031.pc + 2 + (INT16)OP);
}

/*-----------------------------------------------------*/

static void dbc_reg(void)
{
	int reg = TMR_AR0 + ((OP >> 22) & 7);
	int res = (IREG(reg) - 1) & 0xffffff;
	IREG(reg) = res | (IREG(reg) & 0xff000000);
	if (condition(OP >> 16) && !(res & 0x800000))
	{
		tms32031.pc = IREG(OP & 31);
		UPDATEPC(tms32031.pc);
		tms32031_icount -= 3*2;
	}
}

static void dbcd_reg(void)
{
	int reg = TMR_AR0 + ((OP >> 22) & 7);
	int res = (IREG(reg) - 1) & 0xffffff;
	IREG(reg) = res | (IREG(reg) & 0xff000000);
	if (condition(OP >> 16) && !(res & 0x800000))
		execute_delayed(IREG(OP & 31));
}

static void dbc_imm(void)
{
	int reg = TMR_AR0 + ((OP >> 22) & 7);
	int res = (IREG(reg) - 1) & 0xffffff;
	IREG(reg) = res | (IREG(reg) & 0xff000000);
	if (condition(OP >> 16) && !(res & 0x800000))
	{
		tms32031.pc += (INT16)OP;
		UPDATEPC(tms32031.pc);
		tms32031_icount -= 3*2;
	}
}

static void dbcd_imm(void)
{
	int reg = TMR_AR0 + ((OP >> 22) & 7);
	int res = (IREG(reg) - 1) & 0xffffff;
	IREG(reg) = res | (IREG(reg) & 0xff000000);
	if (condition(OP >> 16) && !(res & 0x800000))
		execute_delayed(tms32031.pc + 2 + (INT16)OP);
}

/*-----------------------------------------------------*/

static void callc_reg(void)
{
	if (condition(OP >> 16))
	{
		WMEM(++IREG(TMR_SP), tms32031.pc);
		tms32031.pc = IREG(OP & 31);
		UPDATEPC(tms32031.pc);
		tms32031_icount -= 3*2;
	}
}

static void callc_imm(void)
{
	if (condition(OP >> 16))
	{
		WMEM(++IREG(TMR_SP), tms32031.pc);
		tms32031.pc += (INT16)OP;
		UPDATEPC(tms32031.pc);
		tms32031_icount -= 3*2;
	}
}

/*-----------------------------------------------------*/

static void trap(int trapnum)
{
	WMEM(++IREG(TMR_SP), tms32031.pc);
	IREG(TMR_ST) &= ~GIEFLAG;
	if (tms32031.is_32032)
		tms32031.pc = RMEM(((IREG(TMR_IF) >> 16) << 8) + trapnum);
	else if (tms32031.mcu_mode)
		tms32031.pc = 0x809fc0 + trapnum;
	else
		tms32031.pc = RMEM(trapnum);
	UPDATEPC(tms32031.pc);
	tms32031_icount -= 4*2;
}

static void trapc(void)
{
	if (condition(OP >> 16))
		trap(OP & 0x3f);
}

/*-----------------------------------------------------*/

static void retic_reg(void)
{
	if (condition(OP >> 16))
	{
		tms32031.pc = RMEM(IREG(TMR_SP)--);
		UPDATEPC(tms32031.pc);
		IREG(TMR_ST) |= GIEFLAG;
		tms32031_icount -= 3*2;
		check_irqs();
	}
}

static void retsc_reg(void)
{
	if (condition(OP >> 16))
	{
		tms32031.pc = RMEM(IREG(TMR_SP)--);
		UPDATEPC(tms32031.pc);
		tms32031_icount -= 3*2;
	}
}

/*-----------------------------------------------------*/

static void mpyaddf_0(void)
{
	/* src3 * src4, src1 + src2 */
	UINT32 src3 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(OP));
	LONG2FP(TMR_TEMP1, src3);
	LONG2FP(TMR_TEMP2, src4);
	mpyf(&tms32031.r[TMR_TEMP3], &tms32031.r[TMR_TEMP1], &tms32031.r[TMR_TEMP2]);
	addf(&tms32031.r[((OP >> 22) & 1) | 2], &tms32031.r[(OP >> 19) & 7], &tms32031.r[(OP >> 16) & 7]);
	tms32031.r[(OP >> 23) & 1] = tms32031.r[TMR_TEMP3];
	UPDATE_DEF();
}

static void mpyaddf_1(void)
{
	/* src3 * src1, src4 + src2 */
	UINT32 src3 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(OP));
	LONG2FP(TMR_TEMP1, src3);
	LONG2FP(TMR_TEMP2, src4);
	mpyf(&tms32031.r[TMR_TEMP3], &tms32031.r[TMR_TEMP1], &tms32031.r[(OP >> 19) & 7]);
	addf(&tms32031.r[((OP >> 22) & 1) | 2], &tms32031.r[TMR_TEMP2], &tms32031.r[(OP >> 16) & 7]);
	tms32031.r[(OP >> 23) & 1] = tms32031.r[TMR_TEMP3];
	UPDATE_DEF();
}

static void mpyaddf_2(void)
{
	/* src1 * src2, src3 + src4 */
	UINT32 src3 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(OP));
	LONG2FP(TMR_TEMP1, src3);
	LONG2FP(TMR_TEMP2, src4);
	mpyf(&tms32031.r[TMR_TEMP3], &tms32031.r[(OP >> 19) & 7], &tms32031.r[(OP >> 16) & 7]);
	addf(&tms32031.r[((OP >> 22) & 1) | 2], &tms32031.r[TMR_TEMP1], &tms32031.r[TMR_TEMP2]);
	tms32031.r[(OP >> 23) & 1] = tms32031.r[TMR_TEMP3];
	UPDATE_DEF();
}

static void mpyaddf_3(void)
{
	/* src3 * src1, src2 + src4 */
	UINT32 src3 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(OP));
	LONG2FP(TMR_TEMP1, src3);
	LONG2FP(TMR_TEMP2, src4);
	mpyf(&tms32031.r[TMR_TEMP3], &tms32031.r[TMR_TEMP1], &tms32031.r[(OP >> 19) & 7]);
	addf(&tms32031.r[((OP >> 22) & 1) | 2], &tms32031.r[(OP >> 16) & 7], &tms32031.r[TMR_TEMP2]);
	tms32031.r[(OP >> 23) & 1] = tms32031.r[TMR_TEMP3];
	UPDATE_DEF();
}

/*-----------------------------------------------------*/

static void mpysubf_0(void)
{
	/* src3 * src4, src1 - src2 */
	UINT32 src3 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(OP));
	LONG2FP(TMR_TEMP1, src3);
	LONG2FP(TMR_TEMP2, src4);
	mpyf(&tms32031.r[TMR_TEMP3], &tms32031.r[TMR_TEMP1], &tms32031.r[TMR_TEMP2]);
	subf(&tms32031.r[((OP >> 22) & 1) | 2], &tms32031.r[(OP >> 19) & 7], &tms32031.r[(OP >> 16) & 7]);
	tms32031.r[(OP >> 23) & 1] = tms32031.r[TMR_TEMP3];
	UPDATE_DEF();
}

static void mpysubf_1(void)
{
	/* src3 * src1, src4 - src2 */
	UINT32 src3 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(OP));
	LONG2FP(TMR_TEMP1, src3);
	LONG2FP(TMR_TEMP2, src4);
	mpyf(&tms32031.r[TMR_TEMP3], &tms32031.r[TMR_TEMP1], &tms32031.r[(OP >> 19) & 7]);
	subf(&tms32031.r[((OP >> 22) & 1) | 2], &tms32031.r[TMR_TEMP2], &tms32031.r[(OP >> 16) & 7]);
	tms32031.r[(OP >> 23) & 1] = tms32031.r[TMR_TEMP3];
	UPDATE_DEF();
}

static void mpysubf_2(void)
{
	/* src1 * src2, src3 - src4 */
	UINT32 src3 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(OP));
	LONG2FP(TMR_TEMP1, src3);
	LONG2FP(TMR_TEMP2, src4);
	mpyf(&tms32031.r[TMR_TEMP3], &tms32031.r[(OP >> 19) & 7], &tms32031.r[(OP >> 16) & 7]);
	subf(&tms32031.r[((OP >> 22) & 1) | 2], &tms32031.r[TMR_TEMP1], &tms32031.r[TMR_TEMP2]);
	tms32031.r[(OP >> 23) & 1] = tms32031.r[TMR_TEMP3];
	UPDATE_DEF();
}

static void mpysubf_3(void)
{
	/* src3 * src1, src2 - src4 */
	UINT32 src3 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(OP));
	LONG2FP(TMR_TEMP1, src3);
	LONG2FP(TMR_TEMP2, src4);
	mpyf(&tms32031.r[TMR_TEMP3], &tms32031.r[TMR_TEMP1], &tms32031.r[(OP >> 19) & 7]);
	subf(&tms32031.r[((OP >> 22) & 1) | 2], &tms32031.r[(OP >> 16) & 7], &tms32031.r[TMR_TEMP2]);
	tms32031.r[(OP >> 23) & 1] = tms32031.r[TMR_TEMP3];
	UPDATE_DEF();
}

/*-----------------------------------------------------*/

static void mpyaddi_0(void)
{
	/* src3 * src4, src1 + src2 */
	UINT32 src1 = IREG((OP >> 19) & 7);
	UINT32 src2 = IREG((OP >> 16) & 7);
	UINT32 src3 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(OP));
	INT64 mres = (INT64)((INT32)(src3 << 8) >> 8) * (INT64)((INT32)(src4 << 8) >> 8);
	UINT32 ares = src1 + src2;

	CLR_NZVUF();
	if (OVM)
	{
		if (mres < -(INT64)0x80000000 || mres > (INT64)0x7fffffff)
			mres = (mres < 0) ? 0x80000000 : 0x7fffffff;
		if (OVERFLOW_ADD(src1,src2,ares))
			ares = ((INT32)src1 < 0) ? 0x80000000 : 0x7fffffff;
	}
	IREG((OP >> 23) & 1) = mres;
	IREG(((OP >> 22) & 1) | 2) = ares;
	UPDATE_DEF();
}

static void mpyaddi_1(void)
{
	/* src3 * src1, src4 + src2 */
	UINT32 src1 = IREG((OP >> 19) & 7);
	UINT32 src2 = IREG((OP >> 16) & 7);
	UINT32 src3 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(OP));
	INT64 mres = (INT64)((INT32)(src3 << 8) >> 8) * (INT64)((INT32)(src1 << 8) >> 8);
	UINT32 ares = src4 + src2;

	CLR_NZVUF();
	if (OVM)
	{
		if (mres < -(INT64)0x80000000 || mres > (INT64)0x7fffffff)
			mres = (mres < 0) ? 0x80000000 : 0x7fffffff;
		if (OVERFLOW_ADD(src4,src2,ares))
			ares = ((INT32)src4 < 0) ? 0x80000000 : 0x7fffffff;
	}
	IREG((OP >> 23) & 1) = mres;
	IREG(((OP >> 22) & 1) | 2) = ares;
	UPDATE_DEF();
}

static void mpyaddi_2(void)
{
	/* src1 * src2, src3 + src4 */
	UINT32 src1 = IREG((OP >> 19) & 7);
	UINT32 src2 = IREG((OP >> 16) & 7);
	UINT32 src3 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(OP));
	INT64 mres = (INT64)((INT32)(src1 << 8) >> 8) * (INT64)((INT32)(src2 << 8) >> 8);
	UINT32 ares = src3 + src4;

	CLR_NZVUF();
	if (OVM)
	{
		if (mres < -(INT64)0x80000000 || mres > (INT64)0x7fffffff)
			mres = (mres < 0) ? 0x80000000 : 0x7fffffff;
		if (OVERFLOW_ADD(src3,src4,ares))
			ares = ((INT32)src3 < 0) ? 0x80000000 : 0x7fffffff;
	}
	IREG((OP >> 23) & 1) = mres;
	IREG(((OP >> 22) & 1) | 2) = ares;
	UPDATE_DEF();
}

static void mpyaddi_3(void)
{
	/* src3 * src1, src2 + src4 */
	UINT32 src1 = IREG((OP >> 19) & 7);
	UINT32 src2 = IREG((OP >> 16) & 7);
	UINT32 src3 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(OP));
	INT64 mres = (INT64)((INT32)(src3 << 8) >> 8) * (INT64)((INT32)(src1 << 8) >> 8);
	UINT32 ares = src2 + src4;

	CLR_NZVUF();
	if (OVM)
	{
		if (mres < -(INT64)0x80000000 || mres > (INT64)0x7fffffff)
			mres = (mres < 0) ? 0x80000000 : 0x7fffffff;
		if (OVERFLOW_ADD(src2,src4,ares))
			ares = ((INT32)src2 < 0) ? 0x80000000 : 0x7fffffff;
	}
	IREG((OP >> 23) & 1) = mres;
	IREG(((OP >> 22) & 1) | 2) = ares;
	UPDATE_DEF();
}

/*-----------------------------------------------------*/

static void mpysubi_0(void)
{
	/* src3 * src4, src1 - src2 */
	UINT32 src1 = IREG((OP >> 19) & 7);
	UINT32 src2 = IREG((OP >> 16) & 7);
	UINT32 src3 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(OP));
	INT64 mres = (INT64)((INT32)(src3 << 8) >> 8) * (INT64)((INT32)(src4 << 8) >> 8);
	UINT32 ares = src1 - src2;

	CLR_NZVUF();
	if (OVM)
	{
		if (mres < -(INT64)0x80000000 || mres > (INT64)0x7fffffff)
			mres = (mres < 0) ? 0x80000000 : 0x7fffffff;
		if (OVERFLOW_SUB(src1,src2,ares))
			ares = ((INT32)src1 < 0) ? 0x80000000 : 0x7fffffff;
	}
	IREG((OP >> 23) & 1) = mres;
	IREG(((OP >> 22) & 1) | 2) = ares;
	UPDATE_DEF();
}

static void mpysubi_1(void)
{
	/* src3 * src1, src4 - src2 */
	UINT32 src1 = IREG((OP >> 19) & 7);
	UINT32 src2 = IREG((OP >> 16) & 7);
	UINT32 src3 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(OP));
	INT64 mres = (INT64)((INT32)(src3 << 8) >> 8) * (INT64)((INT32)(src1 << 8) >> 8);
	UINT32 ares = src4 - src2;

	CLR_NZVUF();
	if (OVM)
	{
		if (mres < -(INT64)0x80000000 || mres > (INT64)0x7fffffff)
			mres = (mres < 0) ? 0x80000000 : 0x7fffffff;
		if (OVERFLOW_SUB(src4,src2,ares))
			ares = ((INT32)src4 < 0) ? 0x80000000 : 0x7fffffff;
	}
	IREG((OP >> 23) & 1) = mres;
	IREG(((OP >> 22) & 1) | 2) = ares;
	UPDATE_DEF();
}

static void mpysubi_2(void)
{
	/* src1 * src2, src3 - src4 */
	UINT32 src1 = IREG((OP >> 19) & 7);
	UINT32 src2 = IREG((OP >> 16) & 7);
	UINT32 src3 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(OP));
	INT64 mres = (INT64)((INT32)(src1 << 8) >> 8) * (INT64)((INT32)(src2 << 8) >> 8);
	UINT32 ares = src3 - src4;

	CLR_NZVUF();
	if (OVM)
	{
		if (mres < -(INT64)0x80000000 || mres > (INT64)0x7fffffff)
			mres = (mres < 0) ? 0x80000000 : 0x7fffffff;
		if (OVERFLOW_SUB(src3,src4,ares))
			ares = ((INT32)src3 < 0) ? 0x80000000 : 0x7fffffff;
	}
	IREG((OP >> 23) & 1) = mres;
	IREG(((OP >> 22) & 1) | 2) = ares;
	UPDATE_DEF();
}

static void mpysubi_3(void)
{
	/* src3 * src1, src2 - src4 */
	UINT32 src1 = IREG((OP >> 19) & 7);
	UINT32 src2 = IREG((OP >> 16) & 7);
	UINT32 src3 = RMEM(INDIRECT_1_DEF(OP >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(OP));
	INT64 mres = (INT64)((INT32)(src3 << 8) >> 8) * (INT64)((INT32)(src1 << 8) >> 8);
	UINT32 ares = src2 - src4;

	CLR_NZVUF();
	if (OVM)
	{
		if (mres < -(INT64)0x80000000 || mres > (INT64)0x7fffffff)
			mres = (mres < 0) ? 0x80000000 : 0x7fffffff;
		if (OVERFLOW_SUB(src2,src4,ares))
			ares = ((INT32)src2 < 0) ? 0x80000000 : 0x7fffffff;
	}
	IREG((OP >> 23) & 1) = mres;
	IREG(((OP >> 22) & 1) | 2) = ares;
	UPDATE_DEF();
}

/*-----------------------------------------------------*/

static void stfstf(void)
{
	WMEM(INDIRECT_1_DEF(OP >> 8), FP2LONG((OP >> 16) & 7));
	WMEM(INDIRECT_1(OP), FP2LONG((OP >> 22) & 7));
	UPDATE_DEF();
}

static void stisti(void)
{
	WMEM(INDIRECT_1_DEF(OP >> 8), IREG((OP >> 16) & 7));
	WMEM(INDIRECT_1(OP), IREG((OP >> 22) & 7));
	UPDATE_DEF();
}

/*-----------------------------------------------------*/

static void ldfldf(void)
{
	UINT32 res;
	int dreg;

	res = RMEM(INDIRECT_1_DEF(OP >> 8));
	dreg = (OP >> 19) & 7;
	LONG2FP(dreg, res);
	res = RMEM(INDIRECT_1(OP));
	dreg = (OP >> 22) & 7;
	LONG2FP(dreg, res);
	UPDATE_DEF();
}

static void ldildi(void)
{
	IREG((OP >> 19) & 7) = RMEM(INDIRECT_1_DEF(OP >> 8));
	IREG((OP >> 22) & 7) = RMEM(INDIRECT_1(OP));
	UPDATE_DEF();
}

/*-----------------------------------------------------*/

//  src2 = ind(OP)
//  dst2 = ind(OP >> 8)
//  sreg3 = ((OP >> 16) & 7)
//  sreg1 = ((OP >> 19) & 7)
//  dreg1 = ((OP >> 22) & 7)

static void absfstf(void)
{
	UINT32 src3 = FP2LONG((OP >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(OP));
	{
		int dreg = (OP >> 22) & 7;
		LONG2FP(TMR_TEMP1, src2);
		ABSF(dreg, TMR_TEMP1);
	}
	WMEM(INDIRECT_1(OP >> 8), src3);
	UPDATE_DEF();
}

static void absisti(void)
{
	UINT32 src3 = IREG((OP >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(OP));
	{
		int dreg = (OP >> 22) & 7;
		ABSI(dreg, src2);
	}
	WMEM(INDIRECT_1(OP >> 8), src3);
	UPDATE_DEF();
}

static void addf3stf(void)
{
	UINT32 src3 = FP2LONG((OP >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(OP));
	{
		LONG2FP(TMR_TEMP1, src2);
		addf(&tms32031.r[(OP >> 22) & 7], &tms32031.r[(OP >> 19) & 7], &tms32031.r[TMR_TEMP1]);
	}
	WMEM(INDIRECT_1(OP >> 8), src3);
	UPDATE_DEF();
}

static void addi3sti(void)
{
	UINT32 src3 = IREG((OP >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(OP));
	{
		int dreg = (OP >> 22) & 7;
		UINT32 src1 = IREG((OP >> 19) & 7);
		ADDI(dreg, src1, src2);
	}
	WMEM(INDIRECT_1(OP >> 8), src3);
	UPDATE_DEF();
}

static void and3sti(void)
{
	UINT32 src3 = IREG((OP >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(OP));
	{
		int dreg = (OP >> 22) & 7;
		UINT32 src1 = IREG((OP >> 19) & 7);
		AND(dreg, src1, src2);
	}
	WMEM(INDIRECT_1(OP >> 8), src3);
	UPDATE_DEF();
}

static void ash3sti(void)
{
	UINT32 src3 = IREG((OP >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(OP));
	{
		int dreg = (OP >> 22) & 7;
		UINT32 count = IREG((OP >> 19) & 7);
		ASH(dreg, src2, count);
	}
	WMEM(INDIRECT_1(OP >> 8), src3);
	UPDATE_DEF();
}

static void fixsti(void)
{
	UINT32 src3 = IREG((OP >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(OP));
	{
		int dreg = (OP >> 22) & 7;
		LONG2FP(dreg, src2);
		float2int(&tms32031.r[dreg], 1);
	}
	WMEM(INDIRECT_1(OP >> 8), src3);
	UPDATE_DEF();
}

static void floatstf(void)
{
	UINT32 src3 = FP2LONG((OP >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(OP));
	{
		int dreg = (OP >> 22) & 7;
		IREG(dreg) = src2;
		int2float(&tms32031.r[dreg]);
	}
	WMEM(INDIRECT_1(OP >> 8), src3);
	UPDATE_DEF();
}

static void ldfstf(void)
{
	UINT32 src3 = FP2LONG((OP >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(OP));
	{
		int dreg = (OP >> 22) & 7;
		LONG2FP(dreg, src2);
	}
	WMEM(INDIRECT_1(OP >> 8), src3);
	UPDATE_DEF();
}

static void ldisti(void)
{
	UINT32 src3 = IREG((OP >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(OP));
	IREG((OP >> 22) & 7) = src2;
	WMEM(INDIRECT_1(OP >> 8), src3);
	UPDATE_DEF();
}

static void lsh3sti(void)
{
	UINT32 src3 = IREG((OP >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(OP));
	{
		int dreg = (OP >> 22) & 7;
		UINT32 count = IREG((OP >> 19) & 7);
		LSH(dreg, src2, count);
	}
	WMEM(INDIRECT_1(OP >> 8), src3);
	UPDATE_DEF();
}

static void mpyf3stf(void)
{
	UINT32 src3 = FP2LONG((OP >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(OP));
	{
		LONG2FP(TMR_TEMP1, src2);
		mpyf(&tms32031.r[(OP >> 22) & 7], &tms32031.r[(OP >> 19) & 7], &tms32031.r[TMR_TEMP1]);
	}
	WMEM(INDIRECT_1(OP >> 8), src3);
	UPDATE_DEF();
}

static void mpyi3sti(void)
{
	UINT32 src3 = IREG((OP >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(OP));
	{
		int dreg = (OP >> 22) & 7;
		UINT32 src1 = IREG((OP >> 19) & 7);
		MPYI(dreg, src1, src2);
	}
	WMEM(INDIRECT_1(OP >> 8), src3);
	UPDATE_DEF();
}

static void negfstf(void)
{
	UINT32 src3 = FP2LONG((OP >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(OP));
	{
		LONG2FP(TMR_TEMP1, src2);
		negf(&tms32031.r[(OP >> 22) & 7], &tms32031.r[TMR_TEMP1]);
	}
	WMEM(INDIRECT_1(OP >> 8), src3);
	UPDATE_DEF();
}

static void negisti(void)
{
	UINT32 src3 = IREG((OP >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(OP));
	{
		int dreg = (OP >> 22) & 7;
		NEGI(dreg, src2);
	}
	WMEM(INDIRECT_1(OP >> 8), src3);
	UPDATE_DEF();
}

static void notsti(void)
{
	UINT32 src3 = IREG((OP >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(OP));
	{
		int dreg = (OP >> 22) & 7;
		NOT(dreg, src2);
	}
	WMEM(INDIRECT_1(OP >> 8), src3);
	UPDATE_DEF();
}

static void or3sti(void)
{
	UINT32 src3 = IREG((OP >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(OP));
	{
		int dreg = (OP >> 22) & 7;
		UINT32 src1 = IREG((OP >> 19) & 7);
		OR(dreg, src1, src2);
	}
	WMEM(INDIRECT_1(OP >> 8), src3);
	UPDATE_DEF();
}

static void subf3stf(void)
{
	UINT32 src3 = FP2LONG((OP >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(OP));
	{
		LONG2FP(TMR_TEMP1, src2);
		subf(&tms32031.r[(OP >> 22) & 7], &tms32031.r[TMR_TEMP1], &tms32031.r[(OP >> 19) & 7]);
	}
	WMEM(INDIRECT_1(OP >> 8), src3);
	UPDATE_DEF();
}

static void subi3sti(void)
{
	UINT32 src3 = IREG((OP >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(OP));
	{
		int dreg = (OP >> 22) & 7;
		UINT32 src1 = IREG((OP >> 19) & 7);
		SUBI(dreg, src2, src1);
	}
	WMEM(INDIRECT_1(OP >> 8), src3);
	UPDATE_DEF();
}

static void xor3sti(void)
{
	UINT32 src3 = IREG((OP >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(OP));
	{
		int dreg = (OP >> 22) & 7;
		UINT32 src1 = IREG((OP >> 19) & 7);
		XOR(dreg, src1, src2);
	}
	WMEM(INDIRECT_1(OP >> 8), src3);
	UPDATE_DEF();
}


/***************************************************************************
    FUNCTION TABLE
***************************************************************************/

void (*const tms32031ops[])(void) =
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
