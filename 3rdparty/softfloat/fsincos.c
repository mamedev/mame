/*============================================================================
This source file is an extension to the SoftFloat IEC/IEEE Floating-point
Arithmetic Package, Release 2b, written for Bochs (x86 achitecture simulator)
floating point emulation.

THIS SOFTWARE IS DISTRIBUTED AS IS, FOR FREE.  Although reasonable effort has
been made to avoid it, THIS SOFTWARE MAY CONTAIN FAULTS THAT WILL AT TIMES
RESULT IN INCORRECT BEHAVIOR.  USE OF THIS SOFTWARE IS RESTRICTED TO PERSONS
AND ORGANIZATIONS WHO CAN AND WILL TAKE FULL RESPONSIBILITY FOR ALL LOSSES,
COSTS, OR OTHER PROBLEMS THEY INCUR DUE TO THE SOFTWARE, AND WHO FURTHERMORE
EFFECTIVELY INDEMNIFY JOHN HAUSER AND THE INTERNATIONAL COMPUTER SCIENCE
INSTITUTE (possibly via similar legal warning) AGAINST ALL LOSSES, COSTS, OR
OTHER PROBLEMS INCURRED BY THEIR CUSTOMERS AND CLIENTS DUE TO THE SOFTWARE.

Derivative works are acceptable, even for commercial purposes, so long as
(1) the source code for the derivative work includes prominent notice that
the work is derivative, and (2) the source code includes prominent notice with
these four paragraphs for those parts of this code that are retained.
=============================================================================*/

/*============================================================================
 * Written for Bochs (x86 achitecture simulator) by
 *            Stanislav Shwartsman [sshwarts at sourceforge net]
 * ==========================================================================*/

#define FLOAT128

#define USE_estimateDiv128To64
#include "mamesf.h"
#include "softfloat.h"
//#include "softfloat-specialize"
#include "fpu_constant.h"

static const floatx80 floatx80_one = packFloatx80(0, 0x3fff, U64(0x8000000000000000));
static const floatx80 floatx80_default_nan = packFloatx80(0, 0xffff, U64(0xffffffffffffffff));

#define packFloat2x128m(zHi, zLo) {(zHi), (zLo)}
#define PACK_FLOAT_128(hi,lo) packFloat2x128m(LIT64(hi),LIT64(lo))

#define EXP_BIAS 0x3FFF

/*----------------------------------------------------------------------------
| Returns the fraction bits of the extended double-precision floating-point
| value `a'.
*----------------------------------------------------------------------------*/

INLINE bits64 extractFloatx80Frac( floatx80 a )
{
	return a.low;

}

/*----------------------------------------------------------------------------
| Returns the exponent bits of the extended double-precision floating-point
| value `a'.
*----------------------------------------------------------------------------*/

INLINE int32 extractFloatx80Exp( floatx80 a )
{
	return a.high & 0x7FFF;

}

/*----------------------------------------------------------------------------
| Returns the sign bit of the extended double-precision floating-point value
| `a'.
*----------------------------------------------------------------------------*/

INLINE flag extractFloatx80Sign( floatx80 a )
{
	return a.high>>15;

}

/*----------------------------------------------------------------------------
| Takes extended double-precision floating-point  NaN  `a' and returns the
| appropriate NaN result. If `a' is a signaling NaN, the invalid exception
| is raised.
*----------------------------------------------------------------------------*/

INLINE floatx80 propagateFloatx80NaNOneArg(floatx80 a)
{
	if (floatx80_is_signaling_nan(a))
		float_raise(float_flag_invalid);

	a.low |= U64(0xC000000000000000);

	return a;
}

/*----------------------------------------------------------------------------
| Normalizes the subnormal extended double-precision floating-point value
| represented by the denormalized significand `aSig'.  The normalized exponent
| and significand are stored at the locations pointed to by `zExpPtr' and
| `zSigPtr', respectively.
*----------------------------------------------------------------------------*/

void normalizeFloatx80Subnormal(UINT64 aSig, INT32 *zExpPtr, UINT64 *zSigPtr)
{
	int shiftCount = countLeadingZeros64(aSig);
	*zSigPtr = aSig<<shiftCount;
	*zExpPtr = 1 - shiftCount;
}

/* reduce trigonometric function argument using 128-bit precision
   M_PI approximation */
static UINT64 argument_reduction_kernel(UINT64 aSig0, int Exp, UINT64 *zSig0, UINT64 *zSig1)
{
	UINT64 term0, term1, term2;
	UINT64 aSig1 = 0;

	shortShift128Left(aSig1, aSig0, Exp, &aSig1, &aSig0);
	UINT64 q = estimateDiv128To64(aSig1, aSig0, FLOAT_PI_HI);
	mul128By64To192(FLOAT_PI_HI, FLOAT_PI_LO, q, &term0, &term1, &term2);
	sub128(aSig1, aSig0, term0, term1, zSig1, zSig0);
	while ((INT64)(*zSig1) < 0) {
		--q;
		add192(*zSig1, *zSig0, term2, 0, FLOAT_PI_HI, FLOAT_PI_LO, zSig1, zSig0, &term2);
	}
	*zSig1 = term2;
	return q;
}

static int reduce_trig_arg(int expDiff, int &zSign, UINT64 &aSig0, UINT64 &aSig1)
{
	UINT64 term0, term1, q = 0;

	if (expDiff < 0) {
		shift128Right(aSig0, 0, 1, &aSig0, &aSig1);
		expDiff = 0;
	}
	if (expDiff > 0) {
		q = argument_reduction_kernel(aSig0, expDiff, &aSig0, &aSig1);
	}
	else {
		if (FLOAT_PI_HI <= aSig0) {
			aSig0 -= FLOAT_PI_HI;
			q = 1;
		}
	}

	shift128Right(FLOAT_PI_HI, FLOAT_PI_LO, 1, &term0, &term1);
	if (! lt128(aSig0, aSig1, term0, term1))
	{
		int lt = lt128(term0, term1, aSig0, aSig1);
		int eq = eq128(aSig0, aSig1, term0, term1);

		if ((eq && (q & 1)) || lt) {
			zSign = !zSign;
			++q;
		}
		if (lt) sub128(FLOAT_PI_HI, FLOAT_PI_LO, aSig0, aSig1, &aSig0, &aSig1);
	}

	return (int)(q & 3);
}

#define SIN_ARR_SIZE 11
#define COS_ARR_SIZE 11

static float128 sin_arr[SIN_ARR_SIZE] =
{
	PACK_FLOAT_128(0x3fff000000000000, 0x0000000000000000), /*  1 */
	PACK_FLOAT_128(0xbffc555555555555, 0x5555555555555555), /*  3 */
	PACK_FLOAT_128(0x3ff8111111111111, 0x1111111111111111), /*  5 */
	PACK_FLOAT_128(0xbff2a01a01a01a01, 0xa01a01a01a01a01a), /*  7 */
	PACK_FLOAT_128(0x3fec71de3a556c73, 0x38faac1c88e50017), /*  9 */
	PACK_FLOAT_128(0xbfe5ae64567f544e, 0x38fe747e4b837dc7), /* 11 */
	PACK_FLOAT_128(0x3fde6124613a86d0, 0x97ca38331d23af68), /* 13 */
	PACK_FLOAT_128(0xbfd6ae7f3e733b81, 0xf11d8656b0ee8cb0), /* 15 */
	PACK_FLOAT_128(0x3fce952c77030ad4, 0xa6b2605197771b00), /* 17 */
	PACK_FLOAT_128(0xbfc62f49b4681415, 0x724ca1ec3b7b9675), /* 19 */
	PACK_FLOAT_128(0x3fbd71b8ef6dcf57, 0x18bef146fcee6e45)  /* 21 */
};

static float128 cos_arr[COS_ARR_SIZE] =
{
	PACK_FLOAT_128(0x3fff000000000000, 0x0000000000000000), /*  0 */
	PACK_FLOAT_128(0xbffe000000000000, 0x0000000000000000), /*  2 */
	PACK_FLOAT_128(0x3ffa555555555555, 0x5555555555555555), /*  4 */
	PACK_FLOAT_128(0xbff56c16c16c16c1, 0x6c16c16c16c16c17), /*  6 */
	PACK_FLOAT_128(0x3fefa01a01a01a01, 0xa01a01a01a01a01a), /*  8 */
	PACK_FLOAT_128(0xbfe927e4fb7789f5, 0xc72ef016d3ea6679), /* 10 */
	PACK_FLOAT_128(0x3fe21eed8eff8d89, 0x7b544da987acfe85), /* 12 */
	PACK_FLOAT_128(0xbfda93974a8c07c9, 0xd20badf145dfa3e5), /* 14 */
	PACK_FLOAT_128(0x3fd2ae7f3e733b81, 0xf11d8656b0ee8cb0), /* 16 */
	PACK_FLOAT_128(0xbfca6827863b97d9, 0x77bb004886a2c2ab), /* 18 */
	PACK_FLOAT_128(0x3fc1e542ba402022, 0x507a9cad2bf8f0bb)  /* 20 */
};

extern float128 OddPoly (float128 x, float128 *arr, unsigned n);

/* 0 <= x <= pi/4 */
INLINE float128 poly_sin(float128 x)
{
	//                 3     5     7     9     11     13     15
	//                x     x     x     x     x      x      x
	// sin (x) ~ x - --- + --- - --- + --- - ---- + ---- - ---- =
	//                3!    5!    7!    9!    11!    13!    15!
	//
	//                 2     4     6     8     10     12     14
	//                x     x     x     x     x      x      x
	//   = x * [ 1 - --- + --- - --- + --- - ---- + ---- - ---- ] =
	//                3!    5!    7!    9!    11!    13!    15!
	//
	//           3                          3
	//          --       4k                --        4k+2
	//   p(x) = >  C  * x   > 0     q(x) = >  C   * x     < 0
	//          --  2k                     --  2k+1
	//          k=0                        k=0
	//
	//                          2
	//   sin(x) ~ x * [ p(x) + x * q(x) ]
	//

	return OddPoly(x, sin_arr, SIN_ARR_SIZE);
}

extern float128 EvenPoly(float128 x, float128 *arr, unsigned n);

/* 0 <= x <= pi/4 */
INLINE float128 poly_cos(float128 x)
{
	//                 2     4     6     8     10     12     14
	//                x     x     x     x     x      x      x
	// cos (x) ~ 1 - --- + --- - --- + --- - ---- + ---- - ----
	//                2!    4!    6!    8!    10!    12!    14!
	//
	//           3                          3
	//          --       4k                --        4k+2
	//   p(x) = >  C  * x   > 0     q(x) = >  C   * x     < 0
	//          --  2k                     --  2k+1
	//          k=0                        k=0
	//
	//                      2
	//   cos(x) ~ [ p(x) + x * q(x) ]
	//

	return EvenPoly(x, cos_arr, COS_ARR_SIZE);
}

INLINE void sincos_invalid(floatx80 *sin_a, floatx80 *cos_a, floatx80 a)
{
	if (sin_a) *sin_a = a;
	if (cos_a) *cos_a = a;
}

INLINE void sincos_tiny_argument(floatx80 *sin_a, floatx80 *cos_a, floatx80 a)
{
	if (sin_a) *sin_a = a;
	if (cos_a) *cos_a = floatx80_one;
}

static floatx80 sincos_approximation(int neg, float128 r, UINT64 quotient)
{
	if (quotient & 0x1) {
		r = poly_cos(r);
		neg = 0;
	} else  {
		r = poly_sin(r);
	}

	floatx80 result = float128_to_floatx80(r);
	if (quotient & 0x2)
		neg = ! neg;

	if (neg)
		result = floatx80_chs(result);

	return result;
}

// =================================================
// SFFSINCOS               Compute sin(x) and cos(x)
// =================================================

//
// Uses the following identities:
// ----------------------------------------------------------
//
//  sin(-x) = -sin(x)
//  cos(-x) =  cos(x)
//
//  sin(x+y) = sin(x)*cos(y)+cos(x)*sin(y)
//  cos(x+y) = sin(x)*sin(y)+cos(x)*cos(y)
//
//  sin(x+ pi/2)  =  cos(x)
//  sin(x+ pi)    = -sin(x)
//  sin(x+3pi/2)  = -cos(x)
//  sin(x+2pi)    =  sin(x)
//

int sf_fsincos(floatx80 a, floatx80 *sin_a, floatx80 *cos_a)
{
	UINT64 aSig0, aSig1 = 0;
	INT32 aExp, zExp, expDiff;
	int aSign, zSign;
	int q = 0;

	aSig0 = extractFloatx80Frac(a);
	aExp = extractFloatx80Exp(a);
	aSign = extractFloatx80Sign(a);

	/* invalid argument */
	if (aExp == 0x7FFF) {
		if ((UINT64) (aSig0<<1)) {
			sincos_invalid(sin_a, cos_a, propagateFloatx80NaNOneArg(a));
			return 0;
		}

		float_raise(float_flag_invalid);
		sincos_invalid(sin_a, cos_a, floatx80_default_nan);
		return 0;
	}

	if (aExp == 0) {
		if (aSig0 == 0) {
			sincos_tiny_argument(sin_a, cos_a, a);
			return 0;
		}

//        float_raise(float_flag_denormal);

		/* handle pseudo denormals */
		if (! (aSig0 & U64(0x8000000000000000)))
		{
			float_raise(float_flag_inexact);
			if (sin_a)
				float_raise(float_flag_underflow);
			sincos_tiny_argument(sin_a, cos_a, a);
			return 0;
		}

		normalizeFloatx80Subnormal(aSig0, &aExp, &aSig0);
	}

	zSign = aSign;
	zExp = EXP_BIAS;
	expDiff = aExp - zExp;

	/* argument is out-of-range */
	if (expDiff >= 63)
		return -1;

	float_raise(float_flag_inexact);

	if (expDiff < -1) {    // doesn't require reduction
		if (expDiff <= -68) {
			a = packFloatx80(aSign, aExp, aSig0);
			sincos_tiny_argument(sin_a, cos_a, a);
			return 0;
		}
		zExp = aExp;
	}
	else {
		q = reduce_trig_arg(expDiff, zSign, aSig0, aSig1);
	}

	/* **************************** */
	/* argument reduction completed */
	/* **************************** */

	/* using float128 for approximation */
	float128 r = normalizeRoundAndPackFloat128(0, zExp-0x10, aSig0, aSig1);

	if (aSign) q = -q;
	if (sin_a) *sin_a = sincos_approximation(zSign, r,   q);
	if (cos_a) *cos_a = sincos_approximation(zSign, r, q+1);

	return 0;
}

int floatx80_fsin(floatx80 &a)
{
	return sf_fsincos(a, &a, 0);
}

int floatx80_fcos(floatx80 &a)
{
	return sf_fsincos(a, 0, &a);
}

// =================================================
// FPTAN                 Compute tan(x)
// =================================================

//
// Uses the following identities:
//
// 1. ----------------------------------------------------------
//
//  sin(-x) = -sin(x)
//  cos(-x) =  cos(x)
//
//  sin(x+y) = sin(x)*cos(y)+cos(x)*sin(y)
//  cos(x+y) = sin(x)*sin(y)+cos(x)*cos(y)
//
//  sin(x+ pi/2)  =  cos(x)
//  sin(x+ pi)    = -sin(x)
//  sin(x+3pi/2)  = -cos(x)
//  sin(x+2pi)    =  sin(x)
//
// 2. ----------------------------------------------------------
//
//           sin(x)
//  tan(x) = ------
//           cos(x)
//

int floatx80_ftan(floatx80 &a)
{
	UINT64 aSig0, aSig1 = 0;
	INT32 aExp, zExp, expDiff;
	int aSign, zSign;
	int q = 0;

	aSig0 = extractFloatx80Frac(a);
	aExp = extractFloatx80Exp(a);
	aSign = extractFloatx80Sign(a);

	/* invalid argument */
	if (aExp == 0x7FFF) {
		if ((UINT64) (aSig0<<1))
		{
			a = propagateFloatx80NaNOneArg(a);
			return 0;
		}

		float_raise(float_flag_invalid);
		a = floatx80_default_nan;
		return 0;
	}

	if (aExp == 0) {
		if (aSig0 == 0) return 0;
//        float_raise(float_flag_denormal);
		/* handle pseudo denormals */
		if (! (aSig0 & U64(0x8000000000000000)))
		{
			float_raise(float_flag_inexact | float_flag_underflow);
			return 0;
		}
		normalizeFloatx80Subnormal(aSig0, &aExp, &aSig0);
	}

	zSign = aSign;
	zExp = EXP_BIAS;
	expDiff = aExp - zExp;

	/* argument is out-of-range */
	if (expDiff >= 63)
		return -1;

	float_raise(float_flag_inexact);

	if (expDiff < -1) {    // doesn't require reduction
		if (expDiff <= -68) {
			a = packFloatx80(aSign, aExp, aSig0);
			return 0;
		}
		zExp = aExp;
	}
	else {
		q = reduce_trig_arg(expDiff, zSign, aSig0, aSig1);
	}

	/* **************************** */
	/* argument reduction completed */
	/* **************************** */

	/* using float128 for approximation */
	float128 r = normalizeRoundAndPackFloat128(0, zExp-0x10, aSig0, aSig1);

	float128 sin_r = poly_sin(r);
	float128 cos_r = poly_cos(r);

	if (q & 0x1) {
		r = float128_div(cos_r, sin_r);
		zSign = ! zSign;
	} else {
		r = float128_div(sin_r, cos_r);
	}

	a = float128_to_floatx80(r);
	if (zSign)
		a = floatx80_chs(a);

	return 0;
}

//                            2         3         4               n
// f(x) ~ C + (C * x) + (C * x) + (C * x) + (C * x) + ... + (C * x)
//         0    1         2         3         4               n
//
//          --       2k                --        2k+1
//   p(x) = >  C  * x           q(x) = >  C   * x
//          --  2k                     --  2k+1
//
//   f(x) ~ [ p(x) + x * q(x) ]
//

float128 EvalPoly(float128 x, float128 *arr, unsigned n)
{
	float128 x2 = float128_mul(x, x);
	unsigned i;

	assert(n > 1);

	float128 r1 = arr[--n];
	i = n;
	while(i >= 2) {
		r1 = float128_mul(r1, x2);
		i -= 2;
		r1 = float128_add(r1, arr[i]);
	}
	if (i) r1 = float128_mul(r1, x);

	float128 r2 = arr[--n];
	i = n;
	while(i >= 2) {
		r2 = float128_mul(r2, x2);
		i -= 2;
		r2 = float128_add(r2, arr[i]);
	}
	if (i) r2 = float128_mul(r2, x);

	return float128_add(r1, r2);
}

//                  2         4         6         8               2n
// f(x) ~ C + (C * x) + (C * x) + (C * x) + (C * x) + ... + (C * x)
//         0    1         2         3         4               n
//
//          --       4k                --        4k+2
//   p(x) = >  C  * x           q(x) = >  C   * x
//          --  2k                     --  2k+1
//
//                    2
//   f(x) ~ [ p(x) + x * q(x) ]
//

float128 EvenPoly(float128 x, float128 *arr, unsigned n)
{
		return EvalPoly(float128_mul(x, x), arr, n);
}

//                        3         5         7         9               2n+1
// f(x) ~ (C * x) + (C * x) + (C * x) + (C * x) + (C * x) + ... + (C * x)
//          0         1         2         3         4               n
//                        2         4         6         8               2n
//      = x * [ C + (C * x) + (C * x) + (C * x) + (C * x) + ... + (C * x)
//               0    1         2         3         4               n
//
//          --       4k                --        4k+2
//   p(x) = >  C  * x           q(x) = >  C   * x
//          --  2k                     --  2k+1
//
//                        2
//   f(x) ~ x * [ p(x) + x * q(x) ]
//

float128 OddPoly(float128 x, float128 *arr, unsigned n)
{
		return float128_mul(x, EvenPoly(x, arr, n));
}

/*----------------------------------------------------------------------------
| Scales extended double-precision floating-point value in operand `a' by
| value `b'. The function truncates the value in the second operand 'b' to
| an integral value and adds that value to the exponent of the operand 'a'.
| The operation performed according to the IEC/IEEE Standard for Binary
| Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

extern floatx80 propagateFloatx80NaN( floatx80 a, floatx80 b );

floatx80 floatx80_scale(floatx80 a, floatx80 b)
{
	sbits32 aExp, bExp;
	bits64 aSig, bSig;

	// handle unsupported extended double-precision floating encodings
/*    if (floatx80_is_unsupported(a) || floatx80_is_unsupported(b))
    {
        float_raise(float_flag_invalid);
        return floatx80_default_nan;
    }*/

	aSig = extractFloatx80Frac(a);
	aExp = extractFloatx80Exp(a);
	int aSign = extractFloatx80Sign(a);
	bSig = extractFloatx80Frac(b);
	bExp = extractFloatx80Exp(b);
	int bSign = extractFloatx80Sign(b);

	if (aExp == 0x7FFF) {
		if ((bits64) (aSig<<1) || ((bExp == 0x7FFF) && (bits64) (bSig<<1)))
		{
			return propagateFloatx80NaN(a, b);
		}
		if ((bExp == 0x7FFF) && bSign) {
			float_raise(float_flag_invalid);
			return floatx80_default_nan;
		}
		if (bSig && (bExp == 0)) float_raise(float_flag_denormal);
		return a;
	}
	if (bExp == 0x7FFF) {
		if ((bits64) (bSig<<1)) return propagateFloatx80NaN(a, b);
		if ((aExp | aSig) == 0) {
			if (! bSign) {
				float_raise(float_flag_invalid);
				return floatx80_default_nan;
			}
			return a;
		}
		if (aSig && (aExp == 0)) float_raise(float_flag_denormal);
		if (bSign) return packFloatx80(aSign, 0, 0);
		return packFloatx80(aSign, 0x7FFF, U64(0x8000000000000000));
	}
	if (aExp == 0) {
		if (aSig == 0) return a;
		float_raise(float_flag_denormal);
		normalizeFloatx80Subnormal(aSig, &aExp, &aSig);
	}
	if (bExp == 0) {
		if (bSig == 0) return a;
		float_raise(float_flag_denormal);
		normalizeFloatx80Subnormal(bSig, &bExp, &bSig);
	}

	if (bExp > 0x400E) {
		/* generate appropriate overflow/underflow */
		return roundAndPackFloatx80(80, aSign,
							bSign ? -0x3FFF : 0x7FFF, aSig, 0);
	}
	if (bExp < 0x3FFF) return a;

	int shiftCount = 0x403E - bExp;
	bSig >>= shiftCount;
	sbits32 scale = bSig;
	if (bSign) scale = -scale; /* -32768..32767 */
	return
		roundAndPackFloatx80(80, aSign, aExp+scale, aSig, 0);
}
