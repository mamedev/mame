/*============================================================================

This C source file is part of the SoftFloat IEEE Floating-Point Arithmetic
Package, Release 3e, by John R. Hauser.

Copyright 2011, 2012, 2013, 2014, 2015, 2016 The Regents of the University of
California.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
    this list of conditions, and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions, and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

 3. Neither the name of the University nor the names of its contributors may
    be used to endorse or promote products derived from this software without
    specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS "AS IS", AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

#include <stdbool.h>
#include <stdint.h>

#include "../source/include/softfloat.h"
#include "../source/include/internals.h"
#include "../source/8086/specialize.h"

#include "fpu_constant.h"
#include "softfloat-helpers.h"
#include "softfloat-extra.h"
#include "softfloat-specialize.h"

/*----------------------------------------------------------------------------
| Scales extended double-precision floating-point value in operand `a' by
| value `b'. The function truncates the value in the second operand 'b' to
| an integral value and adds that value to the exponent of the operand 'a'.
| The operation performed according to the IEC/IEEE Standard for Binary
| Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

extFloat80_t extFloat80_scale(extFloat80_t a, extFloat80_t b)
{
	uint16_t uiA64;
	uint64_t uiA0;
	bool signA;
	int32_t expA;
	uint64_t sigA;
	uint16_t uiB64;
	uint64_t uiB0;
	bool signB;
	int32_t expB;
	uint64_t sigB;
	struct exp32_sig64 normExpSig;

	// handle unsupported extended double-precision floating encodings
	if (extF80_isUnsupported(a) || extF80_isUnsupported(b)) {
invalid:
		softfloat_exceptionFlags |= softfloat_flag_invalid;
		return packToExtF80(defaultNaNExtF80UI64, defaultNaNExtF80UI0);
	}

	/*------------------------------------------------------------------------
	*------------------------------------------------------------------------*/
	uiA64 = a.signExp;
	uiA0  = a.signif;
	signA = signExtF80UI64(uiA64);
	expA  = expExtF80UI64(uiA64);
	sigA  = uiA0;
	uiB64 = b.signExp;
	uiB0  = b.signif;
	signB = signExtF80UI64(uiB64);
	expB  = expExtF80UI64(uiB64);
	sigB  = uiB0;
	/*------------------------------------------------------------------------
	*------------------------------------------------------------------------*/

	if (expA == 0x7FFF) {
		if ((sigA<<1) || ((expB == 0x7FFF) && (sigB<<1))) {
			const uint128 nan = softfloat_propagateNaNExtF80UI(uiA64, uiA0, uiB64, uiB0);
			extFloat80_t rv;
			rv.signExp = nan.v64;
			rv.signif = nan.v0;
			return rv;
		}
		if ((expB == 0x7FFF) && signB) goto invalid;
		if (sigB && !expB)
			softfloat_exceptionFlags |= softfloat_flag_invalid; // actally denormal

		return a;
	}
	if (expB == 0x7FFF) {
		if (sigB<<1) {
			const uint128 nan = softfloat_propagateNaNExtF80UI(uiA64, uiA0, uiB64, uiB0);
			extFloat80_t rv;
			rv.signExp = nan.v64;
			rv.signif = nan.v0;
			return rv;
		}
		if ((expA | sigA) == 0) {
			if (! signB) goto invalid;
			return a;
		}
		if (sigA && !expA)
			softfloat_exceptionFlags |= softfloat_flag_invalid; // actually denormal

		if (signB) return packToExtF80(signA, 0, 0);
		return packToExtF80(signA, 0x7FFF, uint64_t(0x8000000000000000));
	}
	if (! expA) {
		if (sigB && !expB)
			softfloat_exceptionFlags |= softfloat_flag_invalid; // actually denormal
		if (! sigA) return a;
		softfloat_exceptionFlags |= softfloat_flag_invalid; // actually denormal
		normExpSig = softfloat_normSubnormalExtF80Sig(sigA);
		expA = normExpSig.exp + 1;
		sigA = normExpSig.sig;
		if (expB < 0x3FFF)
			return softfloat_normRoundPackToExtF80(signA, expA, sigA, 0, 80);
	}
	if (!expB) {
		if (!sigB) return a;
		softfloat_exceptionFlags |= softfloat_flag_invalid; // actually denormal
		normExpSig = softfloat_normSubnormalExtF80Sig(sigB);
		expB = normExpSig.exp + 1;
		sigB = normExpSig.sig;
	}

	if (expB > 0x400E) {
		/* generate appropriate overflow/underflow */
		return softfloat_roundPackToExtF80(signA, signB ? -0x3FFF : 0x7FFF, sigA, 0, 80);
	}

	if (expB < 0x3FFF) return a;

	int shiftCount = 0x403E - expB;
	sigB >>= shiftCount;
	int32_t scale = (int32_t) sigB;
	if (signB) scale = -scale; /* -32768..32767 */

	return softfloat_roundPackToExtF80(signA, expA + scale, sigA, 0, 80);
}
