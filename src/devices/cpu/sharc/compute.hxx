// license:BSD-3-Clause
// copyright-holders:Ville Linde
/* compute operations */

#include <cmath>
#include <limits>

#define CLEAR_ALU_FLAGS()           do { m_core->astat &= ~(AZ | AN | AV | AC | AS | AI); } while (false)

#define SET_FLAG_AZ(r)              do { m_core->astat |= ((r) == 0) ? AZ : 0; } while (false)
#define SET_FLAG_AN(r)              do { m_core->astat |= ((r) & 0x80000000) ? AN : 0; } while (false)
#define SET_FLAG_AC_ADD(r,a,b)      do { m_core->astat |= (uint32_t(r) < uint32_t(a)) ? AC : 0; } while (false)
#define SET_FLAG_AC_SUB(r,a,b)      do { m_core->astat |= (uint32_t(r) <= uint32_t(a)) ? AC : 0; } while (false)
#define SET_FLAG_AV_ADD(r,a,b)      do { if (~((a) ^ (b)) & ((a) ^ (r)) & 0x80000000) { m_core->astat |= AV; m_core->stky |= AOS; } } while (false)
#define SET_FLAG_AV_SUB(r,a,b)      do { if (((a) ^ (b)) & ((a) ^ (r)) & 0x80000000) { m_core->astat |= AV; m_core->stky |= AOS; } } while (false)

#define CLEAR_MULTIPLIER_FLAGS()    do { m_core->astat &= ~(MN | MV | MU | MI); } while (false)

#define SET_FLAG_MN(r)              do { m_core->astat |= (((r) & 0x80000000) ? MN : 0); } while (false)
#define SET_FLAG_MV(r)              do { m_core->astat |= (((uint32_t((r) >> 32) != 0) && (uint32_t((r) >> 32) != 0xffffffff)) ? MV : 0); } while (false)

/* TODO: MU needs 80-bit result */
#define SET_FLAG_MU(r)              do { m_core->astat |= (((uint32_t((r) >> 32) == 0) && (uint32_t(r)) != 0) ? MU : 0); } while (false)

// saturate overflowed result
inline void SATURATE(uint32_t &r)                       { r = uint32_t((int32_t(r) >> 31) ^ 0x80000000); }

constexpr bool IS_FLOAT_ZERO(uint32_t r)                { return (r & (FLOAT_EXPONENT_MASK | FLOAT_MANTISSA_MASK)) == 0; }
constexpr bool IS_FLOAT_DENORMAL(uint32_t r)            { return ((r & FLOAT_EXPONENT_MASK) == 0) && ((r & FLOAT_MANTISSA_MASK) != 0); }
constexpr bool IS_FLOAT_DENORMAL_OR_ZERO(uint32_t r)    { return (r & FLOAT_EXPONENT_MASK) == 0; }
constexpr bool IS_FLOAT_NAN(uint32_t r)                 { return ((r & FLOAT_EXPONENT_MASK) == FLOAT_EXPONENT_MASK) && ((r & FLOAT_MANTISSA_MASK) != 0); }
constexpr bool IS_FLOAT_INFINITY(uint32_t r)            { return (r & (FLOAT_EXPONENT_MASK | FLOAT_MANTISSA_MASK)) == FLOAT_INFINITY; }
constexpr bool IS_FLOAT_NEGATIVE(uint32_t r)            { return (r & FLOAT_SIGN_MASK) && !IS_FLOAT_NAN(r); }

inline bool IS_FLOAT_NAN_ADD(uint32_t a, uint32_t b)
{
	bool const aemax = (a & FLOAT_EXPONENT_MASK) == FLOAT_EXPONENT_MASK;
	bool const bemax = (b & FLOAT_EXPONENT_MASK) == FLOAT_EXPONENT_MASK;
	return (aemax && ((a & FLOAT_MANTISSA_MASK) != 0)) || (bemax && ((b & FLOAT_MANTISSA_MASK) != 0)) || (aemax && bemax && ((a ^ b) & FLOAT_SIGN_MASK));
}

inline bool IS_FLOAT_NAN_SUB(uint32_t a, uint32_t b)
{
	bool const aemax = (a & FLOAT_EXPONENT_MASK) == FLOAT_EXPONENT_MASK;
	bool const bemax = (b & FLOAT_EXPONENT_MASK) == FLOAT_EXPONENT_MASK;
	return (aemax && ((a & FLOAT_MANTISSA_MASK) != 0)) || (bemax && ((b & FLOAT_MANTISSA_MASK) != 0)) || (aemax && bemax && !((a ^ b) & FLOAT_SIGN_MASK));
}

inline bool IS_FLOAT_NAN_MUL(uint32_t a, uint32_t b)
{
	bool const aemax = (a & FLOAT_EXPONENT_MASK) == FLOAT_EXPONENT_MASK;
	bool const bemax = (b & FLOAT_EXPONENT_MASK) == FLOAT_EXPONENT_MASK;
	return (aemax && (((a & FLOAT_MANTISSA_MASK) != 0) || IS_FLOAT_ZERO(b))) || (bemax && (((b & FLOAT_MANTISSA_MASK) != 0) || IS_FLOAT_ZERO(a)));
}

constexpr uint32_t FLOAT_FLUSH_DENORMAL(uint32_t r)     { return IS_FLOAT_DENORMAL_OR_ZERO(r) ? (r & FLOAT_SIGN_MASK) : r; }


/*****************************************************************************/

// Mantissa lookup-table for RECIPS opcode
const uint32_t adsp21062_device::recips_mantissa_lookup[128] =
{
	0x007f8000, 0x007e0000, 0x007c0000, 0x007a0000,
	0x00780000, 0x00760000, 0x00740000, 0x00720000,
	0x00700000, 0x006f0000, 0x006d0000, 0x006b0000,
	0x006a0000, 0x00680000, 0x00660000, 0x00650000,
	0x00630000, 0x00610000, 0x00600000, 0x005e0000,
	0x005d0000, 0x005b0000, 0x005a0000, 0x00590000,
	0x00570000, 0x00560000, 0x00540000, 0x00530000,
	0x00520000, 0x00500000, 0x004f0000, 0x004e0000,
	0x004c0000, 0x004b0000, 0x004a0000, 0x00490000,
	0x00470000, 0x00460000, 0x00450000, 0x00440000,
	0x00430000, 0x00410000, 0x00400000, 0x003f0000,
	0x003e0000, 0x003d0000, 0x003c0000, 0x003b0000,
	0x003a0000, 0x00390000, 0x00380000, 0x00370000,
	0x00360000, 0x00350000, 0x00340000, 0x00330000,
	0x00320000, 0x00310000, 0x00300000, 0x002f0000,
	0x002e0000, 0x002d0000, 0x002c0000, 0x002b0000,
	0x002a0000, 0x00290000, 0x00280000, 0x00280000,
	0x00270000, 0x00260000, 0x00250000, 0x00240000,
	0x00230000, 0x00230000, 0x00220000, 0x00210000,
	0x00200000, 0x001f0000, 0x001f0000, 0x001e0000,
	0x001d0000, 0x001c0000, 0x001c0000, 0x001b0000,
	0x001a0000, 0x00190000, 0x00190000, 0x00180000,
	0x00170000, 0x00170000, 0x00160000, 0x00150000,
	0x00140000, 0x00140000, 0x00130000, 0x00120000,
	0x00120000, 0x00110000, 0x00100000, 0x00100000,
	0x000f0000, 0x000f0000, 0x000e0000, 0x000d0000,
	0x000d0000, 0x000c0000, 0x000c0000, 0x000b0000,
	0x000a0000, 0x000a0000, 0x00090000, 0x00090000,
	0x00080000, 0x00070000, 0x00070000, 0x00060000,
	0x00060000, 0x00050000, 0x00050000, 0x00040000,
	0x00040000, 0x00030000, 0x00030000, 0x00020000,
	0x00020000, 0x00010000, 0x00010000, 0x00000000,
};

// Mantissa lookup-table for RSQRTS opcode
const uint32_t adsp21062_device::rsqrts_mantissa_lookup[128] =
{
	0x00350000, 0x00330000, 0x00320000, 0x00300000,
	0x002f0000, 0x002e0000, 0x002d0000, 0x002b0000,
	0x002a0000, 0x00290000, 0x00280000, 0x00270000,
	0x00260000, 0x00250000, 0x00230000, 0x00220000,
	0x00210000, 0x00200000, 0x001f0000, 0x001e0000,
	0x001e0000, 0x001d0000, 0x001c0000, 0x001b0000,
	0x001a0000, 0x00190000, 0x00180000, 0x00170000,
	0x00160000, 0x00160000, 0x00150000, 0x00140000,
	0x00130000, 0x00130000, 0x00120000, 0x00110000,
	0x00100000, 0x00100000, 0x000f0000, 0x000e0000,
	0x000e0000, 0x000d0000, 0x000c0000, 0x000b0000,
	0x000b0000, 0x000a0000, 0x000a0000, 0x00090000,
	0x00080000, 0x00080000, 0x00070000, 0x00070000,
	0x00060000, 0x00050000, 0x00050000, 0x00040000,
	0x00040000, 0x00030000, 0x00030000, 0x00020000,
	0x00020000, 0x00010000, 0x00010000, 0x00000000,
	0x007f8000, 0x007e0000, 0x007c0000, 0x007a0000,
	0x00780000, 0x00760000, 0x00740000, 0x00730000,
	0x00710000, 0x006f0000, 0x006e0000, 0x006c0000,
	0x006a0000, 0x00690000, 0x00670000, 0x00660000,
	0x00640000, 0x00630000, 0x00620000, 0x00600000,
	0x005f0000, 0x005e0000, 0x005c0000, 0x005b0000,
	0x005a0000, 0x00590000, 0x00570000, 0x00560000,
	0x00550000, 0x00540000, 0x00530000, 0x00520000,
	0x00510000, 0x004f0000, 0x004e0000, 0x004d0000,
	0x004c0000, 0x004b0000, 0x004a0000, 0x00490000,
	0x00480000, 0x00470000, 0x00460000, 0x00450000,
	0x00450000, 0x00440000, 0x00430000, 0x00420000,
	0x00410000, 0x00400000, 0x003f0000, 0x003e0000,
	0x003e0000, 0x003d0000, 0x003c0000, 0x003b0000,
	0x003a0000, 0x003a0000, 0x00390000, 0x00380000,
	0x00370000, 0x00370000, 0x00360000, 0x00350000,
};


/*****************************************************************************/
/* ALU helpers */

/* Fx + Fy */
adsp21062_device::SHARC_REG adsp21062_device::FADD(int fx, int fy)
{
	SHARC_REG r;
	if (IS_FLOAT_NAN_ADD(REG(fx), REG(fy)))
	{
		r.r = FLOAT_CANONICAL_NAN;
		m_core->astat |= AI;
		m_core->stky |= AIS;
	}
	else
	{
		SHARC_REG x, y;
		x.r = FLOAT_FLUSH_DENORMAL(REG(fx));
		y.r = FLOAT_FLUSH_DENORMAL(REG(fy));
		r.f = x.f + y.f;

		if (IS_FLOAT_INFINITY(r.r))
		{
			m_core->astat |= AV;
			m_core->stky |= AVS;
			if (m_core->mode1 & MODE1_TRUNCATE)
				r.r = (r.r & FLOAT_SIGN_MASK) | 0x7f7fffff;
		}
		else if (IS_FLOAT_DENORMAL_OR_ZERO(r.r))
		{
			m_core->astat |= AZ;
			m_core->stky |= ((r.r & FLOAT_MANTISSA_MASK) != 0) ? AUS : 0;
			r.r &= FLOAT_SIGN_MASK;
		}
		m_core->astat |= (r.r & FLOAT_SIGN_MASK) ? AN : 0;
	}

	m_core->astat |= AF;
	return r;
}

/* Fx - Fy */
adsp21062_device::SHARC_REG adsp21062_device::FSUB(int fx, int fy)
{
	SHARC_REG r;
	if (IS_FLOAT_NAN_SUB(REG(fx), REG(fy)))
	{
		r.r = FLOAT_CANONICAL_NAN;
		m_core->astat |= AI;
		m_core->stky |= AIS;
	}
	else
	{
		SHARC_REG x, y;
		x.r = FLOAT_FLUSH_DENORMAL(REG(fx));
		y.r = FLOAT_FLUSH_DENORMAL(REG(fy));
		r.f = x.f - y.f;

		if (IS_FLOAT_INFINITY(r.r))
		{
			m_core->astat |= AV;
			m_core->stky |= AVS;
			if (m_core->mode1 & MODE1_TRUNCATE)
				r.r = (r.r & FLOAT_SIGN_MASK) | 0x7f7fffff;
		}
		else if (IS_FLOAT_DENORMAL_OR_ZERO(r.r))
		{
			m_core->astat |= AZ;
			m_core->stky |= ((r.r & FLOAT_MANTISSA_MASK) != 0) ? AUS : 0;
			r.r &= FLOAT_SIGN_MASK;
		}
		m_core->astat |= (r.r & FLOAT_SIGN_MASK) ? AN : 0;
	}

	m_core->astat |= AF;
	return r;
}

/* (Fx + Fy) / 2 */
adsp21062_device::SHARC_REG adsp21062_device::FAVG(int fx, int fy)
{
	SHARC_REG r;
	if (IS_FLOAT_NAN_ADD(REG(fx), REG(fy)))
	{
		r.r = FLOAT_CANONICAL_NAN;
		m_core->astat |= AI;
		m_core->stky |= AIS;
	}
	else
	{
		// TODO: this loses one bit of range due to the intermediate sub being constrained to single precision
		SHARC_REG x, y;
		x.r = FLOAT_FLUSH_DENORMAL(REG(fx));
		y.r = FLOAT_FLUSH_DENORMAL(REG(fy));
		r.f = (x.f + y.f) * 0.5f;

		if (IS_FLOAT_INFINITY(r.r))
		{
			m_core->astat |= AV;
			m_core->stky |= AVS;
			if (m_core->mode1 & MODE1_TRUNCATE)
				r.r = (r.r & FLOAT_SIGN_MASK) | 0x7f7fffff;
		}
		else if (IS_FLOAT_DENORMAL_OR_ZERO(r.r))
		{
			m_core->astat |= AZ;
			m_core->stky |= ((r.r & FLOAT_MANTISSA_MASK) != 0) ? AUS : 0;
			r.r &= FLOAT_SIGN_MASK;
		}
		m_core->astat |= (r.r & FLOAT_SIGN_MASK) ? AN : 0;
	}

	m_core->astat |= AF;
	return r;
}

/* ABS Fx */
adsp21062_device::SHARC_REG adsp21062_device::FABS(int fx)
{
	SHARC_REG r;
	if (IS_FLOAT_NAN(REG(fx)))
	{
		r.r = FLOAT_CANONICAL_NAN;
		m_core->astat |= AI;
		m_core->stky |= AIS;
	}
	else
	{
		if (IS_FLOAT_DENORMAL_OR_ZERO(REG(fx)))
		{
			r.r = 0;
			m_core->astat |= AZ;
		}
		else
		{
			r.r = REG(fx) & ~FLOAT_SIGN_MASK;
		}
		m_core->astat |= (REG(fx) & FLOAT_SIGN_MASK) ? AS : 0;
	}

	m_core->astat |= AF;
	return r;
}

/* MIN(Fx, Fy) */
adsp21062_device::SHARC_REG adsp21062_device::FMIN(int fx, int fy)
{
	SHARC_REG r;
	if (IS_FLOAT_NAN(REG(fx)) || IS_FLOAT_NAN(REG(fy)))
	{
		r.r = FLOAT_CANONICAL_NAN;
		m_core->astat |= AI;
		m_core->stky |= AIS;
	}
	else
	{
		// TODO: flush denormals to zero, handle negative versus positive zero
		r.f = std::min(FREG(fx), FREG(fy));

		m_core->astat |= (r.f < 0.0f) ? AN : 0;
		m_core->astat |= (IS_FLOAT_ZERO(r.r)) ? AZ : 0;
	}

	m_core->astat |= AF;
	return r;
}

/* MAX(Fx, Fy) */
adsp21062_device::SHARC_REG adsp21062_device::FMAX(int fx, int fy)
{
	SHARC_REG r;
	if (IS_FLOAT_NAN(REG(fx)) || IS_FLOAT_NAN(REG(fy)))
	{
		r.r = FLOAT_CANONICAL_NAN;
		m_core->astat |= AI;
		m_core->stky |= AIS;
	}
	else
	{
		// TODO: flush denormals to zero, handle negative versus positive zero
		r.f = std::max(FREG(fx), FREG(fy));

		m_core->astat |= (r.f < 0.0f) ? AN : 0;
		m_core->astat |= (IS_FLOAT_ZERO(r.r)) ? AZ : 0;
	}

	m_core->astat |= AF;
	return r;
}

/* Fx + Fy,   Fx - Fy */
std::pair<adsp21062_device::SHARC_REG, adsp21062_device::SHARC_REG> adsp21062_device::FADD_FSUB(int fx, int fy)
{
	std::pair<SHARC_REG, SHARC_REG> r;
	bool const xemax = (REG(fx) & FLOAT_EXPONENT_MASK) == FLOAT_EXPONENT_MASK;
	bool const yemax = (REG(fy) & FLOAT_EXPONENT_MASK) == FLOAT_EXPONENT_MASK;

	if ((xemax && ((REG(fx) & FLOAT_MANTISSA_MASK) != 0)) || (yemax && (REG(fy) & FLOAT_MANTISSA_MASK) != 0))
	{
		// at least one NaN
		r.first.r = FLOAT_CANONICAL_NAN;
		r.second.r = FLOAT_CANONICAL_NAN;
		m_core->astat |= AI;
		m_core->stky |= AIS;
	}
	else if (!xemax || !yemax)
	{
		// neither NaN, at least one not infinity
		SHARC_REG x, y;
		x.r = FLOAT_FLUSH_DENORMAL(REG(fx));
		y.r = FLOAT_FLUSH_DENORMAL(REG(fy));
		r.first.f = x.f + y.f;
		r.second.f = x.f - y.f;

		if (IS_FLOAT_INFINITY(r.first.r))
		{
			m_core->astat |= AV;
			m_core->stky |= AVS;
			if (m_core->mode1 & MODE1_TRUNCATE)
				r.first.r = (r.first.r & FLOAT_SIGN_MASK) | 0x7f7fffff;
		}
		else if (IS_FLOAT_DENORMAL_OR_ZERO(r.first.r))
		{
			m_core->astat |= AZ;
			m_core->stky |= ((r.first.r & FLOAT_MANTISSA_MASK) != 0) ? AUS : 0;
			r.first.r &= FLOAT_SIGN_MASK;
		}

		if (IS_FLOAT_INFINITY(r.second.r))
		{
			m_core->astat |= AV;
			m_core->stky |= AVS;
			if (m_core->mode1 & MODE1_TRUNCATE)
				r.second.r = (r.second.r & FLOAT_SIGN_MASK) | 0x7f7fffff;
		}
		else if (IS_FLOAT_DENORMAL_OR_ZERO(r.second.r))
		{
			m_core->astat |= AZ;
			m_core->stky |= ((r.second.r & FLOAT_MANTISSA_MASK) != 0) ? AUS : 0;
			r.second.r &= FLOAT_SIGN_MASK;
		}

		m_core->astat |= ((r.first.r | r.second.r) & FLOAT_SIGN_MASK) ? AN : 0;
	}
	else
	{
		// both infinity
		if ((REG(fx) ^ REG(fy)) & FLOAT_SIGN_MASK)
		{
			// opposite signs - sum is NaN
			r.first.r = FLOAT_CANONICAL_NAN;
			r.second.f =  (REG(fx) & FLOAT_SIGN_MASK) | ((m_core->mode1 & MODE1_TRUNCATE) ? 0x7f7fffff : FLOAT_INFINITY);
		}
		else
		{
			// same sign - difference is NaN
			r.first.f =  (REG(fx) & FLOAT_SIGN_MASK) | ((m_core->mode1 & MODE1_TRUNCATE) ? 0x7f7fffff : FLOAT_INFINITY);
			r.second.r = FLOAT_CANONICAL_NAN;
		}

		m_core->astat |= (REG(fx) & FLOAT_SIGN_MASK) ? AN : 0;
		m_core->astat |= AV | AI;
		m_core->stky |= AVS | AIS;
	}

	m_core->astat |= AF;
	return r;
}

uint32_t adsp21062_device::SCALB(SHARC_REG fx, int ry)
{
	uint32_t const mantissa = fx.r & FLOAT_MANTISSA_MASK;
	uint32_t const sign = fx.r & FLOAT_SIGN_MASK;

	int exponent = float_get_unbiased_exponent(fx.r);
	exponent += int32_t(REG(ry));

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
		return sign | float_make_biased_exponent(exponent) | mantissa;
	}
}


/*****************************************************************************/
/* Multiplier helpers */

/* Fn = Fx * Fy */
adsp21062_device::SHARC_REG adsp21062_device::FMUL(int fx, int fy)
{
	SHARC_REG r;
	if (IS_FLOAT_NAN_MUL(REG(fx), REG(fy)))
	{
		r.r = FLOAT_CANONICAL_NAN;
		m_core->astat |= MI;
		m_core->stky |= MIS;
	}
	else
	{
		r.f = FREG(fx) * FREG(fy);

		m_core->astat |= (r.r & FLOAT_SIGN_MASK) ? MN : 0;
		m_core->astat |= IS_FLOAT_INFINITY(r.r) ? MV : 0;
		m_core->astat |= IS_FLOAT_DENORMAL(r.r) ? MU : 0;
		m_core->stky |= IS_FLOAT_INFINITY(r.r) ? MVS : 0;
		m_core->stky |= IS_FLOAT_DENORMAL(r.r) ? MUS : 0;
	}
	return r;
}


/*****************************************************************************/
/* Integer ALU operations */

/* Rn = Rx + Ry */
void adsp21062_device::compute_add(int rn, int rx, int ry)
{
	uint32_t r = REG(rx) + REG(ry);

	CLEAR_ALU_FLAGS();
	SET_FLAG_AV_ADD(r, REG(rx), REG(ry));
	SET_FLAG_AC_ADD(r, REG(rx), REG(ry));

	if ((m_core->mode1 & MODE1_ALUSAT) && (m_core->astat & AV))
		SATURATE(r);

	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);

	REG(rn) = r;
	m_core->astat &= ~AF;
}

/* Rn = Rx - Ry */
void adsp21062_device::compute_sub(int rn, int rx, int ry)
{
	uint32_t r = REG(rx) - REG(ry);

	CLEAR_ALU_FLAGS();
	SET_FLAG_AV_SUB(r, REG(rx), REG(ry));
	SET_FLAG_AC_SUB(r, REG(rx), REG(ry));

	if ((m_core->mode1 & MODE1_ALUSAT) && (m_core->astat & AV))
		SATURATE(r);

	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);

	REG(rn) = r;
	m_core->astat &= ~AF;
}

/* Rn = Rx + Ry + CI */
void adsp21062_device::compute_add_ci(int rn, int rx, int ry)
{
	int const c = (m_core->astat & AC) ? 1 : 0;
	uint32_t r = REG(rx) + REG(ry) + c;

	CLEAR_ALU_FLAGS();
	SET_FLAG_AV_ADD(r, REG(rx), REG(ry));
	SET_FLAG_AC_ADD(r, REG(rx), REG(ry));
	if ((c == 1) && (REG(ry) == 0xffffffff))
		m_core->astat |= AC;

	if ((m_core->mode1 & MODE1_ALUSAT) && (m_core->astat & AV))
		SATURATE(r);

	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);

	REG(rn) = r;
	m_core->astat &= ~AF;
}

/* Rn = Rx - Ry + CI - 1 */
void adsp21062_device::compute_sub_ci(int rn, int rx, int ry)
{
	int const c = (m_core->astat & AC) ? 1 : 0;
	uint32_t r = REG(rx) - REG(ry) + c - 1;

	CLEAR_ALU_FLAGS();
	SET_FLAG_AV_SUB(r, REG(rx), REG(ry));
	if ((c != 0) || (REG(ry) != 0xffffffff))
		SET_FLAG_AC_SUB(r, REG(rx), REG(ry));

	if ((m_core->mode1 & MODE1_ALUSAT) && (m_core->astat & AV))
		SATURATE(r);

	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);

	REG(rn) = r;
	m_core->astat &= ~AF;
}

/* COMP(Rx, Ry) */
void adsp21062_device::compute_comp(int rx, int ry)
{
	CLEAR_ALU_FLAGS();
	if (REG(rx) == REG(ry))
		m_core->astat |= AZ;
	else if (int32_t(REG(rx)) < int32_t(REG(ry)))
		m_core->astat |= AN;

	// Update ASTAT compare accumulation register
	uint32_t comp_accum = (m_core->astat >> 1) & 0x7f000000;
	if ((m_core->astat & (AZ | AN)) == 0)
		comp_accum |= 0x80000000;

	m_core->astat &= 0x00ffffff;
	m_core->astat |= comp_accum;

	m_core->astat &= ~AF;
}

/* Rn = Rx + CI */
void adsp21062_device::compute_add_ci(int rn, int rx)
{
	int const c = (m_core->astat & AC) ? 1 : 0;
	uint32_t r = REG(rx) + c;

	CLEAR_ALU_FLAGS();
	SET_FLAG_AV_ADD(r, REG(rx), 0);
	SET_FLAG_AC_ADD(r, REG(rx), 0);

	if ((m_core->mode1 & MODE1_ALUSAT) && (m_core->astat & AV))
		SATURATE(r);

	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);

	REG(rn) = r;
	m_core->astat &= ~AF;
}

/* Rn = Rx + CI - 1 */
void adsp21062_device::compute_sub_ci(int rn, int rx)
{
	int const c = (m_core->astat & AC) ? 1 : 0;
	uint32_t r = REG(rx) + c - 1;

	CLEAR_ALU_FLAGS();
	SET_FLAG_AV_SUB(r, REG(rx), 0);
	SET_FLAG_AC_SUB(r, REG(rx), 0);

	if ((m_core->mode1 & MODE1_ALUSAT) && (m_core->astat & AV))
		SATURATE(r);

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
	SET_FLAG_AV_ADD(r, REG(rx), 1);
	SET_FLAG_AC_ADD(r, REG(rx), 1);

	if ((m_core->mode1 & MODE1_ALUSAT) && (m_core->astat & AV))
		SATURATE(r);

	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);

	REG(rn) = r;
	m_core->astat &= ~AF;
}

/* Rn = Rx - 1 */
void adsp21062_device::compute_dec(int rn, int rx)
{
	uint32_t r = REG(rx) - 1;

	CLEAR_ALU_FLAGS();
	SET_FLAG_AV_SUB(r, REG(rx), 1);
	SET_FLAG_AC_SUB(r, REG(rx), 1);

	if ((m_core->mode1 & MODE1_ALUSAT) && (m_core->astat & AV))
		SATURATE(r);

	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);

	REG(rn) = r;
	m_core->astat &= ~AF;
}

/* Rn = -Rx */
void adsp21062_device::compute_neg(int rn, int rx)
{
	uint32_t r = -int32_t(REG(rx));

	CLEAR_ALU_FLAGS();
	SET_FLAG_AV_SUB(r, 0, REG(rx));
	SET_FLAG_AC_SUB(r, 0, REG(rx));

	if ((m_core->mode1 & MODE1_ALUSAT) && (m_core->astat & AV))
		SATURATE(r);

	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);

	REG(rn) = r;
	m_core->astat &= ~AF;
}

/* Rn = ABS Rx */
void adsp21062_device::compute_abs(int rn, int rx)
{
	uint32_t r = std::abs(int32_t(REG(rx)));

	CLEAR_ALU_FLAGS();
	if (int32_t(r) < 0)
	{
		m_core->astat |= AV;
		m_core->stky |= AOS;
		if (m_core->mode1 & MODE1_ALUSAT)
			r = 0x7fffffff;
	}

	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);
	m_core->astat |= (int32_t(REG(rx)) < 0) ? AS : 0;

	REG(rn) = r;
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

/* Rn = Rx AND Ry */
void adsp21062_device::compute_and(int rn, int rx, int ry)
{
	uint32_t const r = REG(rx) & REG(ry);

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);

	REG(rn) = r;
	m_core->astat &= ~AF;
}

/* Rn = Rx OR Ry */
void adsp21062_device::compute_or(int rn, int rx, int ry)
{
	uint32_t const r = REG(rx) | REG(ry);

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);

	REG(rn) = r;
	m_core->astat &= ~AF;
}

/* Rn = Rx XOR Ry */
void adsp21062_device::compute_xor(int rn, int rx, int ry)
{
	uint32_t const r = REG(rx) ^ REG(ry);

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);

	REG(rn) = r;
	m_core->astat &= ~AF;
}

/* Rn = NOT Rx */
void adsp21062_device::compute_not(int rn, int rx)
{
	uint32_t const r = ~REG(rx);

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);

	REG(rn) = r;
	m_core->astat &= ~AF;
}

/* Rn = MIN(Rx, Ry) */
void adsp21062_device::compute_min(int rn, int rx, int ry)
{
	uint32_t const r = std::min(int32_t(REG(rx)), int32_t(REG(ry)));

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);

	REG(rn) = r;
	m_core->astat &= ~AF;
}

/* Rn = MAX(Rx, Ry) */
void adsp21062_device::compute_max(int rn, int rx, int ry)
{
	uint32_t const r = std::max(int32_t(REG(rx)), int32_t(REG(ry)));

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);

	REG(rn) = r;
	m_core->astat &= ~AF;
}

/* Rn = CLIP Rx BY Ry */
void adsp21062_device::compute_clip(int rn, int rx, int ry)
{
	const int32_t absry = std::abs(int32_t(REG(ry)));
	const uint32_t r = std::clamp(int32_t(REG(rx)), -absry, absry);

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(r);
	SET_FLAG_AZ(r);

	REG(rn) = r;
	m_core->astat &= ~AF;
}


/*****************************************************************************/
/* Floating-point ALU operations */

/* Fn = Fx + Fy */
void adsp21062_device::compute_fadd(int fn, int fx, int fy)
{
	CLEAR_ALU_FLAGS();
	FREG(fn) = FADD(fx, fy).f;
}

/* Fn = Fx - Fy */
void adsp21062_device::compute_fsub(int fn, int fx, int fy)
{
	CLEAR_ALU_FLAGS();
	FREG(fn) = FSUB(fx, fy).f;
}

/* Fn = ABS(Fx + Fy) */
void adsp21062_device::compute_fadd_abs(int fn, int fx, int fy)
{
	CLEAR_ALU_FLAGS();

	SHARC_REG r;
	if (IS_FLOAT_NAN_ADD(REG(fx), REG(fy)))
	{
		r.r = FLOAT_CANONICAL_NAN;
		m_core->astat |= AI;
		m_core->stky |= AIS;
	}
	else
	{
		SHARC_REG x, y;
		x.r = FLOAT_FLUSH_DENORMAL(REG(fx));
		y.r = FLOAT_FLUSH_DENORMAL(REG(fy));
		r.f = x.f + y.f;
		r.r &= ~FLOAT_SIGN_MASK;

		if (r.r == FLOAT_EXPONENT_MASK) // sign bit won't be set, avoid redundant masking
		{
			m_core->astat |= AV;
			m_core->stky |= AVS;
			if (m_core->mode1 & MODE1_TRUNCATE)
				r.r = 0x7f7fffff;
		}
		else if (IS_FLOAT_DENORMAL_OR_ZERO(r.r))
		{
			m_core->astat |= AZ;
			m_core->stky |= ((r.r & FLOAT_MANTISSA_MASK) != 0) ? AUS : 0;
			r.r = 0;
		}
	}

	FREG(fn) = r.f;
	m_core->astat |= AF;
}

/* Fn = ABS(Fx - Fy) */
void adsp21062_device::compute_fsub_abs(int fn, int fx, int fy)
{
	CLEAR_ALU_FLAGS();

	SHARC_REG r;
	if (IS_FLOAT_NAN_SUB(REG(fx), REG(fy)))
	{
		r.r = FLOAT_CANONICAL_NAN;
		m_core->astat |= AI;
		m_core->stky |= AIS;
	}
	else
	{
		SHARC_REG x, y;
		x.r = FLOAT_FLUSH_DENORMAL(REG(fx));
		y.r = FLOAT_FLUSH_DENORMAL(REG(fy));
		r.f = x.f - y.f;
		r.r &= ~FLOAT_SIGN_MASK;

		if (r.r == FLOAT_EXPONENT_MASK) // sign bit won't be set, avoid redundant masking
		{
			m_core->astat |= AV;
			m_core->stky |= AVS;
			if (m_core->mode1 & MODE1_TRUNCATE)
				r.r = 0x7f7fffff;
		}
		else if (IS_FLOAT_DENORMAL_OR_ZERO(r.r))
		{
			m_core->astat |= AZ;
			m_core->stky |= ((r.r & FLOAT_MANTISSA_MASK) != 0) ? AUS : 0;
			r.r = 0;
		}
	}

	FREG(fn) = r.f;
	m_core->astat |= AF;
}

/* Fn = (Fx + Fy) / 2 */
void adsp21062_device::compute_favg(int fn, int fx, int fy)
{
	CLEAR_ALU_FLAGS();
	FREG(fn) = FAVG(fx, fy).f;
}

/* COMP(Fx, Fy) */
void adsp21062_device::compute_fcomp(int fx, int fy)
{
	CLEAR_ALU_FLAGS();
	if (IS_FLOAT_NAN(REG(fx)) || IS_FLOAT_NAN(REG(fy)))
	{
		m_core->astat |= AI;
		m_core->stky |= AIS;
	}
	else
	{
		if (FREG(fx) == FREG(fy))
			m_core->astat |= AZ;
		else if (FREG(fx) < FREG(fy))
			m_core->astat |= AN;
	}

	// Update ASTAT compare accumulation register
	// TODO: confirm whether this is set for unordered operands
	uint32_t comp_accum = (m_core->astat >> 1) & 0x7f000000;
	if ((m_core->astat & (AZ | AN)) == 0)
		comp_accum |= 0x80000000;

	m_core->astat &= 0x00ffffff;
	m_core->astat |= comp_accum;

	m_core->astat |= AF;
}

/* Fn = -Fx */
void adsp21062_device::compute_fneg(int fn, int fx)
{
	CLEAR_ALU_FLAGS();

	SHARC_REG r;
	if (IS_FLOAT_NAN(REG(fx)))
	{
		r.r = FLOAT_CANONICAL_NAN;
		m_core->astat |= AI;
		m_core->stky |= AIS;
	}
	else
	{
		if (IS_FLOAT_DENORMAL_OR_ZERO(REG(fx)))
		{
			r.r = ~REG(fx) & FLOAT_SIGN_MASK;
			m_core->astat |= AZ;
		}
		else
		{
			r.r = REG(fx) ^ FLOAT_SIGN_MASK;
		}
		m_core->astat |= (r.r & FLOAT_SIGN_MASK) ? AN : 0;
	}

	FREG(fn) = r.f;
	m_core->astat |= AF;
}

/* Fn = ABS Fx */
void adsp21062_device::compute_fabs(int fn, int fx)
{
	CLEAR_ALU_FLAGS();
	FREG(fn) = FABS(fx).f;
}

/* Fn = PASS Fx */
void adsp21062_device::compute_fpass(int fn, int fx)
{
	CLEAR_ALU_FLAGS();

	SHARC_REG r;
	if (IS_FLOAT_NAN(REG(fx)))
	{
		r.r = FLOAT_CANONICAL_NAN;
		m_core->astat |= AI;
		m_core->stky |= AIS;
	}
	else
	{
		if (IS_FLOAT_DENORMAL_OR_ZERO(REG(fx)))
		{
			r.r = REG(fx) & FLOAT_SIGN_MASK;
			m_core->astat |= AZ;
		}
		else
		{
			r.r = REG(fx);
		}
		m_core->astat |= (r.r & FLOAT_SIGN_MASK) ? AN : 0;
	}

	FREG(fn) = r.f;
	m_core->astat |= AF;
}

/* Fn = SCALB Fx BY Fy */
void adsp21062_device::compute_scalb(int fn, int fx, int ry)
{
	// verified
	SHARC_REG r;
	CLEAR_ALU_FLAGS();

	if (IS_FLOAT_NAN(REG(fx)))
	{
		m_core->astat |= AI;
		m_core->stky |= AIS;

		REG(fn) = FLOAT_CANONICAL_NAN;
	}
	else
	{
		r.r = SCALB(m_core->r[fx], ry);

		// AN
		SET_FLAG_AN(r.r);
		// AZ
		m_core->astat |= IS_FLOAT_ZERO(r.r) ? AZ : 0;
		// AUS
		m_core->stky |= (IS_FLOAT_DENORMAL(r.r)) ? AUS : 0;

		FREG(fn) = r.f;
	}
	m_core->astat |= AF;
}

/* Rn = LOGB Fx */
void adsp21062_device::compute_logb(int rn, int fx)
{
	// verified
	uint32_t r = REG(fx);

	CLEAR_ALU_FLAGS();

	if (IS_FLOAT_INFINITY(REG(fx)))
	{
		REG(rn) = FLOAT_INFINITY;

		m_core->astat |= AV;
	}
	else if (IS_FLOAT_ZERO(REG(fx)))
	{
		REG(rn) = FLOAT_SIGN_MASK | FLOAT_INFINITY;

		m_core->astat |= AV;
	}
	else if (IS_FLOAT_NAN(REG(fx)))
	{
		REG(rn) = FLOAT_CANONICAL_NAN;

		m_core->astat |= AI;
		m_core->stky |= AIS;
	}
	else
	{
		int exponent = float_get_unbiased_exponent(r);

		// AN
		SET_FLAG_AN(exponent);
		// AZ
		SET_FLAG_AZ(exponent);

		REG(rn) = exponent;
	}
	m_core->astat |= AF;
}

/* Rn = FIX Fx BY Ry */
void adsp21062_device::compute_fix_scaled(int rn, int fx, int ry)
{
	int32_t alu_i;
	SHARC_REG r_alu;

	r_alu.r = SCALB(m_core->r[fx], ry);
	if (m_core->mode1 & MODE1_TRUNCATE)
	{
		alu_i = int32_t(floorf(r_alu.f));
	}
	else
	{
		alu_i = int32_t(nearbyintf(r_alu.f)); // assume rounding mode is set to FE_TONEAREST
	}

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(alu_i);
	// AZ
	SET_FLAG_AZ(alu_i);
	// AU
	m_core->stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	// AI
	m_core->astat |= (IS_FLOAT_NAN(REG(fx))) ? AI : 0;
	/* TODO: AV flag */

	REG(rn) = alu_i;
	m_core->astat |= AF;
}

/* Rn = FIX Fx */
void adsp21062_device::compute_fix(int rn, int fx)
{
	int32_t alu_i;
	SHARC_REG r_alu;

	r_alu.f = FREG(fx);
	if (m_core->mode1 & MODE1_TRUNCATE)
	{
		alu_i = int32_t(floorf(r_alu.f));
	}
	else
	{
		alu_i = int32_t(nearbyintf(r_alu.f)); // assume rounding mode is set to FE_TONEAREST
	}

	CLEAR_ALU_FLAGS();
	SET_FLAG_AN(alu_i);
	// AZ
	SET_FLAG_AZ(alu_i);
	// AU
	m_core->stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	// AI
	m_core->astat |= (IS_FLOAT_NAN(REG(fx))) ? AI : 0;
	/* TODO: AV flag */

	REG(rn) = alu_i;
	m_core->astat |= AF;
}

/* Fn = FLOAT Rx BY Ry */
void adsp21062_device::compute_float_scaled(int fn, int rx, int ry)
{
	SHARC_REG x;
	x.f = float(int32_t(REG(rx)));

	// verified
	CLEAR_ALU_FLAGS();

	REG(fn) = SCALB(x, ry);

	// AN
	SET_FLAG_AN(REG(fn));
	// AZ
	m_core->astat |= IS_FLOAT_DENORMAL_OR_ZERO(REG(fn)) ? AZ : 0;
	// AU
	m_core->stky |= IS_FLOAT_DENORMAL(REG(fn)) ? AUS : 0;

	m_core->astat |= AF;
}

/* Fn = FLOAT Rx */
void adsp21062_device::compute_float(int fn, int rx)
{
	// verified
	FREG(fn) = (float)(int32_t)REG(rx);

	CLEAR_ALU_FLAGS();
	// AN
	SET_FLAG_AN(REG(fn));
	// AZ
	m_core->astat |= IS_FLOAT_DENORMAL_OR_ZERO(REG(fn)) ? AZ : 0;
	// AUS
	m_core->stky |= IS_FLOAT_DENORMAL(REG(fn)) ? AUS : 0;
	/* TODO: AV flag */

	m_core->astat |= AF;
}

/* Fn = RECIPS Fx */
void adsp21062_device::compute_recips(int fn, int fx)
{
	// verified
	uint32_t r;

	CLEAR_ALU_FLAGS();

	if (IS_FLOAT_NAN(REG(fx)))
	{
		// NaN
		r = FLOAT_CANONICAL_NAN;

		// AI
		m_core->astat |= AI;

		// AIS
		m_core->stky |= AIS;
	}
	else if (IS_FLOAT_ZERO(REG(fx)))
	{
		// +- Zero
		r = (REG(fx) & FLOAT_SIGN_MASK) | FLOAT_INFINITY;

		m_core->astat |= AV;
	}
	else
	{
		uint32_t mantissa = REG(fx) & FLOAT_MANTISSA_MASK;
		uint32_t sign = REG(fx) & FLOAT_SIGN_MASK;

		int res_exponent = -float_get_unbiased_exponent(REG(fx)) - 1;

		uint32_t res_mantissa = recips_mantissa_lookup[mantissa >> 16];

		if (res_exponent > 125 || res_exponent < -126)
		{
			res_exponent = 0;
			res_mantissa = 0;
		}
		else
		{
			res_exponent = (res_exponent + FLOAT_EXPONENT_BIAS) & 0xff;
		}

		r = sign | (uint32_t(res_exponent) << FLOAT_EXPONENT_SHIFT) | res_mantissa;

		SET_FLAG_AN(REG(fx));
		// AZ
		m_core->astat |= (IS_FLOAT_ZERO(r)) ? AZ : 0;
	}

	REG(fn) = r;
	m_core->astat |= AF;
}

/* Fn = RSQRTS Fx */
void adsp21062_device::compute_rsqrts(int fn, int fx)
{
	// verified
	uint32_t r;

	if (uint32_t(REG(fx)) > 0x80000000)
	{
		// non-zero negative
		r = FLOAT_CANONICAL_NAN;
	}
	else if (IS_FLOAT_NAN(REG(fx)))
	{
		// NaN
		r = FLOAT_CANONICAL_NAN;
	}
	else
	{
		uint32_t mantissa = REG(fx) & 0xffffff;   // mantissa + LSB of biased exponent
		uint32_t sign = REG(fx) & FLOAT_SIGN_MASK;

		int32_t res_exponent = -(float_get_unbiased_exponent(REG(fx)) >> 1) - 1;

		uint32_t res_mantissa = rsqrts_mantissa_lookup[mantissa >> 17];

		r = sign | float_make_biased_exponent(res_exponent) | res_mantissa;
	}

	CLEAR_ALU_FLAGS();
	// AN
	m_core->astat |= (REG(fx) == 0x80000000) ? AN : 0;
	// AZ & AV
	m_core->astat |= (IS_FLOAT_ZERO(r)) ? AZ : 0;
	m_core->astat |= (IS_FLOAT_ZERO(REG(fx))) ? AV : 0;
	// AI
	m_core->astat |= (IS_FLOAT_NAN(REG(fx)) || (REG(fx) & FLOAT_SIGN_MASK)) ? AI : 0;
	// AIS
	if (m_core->astat & AI)   m_core->stky |= AIS;

	REG(fn) = r;
	m_core->astat |= AF;
}

/* Fn = COPYSIGN(Fx, Fy) */
void adsp21062_device::compute_fcopysign(int fn, int fx, int fy)
{
	CLEAR_ALU_FLAGS();

	SHARC_REG r;
	if (IS_FLOAT_NAN(REG(fx)) || IS_FLOAT_NAN(REG(fy)))
	{
		r.r = FLOAT_CANONICAL_NAN;
		m_core->astat |= AI;
		m_core->stky |= AIS;
	}
	else
	{
		r.r = REG(fy) & FLOAT_SIGN_MASK;
		m_core->astat |= r.r ? AN : 0; // only the sign bit is set here so it avoids need to mask
		if (IS_FLOAT_DENORMAL_OR_ZERO(REG(fx)))
			m_core->astat |= AZ;
		else
			r.r |= REG(fx) & ~FLOAT_SIGN_MASK;
	}

	FREG(fn) = r.f;
	m_core->astat |= AF;
}

/* Fn = MIN(Fx, Fy) */
void adsp21062_device::compute_fmin(int fn, int fx, int fy)
{
	CLEAR_ALU_FLAGS();
	FREG(fn) = FMIN(fx, fy).f;
}

/* Fn = MAX(Fx, Fy) */
void adsp21062_device::compute_fmax(int fn, int fx, int fy)
{
	CLEAR_ALU_FLAGS();
	FREG(fn) = FMAX(fx, fy).f;
}

/* Fn = CLIP Fx BY Fy */
void adsp21062_device::compute_fclip(int fn, int fx, int fy)
{
	CLEAR_ALU_FLAGS();

	SHARC_REG r;
	if (IS_FLOAT_NAN(REG(fx)) || IS_FLOAT_NAN(REG(fy)))
	{
		r.r = FLOAT_CANONICAL_NAN;
		m_core->astat |= AI;
		m_core->stky |= AIS;
	}
	else
	{
		if (IS_FLOAT_DENORMAL_OR_ZERO(REG(fx)) || IS_FLOAT_DENORMAL_OR_ZERO(REG(fy)))
		{
			r.r = REG(fx) & FLOAT_SIGN_MASK;
			m_core->astat |= AZ;
		}
		else
		{
			SHARC_REG absry, negabsry;
			absry.r = REG(fy) & ~FLOAT_SIGN_MASK;
			negabsry.r = REG(fy) | FLOAT_SIGN_MASK;
			r.f = std::clamp(FREG(fx), negabsry.f, absry.f);
			m_core->astat |= IS_FLOAT_ZERO(r.r) ? AZ : 0;
		}
		m_core->astat |= (r.r & FLOAT_SIGN_MASK) ? AN : 0;
	}

	FREG(fn) = r.f;
	m_core->astat |= AF;
}


/*****************************************************************************/
/* Multiplier opcodes */

/* Rn = (unsigned)Rx * (unsigned)Ry, integer, no rounding */
void adsp21062_device::compute_mul_uuin(int rn, int rx, int ry)
{
	uint64_t r = mulu_32x32(REG(rx), REG(ry));

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN(uint32_t(r));
	SET_FLAG_MV(r);
	SET_FLAG_MU(r);

	REG(rn) = uint32_t(r);
}

/* Rn = (signed)Rx * (signed)Ry, integer, no rounding */
void adsp21062_device::compute_mul_ssin(int rn, int rx, int ry)
{
	uint64_t r = mul_32x32(REG(rx), REG(ry));

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN(uint32_t(r));
	SET_FLAG_MV(r);
	SET_FLAG_MU(r);

	REG(rn) = uint32_t(r);
}

/* MRF + (signed)Rx * (signed)Ry, integer, no rounding */
uint32_t adsp21062_device::compute_mrf_plus_mul_ssin(int rx, int ry)
{
	uint64_t r = m_core->mrf + mul_32x32(REG(rx), REG(ry));

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN(uint32_t(r));
	SET_FLAG_MV(r);
	SET_FLAG_MU(r);

	return uint32_t(r);
}

/* MRB + (signed)Rx * (signed)Ry, integer, no rounding */
uint32_t adsp21062_device::compute_mrb_plus_mul_ssin(int rx, int ry)
{
	int64_t r = m_core->mrb + mul_32x32(REG(rx), REG(ry));

	CLEAR_MULTIPLIER_FLAGS();
	SET_FLAG_MN(uint32_t(r));
	SET_FLAG_MV(r);
	SET_FLAG_MU(r);

	return uint32_t(r);
}

/* Fn = Fx * Fy */
void adsp21062_device::compute_fmul(int fn, int fx, int fy)
{
	CLEAR_MULTIPLIER_FLAGS();
	FREG(fn) = FMUL(fx, fy).f;
}

/*****************************************************************************/

/* multi function opcodes */

/* integer*/
void adsp21062_device::compute_multi_mr_to_reg(int ai, int rk)
{
	switch (ai)
	{
		case 0:     SET_UREG(rk, uint32_t(m_core->mrf)); break;
		case 1:     SET_UREG(rk, uint32_t(m_core->mrf >> 32)); break;
		case 2:     fatalerror("SHARC: tried to load MR2F\n"); break;
		case 4:     SET_UREG(rk, uint32_t(m_core->mrb)); break;
		case 5:     SET_UREG(rk, uint32_t(m_core->mrb >> 32)); break;
		case 6:     fatalerror("SHARC: tried to load MR2B\n"); break;
		default:    fatalerror("SHARC: unknown ai %d in mr_to_reg\n", ai);
	}

	CLEAR_MULTIPLIER_FLAGS();
}

void adsp21062_device::compute_multi_reg_to_mr(int ai, int rk)
{
	switch (ai)
	{
		case 0:     m_core->mrf &= ~0xffffffff; m_core->mrf |= GET_UREG(rk); break;
		case 1:     m_core->mrf &= 0xffffffff; m_core->mrf |= uint64_t(GET_UREG(rk)) << 32; break;
		case 2:     fatalerror("SHARC: tried to write MR2F\n"); break;
		case 4:     m_core->mrb &= ~0xffffffff; m_core->mrb |= GET_UREG(rk); break;
		case 5:     m_core->mrb &= 0xffffffff; m_core->mrb |= uint64_t(GET_UREG(rk)) << 32; break;
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
	bool const av_add = ~(REG(rx) ^ REG(ry)) & (REG(rx) ^ r_add) & 0x80000000;
	bool const av_sub = (REG(rx) ^ REG(ry)) & (REG(rx) ^ r_sub) & 0x80000000;

	CLEAR_ALU_FLAGS();
	if (av_add || av_sub)
	{
		m_core->astat |= AV;
		m_core->stky |= AOS;
	}
	m_core->astat |= ((r_add < uint32_t(REG(rx))) || (r_sub <= uint32_t(REG(rx)))) ? AC : 0;

	if (m_core->mode1 & MODE1_ALUSAT)
	{
		if (av_add) SATURATE(r_add);
		if (av_sub) SATURATE(r_sub);
	}

	m_core->astat |= ((r_add == 0) || (r_sub == 0)) ? AZ : 0;
	m_core->astat |= ((r_add | r_sub) & 0x80000000) ? AN : 0;

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


/*****************************************************************************/
/* Dual add/subtract */

/* Fa = Fx + Fy,   Fs = Fx - Fy */
void adsp21062_device::compute_dual_fadd_fsub(int fa, int fs, int fx, int fy)
{
	CLEAR_ALU_FLAGS();
	auto [r_add, r_sub] = FADD_FSUB(fx, fy);
	FREG(fa) = r_add.f;
	FREG(fs) = r_sub.f;
}


/*****************************************************************************/
/* Floating-point multiplication and ALU operation */

/* Fm = Fxm * Fym,   Fa = Fxa + Fya */
void adsp21062_device::compute_fmul_fadd(int fm, int fxm, int fym, int fa, int fxa, int fya)
{
	CLEAR_MULTIPLIER_FLAGS();
	CLEAR_ALU_FLAGS();

	SHARC_REG r_mul = FMUL(fxm, fym);
	SHARC_REG r_alu = FADD(fxa, fya);

	FREG(fm) = r_mul.f;
	FREG(fa) = r_alu.f;
}

/* Fm = Fxm * Fym,   Fa = Fxa - Fya */
void adsp21062_device::compute_fmul_fsub(int fm, int fxm, int fym, int fa, int fxa, int fya)
{
	CLEAR_MULTIPLIER_FLAGS();
	CLEAR_ALU_FLAGS();

	SHARC_REG r_mul = FMUL(fxm, fym);
	SHARC_REG r_alu = FSUB(fxa, fya);

	FREG(fm) = r_mul.f;
	FREG(fa) = r_alu.f;
}

/* Fm = Fxm * Fym,   Fa = FLOAT Rxa BY Rya */
void adsp21062_device::compute_fmul_float_scaled(int fm, int fxm, int fym, int fa, int rxa, int rya)
{
	CLEAR_MULTIPLIER_FLAGS();
	CLEAR_ALU_FLAGS();

	SHARC_REG r_mul = FMUL(fxm, fym);

	SHARC_REG x;
	SHARC_REG r_alu;

	x.f = float(int32_t(REG(rxa)));

	r_alu.r = SCALB(x, rya);

	m_core->astat |= (r_alu.f < 0.0f) ? AN : 0;
	// AZ
	m_core->astat |= IS_FLOAT_DENORMAL_OR_ZERO(r_alu.r) ? AZ : 0;
	// AU
	m_core->stky |= IS_FLOAT_DENORMAL(r_alu.r) ? AUS : 0;
	/* TODO: set AV if overflowed */

	FREG(fm) = r_mul.f;
	FREG(fa) = r_alu.f;
	m_core->astat |= AF;
}

/* Fm = Fxm * Fym,   Ra = FIX Fxa BY Rya */
void adsp21062_device::compute_fmul_fix_scaled(int fm, int fxm, int fym, int ra, int fxa, int rya)
{
	CLEAR_MULTIPLIER_FLAGS();
	CLEAR_ALU_FLAGS();

	SHARC_REG r_mul = FMUL(fxm, fym);

	int32_t alu_i;
	SHARC_REG r_alu;

	r_alu.r = SCALB(m_core->r[fxa], rya);

	if (m_core->mode1 & MODE1_TRUNCATE)
	{
		alu_i = int32_t(r_alu.f);
	}
	else
	{
		alu_i = int32_t(nearbyintf(r_alu.f)); // assume rounding mode is set to FE_TONEAREST
	}

	SET_FLAG_AN(alu_i);
	// AZ
	SET_FLAG_AZ(alu_i);
	// AU
	m_core->stky |= (IS_FLOAT_DENORMAL(r_alu.r)) ? AUS : 0;
	// AI
	m_core->astat |= (IS_FLOAT_NAN(REG(fxa))) ? AI : 0;
	/* TODO: AV flag */

	FREG(fm) = r_mul.f;
	REG(ra) = alu_i;
	m_core->astat |= AF;
}

void adsp21062_device::compute_fmul_favg(int fm, int fxm, int fym, int fa, int fxa, int fya)
{
	CLEAR_MULTIPLIER_FLAGS();
	CLEAR_ALU_FLAGS();

	SHARC_REG r_mul = FMUL(fxm, fym);
	SHARC_REG r_alu = FAVG(fxa, fya);

	FREG(fm) = r_mul.f;
	FREG(fa) = r_alu.f;
}

void adsp21062_device::compute_fmul_fabs(int fm, int fxm, int fym, int fa, int fxa)
{
	CLEAR_MULTIPLIER_FLAGS();
	CLEAR_ALU_FLAGS();

	SHARC_REG r_mul = FMUL(fxm, fym);
	SHARC_REG r_alu = FABS(fxa);

	FREG(fm) = r_mul.f;
	FREG(fa) = r_alu.f;
}

/* Fm = Fxm * Fym,   Fa = MAX(Fxa, Fya) */
void adsp21062_device::compute_fmul_fmax(int fm, int fxm, int fym, int fa, int fxa, int fya)
{
	CLEAR_MULTIPLIER_FLAGS();
	CLEAR_ALU_FLAGS();

	SHARC_REG r_mul = FMUL(fxm, fym);
	SHARC_REG r_alu = FMAX(fxa, fya);

	FREG(fm) = r_mul.f;
	FREG(fa) = r_alu.f;
}


/* Fm = Fxm * Fym,   Fa = MIN(Fxa, Fya) */
void adsp21062_device::compute_fmul_fmin(int fm, int fxm, int fym, int fa, int fxa, int fya)
{
	CLEAR_MULTIPLIER_FLAGS();
	CLEAR_ALU_FLAGS();

	SHARC_REG r_mul = FMUL(fxm, fym);
	SHARC_REG r_alu = FMIN(fxa, fya);

	FREG(fm) = r_mul.f;
	FREG(fa) = r_alu.f;
}


/*****************************************************************************/
/* Multiplication and dual add/subtract */

/* Fm = Fxm * Fym,   Fa = Fxa + Fya,   Fs = Fxa - Fya */
void adsp21062_device::compute_fmul_dual_fadd_fsub(int fm, int fxm, int fym, int fa, int fs, int fxa, int fya)
{
	CLEAR_MULTIPLIER_FLAGS();
	CLEAR_ALU_FLAGS();

	SHARC_REG r_mul = FMUL(fxm, fym);
	auto [r_add, r_sub] = FADD_FSUB(fxa, fya);

	FREG(fm) = r_mul.f;
	FREG(fa) = r_add.f;
	FREG(fs) = r_sub.f;
}
