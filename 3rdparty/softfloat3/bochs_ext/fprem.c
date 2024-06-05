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

#define USE_estimateDiv128To64
#define FLOAT128

#include <stdint.h>

#include "../build/MAME/platform.h"
#include "../source/include/internals.h"
#include "../source/include/softfloat.h"

#include "../source/8086/specialize.h"

#include "fpu_constant.h"
#include "softfloat-extra.h"
#include "softfloat-helpers.h"
#include "softfloat-specialize.h"

/* executes single exponent reduction cycle */
static uint64_t remainder_kernel(uint64_t aSig0, uint64_t bSig, int expDiff, uint64_t *zSig0, uint64_t *zSig1)
{
	uint128 term, z;
	uint64_t aSig1 = 0;
	shortShift128Left(aSig1, aSig0, expDiff, &aSig1, &aSig0);
	uint64_t q = estimateDiv128To64(aSig1, aSig0, bSig);
	term = softfloat_mul64To128(bSig, q);
	z = softfloat_sub128(aSig1, aSig0, term.v64, term.v0);
	while ((int64_t) z.v64 < 0) {
		--q;
		z = softfloat_add128(z.v64, z.v0, 0, bSig);
	}
	*zSig0 = z.v0;
	*zSig1 = z.v64;
	return q;
}

static int do_fprem(extFloat80_t a, extFloat80_t b, extFloat80_t &r, uint64_t &q, int rounding_mode)
{
	int32_t aExp, bExp, zExp, expDiff;
	uint64_t aSig0, aSig1 = 0, bSig;
	int aSign;
	struct exp32_sig64 normExpSig;
	uint128 term;

	q = 0;

	// handle unsupported extended double-precision floating encodings
	if (extF80_isUnsupported(a) || extF80_isUnsupported(b)) {
		softfloat_exceptionFlags |= softfloat_flag_invalid;
		r = floatx80_default_nan;
		return -1;
	}

	aSig0 = extF80_fraction(a);
	aExp = extF80_exp(a);
	aSign = extF80_sign(a);
	bSig = extF80_fraction(b);
	bExp = extF80_exp(b);

	if (aExp == 0x7FFF) {
		if ((aSig0<<1) || ((bExp == 0x7FFF) && (bSig<<1))) {
			const uint128 nan = softfloat_propagateNaNExtF80UI(a.signExp, a.signif, b.signExp, b.signif);
			r.signExp = nan.v64;
			r.signif = nan.v0;
			return -1;
		}
		softfloat_exceptionFlags |= softfloat_flag_invalid;
		r = floatx80_default_nan;
		return -1;
	}
	if (bExp == 0x7FFF) {
		if (bSig << 1) {
			const uint128 nan = softfloat_propagateNaNExtF80UI(a.signExp, a.signif, b.signExp, b.signif);
			r.signExp = nan.v64;
			r.signif = nan.v0;
			return -1;
		}
		if (! aExp && aSig0) {
			softfloat_exceptionFlags |= softfloat_flag_invalid; // actually denormal
			normExpSig = softfloat_normSubnormalExtF80Sig(aSig0);
			aExp = normExpSig.exp + 1;
			aSig0 = normExpSig.sig;
			r = (a.signif & uint64_t(0x8000000000000000)) ? packToExtF80(aSign, aExp, aSig0) : a;
			return 0;
		}
		r = a;
		return 0;

	}
	if (! bExp) {
		if (! bSig) {
			softfloat_exceptionFlags |= softfloat_flag_invalid;
			r = floatx80_default_nan;
			return -1;
		}
		softfloat_exceptionFlags |= softfloat_flag_invalid; // actually denormal
		normExpSig = softfloat_normSubnormalExtF80Sig(bSig);
		bExp = normExpSig.exp + 1;
		bSig = normExpSig.sig;
	}
	if (! aExp) {
		if (! aSig0) {
			r = a;
			return 0;
		}
		softfloat_exceptionFlags |= softfloat_flag_invalid; // actually denormal
		normExpSig = softfloat_normSubnormalExtF80Sig(aSig0);
		aExp = normExpSig.exp + 1;
		aSig0 = normExpSig.sig;
	}

	expDiff = aExp - bExp;
	int overflow = 0;

	if (expDiff >= 64) {
		int n = (expDiff & 0x1f) | 0x20;
		remainder_kernel(aSig0, bSig, n, &aSig0, &aSig1);
		zExp = aExp - n;
		overflow = 1;
	}
	else {
		zExp = bExp;

		if (expDiff < 0) {
			if (expDiff < -1) {
			   r = (a.signif & uint64_t(0x8000000000000000)) ? packToExtF80(aSign, aExp, aSig0) : a;
			   return 0;
			}
			shortShift128Right(aSig0, 0, 1, &aSig0, &aSig1);
			expDiff = 0;
		}

		if (expDiff > 0) {
			q = remainder_kernel(aSig0, bSig, expDiff, &aSig0, &aSig1);
		}
		else {
			if (bSig <= aSig0) {
			   aSig0 -= bSig;
			   q = 1;
			}
		}

		if (rounding_mode == softfloat_round_near_even) {
			uint64_t term0, term1;
			shortShift128Right(bSig, 0, 1, &term0, &term1);

			if (! softfloat_lt128(aSig0, aSig1, term0, term1)) {
				int lt = softfloat_lt128(term0, term1, aSig0, aSig1);
				int eq = softfloat_eq128(aSig0, aSig1, term0, term1);

				if ((eq && (q & 1)) || lt) {
					aSign = !aSign;
					++q;
				}
				if (lt) {
					term = softfloat_sub128(bSig, 0, aSig0, aSig1);
					aSig0 = term.v64;
					aSig1 = term.v0;
				}
			}
		}
	}

	r = softfloat_normRoundPackToExtF80(aSign, zExp, aSig0, aSig1, 80);
	return overflow;
}

/*----------------------------------------------------------------------------
| Returns the remainder of the extended double-precision floating-point value
| `a' with respect to the corresponding value `b'.  The operation is performed
| according to the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

int extFloat80_ieee754_remainder(extFloat80_t a, extFloat80_t b, extFloat80_t &r, uint64_t &q)
{
	return do_fprem(a, b, r, q, softfloat_round_near_even);
}

/*----------------------------------------------------------------------------
| Returns the remainder of the extended double-precision floating-point value
| `a' with  respect to  the corresponding value `b'. Unlike previous function
| the  function  does not compute  the remainder  specified  in  the IEC/IEEE
| Standard  for Binary  Floating-Point  Arithmetic.  This  function  operates
| differently  from the  previous  function in  the way  that it  rounds  the
| quotient of 'a' divided by 'b' to an integer.
*----------------------------------------------------------------------------*/

int extFloat80_remainder(extFloat80_t a, extFloat80_t b, extFloat80_t &r, uint64_t &q)
{
	return do_fprem(a, b, r, q, softfloat_round_minMag);
}

static extFloat80_t propagateFloatx80NaNOneArg(extFloat80_t a)
{
	a.signif |= uint64_t(0x4000000000000000);
	return a;
}

extFloat80_t extFloat80_getman(extFloat80_t a)
{
	const int aSign = (a.signExp >> 15);
	int32_t aExp = a.signExp & 0x7fff;
	uint64_t aFrac = a.signif;

	if (aExp == 0x7fff)
	{
		if ((uint64_t)(aFrac << 1))
		{
			return propagateFloatx80NaNOneArg(a);
		}

		softfloat_raiseFlags(softfloat_flag_invalid);
		return floatx80_default_nan;
	}

	if (!aExp)
	{
		if (!aSign)
		{
			return packFloatx80(aSign, 0, 0);
		}
		else
		{
			// normalize the subnormal value
			const int leadingZeroes = softfloat_countLeadingZeros64(aFrac);
			aFrac = aFrac << leadingZeroes;
			aExp = -leadingZeroes;
		}
	}

	return packFloatx80(aSign, 0x3fff, aFrac);
}
