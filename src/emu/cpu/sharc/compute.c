/* compute operations */

#include <math.h>

#define CLEAR_ALU_FLAGS()       (cpustate->astat &= ~(AZ|AN|AV|AC|AS|AI))

#define SET_FLAG_AZ(r)          { cpustate->astat |= (((r) == 0) ? AZ : 0); }
#define SET_FLAG_AN(r)          { cpustate->astat |= (((r) & 0x80000000) ? AN : 0); }
#define SET_FLAG_AC_ADD(r,a,b)  { cpustate->astat |= (((UINT32)r < (UINT32)a) ? AC : 0); }
#define SET_FLAG_AV_ADD(r,a,b)  { cpustate->astat |= (((~((a) ^ (b)) & ((a) ^ (r))) & 0x80000000) ? AV : 0); }
#define SET_FLAG_AC_SUB(r,a,b)  { cpustate->astat |= ((!((UINT32)a < (UINT32)b)) ? AC : 0); }
#define SET_FLAG_AV_SUB(r,a,b)  { cpustate->astat |= ((( ((a) ^ (b)) & ((a) ^ (r))) & 0x80000000) ? AV : 0); }

#define IS_FLOAT_ZERO(r)        ((((r) & 0x7fffffff) == 0))
#define IS_FLOAT_DENORMAL(r)    ((((r) & 0x7f800000) == 0) && (((r) & 0x7fffff) != 0))
#define IS_FLOAT_NAN(r)         ((((r) & 0x7f800000) == 0x7f800000) && (((r) & 0x7fffff) != 0))
#define IS_FLOAT_INFINITY(r)    (((r) & 0x7fffffff) == 0x7f800000)

#define CLEAR_MULTIPLIER_FLAGS()    (cpustate->astat &= ~(MN|MV|MU|MI))

#define SET_FLAG_MN(r)          { cpustate->astat |= (((r) & 0x80000000) ? MN : 0); }
#define SET_FLAG_MV(r)          { cpustate->astat |= ((((UINT32)((r) >> 32) != 0) && ((UINT32)((r) >> 32) != 0xffffffff)) ? MV : 0); }

/* TODO: MU needs 80-bit result */
#define SET_FLAG_MU(r)          { cpustate->astat |= ((((UINT32)((r) >> 32) == 0) && ((UINT32)(r)) != 0) ? MU : 0); }


#define FLOAT_SIGN          0x80000000
#define FLOAT_INFINITY      0x7f800000
#define FLOAT_MANTISSA      0x007fffff

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
INLINE void compute_add(SHARC_REGS *cpustate, int rn, int rx, int ry)
{
	UINT32 r = REG(rx) + REG(ry);

	if (cpustate->mode1 & MODE1_ALUSAT)
		fatalerror("SHARC: compute_add: ALU saturation not implemented!\n");

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	SET_FLAG_AV_ADD(r, REG(rx), REG(ry));
	SET_FLAG_AC_ADD(r, REG(rx), REG(ry));
	REG(rn) = r;

	cpustate->astat &= ~AF;
}

/* Rn = Rx - Ry */
INLINE void compute_sub(SHARC_REGS *cpustate, int rn, int rx, int ry)
{
	UINT32 r = REG(rx) - REG(ry);

	if (cpustate->mode1 & MODE1_ALUSAT)
		fatalerror("SHARC: compute_sub: ALU saturation not implemented!\n");

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	SET_FLAG_AV_SUB(r, REG(rx), REG(ry));
	SET_FLAG_AC_SUB(r, REG(rx), REG(ry));
	REG(rn) = r;

	cpustate->astat &= ~AF;
}

/* Rn = Rx + Ry + CI */
INLINE void compute_add_ci(SHARC_REGS *cpustate, int rn, int rx, int ry)
{
	int c = (cpustate->astat & AC) ? 1 : 0;
	UINT32 r = REG(rx) + REG(ry) + c;

	if (cpustate->mode1 & MODE1_ALUSAT)
		fatalerror("SHARC: compute_add_ci: ALU saturation not implemented!\n");

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	SET_FLAG_AV_ADD(r, REG(rx), REG(ry)+c);
	SET_FLAG_AC_ADD(r, REG(rx), REG(ry)+c);
	REG(rn) = r;

	cpustate->astat &= ~AF;
}

/* Rn = Rx - Ry + CI - 1 */
INLINE void compute_sub_ci(SHARC_REGS *cpustate, int rn, int rx, int ry)
{
	int c = (cpustate->astat & AC) ? 1 : 0;
	UINT32 r = REG(rx) - REG(ry) + c - 1;

	if (cpustate->mode1 & MODE1_ALUSAT)
		fatalerror("SHARC: compute_sub_ci: ALU saturation not implemented!\n");

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	SET_FLAG_AV_SUB(r, REG(rx), REG(ry)+c-1);
	SET_FLAG_AC_SUB(r, REG(rx), REG(ry)+c-1);
	REG(rn) = r;

	cpustate->astat &= ~AF;
}

/* Rn = Rx AND Ry */
INLINE void compute_and(SHARC_REGS *cpustate, int rn, int rx, int ry)
{
	UINT32 r = REG(rx) & REG(ry);

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	REG(rn) = r;

	cpustate->astat &= ~AF;
}

/* COMP(Rx, Ry) */
INLINE void compute_comp(SHARC_REGS *cpustate, int rx, int ry)
{
	UINT32 comp_accum;

	CLEAR_ALU_FLAGS();
	if( REG(rx) == REG(ry) )
		cpustate->astat |= AZ;
	if( (INT32)REG(rx) < (INT32)REG(ry) )
		cpustate->astat |= AN;

	// Update ASTAT compare accumulation register
	comp_accum = (cpustate->astat >> 24) & 0xff;
	comp_accum >>= 1;
	if ((cpustate->astat & (AZ|AN)) == 0)
	{
		comp_accum |= 0x80;
	}
	cpustate->astat &= 0xffffff;
	cpustate->astat |= comp_accum << 24;

	cpustate->astat &= ~AF;
}

/* Rn = PASS Rx */
INLINE void compute_pass(SHARC_REGS *cpustate, int rn, int rx)
{
	CLEAR_ALU_FLAGS();
	/* TODO: floating-point extension field is set to 0 */

	REG(rn) = REG(rx);
	if (REG(rn) == 0)
		cpustate->astat |= AZ;
	if (REG(rn) & 0x80000000)
		cpustate->astat |= AN;

	cpustate->astat &= ~AF;
}

/* Rn = Rx XOR Ry */
INLINE void compute_xor(SHARC_REGS *cpustate, int rn, int rx, int ry)
{
	UINT32 r = REG(rx) ^ REG(ry);
	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	REG(rn) = r;

	cpustate->astat &= ~AF;
}

/* Rn = Rx OR Ry */
INLINE void compute_or(SHARC_REGS *cpustate, int rn, int rx, int ry)
{
	UINT32 r = REG(rx) | REG(ry);
	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	REG(rn) = r;

	cpustate->astat &= ~AF;
}

/* Rn = Rx + 1 */
INLINE void compute_inc(SHARC_REGS *cpustate, int rn, int rx)
{
	UINT32 r = REG(rx) + 1;

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	SET_FLAG_AV_ADD(r, REG(rx), 1);
	SET_FLAG_AC_ADD(r, REG(rx), 1);

	REG(rn) = r;

	cpustate->astat &= ~AF;
}

/* Rn = Rx - 1 */
INLINE void compute_dec(SHARC_REGS *cpustate, int rn, int rx)
{
	UINT32 r = REG(rx) - 1;

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	SET_FLAG_AV_SUB(r, REG(rx), 1);
	SET_FLAG_AC_SUB(r, REG(rx), 1);

	REG(rn) = r;

	cpustate->astat &= ~AF;
}

/* Rn = MIN(Rx, Ry) */
INLINE void compute_min(SHARC_REGS *cpustate, int rn, int rx, int ry)
{
	UINT32 r = MIN((INT32)REG(rx), (INT32)REG(ry));

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);

	REG(rn) = r;

	cpustate->astat &= ~AF;
}

/* Rn = MAX(Rx, Ry) */
INLINE void compute_max(SHARC_REGS *cpustate, int rn, int rx, int ry)
{
	UINT32 r = MAX((INT32)REG(rx), (INT32)REG(ry));

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);

	REG(rn) = r;

	cpustate->astat &= ~AF;
}

/* Rn = -Rx */
INLINE void compute_neg(SHARC_REGS *cpustate, int rn, int rx)
{
	UINT32 r = -(INT32)(REG(rx));

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	SET_FLAG_AV_SUB(r, 0, REG(rx));
	SET_FLAG_AC_SUB(r, 0, REG(rx));

	REG(rn) = r;

	cpustate->astat &= ~AF;
}

/* Rn = NOT Rx */
INLINE void compute_not(SHARC_REGS *cpustate, int rn, int rx)
{
	UINT32 r = ~REG(rx);

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);

	REG(rn) = r;

	cpustate->astat &= ~AF;
}

/*****************************************************************************/
/* Floating-point ALU operations */

INLINE UINT32 SCALB(SHARC_REGS *cpustate, SHARC_REG rx, int ry)
{
	UINT32 mantissa = rx.r & FLOAT_MANTISSA;
	UINT32 sign = rx.r & FLOAT_SIGN;

	int exponent = ((rx.r >> 23) & 0xff) - 127;
	exponent += (INT32)(REG(ry));

	if (exponent > 127)
	{
		// overflow
		cpustate->astat |= AV;
		return sign | FLOAT_INFINITY;
	}
	else if (exponent < -126)
	{
		// denormal
		cpustate->astat |= AZ;
		return sign;
	}
	else
	{
		return sign | (((exponent + 127) & 0xff) << 23) | mantissa;
	}
}

/* Fn = FLOAT Rx */
INLINE void compute_float(SHARC_REGS *cpustate, int rn, int rx)
{
	// verified
	FREG(rn) = (float)(INT32)REG(rx);

	CLEAR_ALU_FLAGS();
	// AN
	SET_FLAG_AN(REG(rn));
	// AZ
	cpustate->astat |= (IS_FLOAT_DENORMAL(REG(rn)) || IS_FLOAT_ZERO(REG(rn))) ? AZ : 0;
	// AUS
	cpustate->stky |= (IS_FLOAT_DENORMAL(REG(rn))) ? AUS : 0;
	/* TODO: AV flag */

	cpustate->astat |= AF;
}

/* Rn = FIX Fx */
INLINE void compute_fix(SHARC_REGS *cpustate, int rn, int rx)
{
	INT32 alu_i;
	SHARC_REG r_alu;

	r_alu.f = FREG(rx);
	if (cpustate->mode1 & MODE1_TRUNCATE)
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
	cpustate->stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	// AI
	cpustate->astat |= (IS_FLOAT_NAN(REG(rx))) ? AI : 0;
	/* TODO: AV flag */

	REG(rn) = alu_i;
	cpustate->astat |= AF;
}

/* Rn = FIX Fx BY Ry */
INLINE void compute_fix_scaled(SHARC_REGS *cpustate, int rn, int rx, int ry)
{
	INT32 alu_i;
	SHARC_REG r_alu;

	r_alu.r = SCALB(cpustate, cpustate->r[rx], ry);
	if (cpustate->mode1 & MODE1_TRUNCATE)
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
	cpustate->stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	// AI
	cpustate->astat |= (IS_FLOAT_NAN(REG(rx))) ? AI : 0;
	/* TODO: AV flag */

	REG(rn) = alu_i;
	cpustate->astat |= AF;
}

/* Fn = FLOAT Rx BY Ry */
INLINE void compute_float_scaled(SHARC_REGS *cpustate, int rn, int rx, int ry)
{
	SHARC_REG x;
	x.f = (float)(INT32)(REG(rx));

	// verified
	CLEAR_ALU_FLAGS();

	REG(rn) = SCALB(cpustate, x, ry);

	// AN
	SET_FLAG_AN(REG(rn));
	// AZ
	cpustate->astat |= (IS_FLOAT_DENORMAL(REG(rn)) || IS_FLOAT_ZERO(REG(rn))) ? AZ : 0;
	// AU
	cpustate->stky |= (IS_FLOAT_DENORMAL(REG(rn))) ? AUS : 0;

	cpustate->astat |= AF;
}

/* Rn = LOGB Fx */
INLINE void compute_logb(SHARC_REGS *cpustate, int rn, int rx)
{
	// verified
	UINT32 r = REG(rx);

	CLEAR_ALU_FLAGS();

	if (IS_FLOAT_INFINITY(REG(rx)))
	{
		REG(rn) = FLOAT_INFINITY;

		cpustate->astat |= AV;
	}
	else if (IS_FLOAT_ZERO(REG(rx)))
	{
		REG(rn) = FLOAT_SIGN | FLOAT_INFINITY;

		cpustate->astat |= AV;
	}
	else if (IS_FLOAT_NAN(REG(rx)))
	{
		REG(rn) = 0xffffffff;

		cpustate->astat |= AI;
		cpustate->stky |= AIS;
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
	cpustate->astat |= AF;
}

/* Fn = SCALB Fx BY Fy */
INLINE void compute_scalb(SHARC_REGS *cpustate, int rn, int rx, int ry)
{
	// verified
	SHARC_REG r;
	CLEAR_ALU_FLAGS();

	if (IS_FLOAT_NAN(REG(rx)))
	{
		cpustate->astat |= AI;
		cpustate->stky |= AIS;

		REG(rn) = 0xffffffff;
	}
	else
	{
		r.r = SCALB(cpustate, cpustate->r[rx], ry);

		// AN
		SET_FLAG_AN(r.r);
		// AZ
		cpustate->astat |= IS_FLOAT_ZERO(r.r) ? AZ : 0;
		// AUS
		cpustate->stky |= (IS_FLOAT_DENORMAL(r.r)) ? AUS : 0;

		FREG(rn) = r.f;
	}
	cpustate->astat |= AF;
}

/* Fn = Fx + Fy */
INLINE void compute_fadd(SHARC_REGS *cpustate, int rn, int rx, int ry)
{
	SHARC_REG r;
	r.f = FREG(rx) + FREG(ry);

	CLEAR_ALU_FLAGS();
	// AN
	cpustate->astat |= (r.f < 0.0f) ? AN : 0;
	// AZ
	cpustate->astat |= (IS_FLOAT_DENORMAL(r.r) || IS_FLOAT_ZERO(r.r)) ? AZ : 0;
	// AUS
	cpustate->stky |= (IS_FLOAT_DENORMAL(r.r)) ? AUS : 0;
	// AI
	cpustate->astat |= (IS_FLOAT_NAN(REG(rx)) || IS_FLOAT_NAN(REG(ry))) ? AI : 0;
	/* TODO: AV flag */

	// AIS
	if (cpustate->astat & AI)   cpustate->stky |= AIS;

	FREG(rn) = r.f;
	cpustate->astat |= AF;
}

/* Fn = Fx - Fy */
INLINE void compute_fsub(SHARC_REGS *cpustate, int rn, int rx, int ry)
{
	SHARC_REG r;
	r.f = FREG(rx) - FREG(ry);

	CLEAR_ALU_FLAGS();
	// AN
	cpustate->astat |= (r.f < 0.0f) ? AN : 0;
	// AZ
	cpustate->astat |= (IS_FLOAT_DENORMAL(r.r) || IS_FLOAT_ZERO(r.r)) ? AZ : 0;
	// AUS
	cpustate->stky |= (IS_FLOAT_DENORMAL(r.r)) ? AUS : 0;
	// AI
	cpustate->astat |= (IS_FLOAT_NAN(REG(rx)) || IS_FLOAT_NAN(REG(ry))) ? AI : 0;
	/* TODO: AV flag */

	// AIS
	if (cpustate->astat & AI)   cpustate->stky |= AIS;

	FREG(rn) = r.f;
	cpustate->astat |= AF;
}

/* Fn = -Fx */
INLINE void compute_fneg(SHARC_REGS *cpustate, int rn, int rx)
{
	SHARC_REG r;
	r.f = -FREG(rx);

	CLEAR_ALU_FLAGS();
	// AZ
	cpustate->astat |= (IS_FLOAT_ZERO(r.r)) ? AZ : 0;
	// AN
	cpustate->astat |= (r.f < 0.0f) ? AN : 0;
	// AI
	cpustate->astat |= (IS_FLOAT_NAN(REG(rx))) ? AI : 0;

	// AIS
	if (cpustate->astat & AI)   cpustate->stky |= AIS;

	FREG(rn) = r.f;
	cpustate->astat |= AF;
}

/* COMP(Fx, Fy) */
INLINE void compute_fcomp(SHARC_REGS *cpustate, int rx, int ry)
{
	UINT32 comp_accum;

	CLEAR_ALU_FLAGS();
	// AZ
	if( FREG(rx) == FREG(ry) )
		cpustate->astat |= AZ;
	// AN
	if( FREG(rx) < FREG(ry) )
		cpustate->astat |= AN;
	// AI
	cpustate->astat |= (IS_FLOAT_NAN(REG(rx)) || IS_FLOAT_NAN(REG(ry))) ? AI : 0;

	// AIS
	if (cpustate->astat & AI)   cpustate->stky |= AIS;

	// Update ASTAT compare accumulation register
	comp_accum = (cpustate->astat >> 24) & 0xff;
	comp_accum >>= 1;
	if ((cpustate->astat & (AZ|AN)) == 0)
	{
		comp_accum |= 0x80;
	}
	cpustate->astat &= 0xffffff;
	cpustate->astat |= comp_accum << 24;
	cpustate->astat |= AF;
}

/* Fn = ABS(Fx + Fy) */
INLINE void compute_fabs_plus(SHARC_REGS *cpustate, int rn, int rx, int ry)
{
	SHARC_REG r;
	r.f = fabs(FREG(rx) + FREG(ry));

	CLEAR_ALU_FLAGS();
	// AZ
	cpustate->astat |= (IS_FLOAT_DENORMAL(r.r) || IS_FLOAT_ZERO(r.r)) ? AZ : 0;
	// AUS
	cpustate->stky |= (IS_FLOAT_DENORMAL(r.r)) ? AUS : 0;
	// AI
	cpustate->astat |= (IS_FLOAT_NAN(REG(rx)) || IS_FLOAT_NAN(REG(ry))) ? AI : 0;
	/* TODO: AV flag */

	// AIS
	if (cpustate->astat & AI)   cpustate->stky |= AIS;

	FREG(rn) = r.f;
	cpustate->astat |= AF;
}

/* Fn = MAX(Fx, Fy) */
INLINE void compute_fmax(SHARC_REGS *cpustate, int rn, int rx, int ry)
{
	SHARC_REG r_alu;

	r_alu.f = MAX(FREG(rx), FREG(ry));

	CLEAR_ALU_FLAGS();
	cpustate->astat |= (r_alu.f < 0.0f) ? AN : 0;
	// AZ
	cpustate->astat |= (IS_FLOAT_ZERO(r_alu.r)) ? AZ : 0;
	// AU
	cpustate->stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	// AI
	cpustate->astat |= (IS_FLOAT_NAN(REG(rx)) || IS_FLOAT_NAN(REG(ry))) ? AI : 0;
	/* TODO: AV flag */

	FREG(rn) = r_alu.f;
	cpustate->astat |= AF;
}

/* Fn = MIN(Fx, Fy) */
INLINE void compute_fmin(SHARC_REGS *cpustate, int rn, int rx, int ry)
{
	SHARC_REG r_alu;

	r_alu.f = MIN(FREG(rx), FREG(ry));

	CLEAR_ALU_FLAGS();
	cpustate->astat |= (r_alu.f < 0.0f) ? AN : 0;
	// AZ
	cpustate->astat |= (IS_FLOAT_ZERO(r_alu.r)) ? AZ : 0;
	// AU
	cpustate->stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	// AI
	cpustate->astat |= (IS_FLOAT_NAN(REG(rx)) || IS_FLOAT_NAN(REG(ry))) ? AI : 0;
	/* TODO: AV flag */

	FREG(rn) = r_alu.f;
	cpustate->astat |= AF;
}

/* Fn = CLIP Fx BY Fy */
INLINE void compute_fclip(SHARC_REGS *cpustate, int rn, int rx, int ry)
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
	cpustate->astat |= (IS_FLOAT_ZERO(r_alu.r)) ? AZ : 0;
	// AU
	cpustate->stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	// AI
	cpustate->astat |= (IS_FLOAT_NAN(REG(rx)) || IS_FLOAT_NAN(REG(ry))) ? AI : 0;

	FREG(rn) = r_alu.f;
	cpustate->astat |= AF;
}

/* Fn = RECIPS Fx */
INLINE void compute_recips(SHARC_REGS *cpustate, int rn, int rx)
{
	// verified
	UINT32 r;

	CLEAR_ALU_FLAGS();

	if (IS_FLOAT_NAN(REG(rx)))
	{
		// NaN
		r = 0xffffffff;

		// AI
		cpustate->astat |= AI;

		// AIS
		cpustate->stky |= AIS;
	}
	else if (IS_FLOAT_ZERO(REG(rx)))
	{
		// +- Zero
		r = (REG(rx) & FLOAT_SIGN) | FLOAT_INFINITY;

		cpustate->astat |= AZ;
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
		cpustate->astat |= (IS_FLOAT_ZERO(r)) ? AZ : 0;
		cpustate->astat |= (IS_FLOAT_ZERO(REG(rx))) ? AV : 0;
		// AI
		cpustate->astat |= (IS_FLOAT_NAN(REG(rx))) ? AI : 0;

		// AIS
		if (cpustate->astat & AI)   cpustate->stky |= AIS;
	}

	// AF
	cpustate->astat |= AF;

	REG(rn) = r;
}

/* Fn = RSQRTS Fx */
INLINE void compute_rsqrts(SHARC_REGS *cpustate, int rn, int rx)
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
		UINT32 mantissa = REG(rx) & 0xffffff;   // mantissa + LSB of biased exponent
		UINT32 exponent = (REG(rx) >> 23) & 0xff;
		UINT32 sign = REG(rx) & FLOAT_SIGN;

		UINT32 res_mantissa = rsqrts_mantissa_lookup[mantissa >> 17];

		int res_exponent = -((exponent - 127) / 2) - 1;
		res_exponent = (res_exponent + 127) & 0xff;

		r = sign | (res_exponent << 23) | res_mantissa;
	}

	CLEAR_ALU_FLAGS();
	// AN
	cpustate->astat |= (REG(rx) == 0x80000000) ? AN : 0;
	// AZ & AV
	cpustate->astat |= (IS_FLOAT_ZERO(r)) ? AZ : 0;
	cpustate->astat |= (IS_FLOAT_ZERO(REG(rx))) ? AV : 0;
	// AI
	cpustate->astat |= (IS_FLOAT_NAN(REG(rx)) || (REG(rx) & 0x80000000)) ? AI : 0;
	// AIS
	if (cpustate->astat & AI)   cpustate->stky |= AIS;
	// AF
	cpustate->astat |= AF;

	REG(rn) = r;
}


/* Fn = PASS Fx */
INLINE void compute_fpass(SHARC_REGS *cpustate, int rn, int rx)
{
	SHARC_REG r;
	r.f = FREG(rx);

	CLEAR_ALU_FLAGS();
	// AN
	cpustate->astat |= (r.f < 0.0f) ? AN : 0;
	// AZ
	cpustate->astat |= (IS_FLOAT_ZERO(r.r)) ? AZ : 0;
	// AI
	cpustate->astat |= (IS_FLOAT_NAN(REG(rx))) ? AI : 0;

	FREG(rn) = r.f;
	cpustate->astat |= AF;
}

/* Fn = ABS Fx */
INLINE void compute_fabs(SHARC_REGS *cpustate, int rn, int rx)
{
	SHARC_REG r;
	r.f = fabs(FREG(rx));

	CLEAR_ALU_FLAGS();
	// AN
	cpustate->astat |= (r.f < 0.0f) ? AN : 0;
	// AZ
	cpustate->astat |= (IS_FLOAT_ZERO(r.r)) ? AZ : 0;
	// AI
	cpustate->astat |= (IS_FLOAT_NAN(REG(rx))) ? AI : 0;

	FREG(rn) = r.f;
	cpustate->astat |= AF;
}

/*****************************************************************************/
/* Multiplier opcodes */

/* Rn = (unsigned)Rx * (unsigned)Ry, integer, no rounding */
INLINE void compute_mul_uuin(SHARC_REGS *cpustate, int rn, int rx, int ry)
{
	UINT64 r = (UINT64)(UINT32)REG(rx) * (UINT64)(UINT32)REG(ry);

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN((UINT32)r);
	SET_FLAG_MV(r);
	SET_FLAG_MU(r);

	REG(rn) = (UINT32)(r);
}

/* Rn = (signed)Rx * (signed)Ry, integer, no rounding */
INLINE void compute_mul_ssin(SHARC_REGS *cpustate, int rn, int rx, int ry)
{
	UINT64 r = (INT64)(INT32)REG(rx) * (INT64)(INT32)REG(ry);

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN((UINT32)r);
	SET_FLAG_MV(r);
	SET_FLAG_MU(r);

	REG(rn) = (UINT32)(r);
}

/* MRF + (signed)Rx * (signed)Ry, integer, no rounding */
INLINE UINT32 compute_mrf_plus_mul_ssin(SHARC_REGS *cpustate, int rx, int ry)
{
	UINT64 r = cpustate->mrf + ((INT64)(INT32)REG(rx) * (INT64)(INT32)REG(ry));

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN((UINT32)r);
	SET_FLAG_MV(r);
	SET_FLAG_MU(r);

	return (UINT32)(r);
}

/* MRB + (signed)Rx * (signed)Ry, integer, no rounding */
INLINE UINT32 compute_mrb_plus_mul_ssin(SHARC_REGS *cpustate, int rx, int ry)
{
	INT64 r = cpustate->mrb + ((INT64)(INT32)REG(rx) * (INT64)(INT32)REG(ry));

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN((UINT32)r);
	SET_FLAG_MV(r);
	SET_FLAG_MU(r);

	return (UINT32)(r);
}

/* Fn = Fx * Fy */
INLINE void compute_fmul(SHARC_REGS *cpustate, int rn, int rx, int ry)
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
INLINE void compute_multi_mr_to_reg(SHARC_REGS *cpustate, int ai, int rk)
{
	switch(ai)
	{
		case 0:     SET_UREG(cpustate, rk, (UINT32)(cpustate->mrf)); break;
		case 1:     SET_UREG(cpustate, rk, (UINT32)(cpustate->mrf >> 32)); break;
		case 2:     fatalerror("SHARC: tried to load MR2F\n"); break;
		case 4:     SET_UREG(cpustate, rk, (UINT32)(cpustate->mrb)); break;
		case 5:     SET_UREG(cpustate, rk, (UINT32)(cpustate->mrb >> 32)); break;
		case 6:     fatalerror("SHARC: tried to load MR2B\n"); break;
		default:    fatalerror("SHARC: unknown ai %d in mr_to_reg\n", ai);
	}

	CLEAR_MULTIPLIER_FLAGS();
}

INLINE void compute_multi_reg_to_mr(SHARC_REGS *cpustate, int ai, int rk)
{
	switch(ai)
	{
		case 0:     cpustate->mrf &= ~0xffffffff; cpustate->mrf |= GET_UREG(cpustate, rk); break;
		case 1:     cpustate->mrf &= 0xffffffff; cpustate->mrf |= (UINT64)(GET_UREG(cpustate, rk)) << 32; break;
		case 2:     fatalerror("SHARC: tried to write MR2F\n"); break;
		case 4:     cpustate->mrb &= ~0xffffffff; cpustate->mrb |= GET_UREG(cpustate, rk); break;
		case 5:     cpustate->mrb &= 0xffffffff; cpustate->mrb |= (UINT64)(GET_UREG(cpustate, rk)) << 32; break;
		case 6:     fatalerror("SHARC: tried to write MR2B\n"); break;
		default:    fatalerror("SHARC: unknown ai %d in reg_to_mr\n", ai);
	}

	CLEAR_MULTIPLIER_FLAGS();
}

/* Ra = Rx + Ry,   Rs = Rx - Ry */
INLINE void compute_dual_add_sub(SHARC_REGS *cpustate, int ra, int rs, int rx, int ry)
{
	UINT32 r_add = REG(rx) + REG(ry);
	UINT32 r_sub = REG(rx) - REG(ry);

	CLEAR_ALU_FLAGS();
	if (r_add == 0 || r_sub == 0)
	{
		cpustate->astat |= AZ;
	}
	if (r_add & 0x80000000 || r_sub & 0x80000000)
	{
		cpustate->astat |= AN;
	}
	if (((~(REG(rx) ^ REG(ry)) & (REG(rx) ^ r_add)) & 0x80000000) ||
		(( (REG(rx) ^ REG(ry)) & (REG(rx) ^ r_sub)) & 0x80000000))
	{
		cpustate->astat |= AV;
	}
	if (((UINT32)r_add < (UINT32)REG(rx)) ||
		(!((UINT32)r_sub < (UINT32)REG(rx))))
	{
		cpustate->astat |= AC;
	}

	REG(ra) = r_add;
	REG(rs) = r_sub;

	cpustate->astat &= ~AF;
}

/* Rm = (signed)Rxm * (signed)Rym, fractional, rounding,   Ra = Rxa + Rya */
INLINE void compute_mul_ssfr_add(SHARC_REGS *cpustate, int rm, int rxm, int rym, int ra, int rxa, int rya)
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

	cpustate->astat &= ~AF;
}

/* Rm = (signed)Rxm * (signed)Rym, fractional, rounding,   Ra = Rxa - Rya */
INLINE void compute_mul_ssfr_sub(SHARC_REGS *cpustate, int rm, int rxm, int rym, int ra, int rxa, int rya)
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

	cpustate->astat &= ~AF;
}


/* floating-point */

/* Fa = Fx + Fy,   Fs = Fx - Fy */
INLINE void compute_dual_fadd_fsub(SHARC_REGS *cpustate, int ra, int rs, int rx, int ry)
{
	SHARC_REG r_add, r_sub;
	r_add.f = FREG(rx) + FREG(ry);
	r_sub.f = FREG(rx) - FREG(ry);

	CLEAR_ALU_FLAGS();
	// AN
	cpustate->astat |= ((r_add.f < 0.0f) || (r_sub.f < 0.0f)) ? AN : 0;
	// AZ
	cpustate->astat |= (IS_FLOAT_DENORMAL(r_add.r) || IS_FLOAT_ZERO(r_add.r) ||
					IS_FLOAT_DENORMAL(r_sub.r) || IS_FLOAT_ZERO(r_sub.r)) ? AZ : 0;
	// AUS
	cpustate->stky |= (IS_FLOAT_DENORMAL(r_add.r) || IS_FLOAT_DENORMAL(r_sub.r)) ? AUS : 0;
	// AI
	cpustate->astat |= (IS_FLOAT_NAN(REG(rx)) || IS_FLOAT_NAN(REG(ry))) ? AI : 0;
	/* TODO: AV flag */

	// AIS
	if (cpustate->astat & AI)   cpustate->stky |= AIS;

	FREG(ra) = r_add.f;
	FREG(rs) = r_sub.f;
	cpustate->astat |= AF;
}

/* Fm = Fxm * Fym,   Fa = Fxa + Fya */
INLINE void compute_fmul_fadd(SHARC_REGS *cpustate, int fm, int fxm, int fym, int fa, int fxa, int fya)
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
	cpustate->astat |= (r_add.f < 0.0f) ? AN : 0;
	// AZ
	cpustate->astat |= (IS_FLOAT_DENORMAL(r_add.r) || IS_FLOAT_ZERO(r_add.r)) ? AZ : 0;
	// AU
	cpustate->stky |= (IS_FLOAT_DENORMAL(r_add.r)) ? AUS : 0;
	// AI
	cpustate->astat |= (IS_FLOAT_NAN(REG(fxa)) || IS_FLOAT_NAN(REG(fya))) ? AI : 0;
	/* TODO: AV flag */

	// AIS
	if (cpustate->astat & AI)   cpustate->stky |= AIS;

	FREG(fm) = r_mul.f;
	FREG(fa) = r_add.f;
	cpustate->astat |= AF;
}

/* Fm = Fxm * Fym,   Fa = Fxa - Fya */
INLINE void compute_fmul_fsub(SHARC_REGS *cpustate, int fm, int fxm, int fym, int fa, int fxa, int fya)
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
	cpustate->astat |= (r_sub.f < 0.0f) ? AN : 0;
	// AZ
	cpustate->astat |= (IS_FLOAT_DENORMAL(r_sub.r) || IS_FLOAT_ZERO(r_sub.r)) ? AZ : 0;
	// AU
	cpustate->stky |= (IS_FLOAT_DENORMAL(r_sub.r)) ? AUS : 0;
	// AI
	cpustate->astat |= (IS_FLOAT_NAN(REG(fxa)) || IS_FLOAT_NAN(REG(fya))) ? AI : 0;
	/* TODO: AV flag */

	// AIS
	if (cpustate->astat & AI)   cpustate->stky |= AIS;

	FREG(fm) = r_mul.f;
	FREG(fa) = r_sub.f;
	cpustate->astat |= AF;
}

/* Fm = Fxm * Fym,   Fa = FLOAT Fxa BY Fya */
INLINE void compute_fmul_float_scaled(SHARC_REGS *cpustate, int fm, int fxm, int fym, int fa, int fxa, int fya)
{
	SHARC_REG x;
	SHARC_REG r_mul, r_alu;
	r_mul.f = FREG(fxm) * FREG(fym);

	x.f = (float)(INT32)REG(fxa);

	r_alu.r = SCALB(cpustate, x, fya);

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN(r_mul.r);
	/* TODO: MV flag */
	/* TODO: MU flag */
	/* TODO: MI flag */

	CLEAR_ALU_FLAGS();
	cpustate->astat |= (r_alu.f < 0.0f) ? AN : 0;
	// AZ
	cpustate->astat |= (IS_FLOAT_DENORMAL(r_alu.r) || IS_FLOAT_ZERO(r_alu.r)) ? AZ : 0;
	// AU
	cpustate->stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	/* TODO: set AV if overflowed */

	FREG(fm) = r_mul.f;
	FREG(fa) = r_alu.f;
	cpustate->astat |= AF;
}

/* Fm = Fxm * Fym,   Fa = FIX Fxa BY Fya */
INLINE void compute_fmul_fix_scaled(SHARC_REGS *cpustate, int fm, int fxm, int fym, int fa, int fxa, int fya)
{
	INT32 alu_i;
	SHARC_REG r_mul, r_alu;
	r_mul.f = FREG(fxm) * FREG(fym);

	r_alu.r = SCALB(cpustate, cpustate->r[fxa], fya);

	if (cpustate->mode1 & MODE1_TRUNCATE)
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
	cpustate->stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	// AI
	cpustate->astat |= (IS_FLOAT_NAN(REG(fxa))) ? AI : 0;
	/* TODO: AV flag */

	FREG(fm) = r_mul.f;
	REG(fa) = alu_i;
	cpustate->astat |= AF;
}


/* Fm = Fxm * Fym,   Fa = MAX(Fxa, Fya) */
INLINE void compute_fmul_fmax(SHARC_REGS *cpustate, int fm, int fxm, int fym, int fa, int fxa, int fya)
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
	cpustate->astat |= (r_alu.f < 0.0f) ? AN : 0;
	// AZ
	cpustate->astat |= (IS_FLOAT_ZERO(r_alu.r)) ? AZ : 0;
	// AU
	cpustate->stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	// AI
	cpustate->astat |= (IS_FLOAT_NAN(REG(fxa)) || IS_FLOAT_NAN(REG(fya))) ? AI : 0;
	/* TODO: AV flag */

	FREG(fm) = r_mul.f;
	FREG(fa) = r_alu.f;
	cpustate->astat |= AF;
}


/* Fm = Fxm * Fym,   Fa = MIN(Fxa, Fya) */
INLINE void compute_fmul_fmin(SHARC_REGS *cpustate, int fm, int fxm, int fym, int fa, int fxa, int fya)
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
	cpustate->astat |= (r_alu.f < 0.0f) ? AN : 0;
	// AZ
	cpustate->astat |= (IS_FLOAT_ZERO(r_alu.r)) ? AZ : 0;
	// AU
	cpustate->stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	// AI
	cpustate->astat |= (IS_FLOAT_NAN(REG(fxa)) || IS_FLOAT_NAN(REG(fya))) ? AI : 0;
	/* TODO: AV flag */

	FREG(fm) = r_mul.f;
	FREG(fa) = r_alu.f;
	cpustate->astat |= AF;
}



/* Fm = Fxm * Fym,   Fa = Fxa + Fya,   Fs = Fxa - Fya */
INLINE void compute_fmul_dual_fadd_fsub(SHARC_REGS *cpustate, int fm, int fxm, int fym, int fa, int fs, int fxa, int fya)
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
	cpustate->astat |= ((r_add.r < 0.0f) || (r_sub.r < 0.0f)) ? AN : 0;
	// AZ
	cpustate->astat |= (IS_FLOAT_DENORMAL(r_add.r) || IS_FLOAT_ZERO(r_add.r) ||
					IS_FLOAT_DENORMAL(r_sub.r) || IS_FLOAT_ZERO(r_sub.r)) ? AZ : 0;
	// AUS
	cpustate->stky |= (IS_FLOAT_DENORMAL(r_add.r) || IS_FLOAT_DENORMAL(r_sub.r)) ? AUS : 0;
	// AI
	cpustate->astat |= (IS_FLOAT_NAN(REG(fxa)) || IS_FLOAT_NAN(REG(fya))) ? AI : 0;
	/* TODO: AV flag */

	// AIS
	if (cpustate->astat & AI)   cpustate->stky |= AIS;

	FREG(fm) = r_mul.f;
	FREG(fa) = r_add.f;
	FREG(fs) = r_sub.f;
	cpustate->astat |= AF;
}
