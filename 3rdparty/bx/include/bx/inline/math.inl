/*
 * Copyright 2011-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

// FPU math lib

#ifndef BX_MATH_H_HEADER_GUARD
#	error "Must be included from bx/math.h!"
#endif // BX_MATH_H_HEADER_GUARD

#include <bx/simd_t.h>
#include <bx/uint32_t.h>

namespace bx
{
	inline BX_CONSTEXPR_FUNC float toRad(float _deg)
	{
		return _deg * kPi / 180.0f;
	}

	inline BX_CONSTEXPR_FUNC float toDeg(float _rad)
	{
		return _rad * 180.0f / kPi;
	}

	inline BX_CONSTEXPR_FUNC uint32_t floatToBits(float _a)
	{
		return bitCast<uint32_t>(_a);
	}

	inline BX_CONSTEXPR_FUNC float bitsToFloat(uint32_t _a)
	{
		return bitCast<float>(_a);
	}

	inline BX_CONSTEXPR_FUNC uint64_t doubleToBits(double _a)
	{
		return bitCast<uint64_t>(_a);
	}

	inline BX_CONSTEXPR_FUNC double bitsToDouble(uint64_t _a)
	{
		return bitCast<double>(_a);
	}

	inline BX_CONSTEXPR_FUNC uint32_t floatFlip(uint32_t _value)
	{
		// Reference(s):
		// - http://archive.fo/2012.12.08-212402/http://stereopsis.com/radix.html
		//
		const uint32_t tmp0   = uint32_sra(_value, 31);
		const uint32_t tmp1   = uint32_neg(tmp0);
		const uint32_t mask   = uint32_or(tmp1, kFloatSignMask);
		const uint32_t result = uint32_xor(_value, mask);
		return result;
	}

	inline BX_CONSTEXPR_FUNC bool isNan(float _f)
	{
		const uint32_t tmp = floatToBits(_f) & INT32_MAX;
		return tmp > kFloatExponentMask;
	}

	inline BX_CONSTEXPR_FUNC bool isNan(double _f)
	{
		const uint64_t tmp = doubleToBits(_f) & INT64_MAX;
		return tmp > kDoubleExponentMask;
	}

	inline BX_CONSTEXPR_FUNC bool isFinite(float _f)
	{
		const uint32_t tmp = floatToBits(_f) & INT32_MAX;
		return tmp < kFloatExponentMask;
	}

	inline BX_CONSTEXPR_FUNC bool isFinite(double _f)
	{
		const uint64_t tmp = doubleToBits(_f) & INT64_MAX;
		return tmp < kDoubleExponentMask;
	}

	inline BX_CONSTEXPR_FUNC bool isInfinite(float _f)
	{
		const uint32_t tmp = floatToBits(_f) & INT32_MAX;
		return tmp == kFloatExponentMask;
	}

	inline BX_CONSTEXPR_FUNC bool isInfinite(double _f)
	{
		const uint64_t tmp = doubleToBits(_f) & INT64_MAX;
		return tmp == kDoubleExponentMask;
	}

	inline BX_CONSTEXPR_FUNC float floor(float _a)
	{
		if (_a < 0.0f)
		{
			const float fr = fract(-_a);
			const float tr = trunc(-_a);

			return -tr - float(0.0f != fr);
		}

		return _a - fract(_a);
	}

	inline BX_CONSTEXPR_FUNC float ceil(float _a)
	{
		return -floor(-_a);
	}

	inline BX_CONSTEXPR_FUNC float round(float _a)
	{
		return floor(_a + 0.5f);
	}

	inline BX_CONSTEXPR_FUNC float lerp(float _a, float _b, float _t)
	{
		// Reference(s):
		// - Linear interpolation past, present and future
		//   https://web.archive.org/web/20200404165201/https://fgiesen.wordpress.com/2012/08/15/linear-interpolation-past-present-and-future/
		//
		return mad(_t, _b, nms(_t, _a, _a) );
	}

	inline BX_CONSTEXPR_FUNC float invLerp(float _a, float _b, float _value)
	{
		return (_value - _a) / (_b - _a);
	}

	inline BX_CONSTEXPR_FUNC float sign(float _a)
	{
		return float( (0.0f < _a) - (0.0f > _a) );
	}

	inline BX_CONSTEXPR_FUNC bool signBit(float _a)
	{
		return -0.0f == _a ? 0.0f != _a : 0.0f > _a;
	}

	inline BX_CONSTEXPR_FUNC float copySign(float _value, float _sign)
	{
#if BX_COMPILER_MSVC
		return signBit(_value) != signBit(_sign) ? -_value : _value;
#else
		return __builtin_copysign(_value, _sign);
#endif // BX_COMPILER_MSVC
	}

	inline BX_CONSTEXPR_FUNC float abs(float _a)
	{
		return _a < 0.0f ? -_a : _a;
	}

	inline BX_CONSTEXPR_FUNC float square(float _a)
	{
		return _a * _a;
	}

	inline BX_CONSTEXPR_FUNC float cos(float _a)
	{
		const float scaled = _a * 2.0f*kInvPi;
		const float real   = floor(scaled);
		const float xx     = _a - real * kPiHalf;
		const int32_t bits = int32_t(real) & 3;

		constexpr float kSinC2  = -0.16666667163372039794921875f;
		constexpr float kSinC4  =  8.333347737789154052734375e-3f;
		constexpr float kSinC6  = -1.9842604524455964565277099609375e-4f;
		constexpr float kSinC8  =  2.760012648650445044040679931640625e-6f;
		constexpr float kSinC10 = -2.50293279435709337121807038784027099609375e-8f;

		constexpr float kCosC2  = -0.5f;
		constexpr float kCosC4  =  4.166664183139801025390625e-2f;
		constexpr float kCosC6  = -1.388833043165504932403564453125e-3f;
		constexpr float kCosC8  =  2.47562347794882953166961669921875e-5f;
		constexpr float kCosC10 = -2.59630184018533327616751194000244140625e-7f;

		float c0  = xx;
		float c2  = kSinC2;
		float c4  = kSinC4;
		float c6  = kSinC6;
		float c8  = kSinC8;
		float c10 = kSinC10;

		if (bits == 0
		||  bits == 2)
		{
			c0  = 1.0f;
			c2  = kCosC2;
			c4  = kCosC4;
			c6  = kCosC6;
			c8  = kCosC8;
			c10 = kCosC10;
		}

		const float xsq    = square(xx);
		const float tmp0   = mad(c10,  xsq, c8 );
		const float tmp1   = mad(tmp0, xsq, c6 );
		const float tmp2   = mad(tmp1, xsq, c4 );
		const float tmp3   = mad(tmp2, xsq, c2 );
		const float tmp4   = mad(tmp3, xsq, 1.0);
		const float result = tmp4 * c0;

		return bits == 1 || bits == 2
			? -result
			:  result
			;
	}

	inline BX_CONSTEXPR_FUNC float acos(float _a)
	{
		constexpr float kAcosC0 =  1.5707288f;
		constexpr float kAcosC1 = -0.2121144f;
		constexpr float kAcosC2 =  0.0742610f;
		constexpr float kAcosC3 = -0.0187293f;

		const float absa   = abs(_a);
		const float tmp0   = mad(kAcosC3, absa, kAcosC2);
		const float tmp1   = mad(tmp0,    absa, kAcosC1);
		const float tmp2   = mad(tmp1,    absa, kAcosC0);
		const float tmp3   = tmp2 * sqrt(1.0f - absa);
		const float negate = float(_a < 0.0f);
		const float tmp4   = tmp3 - 2.0f*negate*tmp3;
		const float result = negate*kPi + tmp4;

		return result;
	}

	inline void sinCosApprox(float& _outSinApprox, float& _outCos, float _a)
	{
		const float aa     = _a - floor(_a*kInvPi2)*kPi2;
		const float absA   = abs(aa);
		const float cosA   = cos(absA);
		const float cosASq = square(cosA);
		const float tmp0   = sqrt(1.0f - cosASq);
		const float tmp1   = aa > 0.0f && aa < kPi ? 1.0f : -1.0f;
		const float sinA   = mul(tmp0, tmp1);

		_outSinApprox = sinA;
		_outCos = cosA;
	}

	inline BX_CONSTEXPR_FUNC float sin(float _a)
	{
		return cos(_a - kPiHalf);
	}

	inline BX_CONSTEXPR_FUNC float sinh(float _a)
	{
		return 0.5f*(exp(_a) - exp(-_a) );
	}

	inline BX_CONSTEXPR_FUNC float asin(float _a)
	{
		return kPiHalf - acos(_a);
	}

	inline BX_CONSTEXPR_FUNC float cosh(float _a)
	{
		return 0.5f*(exp(_a) + exp(-_a) );
	}

	inline BX_CONSTEXPR_FUNC float tan(float _a)
	{
		return sin(_a) / cos(_a);
	}

	inline BX_CONSTEXPR_FUNC float tanh(float _a)
	{
		const float tmp0   = exp(2.0f*_a);
		const float tmp1   = tmp0 - 1.0f;
		const float tmp2   = tmp0 + 1.0f;
		const float result = tmp1 / tmp2;

		return result;
	}

	inline BX_CONSTEXPR_FUNC float atan(float _a)
	{
		return atan2(_a, 1.0f);
	}

	inline BX_CONSTEXPR_FUNC float atan2(float _y, float _x)
	{
		const float ax     = abs(_x);
		const float ay     = abs(_y);
		const float maxaxy = max(ax, ay);
		const float minaxy = min(ax, ay);

		if (maxaxy == 0.0f)
		{
			return _y < 0.0f ? -0.0f : 0.0f;
		}

		constexpr float kAtan2C0 = -0.013480470f;
		constexpr float kAtan2C1 =  0.057477314f;
		constexpr float kAtan2C2 = -0.121239071f;
		constexpr float kAtan2C3 =  0.195635925f;
		constexpr float kAtan2C4 = -0.332994597f;
		constexpr float kAtan2C5 =  0.999995630f;

		const float mxy    = minaxy / maxaxy;
		const float mxysq  = square(mxy);
		const float tmp0   = mad(kAtan2C0, mxysq, kAtan2C1);
		const float tmp1   = mad(tmp0,     mxysq, kAtan2C2);
		const float tmp2   = mad(tmp1,     mxysq, kAtan2C3);
		const float tmp3   = mad(tmp2,     mxysq, kAtan2C4);
		const float tmp4   = mad(tmp3,     mxysq, kAtan2C5);
		const float tmp5   = tmp4 * mxy;
		const float tmp6   = ay > ax   ? kPiHalf - tmp5 : tmp5;
		const float tmp7   = _x < 0.0f ? kPi     - tmp6 : tmp6;
		const float result = _y < 0.0f ? -tmp7 : tmp7;

		return result;
	}

	inline BX_CONSTEXPR_FUNC float frexp(float _a, int32_t* _outExp)
	{
		const uint32_t ftob     = floatToBits(_a);
		const uint32_t masked0  = uint32_and(ftob, kFloatExponentMask);
		const uint32_t exp0     = uint32_srl(masked0, kFloatExponentBitShift);
		const uint32_t masked1  = uint32_and(ftob,   kFloatSignMask | kFloatMantissaMask);
		const uint32_t bits     = uint32_or(masked1, UINT32_C(0x3f000000) );
		const float    result   = bitsToFloat(bits);

		*_outExp = int32_t(exp0 - 0x7e);

		return result;
	}

	inline BX_CONSTEXPR_FUNC float ldexp(float _a, int32_t _b)
	{
		const uint32_t ftob     = floatToBits(_a);
		const uint32_t masked   = uint32_and(ftob, kFloatSignMask | kFloatExponentMask);
		const uint32_t expsign0 = uint32_sra(masked, kFloatExponentBitShift);
		const uint32_t tmp      = uint32_iadd(expsign0, _b);
		const uint32_t expsign1 = uint32_sll(tmp, kFloatExponentBitShift);
		const uint32_t mantissa = uint32_and(ftob, kFloatMantissaMask);
		const uint32_t bits     = uint32_or(mantissa, expsign1);
		const float    result   = bitsToFloat(bits);

		return result;
	}

	inline BX_CONSTEXPR_FUNC float exp(float _a)
	{
		if (abs(_a) <= kNearZero)
		{
			return _a + 1.0f;
		}

		constexpr float kExpC0  =  1.66666666666666019037e-01f;
		constexpr float kExpC1  = -2.77777777770155933842e-03f;
		constexpr float kExpC2  =  6.61375632143793436117e-05f;
		constexpr float kExpC3  = -1.65339022054652515390e-06f;
		constexpr float kExpC4  =  4.13813679705723846039e-08f;
		constexpr float kLogNat2Lo = 1.90821492927058770002e-10f;

		const float kk     = round(_a*kInvLogNat2);
		const float hi     = _a - kk*kLogNat2;
		const float lo     =      kk*kLogNat2Lo;
		const float hml    = hi - lo;
		const float hmlsq  = square(hml);
		const float tmp0   = mad(kExpC4, hmlsq, kExpC3);
		const float tmp1   = mad(tmp0,   hmlsq, kExpC2);
		const float tmp2   = mad(tmp1,   hmlsq, kExpC1);
		const float tmp3   = mad(tmp2,   hmlsq, kExpC0);
		const float tmp4   = hml - hmlsq * tmp3;
		const float tmp5   = hml*tmp4/(2.0f-tmp4);
		const float tmp6   = 1.0f - ( (lo - tmp5) - hi);
		const float result = ldexp(tmp6, int32_t(kk) );

		return result;
	}

	inline BX_CONSTEXPR_FUNC float log(float _a)
	{
		int32_t exp = 0;
		float ff = frexp(_a, &exp);

		if (ff < kSqrt2*0.5f)
		{
			ff *= 2.0f;
			--exp;
		}

		constexpr float kLogC0 = 6.666666666666735130e-01f;
		constexpr float kLogC1 = 3.999999999940941908e-01f;
		constexpr float kLogC2 = 2.857142874366239149e-01f;
		constexpr float kLogC3 = 2.222219843214978396e-01f;
		constexpr float kLogC4 = 1.818357216161805012e-01f;
		constexpr float kLogC5 = 1.531383769920937332e-01f;
		constexpr float kLogC6 = 1.479819860511658591e-01f;
		constexpr float kLogNat2Lo = 1.90821492927058770002e-10f;

		ff -= 1.0f;
		const float kk     = float(exp);
		const float hi     = kk*kLogNat2;
		const float lo     = kk*kLogNat2Lo;
		const float ss     = ff / (2.0f + ff);
		const float s2     = square(ss);
		const float s4     = square(s2);

		const float tmp0   = mad(kLogC6, s4, kLogC4);
		const float tmp1   = mad(tmp0,   s4, kLogC2);
		const float tmp2   = mad(tmp1,   s4, kLogC0);
		const float t1     = s2*tmp2;

		const float tmp3   = mad(kLogC5, s4, kLogC3);
		const float tmp4   = mad(tmp3,   s4, kLogC1);
		const float t2     = s4*tmp4;

		const float t12    = t1 + t2;
		const float hfsq   = 0.5f*square(ff);
		const float result = hi - ( (hfsq - (ss*(hfsq+t12) + lo) ) - ff);

		return result;
	}

	inline BX_CONSTEXPR_FUNC float pow(float _a, float _b)
	{
		if (abs(_b) < kFloatSmallest)
		{
			return 1.0f;
		}

		if (abs(_a) < kFloatSmallest)
		{
			return 0.0f;
		}

		return copySign(exp(_b * log(abs(_a) ) ), _a);
	}

	inline BX_CONSTEXPR_FUNC float exp2(float _a)
	{
		return pow(2.0f, _a);
	}

	inline BX_CONSTEXPR_FUNC float log2(float _a)
	{
		return log(_a) * kInvLogNat2;
	}

	template<>
	inline BX_CONSTEXPR_FUNC uint8_t countBits(uint32_t _val)
	{
#if BX_COMPILER_GCC || BX_COMPILER_CLANG
		return __builtin_popcount(_val);
#else
		const uint32_t tmp0   = uint32_srl(_val, 1);
		const uint32_t tmp1   = uint32_and(tmp0, 0x55555555);
		const uint32_t tmp2   = uint32_sub(_val, tmp1);
		const uint32_t tmp3   = uint32_and(tmp2, 0xc30c30c3);
		const uint32_t tmp4   = uint32_srl(tmp2, 2);
		const uint32_t tmp5   = uint32_and(tmp4, 0xc30c30c3);
		const uint32_t tmp6   = uint32_srl(tmp2, 4);
		const uint32_t tmp7   = uint32_and(tmp6, 0xc30c30c3);
		const uint32_t tmp8   = uint32_add(tmp3, tmp5);
		const uint32_t tmp9   = uint32_add(tmp7, tmp8);
		const uint32_t tmpA   = uint32_srl(tmp9, 6);
		const uint32_t tmpB   = uint32_add(tmp9, tmpA);
		const uint32_t tmpC   = uint32_srl(tmpB, 12);
		const uint32_t tmpD   = uint32_srl(tmpB, 24);
		const uint32_t tmpE   = uint32_add(tmpB, tmpC);
		const uint32_t tmpF   = uint32_add(tmpD, tmpE);
		const uint32_t result = uint32_and(tmpF, 0x3f);

		return uint8_t(result);
#endif // BX_COMPILER_*
	}

	template<>
	inline BX_CONSTEXPR_FUNC uint8_t countBits(unsigned long long _val)
	{
#if BX_COMPILER_GCC || BX_COMPILER_CLANG
		return __builtin_popcountll(_val);
#else
		const uint32_t lo = uint32_t(_val&UINT32_MAX);
		const uint32_t hi = uint32_t(_val>>32);

		return countBits<uint32_t>(lo)
			+  countBits<uint32_t>(hi)
			;
#endif // BX_COMPILER_*
	}

	template<>
	inline BX_CONSTEXPR_FUNC uint8_t countBits(unsigned long _val)
	{
		return countBits<unsigned long long>(_val);
	}

	template<> inline BX_CONSTEXPR_FUNC uint8_t countBits(uint8_t  _val) { return countBits<uint32_t>(_val); }
	template<> inline BX_CONSTEXPR_FUNC uint8_t countBits(int8_t   _val) { return countBits<uint8_t >(_val); }
	template<> inline BX_CONSTEXPR_FUNC uint8_t countBits(uint16_t _val) { return countBits<uint32_t>(_val); }
	template<> inline BX_CONSTEXPR_FUNC uint8_t countBits(int16_t  _val) { return countBits<uint16_t>(_val); }
	template<> inline BX_CONSTEXPR_FUNC uint8_t countBits(int32_t  _val) { return countBits<uint32_t>(_val); }
	template<> inline BX_CONSTEXPR_FUNC uint8_t countBits(int64_t  _val) { return countBits<uint64_t>(_val); }

	template<>
	inline BX_CONSTEXPR_FUNC uint8_t countLeadingZeros(uint32_t _val)
	{
#if BX_COMPILER_GCC || BX_COMPILER_CLANG
		return 0 == _val ? 32 : __builtin_clz(_val);
#else
		const uint32_t tmp0   = uint32_srl(_val, 1);
		const uint32_t tmp1   = uint32_or(tmp0, _val);
		const uint32_t tmp2   = uint32_srl(tmp1, 2);
		const uint32_t tmp3   = uint32_or(tmp2, tmp1);
		const uint32_t tmp4   = uint32_srl(tmp3, 4);
		const uint32_t tmp5   = uint32_or(tmp4, tmp3);
		const uint32_t tmp6   = uint32_srl(tmp5, 8);
		const uint32_t tmp7   = uint32_or(tmp6, tmp5);
		const uint32_t tmp8   = uint32_srl(tmp7, 16);
		const uint32_t tmp9   = uint32_or(tmp8, tmp7);
		const uint32_t tmpA   = uint32_not(tmp9);
		const uint32_t result = uint32_cntbits(tmpA);

		return uint8_t(result);
#endif // BX_COMPILER_*
	}

	template<>
	inline BX_CONSTEXPR_FUNC uint8_t countLeadingZeros(unsigned long long _val)
	{
#if BX_COMPILER_GCC || BX_COMPILER_CLANG
		return 0 == _val ? 64 : __builtin_clzll(_val);
#else
		return _val & UINT64_C(0xffffffff00000000)
			 ? countLeadingZeros<uint32_t>(uint32_t(_val>>32) )
			 : countLeadingZeros<uint32_t>(uint32_t(_val) ) + 32
			 ;
#endif // BX_COMPILER_*
	}

	template<>
	inline BX_CONSTEXPR_FUNC uint8_t countLeadingZeros(unsigned long _val)
	{
		return countLeadingZeros<unsigned long long>(_val);
	}

	template<> inline BX_CONSTEXPR_FUNC uint8_t countLeadingZeros(uint8_t  _val) { return countLeadingZeros<uint32_t>(_val)-24; }
	template<> inline BX_CONSTEXPR_FUNC uint8_t countLeadingZeros(int8_t   _val) { return countLeadingZeros<uint8_t >(_val);    }
	template<> inline BX_CONSTEXPR_FUNC uint8_t countLeadingZeros(uint16_t _val) { return countLeadingZeros<uint32_t>(_val)-16; }
	template<> inline BX_CONSTEXPR_FUNC uint8_t countLeadingZeros(int16_t  _val) { return countLeadingZeros<uint16_t>(_val);    }
	template<> inline BX_CONSTEXPR_FUNC uint8_t countLeadingZeros(int32_t  _val) { return countLeadingZeros<uint32_t>(_val);    }
	template<> inline BX_CONSTEXPR_FUNC uint8_t countLeadingZeros(int64_t  _val) { return countLeadingZeros<uint64_t>(_val);    }

	template<>
	inline BX_CONSTEXPR_FUNC uint8_t countTrailingZeros(uint32_t _val)
	{
#if BX_COMPILER_GCC || BX_COMPILER_CLANG
		return 0 == _val ? 32 : __builtin_ctz(_val);
#else
		const uint32_t tmp0   = uint32_not(_val);
		const uint32_t tmp1   = uint32_dec(_val);
		const uint32_t tmp2   = uint32_and(tmp0, tmp1);
		const uint32_t result = uint32_cntbits(tmp2);

		return uint8_t(result);
#endif // BX_COMPILER_*
	}

	template<>
	inline BX_CONSTEXPR_FUNC uint8_t countTrailingZeros(unsigned long long _val)
	{
#if BX_COMPILER_GCC || BX_COMPILER_CLANG
		return 0 == _val ? 64 : __builtin_ctzll(_val);
#else
		return _val & UINT64_C(0xffffffff)
			? countTrailingZeros<uint32_t>(uint32_t(_val) )
			: countTrailingZeros<uint32_t>(uint32_t(_val>>32) ) + 32
			;
#endif // BX_COMPILER_*
	}

	template<>
	inline BX_CONSTEXPR_FUNC uint8_t countTrailingZeros(unsigned long _val)
	{
		return countTrailingZeros<unsigned long long>(_val);
	}

	template<> inline BX_CONSTEXPR_FUNC uint8_t countTrailingZeros(uint8_t  _val) { return min<uint8_t>(8,  countTrailingZeros<uint32_t>(_val) ); }
	template<> inline BX_CONSTEXPR_FUNC uint8_t countTrailingZeros(int8_t   _val) { return                  countTrailingZeros<uint8_t >(_val);   }
	template<> inline BX_CONSTEXPR_FUNC uint8_t countTrailingZeros(uint16_t _val) { return min<uint8_t>(16, countTrailingZeros<uint32_t>(_val) ); }
	template<> inline BX_CONSTEXPR_FUNC uint8_t countTrailingZeros(int16_t  _val) { return                  countTrailingZeros<uint16_t>(_val);   }
	template<> inline BX_CONSTEXPR_FUNC uint8_t countTrailingZeros(int32_t  _val) { return                  countTrailingZeros<uint32_t>(_val);   }
	template<> inline BX_CONSTEXPR_FUNC uint8_t countTrailingZeros(int64_t  _val) { return                  countTrailingZeros<uint64_t>(_val);   }

	template<typename Ty>
	inline BX_CONSTEXPR_FUNC uint8_t findFirstSet(Ty _val)
	{
		BX_STATIC_ASSERT(isInteger<Ty>(), "Type Ty must be of integer type!");
		return Ty(0) == _val ? uint8_t(0) : countTrailingZeros<Ty>(_val) + 1;
	}

	template<typename Ty>
	inline BX_CONSTEXPR_FUNC uint8_t findLastSet(Ty _val)
	{
		BX_STATIC_ASSERT(isInteger<Ty>(), "Type Ty must be of integer type!");
		return Ty(0) == _val ? uint8_t(0) : sizeof(Ty)*8 - countLeadingZeros<Ty>(_val);
	}

	template<typename Ty>
	inline BX_CONSTEXPR_FUNC uint8_t ceilLog2(Ty _a)
	{
		BX_STATIC_ASSERT(isInteger<Ty>(), "Type Ty must be of integer type!");
		return Ty(_a) < Ty(1) ? Ty(0) : sizeof(Ty)*8 - countLeadingZeros<Ty>(_a - 1);
	}

	template<typename Ty>
	inline BX_CONSTEXPR_FUNC uint8_t floorLog2(Ty _a)
	{
		BX_STATIC_ASSERT(isInteger<Ty>(), "Type Ty must be of integer type!");
		return Ty(_a) < Ty(1) ? Ty(0) : sizeof(Ty)*8 - 1 - countLeadingZeros<Ty>(_a);
	}

	template<typename Ty>
	inline BX_CONSTEXPR_FUNC Ty nextPow2(Ty _a)
	{
		const uint8_t log2 = ceilLog2(_a);
		BX_ASSERT(log2 < sizeof(Ty)*8
			, "Type Ty cannot represent the next power-of-two value (1<<%u is larger than %u-bit type)."
			, log2
			, sizeof(Ty)*8
			);
		return Ty(1)<<log2;
	}

	inline BX_CONSTEXPR_FUNC float rsqrtRef(float _a)
	{
		if (_a < kFloatSmallest)
		{
			return kFloatInfinity;
		}

		return pow(_a, -0.5f);
	}

	inline BX_CONST_FUNC float rsqrtSimd(float _a)
	{
		if (_a < kFloatSmallest)
		{
			return kFloatInfinity;
		}

		const simd128_t aa = simd_splat(_a);
#if BX_SIMD_NEON
		const simd128_t rsqrta = simd_rsqrt_nr(aa);
#else
		const simd128_t rsqrta = simd_rsqrt_ni(aa);
#endif // BX_SIMD_NEON

		float result = 0.0f;
		simd_stx(&result, rsqrta);

		return result;
	}

	inline BX_CONSTEXPR_FUNC float sqrtRef(float _a)
	{
		if (_a < 0.0f)
		{
			return bitsToFloat(kFloatExponentMask | kFloatMantissaMask);
		}

		return _a * pow(_a, -0.5f);
	}

	inline BX_CONST_FUNC float sqrtSimd(float _a)
	{
		if (_a < 0.0f)
		{
			return bitsToFloat(kFloatExponentMask | kFloatMantissaMask);
		}
		else if (_a < kFloatSmallest)
		{
			return 0.0f;
		}

		const simd128_t aa   = simd_splat(_a);
		const simd128_t sqrt = simd_sqrt(aa);

		float result = 0.0f;
		simd_stx(&result, sqrt);

		return result;
	}

	inline BX_CONSTEXPR_FUNC float rsqrt(float _a)
	{
#if BX_SIMD_SUPPORTED
		if (isConstantEvaluated() )
		{
			return rsqrtRef(_a);
		}

		return rsqrtSimd(_a);
#else
		return rsqrtRef(_a);
#endif // BX_SIMD_SUPPORTED
	}

	inline BX_CONSTEXPR_FUNC float sqrt(float _a)
	{
#if BX_SIMD_SUPPORTED
		if (isConstantEvaluated() )
		{
			return sqrtRef(_a);
		}

		return sqrtSimd(_a);
#else
		return sqrtRef(_a);
#endif // BX_SIMD_SUPPORTED
	}

	inline BX_CONSTEXPR_FUNC float trunc(float _a)
	{
		return float(int(_a) );
	}

	inline BX_CONSTEXPR_FUNC float fract(float _a)
	{
		return _a - trunc(_a);
	}

	inline BX_CONSTEXPR_FUNC float nms(float _a, float _b, float _c)
	{
		return _c - _a * _b;
	}

	inline BX_CONSTEXPR_FUNC float add(float _a, float _b)
	{
		return _a + _b;
	}

	inline BX_CONSTEXPR_FUNC float sub(float _a, float _b)
	{
		return _a - _b;
	}

	inline BX_CONSTEXPR_FUNC float mul(float _a, float _b)
	{
		return _a * _b;
	}

	inline BX_CONSTEXPR_FUNC float mad(float _a, float _b, float _c)
	{
		return add(mul(_a, _b), _c);
	}

	inline BX_CONSTEXPR_FUNC float rcp(float _a)
	{
		return 1.0f / _a;
	}

	inline BX_CONSTEXPR_FUNC float rcpSafe(float _a)
	{
		return rcp(copySign(max(kFloatSmallest, abs(_a) ), _a) );
	}

	inline BX_CONSTEXPR_FUNC float div(float _a, float _b)
	{
		return mul(_a, rcp(_b) );
	}

	inline BX_CONSTEXPR_FUNC float divSafe(float _a, float _b)
	{
		return mul(_a, rcpSafe(_b) );
	}

	inline BX_CONSTEXPR_FUNC float ceilDiv(float _a, float _b)
	{
		return div(_a + _b - 1, _b);
	}

	inline BX_CONSTEXPR_FUNC float ceilDivSafe(float _a, float _b)
	{
		return divSafe(_a + _b - 1, _b);
	}

	inline BX_CONSTEXPR_FUNC float mod(float _a, float _b)
	{
		return _a - _b * floor(div(_a, _b) );
	}

	inline BX_CONSTEXPR_FUNC bool isEqual(float _a, float _b, float _epsilon)
	{
		// Reference(s):
		// - Floating-point tolerances revisited
		//   https://web.archive.org/web/20181103180318/http://realtimecollisiondetection.net/blog/?p=89
		//
		const float lhs = abs(_a - _b);
		const float rhs = _epsilon * max(1.0f, abs(_a), abs(_b) );
		return lhs <= rhs;
	}

	inline BX_CONST_FUNC bool isEqual(const float* _a, const float* _b, uint32_t _num, float _epsilon)
	{
		bool result = isEqual(_a[0], _b[0], _epsilon);
		for (uint32_t ii = 1; result && ii < _num; ++ii)
		{
			result = isEqual(_a[ii], _b[ii], _epsilon);
		}
		return result;
	}

	inline BX_CONSTEXPR_FUNC float wrap(float _a, float _wrap)
	{
		const float tmp0   = mod(_a, _wrap);
		const float result = tmp0 < 0.0f ? _wrap + tmp0 : tmp0;
		return result;
	}

	inline BX_CONSTEXPR_FUNC float step(float _edge, float _a)
	{
		return _a < _edge ? 0.0f : 1.0f;
	}

	inline BX_CONSTEXPR_FUNC float pulse(float _a, float _start, float _end)
	{
		return step(_a, _start) - step(_a, _end);
	}

	inline BX_CONSTEXPR_FUNC float smoothStep(float _a)
	{
		return square(_a)*(3.0f - 2.0f*_a);
	}

	inline BX_CONSTEXPR_FUNC float invSmoothStep(float _a)
	{
		return 0.5f - sin(asin(1.0f - 2.0f * _a) / 3.0f);
	}

	inline BX_CONSTEXPR_FUNC float bias(float _time, float _bias)
	{
		return _time / ( ( (1.0f/_bias - 2.0f)*(1.0f - _time) ) + 1.0f);
	}

	inline BX_CONSTEXPR_FUNC float gain(float _time, float _gain)
	{
		// Reference(s):
		// - Bias And Gain Are Your Friend
		//   https://web.archive.org/web/20181126040535/https://blog.demofox.org/2012/09/24/bias-and-gain-are-your-friend/
		//   https://web.archive.org/web/20181126040558/http://demofox.org/biasgain.html
		//
		if (_time < 0.5f)
		{
			return bias(_time * 2.0f, _gain) * 0.5f;
		}

		return bias(_time * 2.0f - 1.0f, 1.0f - _gain) * 0.5f + 0.5f;
	}

	inline BX_CONSTEXPR_FUNC float angleDiff(float _a, float _b)
	{
		const float dist = wrap(_b - _a, kPi2);
		return wrap(dist*2.0f, kPi2) - dist;
	}

	inline BX_CONSTEXPR_FUNC float angleLerp(float _a, float _b, float _t)
	{
		return _a + angleDiff(_a, _b) * _t;
	}

	template<typename Ty>
	inline Ty load(const void* _ptr)
	{
		Ty result(InitNone);
		memCopy(&result, _ptr, sizeof(Ty) );
		return result;
	}

	template<typename Ty>
	inline void store(void* _ptr, const Ty& _a)
	{
		memCopy(_ptr, &_a, sizeof(Ty) );
	}

	inline Vec3::Vec3(InitNoneTag)
	{
	}

	constexpr Vec3::Vec3(InitZeroTag)
		: x(0.0f)
		, y(0.0f)
		, z(0.0f)
	{
	}

	constexpr Vec3::Vec3(InitIdentityTag)
		: x(0.0f)
		, y(0.0f)
		, z(0.0f)
	{
	}

	constexpr Vec3::Vec3(float _v)
		: x(_v)
		, y(_v)
		, z(_v)
	{
	}

	constexpr Vec3::Vec3(float _x, float _y, float _z)
		: x(_x)
		, y(_y)
		, z(_z)
	{
	}

	inline Plane::Plane(InitNoneTag)
		: normal(InitNone)
	{
	}

	constexpr Plane::Plane(InitZeroTag)
		: normal(InitZero)
		, dist(0.0f)
	{
	}

	constexpr Plane::Plane(InitIdentityTag)
		: normal(0.0f, 1.0f, 0.0f)
		, dist(0.0f)
	{
	}

	constexpr Plane::Plane(Vec3 _normal, float _dist)
		: normal(_normal)
		, dist(_dist)
	{
	}

	inline Quaternion::Quaternion(InitNoneTag)
	{
	}

	constexpr Quaternion::Quaternion(InitZeroTag)
		: x(0.0f)
		, y(0.0f)
		, z(0.0f)
		, w(0.0f)
	{
	}

	constexpr Quaternion::Quaternion(InitIdentityTag)
		: x(0.0f)
		, y(0.0f)
		, z(0.0f)
		, w(1.0f)
	{
	}

	constexpr Quaternion::Quaternion(float _x, float _y, float _z, float _w)
		: x(_x)
		, y(_y)
		, z(_z)
		, w(_w)
	{
	}

	inline BX_CONSTEXPR_FUNC Vec3 round(const Vec3 _a)
	{
		return
		{
			round(_a.x),
			round(_a.y),
			round(_a.z),
		};
	}

	inline BX_CONSTEXPR_FUNC Vec3 abs(const Vec3 _a)
	{
		return
		{
			abs(_a.x),
			abs(_a.y),
			abs(_a.z),
		};
	}

	inline BX_CONSTEXPR_FUNC Vec3 neg(const Vec3 _a)
	{
		return
		{
			-_a.x,
			-_a.y,
			-_a.z,
		};
	}

	inline BX_CONSTEXPR_FUNC Vec3 add(const Vec3 _a, const Vec3 _b)
	{
		return
		{
			_a.x + _b.x,
			_a.y + _b.y,
			_a.z + _b.z,
		};
	}

	inline BX_CONSTEXPR_FUNC Vec3 add(const Vec3 _a, float _b)
	{
		return
		{
			_a.x + _b,
			_a.y + _b,
			_a.z + _b,
		};
	}

	inline BX_CONSTEXPR_FUNC Vec3 sub(const Vec3 _a, const Vec3 _b)
	{
		return
		{
			_a.x - _b.x,
			_a.y - _b.y,
			_a.z - _b.z,
		};
	}

	inline BX_CONSTEXPR_FUNC Vec3 sub(const Vec3 _a, float _b)
	{
		return
		{
			_a.x - _b,
			_a.y - _b,
			_a.z - _b,
		};
	}

	inline BX_CONSTEXPR_FUNC Vec3 mul(const Vec3 _a, const Vec3 _b)
	{
		return
		{
			_a.x * _b.x,
			_a.y * _b.y,
			_a.z * _b.z,
		};
	}

	inline BX_CONSTEXPR_FUNC Vec3 mul(const Vec3 _a, float _b)
	{
		return
		{
			_a.x * _b,
			_a.y * _b,
			_a.z * _b,
		};
	}

	inline BX_CONSTEXPR_FUNC Vec3 div(const Vec3 _a, const Vec3 _b)
	{
		return mul(_a, rcp(_b) );
	}

	inline BX_CONSTEXPR_FUNC Vec3 divSafe(const Vec3 _a, const Vec3 _b)
	{
		return mul(_a, rcpSafe(_b) );
	}

	inline BX_CONSTEXPR_FUNC Vec3 div(const Vec3 _a, float _b)
	{
		return mul(_a, rcp(_b) );
	}

	inline BX_CONSTEXPR_FUNC Vec3 divSafe(const Vec3 _a, float _b)
	{
		return mul(_a, rcpSafe(_b) );
	}

	inline BX_CONSTEXPR_FUNC Vec3 nms(const Vec3 _a, const float _b, const Vec3 _c)
	{
		return sub(_c, mul(_a, _b) );
	}

	inline BX_CONSTEXPR_FUNC Vec3 nms(const Vec3 _a, const Vec3 _b, const Vec3 _c)
	{
		return sub(_c, mul(_a, _b) );
	}

	inline BX_CONSTEXPR_FUNC Vec3 mad(const Vec3 _a, const float _b, const Vec3 _c)
	{
		return add(mul(_a, _b), _c);
	}

	inline BX_CONSTEXPR_FUNC Vec3 mad(const Vec3 _a, const Vec3 _b, const Vec3 _c)
	{
		return add(mul(_a, _b), _c);
	}

	inline BX_CONSTEXPR_FUNC float dot(const Vec3 _a, const Vec3 _b)
	{
		return _a.x*_b.x + _a.y*_b.y + _a.z*_b.z;
	}

	inline BX_CONSTEXPR_FUNC Vec3 cross(const Vec3 _a, const Vec3 _b)
	{
		return
		{
			_a.y*_b.z - _a.z*_b.y,
			_a.z*_b.x - _a.x*_b.z,
			_a.x*_b.y - _a.y*_b.x,
		};
	}

	inline BX_CONSTEXPR_FUNC float length(const Vec3 _a)
	{
		return sqrt(dot(_a, _a) );
	}

	inline BX_CONSTEXPR_FUNC float distanceSq(const Vec3 _a, const Vec3 _b)
	{
		const Vec3 ba = sub(_b, _a);
		return dot(ba, ba);
	}

	inline BX_CONSTEXPR_FUNC float distance(const Vec3 _a, const Vec3 _b)
	{
		return length(sub(_b, _a) );
	}

	inline BX_CONSTEXPR_FUNC Vec3 lerp(const Vec3 _a, const Vec3 _b, float _t)
	{
		return
		{
			lerp(_a.x, _b.x, _t),
			lerp(_a.y, _b.y, _t),
			lerp(_a.z, _b.z, _t),
		};
	}

	inline BX_CONSTEXPR_FUNC Vec3 lerp(const Vec3 _a, const Vec3 _b, const Vec3 _t)
	{
		return
		{
			lerp(_a.x, _b.x, _t.x),
			lerp(_a.y, _b.y, _t.y),
			lerp(_a.z, _b.z, _t.z),
		};
	}

	inline BX_CONSTEXPR_FUNC Vec3 normalize(const Vec3 _a)
	{
		const float len   = length(_a);
		const Vec3 result = divSafe(_a, len);
		return result;
	}

	inline BX_CONSTEXPR_FUNC Vec3 min(const Vec3 _a, const Vec3 _b)
	{
		return
		{
			min(_a.x, _b.x),
			min(_a.y, _b.y),
			min(_a.z, _b.z),
		};
	}

	inline BX_CONSTEXPR_FUNC Vec3 max(const Vec3 _a, const Vec3 _b)
	{
		return
		{
			max(_a.x, _b.x),
			max(_a.y, _b.y),
			max(_a.z, _b.z),
		};
	}

	inline BX_CONSTEXPR_FUNC Vec3 rcp(const Vec3 _a)
	{
		return
		{
			rcp(_a.x),
			rcp(_a.y),
			rcp(_a.z),
		};
	}

	inline BX_CONSTEXPR_FUNC Vec3 rcpSafe(const Vec3 _a)
	{
		return
		{
			rcpSafe(_a.x),
			rcpSafe(_a.y),
			rcpSafe(_a.z),
		};
	}

	inline BX_CONSTEXPR_FUNC bool isEqual(const Vec3 _a, const Vec3 _b, float _epsilon)
	{
		return isEqual(_a.x, _b.x, _epsilon)
			&& isEqual(_a.y, _b.y, _epsilon)
			&& isEqual(_a.z, _b.z, _epsilon)
			;
	}

	inline void calcTangentFrame(Vec3& _outT, Vec3& _outB, const Vec3 _n)
	{
		const float nx = _n.x;
		const float ny = _n.y;
		const float nz = _n.z;

		if (abs(nx) > abs(nz) )
		{
			const float invLen = rcpSafe(sqrt(nx*nx + nz*nz) );
			_outT.x = -nz * invLen;
			_outT.y =  0.0f;
			_outT.z =  nx * invLen;
		}
		else
		{
			const float invLen = rcpSafe(sqrt(ny*ny + nz*nz) );
			_outT.x =  0.0f;
			_outT.y =  nz * invLen;
			_outT.z = -ny * invLen;
		}

		_outB = cross(_n, _outT);
	}

	inline void calcTangentFrame(Vec3& _outT, Vec3& _outB, const Vec3 _n, float _angle)
	{
		calcTangentFrame(_outT, _outB, _n);

		const float sa = sin(_angle);
		const float ca = cos(_angle);

		_outT.x = -sa * _outB.x + ca * _outT.x;
		_outT.y = -sa * _outB.y + ca * _outT.y;
		_outT.z = -sa * _outB.z + ca * _outT.z;

		_outB = cross(_n, _outT);
	}

	inline BX_CONSTEXPR_FUNC Vec3 fromLatLong(float _u, float _v)
	{
		const float phi   = _u * kPi2;
		const float theta = _v * kPi;

		const float st = sin(theta);
		const float sp = sin(phi);
		const float ct = cos(theta);
		const float cp = cos(phi);

		return
		{
			-st*sp,
			 ct,
			-st*cp,
		};
	}

	inline void toLatLong(float* _outU, float* _outV, const Vec3 _dir)
	{
		const float phi   = atan2(_dir.x, _dir.z);
		const float theta = acos(_dir.y);

		*_outU = (kPi + phi)/kPi2;
		*_outV = theta*kInvPi;
	}

	inline BX_CONSTEXPR_FUNC Quaternion invert(const Quaternion _a)
	{
		return
		{
			-_a.x,
			-_a.y,
			-_a.z,
			 _a.w,
		};
	}

	inline BX_CONSTEXPR_FUNC Vec3 mulXyz(const Quaternion _a, const Quaternion _b)
	{
		const float ax = _a.x;
		const float ay = _a.y;
		const float az = _a.z;
		const float aw = _a.w;

		const float bx = _b.x;
		const float by = _b.y;
		const float bz = _b.z;
		const float bw = _b.w;

		return
		{
			aw * bx + ax * bw + ay * bz - az * by,
			aw * by - ax * bz + ay * bw + az * bx,
			aw * bz + ax * by - ay * bx + az * bw,
		};
	}

	inline BX_CONSTEXPR_FUNC Quaternion add(const Quaternion _a, const Quaternion _b)
	{
		return
		{
			_a.x + _b.x,
			_a.y + _b.y,
			_a.z + _b.z,
			_a.w + _b.w,
		};
	}

	inline BX_CONSTEXPR_FUNC Quaternion sub(const Quaternion _a, const Quaternion _b)
	{
		return
		{
			_a.x - _b.x,
			_a.y - _b.y,
			_a.z - _b.z,
			_a.w - _b.w,
		};
	}

	inline BX_CONSTEXPR_FUNC Quaternion mul(const Quaternion _a, float _b)
	{
		return
		{
			_a.x * _b,
			_a.y * _b,
			_a.z * _b,
			_a.w * _b,
		};
	}

	inline BX_CONSTEXPR_FUNC Quaternion mul(const Quaternion _a, const Quaternion _b)
	{
		const float ax = _a.x;
		const float ay = _a.y;
		const float az = _a.z;
		const float aw = _a.w;

		const float bx = _b.x;
		const float by = _b.y;
		const float bz = _b.z;
		const float bw = _b.w;

		return
		{
			aw * bx + ax * bw + ay * bz - az * by,
			aw * by - ax * bz + ay * bw + az * bx,
			aw * bz + ax * by - ay * bx + az * bw,
			aw * bw - ax * bx - ay * by - az * bz,
		};
	}

	inline BX_CONSTEXPR_FUNC Vec3 mul(const Vec3 _v, const Quaternion _q)
	{
		const Quaternion tmp0 = invert(_q);
		const Quaternion qv   = { _v.x, _v.y, _v.z, 0.0f };
		const Quaternion tmp1 = mul(tmp0, qv);
		const Vec3 result     = mulXyz(tmp1, _q);

		return result;
	}

	inline BX_CONSTEXPR_FUNC float dot(const Quaternion _a, const Quaternion _b)
	{
		return
			  _a.x * _b.x
			+ _a.y * _b.y
			+ _a.z * _b.z
			+ _a.w * _b.w
			;
	}

	inline BX_CONSTEXPR_FUNC Quaternion normalize(const Quaternion _a)
	{
		const float norm = dot(_a, _a);
		if (0.0f < norm)
		{
			const float invNorm = rsqrt(norm);

			return mul(_a, invNorm);
		}

		return
		{
			0.0f,
			0.0f,
			0.0f,
			1.0f,
		};
	}

	inline BX_CONSTEXPR_FUNC Quaternion lerp(const Quaternion _a, const Quaternion _b, float _t)
	{
		const float sa    = 1.0f - _t;
		const float adotb = dot(_a, _b);
		const float sb    = sign(adotb) * _t;

		const Quaternion aa = mul(_a, sa);
		const Quaternion bb = mul(_b, sb);
		const Quaternion qq = add(aa, bb);

		return normalize(qq);
	}

	inline BX_CONST_FUNC Quaternion fromEuler(const Vec3 _euler)
	{
		const float sx = sin(_euler.x * 0.5f);
		const float cx = cos(_euler.x * 0.5f);
		const float sy = sin(_euler.y * 0.5f);
		const float cy = cos(_euler.y * 0.5f);
		const float sz = sin(_euler.z * 0.5f);
		const float cz = cos(_euler.z * 0.5f);

		return
		{
			sx * cy * cz - cx * sy * sz,
			cx * sy * cz + sx * cy * sz,
			cx * cy * sz - sx * sy * cz,
			cx * cy * cz + sx * sy * sz,
		};
	}

	inline BX_CONST_FUNC Vec3 toEuler(const Quaternion _a)
	{
		const float xx  = _a.x;
		const float yy  = _a.y;
		const float zz  = _a.z;
		const float ww  = _a.w;
		const float xsq = square(xx);
		const float ysq = square(yy);
		const float zsq = square(zz);

		return
		{
			atan2(2.0f * (xx * ww - yy * zz), 1.0f - 2.0f * (xsq + zsq) ),
			atan2(2.0f * (yy * ww + xx * zz), 1.0f - 2.0f * (ysq + zsq) ),
			asin( 2.0f * (xx * yy + zz * ww) ),
		};
	}

	inline BX_CONST_FUNC Vec3 toXAxis(const Quaternion _a)
	{
		const float xx  = _a.x;
		const float yy  = _a.y;
		const float zz  = _a.z;
		const float ww  = _a.w;
		const float ysq = square(yy);
		const float zsq = square(zz);

		return
		{
			1.0f - 2.0f * ysq     - 2.0f * zsq,
			       2.0f * xx * yy + 2.0f * zz * ww,
			       2.0f * xx * zz - 2.0f * yy * ww,
		};
	}

	inline BX_CONST_FUNC Vec3 toYAxis(const Quaternion _a)
	{
		const float xx  = _a.x;
		const float yy  = _a.y;
		const float zz  = _a.z;
		const float ww  = _a.w;
		const float xsq = square(xx);
		const float zsq = square(zz);

		return
		{
			       2.0f * xx * yy - 2.0f * zz * ww,
			1.0f - 2.0f * xsq     - 2.0f * zsq,
			       2.0f * yy * zz + 2.0f * xx * ww,
		};
	}

	inline BX_CONST_FUNC Vec3 toZAxis(const Quaternion _a)
	{
		const float xx  = _a.x;
		const float yy  = _a.y;
		const float zz  = _a.z;
		const float ww  = _a.w;
		const float xsq = square(xx);
		const float ysq = square(yy);

		return
		{
			       2.0f * xx * zz + 2.0f * yy * ww,
			       2.0f * yy * zz - 2.0f * xx * ww,
			1.0f - 2.0f * xsq     - 2.0f * ysq,
		};
	}

	inline BX_CONST_FUNC Quaternion fromAxisAngle(const Vec3 _axis, float _angle)
	{
		const float ha = _angle * 0.5f;
		const float sa = sin(ha);

		return
		{
			_axis.x * sa,
			_axis.y * sa,
			_axis.z * sa,
			cos(ha),
		};
	}

	inline void toAxisAngle(Vec3& _outAxis, float& _outAngle, const Quaternion _a)
	{
		const float ww = _a.w;
		const float sa = sqrt(1.0f - square(ww) );

		_outAngle = 2.0f * acos(ww);

		if (0.001f > sa)
		{
			_outAxis = { _a.x, _a.y, _a.z };
			return;
		}

		const float invSa = rcpSafe(sa);

		_outAxis = { _a.x * invSa, _a.y * invSa, _a.z * invSa };
	}

	inline BX_CONST_FUNC Quaternion rotateX(float _ax)
	{
		const float hx = _ax * 0.5f;

		return
		{
			sin(hx),
			0.0f,
			0.0f,
			cos(hx),
		};
	}

	inline BX_CONST_FUNC Quaternion rotateY(float _ay)
	{
		const float hy = _ay * 0.5f;

		return
		{
			0.0f,
			sin(hy),
			0.0f,
			cos(hy),
		};
	}

	inline BX_CONST_FUNC Quaternion rotateZ(float _az)
	{
		const float hz = _az * 0.5f;

		return
		{
			0.0f,
			0.0f,
			sin(hz),
			cos(hz),
		};
	}

	inline BX_CONSTEXPR_FUNC bool isEqual(const Quaternion _a, const Quaternion _b, float _epsilon)
	{
		return isEqual(_a.x, _b.x, _epsilon)
			&& isEqual(_a.y, _b.y, _epsilon)
			&& isEqual(_a.z, _b.z, _epsilon)
			&& isEqual(_a.w, _b.w, _epsilon)
			;
	}

	inline void mtxIdentity(float* _result)
	{
		memSet(_result, 0, sizeof(float)*16);
		_result[0] = _result[5] = _result[10] = _result[15] = 1.0f;
	}

	inline void mtxTranslate(float* _result, float _tx, float _ty, float _tz)
	{
		mtxIdentity(_result);
		_result[12] = _tx;
		_result[13] = _ty;
		_result[14] = _tz;
	}

	inline void mtxScale(float* _result, float _sx, float _sy, float _sz)
	{
		memSet(_result, 0, sizeof(float) * 16);
		_result[0]  = _sx;
		_result[5]  = _sy;
		_result[10] = _sz;
		_result[15] = 1.0f;
	}

	inline void mtxScale(float* _result, float _scale)
	{
		mtxScale(_result, _scale, _scale, _scale);
	}

	inline void mtxFromNormal(float* _result, const Vec3& _normal, float _scale, const Vec3& _pos)
	{
		Vec3 tangent(InitNone);
		Vec3 bitangent(InitNone);
		calcTangentFrame(tangent, bitangent, _normal);

		store(&_result[ 0], mul(bitangent, _scale) );
		store(&_result[ 4], mul(_normal,   _scale) );
		store(&_result[ 8], mul(tangent,   _scale) );

		_result[ 3] = 0.0f;
		_result[ 7] = 0.0f;
		_result[11] = 0.0f;
		_result[12] = _pos.x;
		_result[13] = _pos.y;
		_result[14] = _pos.z;
		_result[15] = 1.0f;
	}

	inline void mtxFromNormal(float* _result, const Vec3& _normal, float _scale, const Vec3& _pos, float _angle)
	{
		Vec3 tangent(InitNone);
		Vec3 bitangent(InitNone);
		calcTangentFrame(tangent, bitangent, _normal, _angle);

		store(&_result[0], mul(bitangent, _scale) );
		store(&_result[4], mul(_normal,   _scale) );
		store(&_result[8], mul(tangent,   _scale) );

		_result[ 3] = 0.0f;
		_result[ 7] = 0.0f;
		_result[11] = 0.0f;
		_result[12] = _pos.x;
		_result[13] = _pos.y;
		_result[14] = _pos.z;
		_result[15] = 1.0f;
	}

	inline void mtxFromQuaternion(float* _result, const Quaternion& _rotation)
	{
		const float qx = _rotation.x;
		const float qy = _rotation.y;
		const float qz = _rotation.z;
		const float qw = _rotation.w;

		const float x2  = qx + qx;
		const float y2  = qy + qy;
		const float z2  = qz + qz;
		const float x2x = x2 * qx;
		const float x2y = x2 * qy;
		const float x2z = x2 * qz;
		const float x2w = x2 * qw;
		const float y2y = y2 * qy;
		const float y2z = y2 * qz;
		const float y2w = y2 * qw;
		const float z2z = z2 * qz;
		const float z2w = z2 * qw;

		_result[ 0] = 1.0f - (y2y + z2z);
		_result[ 1] =         x2y - z2w;
		_result[ 2] =         x2z + y2w;
		_result[ 3] = 0.0f;

		_result[ 4] =         x2y + z2w;
		_result[ 5] = 1.0f - (x2x + z2z);
		_result[ 6] =         y2z - x2w;
		_result[ 7] = 0.0f;

		_result[ 8] =         x2z - y2w;
		_result[ 9] =         y2z + x2w;
		_result[10] = 1.0f - (x2x + y2y);
		_result[11] = 0.0f;

		_result[12] = 0.0f;
		_result[13] = 0.0f;
		_result[14] = 0.0f;
		_result[15] = 1.0f;
	}

	inline void mtxFromQuaternion(float* _result, const Quaternion& _rotation, const Vec3& _translation)
	{
		mtxFromQuaternion(_result, _rotation);
		store(&_result[12], neg(mulXyz0(_translation, _result) ) );
	}

	inline Vec3 mul(const Vec3& _vec, const float* _mat)
	{
		Vec3 result(InitNone);
		result.x = _vec.x * _mat[0] + _vec.y * _mat[4] + _vec.z * _mat[ 8] + _mat[12];
		result.y = _vec.x * _mat[1] + _vec.y * _mat[5] + _vec.z * _mat[ 9] + _mat[13];
		result.z = _vec.x * _mat[2] + _vec.y * _mat[6] + _vec.z * _mat[10] + _mat[14];
		return result;
	}

	inline Vec3 mulXyz0(const Vec3& _vec, const float* _mat)
	{
		Vec3 result(InitNone);
		result.x = _vec.x * _mat[0] + _vec.y * _mat[4] + _vec.z * _mat[ 8];
		result.y = _vec.x * _mat[1] + _vec.y * _mat[5] + _vec.z * _mat[ 9];
		result.z = _vec.x * _mat[2] + _vec.y * _mat[6] + _vec.z * _mat[10];
		return result;
	}

	inline Vec3 mulH(const Vec3& _vec, const float* _mat)
	{
		const float xx   = _vec.x * _mat[0] + _vec.y * _mat[4] + _vec.z * _mat[ 8] + _mat[12];
		const float yy   = _vec.x * _mat[1] + _vec.y * _mat[5] + _vec.z * _mat[ 9] + _mat[13];
		const float zz   = _vec.x * _mat[2] + _vec.y * _mat[6] + _vec.z * _mat[10] + _mat[14];
		const float ww   = _vec.x * _mat[3] + _vec.y * _mat[7] + _vec.z * _mat[11] + _mat[15];
		const float invW = sign(ww) / ww;

		Vec3 result =
		{
			xx * invW,
			yy * invW,
			zz * invW,
		};

		return result;
	}

	inline void vec4MulMtx(float* _result, const float* _vec, const float* _mat)
	{
		_result[0] = _vec[0] * _mat[ 0] + _vec[1] * _mat[4] + _vec[2] * _mat[ 8] + _vec[3] * _mat[12];
		_result[1] = _vec[0] * _mat[ 1] + _vec[1] * _mat[5] + _vec[2] * _mat[ 9] + _vec[3] * _mat[13];
		_result[2] = _vec[0] * _mat[ 2] + _vec[1] * _mat[6] + _vec[2] * _mat[10] + _vec[3] * _mat[14];
		_result[3] = _vec[0] * _mat[ 3] + _vec[1] * _mat[7] + _vec[2] * _mat[11] + _vec[3] * _mat[15];
	}

	inline void mtxMul(float* _result, const float* _a, const float* _b)
	{
		vec4MulMtx(&_result[ 0], &_a[ 0], _b);
		vec4MulMtx(&_result[ 4], &_a[ 4], _b);
		vec4MulMtx(&_result[ 8], &_a[ 8], _b);
		vec4MulMtx(&_result[12], &_a[12], _b);
	}

	inline void mtxTranspose(float* _result, const float* _a)
	{
		_result[ 0] = _a[ 0];
		_result[ 4] = _a[ 1];
		_result[ 8] = _a[ 2];
		_result[12] = _a[ 3];
		_result[ 1] = _a[ 4];
		_result[ 5] = _a[ 5];
		_result[ 9] = _a[ 6];
		_result[13] = _a[ 7];
		_result[ 2] = _a[ 8];
		_result[ 6] = _a[ 9];
		_result[10] = _a[10];
		_result[14] = _a[11];
		_result[ 3] = _a[12];
		_result[ 7] = _a[13];
		_result[11] = _a[14];
		_result[15] = _a[15];
	}

	inline Vec3 calcNormal(const Vec3& _va, const Vec3& _vb, const Vec3& _vc)
	{
		const Vec3 ba    = sub(_vb, _va);
		const Vec3 ca    = sub(_vc, _va);
		const Vec3 baxca = cross(ba, ca);

		return normalize(baxca);
	}

	inline void calcPlane(Plane& _outPlane, const Vec3& _va, const Vec3& _vb, const Vec3& _vc)
	{
		Vec3 normal = calcNormal(_va, _vb, _vc);
		calcPlane(_outPlane, normal, _va);
	}

	inline void calcPlane(Plane& _outPlane, const Vec3& _normal, const Vec3& _pos)
	{
		_outPlane.normal = _normal;
		_outPlane.dist   = -dot(_normal, _pos);
	}

	inline BX_CONSTEXPR_FUNC float distance(const Plane& _plane, const Vec3& _pos)
	{
		return dot(_plane.normal, _pos) + _plane.dist;
	}

	inline BX_CONSTEXPR_FUNC bool isEqual(const Plane& _a, const Plane& _b, float _epsilon)
	{
		return isEqual(_a.normal, _b.normal, _epsilon)
			&& isEqual(_a.dist,   _b.dist,   _epsilon)
			;
	}

	inline BX_CONSTEXPR_FUNC float toLinear(float _a)
	{
		const float lo     = _a / 12.92f;
		const float hi     = pow( (_a + 0.055f) / 1.055f, 2.4f);
		const float result = lerp(hi, lo, _a <= 0.04045f);
		return result;
	}

	inline BX_CONSTEXPR_FUNC float toGamma(float _a)
	{
		const float lo     = _a * 12.92f;
		const float hi     = pow(abs(_a), 1.0f/2.4f) * 1.055f - 0.055f;
		const float result = lerp(hi, lo, _a <= 0.0031308f);
		return result;
	}

} // namespace bx
