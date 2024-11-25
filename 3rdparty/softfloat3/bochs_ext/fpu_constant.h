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

#ifndef _FPU_CONSTANTS_H_
#define _FPU_CONSTANTS_H_

// Pentium CPU uses only 68-bit precision M_PI approximation
// #define BETTER_THAN_PENTIUM

/*============================================================================
 * Written for Bochs (x86 achitecture simulator) by
 *            Stanislav Shwartsman [sshwarts at sourceforge net]
 * ==========================================================================*/

//////////////////////////////
// PI, PI/2, PI/4 constants
//////////////////////////////

#define FLOATX80_PI_EXP  (0x4000)

// 128-bit PI fraction
#ifdef BETTER_THAN_PENTIUM
#define FLOAT_PI_HI (uint64_t(0xc90fdaa22168c234))
#define FLOAT_PI_LO (uint64_t(0xc4c6628b80dc1cd1))
#else
#define FLOAT_PI_HI (uint64_t(0xc90fdaa22168c234))
#define FLOAT_PI_LO (uint64_t(0xC000000000000000))
#endif

#define FLOATX80_PI2_EXP  (0x3FFF)
#define FLOATX80_PI4_EXP  (0x3FFE)

//////////////////////////////
// 3PI/4 constant
//////////////////////////////

#define FLOATX80_3PI4_EXP (0x4000)

// 128-bit 3PI/4 fraction
#ifdef BETTER_THAN_PENTIUM
#define FLOAT_3PI4_HI (uint64_t(0x96cbe3f9990e91a7))
#define FLOAT_3PI4_LO (uint64_t(0x9394c9e8a0a5159c))
#else
#define FLOAT_3PI4_HI (uint64_t(0x96cbe3f9990e91a7))
#define FLOAT_3PI4_LO (uint64_t(0x9000000000000000))
#endif

//////////////////////////////
// 1/LN2 constant
//////////////////////////////

#define FLOAT_LN2INV_EXP  (0x3FFF)

// 128-bit 1/LN2 fraction
#ifdef BETTER_THAN_PENTIUM
#define FLOAT_LN2INV_HI (uint64_t(0xb8aa3b295c17f0bb))
#define FLOAT_LN2INV_LO (uint64_t(0xbe87fed0691d3e89))
#else
#define FLOAT_LN2INV_HI (uint64_t(0xb8aa3b295c17f0bb))
#define FLOAT_LN2INV_LO (uint64_t(0xC000000000000000))
#endif

#endif
