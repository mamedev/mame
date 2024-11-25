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

#include "../build/MAME/platform.h"

#include "../source/include/softfloat.h"
#include "../source/include/internals.h"
#include "softfloat-extra.h"
#include "softfloat-helpers.h"
#include "softfloat-specialize.h"

static const extFloat80_t floatx80_posone = packFloatx80(0, 0x3fff, uint64_t(0x8000000000000000));
static const extFloat80_t floatx80_negone = packFloatx80(1, 0x3fff, uint64_t(0x8000000000000000));
static const extFloat80_t floatx80_neghalf = packFloatx80(1, 0x3ffe, uint64_t(0x8000000000000000));
static const float128_t float128_e =
	packFloat128(uint64_t(0x40005BF0A8B14576), uint64_t(0x95355FB8AC404E7A));
static const float128_t float128_ln2   =
	packFloat128(uint64_t(0x3ffe62e42fefa39e), uint64_t(0xf35793c7673007e6));
static const float128_t float128_ln10   =
	packFloat128(uint64_t(0x400026bb1bbb5551), uint64_t(0x582dd4adac5705a6));

#ifdef BETTER_THAN_PENTIUM

#define LN2_SIG_HI uint64_t(0xb17217f7d1cf79ab)
#define LN2_SIG_LO uint64_t(0xc9e3b39800000000)  /* 96 bit precision */

#define LN10_SIG_HI uint64_t(0xae585ba95bb0a2aa)
#define LN10_SIG_LO uint64_t(0x763776d400000000) /* 96 bit precision */

#else

#define LN2_SIG_HI uint64_t(0xb17217f7d1cf79ab)
#define LN2_SIG_LO uint64_t(0xc000000000000000)  /* 67-bit precision */

#define LN10_SIG_HI uint64_t(0xae585ba95bb0a2aa)
#define LN10_SIG_LO uint64_t(0x7000000000000000) /* 67-bit precision */

#endif

#define EXP_ARR_SIZE 15

static float128_t exp_arr[EXP_ARR_SIZE] =
{
	PACK_FLOAT_128(0x3fff000000000000, 0x0000000000000000), /*  1 */
	PACK_FLOAT_128(0x3ffe000000000000, 0x0000000000000000), /*  2 */
	PACK_FLOAT_128(0x3ffc555555555555, 0x5555555555555555), /*  3 */
	PACK_FLOAT_128(0x3ffa555555555555, 0x5555555555555555), /*  4 */
	PACK_FLOAT_128(0x3ff8111111111111, 0x1111111111111111), /*  5 */
	PACK_FLOAT_128(0x3ff56c16c16c16c1, 0x6c16c16c16c16c17), /*  6 */
	PACK_FLOAT_128(0x3ff2a01a01a01a01, 0xa01a01a01a01a01a), /*  7 */
	PACK_FLOAT_128(0x3fefa01a01a01a01, 0xa01a01a01a01a01a), /*  8 */
	PACK_FLOAT_128(0x3fec71de3a556c73, 0x38faac1c88e50017), /*  9 */
	PACK_FLOAT_128(0x3fe927e4fb7789f5, 0xc72ef016d3ea6679), /* 10 */
	PACK_FLOAT_128(0x3fe5ae64567f544e, 0x38fe747e4b837dc7), /* 11 */
	PACK_FLOAT_128(0x3fe21eed8eff8d89, 0x7b544da987acfe85), /* 12 */
	PACK_FLOAT_128(0x3fde6124613a86d0, 0x97ca38331d23af68), /* 13 */
	PACK_FLOAT_128(0x3fda93974a8c07c9, 0xd20badf145dfa3e5), /* 14 */
	PACK_FLOAT_128(0x3fd6ae7f3e733b81, 0xf11d8656b0ee8cb0)  /* 15 */
};

extern float128_t EvalPoly(float128_t x, const float128_t *arr, int n);

/* required -1 < x < 1 */
static float128_t poly_exp(float128_t x)
{
/*
                  2     3     4     5     6     7     8     9
     x           x     x     x     x     x     x     x     x
    e - 1 ~ x + --- + --- + --- + --- + --- + --- + --- + --- + ...
                 2!    3!    4!    5!    6!    7!    8!    9!
   
                        2     3     4     5     6     7     8
                 x     x     x     x     x     x     x     x
      = x [ 1 + --- + --- + --- + --- + --- + --- + --- + --- + ... ]
                 2!    3!    4!    5!    6!    7!    8!    9!
   
              8                          8
             --       2k                --        2k+1
      p(x) = >  C  * x           q(x) = >  C   * x
             --  2k                     --  2k+1
             k=0                        k=0
   
       x
      e  - 1 ~ x * [ p(x) + x * q(x) ]
   
*/
	float128_t t = EvalPoly(x, (const float128_t*) exp_arr, EXP_ARR_SIZE);
	return f128_mul(t, x);
}

// =================================================
//                                  x
// FX2M1                   Compute 2  - 1
// =================================================

//
// Uses the following identities:
//
// 1. ----------------------------------------------------------
//      x    x*ln(2)
//     2  = e
//
// 2. ----------------------------------------------------------
//                      2     3     4     5           n
//      x        x     x     x     x     x           x
//     e  = 1 + --- + --- + --- + --- + --- + ... + --- + ...
//               1!    2!    3!    4!    5!          n!
//
extFloat80_t extFloat80_2xm1(extFloat80_t a)
{
	uint64_t zSig0, zSig1, zSig2;
	struct exp32_sig64 normExpSig;

	// handle unsupported extended double-precision floating encodings
	if (extF80_isUnsupported(a)) {
		softfloat_exceptionFlags |= softfloat_flag_invalid;
		return floatx80_default_nan;
	}

	uint64_t aSig = extF80_fraction(a);
	int32_t aExp = extF80_exp(a);
	int aSign = extF80_sign(a);

	if (aExp == 0x7FFF) {
		if (aSig << 1) {
			const uint128 nan = softfloat_propagateNaNExtF80UI(a.signExp, aSig, 0, 0);
			extFloat80_t rv;
			rv.signExp = nan.v64;
			rv.signif = nan.v0;
			return rv;
		}

		return (aSign) ? floatx80_negone : a;
	}

	if (! aExp) {
		if (! aSig) return a;
		softfloat_exceptionFlags |= softfloat_flag_inexact; // denormal also
		normExpSig = softfloat_normSubnormalExtF80Sig(aSig);
		aExp = normExpSig.exp + 1;
		aSig = normExpSig.sig;

	tiny_argument:
		mul128By64To192(LN2_SIG_HI, LN2_SIG_LO, aSig, &zSig0, &zSig1, &zSig2);
		if (0 < (int64_t) zSig0) {
			shortShift128Left(zSig0, zSig1, 1, &zSig0, &zSig1);
			--aExp;
		}
		return softfloat_roundPackToExtF80(aSign, aExp, zSig0, zSig1, 80);
	}

	softfloat_exceptionFlags |= softfloat_flag_inexact;

	if (aExp < 0x3FFF)
	{
		if (aExp < FLOATX80_EXP_BIAS-68)
			goto tiny_argument;

		/* ******************************** */
		/* using float128 for approximation */
		/* ******************************** */

		float128_t x = extF80_to_f128(a);
		x = f128_mul(x, float128_ln2);
		x = poly_exp(x);
		return f128_to_extF80(x);
	}
	else
	{
		if (a.signExp == 0xBFFF && ! (aSig<<1))
		   return floatx80_neghalf;

		return a;
	}
}

// =================================================
//                                  x
//                         Compute e
// =================================================

//
// Uses the following identities:
//
// 1. ----------------------------------------------------------
//                      2     3     4     5           n
//      x        x     x     x     x     x           x
//     e  = 1 + --- + --- + --- + --- + --- + ... + --- + ...
//               1!    2!    3!    4!    5!          n!
//
extFloat80_t extFloat80_etox(extFloat80_t a)
{
	uint64_t zSig0, zSig1, zSig2;
	struct exp32_sig64 normExpSig;

	// handle unsupported extended double-precision floating encodings
	if (extF80_isUnsupported(a)) {
		softfloat_exceptionFlags |= softfloat_flag_invalid;
		return floatx80_default_nan;
	}

	uint64_t aSig = extF80_fraction(a);
	int32_t aExp = extF80_exp(a);
	int aSign = extF80_sign(a);

	if (aExp == 0x7FFF) {
		if (aSig << 1) {
			const uint128 nan = softfloat_propagateNaNExtF80UI(a.signExp, aSig, 0, 0);
			extFloat80_t rv;
			rv.signExp = nan.v64;
			rv.signif = nan.v0;
			return rv;
		}

		return (aSign) ? floatx80_negone : a;
	}

	if (! aExp) {
		if (! aSig) return a;
		softfloat_exceptionFlags |= softfloat_flag_inexact; // denormal also
		normExpSig = softfloat_normSubnormalExtF80Sig(aSig);
		aExp = normExpSig.exp + 1;
		aSig = normExpSig.sig;

		mul128By64To192(0x1000000000000000, 0, aSig, &zSig0, &zSig1, &zSig2);
		if (0 < (int64_t) zSig0) {
			shortShift128Left(zSig0, zSig1, 1, &zSig0, &zSig1);
			--aExp;
		}
		return softfloat_roundPackToExtF80(aSign, aExp, zSig0, zSig1, 80);
	}

	softfloat_exceptionFlags |= softfloat_flag_inexact;

	const extFloat80_t base = f128_to_extF80(float128_e);
	extFloat80_t val = floatx80_posone;
	float128_t x;

	while (!(extF80_lt(a, floatx80_posone))) {
		val = extF80_mul(val, base);

		if (extF80_exp(val) == 0x7FFF) {  // inf
			goto ret;
		}

		a = extF80_sub(a, floatx80_posone);
	}

	while (extF80_le(a, floatx80_negone)) {
		val = extF80_div(val, base);

		if (!extF80_exp(val)) {  // subnormal
			goto ret;
		}

		a = extF80_add(a, floatx80_posone);
	}

ret:
	/* ******************************** */
	/* using float128 for approximation */
	/* ******************************** */
	x = extF80_to_f128(a);
	x = poly_exp(x);

	val = extF80_mul(val, extF80_add(f128_to_extF80(x), floatx80_posone));

	return val;
}

// =================================================
//                                  x
//                         Compute 2
// =================================================

//
// Uses the following identities:
//
// 1. ----------------------------------------------------------
//      x    x*ln(2)
//     2  = e
//
// 2. ----------------------------------------------------------
//                      2     3     4     5           n
//      x        x     x     x     x     x           x
//     e  = 1 + --- + --- + --- + --- + --- + ... + --- + ...
//               1!    2!    3!    4!    5!          n!
//
extFloat80_t extFloat80_2tox(extFloat80_t a)
{
	uint64_t zSig0, zSig1, zSig2;
	struct exp32_sig64 normExpSig;

	// handle unsupported extended double-precision floating encodings
	if (extF80_isUnsupported(a)) {
		softfloat_exceptionFlags |= softfloat_flag_invalid;
		return floatx80_default_nan;
	}

	uint64_t aSig = extF80_fraction(a);
	int32_t aExp = extF80_exp(a);
	int aSign = extF80_sign(a);

	if (aExp == 0x7FFF) {  // inf or NaN
		if (aSig << 1) {  // NaN
			const uint128 nan = softfloat_propagateNaNExtF80UI(a.signExp, aSig, 0, 0);
			extFloat80_t rv;
			rv.signExp = nan.v64;
			rv.signif = nan.v0;
			return rv;
		}

		return (aSign) ? floatx80_negone : a;  // -inf or +inf
	}

	if (! aExp) {  // subnormal
		if (! aSig) return a;
		softfloat_exceptionFlags |= softfloat_flag_inexact; // denormal also
		normExpSig = softfloat_normSubnormalExtF80Sig(aSig);
		aExp = normExpSig.exp + 1;
		aSig = normExpSig.sig;

		mul128By64To192(LN2_SIG_HI, LN2_SIG_LO, aSig, &zSig0, &zSig1, &zSig2);
		if (0 < (int64_t) zSig0) {
			shortShift128Left(zSig0, zSig1, 1, &zSig0, &zSig1);
			--aExp;
		}
		return softfloat_roundPackToExtF80(aSign, aExp, zSig0, zSig1, 80);
	}

	softfloat_exceptionFlags |= softfloat_flag_inexact;

	const extFloat80_t base = i32_to_extF80(2);
	extFloat80_t val = floatx80_posone;
	float128_t x;

	while (!(extF80_lt(a, floatx80_posone))) {
		val = extF80_mul(val, base);

		if (extF80_exp(val) == 0x7FFF) {  // inf
			goto ret;
		}

		a = extF80_sub(a, floatx80_posone);
	}

	while (extF80_le(a, floatx80_negone)) {
		val = extF80_div(val, base);

		if (!extF80_exp(val)) {  // subnormal
			goto ret;
		}

		a = extF80_add(a, floatx80_posone);
	}

ret:
	/* ******************************** */
	/* using float128 for approximation */
	/* ******************************** */
	x = extF80_to_f128(a);
	x = f128_mul(x, float128_ln2);
	x = poly_exp(x);

	val = extF80_mul(val, extF80_add(f128_to_extF80(x), floatx80_posone));

	return val;
}

// =================================================
//                                   x
//                         Compute 10
// =================================================

//
// Uses the following identities:
//
// 1. ----------------------------------------------------------
//      x    x*ln(10)
//     2  = e
//
// 2. ----------------------------------------------------------
//                      2     3     4     5           n
//      x        x     x     x     x     x           x
//     e  = 1 + --- + --- + --- + --- + --- + ... + --- + ...
//               1!    2!    3!    4!    5!          n!
//
extFloat80_t extFloat80_10tox(extFloat80_t a)
{
	uint64_t zSig0, zSig1, zSig2;
	struct exp32_sig64 normExpSig;

	// handle unsupported extended double-precision floating encodings
	if (extF80_isUnsupported(a)) {
		softfloat_exceptionFlags |= softfloat_flag_invalid;
		return floatx80_default_nan;
	}

	uint64_t aSig = extF80_fraction(a);
	int32_t aExp = extF80_exp(a);
	int aSign = extF80_sign(a);

	if (aExp == 0x7FFF) {
		if (aSig << 1) {
			const uint128 nan = softfloat_propagateNaNExtF80UI(a.signExp, aSig, 0, 0);
			extFloat80_t rv;
			rv.signExp = nan.v64;
			rv.signif = nan.v0;
			return rv;
		}

		return (aSign) ? floatx80_negone : a;
	}

	if (! aExp) {
		if (! aSig) return a;
		softfloat_exceptionFlags |= softfloat_flag_inexact; // denormal also
		normExpSig = softfloat_normSubnormalExtF80Sig(aSig);
		aExp = normExpSig.exp + 1;
		aSig = normExpSig.sig;

		mul128By64To192(LN10_SIG_HI, LN10_SIG_LO, aSig, &zSig0, &zSig1, &zSig2);
		if (0 < (int64_t) zSig0) {
			shortShift128Left(zSig0, zSig1, 1, &zSig0, &zSig1);
			--aExp;
		}
		return softfloat_roundPackToExtF80(aSign, aExp, zSig0, zSig1, 80);
	}

	softfloat_exceptionFlags |= softfloat_flag_inexact;

	const extFloat80_t base = i32_to_extF80(10);
	extFloat80_t val = floatx80_posone;
	float128_t x;

	while (!(extF80_lt(a, floatx80_posone))) {
		val = extF80_mul(val, base);

		if (extF80_exp(val) == 0x7FFF) {  // inf
			goto ret;
		}

		a = extF80_sub(a, floatx80_posone);
	}

	while (extF80_le(a, floatx80_negone)) {
		val = extF80_div(val, base);

		if (!extF80_exp(val)) {  // subnormal
			goto ret;
		}

		a = extF80_add(a, floatx80_posone);
	}

ret:
	/* ******************************** */
	/* using float128 for approximation */
	/* ******************************** */
	x = extF80_to_f128(a);
	x = f128_mul(x, float128_ln10);
	x = poly_exp(x);

	val = extF80_mul(val, extF80_add(f128_to_extF80(x), floatx80_posone));

	return val;
}
