// license:BSD-3-Clause
// copyright-holders:Ville Linde
/* compute operations */

#include <cmath>

#define CLEAR_ALU_FLAGS()       (m_core->astat &= ~(AZ|AN|AV|AC|AS|AI))

#define SET_FLAG_AZ(r)          { m_core->astat |= (((r) == 0) ? AZ : 0); }
#define SET_FLAG_AN(r)          { m_core->astat |= (((r) & 0x80000000) ? AN : 0); }
#define SET_FLAG_AC_ADD(r,a,b)  { m_core->astat |= (((uint32_t)r < (uint32_t)a) ? AC : 0); }
#define SET_FLAG_AV_ADD(r,a,b)  { m_core->astat |= (((~((a) ^ (b)) & ((a) ^ (r))) & 0x80000000) ? AV : 0); }
#define SET_FLAG_AC_SUB(r,a,b)  { m_core->astat |= ((!((uint32_t)a < (uint32_t)b)) ? AC : 0); }
#define SET_FLAG_AV_SUB(r,a,b)  { m_core->astat |= ((( ((a) ^ (b)) & ((a) ^ (r))) & 0x80000000) ? AV : 0); }

#define IS_FLOAT_ZERO(r)        ((((r) & 0x7fffffff) == 0))
#define IS_FLOAT_DENORMAL(r)    ((((r) & 0x7f800000) == 0) && (((r) & 0x7fffff) != 0))
#define IS_FLOAT_NAN(r)         ((((r) & 0x7f800000) == 0x7f800000) && (((r) & 0x7fffff) != 0))
#define IS_FLOAT_INFINITY(r)    (((r) & 0x7fffffff) == 0x7f800000)

#define CLEAR_MULTIPLIER_FLAGS()    (m_core->astat &= ~(MN|MV|MU|MI))

#define SET_FLAG_MN(r)          { m_core->astat |= (((r) & 0x80000000) ? MN : 0); }
#define SET_FLAG_MV(r)          { m_core->astat |= ((((uint32_t)((r) >> 32) != 0) && ((uint32_t)((r) >> 32) != 0xffffffff)) ? MV : 0); }

/* TODO: MU needs 80-bit result */
#define SET_FLAG_MU(r)          { m_core->astat |= ((((uint32_t)((r) >> 32) == 0) && ((uint32_t)(r)) != 0) ? MU : 0); }


#define FLOAT_SIGN          0x80000000
#define FLOAT_INFINITY      0x7f800000
#define FLOAT_MANTISSA      0x007fffff

/*****************************************************************************/

// Mantissa lookup-table for RECIPS opcode
static const uint32_t recips_mantissa_lookup[128] =
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
static const uint32_t rsqrts_mantissa_lookup[128] =
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
void adsp21062_device::compute_add(int rn, int rx, int ry)
{
	uint32_t r = REG(rx) + REG(ry);

	if (m_core->mode1 & MODE1_ALUSAT)
		fatalerror("SHARC: compute_add: ALU saturation not implemented!\n");

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	SET_FLAG_AV_ADD(r, REG(rx), REG(ry));
	SET_FLAG_AC_ADD(r, REG(rx), REG(ry));
	REG(rn) = r;

	m_core->astat &= ~AF;
}

/* Rn = Rx - Ry */
void adsp21062_device::compute_sub(int rn, int rx, int ry)
{
	uint32_t r = REG(rx) - REG(ry);

	if (m_core->mode1 & MODE1_ALUSAT)
		fatalerror("SHARC: compute_sub: ALU saturation not implemented!\n");

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	SET_FLAG_AV_SUB(r, REG(rx), REG(ry));
	SET_FLAG_AC_SUB(r, REG(rx), REG(ry));
	REG(rn) = r;

	m_core->astat &= ~AF;
}

/* Rn = Rx + Ry + CI */
void adsp21062_device::compute_add_ci(int rn, int rx, int ry)
{
	int c = (m_core->astat & AC) ? 1 : 0;
	uint32_t r = REG(rx) + REG(ry) + c;

	if (m_core->mode1 & MODE1_ALUSAT)
		fatalerror("SHARC: compute_add_ci: ALU saturation not implemented!\n");

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	SET_FLAG_AV_ADD(r, REG(rx), REG(ry)+c);
	SET_FLAG_AC_ADD(r, REG(rx), REG(ry)+c);
	REG(rn) = r;

	m_core->astat &= ~AF;
}

/* Rn = Rx - Ry + CI - 1 */
void adsp21062_device::compute_sub_ci(int rn, int rx, int ry)
{
	int c = (m_core->astat & AC) ? 1 : 0;
	uint32_t r = REG(rx) - REG(ry) + c - 1;

	if (m_core->mode1 & MODE1_ALUSAT)
		fatalerror("SHARC: compute_sub_ci: ALU saturation not implemented!\n");

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	SET_FLAG_AV_SUB(r, REG(rx), REG(ry)+c-1);
	SET_FLAG_AC_SUB(r, REG(rx), REG(ry)+c-1);
	REG(rn) = r;

	m_core->astat &= ~AF;
}

/* Rn = Rx AND Ry */
void adsp21062_device::compute_and(int rn, int rx, int ry)
{
	uint32_t r = REG(rx) & REG(ry);

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	REG(rn) = r;

	m_core->astat &= ~AF;
}

/* COMP(Rx, Ry) */
void adsp21062_device::compute_comp(int rx, int ry)
{
	uint32_t comp_accum;

	CLEAR_ALU_FLAGS();
	if( REG(rx) == REG(ry) )
		m_core->astat |= AZ;
	if( (int32_t)REG(rx) < (int32_t)REG(ry) )
		m_core->astat |= AN;

	// Update ASTAT compare accumulation register
	comp_accum = (m_core->astat >> 24) & 0xff;
	comp_accum >>= 1;
	if ((m_core->astat & (AZ|AN)) == 0)
	{
		comp_accum |= 0x80;
	}
	m_core->astat &= 0xffffff;
	m_core->astat |= comp_accum << 24;

	m_core->astat &= ~AF;
}

/* Rn = PASS Rx */
void adsp21062_device::compute_pass(int rn, int rx)
{
	CLEAR_ALU_FLAGS();
	/* TODO: floating-point extension field is set to 0 */

	REG(rn) = REG(rx);
	if (REG(rn) == 0)
		m_core->astat |= AZ;
	if (REG(rn) & 0x80000000)
		m_core->astat |= AN;

	m_core->astat &= ~AF;
}

/* Rn = Rx XOR Ry */
void adsp21062_device::compute_xor(int rn, int rx, int ry)
{
	uint32_t r = REG(rx) ^ REG(ry);
	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	REG(rn) = r;

	m_core->astat &= ~AF;
}

/* Rn = Rx OR Ry */
void adsp21062_device::compute_or(int rn, int rx, int ry)
{
	uint32_t r = REG(rx) | REG(ry);
	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	REG(rn) = r;

	m_core->astat &= ~AF;
}

/* Rn = Rx + 1 */
void adsp21062_device::compute_inc(int rn, int rx)
{
	uint32_t r = REG(rx) + 1;

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	SET_FLAG_AV_ADD(r, REG(rx), 1);
	SET_FLAG_AC_ADD(r, REG(rx), 1);

	REG(rn) = r;

	m_core->astat &= ~AF;
}

/* Rn = Rx - 1 */
void adsp21062_device::compute_dec(int rn, int rx)
{
	uint32_t r = REG(rx) - 1;

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	SET_FLAG_AV_SUB(r, REG(rx), 1);
	SET_FLAG_AC_SUB(r, REG(rx), 1);

	REG(rn) = r;

	m_core->astat &= ~AF;
}

/* Rn = MIN(Rx, Ry) */
void adsp21062_device::compute_min(int rn, int rx, int ry)
{
	uint32_t r = std::min((int32_t)REG(rx), (int32_t)REG(ry));

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);

	REG(rn) = r;

	m_core->astat &= ~AF;
}

/* Rn = MAX(Rx, Ry) */
void adsp21062_device::compute_max(int rn, int rx, int ry)
{
	uint32_t r = std::max((int32_t)REG(rx), (int32_t)REG(ry));

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);

	REG(rn) = r;

	m_core->astat &= ~AF;
}

/* Rn = -Rx */
void adsp21062_device::compute_neg(int rn, int rx)
{
	uint32_t r = -(int32_t)(REG(rx));

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	SET_FLAG_AV_SUB(r, 0, REG(rx));
	SET_FLAG_AC_SUB(r, 0, REG(rx));

	REG(rn) = r;

	m_core->astat &= ~AF;
}

/* Rn = NOT Rx */
void adsp21062_device::compute_not(int rn, int rx)
{
	uint32_t r = ~REG(rx);

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);

	REG(rn) = r;

	m_core->astat &= ~AF;
}

/*****************************************************************************/
/* Floating-point ALU operations */

uint32_t adsp21062_device::SCALB(SHARC_REG rx, int ry)
{
	uint32_t mantissa = rx.r & FLOAT_MANTISSA;
	uint32_t sign = rx.r & FLOAT_SIGN;

	int exponent = ((rx.r >> 23) & 0xff) - 127;
	exponent += (int32_t)(REG(ry));

	if (exponent > 127)
	{
		// overflow
		m_core->astat |= AV;
		return sign | FLOAT_INFINITY;
	}
	else if (exponent < -126)
	{
		// denormal
		m_core->astat |= AZ;
		return sign;
	}
	else
	{
		return sign | (((exponent + 127) & 0xff) << 23) | mantissa;
	}
}

/* Fn = FLOAT Rx */
void adsp21062_device::compute_float(int rn, int rx)
{
	// verified
	FREG(rn) = (float)(int32_t)REG(rx);

	CLEAR_ALU_FLAGS();
	// AN
	SET_FLAG_AN(REG(rn));
	// AZ
	m_core->astat |= (IS_FLOAT_DENORMAL(REG(rn)) || IS_FLOAT_ZERO(REG(rn))) ? AZ : 0;
	// AUS
	m_core->stky |= (IS_FLOAT_DENORMAL(REG(rn))) ? AUS : 0;
	/* TODO: AV flag */

	m_core->astat |= AF;
}

/* Rn = FIX Fx */
void adsp21062_device::compute_fix(int rn, int rx)
{
	int32_t alu_i;
	SHARC_REG r_alu;

	r_alu.f = FREG(rx);
	if (m_core->mode1 & MODE1_TRUNCATE)
	{
		alu_i = (int32_t)(r_alu.f);
	}
	else
	{
		alu_i = (int32_t)(r_alu.f < 0 ? (r_alu.f - 0.5f) : (r_alu.f + 0.5f));
	}

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(alu_i);
	// AZ
	SET_FLAG_AZ(alu_i);
	// AU
	m_core->stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	// AI
	m_core->astat |= (IS_FLOAT_NAN(REG(rx))) ? AI : 0;
	/* TODO: AV flag */

	REG(rn) = alu_i;
	m_core->astat |= AF;
}

/* Rn = FIX Fx BY Ry */
void adsp21062_device::compute_fix_scaled(int rn, int rx, int ry)
{
	int32_t alu_i;
	SHARC_REG r_alu;

	r_alu.r = SCALB(m_core->r[rx], ry);
	if (m_core->mode1 & MODE1_TRUNCATE)
	{
		alu_i = (int32_t)(r_alu.f);
	}
	else
	{
		alu_i = (int32_t)(r_alu.f < 0 ? (r_alu.f - 0.5f) : (r_alu.f + 0.5f));
	}

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(alu_i);
	// AZ
	SET_FLAG_AZ(alu_i);
	// AU
	m_core->stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	// AI
	m_core->astat |= (IS_FLOAT_NAN(REG(rx))) ? AI : 0;
	/* TODO: AV flag */

	REG(rn) = alu_i;
	m_core->astat |= AF;
}

/* Fn = FLOAT Rx BY Ry */
void adsp21062_device::compute_float_scaled(int rn, int rx, int ry)
{
	SHARC_REG x;
	x.f = (float)(int32_t)(REG(rx));

	// verified
	CLEAR_ALU_FLAGS();

	REG(rn) = SCALB(x, ry);

	// AN
	SET_FLAG_AN(REG(rn));
	// AZ
	m_core->astat |= (IS_FLOAT_DENORMAL(REG(rn)) || IS_FLOAT_ZERO(REG(rn))) ? AZ : 0;
	// AU
	m_core->stky |= (IS_FLOAT_DENORMAL(REG(rn))) ? AUS : 0;

	m_core->astat |= AF;
}

/* Rn = LOGB Fx */
void adsp21062_device::compute_logb(int rn, int rx)
{
	// verified
	uint32_t r = REG(rx);

	CLEAR_ALU_FLAGS();

	if (IS_FLOAT_INFINITY(REG(rx)))
	{
		REG(rn) = FLOAT_INFINITY;

		m_core->astat |= AV;
	}
	else if (IS_FLOAT_ZERO(REG(rx)))
	{
		REG(rn) = FLOAT_SIGN | FLOAT_INFINITY;

		m_core->astat |= AV;
	}
	else if (IS_FLOAT_NAN(REG(rx)))
	{
		REG(rn) = 0xffffffff;

		m_core->astat |= AI;
		m_core->stky |= AIS;
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
	m_core->astat |= AF;
}

/* Fn = SCALB Fx BY Fy */
void adsp21062_device::compute_scalb(int rn, int rx, int ry)
{
	// verified
	SHARC_REG r;
	CLEAR_ALU_FLAGS();

	if (IS_FLOAT_NAN(REG(rx)))
	{
		m_core->astat |= AI;
		m_core->stky |= AIS;

		REG(rn) = 0xffffffff;
	}
	else
	{
		r.r = SCALB(m_core->r[rx], ry);

		// AN
		SET_FLAG_AN(r.r);
		// AZ
		m_core->astat |= IS_FLOAT_ZERO(r.r) ? AZ : 0;
		// AUS
		m_core->stky |= (IS_FLOAT_DENORMAL(r.r)) ? AUS : 0;

		FREG(rn) = r.f;
	}
	m_core->astat |= AF;
}

/* Fn = Fx + Fy */
void adsp21062_device::compute_fadd(int rn, int rx, int ry)
{
	SHARC_REG r;
	r.f = FREG(rx) + FREG(ry);

	CLEAR_ALU_FLAGS();
	// AN
	m_core->astat |= (r.f < 0.0f) ? AN : 0;
	// AZ
	m_core->astat |= (IS_FLOAT_DENORMAL(r.r) || IS_FLOAT_ZERO(r.r)) ? AZ : 0;
	// AUS
	m_core->stky |= (IS_FLOAT_DENORMAL(r.r)) ? AUS : 0;
	// AI
	m_core->astat |= (IS_FLOAT_NAN(REG(rx)) || IS_FLOAT_NAN(REG(ry))) ? AI : 0;
	/* TODO: AV flag */

	// AIS
	if (m_core->astat & AI)   m_core->stky |= AIS;

	FREG(rn) = r.f;
	m_core->astat |= AF;
}

void adsp21062_device::compute_favg(int rn, int rx, int ry)
{
	SHARC_REG r;
	r.f = (FREG(rx) + FREG(ry)) / (float) 2.0f;

	CLEAR_ALU_FLAGS();
	// AN
	m_core->astat |= (r.f < 0.0f) ? AN : 0;
	// AZ
	m_core->astat |= (IS_FLOAT_DENORMAL(r.r) || IS_FLOAT_ZERO(r.r)) ? AZ : 0;
	// AUS
	m_core->stky |= (IS_FLOAT_DENORMAL(r.r)) ? AUS : 0;
	// AI
	m_core->astat |= (IS_FLOAT_NAN(REG(rx)) || IS_FLOAT_NAN(REG(ry))) ? AI : 0;
	/* TODO: AV flag */

	// AIS
	if (m_core->astat & AI)   m_core->stky |= AIS;

	FREG(rn) = r.f;
	m_core->astat |= AF;
}

/* Fn = Fx - Fy */
void adsp21062_device::compute_fsub(int rn, int rx, int ry)
{
	SHARC_REG r;
	r.f = FREG(rx) - FREG(ry);

	CLEAR_ALU_FLAGS();
	// AN
	m_core->astat |= (r.f < 0.0f) ? AN : 0;
	// AZ
	m_core->astat |= (IS_FLOAT_DENORMAL(r.r) || IS_FLOAT_ZERO(r.r)) ? AZ : 0;
	// AUS
	m_core->stky |= (IS_FLOAT_DENORMAL(r.r)) ? AUS : 0;
	// AI
	m_core->astat |= (IS_FLOAT_NAN(REG(rx)) || IS_FLOAT_NAN(REG(ry))) ? AI : 0;
	/* TODO: AV flag */

	// AIS
	if (m_core->astat & AI)   m_core->stky |= AIS;

	FREG(rn) = r.f;
	m_core->astat |= AF;
}

/* Fn = -Fx */
void adsp21062_device::compute_fneg(int rn, int rx)
{
	SHARC_REG r;
	r.f = -FREG(rx);

	CLEAR_ALU_FLAGS();
	// AZ
	m_core->astat |= (IS_FLOAT_ZERO(r.r)) ? AZ : 0;
	// AN
	m_core->astat |= (r.f < 0.0f) ? AN : 0;
	// AI
	m_core->astat |= (IS_FLOAT_NAN(REG(rx))) ? AI : 0;

	// AIS
	if (m_core->astat & AI)   m_core->stky |= AIS;

	FREG(rn) = r.f;
	m_core->astat |= AF;
}

/* COMP(Fx, Fy) */
void adsp21062_device::compute_fcomp(int rx, int ry)
{
	uint32_t comp_accum;

	CLEAR_ALU_FLAGS();
	// AZ
	if( FREG(rx) == FREG(ry) )
		m_core->astat |= AZ;
	// AN
	if( FREG(rx) < FREG(ry) )
		m_core->astat |= AN;
	// AI
	m_core->astat |= (IS_FLOAT_NAN(REG(rx)) || IS_FLOAT_NAN(REG(ry))) ? AI : 0;

	// AIS
	if (m_core->astat & AI)   m_core->stky |= AIS;

	// Update ASTAT compare accumulation register
	comp_accum = (m_core->astat >> 24) & 0xff;
	comp_accum >>= 1;
	if ((m_core->astat & (AZ|AN)) == 0)
	{
		comp_accum |= 0x80;
	}
	m_core->astat &= 0xffffff;
	m_core->astat |= comp_accum << 24;
	m_core->astat |= AF;
}

/* Fn = ABS(Fx + Fy) */
void adsp21062_device::compute_fabs_plus(int rn, int rx, int ry)
{
	SHARC_REG r;
	r.f = fabs(FREG(rx) + FREG(ry));

	CLEAR_ALU_FLAGS();
	// AZ
	m_core->astat |= (IS_FLOAT_DENORMAL(r.r) || IS_FLOAT_ZERO(r.r)) ? AZ : 0;
	// AUS
	m_core->stky |= (IS_FLOAT_DENORMAL(r.r)) ? AUS : 0;
	// AI
	m_core->astat |= (IS_FLOAT_NAN(REG(rx)) || IS_FLOAT_NAN(REG(ry))) ? AI : 0;
	/* TODO: AV flag */

	// AIS
	if (m_core->astat & AI)   m_core->stky |= AIS;

	FREG(rn) = r.f;
	m_core->astat |= AF;
}

/* Fn = MAX(Fx, Fy) */
void adsp21062_device::compute_fmax(int rn, int rx, int ry)
{
	SHARC_REG r_alu;

	r_alu.f = std::max(FREG(rx), FREG(ry));

	CLEAR_ALU_FLAGS();
	m_core->astat |= (r_alu.f < 0.0f) ? AN : 0;
	// AZ
	m_core->astat |= (IS_FLOAT_ZERO(r_alu.r)) ? AZ : 0;
	// AU
	m_core->stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	// AI
	m_core->astat |= (IS_FLOAT_NAN(REG(rx)) || IS_FLOAT_NAN(REG(ry))) ? AI : 0;
	/* TODO: AV flag */

	FREG(rn) = r_alu.f;
	m_core->astat |= AF;
}

/* Fn = MIN(Fx, Fy) */
void adsp21062_device::compute_fmin(int rn, int rx, int ry)
{
	SHARC_REG r_alu;

	r_alu.f = std::min(FREG(rx), FREG(ry));

	CLEAR_ALU_FLAGS();
	m_core->astat |= (r_alu.f < 0.0f) ? AN : 0;
	// AZ
	m_core->astat |= (IS_FLOAT_ZERO(r_alu.r)) ? AZ : 0;
	// AU
	m_core->stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	// AI
	m_core->astat |= (IS_FLOAT_NAN(REG(rx)) || IS_FLOAT_NAN(REG(ry))) ? AI : 0;
	/* TODO: AV flag */

	FREG(rn) = r_alu.f;
	m_core->astat |= AF;
}

/* Fn = COPYSIGN(Fx, Fy) */
void adsp21062_device::compute_fcopysign(int rn, int rx, int ry)
{
	SHARC_REG r_alu;

	r_alu.r = (REG(rx) & 0x7fffffff) | (REG(ry) & 0x80000000); // TODO DENORM and NAN cases ?

	CLEAR_ALU_FLAGS();
	m_core->astat |= (r_alu.f < 0.0f) ? AN : 0;
	// AZ
	m_core->astat |= (IS_FLOAT_ZERO(r_alu.r)) ? AZ : 0;
	// AI
	m_core->astat |= (IS_FLOAT_NAN(REG(rx)) || IS_FLOAT_NAN(REG(ry))) ? AI : 0;

	FREG(rn) = r_alu.f;
	m_core->astat |= AF;
}

/* Fn = CLIP Fx BY Fy */
void adsp21062_device::compute_fclip(int rn, int rx, int ry)
{
	SHARC_REG r_alu;

	if (FREG(rx) < fabsf(FREG(ry)))
	{
		r_alu.f = FREG(rx);
	}
	else
	{
		if (FREG(rx) >= 0.0f)
		{
			r_alu.f = fabsf(FREG(ry));
		}
		else
		{
			r_alu.f = -fabsf(FREG(ry));
		}
	}


	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r_alu.r);
	// AZ
	m_core->astat |= (IS_FLOAT_ZERO(r_alu.r)) ? AZ : 0;
	// AU
	m_core->stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	// AI
	m_core->astat |= (IS_FLOAT_NAN(REG(rx)) || IS_FLOAT_NAN(REG(ry))) ? AI : 0;

	FREG(rn) = r_alu.f;
	m_core->astat |= AF;
}

/* Fn = RECIPS Fx */
void adsp21062_device::compute_recips(int rn, int rx)
{
	// verified
	uint32_t r;

	CLEAR_ALU_FLAGS();

	if (IS_FLOAT_NAN(REG(rx)))
	{
		// NaN
		r = 0xffffffff;

		// AI
		m_core->astat |= AI;

		// AIS
		m_core->stky |= AIS;
	}
	else if (IS_FLOAT_ZERO(REG(rx)))
	{
		// +- Zero
		r = (REG(rx) & FLOAT_SIGN) | FLOAT_INFINITY;

		m_core->astat |= AZ;
	}
	else
	{
		uint32_t mantissa = REG(rx) & 0x7fffff;
		uint32_t exponent = (REG(rx) >> 23) & 0xff;
		uint32_t sign = REG(rx) & FLOAT_SIGN;

		uint32_t res_mantissa = recips_mantissa_lookup[mantissa >> 16];

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
		m_core->astat |= (IS_FLOAT_ZERO(r)) ? AZ : 0;
		m_core->astat |= (IS_FLOAT_ZERO(REG(rx))) ? AV : 0;
		// AI
		m_core->astat |= (IS_FLOAT_NAN(REG(rx))) ? AI : 0;

		// AIS
		if (m_core->astat & AI)   m_core->stky |= AIS;
	}

	// AF
	m_core->astat |= AF;

	REG(rn) = r;
}

/* Fn = RSQRTS Fx */
void adsp21062_device::compute_rsqrts(int rn, int rx)
{
	// verified
	uint32_t r;

	if ((uint32_t)(REG(rx)) > 0x80000000)
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
		uint32_t mantissa = REG(rx) & 0xffffff;   // mantissa + LSB of biased exponent
		uint32_t exponent = (REG(rx) >> 23) & 0xff;
		uint32_t sign = REG(rx) & FLOAT_SIGN;

		uint32_t res_mantissa = rsqrts_mantissa_lookup[mantissa >> 17];

		int res_exponent = -((exponent - 127) / 2) - 1;
		res_exponent = (res_exponent + 127) & 0xff;

		r = sign | (res_exponent << 23) | res_mantissa;
	}

	CLEAR_ALU_FLAGS();
	// AN
	m_core->astat |= (REG(rx) == 0x80000000) ? AN : 0;
	// AZ & AV
	m_core->astat |= (IS_FLOAT_ZERO(r)) ? AZ : 0;
	m_core->astat |= (IS_FLOAT_ZERO(REG(rx))) ? AV : 0;
	// AI
	m_core->astat |= (IS_FLOAT_NAN(REG(rx)) || (REG(rx) & 0x80000000)) ? AI : 0;
	// AIS
	if (m_core->astat & AI)   m_core->stky |= AIS;
	// AF
	m_core->astat |= AF;

	REG(rn) = r;
}


/* Fn = PASS Fx */
void adsp21062_device::compute_fpass(int rn, int rx)
{
	SHARC_REG r;
	r.f = FREG(rx);

	CLEAR_ALU_FLAGS();
	// AN
	m_core->astat |= (r.f < 0.0f) ? AN : 0;
	// AZ
	m_core->astat |= (IS_FLOAT_ZERO(r.r)) ? AZ : 0;
	// AI
	m_core->astat |= (IS_FLOAT_NAN(REG(rx))) ? AI : 0;

	FREG(rn) = r.f;
	m_core->astat |= AF;
}

/* Fn = ABS Fx */
void adsp21062_device::compute_fabs(int rn, int rx)
{
	SHARC_REG r;
	r.f = fabs(FREG(rx));

	CLEAR_ALU_FLAGS();
	// AN
	m_core->astat |= (r.f < 0.0f) ? AN : 0;
	// AZ
	m_core->astat |= (IS_FLOAT_ZERO(r.r)) ? AZ : 0;
	// AI
	m_core->astat |= (IS_FLOAT_NAN(REG(rx))) ? AI : 0;

	FREG(rn) = r.f;
	m_core->astat |= AF;
}

/*****************************************************************************/
/* Multiplier opcodes */

/* Rn = (unsigned)Rx * (unsigned)Ry, integer, no rounding */
void adsp21062_device::compute_mul_uuin(int rn, int rx, int ry)
{
	uint64_t r = mulu_32x32(REG(rx), REG(ry));

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN((uint32_t)r);
	SET_FLAG_MV(r);
	SET_FLAG_MU(r);

	REG(rn) = (uint32_t)(r);
}

/* Rn = (signed)Rx * (signed)Ry, integer, no rounding */
void adsp21062_device::compute_mul_ssin(int rn, int rx, int ry)
{
	uint64_t r = mul_32x32(REG(rx), REG(ry));

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN((uint32_t)r);
	SET_FLAG_MV(r);
	SET_FLAG_MU(r);

	REG(rn) = (uint32_t)(r);
}

/* MRF + (signed)Rx * (signed)Ry, integer, no rounding */
uint32_t adsp21062_device::compute_mrf_plus_mul_ssin(int rx, int ry)
{
	uint64_t r = m_core->mrf + mul_32x32(REG(rx), REG(ry));

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN((uint32_t)r);
	SET_FLAG_MV(r);
	SET_FLAG_MU(r);

	return (uint32_t)(r);
}

/* MRB + (signed)Rx * (signed)Ry, integer, no rounding */
uint32_t adsp21062_device::compute_mrb_plus_mul_ssin(int rx, int ry)
{
	int64_t r = m_core->mrb + mul_32x32(REG(rx), REG(ry));

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN((uint32_t)r);
	SET_FLAG_MV(r);
	SET_FLAG_MU(r);

	return (uint32_t)(r);
}

/* Fn = Fx * Fy */
void adsp21062_device::compute_fmul(int rn, int rx, int ry)
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
void adsp21062_device::compute_multi_mr_to_reg(int ai, int rk)
{
	switch(ai)
	{
		case 0:     SET_UREG(rk, (uint32_t)(m_core->mrf)); break;
		case 1:     SET_UREG(rk, (uint32_t)(m_core->mrf >> 32)); break;
		case 2:     fatalerror("SHARC: tried to load MR2F\n"); break;
		case 4:     SET_UREG(rk, (uint32_t)(m_core->mrb)); break;
		case 5:     SET_UREG(rk, (uint32_t)(m_core->mrb >> 32)); break;
		case 6:     fatalerror("SHARC: tried to load MR2B\n"); break;
		default:    fatalerror("SHARC: unknown ai %d in mr_to_reg\n", ai);
	}

	CLEAR_MULTIPLIER_FLAGS();
}

void adsp21062_device::compute_multi_reg_to_mr(int ai, int rk)
{
	switch(ai)
	{
		case 0:     m_core->mrf &= ~0xffffffff; m_core->mrf |= GET_UREG(rk); break;
		case 1:     m_core->mrf &= 0xffffffff; m_core->mrf |= (uint64_t)(GET_UREG(rk)) << 32; break;
		case 2:     fatalerror("SHARC: tried to write MR2F\n"); break;
		case 4:     m_core->mrb &= ~0xffffffff; m_core->mrb |= GET_UREG(rk); break;
		case 5:     m_core->mrb &= 0xffffffff; m_core->mrb |= (uint64_t)(GET_UREG(rk)) << 32; break;
		case 6:     fatalerror("SHARC: tried to write MR2B\n"); break;
		default:    fatalerror("SHARC: unknown ai %d in reg_to_mr\n", ai);
	}

	CLEAR_MULTIPLIER_FLAGS();
}

/* Ra = Rx + Ry,   Rs = Rx - Ry */
void adsp21062_device::compute_dual_add_sub(int ra, int rs, int rx, int ry)
{
	uint32_t r_add = REG(rx) + REG(ry);
	uint32_t r_sub = REG(rx) - REG(ry);

	CLEAR_ALU_FLAGS();
	if (r_add == 0 || r_sub == 0)
	{
		m_core->astat |= AZ;
	}
	if (r_add & 0x80000000 || r_sub & 0x80000000)
	{
		m_core->astat |= AN;
	}
	if (((~(REG(rx) ^ REG(ry)) & (REG(rx) ^ r_add)) & 0x80000000) ||
		(( (REG(rx) ^ REG(ry)) & (REG(rx) ^ r_sub)) & 0x80000000))
	{
		m_core->astat |= AV;
	}
	if (((uint32_t)r_add < (uint32_t)REG(rx)) ||
		(!((uint32_t)r_sub < (uint32_t)REG(rx))))
	{
		m_core->astat |= AC;
	}

	REG(ra) = r_add;
	REG(rs) = r_sub;

	m_core->astat &= ~AF;
}

/* Rm = (signed)Rxm * (signed)Rym, fractional, rounding,   Ra = Rxa + Rya */
void adsp21062_device::compute_mul_ssfr_add(int rm, int rxm, int rym, int ra, int rxa, int rya)
{
	uint32_t r_mul = (uint32_t)mul_32x32_shift(REG(rxm), REG(rym), 31);
	uint32_t r_add = REG(rxa) + REG(rya);

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

	m_core->astat &= ~AF;
}

/* Rm = (signed)Rxm * (signed)Rym, fractional, rounding,   Ra = Rxa - Rya */
void adsp21062_device::compute_mul_ssfr_sub(int rm, int rxm, int rym, int ra, int rxa, int rya)
{
	uint32_t r_mul = (uint32_t)mul_32x32_shift(REG(rxm), REG(rym), 31);
	uint32_t r_sub = REG(rxa) - REG(rya);

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

	m_core->astat &= ~AF;
}


/* floating-point */

/* Fa = Fx + Fy,   Fs = Fx - Fy */
void adsp21062_device::compute_dual_fadd_fsub(int ra, int rs, int rx, int ry)
{
	SHARC_REG r_add, r_sub;
	r_add.f = FREG(rx) + FREG(ry);
	r_sub.f = FREG(rx) - FREG(ry);

	CLEAR_ALU_FLAGS();
	// AN
	m_core->astat |= ((r_add.f < 0.0f) || (r_sub.f < 0.0f)) ? AN : 0;
	// AZ
	m_core->astat |= (IS_FLOAT_DENORMAL(r_add.r) || IS_FLOAT_ZERO(r_add.r) ||
					IS_FLOAT_DENORMAL(r_sub.r) || IS_FLOAT_ZERO(r_sub.r)) ? AZ : 0;
	// AUS
	m_core->stky |= (IS_FLOAT_DENORMAL(r_add.r) || IS_FLOAT_DENORMAL(r_sub.r)) ? AUS : 0;
	// AI
	m_core->astat |= (IS_FLOAT_NAN(REG(rx)) || IS_FLOAT_NAN(REG(ry))) ? AI : 0;
	/* TODO: AV flag */

	// AIS
	if (m_core->astat & AI)   m_core->stky |= AIS;

	FREG(ra) = r_add.f;
	FREG(rs) = r_sub.f;
	m_core->astat |= AF;
}

/* Fm = Fxm * Fym,   Fa = Fxa + Fya */
void adsp21062_device::compute_fmul_fadd(int fm, int fxm, int fym, int fa, int fxa, int fya)
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
	m_core->astat |= (r_add.f < 0.0f) ? AN : 0;
	// AZ
	m_core->astat |= (IS_FLOAT_DENORMAL(r_add.r) || IS_FLOAT_ZERO(r_add.r)) ? AZ : 0;
	// AU
	m_core->stky |= (IS_FLOAT_DENORMAL(r_add.r)) ? AUS : 0;
	// AI
	m_core->astat |= (IS_FLOAT_NAN(REG(fxa)) || IS_FLOAT_NAN(REG(fya))) ? AI : 0;
	/* TODO: AV flag */

	// AIS
	if (m_core->astat & AI)   m_core->stky |= AIS;

	FREG(fm) = r_mul.f;
	FREG(fa) = r_add.f;
	m_core->astat |= AF;
}

/* Fm = Fxm * Fym,   Fa = Fxa - Fya */
void adsp21062_device::compute_fmul_fsub(int fm, int fxm, int fym, int fa, int fxa, int fya)
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
	m_core->astat |= (r_sub.f < 0.0f) ? AN : 0;
	// AZ
	m_core->astat |= (IS_FLOAT_DENORMAL(r_sub.r) || IS_FLOAT_ZERO(r_sub.r)) ? AZ : 0;
	// AU
	m_core->stky |= (IS_FLOAT_DENORMAL(r_sub.r)) ? AUS : 0;
	// AI
	m_core->astat |= (IS_FLOAT_NAN(REG(fxa)) || IS_FLOAT_NAN(REG(fya))) ? AI : 0;
	/* TODO: AV flag */

	// AIS
	if (m_core->astat & AI)   m_core->stky |= AIS;

	FREG(fm) = r_mul.f;
	FREG(fa) = r_sub.f;
	m_core->astat |= AF;
}

/* Fm = Fxm * Fym,   Fa = FLOAT Fxa BY Fya */
void adsp21062_device::compute_fmul_float_scaled(int fm, int fxm, int fym, int fa, int fxa, int fya)
{
	SHARC_REG x;
	SHARC_REG r_mul, r_alu;
	r_mul.f = FREG(fxm) * FREG(fym);

	x.f = (float)(int32_t)REG(fxa);

	r_alu.r = SCALB(x, fya);

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN(r_mul.r);
	/* TODO: MV flag */
	/* TODO: MU flag */
	/* TODO: MI flag */

	CLEAR_ALU_FLAGS();
	m_core->astat |= (r_alu.f < 0.0f) ? AN : 0;
	// AZ
	m_core->astat |= (IS_FLOAT_DENORMAL(r_alu.r) || IS_FLOAT_ZERO(r_alu.r)) ? AZ : 0;
	// AU
	m_core->stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	/* TODO: set AV if overflowed */

	FREG(fm) = r_mul.f;
	FREG(fa) = r_alu.f;
	m_core->astat |= AF;
}

/* Fm = Fxm * Fym,   Fa = FIX Fxa BY Fya */
void adsp21062_device::compute_fmul_fix_scaled(int fm, int fxm, int fym, int fa, int fxa, int fya)
{
	int32_t alu_i;
	SHARC_REG r_mul, r_alu;
	r_mul.f = FREG(fxm) * FREG(fym);

	r_alu.r = SCALB(m_core->r[fxa], fya);

	if (m_core->mode1 & MODE1_TRUNCATE)
	{
		alu_i = (int32_t)(r_alu.f);
	}
	else
	{
		alu_i = (int32_t)(r_alu.f < 0 ? (r_alu.f - 0.5f) : (r_alu.f + 0.5f));
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
	m_core->stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	// AI
	m_core->astat |= (IS_FLOAT_NAN(REG(fxa))) ? AI : 0;
	/* TODO: AV flag */

	FREG(fm) = r_mul.f;
	REG(fa) = alu_i; // TODO: check this, should be RA?
	m_core->astat |= AF;
}

void adsp21062_device::compute_fmul_avg(int fm, int fxm, int fym, int fa, int fxa, int fya)
{
	int32_t alu_i;
	SHARC_REG r_mul, r_alu;
	r_mul.f = FREG(fxm) * FREG(fym);
	r_alu.f = (FREG(fxa) + FREG(fya))/((float) 2.0);

	// TODO: are flags right for this?
	if (m_core->mode1 & MODE1_TRUNCATE)
	{
		alu_i = (int32_t)(r_alu.f);
	}
	else
	{
		alu_i = (int32_t)(r_alu.f < 0 ? (r_alu.f - 0.5f) : (r_alu.f + 0.5f));
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
	m_core->stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	// AI
	m_core->astat |= (IS_FLOAT_NAN(REG(fxa))) ? AI : 0;
	/* TODO: AV flag */

	FREG(fm) = r_mul.f;
	REG(fa) = alu_i;
	m_core->astat |= AF;
}

void adsp21062_device::compute_fmul_abs(int fm, int fxm, int fym, int fa, int fxa, int fya)
{
	int32_t alu_i;
	SHARC_REG r_mul, r_alu;
	r_mul.f = FREG(fxm) * FREG(fym);
	r_alu.f = (float) fabs(FREG(fxa));

	// TODO: are flags right for this?
	if (m_core->mode1 & MODE1_TRUNCATE)
	{
		alu_i = (int32_t)(r_alu.f);
	}
	else
	{
		alu_i = (int32_t)(r_alu.f < 0 ? (r_alu.f - 0.5f) : (r_alu.f + 0.5f));
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
	m_core->stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	// AI
	m_core->astat |= (IS_FLOAT_NAN(REG(fxa))) ? AI : 0;
	/* TODO: AV flag */

	FREG(fm) = r_mul.f;
	REG(fa) = alu_i;
	m_core->astat |= AF;
}

/* Fm = Fxm * Fym,   Fa = MAX(Fxa, Fya) */
void adsp21062_device::compute_fmul_fmax(int fm, int fxm, int fym, int fa, int fxa, int fya)
{
	SHARC_REG r_mul, r_alu;
	r_mul.f = FREG(fxm) * FREG(fym);

	r_alu.f = std::max(FREG(fxa), FREG(fya));

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN(r_mul.r);
	/* TODO: MV flag */
	/* TODO: MU flag */
	/* TODO: MI flag */

	CLEAR_ALU_FLAGS();
	m_core->astat |= (r_alu.f < 0.0f) ? AN : 0;
	// AZ
	m_core->astat |= (IS_FLOAT_ZERO(r_alu.r)) ? AZ : 0;
	// AU
	m_core->stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	// AI
	m_core->astat |= (IS_FLOAT_NAN(REG(fxa)) || IS_FLOAT_NAN(REG(fya))) ? AI : 0;
	/* TODO: AV flag */

	FREG(fm) = r_mul.f;
	FREG(fa) = r_alu.f;
	m_core->astat |= AF;
}


/* Fm = Fxm * Fym,   Fa = MIN(Fxa, Fya) */
void adsp21062_device::compute_fmul_fmin(int fm, int fxm, int fym, int fa, int fxa, int fya)
{
	SHARC_REG r_mul, r_alu;
	r_mul.f = FREG(fxm) * FREG(fym);

	r_alu.f = std::min(FREG(fxa), FREG(fya));

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN(r_mul.r);
	/* TODO: MV flag */
	/* TODO: MU flag */
	/* TODO: MI flag */

	CLEAR_ALU_FLAGS();
	m_core->astat |= (r_alu.f < 0.0f) ? AN : 0;
	// AZ
	m_core->astat |= (IS_FLOAT_ZERO(r_alu.r)) ? AZ : 0;
	// AU
	m_core->stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	// AI
	m_core->astat |= (IS_FLOAT_NAN(REG(fxa)) || IS_FLOAT_NAN(REG(fya))) ? AI : 0;
	/* TODO: AV flag */

	FREG(fm) = r_mul.f;
	FREG(fa) = r_alu.f;
	m_core->astat |= AF;
}



/* Fm = Fxm * Fym,   Fa = Fxa + Fya,   Fs = Fxa - Fya */
void adsp21062_device::compute_fmul_dual_fadd_fsub(int fm, int fxm, int fym, int fa, int fs, int fxa, int fya)
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
	m_core->astat |= ((r_add.r < 0.0f) || (r_sub.r < 0.0f)) ? AN : 0;
	// AZ
	m_core->astat |= (IS_FLOAT_DENORMAL(r_add.r) || IS_FLOAT_ZERO(r_add.r) ||
					IS_FLOAT_DENORMAL(r_sub.r) || IS_FLOAT_ZERO(r_sub.r)) ? AZ : 0;
	// AUS
	m_core->stky |= (IS_FLOAT_DENORMAL(r_add.r) || IS_FLOAT_DENORMAL(r_sub.r)) ? AUS : 0;
	// AI
	m_core->astat |= (IS_FLOAT_NAN(REG(fxa)) || IS_FLOAT_NAN(REG(fya))) ? AI : 0;
	/* TODO: AV flag */

	// AIS
	if (m_core->astat & AI)   m_core->stky |= AIS;

	FREG(fm) = r_mul.f;
	FREG(fa) = r_add.f;
	FREG(fs) = r_sub.f;
	m_core->astat |= AF;
}
