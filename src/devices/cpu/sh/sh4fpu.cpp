// license:MIT
// copyright-holders:Stefanos Kornilios Mitsis Poiitidis, R. Belmont
/*****************************************************************************
 *
 *   sh4fpu.cpp
 *
 *   SH-4 vector and approximation FPU instructions: FIPR, FTRV, FSCA, FSRRA
 *
 *   Derived from https://gist.github.com/skmp/4bd7b38149fc455978b5104201573236
 *
 *   None of these is an IEEE-754 operation.  All four run on one hardware
 *   block (Hitachi, US patent 6,038,582): four truncated Booth radix-4
 *   multipliers, an aligner per product, a four-input adder and a single
 *   normalise and round stage.  FIPR uses the block once and FTRV is four
 *   FIPRs.  FSCA and FSRRA are microprograms of four and three steps that
 *   evaluate a cubic around a table node through the same multipliers,
 *   feeding product registers back in as operands.
 *
 *   The multipliers
 *     Each forms a 28-bit product from a Booth recoded operand and a
 *     multiplicand.  Partial product rows are cut below column 16 of the
 *     product, a negative row is a one's complement whose hot one is only
 *     there from column 18 up, rows 0-2 add a majority carry from column
 *     15 and rows 6-8 lose the parity of their column 16 bits.  2^19 is
 *     added and the low 20 bits are dropped.  FIPR recodes the FVn operand
 *     and FTRV the matrix element.
 *
 *   FIPR, FTRV
 *     The four products are aligned to the largest product exponent by
 *     truncation, without guard bits, added exactly, and the sum is
 *     rounded once.
 *     - A product with a zero operand, or whose biased exponents add up
 *       to 75 or less, is discarded.
 *     - With FPSCR.DN set denormal operands are zeroes and a result below
 *       the normal range is a signed zero.  With it clear a denormal
 *       enters the multiplier as it is (no hidden bit, exponent 1) and a
 *       result below the normal range is a denormal rounded at its own
 *       position.  There is no FPU error.
 *     - Any NaN operand, infinity * 0 or infinities of both signs give
 *       the qNaN.  One infinity keeps its sign, while two or more of the
 *       same sign take the sign of the first product, whatever that
 *       product is.  An exact zero sum is +0.
 *     - A finite result is always inexact.  It also underflows if its
 *       exponent field is zero, 0 * 0 included.
 *
 *   FSCA
 *     The turn is cut into 64 nodes and a cubic around the node centre is
 *     evaluated on a 2^-26 grid.  The coefficients come from a ROM of 17
 *     words holding values derived from sin(m * pi / 32).  An instruction
 *     reads word m and word 16 - m: in an odd quadrant the two swap roles
 *     and the odd terms change sign, in the second half turn the result
 *     is negated.  The upper half of FPUL is ignored and the result is
 *     always inexact.
 *
 *   FSRRA
 *     The mantissa is split around the nearest of 17 nodes per exponent
 *     parity and a cubic is evaluated on a 2^-26 grid.  The exponent of
 *     the result depends on the exponent of the operand alone.  The
 *     result is always inexact, for powers of four too.
 *
 *   With FPSCR.PR set all four instructions do nothing at all.
 *
 *   FPU exception traps are not emulated.  On hardware a trap leaves the
 *   destination alone while the cause and flag fields are still updated,
 *   and what the enables do isn't what the manual says:
 *     FSCA   traps if the inexact enable is set
 *     FSRRA  traps on an FPU error, on V and Z if they are enabled, and
 *            always if the inexact enable is set, even for the operands
 *            that raise nothing
 *     FIPR   traps on V if that is enabled, and always if any of the O,
 *            U and I enables is set
 *     FTRV   always traps if any of the V, O, U and I enables is set
 *   With FPSCR.PR set FSCA, FSRRA and FIPR still trap if the inexact
 *   enable is set, and FTRV if any of V, O, U and I is.
 *
 *****************************************************************************/

#include "emu.h"
#include "sh4fpu.h"

#include <algorithm>
#include <bit>


namespace sh4fpu {

namespace {

constexpr u32 SIGN = 0x8000'0000;
constexpr u32 QNAN = 0x7fbf'ffff;
constexpr u32 INFINITY_BITS = 0x7f80'0000;
constexpr u32 LARGEST_BITS = 0x7f7f'ffff;

// every product register adds this before it drops its low 20 bits
constexpr s64 ROUND = s64(1) << 19;

// a FIPR product register p is worth p * 2^(ea + eb - PRODUCT_SCALE), ea and eb being biased
// exponents: two biases and two 23-bit fractions, less the 20 bits the register drops
constexpr int PRODUCT_SCALE = (2 * (127 + 23)) - 20;

// the FSCA even term's constant, about 4 * pi
constexpr s64 FSCA_KC = 0x648340;

struct fsca_word
{
	u32 c0;     // sine at the node centre
	u32 l;      // sin * pi / 2: 23-bit normalised field...
	u8  e;      // ...and its exponent
	u16 q;      // sin * 2646
};

struct fsrra_word
{
	u32 c0;     // reciprocal square root at the node
	u32 rec1;   // linear coefficient: 23-bit normalised field...
	u8  e1;     // ...and its exponent
	u16 c2f;    // square coefficient, fixed point
	u16 c3f;    // cubic coefficient, fixed point
};

// word m is derived from sin(m * pi / 32)
constexpr fsca_word FSCA_ROM[17] =
{
	{          0, 0x000000, 0, 0x000 },
	{  6'577'819, 0x4ed47f, 3, 0x103 },
	{ 13'092'290, 0x4e7352, 2, 0x204 },
	{ 19'480'674, 0x74bafb, 2, 0x300 },
	{ 25'681'450, 0x4cf16d, 1, 0x3f5 },
	{ 31'634'899, 0x5ec7aa, 1, 0x4df },
	{ 37'283'686, 0x6fb43b, 1, 0x5be },
	{ 42'573'411, 0x7f8d66, 1, 0x68f },
	{ 47'453'131, 0x47160c, 0, 0x74f },
	{ 51'875'851, 0x4db624, 0, 0x7fd },
	{ 55'798'982, 0x5396a4, 0, 0x898 },
	{ 59'184'735, 0x58a910, 0, 0x91d },
	{ 62'000'504, 0x5ce0e5, 0, 0x98c },
	{ 64'219'180, 0x6033bf, 0, 0x9e4 },
	{ 65'819'382, 0x62996b, 0, 0xa23 },
	{ 66'785'715, 0x640c00, 0, 0xa49 },
	{ 67'108'863, 0x6487ed, 0, 0xa56 }
};

// 17 nodes for an even exponent, then 17 for an odd one
constexpr fsrra_word FSRRA_ROM[34] =
{
	{ 67'108'864, 0x400000, 2, 12296, 320 },
	{ 65'105'160, 0x74dfa6, 1, 10566, 260 },
	{ 63'270'842, 0x6b454d, 1,  9159, 212 },
	{ 61'583'317, 0x62ea16, 1,  8000, 176 },
	{ 60'023'991, 0x5b96df, 1,  7037, 148 },
	{ 58'577'421, 0x552032, 1,  6228, 124 },
	{ 57'230'630, 0x4f6360, 1,  5545, 106 },
	{ 55'972'662, 0x4a4474, 1,  4962,  90 },
	{ 54'794'160, 0x45aca4, 1,  4460,  78 },
	{ 53'687'088, 0x418937, 1,  4029,  68 },
	{ 52'644'524, 0x7b9555, 0,  3651,  58 },
	{ 51'660'427, 0x74c814, 0,  3323,  52 },
	{ 50'729'532, 0x6e94e4, 0,  3035,  46 },
	{ 49'847'212, 0x68e955, 0,  2779,  40 },
	{ 49'009'384, 0x63b5b8, 0,  2553,  36 },
	{ 48'212'432, 0x5eec9f, 0,  2352,  32 },
	{ 47'453'132, 0x5a8279, 0,  2174,  28 },

	{ 47'453'132, 0x5a8279, 1,  8695, 226 },
	{ 46'036'301, 0x52a463, 1,  7471, 183 },
	{ 44'739'243, 0x4bda13, 1,  6476, 151 },
	{ 43'545'981, 0x45f16b, 1,  5658, 124 },
	{ 42'443'371, 0x40c371, 1,  4976, 104 },
	{ 41'420'491, 0x7862d1, 0,  4404,  87 },
	{ 40'468'166, 0x704598, 0,  3921,  74 },
	{ 39'578'650, 0x6907ab, 0,  3508,  63 },
	{ 38'745'321, 0x6288d1, 0,  3154,  55 },
	{ 37'962'507, 0x5cae91, 0,  2847,  47 },
	{ 37'225'301, 0x5762f9, 0,  2581,  41 },
	{ 36'529'438, 0x5293b8, 0,  2350,  36 },
	{ 35'871'196, 0x4e3168, 0,  2146,  32 },
	{ 35'247'303, 0x4a2f01, 0,  1965,  28 },
	{ 34'654'868, 0x468169, 0,  1805,  25 },
	{ 34'091'336, 0x431f22, 0,  1664,  22 },
	{ 33'554'432, 0x400000, 0,  1537,  20 }
};


// the multiplier array: rec is recoded (28 bits signed), mul is the multiplicand (25 bits signed)
inline s64 multiply(s64 rec, s64 mul)
{
	// rows 0-8, the ones that get cut, are made from the low 18 bits of the recoded operand
	const u32 low = u32(rec) & 0x3ffff;
	if (!(mul & 0xffff))
	{
		// A multiplicand like 1.0 has nothing below column 16, so the rows aren't cut anywhere that matters.
		// A negative row still comes to 2^16 less than it should, and the bits the group rules look at
		// are set in the negative rows and no others.
		const u32 negative = (low >> 1) & ~(low & (low << 1)) & 0x15555;
		const int carry = (std::popcount(negative & 0x00015) >= 2) ? 1 : 0;
		const int parity = std::popcount(negative & 0x15000) & 1;
		return (rec * mul) + (s64(carry - parity - std::popcount(negative)) * 0x10000);
	}

	// what each combination of three operand bits puts in a row
	const s64 rows[8] = { 0, mul, mul, mul * 2, ~(mul * 2), ~mul, ~mul, 0 };

	// rows 9 and up have nothing cut off, so they're taken as one ordinary product
	s64 sum = (rec - util::sext(low, 18)) * mul;

	int carry = 0;
	int parity = 0;
	u32 bits = low << 1;
	for (int k = 0; k < 9; k++, bits >>= 2)
	{
		const s64 row = rows[bits & 7] * (s64(1) << (2 * k));
		sum += row & ~s64(0xffff);
		if (k < 3)
			carry += BIT(row, 15);
		else if (k >= 6)
			parity ^= BIT(row, 16);
	}
	if (carry >= 2)
		sum += 0x10000;
	if (parity)
		sum -= 0x10000;

	return sum;
}

// a product register feeding a multiplier
inline s64 latch(s64 rec, s64 mul)
{
	return (multiply(rec, mul) + ROUND) >> 23;
}

// a product going through its aligner: it's cut off, and every negative adder input gets a hot one
inline s64 align(s64 rec, s64 mul, int shift)
{
	const s64 product = multiply(rec, mul) + ROUND;
	return (product >> shift) + ((product < 0) ? 1 : 0);
}

// keeps 24 bits of a value whose top bit is given
inline u32 round_mantissa(u64 value, int &top, bool rz)
{
	const int shift = top - 23;
	if (shift <= 0)
		return value << -shift;

	u32 result = value >> shift;
	if (!rz)
	{
		const u64 rest = value & util::make_bitmask<u64>(shift);
		const u64 half = u64(1) << (shift - 1);
		if ((rest > half) || ((rest == half) && (result & 1)))
		{
			result++;
			if (result >> 24)
			{
				result >>= 1;
				top++;
			}
		}
	}
	return result;
}

} // anonymous namespace


// FVn is the recoded operand
u32 fipr(const u32 *fvm, const u32 *fvn, bool rz, bool dn, u32 &cause)
{
	bool nan = false;
	bool invalid = false;
	int infinities[2] = { 0, 0 };
	int count = 0;
	s64 product[4];
	int exponent[4];
	int greatest = 0;

	for (int i = 0; i < 4; i++)
	{
		const bool negative = BIT(fvm[i] ^ fvn[i], 31);
		u32 fa = fvm[i] & 0x7fffff;
		u32 fb = fvn[i] & 0x7fffff;
		int ea = BIT(fvm[i], 23, 8);
		int eb = BIT(fvn[i], 23, 8);
		const bool za = !ea && (dn || !fa);
		const bool zb = !eb && (dn || !fb);

		if ((ea == 0xff) || (eb == 0xff))
		{
			if (((ea == 0xff) && fa) || ((eb == 0xff) && fb))
			{
				// the signalling ones have the top bit of the mantissa set
				nan = true;
				if (((ea == 0xff) && BIT(fa, 22)) || ((eb == 0xff) && BIT(fb, 22)))
					invalid = true;
			}
			else if (za || zb)
			{
				nan = true;
				invalid = true;
			}
			else
			{
				infinities[negative ? 1 : 0]++;
			}
			continue;
		}
		if (za || zb)
			continue;

		if (ea)
			fa |= 0x800000;
		else
			ea = 1;
		if (eb)
			fb |= 0x800000;
		else
			eb = 1;
		if ((ea + eb) <= 75)
			continue;

		const s64 p = (multiply(fb, fa) + ROUND) >> 20;
		if (!p)
			continue;
		product[count] = negative ? -p : p;
		exponent[count] = ea + eb;
		greatest = std::max(greatest, ea + eb);
		count++;
	}

	if (infinities[0] && infinities[1])
	{
		nan = true;
		invalid = true;
	}
	if (nan)
	{
		cause = invalid ? CAUSE_V : 0;
		return QNAN;
	}
	if ((infinities[0] + infinities[1]) >= 2)
	{
		cause = 0;
		return ((fvm[0] ^ fvn[0]) & SIGN) | INFINITY_BITS;
	}
	if (infinities[0] || infinities[1])
	{
		cause = 0;
		return (infinities[1] ? SIGN : 0) | INFINITY_BITS;
	}

	s64 sum = 0;
	for (int i = 0; i < count; i++)
	{
		// it's the magnitude that gets cut off
		const int shift = greatest - exponent[i];
		if (shift < 40)
			sum += (product[i] < 0) ? -(-product[i] >> shift) : (product[i] >> shift);
	}
	if (!sum)
	{
		cause = CAUSE_I | CAUSE_U;
		return 0;
	}

	const u32 sign = (sum < 0) ? SIGN : 0;
	const u64 magnitude = (sum < 0) ? -sum : sum;
	int top = std::bit_width(magnitude) - 1;
	int e = top + greatest - PRODUCT_SCALE + 127;
	if ((e < 1) && !dn)
	{
		// a denormal is rounded where it ends, and a carry out of that makes the smallest normal number
		top += 1 - e;
		cause = CAUSE_I | CAUSE_U;
		return sign | ((top < 85) ? round_mantissa(magnitude, top, rz) : 0);
	}

	const u32 result = round_mantissa(magnitude, top, rz);
	e = top + greatest - PRODUCT_SCALE + 127;
	if (e < 1)
	{
		cause = CAUSE_I | CAUSE_U;
		return sign;
	}
	if (e >= 255)
	{
		cause = CAUSE_I | CAUSE_O;
		return sign | (rz ? LARGEST_BITS : INFINITY_BITS);
	}
	cause = CAUSE_I;
	return sign | (u32(e) << 23) | (result & 0x7fffff);
}

u32 ftrv(const u32 *xmtrx, u32 *fv, bool rz, bool dn)
{
	u32 result[4];
	u32 cause = 0;
	for (int i = 0; i < 4; i++)
	{
		const u32 row[4] = { xmtrx[i], xmtrx[i + 4], xmtrx[i + 8], xmtrx[i + 12] };
		u32 raised;
		result[i] = fipr(fv, row, rz, dn, raised);
		cause |= raised;
	}
	std::copy_n(result, 4, fv);
	return cause;
}

u32 fsrra(u32 x, bool rz, bool dn, u32 &cause)
{
	const bool negative = BIT(x, 31);
	const u32 exponent = BIT(x, 23, 8);
	const u32 mantissa = x & 0x7fffff;

	if (exponent == 0xff)
	{
		// NaNs don't propagate, and +infinity isn't even inexact
		if (mantissa)
			cause = BIT(mantissa, 22) ? CAUSE_V : 0;
		else
			cause = negative ? CAUSE_V : 0;
		return (mantissa || negative) ? QNAN : 0;
	}
	if (!exponent)
	{
		// -0 gives -infinity rather than being invalid
		if (!mantissa || dn)
		{
			cause = CAUSE_Z;
			return (x & SIGN) | INFINITY_BITS;
		}
		if (negative)
		{
			cause = CAUSE_V;
			return QNAN;
		}
		cause = CAUSE_E;
		return x;
	}
	if (negative)
	{
		cause = CAUSE_V;
		return QNAN;
	}

	// x = 1.m * 2^parity * 4^k
	const int parity = (exponent - 127) & 1;
	const int k = (int(exponent) - 127 - parity) / 2;
	const int node = (mantissa + (1 << 18)) >> 19;
	const fsrra_word &word = FSRRA_ROM[(parity ? 17 : 0) + node];

	const s64 dx = (s64(mantissa) - (s64(node) << 19)) * 16;
	const s64 sq = latch(dx, dx);
	const s64 t1 = align(word.rec1, dx, 26 - word.e1);
	const s64 t2 = align(s64(word.c2f) << 9, sq, 29);
	const s64 t3 = align(dx, latch(s64(word.c3f) << 8, sq), 27);
	const u64 value = s64(word.c0) - t1 + t2 - t3;

	// only an exact power of four gets as far as 2^26
	int top = BIT(value, 26) ? 26 : 25;
	const u32 result = round_mantissa(value, top, rz);
	cause = CAUSE_I;
	return (u32(top - 26 - k + 127) << 23) | (result & 0x7fffff);
}

s32 fsca(u32 angle)
{
	const u32 node = BIT(angle + 512, 10, 6);
	const bool swap = BIT(node, 4);
	const int m = swap ? (16 - (node & 15)) : (node & 15);
	const fsca_word &own = FSCA_ROM[m];
	const fsca_word &other = FSCA_ROM[16 - m];

	// the offset is from the centre of the node
	const s64 dx = (s64((angle + 512) & 1023) - 512) * 8192;
	const s64 t1 = align(other.l, dx, 23 + other.e);
	const s64 t2 = align(own.l, latch(dx, latch(FSCA_KC, dx)), 27 + own.e);
	const s64 t3 = align(s64(other.q) << 5, latch(latch(dx, dx), dx), 26);
	const s64 value = s64(own.c0) + (swap ? (t3 - t1) : (t1 - t3)) - t2;

	return BIT(node, 5) ? -value : value;
}

u32 fsca_round(s32 value, bool rz)
{
	// zero is +0 at both crossings
	if (!value)
		return 0;

	const u32 magnitude = (value < 0) ? -value : value;
	int top = std::bit_width(magnitude) - 1;
	const u32 result = round_mantissa(magnitude, top, rz);
	return ((value < 0) ? SIGN : 0) | (u32(127 - 26 + top) << 23) | (result & 0x7fffff);
}

} // namespace sh4fpu
