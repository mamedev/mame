// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    dsp3210dau.hxx
    The DSP3210's data arithmetic unit, modelled exactly in integers.

    Written from the DSP3210 Information Manual (Sept 1991): 3.4.2 (the
    DSP32 floating-point format), 8.2.1-8.2.3 (accumulators, guard bits,
    Fig. 8-9 precision), 8.2.4 (type conversions), 8.2.5 (rounding modes),
    7.5.3.4 (overflow/underflow) and the ch. 4 instruction pages.  Where
    the manual does not decide, the behaviour follows the exact DAU of the
    dsp3210-sdk (github.com/pappadf/dsp3210-sdk, MIT):

      * the adder truncates when it aligns exponents (the manual provides
        `round` as the only rounding step and never mentions rounding in
        the adder);
      * the product enters the adder at its full 46 fraction bits and
        only the final accumulator write truncates to 31; a literal
        reading of "the adder inputs contain eight guard bits" would cut
        the product to 32 fraction bits before the add.

    Representation.  A memory word is mantissa[31:8] | exponent[7:0]; the
    24-bit mantissa is one 2's-complement quantity s(!s).f...f, so the
    represented mantissa M lies in [1,2) or [-2,-1) and the implicit
    leading bit is !s.  A 40-bit accumulator adds eight guard bits:
    mantissa[39:16] | guard[15:8] | exponent[7:0].  acc_t keeps the
    mantissa+guard *including* the implicit bit as a fixed-point integer
    with 31 fraction bits, so 1.0 is {2^31, 128} and -1.0 normalises to
    {-2^32, 127} (-2 x 2^-1, which is why -1.0 stores as 0x8000007f); an
    exponent of 0 is zero, whatever the mantissa says ("dirty zeros").

    Nothing here touches the device, so the datapath can be compiled and
    tested on its own.

***************************************************************************/

#ifndef MAME_CPU_DSP32_DSP3210DAU_HXX
#define MAME_CPU_DSP32_DSP3210DAU_HXX

#pragma once

#include "dsp3210dau.h"

#include <cmath>
#include <cstdint>

namespace dsp3210dau {

constexpr int ACC_FRAC = 31;                    // fraction bits of an accumulator mantissa
constexpr int PROD_FRAC = 46;                   // fraction bits of a product (23 + 23)
constexpr int64_t ACC_ONE = int64_t(1) << ACC_FRAC;
constexpr uint32_t WORD_ONE = 0x00000080;       // 1.0 as a memory word

inline acc_t zero() { return { 0, 0 }; }

// shift a signed value left through the unsigned type: the datapath
// shifts the two's-complement bit pattern
inline int64_t shl(int64_t v, int k) { return int64_t(uint64_t(v) << k); }

// bring m into [2^frac, 2^(frac+1)) or [-2^(frac+1), -2^frac), adjusting
// e; right shifts are arithmetic, i.e. they truncate.  -2^frac (M = -1)
// is not a normal form: it becomes -2^(frac+1) with e - 1.
inline void normalize(int64_t &m, int &e, int frac)
{
	if (m == 0)
	{
		e = 0;
		return;
	}
	const int64_t lo = int64_t(1) << frac;
	const int64_t hi = lo << 1;
	while (m >= hi || m < -hi)
	{
		m >>= 1;
		e++;
	}
	while ((m > 0 && m < lo) || (m < 0 && m >= -lo))
	{
		m = shl(m, 1);
		e--;
	}
}

// the 24-bit mantissa field of a word as the full 2's-complement value
// with its implicit bit, at 23 fraction bits
inline int64_t mant24(uint32_t w)
{
	const int32_t raw = int32_t(w & 0xffffff00) >> 8;
	return int64_t(raw) + ((raw < 0) ? -(int64_t(1) << 23) : (int64_t(1) << 23));
}

// memory word -> accumulator: guard bits zero, e = 0 is zero
inline acc_t unpack(uint32_t w)
{
	if ((w & 0xff) == 0)
	{
		return zero();
	}
	return { shl(mant24(w), 8), int16_t(w & 0xff) };
}

// accumulator -> memory word: the guard bits are truncated
inline uint32_t pack(acc_t a)
{
	if (a.e == 0 || a.m == 0)
	{
		return 0;
	}
	const int64_t m24 = a.m >> 8;
	const uint32_t raw = uint32_t(m24 - ((m24 < 0) ? -(int64_t(1) << 23) : (int64_t(1) << 23)));
	return ((raw & 0xffffff) << 8) | (uint32_t(a.e) & 0xff);
}

// the stored mantissa+guard field, accumulator bits 39:8 (the implicit
// bit removed); this is the "integer lane" int16/int32/oc deposit into
// and ic/float16/float32 read back
inline uint32_t lane(acc_t a)
{
	if (a.e == 0)
	{
		return 0;
	}
	return uint32_t(a.m - ((a.m < 0) ? -ACC_ONE : ACC_ONE));
}

// deposit a 32-bit integer in the lane; the exponent is architecturally
// unpredictable and is taken from the adder input so that the value
// still reads as a number
inline acc_t from_lane(int32_t v, int16_t e)
{
	return { (v >= 0) ? int64_t(v) + ACC_ONE : int64_t(v) - ACC_ONE, e };
}

// the multiplier: an exact 25 x 25-bit signed product at 46 fraction
// bits.  Its inputs are memory words; an accumulator feeding it has had
// its guard bits truncated by pack() (8.2.3).  The product keeps three
// integer bits, so it may be denormalised (-2 x -2 = 4).
inline void multiply(uint32_t x, uint32_t y, int64_t &pm, int &pe)
{
	const int ex = x & 0xff;
	const int ey = y & 0xff;
	if (ex == 0 || ey == 0)
	{
		pm = 0;
		pe = 0;
		return;
	}
	pm = mant24(x) * mant24(y);
	pe = ex + ey - 128;
}

// the adder: S + P.  S is an accumulator with its guard bits or a
// memory word with zero guard bits; exponents are aligned by arithmetic
// right shift, the sum is normalised and truncated to the accumulator's
// 31 fraction bits.  An S with e <= 0 and a nonzero mantissa is not a
// zero here (a negated 2^-127, say): it underflows in check_range.
inline acc_t accumulate(acc_t s, int64_t pm, int pe)
{
	const int64_t sm = (s.m == 0) ? 0 : shl(s.m, PROD_FRAC - ACC_FRAC);
	int64_t rm;
	int re;

	if (sm == 0 && pm == 0)
	{
		return zero();
	}
	if (sm == 0)
	{
		rm = pm;
		re = pe;
	}
	else if (pm == 0)
	{
		rm = sm;
		re = s.e;
	}
	else if (s.e >= pe)
	{
		const int d = s.e - pe;
		rm = sm + ((d > 62) ? 0 : (pm >> d));
		re = s.e;
	}
	else
	{
		const int d = pe - s.e;
		rm = pm + ((d > 62) ? 0 : (sm >> d));
		re = pe;
	}
	normalize(rm, re, PROD_FRAC);
	if (rm == 0)
	{
		return zero();
	}
	int64_t m = rm >> (PROD_FRAC - ACC_FRAC);
	normalize(m, re, ACC_FRAC);
	return { m, int16_t(re) };
}

inline acc_t negate(acc_t a)
{
	if (a.e == 0)
	{
		return a;
	}
	int64_t m = -a.m;
	int e = a.e;
	normalize(m, e, ACC_FRAC);          // -(-2) = 2 needs e + 1
	return { m, int16_t(e) };
}

// round: 40 -> 32 bits, to nearest with ties to the greater value, the
// guard bits cleared; a zero-exponent operand is a (dirty) zero
inline acc_t round(acc_t a)
{
	if (a.e == 0)
	{
		return zero();
	}
	int64_t m = shl((a.m + 128) >> 8, 8);
	int e = a.e;
	normalize(m, e, ACC_FRAC);
	return { m, int16_t(e) };
}

// exact integer -> float (any 32-bit integer fits the 32 mantissa+guard bits)
inline acc_t from_int(int64_t v)
{
	if (v == 0)
	{
		return zero();
	}
	int64_t m = v;
	int e = 128 + ACC_FRAC;
	normalize(m, e, ACC_FRAC);
	return { m, int16_t(e) };
}

// float -> integer with the dauc[5:4] modes (8.2.5): x0 round to nearest
// with ties up (floor(v + 1/2)), 01 truncate towards -inf (floor), 11
// truncate towards zero; saturating to the range of `bits` (8 = unsigned
// byte, 16 and 32 = signed)
inline int64_t to_int(acc_t a, int mode, int bits)
{
	int64_t lo, hi;
	if (bits == 8)
	{
		lo = 0;
		hi = 255;
	}
	else if (bits == 16)
	{
		lo = -32768;
		hi = 32767;
	}
	else
	{
		lo = INT32_MIN;
		hi = INT32_MAX;
	}

	if (a.e == 0)
	{
		return 0;
	}
	const int shift = a.e - 128 - ACC_FRAC;     // value = m * 2^shift
	int64_t i;
	if (shift >= 0)
	{
		// integral, and beyond 24 places the magnitude is past every range
		if (shift > 24)
		{
			return (a.m < 0) ? lo : hi;
		}
		i = shl(a.m, shift);
	}
	else
	{
		const int sh = -shift;
		if (sh > 63)
		{
			// the whole mantissa shifts out; the mode still decides
			// (floor of a negative is -1)
			i = (mode == 1 && a.m < 0) ? -1 : 0;
		}
		else
		{
			const int64_t frac_mask = int64_t((uint64_t(1) << sh) - 1);
			i = a.m >> sh;                              // floor
			if (!(mode & 1))
			{
				if ((a.m >> (sh - 1)) & 1)              // the most significant fraction bit
				{
					i += 1;
				}
			}
			else if (mode == 3)
			{
				if (a.m < 0 && (a.m & frac_mask))
				{
					i += 1;
				}
			}
		}
	}
	if (i < lo)
	{
		return lo;
	}
	if (i > hi)
	{
		return hi;
	}
	return i;
}

// DSP32 -> IEEE 754 single, truncating the fraction; out-of-range values
// become infinities, below-range values zero (8.2.4.1)
inline uint32_t to_ieee(acc_t a)
{
	if (a.e == 0 || a.m == 0)
	{
		return 0;
	}
	int64_t m = a.m;
	int e = a.e - 128;
	uint32_t sign = 0;
	if (m < 0)
	{
		sign = 0x80000000;
		m = -m;                                         // |M| in (1, 2]
		if (m == (int64_t(1) << 32))
		{
			m >>= 1;
			e += 1;
		}
	}
	const uint32_t frac = uint32_t((m - ACC_ONE) >> 8); // 31 -> 23 fraction bits
	e += 127;
	if (e >= 255)
	{
		return sign | 0x7f800000;
	}
	if (e <= 0)
	{
		return sign;
	}
	return sign | (uint32_t(e) << 23) | (frac & 0x7fffff);
}

// IEEE 754 single -> DSP32 for a normal number (the caller handles the
// exponent-0 and exponent-255 classes), saturating at the DSP32 range
inline acc_t from_ieee(uint32_t w)
{
	const bool sign = (w >> 31) != 0;
	const int exp = (w >> 23) & 0xff;
	if (exp == 0)
	{
		return zero();
	}
	int64_t m = ACC_ONE | (int64_t(w & 0x7fffff) << 8);
	int e = exp - 127 + 128;
	if (sign)
	{
		m = -m;
		normalize(m, e, ACC_FRAC);
	}
	if (e > 255)
	{
		return { sign ? -(int64_t(1) << 32) : (int64_t(1) << 32) - 1, 255 };
	}
	if (e < 1)
	{
		return zero();
	}
	return { m, int16_t(e) };
}

// companded bytes (8.2.4.2).  The mu-law decode is a half-integer, so
// both decoders return twice the value:
//   mu-law  ~m0 ~m1 ~m2 ~m3 ~n0 ~n1 ~n2 ~s     Y = (-1)^s ((16.5 + M) 2^N - 16.5)
//   A-law   ~m0 m1 ~m2 m3 ~n0 n1 ~n2 ~s        Y = (-1)^s (16.5 + M) 2^N   (N >= 1)
//                                              Y = (-1)^s (0.5 + M) 2      (N = 0)
inline int64_t mulaw_decode2(uint8_t b)
{
	const unsigned u = ~b & 0xff;
	const int64_t mm = u & 15;
	const int nn = (u >> 4) & 7;
	const int64_t y = ((33 + 2 * mm) << nn) - 33;
	return (u & 0x80) ? -y : y;
}

inline int64_t alaw_decode2(uint8_t b)
{
	const unsigned u = (b ^ 0xd5) & 0xff;
	const int64_t mm = u & 15;
	const int nn = (u >> 4) & 7;
	const int64_t y = nn ? ((33 + 2 * mm) << nn) : (1 + 2 * mm) * 2;
	return (u & 0x80) ? -y : y;
}

// float -> companded byte: the code whose decoded value is nearest,
// compared on the doubled integers (2Y rounded to nearest first)
inline uint8_t companded_encode(acc_t v, bool alaw)
{
	acc_t t = v;
	if (t.e)
	{
		t.e++;                                          // exactly 2Y
	}
	const int64_t v2 = to_int(t, 0, 32);
	int best = 0;
	int64_t bestd = INT64_MAX;
	for (int b = 0; b < 256; b++)
	{
		int64_t d = (alaw ? alaw_decode2(b) : mulaw_decode2(b)) - v2;
		if (d < 0)
		{
			d = -d;
		}
		if (d < bestd)
		{
			bestd = d;
			best = b;
		}
	}
	return uint8_t(best);
}

// overflow and underflow are judged on the 40-bit value: an exponent
// above 255 saturates the mantissa+guard field, a nonzero mantissa with
// an exponent below 1 flushes to zero (7.5.3.4)
inline acc_t check_range(acc_t r, bool &overflow, bool &underflow)
{
	overflow = false;
	underflow = false;
	if (r.e > 255)
	{
		overflow = true;
		return { (r.m < 0) ? -(int64_t(1) << 32) : (int64_t(1) << 32) - 1, 255 };
	}
	if (r.m != 0 && r.e < 1)
	{
		underflow = true;
		return zero();
	}
	return r;
}

// for the debugger only
inline double to_double(acc_t a)
{
	if (a.e == 0)
	{
		return 0.0;
	}
	return std::ldexp(double(a.m), a.e - 128 - ACC_FRAC);
}

} // namespace dsp3210dau

#endif // MAME_CPU_DSP32_DSP3210DAU_HXX
