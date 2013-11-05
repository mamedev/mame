
/*============================================================================

This C header file is part of the SoftFloat IEC/IEEE Floating-point Arithmetic
Package, Release 2b.

Written by John R. Hauser.  This work was made possible in part by the
International Computer Science Institute, located at Suite 600, 1947 Center
Street, Berkeley, California 94704.  Funding was partially provided by the
National Science Foundation under grant MIP-9311980.  The original version
of this code was written as part of a project to build a fixed-point vector
processor in collaboration with the University of California at Berkeley,
overseen by Profs. Nelson Morgan and John Wawrzynek.  More information
is available through the Web page `http://www.cs.berkeley.edu/~jhauser/
arithmetic/SoftFloat.html'.

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

/*----------------------------------------------------------------------------
| The macro `FLOATX80' must be defined to enable the extended double-precision
| floating-point format `floatx80'.  If this macro is not defined, the
| `floatx80' type will not be defined, and none of the functions that either
| input or output the `floatx80' type will be defined.  The same applies to
| the `FLOAT128' macro and the quadruple-precision format `float128'.
*----------------------------------------------------------------------------*/
#define FLOATX80
#define FLOAT128

/*----------------------------------------------------------------------------
| Software IEC/IEEE floating-point types.
*----------------------------------------------------------------------------*/
typedef bits32 float32;
typedef bits64 float64;
#ifdef FLOATX80
typedef struct {
	bits16 high;
	bits64 low;
} floatx80;
#endif
#ifdef FLOAT128
typedef struct {
	bits64 high, low;
} float128;
#endif

/*----------------------------------------------------------------------------
| Primitive arithmetic functions, including multi-word arithmetic, and
| division and square root approximations.  (Can be specialized to target if
| desired.)
*----------------------------------------------------------------------------*/
#include "softfloat-macros"

/*----------------------------------------------------------------------------
| Software IEC/IEEE floating-point underflow tininess-detection mode.
*----------------------------------------------------------------------------*/
extern int8 float_detect_tininess;
enum {
	float_tininess_after_rounding  = 0,
	float_tininess_before_rounding = 1
};

/*----------------------------------------------------------------------------
| Software IEC/IEEE floating-point rounding mode.
*----------------------------------------------------------------------------*/
extern int8 float_rounding_mode;
enum {
	float_round_nearest_even = 0,
	float_round_to_zero      = 1,
	float_round_down         = 2,
	float_round_up           = 3
};

/*----------------------------------------------------------------------------
| Software IEC/IEEE floating-point exception flags.
*----------------------------------------------------------------------------*/
extern int8 float_exception_flags;
enum {
	float_flag_invalid = 0x01, float_flag_denormal = 0x02, float_flag_divbyzero = 0x04, float_flag_overflow = 0x08,
	float_flag_underflow = 0x10, float_flag_inexact = 0x20
};

/*----------------------------------------------------------------------------
| Routine to raise any or all of the software IEC/IEEE floating-point
| exception flags.
*----------------------------------------------------------------------------*/
void float_raise( int8 );

/*----------------------------------------------------------------------------
| Software IEC/IEEE integer-to-floating-point conversion routines.
*----------------------------------------------------------------------------*/
float32 int32_to_float32( int32 );
float64 int32_to_float64( int32 );
#ifdef FLOATX80
floatx80 int32_to_floatx80( int32 );
#endif
#ifdef FLOAT128
float128 int32_to_float128( int32 );
#endif
float32 int64_to_float32( int64 );
float64 int64_to_float64( int64 );
#ifdef FLOATX80
floatx80 int64_to_floatx80( int64 );
#endif
#ifdef FLOAT128
float128 int64_to_float128( int64 );
#endif

/*----------------------------------------------------------------------------
| Software IEC/IEEE single-precision conversion routines.
*----------------------------------------------------------------------------*/
int32 float32_to_int32( float32 );
int32 float32_to_int32_round_to_zero( float32 );
int64 float32_to_int64( float32 );
int64 float32_to_int64_round_to_zero( float32 );
float64 float32_to_float64( float32 );
#ifdef FLOATX80
floatx80 float32_to_floatx80( float32 );
#endif
#ifdef FLOAT128
float128 float32_to_float128( float32 );
#endif

/*----------------------------------------------------------------------------
| Software IEC/IEEE single-precision operations.
*----------------------------------------------------------------------------*/
float32 float32_round_to_int( float32 );
float32 float32_add( float32, float32 );
float32 float32_sub( float32, float32 );
float32 float32_mul( float32, float32 );
float32 float32_div( float32, float32 );
float32 float32_rem( float32, float32 );
float32 float32_sqrt( float32 );
flag float32_eq( float32, float32 );
flag float32_le( float32, float32 );
flag float32_lt( float32, float32 );
flag float32_eq_signaling( float32, float32 );
flag float32_le_quiet( float32, float32 );
flag float32_lt_quiet( float32, float32 );
flag float32_is_signaling_nan( float32 );

/*----------------------------------------------------------------------------
| Software IEC/IEEE double-precision conversion routines.
*----------------------------------------------------------------------------*/
int32 float64_to_int32( float64 );
int32 float64_to_int32_round_to_zero( float64 );
int64 float64_to_int64( float64 );
int64 float64_to_int64_round_to_zero( float64 );
float32 float64_to_float32( float64 );
#ifdef FLOATX80
floatx80 float64_to_floatx80( float64 );
#endif
#ifdef FLOAT128
float128 float64_to_float128( float64 );
#endif

/*----------------------------------------------------------------------------
| Software IEC/IEEE double-precision operations.
*----------------------------------------------------------------------------*/
float64 float64_round_to_int( float64 );
float64 float64_add( float64, float64 );
float64 float64_sub( float64, float64 );
float64 float64_mul( float64, float64 );
float64 float64_div( float64, float64 );
float64 float64_rem( float64, float64 );
float64 float64_sqrt( float64 );
flag float64_eq( float64, float64 );
flag float64_le( float64, float64 );
flag float64_lt( float64, float64 );
flag float64_eq_signaling( float64, float64 );
flag float64_le_quiet( float64, float64 );
flag float64_lt_quiet( float64, float64 );
flag float64_is_signaling_nan( float64 );

#ifdef FLOATX80

/*----------------------------------------------------------------------------
| Software IEC/IEEE extended double-precision conversion routines.
*----------------------------------------------------------------------------*/
int32 floatx80_to_int32( floatx80 );
int32 floatx80_to_int32_round_to_zero( floatx80 );
int64 floatx80_to_int64( floatx80 );
int64 floatx80_to_int64_round_to_zero( floatx80 );
float32 floatx80_to_float32( floatx80 );
float64 floatx80_to_float64( floatx80 );
#ifdef FLOAT128
float128 floatx80_to_float128( floatx80 );
#endif
floatx80 floatx80_scale(floatx80 a, floatx80 b);

/*----------------------------------------------------------------------------
| Packs the sign `zSign', exponent `zExp', and significand `zSig' into an
| extended double-precision floating-point value, returning the result.
*----------------------------------------------------------------------------*/

INLINE floatx80 packFloatx80( flag zSign, int32 zExp, bits64 zSig )
{
	floatx80 z;

	z.low = zSig;
	z.high = ( ( (bits16) zSign )<<15 ) + zExp;
	return z;

}

/*----------------------------------------------------------------------------
| Software IEC/IEEE extended double-precision rounding precision.  Valid
| values are 32, 64, and 80.
*----------------------------------------------------------------------------*/
extern int8 floatx80_rounding_precision;

/*----------------------------------------------------------------------------
| Software IEC/IEEE extended double-precision operations.
*----------------------------------------------------------------------------*/
floatx80 floatx80_round_to_int( floatx80 );
floatx80 floatx80_add( floatx80, floatx80 );
floatx80 floatx80_sub( floatx80, floatx80 );
floatx80 floatx80_mul( floatx80, floatx80 );
floatx80 floatx80_div( floatx80, floatx80 );
floatx80 floatx80_rem( floatx80, floatx80 );
floatx80 floatx80_sqrt( floatx80 );
flag floatx80_eq( floatx80, floatx80 );
flag floatx80_le( floatx80, floatx80 );
flag floatx80_lt( floatx80, floatx80 );
flag floatx80_eq_signaling( floatx80, floatx80 );
flag floatx80_le_quiet( floatx80, floatx80 );
flag floatx80_lt_quiet( floatx80, floatx80 );
flag floatx80_is_signaling_nan( floatx80 );

int floatx80_fsin(floatx80 &a);
int floatx80_fcos(floatx80 &a);
int floatx80_ftan(floatx80 &a);

floatx80 floatx80_flognp1(floatx80 a);
floatx80 floatx80_flogn(floatx80 a);
floatx80 floatx80_flog2(floatx80 a);
floatx80 floatx80_flog10(floatx80 a);

// roundAndPackFloatx80 used to be in softfloat-round-pack, is now in softfloat.c
floatx80 roundAndPackFloatx80(int8 roundingPrecision, flag zSign, int32 zExp, bits64 zSig0, bits64 zSig1);

#endif

#ifdef FLOAT128

/*----------------------------------------------------------------------------
| Software IEC/IEEE quadruple-precision conversion routines.
*----------------------------------------------------------------------------*/
int32 float128_to_int32( float128 );
int32 float128_to_int32_round_to_zero( float128 );
int64 float128_to_int64( float128 );
int64 float128_to_int64_round_to_zero( float128 );
float32 float128_to_float32( float128 );
float64 float128_to_float64( float128 );
#ifdef FLOATX80
floatx80 float128_to_floatx80( float128 );
#endif

/*----------------------------------------------------------------------------
| Software IEC/IEEE quadruple-precision operations.
*----------------------------------------------------------------------------*/
float128 float128_round_to_int( float128 );
float128 float128_add( float128, float128 );
float128 float128_sub( float128, float128 );
float128 float128_mul( float128, float128 );
float128 float128_div( float128, float128 );
float128 float128_rem( float128, float128 );
float128 float128_sqrt( float128 );
flag float128_eq( float128, float128 );
flag float128_le( float128, float128 );
flag float128_lt( float128, float128 );
flag float128_eq_signaling( float128, float128 );
flag float128_le_quiet( float128, float128 );
flag float128_lt_quiet( float128, float128 );
flag float128_is_signaling_nan( float128 );

/*----------------------------------------------------------------------------
| Packs the sign `zSign', the exponent `zExp', and the significand formed
| by the concatenation of `zSig0' and `zSig1' into a quadruple-precision
| floating-point value, returning the result.  After being shifted into the
| proper positions, the three fields `zSign', `zExp', and `zSig0' are simply
| added together to form the most significant 32 bits of the result.  This
| means that any integer portion of `zSig0' will be added into the exponent.
| Since a properly normalized significand will have an integer portion equal
| to 1, the `zExp' input should be 1 less than the desired result exponent
| whenever `zSig0' and `zSig1' concatenated form a complete, normalized
| significand.
*----------------------------------------------------------------------------*/

INLINE float128
	packFloat128( flag zSign, int32 zExp, bits64 zSig0, bits64 zSig1 )
{
	float128 z;

	z.low = zSig1;
	z.high = ( ( (bits64) zSign )<<63 ) + ( ( (bits64) zExp )<<48 ) + zSig0;
	return z;

}

/*----------------------------------------------------------------------------
| Takes an abstract floating-point value having sign `zSign', exponent `zExp',
| and extended significand formed by the concatenation of `zSig0', `zSig1',
| and `zSig2', and returns the proper quadruple-precision floating-point value
| corresponding to the abstract input.  Ordinarily, the abstract value is
| simply rounded and packed into the quadruple-precision format, with the
| inexact exception raised if the abstract input cannot be represented
| exactly.  However, if the abstract value is too large, the overflow and
| inexact exceptions are raised and an infinity or maximal finite value is
| returned.  If the abstract value is too small, the input value is rounded to
| a subnormal number, and the underflow and inexact exceptions are raised if
| the abstract input cannot be represented exactly as a subnormal quadruple-
| precision floating-point number.
|     The input significand must be normalized or smaller.  If the input
| significand is not normalized, `zExp' must be 0; in that case, the result
| returned is a subnormal number, and it must not require rounding.  In the
| usual case that the input significand is normalized, `zExp' must be 1 less
| than the ``true'' floating-point exponent.  The handling of underflow and
| overflow follows the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

INLINE float128
	roundAndPackFloat128(
		flag zSign, int32 zExp, bits64 zSig0, bits64 zSig1, bits64 zSig2 )
{
	int8 roundingMode;
	flag roundNearestEven, increment, isTiny;

	roundingMode = float_rounding_mode;
	roundNearestEven = ( roundingMode == float_round_nearest_even );
	increment = ( (sbits64) zSig2 < 0 );
	if ( ! roundNearestEven ) {
		if ( roundingMode == float_round_to_zero ) {
			increment = 0;
		}
		else {
			if ( zSign ) {
				increment = ( roundingMode == float_round_down ) && zSig2;
			}
			else {
				increment = ( roundingMode == float_round_up ) && zSig2;
			}
		}
	}
	if ( 0x7FFD <= (bits32) zExp ) {
		if (    ( 0x7FFD < zExp )
				|| (    ( zExp == 0x7FFD )
					&& eq128(
							LIT64( 0x0001FFFFFFFFFFFF ),
							LIT64( 0xFFFFFFFFFFFFFFFF ),
							zSig0,
							zSig1
						)
					&& increment
				)
			) {
			float_raise( float_flag_overflow | float_flag_inexact );
			if (    ( roundingMode == float_round_to_zero )
					|| ( zSign && ( roundingMode == float_round_up ) )
					|| ( ! zSign && ( roundingMode == float_round_down ) )
				) {
				return
					packFloat128(
						zSign,
						0x7FFE,
						LIT64( 0x0000FFFFFFFFFFFF ),
						LIT64( 0xFFFFFFFFFFFFFFFF )
					);
			}
			return packFloat128( zSign, 0x7FFF, 0, 0 );
		}
		if ( zExp < 0 ) {
			isTiny =
					( float_detect_tininess == float_tininess_before_rounding )
				|| ( zExp < -1 )
				|| ! increment
				|| lt128(
						zSig0,
						zSig1,
						LIT64( 0x0001FFFFFFFFFFFF ),
						LIT64( 0xFFFFFFFFFFFFFFFF )
					);
			shift128ExtraRightJamming(
				zSig0, zSig1, zSig2, - zExp, &zSig0, &zSig1, &zSig2 );
			zExp = 0;
			if ( isTiny && zSig2 ) float_raise( float_flag_underflow );
			if ( roundNearestEven ) {
				increment = ( (sbits64) zSig2 < 0 );
			}
			else {
				if ( zSign ) {
					increment = ( roundingMode == float_round_down ) && zSig2;
				}
				else {
					increment = ( roundingMode == float_round_up ) && zSig2;
				}
			}
		}
	}
	if ( zSig2 ) float_exception_flags |= float_flag_inexact;
	if ( increment ) {
		add128( zSig0, zSig1, 0, 1, &zSig0, &zSig1 );
		zSig1 &= ~ ( ( zSig2 + zSig2 == 0 ) & roundNearestEven );
	}
	else {
		if ( ( zSig0 | zSig1 ) == 0 ) zExp = 0;
	}
	return packFloat128( zSign, zExp, zSig0, zSig1 );

}

/*----------------------------------------------------------------------------
| Takes an abstract floating-point value having sign `zSign', exponent `zExp',
| and significand formed by the concatenation of `zSig0' and `zSig1', and
| returns the proper quadruple-precision floating-point value corresponding
| to the abstract input.  This routine is just like `roundAndPackFloat128'
| except that the input significand has fewer bits and does not have to be
| normalized.  In all cases, `zExp' must be 1 less than the ``true'' floating-
| point exponent.
*----------------------------------------------------------------------------*/

INLINE float128
	normalizeRoundAndPackFloat128(
		flag zSign, int32 zExp, bits64 zSig0, bits64 zSig1 )
{
	int8 shiftCount;
	bits64 zSig2;

	if ( zSig0 == 0 ) {
		zSig0 = zSig1;
		zSig1 = 0;
		zExp -= 64;
	}
	shiftCount = countLeadingZeros64( zSig0 ) - 15;
	if ( 0 <= shiftCount ) {
		zSig2 = 0;
		shortShift128Left( zSig0, zSig1, shiftCount, &zSig0, &zSig1 );
	}
	else {
		shift128ExtraRightJamming(
			zSig0, zSig1, 0, - shiftCount, &zSig0, &zSig1, &zSig2 );
	}
	zExp -= shiftCount;
	return roundAndPackFloat128( zSign, zExp, zSig0, zSig1, zSig2 );

}
#endif
