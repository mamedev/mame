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
 * Adapted for 3rdparty/softfloat in MAME by Calvin Buckley (05/2021)
 * ==========================================================================*/

#define FLOAT128

#include "mamesf.h"
#include "softfloat.h"
//#include "softfloat-specialize"
#include "fpu_constant.h"

/* XXX: These are common w/ fsincos/fyl2x; should be moved to common header? */
#define packFloat_128(zHi, zLo) {(zHi), (zLo)}
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
| Normalizes the subnormal extended double-precision floating-point value
| represented by the denormalized significand `aSig'.  The normalized exponent
| and significand are stored at the locations pointed to by `zExpPtr' and
| `zSigPtr', respectively.
*----------------------------------------------------------------------------*/

INLINE void normalizeFloatx80Subnormal(uint64_t aSig, int32_t *zExpPtr, uint64_t *zSigPtr)
{
	int shiftCount = countLeadingZeros64(aSig);
	*zSigPtr = aSig<<shiftCount;
	*zExpPtr = 1 - shiftCount;
}

/*----------------------------------------------------------------------------
| Returns 1 if the extended double-precision floating-point value `a' is a
| NaN; otherwise returns 0.
*----------------------------------------------------------------------------*/

INLINE int floatx80_is_nan(floatx80 a)
{
	return ((a.high & 0x7FFF) == 0x7FFF) && (int64_t) (a.low<<1);
}

/*----------------------------------------------------------------------------
| Takes two extended double-precision floating-point values `a' and `b', one
| of which is a NaN, and returns the appropriate NaN result.  If either `a' or
| `b' is a signaling NaN, the invalid exception is raised.
*----------------------------------------------------------------------------*/

INLINE floatx80 propagateFloatx80NaN( floatx80 a, floatx80 b )
{
    flag aIsNaN, aIsSignalingNaN, bIsNaN, bIsSignalingNaN;

    aIsNaN = floatx80_is_nan( a );
    aIsSignalingNaN = floatx80_is_signaling_nan( a );
    bIsNaN = floatx80_is_nan( b );
    bIsSignalingNaN = floatx80_is_signaling_nan( b );
    a.low |= LIT64( 0xC000000000000000 );
    b.low |= LIT64( 0xC000000000000000 );
    if ( aIsSignalingNaN | bIsSignalingNaN ) float_raise( float_flag_invalid );
    if ( aIsNaN ) {
        return ( aIsSignalingNaN & bIsNaN ) ? b : a;
    }
    else {
        return b;
    }

}

/*----------------------------------------------------------------------------
| Returns the exponent bits of the quadruple-precision floating-point value
| `a'.
*----------------------------------------------------------------------------*/

INLINE int32 extractFloat128Exp( float128 a )
{
	return ( a.high>>48 ) & 0x7FFF;

}

/* end copied */

#define FPATAN_ARR_SIZE 11

static const float128 float128_one =
        packFloat_128(0x3fff000000000000U, 0x0000000000000000U);
static const float128 float128_sqrt3 =
        packFloat_128(0x3fffbb67ae8584caU, 0xa73b25742d7078b8U);
static const floatx80 floatx80_one = packFloatx80(0, 0x3fff, 0x8000000000000000U);
static const floatx80 floatx80_pi  =
        packFloatx80(0, 0x4000, 0xc90fdaa22168c235U);

static const float128 float128_pi2 =
        packFloat_128(0x3fff921fb54442d1U, 0x8469898CC5170416U);
static const float128 float128_pi4 =
        packFloat_128(0x3ffe921fb54442d1U, 0x8469898CC5170416U);
static const float128 float128_pi6 =
        packFloat_128(0x3ffe0c152382d736U, 0x58465BB32E0F580FU);

static float128 atan_arr[FPATAN_ARR_SIZE] =
{
    PACK_FLOAT_128(0x3fff000000000000, 0x0000000000000000), /*  1 */
    PACK_FLOAT_128(0xbffd555555555555, 0x5555555555555555), /*  3 */
    PACK_FLOAT_128(0x3ffc999999999999, 0x999999999999999a), /*  5 */
    PACK_FLOAT_128(0xbffc249249249249, 0x2492492492492492), /*  7 */
    PACK_FLOAT_128(0x3ffbc71c71c71c71, 0xc71c71c71c71c71c), /*  9 */
    PACK_FLOAT_128(0xbffb745d1745d174, 0x5d1745d1745d1746), /* 11 */
    PACK_FLOAT_128(0x3ffb3b13b13b13b1, 0x3b13b13b13b13b14), /* 13 */
    PACK_FLOAT_128(0xbffb111111111111, 0x1111111111111111), /* 15 */
    PACK_FLOAT_128(0x3ffae1e1e1e1e1e1, 0xe1e1e1e1e1e1e1e2), /* 17 */
    PACK_FLOAT_128(0xbffaaf286bca1af2, 0x86bca1af286bca1b), /* 19 */
    PACK_FLOAT_128(0x3ffa861861861861, 0x8618618618618618)  /* 21 */
};

extern float128 OddPoly(float128 x, float128 *arr, unsigned n);

/* |x| < 1/4 */
static float128 poly_atan(float128 x1)
{
/*
    //                 3     5     7     9     11     13     15     17
    //                x     x     x     x     x      x      x      x
    // atan(x) ~ x - --- + --- - --- + --- - ---- + ---- - ---- + ----
    //                3     5     7     9     11     13     15     17
    //
    //                 2     4     6     8     10     12     14     16
    //                x     x     x     x     x      x      x      x
    //   = x * [ 1 - --- + --- - --- + --- - ---- + ---- - ---- + ---- ]
    //                3     5     7     9     11     13     15     17
    //
    //           5                          5
    //          --       4k                --        4k+2
    //   p(x) = >  C  * x           q(x) = >  C   * x
    //          --  2k                     --  2k+1
    //          k=0                        k=0
    //
    //                            2
    //    atan(x) ~ x * [ p(x) + x * q(x) ]
    //
*/
    return OddPoly(x1, atan_arr, FPATAN_ARR_SIZE);
}

// =================================================
// FPATAN                  Compute y * log (x)
//                                        2
// =================================================

//
// Uses the following identities:
//
// 1. ----------------------------------------------------------
//
//   atan(-x) = -atan(x)
//
// 2. ----------------------------------------------------------
//
//                             x + y
//   atan(x) + atan(y) = atan -------, xy < 1
//                             1-xy
//
//                             x + y
//   atan(x) + atan(y) = atan ------- + PI, x > 0, xy > 1
//                             1-xy
//
//                             x + y
//   atan(x) + atan(y) = atan ------- - PI, x < 0, xy > 1
//                             1-xy
//
// 3. ----------------------------------------------------------
//
//   atan(x) = atan(INF) + atan(- 1/x)
//
//                           x-1
//   atan(x) = PI/4 + atan( ----- )
//                           x+1
//
//                           x * sqrt(3) - 1
//   atan(x) = PI/6 + atan( ----------------- )
//                             x + sqrt(3)
//
// 4. ----------------------------------------------------------
//                   3     5     7     9                 2n+1
//                  x     x     x     x              n  x
//   atan(x) = x - --- + --- - --- + --- - ... + (-1)  ------ + ...
//                  3     5     7     9                 2n+1
//

floatx80 floatx80_fpatan(floatx80 a, floatx80 b)
{
    uint64_t aSig = extractFloatx80Frac(a);
    int32_t aExp = extractFloatx80Exp(a);
    int aSign = extractFloatx80Sign(a);
    uint64_t bSig = extractFloatx80Frac(b);
    int32_t bExp = extractFloatx80Exp(b);
    int bSign = extractFloatx80Sign(b);

    int zSign = aSign ^ bSign;

    if (bExp == 0x7FFF)
    {
        if ((uint64_t) (bSig<<1))
            return propagateFloatx80NaN(a, b);

        if (aExp == 0x7FFF) {
            if ((uint64_t) (aSig<<1))
                return propagateFloatx80NaN(a, b);

            if (aSign) {   /* return 3PI/4 */
                return roundAndPackFloatx80(80, bSign,
                        FLOATX80_3PI4_EXP, FLOAT_3PI4_HI, FLOAT_3PI4_LO);
            }
            else {         /* return  PI/4 */
                return roundAndPackFloatx80(80, bSign,
                        FLOATX80_PI4_EXP, FLOAT_PI_HI, FLOAT_PI_LO);
            }
        }

        if (aSig && (aExp == 0))
            float_raise(float_flag_denormal);

        /* return PI/2 */
        return roundAndPackFloatx80(80, bSign, FLOATX80_PI2_EXP, FLOAT_PI_HI, FLOAT_PI_LO);
    }
    if (aExp == 0x7FFF)
    {
        if ((uint64_t) (aSig<<1))
            return propagateFloatx80NaN(a, b);

        if (bSig && (bExp == 0))
            float_raise(float_flag_denormal);

return_PI_or_ZERO:

        if (aSign) {   /* return PI */
            return roundAndPackFloatx80(80, bSign, FLOATX80_PI_EXP, FLOAT_PI_HI, FLOAT_PI_LO);
        } else {       /* return  0 */
            return packFloatx80(bSign, 0, 0);
        }
    }
    if (bExp == 0)
    {
        if (bSig == 0) {
             if (aSig && (aExp == 0)) float_raise(float_flag_denormal);
             goto return_PI_or_ZERO;
        }

        float_raise(float_flag_denormal);
        normalizeFloatx80Subnormal(bSig, &bExp, &bSig);
    }
    if (aExp == 0)
    {
        if (aSig == 0)   /* return PI/2 */
            return roundAndPackFloatx80(80, bSign, FLOATX80_PI2_EXP, FLOAT_PI_HI, FLOAT_PI_LO);

        float_raise(float_flag_denormal);
        normalizeFloatx80Subnormal(aSig, &aExp, &aSig);
    }

    float_raise(float_flag_inexact);

    /* |a| = |b| ==> return PI/4 */
    if (aSig == bSig && aExp == bExp)
        return roundAndPackFloatx80(80, bSign, FLOATX80_PI4_EXP, FLOAT_PI_HI, FLOAT_PI_LO);

    /* ******************************** */
    /* using float128 for approximation */
    /* ******************************** */

    float128 a128 = normalizeRoundAndPackFloat128(0, aExp-0x10, aSig, 0);
    float128 b128 = normalizeRoundAndPackFloat128(0, bExp-0x10, bSig, 0);
    float128 x;
    int swap = 0, add_pi6 = 0, add_pi4 = 0;

    if (aExp > bExp || (aExp == bExp && aSig > bSig))
    {
        x = float128_div(b128, a128);
    }
    else {
        x = float128_div(a128, b128);
        swap = 1;
    }

    int32_t xExp = extractFloat128Exp(x);

    if (xExp <= EXP_BIAS-40)
        goto approximation_completed;

    if (x.high >= 0x3ffe800000000000U)        // 3/4 < x < 1
    {
        /*
        arctan(x) = arctan((x-1)/(x+1)) + pi/4
        */
        float128 t1 = float128_sub(x, float128_one);
        float128 t2 = float128_add(x, float128_one);
        x = float128_div(t1, t2);
        add_pi4 = 1;
    }
    else
    {
        /* argument correction */
        if (xExp >= 0x3FFD)                     // 1/4 < x < 3/4
        {
            /*
            arctan(x) = arctan((x*sqrt(3)-1)/(x+sqrt(3))) + pi/6
            */
            float128 t1 = float128_mul(x, float128_sqrt3);
            float128 t2 = float128_add(x, float128_sqrt3);
            x = float128_sub(t1, float128_one);
            x = float128_div(x, t2);
            add_pi6 = 1;
        }
    }

    x = poly_atan(x);
    if (add_pi6) x = float128_add(x, float128_pi6);
    if (add_pi4) x = float128_add(x, float128_pi4);

approximation_completed:
    if (swap) x = float128_sub(float128_pi2, x);
    floatx80 result = float128_to_floatx80(x);
    if (zSign) floatx80_chs(result);
    int rSign = extractFloatx80Sign(result);
    if (!bSign && rSign)
        return floatx80_add(result, floatx80_pi);
    if (bSign && !rSign)
        return floatx80_sub(result, floatx80_pi);
    return result;
}

// The former function maps to x87 FPATAN, but we can simulate 68881 FATAN with
// it by simply hardcoding one here.
floatx80 floatx80_fatan(floatx80 a)
{
	return floatx80_fpatan(a, floatx80_one);
}
