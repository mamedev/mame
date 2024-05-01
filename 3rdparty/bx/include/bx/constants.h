/*
 * Copyright 2011-2024 Branimir Karadzic. All rights reserved.
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

	/// The reciprocal of kPi. 1/kPi
	constexpr float    kInvPi          = 1.0f/kPi;

	/// The reciprocal of kPi2. 1/kPi2
	constexpr float    kInvPi2         = 1.0f/kPi2;

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

	///
	constexpr uint8_t  kHalfSignNumBits      = 1;
	constexpr uint8_t  kHalfSignBitShift     = 15;
	constexpr uint16_t kHalfSignMask         = UINT16_C(0x8000);
	constexpr uint8_t  kHalfExponentNumBits  = 5;
	constexpr uint8_t  kHalfExponentBitShift = 10;
	constexpr uint16_t kHalfExponentMask     = UINT16_C(0x7c00);
	constexpr uint32_t kHalfExponentBias     = 15;
	constexpr uint8_t  kHalfMantissaNumBits  = 10;
	constexpr uint8_t  kHalfMantissaBitShift = 0;
	constexpr uint16_t kHalfMantissaMask     = UINT16_C(0x03ff);

	///
	constexpr uint8_t  kFloatSignNumBits      = 1;
	constexpr uint8_t  kFloatSignBitShift     = 31;
	constexpr uint32_t kFloatSignMask         = UINT32_C(0x80000000);
	constexpr uint8_t  kFloatExponentNumBits  = 8;
	constexpr uint8_t  kFloatExponentBitShift = 23;
	constexpr uint32_t kFloatExponentMask     = UINT32_C(0x7f800000);
	constexpr uint32_t kFloatExponentBias     = 127;
	constexpr uint8_t  kFloatMantissaNumBits  = 23;
	constexpr uint8_t  kFloatMantissaBitShift = 0;
	constexpr uint32_t kFloatMantissaMask     = UINT32_C(0x007fffff);

	/// Smallest normalized positive floating-point number.
	constexpr float    kFloatSmallest  = 1.175494351e-38f;

	/// Maximum representable floating-point number.
	constexpr float    kFloatLargest   = 3.402823466e+38f;

	///
	extern const float kFloatInfinity;

	///
	constexpr uint8_t  kDoubleSignNumBits     = 1;
	constexpr uint8_t  kDoubleSignBitShift    = 63;
	constexpr uint64_t kDoubleSignMask        = UINT64_C(0x8000000000000000);
	constexpr uint8_t  kDoubleExponentNumBits = 11;
	constexpr uint8_t  kDoubleExponentShift   = 52;
	constexpr uint64_t kDoubleExponentMask    = UINT64_C(0x7ff0000000000000);
	constexpr uint32_t kDoubleExponentBias    = 1023;
	constexpr uint8_t  kDoubleMantissaNumBits = 52;
	constexpr uint8_t  kDoubleMantissaShift   = 0;
	constexpr uint64_t kDoubleMantissaMask    = UINT64_C(0x000fffffffffffff);

	/// Smallest normalized positive double-precision floating-point number.
	constexpr double   kDoubleSmallest = 2.2250738585072014e-308;

	/// Largest representable double-precision floating-point number.
	constexpr double   kDoubleLargest  = 1.7976931348623158e+308;

	///
	extern const double kDoubleInfinity;

} // namespace bx

#endif // BX_CONSTANTS_H_HEADER_GUARD
