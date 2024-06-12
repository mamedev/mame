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
 * Adapted for Bochs (x86 achitecture simulator) by
 *            Stanislav Shwartsman [sshwarts at sourceforge net]
 * ==========================================================================*/

#ifndef _SOFTFLOAT_HELPERS_H_
#define _SOFTFLOAT_HELPERS_H_

#include <assert.h>
#include <stdint.h>

#include "../source/include/primitives.h"
#include "../source/include/primitiveTypes.h"

/*----------------------------------------------------------------------------
| Multiplies the 128-bit value formed by concatenating `a0' and `a1' by
| `b' to obtain a 192-bit product.  The product is broken into three 64-bit
| pieces which are stored at the locations pointed to by `z0Ptr', `z1Ptr', and
| `z2Ptr'.
*----------------------------------------------------------------------------*/

inline void mul128By64To192(uint64_t a64, uint64_t a0, uint64_t b, uint64_t *z0Ptr, uint64_t *z1Ptr, uint64_t *z2Ptr)
{
	uint64_t zPtr[4];

	softfloat_mul128To256M(a64, a0, 0, b, (uint64_t*) zPtr);

	assert(zPtr[indexWord(4, 3)] == 0);

	*z0Ptr = zPtr[indexWord(4, 2)];
	*z1Ptr = zPtr[indexWord(4, 1)];
	*z2Ptr = zPtr[indexWord(4, 0)];
}

/*----------------------------------------------------------------------------
| Shifts the 128-bit value formed by concatenating `a0' and `a1' left by the
| number of bits given in `count'.  Any bits shifted off are lost.  The value
| of `count' must be less than 64.  The result is broken into two 64-bit
| pieces which are stored at the locations pointed to by `z0Ptr' and `z1Ptr'.
*----------------------------------------------------------------------------*/

inline void shortShift128Left(uint64_t a0, uint64_t a1, int count, uint64_t *z0Ptr, uint64_t *z1Ptr)
{
	*z1Ptr = a1<<count;
	*z0Ptr = (count == 0) ? a0 : (a0<<count) | (a1>>((-count) & 63));
}

/*----------------------------------------------------------------------------
| Shifts the 128-bit value formed by concatenating `a0' and `a1' right by the
| number of bits given in `count'.  Any bits shifted off are lost.  The value
| of `count' must be less than 64. The result is broken into two 64-bit pieces
| which are stored at the locations pointed to by `z0Ptr' and `z1Ptr'.
*----------------------------------------------------------------------------*/

inline void shortShift128Right(uint64_t a0, uint64_t a1, int count, uint64_t *z0Ptr, uint64_t *z1Ptr)
{
	uint64_t z0 = a0, z1 = a1;
	int negCount = (-count) & 63;

	if (count != 0) {
		z1 = (a0<<negCount) | (a1>>count);
		z0 = a0>>count;
	}
	*z1Ptr = z1;
	*z0Ptr = z0;
}

/*----------------------------------------------------------------------------
| Returns an approximation to the 64-bit integer quotient obtained by dividing
| `b' into the 128-bit value formed by concatenating `a0' and `a1'.  The
| divisor `b' must be at least 2^63.  If q is the exact quotient truncated
| toward zero, the approximation returned lies between q and q + 2 inclusive.
| If the exact quotient q is larger than 64 bits, the maximum positive 64-bit
| unsigned integer is returned.
*----------------------------------------------------------------------------*/

#ifdef USE_estimateDiv128To64
static uint64_t estimateDiv128To64(uint64_t a0, uint64_t a1, uint64_t b)
{
	uint128 term, rem;
	uint64_t b0, b1;
	uint64_t z;
	if (b <= a0) return uint64_t(0xFFFFFFFFFFFFFFFF);
	b0 = b>>32;
	z = (b0<<32 <= a0) ? uint64_t(0xFFFFFFFF00000000) : (a0 / b0)<<32;
	term = softfloat_mul64To128(b, z);
	rem = softfloat_sub128(a0, a1, term.v64, term.v0);
	while (((int64_t) rem.v64) < 0) {
		z -= UINT64_C(0x100000000);
		b1 = b<<32;
		rem = softfloat_add128(rem.v64, rem.v0, b0, b1);
	}
	rem.v64 = (rem.v64<<32) | (rem.v0>>32);
	z |= (b0<<32 <= rem.v64) ? 0xFFFFFFFF : rem.v64 / b0;
	return z;
}
#endif

/*----------------------------------------------------------------------------
| Adds the 192-bit value formed by concatenating `a0', `a1', and `a2' to the
| 192-bit value formed by concatenating `b0', `b1', and `b2'.  Addition is
| modulo 2^192, so any carry out is lost.  The result is broken into three
| 64-bit pieces which are stored at the locations pointed to by `z0Ptr',
| `z1Ptr', and `z2Ptr'.
*----------------------------------------------------------------------------*/

inline void add192(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t b0, uint64_t b1, uint64_t b2, uint64_t *z0Ptr, uint64_t *z1Ptr, uint64_t *z2Ptr)
{
	uint64_t z0, z1, z2;
	unsigned carry0, carry1;

	z2 = a2 + b2;
	carry1 = (z2 < a2);
	z1 = a1 + b1;
	carry0 = (z1 < a1);
	z0 = a0 + b0;
	z1 += carry1;
	z0 += (z1 < carry1);
	z0 += carry0;
	*z2Ptr = z2;
	*z1Ptr = z1;
	*z0Ptr = z0;
}

/*----------------------------------------------------------------------------
| Subtracts the 128-bit value formed by concatenating `b0' and `b1' from the
| 128-bit value formed by concatenating `a0' and `a1'.  Subtraction is modulo
| 2^128, so any borrow out (carry out) is lost.  The result is broken into two
| 64-bit pieces which are stored at the locations pointed to by `z0Ptr' and
| `z1Ptr'.
*----------------------------------------------------------------------------*/

inline void sub128(uint64_t a0, uint64_t a1, uint64_t b0, uint64_t b1, uint64_t *z0Ptr, uint64_t *z1Ptr)
{
	*z1Ptr = a1 - b1;
	*z0Ptr = a0 - b0 - (a1 < b1);
}

#endif
