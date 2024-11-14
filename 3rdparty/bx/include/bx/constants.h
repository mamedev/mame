/*
 * Copyright 2011-2022 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#ifndef BX_CONSTANTS_H_HEADER_GUARD
#define BX_CONSTANTS_H_HEADER_GUARD

namespace bx
{
	/// Used to return successful execution of a program code.
	constexpr int32_t  kExitSuccess    = 0;

	/// Used to return unsuccessful execution of a program code.
	constexpr int32_t  kExitFailure    = 1;

	/// The ratio of a circle's circumference to its diameter,
	constexpr float    kPi             = 3.1415926535897932384626433832795f;

	/// The ratio of a circle's circumference to its radius. Pi multiplied by 2, or Tau. pi*2
	constexpr float    kPi2            = 6.2831853071795864769252867665590f;

	/// The reciprocal of pi. 1/pi
	constexpr float    kInvPi          = 1.0f/kPi;

	/// Pi divided by two. pi/2
	constexpr float    kPiHalf         = 1.5707963267948966192313216916398f;

	/// Pi divided by four. pi/4
	constexpr float    kPiQuarter      = 0.7853981633974483096156608458199f;

	/// The square root of two. sqrt(2)
	constexpr float    kSqrt2          = 1.4142135623730950488016887242097f;

	/// ln(10)
	constexpr float    kLogNat10       = 2.3025850929940456840179914546844f;

	/// The logarithm of the e to base 2. ln(kE) / ln(2)
	constexpr float    kInvLogNat2     = 1.4426950408889634073599246810019f;

	/// The natural logarithm of the 2. ln(2)
	constexpr float    kLogNat2Hi      = 0.6931471805599453094172321214582f;

	///
	constexpr float    kLogNat2Lo      = 1.90821492927058770002e-10f;

	/// The base of natural logarithms. e(1)
	constexpr float    kE              = 2.7182818284590452353602874713527f;

	///
	constexpr float    kNearZero       = 1.0f/float(1 << 28);

	/// Smallest normalized positive floating-point number.
	constexpr float    kFloatSmallest  = 1.175494351e-38f;

	/// Maximum representable floating-point number.
	constexpr float    kFloatLargest   = 3.402823466e+38f;

	/// Smallest normalized positive double-precision floating-point number.
	constexpr double   kDoubleSmallest = 2.2250738585072014e-308;

	/// Largest representable double-precision floating-point number.
	constexpr double   kDoubleLargest  = 1.7976931348623158e+308;

	///
	extern const float kInfinity;

} // namespace bx

#endif // BX_CONSTANTS_H_HEADER_GUARD
