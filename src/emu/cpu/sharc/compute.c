/* compute operations */

#include <math.h>

#define CLEAR_ALU_FLAGS()		(sharc.astat &= ~(AZ|AN|AV|AC|AS|AI))

#define SET_FLAG_AZ(r)			{ sharc.astat |= (((r) == 0) ? AZ : 0); }
#define SET_FLAG_AN(r)			{ sharc.astat |= (((r) & 0x80000000) ? AN : 0); }
#define SET_FLAG_AC_ADD(r,a,b)	{ sharc.astat |= (((UINT32)r < (UINT32)a) ? AC : 0); }
#define SET_FLAG_AV_ADD(r,a,b)	{ sharc.astat |= (((~((a) ^ (b)) & ((a) ^ (r))) & 0x80000000) ? AV : 0); }
#define SET_FLAG_AC_SUB(r,a,b)	{ sharc.astat |= ((!((UINT32)a < (UINT32)b)) ? AC : 0); }
#define SET_FLAG_AV_SUB(r,a,b)	{ sharc.astat |= ((( ((a) ^ (b)) & ((a) ^ (r))) & 0x80000000) ? AV : 0); }

#define IS_FLOAT_ZERO(r)		((((r) & 0x7fffffff) == 0))
#define IS_FLOAT_DENORMAL(r)	((((r) & 0x7f800000) == 0) && (((r) & 0x7fffff) != 0))
#define IS_FLOAT_NAN(r)			((((r) & 0x7f800000) == 0x7f800000) && (((r) & 0x7fffff) != 0))
#define IS_FLOAT_INFINITY(r)	(((r) & 0x7fffffff) == 0x7f800000)

#define CLEAR_MULTIPLIER_FLAGS()	(sharc.astat &= ~(MN|MV|MU|MI))

#define SET_FLAG_MN(r)			{ sharc.astat |= (((r) & 0x80000000) ? MN : 0); }
#define SET_FLAG_MV(r)			{ sharc.astat |= ((((UINT32)((r) >> 32) != 0) && ((UINT32)((r) >> 32) != 0xffffffff)) ? MV : 0); }

/* TODO: MU needs 80-bit result */
#define SET_FLAG_MU(r)			{ sharc.astat |= ((((UINT32)((r) >> 32) == 0) && ((UINT32)(r)) != 0) ? MU : 0); }


#define FLOAT_SIGN			0x80000000
#define FLOAT_INFINITY		0x7f800000
#define FLOAT_MANTISSA		0x007fffff

/*****************************************************************************/

// Mantissa lookup-table for RECIPS opcode
static const UINT32 recips_mantissa_lookup[128] =
{
	0x007F8000, 0x007E0000, 0x007C0000, 0x007A0000,
	0x00780000, 0x00760000, 0x00740000, 0x00720000,
	0x00700000, 0x006F0000, 0x006D0000, 0x006B0000,
	0x006A0000, 0x00680000, 0x00660000, 0x00650000,
	0x00630000, 0x00610000, 0x00600000, 0x005E0000,
	0x005D0000, 0x005B0000, 0x005A0000, 0x00590000,
	0x00570000, 0x00560000, 0x00540000, 0x00530000,
	0x00520000, 0x00500000, 0x004F0000, 0x004E0000,
	0x004C0000, 0x004B0000, 0x004A0000, 0x00490000,
	0x00470000, 0x00460000, 0x00450000, 0x00440000,
	0x00430000, 0x00410000, 0x00400000, 0x003F0000,
	0x003E0000, 0x003D0000, 0x003C0000, 0x003B0000,
	0x003A0000, 0x00390000, 0x00380000, 0x00370000,
	0x00360000, 0x00350000, 0x00340000, 0x00330000,
	0x00320000, 0x00310000, 0x00300000, 0x002F0000,
	0x002E0000, 0x002D0000, 0x002C0000, 0x002B0000,
	0x002A0000, 0x00290000, 0x00280000, 0x00280000,
	0x00270000, 0x00260000, 0x00250000, 0x00240000,
	0x00230000, 0x00230000, 0x00220000, 0x00210000,
	0x00200000, 0x001F0000, 0x001F0000, 0x001E0000,
	0x001D0000, 0x001C0000, 0x001C0000, 0x001B0000,
	0x001A0000, 0x00190000, 0x00190000, 0x00180000,
	0x00170000, 0x00170000, 0x00160000, 0x00150000,
	0x00140000, 0x00140000, 0x00130000, 0x00120000,
	0x00120000, 0x00110000, 0x00100000, 0x00100000,
	0x000F0000, 0x000F0000, 0x000E0000, 0x000D0000,
	0x000D0000, 0x000C0000, 0x000C0000, 0x000B0000,
	0x000A0000, 0x000A0000, 0x00090000, 0x00090000,
	0x00080000, 0x00070000, 0x00070000, 0x00060000,
	0x00060000, 0x00050000, 0x00050000, 0x00040000,
	0x00040000, 0x00030000, 0x00030000, 0x00020000,
	0x00020000, 0x00010000, 0x00010000, 0x00000000,
};

// Mantissa lookup-table for RSQRTS opcode
static const UINT32 rsqrts_mantissa_lookup[128] =
{
	0x00350000, 0x00330000, 0x00320000, 0x00300000,
	0x002F0000, 0x002E0000, 0x002D0000, 0x002B0000,
	0x002A0000, 0x00290000, 0x00280000, 0x00270000,
	0x00260000, 0x00250000, 0x00230000, 0x00220000,
	0x00210000, 0x00200000, 0x001F0000, 0x001E0000,
	0x001E0000, 0x001D0000, 0x001C0000, 0x001B0000,
	0x001A0000, 0x00190000, 0x00180000, 0x00170000,
	0x00160000, 0x00160000, 0x00150000, 0x00140000,
	0x00130000, 0x00130000, 0x00120000, 0x00110000,
	0x00100000, 0x00100000, 0x000F0000, 0x000E0000,
	0x000E0000, 0x000D0000, 0x000C0000, 0x000B0000,
	0x000B0000, 0x000A0000, 0x000A0000, 0x00090000,
	0x00080000, 0x00080000, 0x00070000, 0x00070000,
	0x00060000, 0x00050000, 0x00050000, 0x00040000,
	0x00040000, 0x00030000, 0x00030000, 0x00020000,
	0x00020000, 0x00010000, 0x00010000, 0x00000000,
	0x007F8000, 0x007E0000, 0x007C0000, 0x007A0000,
	0x00780000, 0x00760000, 0x00740000, 0x00730000,
	0x00710000, 0x006F0000, 0x006E0000, 0x006C0000,
	0x006A0000, 0x00690000, 0x00670000, 0x00660000,
	0x00640000, 0x00630000, 0x00620000, 0x00600000,
	0x005F0000, 0x005E0000, 0x005C0000, 0x005B0000,
	0x005A0000, 0x00590000, 0x00570000, 0x00560000,
	0x00550000, 0x00540000, 0x00530000, 0x00520000,
	0x00510000, 0x004F0000, 0x004E0000, 0x004D0000,
	0x004C0000, 0x004B0000, 0x004A0000, 0x00490000,
	0x00480000, 0x00470000, 0x00460000, 0x00450000,
	0x00450000, 0x00440000, 0x00430000, 0x00420000,
	0x00410000, 0x00400000, 0x003F0000, 0x003E0000,
	0x003E0000, 0x003D0000, 0x003C0000, 0x003B0000,
	0x003A0000, 0x003A0000, 0x00390000, 0x00380000,
	0x00370000, 0x00370000, 0x00360000, 0x00350000,
};

/*****************************************************************************/
/* Integer ALU operations */

/* Rn = Rx + Ry */
INLINE void compute_add(int rn, int rx, int ry)
{
	UINT32 r = REG(rx) + REG(ry);

	if (sharc.mode1 & MODE1_ALUSAT)
		fatalerror("SHARC: compute_add: ALU saturation not implemented !");

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	SET_FLAG_AV_ADD(r, REG(rx), REG(ry));
	SET_FLAG_AC_ADD(r, REG(rx), REG(ry));
	REG(rn) = r;

	sharc.astat &= ~AF;
}

/* Rn = Rx - Ry */
INLINE void compute_sub(int rn, int rx, int ry)
{
	UINT32 r = REG(rx) - REG(ry);

	if (sharc.mode1 & MODE1_ALUSAT)
		fatalerror("SHARC: compute_sub: ALU saturation not implemented !");

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	SET_FLAG_AV_SUB(r, REG(rx), REG(ry));
	SET_FLAG_AC_SUB(r, REG(rx), REG(ry));
	REG(rn) = r;

	sharc.astat &= ~AF;
}

/* Rn = Rx + Ry + CI */
INLINE void compute_add_ci(int rn, int rx, int ry)
{
	int c = (sharc.astat & AC) ? 1 : 0;
	UINT32 r = REG(rx) + REG(ry) + c;

	if (sharc.mode1 & MODE1_ALUSAT)
		fatalerror("SHARC: compute_add_ci: ALU saturation not implemented !");

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	SET_FLAG_AV_ADD(r, REG(rx), REG(ry)+c);
	SET_FLAG_AC_ADD(r, REG(rx), REG(ry)+c);
	REG(rn) = r;

	sharc.astat &= ~AF;
}

/* Rn = Rx - Ry + CI - 1 */
INLINE void compute_sub_ci(int rn, int rx, int ry)
{
	int c = (sharc.astat & AC) ? 1 : 0;
	UINT32 r = REG(rx) - REG(ry) + c - 1;

	if (sharc.mode1 & MODE1_ALUSAT)
		fatalerror("SHARC: compute_sub_ci: ALU saturation not implemented !");

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	SET_FLAG_AV_SUB(r, REG(rx), REG(ry)+c-1);
	SET_FLAG_AC_SUB(r, REG(rx), REG(ry)+c-1);
	REG(rn) = r;

	sharc.astat &= ~AF;
}

/* Rn = Rx AND Ry */
INLINE void compute_and(int rn, int rx, int ry)
{
	UINT32 r = REG(rx) & REG(ry);

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	REG(rn) = r;

	sharc.astat &= ~AF;
}

/* COMP(Rx, Ry) */
INLINE void compute_comp(int rx, int ry)
{
	UINT32 comp_accum;

	CLEAR_ALU_FLAGS();
	if( REG(rx) == REG(ry) )
		sharc.astat |= AZ;
	if( (INT32)REG(rx) < (INT32)REG(ry) )
		sharc.astat |= AN;

	// Update ASTAT compare accumulation register
	comp_accum = (sharc.astat >> 24) & 0xff;
	comp_accum >>= 1;
	if ((sharc.astat & (AZ|AN)) == 0)
	{
		comp_accum |= 0x80;
	}
	sharc.astat &= 0xffffff;
	sharc.astat |= comp_accum << 24;

	sharc.astat &= ~AF;
}

/* Rn = PASS Rx */
INLINE void compute_pass(int rn, int rx)
{
	CLEAR_ALU_FLAGS();
	/* TODO: floating-point extension field is set to 0 */

	REG(rn) = REG(rx);
	if (REG(rn) == 0)
		sharc.astat |= AZ;
	if (REG(rn) & 0x80000000)
		sharc.astat |= AN;

	sharc.astat &= ~AF;
}

/* Rn = Rx XOR Ry */
INLINE void compute_xor(int rn, int rx, int ry)
{
	UINT32 r = REG(rx) ^ REG(ry);
	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	REG(rn) = r;

	sharc.astat &= ~AF;
}

/* Rn = Rx OR Ry */
INLINE void compute_or(int rn, int rx, int ry)
{
	UINT32 r = REG(rx) | REG(ry);
	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	REG(rn) = r;

	sharc.astat &= ~AF;
}

/* Rn = Rx + 1 */
INLINE void compute_inc(int rn, int rx)
{
	UINT32 r = REG(rx) + 1;

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	SET_FLAG_AV_ADD(r, REG(rx), 1);
	SET_FLAG_AC_ADD(r, REG(rx), 1);

	REG(rn) = r;

	sharc.astat &= ~AF;
}

/* Rn = Rx - 1 */
INLINE void compute_dec(int rn, int rx)
{
	UINT32 r = REG(rx) - 1;

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	SET_FLAG_AV_SUB(r, REG(rx), 1);
	SET_FLAG_AC_SUB(r, REG(rx), 1);

	REG(rn) = r;

	sharc.astat &= ~AF;
}

/* Rn = MIN(Rx, Ry) */
INLINE void compute_min(int rn, int rx, int ry)
{
	UINT32 r = MIN((INT32)REG(rx), (INT32)REG(ry));

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);

	REG(rn) = r;

	sharc.astat &= ~AF;
}

/* Rn = MAX(Rx, Ry) */
INLINE void compute_max(int rn, int rx, int ry)
{
	UINT32 r = MAX((INT32)REG(rx), (INT32)REG(ry));

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);

	REG(rn) = r;

	sharc.astat &= ~AF;
}

/* Rn = -Rx */
INLINE void compute_neg(int rn, int rx)
{
	UINT32 r = -(INT32)(REG(rx));

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	SET_FLAG_AV_SUB(r, 0, REG(rx));
	SET_FLAG_AC_SUB(r, 0, REG(rx));

	REG(rn) = r;

	sharc.astat &= ~AF;
}

/* Rn = NOT Rx */
INLINE void compute_not(int rn, int rx)
{
	UINT32 r = ~REG(rx);

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);

	REG(rn) = r;

	sharc.astat &= ~AF;
}

/*****************************************************************************/
/* Floating-point ALU operations */

INLINE UINT32 SCALB(SHARC_REG rx, int ry)
{
	UINT32 mantissa = rx.r & FLOAT_MANTISSA;
	UINT32 sign = rx.r & FLOAT_SIGN;

	int exponent = ((rx.r >> 23) & 0xff) - 127;
	exponent += (INT32)(REG(ry));

	if (exponent > 127)
	{
		// overflow
		sharc.astat |= AV;
		return sign | FLOAT_INFINITY;
	}
	else if (exponent < -126)
	{
		// denormal
		sharc.astat |= AZ;
		return sign;
	}
	else
	{
		return sign | (((exponent + 127) & 0xff) << 23) | mantissa;
	}
}

/* Fn = FLOAT Rx */
INLINE void compute_float(int rn, int rx)
{
	// verified
	FREG(rn) = (float)(INT32)REG(rx);

	CLEAR_ALU_FLAGS();
	// AN
	SET_FLAG_AN(REG(rn));
	// AZ
	sharc.astat |= (IS_FLOAT_DENORMAL(REG(rn)) || IS_FLOAT_ZERO(REG(rn))) ? AZ : 0;
	// AUS
	sharc.stky |= (IS_FLOAT_DENORMAL(REG(rn))) ? AUS : 0;
	/* TODO: AV flag */

	sharc.astat |= AF;
}

/* Rn = FIX Fx */
INLINE void compute_fix(int rn, int rx)
{
	INT32 alu_i;
	SHARC_REG r_alu;

	r_alu.f = FREG(rx);
	if (sharc.mode1 & MODE1_TRUNCATE)
	{
		alu_i = (INT32)(r_alu.f);
	}
	else
	{
		alu_i = (INT32)(r_alu.f < 0 ? (r_alu.f - 0.5f) : (r_alu.f + 0.5f));
	}

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(alu_i);
	// AZ
	SET_FLAG_AZ(alu_i);
	// AU
	sharc.stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	// AI
	sharc.astat |= (IS_FLOAT_NAN(REG(rx))) ? AI : 0;
	/* TODO: AV flag */

	REG(rn) = alu_i;
	sharc.astat |= AF;
}

/* Rn = FIX Fx BY Ry */
INLINE void compute_fix_scaled(int rn, int rx, int ry)
{
	INT32 alu_i;
	SHARC_REG r_alu;

	r_alu.r = SCALB(sharc.r[rx], ry);
	if (sharc.mode1 & MODE1_TRUNCATE)
	{
		alu_i = (INT32)(r_alu.f);
	}
	else
	{
		alu_i = (INT32)(r_alu.f < 0 ? (r_alu.f - 0.5f) : (r_alu.f + 0.5f));
	}

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(alu_i);
	// AZ
	SET_FLAG_AZ(alu_i);
	// AU
	sharc.stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	// AI
	sharc.astat |= (IS_FLOAT_NAN(REG(rx))) ? AI : 0;
	/* TODO: AV flag */

	REG(rn) = alu_i;
	sharc.astat |= AF;
}

/* Fn = FLOAT Rx BY Ry */
INLINE void compute_float_scaled(int rn, int rx, int ry)
{
	SHARC_REG x;
	x.f = (float)(INT32)(REG(rx));

	// verified
	CLEAR_ALU_FLAGS();

	REG(rn) = SCALB(x, ry);

	// AN
	SET_FLAG_AN(REG(rn));
	// AZ
	sharc.astat |= (IS_FLOAT_DENORMAL(REG(rn)) || IS_FLOAT_ZERO(REG(rn))) ? AZ : 0;
	// AU
	sharc.stky |= (IS_FLOAT_DENORMAL(REG(rn))) ? AUS : 0;

	sharc.astat |= AF;
}

/* Rn = LOGB Fx */
INLINE void compute_logb(int rn, int rx)
{
	// verified
	UINT32 r = REG(rx);

	CLEAR_ALU_FLAGS();

	if (IS_FLOAT_INFINITY(REG(rx)))
	{
		REG(rn) = FLOAT_INFINITY;

		sharc.astat |= AV;
	}
	else if (IS_FLOAT_ZERO(REG(rx)))
	{
		REG(rn) = FLOAT_SIGN | FLOAT_INFINITY;

		sharc.astat |= AV;
	}
	else if (IS_FLOAT_NAN(REG(rx)))
	{
		REG(rn) = 0xffffffff;

		sharc.astat |= AI;
		sharc.stky |= AIS;
	}
	else
	{
		int exponent = (r >> 23) & 0xff;
		exponent -= 127;

		// AN
		SET_FLAG_AN(exponent);
		// AZ
		SET_FLAG_AZ(exponent);

		REG(rn) = exponent;
	}
	sharc.astat |= AF;
}

/* Fn = SCALB Fx BY Fy */
INLINE void compute_scalb(int rn, int rx, int ry)
{
	// verified
	SHARC_REG r;
	CLEAR_ALU_FLAGS();

	if (IS_FLOAT_NAN(REG(rx)))
	{
		sharc.astat |= AI;
		sharc.stky |= AIS;

		REG(rn) = 0xffffffff;
	}
	else
	{
		r.r = SCALB(sharc.r[rx], ry);

		// AN
		SET_FLAG_AN(r.r);
		// AZ
		sharc.astat |= IS_FLOAT_ZERO(r.r) ? AZ : 0;
		// AUS
		sharc.stky |= (IS_FLOAT_DENORMAL(r.r)) ? AUS : 0;

		FREG(rn) = r.f;
	}
	sharc.astat |= AF;
}

/* Fn = Fx + Fy */
INLINE void compute_fadd(int rn, int rx, int ry)
{
	SHARC_REG r;
	r.f = FREG(rx) + FREG(ry);

	CLEAR_ALU_FLAGS();
	// AN
	sharc.astat |= (r.f < 0.0f) ? AN : 0;
	// AZ
	sharc.astat |= (IS_FLOAT_DENORMAL(r.r) || IS_FLOAT_ZERO(r.r)) ? AZ : 0;
	// AUS
	sharc.stky |= (IS_FLOAT_DENORMAL(r.r)) ? AUS : 0;
	// AI
	sharc.astat |= (IS_FLOAT_NAN(REG(rx)) || IS_FLOAT_NAN(REG(ry))) ? AI : 0;
	/* TODO: AV flag */

	// AIS
	if (sharc.astat & AI)	sharc.stky |= AIS;

	FREG(rn) = r.f;
	sharc.astat |= AF;
}

/* Fn = Fx - Fy */
INLINE void compute_fsub(int rn, int rx, int ry)
{
	SHARC_REG r;
	r.f = FREG(rx) - FREG(ry);

	CLEAR_ALU_FLAGS();
	// AN
	sharc.astat |= (r.f < 0.0f) ? AN : 0;
	// AZ
	sharc.astat |= (IS_FLOAT_DENORMAL(r.r) || IS_FLOAT_ZERO(r.r)) ? AZ : 0;
	// AUS
	sharc.stky |= (IS_FLOAT_DENORMAL(r.r)) ? AUS : 0;
	// AI
	sharc.astat |= (IS_FLOAT_NAN(REG(rx)) || IS_FLOAT_NAN(REG(ry))) ? AI : 0;
	/* TODO: AV flag */

	// AIS
	if (sharc.astat & AI)	sharc.stky |= AIS;

	FREG(rn) = r.f;
	sharc.astat |= AF;
}

/* Fn = -Fx */
INLINE void compute_fneg(int rn, int rx)
{
	SHARC_REG r;
	r.f = -FREG(rx);

	CLEAR_ALU_FLAGS();
	// AZ
	sharc.astat |= (IS_FLOAT_ZERO(r.r)) ? AZ : 0;
	// AN
	sharc.astat |= (r.f < 0.0f) ? AN : 0;
	// AI
	sharc.astat |= (IS_FLOAT_NAN(REG(rx))) ? AI : 0;

	// AIS
	if (sharc.astat & AI)	sharc.stky |= AIS;

	FREG(rn) = r.f;
	sharc.astat |= AF;
}

/* COMP(Fx, Fy) */
INLINE void compute_fcomp(int rx, int ry)
{
	UINT32 comp_accum;

	CLEAR_ALU_FLAGS();
	// AZ
	if( FREG(rx) == FREG(ry) )
		sharc.astat |= AZ;
	// AN
	if( FREG(rx) < FREG(ry) )
		sharc.astat |= AN;
	// AI
	sharc.astat |= (IS_FLOAT_NAN(REG(rx)) || IS_FLOAT_NAN(REG(ry))) ? AI : 0;

	// AIS
	if (sharc.astat & AI)	sharc.stky |= AIS;

	// Update ASTAT compare accumulation register
	comp_accum = (sharc.astat >> 24) & 0xff;
	comp_accum >>= 1;
	if ((sharc.astat & (AZ|AN)) == 0)
	{
		comp_accum |= 0x80;
	}
	sharc.astat &= 0xffffff;
	sharc.astat |= comp_accum << 24;
	sharc.astat |= AF;
}

/* Fn = ABS(Fx + Fy) */
INLINE void compute_fabs_plus(int rn, int rx, int ry)
{
	SHARC_REG r;
	r.f = fabs(FREG(rx) + FREG(ry));

	CLEAR_ALU_FLAGS();
	// AZ
	sharc.astat |= (IS_FLOAT_DENORMAL(r.r) || IS_FLOAT_ZERO(r.r)) ? AZ : 0;
	// AUS
	sharc.stky |= (IS_FLOAT_DENORMAL(r.r)) ? AUS : 0;
	// AI
	sharc.astat |= (IS_FLOAT_NAN(REG(rx)) || IS_FLOAT_NAN(REG(ry))) ? AI : 0;
	/* TODO: AV flag */

	// AIS
	if (sharc.astat & AI)	sharc.stky |= AIS;

	FREG(rn) = r.f;
	sharc.astat |= AF;
}

/* Fn = MAX(Fx, Fy) */
INLINE void compute_fmax(int rn, int rx, int ry)
{
	SHARC_REG r_alu;

	r_alu.f = MAX(FREG(rx), FREG(ry));

	CLEAR_ALU_FLAGS();
	sharc.astat |= (r_alu.f < 0.0f) ? AN : 0;
	// AZ
	sharc.astat |= (IS_FLOAT_ZERO(r_alu.r)) ? AZ : 0;
	// AU
	sharc.stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	// AI
	sharc.astat |= (IS_FLOAT_NAN(REG(rx)) || IS_FLOAT_NAN(REG(ry))) ? AI : 0;
	/* TODO: AV flag */

	FREG(rn) = r_alu.f;
	sharc.astat |= AF;
}

/* Fn = MIN(Fx, Fy) */
INLINE void compute_fmin(int rn, int rx, int ry)
{
	SHARC_REG r_alu;

	r_alu.f = MIN(FREG(rx), FREG(ry));

	CLEAR_ALU_FLAGS();
	sharc.astat |= (r_alu.f < 0.0f) ? AN : 0;
	// AZ
	sharc.astat |= (IS_FLOAT_ZERO(r_alu.r)) ? AZ : 0;
	// AU
	sharc.stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	// AI
	sharc.astat |= (IS_FLOAT_NAN(REG(rx)) || IS_FLOAT_NAN(REG(ry))) ? AI : 0;
	/* TODO: AV flag */

	FREG(rn) = r_alu.f;
	sharc.astat |= AF;
}

/* Fn = CLIP Fx BY Fy */
INLINE void compute_fclip(int rn, int rx, int ry)
{
	SHARC_REG r_alu;

	if (FREG(rx) < fabs(FREG(ry)))
	{
		r_alu.f = FREG(rx);
	}
	else
	{
		if (FREG(rx) >= 0.0f)
		{
			r_alu.f = fabs(FREG(ry));
		}
		else
		{
			r_alu.f = -fabs(FREG(ry));
		}
	}


	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r_alu.r);
	// AZ
	sharc.astat |= (IS_FLOAT_ZERO(r_alu.r)) ? AZ : 0;
	// AU
	sharc.stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	// AI
	sharc.astat |= (IS_FLOAT_NAN(REG(rx)) || IS_FLOAT_NAN(REG(ry))) ? AI : 0;

	FREG(rn) = r_alu.f;
	sharc.astat |= AF;
}

/* Fn = RECIPS Fx */
INLINE void compute_recips(int rn, int rx)
{
	// verified
	UINT32 r;

	CLEAR_ALU_FLAGS();

	if (IS_FLOAT_NAN(REG(rx)))
	{
		// NaN
		r = 0xffffffff;

		// AI
		sharc.astat |= AI;

		// AIS
		sharc.stky |= AIS;
	}
	else if (IS_FLOAT_ZERO(REG(rx)))
	{
		// +- Zero
		r = (REG(rx) & FLOAT_SIGN) | FLOAT_INFINITY;

		sharc.astat |= AZ;
	}
	else
	{
		UINT32 mantissa = REG(rx) & 0x7fffff;
		UINT32 exponent = (REG(rx) >> 23) & 0xff;
		UINT32 sign = REG(rx) & FLOAT_SIGN;

		UINT32 res_mantissa = recips_mantissa_lookup[mantissa >> 16];

		int res_exponent = -(exponent - 127) - 1;
		if (res_exponent > 125 || res_exponent < -126)
		{
			res_exponent = 0;
			res_mantissa = 0;
		}
		else
		{
			res_exponent = (res_exponent + 127) & 0xff;
		}

		r = sign | (res_exponent << 23) | res_mantissa;

		SET_FLAG_AN(REG(rx));
		// AZ & AV
		sharc.astat |= (IS_FLOAT_ZERO(r)) ? AZ : 0;
		sharc.astat |= (IS_FLOAT_ZERO(REG(rx))) ? AV : 0;
		// AI
		sharc.astat |= (IS_FLOAT_NAN(REG(rx))) ? AI : 0;

		// AIS
		if (sharc.astat & AI)	sharc.stky |= AIS;
	}

	// AF
	sharc.astat |= AF;

	REG(rn) = r;
}

/* Fn = RSQRTS Fx */
INLINE void compute_rsqrts(int rn, int rx)
{
	// verified
	UINT32 r;

	if ((UINT32)(REG(rx)) > 0x80000000)
	{
		// non-zero negative
		r = 0xffffffff;
	}
	else if (IS_FLOAT_NAN(REG(rx)))
	{
		// NaN
		r = 0xffffffff;
	}
	else
	{
		UINT32 mantissa = REG(rx) & 0xffffff;	// mantissa + LSB of biased exponent
		UINT32 exponent = (REG(rx) >> 23) & 0xff;
		UINT32 sign = REG(rx) & FLOAT_SIGN;

		UINT32 res_mantissa = rsqrts_mantissa_lookup[mantissa >> 17];

		int res_exponent = -((exponent - 127) / 2) - 1;
		res_exponent = (res_exponent + 127) & 0xff;

		r = sign | (res_exponent << 23) | res_mantissa;
	}

	CLEAR_ALU_FLAGS();
	// AN
	sharc.astat |= (REG(rx) == 0x80000000) ? AN : 0;
	// AZ & AV
	sharc.astat |= (IS_FLOAT_ZERO(r)) ? AZ : 0;
	sharc.astat |= (IS_FLOAT_ZERO(REG(rx))) ? AV : 0;
	// AI
	sharc.astat |= (IS_FLOAT_NAN(REG(rx)) || (REG(rx) & 0x80000000)) ? AI : 0;
	// AIS
	if (sharc.astat & AI)	sharc.stky |= AIS;
	// AF
	sharc.astat |= AF;

	REG(rn) = r;
}


/* Fn = PASS Fx */
INLINE void compute_fpass(int rn, int rx)
{
	SHARC_REG r;
	r.f = FREG(rx);

	CLEAR_ALU_FLAGS();
	// AN
	sharc.astat |= (r.f < 0.0f) ? AN : 0;
	// AZ
	sharc.astat |= (IS_FLOAT_ZERO(r.r)) ? AZ : 0;
	// AI
	sharc.astat |= (IS_FLOAT_NAN(REG(rx))) ? AI : 0;

	FREG(rn) = r.f;
	sharc.astat |= AF;
}

/* Fn = ABS Fx */
INLINE void compute_fabs(int rn, int rx)
{
	SHARC_REG r;
	r.f = fabs(FREG(rx));

	CLEAR_ALU_FLAGS();
	// AN
	sharc.astat |= (r.f < 0.0f) ? AN : 0;
	// AZ
	sharc.astat |= (IS_FLOAT_ZERO(r.r)) ? AZ : 0;
	// AI
	sharc.astat |= (IS_FLOAT_NAN(REG(rx))) ? AI : 0;

	FREG(rn) = r.f;
	sharc.astat |= AF;
}

/*****************************************************************************/
/* Multiplier opcodes */

/* Rn = (unsigned)Rx * (unsigned)Ry, integer, no rounding */
INLINE void compute_mul_uuin(int rn, int rx, int ry)
{
	UINT64 r = (UINT64)(UINT32)REG(rx) * (UINT64)(UINT32)REG(ry);

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN((UINT32)r);
	SET_FLAG_MV(r);
	SET_FLAG_MU(r);

	REG(rn) = (UINT32)(r);
}

/* Rn = (signed)Rx * (signed)Ry, integer, no rounding */
INLINE void compute_mul_ssin(int rn, int rx, int ry)
{
	UINT64 r = (INT64)(INT32)REG(rx) * (INT64)(INT32)REG(ry);

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN((UINT32)r);
	SET_FLAG_MV(r);
	SET_FLAG_MU(r);

	REG(rn) = (UINT32)(r);
}

/* MRF + (signed)Rx * (signed)Ry, integer, no rounding */
INLINE UINT32 compute_mrf_plus_mul_ssin(int rx, int ry)
{
	UINT64 r = sharc.mrf + ((INT64)(INT32)REG(rx) * (INT64)(INT32)REG(ry));

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN((UINT32)r);
	SET_FLAG_MV(r);
	SET_FLAG_MU(r);

	return (UINT32)(r);
}

/* MRB + (signed)Rx * (signed)Ry, integer, no rounding */
INLINE UINT32 compute_mrb_plus_mul_ssin(int rx, int ry)
{
	INT64 r = sharc.mrb + ((INT64)(INT32)REG(rx) * (INT64)(INT32)REG(ry));

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN((UINT32)r);
	SET_FLAG_MV(r);
	SET_FLAG_MU(r);

	return (UINT32)(r);
}

/* Fn = Fx * Fy */
INLINE void compute_fmul(int rn, int rx, int ry)
{
	FREG(rn) = FREG(rx) * FREG(ry);

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN(REG(rn));
	/* TODO: MV flag */
	/* TODO: MU flag */
	/* TODO: MI flag */
}

/*****************************************************************************/

/* multi function opcodes */

/* integer*/
INLINE void compute_multi_mr_to_reg(int ai, int rk)
{
	switch(ai)
	{
		case 0:		SET_UREG(rk, (UINT32)(sharc.mrf)); break;
		case 1:		SET_UREG(rk, (UINT32)(sharc.mrf >> 32)); break;
		case 2:		fatalerror("SHARC: tried to load MR2F"); break;
		case 4:		SET_UREG(rk, (UINT32)(sharc.mrb)); break;
		case 5:		SET_UREG(rk, (UINT32)(sharc.mrb >> 32)); break;
		case 6:		fatalerror("SHARC: tried to load MR2B"); break;
		default:	fatalerror("SHARC: unknown ai %d in mr_to_reg", ai);
	}

	CLEAR_MULTIPLIER_FLAGS();
}

INLINE void compute_multi_reg_to_mr(int ai, int rk)
{
	switch(ai)
	{
		case 0:		sharc.mrf &= ~0xffffffff; sharc.mrf |= GET_UREG(rk); break;
		case 1:		sharc.mrf &= 0xffffffff; sharc.mrf |= (UINT64)(GET_UREG(rk)) << 32; break;
		case 2:		fatalerror("SHARC: tried to write MR2F"); break;
		case 4:		sharc.mrb &= ~0xffffffff; sharc.mrb |= GET_UREG(rk); break;
		case 5:		sharc.mrb &= 0xffffffff; sharc.mrb |= (UINT64)(GET_UREG(rk)) << 32; break;
		case 6:		fatalerror("SHARC: tried to write MR2B"); break;
		default:	fatalerror("SHARC: unknown ai %d in reg_to_mr", ai);
	}

	CLEAR_MULTIPLIER_FLAGS();
}

/* Ra = Rx + Ry,   Rs = Rx - Ry */
INLINE void compute_dual_add_sub(int ra, int rs, int rx, int ry)
{
	UINT32 r_add = REG(rx) + REG(ry);
	UINT32 r_sub = REG(rx) - REG(ry);

	CLEAR_ALU_FLAGS();
	if (r_add == 0 || r_sub == 0)
	{
		sharc.astat |= AZ;
	}
	if (r_add & 0x80000000 || r_sub & 0x80000000)
	{
		sharc.astat |= AN;
	}
	if (((~(REG(rx) ^ REG(ry)) & (REG(rx) ^ r_add)) & 0x80000000) ||
		(( (REG(rx) ^ REG(ry)) & (REG(rx) ^ r_sub)) & 0x80000000))
	{
		sharc.astat |= AV;
	}
	if (((UINT32)r_add < (UINT32)REG(rx)) ||
		(!((UINT32)r_sub < (UINT32)REG(rx))))
	{
		sharc.astat |= AC;
	}

	REG(ra) = r_add;
	REG(rs) = r_sub;

	sharc.astat &= ~AF;
}

/* Rm = (signed)Rxm * (signed)Rym, fractional, rounding,   Ra = Rxa + Rya */
INLINE void compute_mul_ssfr_add(int rm, int rxm, int rym, int ra, int rxa, int rya)
{
	UINT32 r_mul = (UINT32)(((INT64)(REG(rxm)) * (INT64)(REG(rym))) >> 31);
	UINT32 r_add = REG(rxa) + REG(rya);

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN(r_mul);
	/* TODO: MV flag */
	/* TODO: MU flag */
	/* TODO: MI flag */

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r_add);
	SET_FLAG_AZ(r_add);
	SET_FLAG_AV_ADD(r_add, REG(rxa), REG(rya));
	SET_FLAG_AC_ADD(r_add, REG(rxa), REG(rya));


	REG(rm) = r_mul;
	REG(ra) = r_add;

	sharc.astat &= ~AF;
}

/* Rm = (signed)Rxm * (signed)Rym, fractional, rounding,   Ra = Rxa - Rya */
INLINE void compute_mul_ssfr_sub(int rm, int rxm, int rym, int ra, int rxa, int rya)
{
	UINT32 r_mul = (UINT32)(((INT64)(REG(rxm)) * (INT64)(REG(rym))) >> 31);
	UINT32 r_sub = REG(rxa) - REG(rya);

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN(r_mul);
	/* TODO: MV flag */
	/* TODO: MU flag */
	/* TODO: MI flag */

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r_sub);
	SET_FLAG_AZ(r_sub);
	SET_FLAG_AV_SUB(r_sub, REG(rxa), REG(rya));
	SET_FLAG_AC_SUB(r_sub, REG(rxa), REG(rya));


	REG(rm) = r_mul;
	REG(ra) = r_sub;

	sharc.astat &= ~AF;
}


/* floating-point */

/* Fa = Fx + Fy,   Fs = Fx - Fy */
INLINE void compute_dual_fadd_fsub(int ra, int rs, int rx, int ry)
{
	SHARC_REG r_add, r_sub;
	r_add.f = FREG(rx) + FREG(ry);
	r_sub.f = FREG(rx) - FREG(ry);

	CLEAR_ALU_FLAGS();
	// AN
	sharc.astat |= ((r_add.f < 0.0f) || (r_sub.f < 0.0f)) ? AN : 0;
	// AZ
	sharc.astat |= (IS_FLOAT_DENORMAL(r_add.r) || IS_FLOAT_ZERO(r_add.r) ||
					IS_FLOAT_DENORMAL(r_sub.r) || IS_FLOAT_ZERO(r_sub.r)) ? AZ : 0;
	// AUS
	sharc.stky |= (IS_FLOAT_DENORMAL(r_add.r) || IS_FLOAT_DENORMAL(r_sub.r)) ? AUS : 0;
	// AI
	sharc.astat |= (IS_FLOAT_NAN(REG(rx)) || IS_FLOAT_NAN(REG(ry))) ? AI : 0;
	/* TODO: AV flag */

	// AIS
	if (sharc.astat & AI)	sharc.stky |= AIS;

	FREG(ra) = r_add.f;
	FREG(rs) = r_sub.f;
	sharc.astat |= AF;
}

/* Fm = Fxm * Fym,   Fa = Fxa + Fya */
INLINE void compute_fmul_fadd(int fm, int fxm, int fym, int fa, int fxa, int fya)
{
	SHARC_REG r_mul, r_add;
	r_mul.f = FREG(fxm) * FREG(fym);
	r_add.f = FREG(fxa) + FREG(fya);

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN(r_mul.r);
	/* TODO: MV flag */
	/* TODO: MU flag */
	/* TODO: MI flag */

	CLEAR_ALU_FLAGS();
	sharc.astat |= (r_add.f < 0.0f) ? AN : 0;
	// AZ
	sharc.astat |= (IS_FLOAT_DENORMAL(r_add.r) || IS_FLOAT_ZERO(r_add.r)) ? AZ : 0;
	// AU
	sharc.stky |= (IS_FLOAT_DENORMAL(r_add.r)) ? AUS : 0;
	// AI
	sharc.astat |= (IS_FLOAT_NAN(REG(fxa)) || IS_FLOAT_NAN(REG(fya))) ? AI : 0;
	/* TODO: AV flag */

	// AIS
	if (sharc.astat & AI)	sharc.stky |= AIS;

	FREG(fm) = r_mul.f;
	FREG(fa) = r_add.f;
	sharc.astat |= AF;
}

/* Fm = Fxm * Fym,   Fa = Fxa - Fya */
INLINE void compute_fmul_fsub(int fm, int fxm, int fym, int fa, int fxa, int fya)
{
	SHARC_REG r_mul, r_sub;
	r_mul.f = FREG(fxm) * FREG(fym);
	r_sub.f = FREG(fxa) - FREG(fya);

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN(r_mul.r);
	/* TODO: MV flag */
	/* TODO: MU flag */
	/* TODO: MI flag */

	CLEAR_ALU_FLAGS();
	sharc.astat |= (r_sub.f < 0.0f) ? AN : 0;
	// AZ
	sharc.astat |= (IS_FLOAT_DENORMAL(r_sub.r) || IS_FLOAT_ZERO(r_sub.r)) ? AZ : 0;
	// AU
	sharc.stky |= (IS_FLOAT_DENORMAL(r_sub.r)) ? AUS : 0;
	// AI
	sharc.astat |= (IS_FLOAT_NAN(REG(fxa)) || IS_FLOAT_NAN(REG(fya))) ? AI : 0;
	/* TODO: AV flag */

	// AIS
	if (sharc.astat & AI)	sharc.stky |= AIS;

	FREG(fm) = r_mul.f;
	FREG(fa) = r_sub.f;
	sharc.astat |= AF;
}

/* Fm = Fxm * Fym,   Fa = FLOAT Fxa BY Fya */
INLINE void compute_fmul_float_scaled(int fm, int fxm, int fym, int fa, int fxa, int fya)
{
	SHARC_REG x;
	SHARC_REG r_mul, r_alu;
	r_mul.f = FREG(fxm) * FREG(fym);

	x.f = (float)(INT32)REG(fxa);

	r_alu.r = SCALB(x, fya);

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN(r_mul.r);
	/* TODO: MV flag */
	/* TODO: MU flag */
	/* TODO: MI flag */

	CLEAR_ALU_FLAGS();
	sharc.astat |= (r_alu.f < 0.0f) ? AN : 0;
	// AZ
	sharc.astat |= (IS_FLOAT_DENORMAL(r_alu.r) || IS_FLOAT_ZERO(r_alu.r)) ? AZ : 0;
	// AU
	sharc.stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	/* TODO: set AV if overflowed */

	FREG(fm) = r_mul.f;
	FREG(fa) = r_alu.f;
	sharc.astat |= AF;
}

/* Fm = Fxm * Fym,   Fa = FIX Fxa BY Fya */
INLINE void compute_fmul_fix_scaled(int fm, int fxm, int fym, int fa, int fxa, int fya)
{
	INT32 alu_i;
	SHARC_REG r_mul, r_alu;
	r_mul.f = FREG(fxm) * FREG(fym);

	r_alu.r = SCALB(sharc.r[fxa], fya);

	if (sharc.mode1 & MODE1_TRUNCATE)
	{
		alu_i = (INT32)(r_alu.f);
	}
	else
	{
		alu_i = (INT32)(r_alu.f < 0 ? (r_alu.f - 0.5f) : (r_alu.f + 0.5f));
	}

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN(r_mul.r);
	/* TODO: MV flag */
	/* TODO: MU flag */
	/* TODO: MI flag */

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(alu_i);
	// AZ
	SET_FLAG_AZ(alu_i);
	// AU
	sharc.stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	// AI
	sharc.astat |= (IS_FLOAT_NAN(REG(fxa))) ? AI : 0;
	/* TODO: AV flag */

	FREG(fm) = r_mul.f;
	REG(fa) = alu_i;
	sharc.astat |= AF;
}


/* Fm = Fxm * Fym,   Fa = MAX(Fxa, Fya) */
INLINE void compute_fmul_fmax(int fm, int fxm, int fym, int fa, int fxa, int fya)
{
	SHARC_REG r_mul, r_alu;
	r_mul.f = FREG(fxm) * FREG(fym);

	r_alu.f = MAX(FREG(fxa), FREG(fya));

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN(r_mul.r);
	/* TODO: MV flag */
	/* TODO: MU flag */
	/* TODO: MI flag */

	CLEAR_ALU_FLAGS();
	sharc.astat |= (r_alu.f < 0.0f) ? AN : 0;
	// AZ
	sharc.astat |= (IS_FLOAT_ZERO(r_alu.r)) ? AZ : 0;
	// AU
	sharc.stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	// AI
	sharc.astat |= (IS_FLOAT_NAN(REG(fxa)) || IS_FLOAT_NAN(REG(fya))) ? AI : 0;
	/* TODO: AV flag */

	FREG(fm) = r_mul.f;
	FREG(fa) = r_alu.f;
	sharc.astat |= AF;
}


/* Fm = Fxm * Fym,   Fa = MIN(Fxa, Fya) */
INLINE void compute_fmul_fmin(int fm, int fxm, int fym, int fa, int fxa, int fya)
{
	SHARC_REG r_mul, r_alu;
	r_mul.f = FREG(fxm) * FREG(fym);

	r_alu.f = MIN(FREG(fxa), FREG(fya));

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN(r_mul.r);
	/* TODO: MV flag */
	/* TODO: MU flag */
	/* TODO: MI flag */

	CLEAR_ALU_FLAGS();
	sharc.astat |= (r_alu.f < 0.0f) ? AN : 0;
	// AZ
	sharc.astat |= (IS_FLOAT_ZERO(r_alu.r)) ? AZ : 0;
	// AU
	sharc.stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	// AI
	sharc.astat |= (IS_FLOAT_NAN(REG(fxa)) || IS_FLOAT_NAN(REG(fya))) ? AI : 0;
	/* TODO: AV flag */

	FREG(fm) = r_mul.f;
	FREG(fa) = r_alu.f;
	sharc.astat |= AF;
}



/* Fm = Fxm * Fym,   Fa = Fxa + Fya,   Fs = Fxa - Fya */
INLINE void compute_fmul_dual_fadd_fsub(int fm, int fxm, int fym, int fa, int fs, int fxa, int fya)
{
	SHARC_REG r_mul, r_add, r_sub;
	r_mul.f = FREG(fxm) * FREG(fym);
	r_add.f = FREG(fxa) + FREG(fya);
	r_sub.f = FREG(fxa) - FREG(fya);

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN(r_mul.r);
	/* TODO: MV flag */
	/* TODO: MU flag */
	/* TODO: MI flag */

	CLEAR_ALU_FLAGS();
	// AN
	sharc.astat |= ((r_add.r < 0.0f) || (r_sub.r < 0.0f)) ? AN : 0;
	// AZ
	sharc.astat |= (IS_FLOAT_DENORMAL(r_add.r) || IS_FLOAT_ZERO(r_add.r) ||
					IS_FLOAT_DENORMAL(r_sub.r) || IS_FLOAT_ZERO(r_sub.r)) ? AZ : 0;
	// AUS
	sharc.stky |= (IS_FLOAT_DENORMAL(r_add.r) || IS_FLOAT_DENORMAL(r_sub.r)) ? AUS : 0;
	// AI
	sharc.astat |= (IS_FLOAT_NAN(REG(fxa)) || IS_FLOAT_NAN(REG(fya))) ? AI : 0;
	/* TODO: AV flag */

	// AIS
	if (sharc.astat & AI)	sharc.stky |= AIS;

	FREG(fm) = r_mul.f;
	FREG(fa) = r_add.f;
	FREG(fs) = r_sub.f;
	sharc.astat |= AF;
}
