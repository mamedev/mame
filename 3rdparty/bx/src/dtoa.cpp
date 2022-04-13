/*
 * Copyright 2010-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "bx_p.h"
#include <bx/cpu.h>
#include <bx/math.h>
#include <bx/string.h>
#include <bx/uint32_t.h>

#include <type_traits>

namespace bx
{
	/*
	 * https://github.com/miloyip/dtoa-benchmark
	 *
	 * Copyright (C) 2014 Milo Yip
	 *
	 * Permission is hereby granted, free of charge, to any person obtaining a copy
	 * of this software and associated documentation files (the "Software"), to deal
	 * in the Software without restriction, including without limitation the rights
	 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	 * copies of the Software, and to permit persons to whom the Software is
	 * furnished to do so, subject to the following conditions:
	 *
	 * The above copyright notice and this permission notice shall be included in
	 * all copies or substantial portions of the Software.
	 *
	 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	 * THE SOFTWARE.
	 *
	 */

	struct DiyFp
	{
		DiyFp()
		{
		}

		DiyFp(uint64_t _f, int32_t _e)
			: f(_f)
			, e(_e)
		{
		}

		DiyFp(double d)
		{
			union
			{
				double d;
				uint64_t u64;
			} u = { d };

			int32_t biased_e = (u.u64 & kDpExponentMask) >> kDpSignificandSize;
			uint64_t significand = (u.u64 & kDpSignificandMask);
			if (biased_e != 0)
			{
				f = significand + kDpHiddenBit;
				e = biased_e - kDpExponentBias;
			}
			else
			{
				f = significand;
				e = kDpMinExponent + 1;
			}
		}

		DiyFp operator-(const DiyFp& rhs) const
		{
			BX_ASSERT(e == rhs.e, "");
			BX_ASSERT(f >= rhs.f, "");
			return DiyFp(f - rhs.f, e);
		}

		DiyFp operator*(const DiyFp& rhs) const
		{
			const uint64_t M32 = UINT32_MAX;
			const uint64_t a = f >> 32;
			const uint64_t b = f & M32;
			const uint64_t c = rhs.f >> 32;
			const uint64_t d = rhs.f & M32;
			const uint64_t ac = a * c;
			const uint64_t bc = b * c;
			const uint64_t ad = a * d;
			const uint64_t bd = b * d;
			uint64_t tmp = (bd >> 32) + (ad & M32) + (bc & M32);
			tmp += 1U << 31;  /// mult_round
			return DiyFp(ac + (ad >> 32) + (bc >> 32) + (tmp >> 32), e + rhs.e + 64);
		}

		DiyFp Normalize() const
		{
			uint32_t s = uint64_cntlz(f);
			return DiyFp(f << s, e - s);
		}

		DiyFp NormalizeBoundary() const
		{
			uint32_t index = uint64_cntlz(f);
			return DiyFp (f << index, e - index);
		}

		void NormalizedBoundaries(DiyFp* minus, DiyFp* plus) const
		{
			DiyFp pl = DiyFp( (f << 1) + 1, e - 1).NormalizeBoundary();
			DiyFp mi = (f == kDpHiddenBit) ? DiyFp( (f << 2) - 1, e - 2) : DiyFp( (f << 1) - 1, e - 1);
			mi.f <<= mi.e - pl.e;
			mi.e = pl.e;
			*plus = pl;
			*minus = mi;
		}

#define UINT64_C2(h, l) ( (static_cast<uint64_t>(h) << 32) | static_cast<uint64_t>(l) )

		static const int32_t  kDiySignificandSize = 64;
		static const int32_t  kDpSignificandSize  = 52;
		static const int32_t  kDpExponentBias     = 0x3FF + kDpSignificandSize;
		static const int32_t  kDpMinExponent      = -kDpExponentBias;
		static const uint64_t kDpExponentMask     = UINT64_C2(0x7FF00000, 0x00000000);
		static const uint64_t kDpSignificandMask  = UINT64_C2(0x000FFFFF, 0xFFFFFFFF);
		static const uint64_t kDpHiddenBit        = UINT64_C2(0x00100000, 0x00000000);

		uint64_t f;
		int32_t e;
	};

	// 10^-348, 10^-340, ..., 10^340
	static const uint64_t s_kCachedPowers_F[] =
	{
		UINT64_C2(0xfa8fd5a0, 0x081c0288), UINT64_C2(0xbaaee17f, 0xa23ebf76),
		UINT64_C2(0x8b16fb20, 0x3055ac76), UINT64_C2(0xcf42894a, 0x5dce35ea),
		UINT64_C2(0x9a6bb0aa, 0x55653b2d), UINT64_C2(0xe61acf03, 0x3d1a45df),
		UINT64_C2(0xab70fe17, 0xc79ac6ca), UINT64_C2(0xff77b1fc, 0xbebcdc4f),
		UINT64_C2(0xbe5691ef, 0x416bd60c), UINT64_C2(0x8dd01fad, 0x907ffc3c),
		UINT64_C2(0xd3515c28, 0x31559a83), UINT64_C2(0x9d71ac8f, 0xada6c9b5),
		UINT64_C2(0xea9c2277, 0x23ee8bcb), UINT64_C2(0xaecc4991, 0x4078536d),
		UINT64_C2(0x823c1279, 0x5db6ce57), UINT64_C2(0xc2109436, 0x4dfb5637),
		UINT64_C2(0x9096ea6f, 0x3848984f), UINT64_C2(0xd77485cb, 0x25823ac7),
		UINT64_C2(0xa086cfcd, 0x97bf97f4), UINT64_C2(0xef340a98, 0x172aace5),
		UINT64_C2(0xb23867fb, 0x2a35b28e), UINT64_C2(0x84c8d4df, 0xd2c63f3b),
		UINT64_C2(0xc5dd4427, 0x1ad3cdba), UINT64_C2(0x936b9fce, 0xbb25c996),
		UINT64_C2(0xdbac6c24, 0x7d62a584), UINT64_C2(0xa3ab6658, 0x0d5fdaf6),
		UINT64_C2(0xf3e2f893, 0xdec3f126), UINT64_C2(0xb5b5ada8, 0xaaff80b8),
		UINT64_C2(0x87625f05, 0x6c7c4a8b), UINT64_C2(0xc9bcff60, 0x34c13053),
		UINT64_C2(0x964e858c, 0x91ba2655), UINT64_C2(0xdff97724, 0x70297ebd),
		UINT64_C2(0xa6dfbd9f, 0xb8e5b88f), UINT64_C2(0xf8a95fcf, 0x88747d94),
		UINT64_C2(0xb9447093, 0x8fa89bcf), UINT64_C2(0x8a08f0f8, 0xbf0f156b),
		UINT64_C2(0xcdb02555, 0x653131b6), UINT64_C2(0x993fe2c6, 0xd07b7fac),
		UINT64_C2(0xe45c10c4, 0x2a2b3b06), UINT64_C2(0xaa242499, 0x697392d3),
		UINT64_C2(0xfd87b5f2, 0x8300ca0e), UINT64_C2(0xbce50864, 0x92111aeb),
		UINT64_C2(0x8cbccc09, 0x6f5088cc), UINT64_C2(0xd1b71758, 0xe219652c),
		UINT64_C2(0x9c400000, 0x00000000), UINT64_C2(0xe8d4a510, 0x00000000),
		UINT64_C2(0xad78ebc5, 0xac620000), UINT64_C2(0x813f3978, 0xf8940984),
		UINT64_C2(0xc097ce7b, 0xc90715b3), UINT64_C2(0x8f7e32ce, 0x7bea5c70),
		UINT64_C2(0xd5d238a4, 0xabe98068), UINT64_C2(0x9f4f2726, 0x179a2245),
		UINT64_C2(0xed63a231, 0xd4c4fb27), UINT64_C2(0xb0de6538, 0x8cc8ada8),
		UINT64_C2(0x83c7088e, 0x1aab65db), UINT64_C2(0xc45d1df9, 0x42711d9a),
		UINT64_C2(0x924d692c, 0xa61be758), UINT64_C2(0xda01ee64, 0x1a708dea),
		UINT64_C2(0xa26da399, 0x9aef774a), UINT64_C2(0xf209787b, 0xb47d6b85),
		UINT64_C2(0xb454e4a1, 0x79dd1877), UINT64_C2(0x865b8692, 0x5b9bc5c2),
		UINT64_C2(0xc83553c5, 0xc8965d3d), UINT64_C2(0x952ab45c, 0xfa97a0b3),
		UINT64_C2(0xde469fbd, 0x99a05fe3), UINT64_C2(0xa59bc234, 0xdb398c25),
		UINT64_C2(0xf6c69a72, 0xa3989f5c), UINT64_C2(0xb7dcbf53, 0x54e9bece),
		UINT64_C2(0x88fcf317, 0xf22241e2), UINT64_C2(0xcc20ce9b, 0xd35c78a5),
		UINT64_C2(0x98165af3, 0x7b2153df), UINT64_C2(0xe2a0b5dc, 0x971f303a),
		UINT64_C2(0xa8d9d153, 0x5ce3b396), UINT64_C2(0xfb9b7cd9, 0xa4a7443c),
		UINT64_C2(0xbb764c4c, 0xa7a44410), UINT64_C2(0x8bab8eef, 0xb6409c1a),
		UINT64_C2(0xd01fef10, 0xa657842c), UINT64_C2(0x9b10a4e5, 0xe9913129),
		UINT64_C2(0xe7109bfb, 0xa19c0c9d), UINT64_C2(0xac2820d9, 0x623bf429),
		UINT64_C2(0x80444b5e, 0x7aa7cf85), UINT64_C2(0xbf21e440, 0x03acdd2d),
		UINT64_C2(0x8e679c2f, 0x5e44ff8f), UINT64_C2(0xd433179d, 0x9c8cb841),
		UINT64_C2(0x9e19db92, 0xb4e31ba9), UINT64_C2(0xeb96bf6e, 0xbadf77d9),
		UINT64_C2(0xaf87023b, 0x9bf0ee6b)
	};

	static const int16_t s_kCachedPowers_E[] =
	{
		-1220, -1193, -1166, -1140, -1113, -1087, -1060, -1034, -1007,  -980,
		 -954,  -927,  -901,  -874,  -847,  -821,  -794,  -768,  -741,  -715,
		 -688,  -661,  -635,  -608,  -582,  -555,  -529,  -502,  -475,  -449,
		 -422,  -396,  -369,  -343,  -316,  -289,  -263,  -236,  -210,  -183,
		 -157,  -130,  -103,   -77,   -50,   -24,     3,    30,    56,    83,
		  109,   136,   162,   189,   216,   242,   269,   295,   322,   348,
		  375,   402,   428,   455,   481,   508,   534,   561,   588,   614,
		  641,   667,   694,   720,   747,   774,   800,   827,   853,   880,
		  907,   933,   960,   986,  1013,  1039,  1066
	};

	static const char s_cDigitsLut[200] =
	{
		'0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6', '0', '7', '0', '8', '0', '9',
		'1', '0', '1', '1', '1', '2', '1', '3', '1', '4', '1', '5', '1', '6', '1', '7', '1', '8', '1', '9',
		'2', '0', '2', '1', '2', '2', '2', '3', '2', '4', '2', '5', '2', '6', '2', '7', '2', '8', '2', '9',
		'3', '0', '3', '1', '3', '2', '3', '3', '3', '4', '3', '5', '3', '6', '3', '7', '3', '8', '3', '9',
		'4', '0', '4', '1', '4', '2', '4', '3', '4', '4', '4', '5', '4', '6', '4', '7', '4', '8', '4', '9',
		'5', '0', '5', '1', '5', '2', '5', '3', '5', '4', '5', '5', '5', '6', '5', '7', '5', '8', '5', '9',
		'6', '0', '6', '1', '6', '2', '6', '3', '6', '4', '6', '5', '6', '6', '6', '7', '6', '8', '6', '9',
		'7', '0', '7', '1', '7', '2', '7', '3', '7', '4', '7', '5', '7', '6', '7', '7', '7', '8', '7', '9',
		'8', '0', '8', '1', '8', '2', '8', '3', '8', '4', '8', '5', '8', '6', '8', '7', '8', '8', '8', '9',
		'9', '0', '9', '1', '9', '2', '9', '3', '9', '4', '9', '5', '9', '6', '9', '7', '9', '8', '9', '9'
	};

	static const uint32_t s_kPow10[] =
	{
		1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000
	};

	DiyFp GetCachedPower(int32_t e, int32_t* K)
	{
		double dk = (-61 - e) * 0.30102999566398114 + 347;	// dk must be positive, so can do ceiling in positive
		int32_t k = static_cast<int32_t>(dk);
		if (k != dk)
		{
			k++;
		}

		uint32_t index = static_cast<uint32_t>( (k >> 3) + 1);
		*K = -(-348 + static_cast<int32_t>(index << 3) );	// decimal exponent no need lookup table

		BX_ASSERT(index < sizeof(s_kCachedPowers_F) / sizeof(s_kCachedPowers_F[0]), "");
		return DiyFp(s_kCachedPowers_F[index], s_kCachedPowers_E[index]);
	}

	void GrisuRound(char* buffer, int32_t len, uint64_t delta, uint64_t rest, uint64_t ten_kappa, uint64_t wp_w)
	{
		while (rest < wp_w
			&& delta - rest >= ten_kappa
			&& (rest + ten_kappa < wp_w || wp_w - rest > rest + ten_kappa - wp_w) )
		{
			buffer[len - 1]--;
			rest += ten_kappa;
		}
	}

	uint32_t CountDecimalDigit32(uint32_t n)
	{
		// Simple pure C++ implementation was faster than __builtin_clz version in this situation.
		if (n < 10) return 1;
		if (n < 100) return 2;
		if (n < 1000) return 3;
		if (n < 10000) return 4;
		if (n < 100000) return 5;
		if (n < 1000000) return 6;
		if (n < 10000000) return 7;
		if (n < 100000000) return 8;
		if (n < 1000000000) return 9;
		return 10;
	}

	void DigitGen(const DiyFp& W, const DiyFp& Mp, uint64_t delta, char* buffer, int32_t* len, int32_t* K)
	{
		const DiyFp one(uint64_t(1) << -Mp.e, Mp.e);
		const DiyFp wp_w = Mp - W;
		uint32_t p1 = static_cast<uint32_t>(Mp.f >> -one.e);
		uint64_t p2 = Mp.f & (one.f - 1);
		int32_t kappa = static_cast<int32_t>(CountDecimalDigit32(p1) );
		*len = 0;

		while (kappa > 0)
		{
			uint32_t d;
			switch (kappa)
			{
				case 10: d = p1 / 1000000000; p1 %= 1000000000; break;
				case  9: d = p1 /  100000000; p1 %=  100000000; break;
				case  8: d = p1 /   10000000; p1 %=   10000000; break;
				case  7: d = p1 /    1000000; p1 %=    1000000; break;
				case  6: d = p1 /     100000; p1 %=     100000; break;
				case  5: d = p1 /      10000; p1 %=      10000; break;
				case  4: d = p1 /       1000; p1 %=       1000; break;
				case  3: d = p1 /        100; p1 %=        100; break;
				case  2: d = p1 /         10; p1 %=         10; break;
				case  1: d = p1;              p1  =          0; break;
				default:
					d = 0;
					break;
			}

			if (d || *len)
			{
				buffer[(*len)++] = '0' + static_cast<char>(d);
			}

			kappa--;
			uint64_t tmp = (static_cast<uint64_t>(p1) << -one.e) + p2;
			if (tmp <= delta)
			{
				*K += kappa;
				GrisuRound(buffer, *len, delta, tmp, static_cast<uint64_t>(s_kPow10[kappa]) << -one.e, wp_w.f);
				return;
			}
		}

		// kappa = 0
		for (;;)
		{
			p2 *= 10;
			delta *= 10;
			char d = static_cast<char>(p2 >> -one.e);
			if (d || *len)
			{
				buffer[(*len)++] = '0' + d;
			}

			p2 &= one.f - 1;
			kappa--;
			if (p2 < delta)
			{
				*K += kappa;
				const int index = -static_cast<int>(kappa);
				GrisuRound(buffer, *len, delta, p2, one.f, wp_w.f * (index < 9 ? s_kPow10[-static_cast<int>(kappa)] : 0));
				return;
			}
		}
	}

	void Grisu2(double value, char* buffer, int32_t* length, int32_t* K)
	{
		const DiyFp v(value);
		DiyFp w_m, w_p;
		v.NormalizedBoundaries(&w_m, &w_p);

		const DiyFp c_mk = GetCachedPower(w_p.e, K);
		const DiyFp W = v.Normalize() * c_mk;
		DiyFp Wp = w_p * c_mk;
		DiyFp Wm = w_m * c_mk;
		Wm.f++;
		Wp.f--;
		DigitGen(W, Wp, Wp.f - Wm.f, buffer, length, K);
	}

	int32_t WriteExponent(int32_t K, char* buffer)
	{
		const char* ptr = buffer;

		if (K < 0)
		{
			*buffer++ = '-';
			K = -K;
		}

		if (K >= 100)
		{
			*buffer++ = '0' + static_cast<char>(K / 100);
			K %= 100;
			const char* d = s_cDigitsLut + K * 2;
			*buffer++ = d[0];
			*buffer++ = d[1];
		}
		else if (K >= 10)
		{
			const char* d = s_cDigitsLut + K * 2;
			*buffer++ = d[0];
			*buffer++ = d[1];
		}
		else
		{
			*buffer++ = '0' + static_cast<char>(K);
		}

		*buffer = '\0';

		return int32_t(buffer - ptr);
	}

	int32_t Prettify(char* buffer, int32_t length, int32_t k)
	{
		const int32_t kk = length + k;	// 10^(kk-1) <= v < 10^kk

		if (length <= kk && kk <= 21)
		{
			// 1234e7 -> 12340000000
			for (int32_t i = length; i < kk; i++)
			{
				buffer[i] = '0';
			}

			buffer[kk] = '.';
			buffer[kk + 1] = '0';
			buffer[kk + 2] = '\0';
			return kk + 2;
		}

		if (0 < kk && kk <= 21)
		{
			// 1234e-2 -> 12.34
			memMove(&buffer[kk + 1], &buffer[kk], length - kk);
			buffer[kk] = '.';
			buffer[length + 1] = '\0';
			return length + 1;
		}

		if (-6 < kk && kk <= 0)
		{
			// 1234e-6 -> 0.001234
			const int32_t offset = 2 - kk;
			memMove(&buffer[offset], &buffer[0], length);
			buffer[0] = '0';
			buffer[1] = '.';
			for (int32_t i = 2; i < offset; i++)
			{
				buffer[i] = '0';
			}

			buffer[length + offset] = '\0';
			return length + offset;
		}

		if (length == 1)
		{
			// 1e30
			buffer[1] = 'e';
			int32_t exp = WriteExponent(kk - 1, &buffer[2]);
			return 2 + exp;
		}

		// 1234e30 -> 1.234e33
		memMove(&buffer[2], &buffer[1], length - 1);
		buffer[1] = '.';
		buffer[length + 1] = 'e';
		int32_t exp = WriteExponent(kk - 1, &buffer[length + 2]);
		return length + 2 + exp;
	}

	int32_t toString(char* _dst, int32_t _max, double _value)
	{
		int32_t sign = 0 != (doubleToBits(_value) & (UINT64_C(1)<<63) ) ? 1 : 0;
		if (1 == sign)
		{
			*_dst++ = '-';
			--_max;
			_value = -_value;
		}

		if (isNan(_value) )
		{
			return (int32_t)strCopy(_dst, _max, "nan") + sign;
		}
		else if (isInfinite(_value) )
		{
			return (int32_t)strCopy(_dst, _max, "inf") + sign;
		}

		int32_t len;
		if (0.0 == _value)
		{
			len = (int32_t)strCopy(_dst, _max, "0.0");
		}
		else
		{
			int32_t kk;
			Grisu2(_value, _dst, &len, &kk);
			len = Prettify(_dst, len, kk);
		}

		return len + sign;
	}

	static void reverse(char* _dst, int32_t _len)
	{
		for (int32_t ii = 0, jj = _len - 1; ii < jj; ++ii, --jj)
		{
			swap(_dst[ii], _dst[jj]);
		}
	}

	template<typename Ty>
	int32_t toStringSigned(char* _dst, int32_t _max, Ty _value, uint32_t _base, char _separator)
	{
		if (_base == 10
		&&  _value < 0)
		{
			if (_max < 1)
			{
				return 0;
			}

			_max = toString(_dst + 1
				, _max - 1
				, typename std::make_unsigned<Ty>::type(-_value)
				, _base
				, _separator
				);
			if (_max == 0)
			{
				return 0;
			}

			*_dst = '-';
			return int32_t(_max + 1);
		}

		return toString(_dst
			, _max
			, typename std::make_unsigned<Ty>::type(_value)
			, _base
			, _separator
			);
	}

	int32_t toString(char* _dst, int32_t _max, int32_t _value, uint32_t _base, char _separator)
	{
		return toStringSigned(_dst, _max, _value, _base, _separator);
	}

	int32_t toString(char* _dst, int32_t _max, int64_t _value, uint32_t _base, char _separator)
	{
		return toStringSigned(_dst, _max, _value, _base, _separator);
	}

	template<typename Ty>
	int32_t toStringUnsigned(char* _dst, int32_t _max, Ty _value, uint32_t _base, char _separator)
	{
		char data[32];
		int32_t len = 0;

		if (_base > 16
		||  _base < 2)
		{
			return 0;
		}

		uint32_t count = 1;

		do
		{
			const Ty rem = _value % _base;
			_value /= _base;
			if (rem < 10)
			{
				data[len++] = char('0' + rem);
			}
			else
			{
				data[len++] = char('a' + rem - 10);
			}

			if ('\0' != _separator
			&&  0 == count%3
			&&  0 != _value)
			{
				data[len++] = _separator;
			}

			++count;
		}
		while (0 != _value);

		if (_max < len + 1)
		{
			return 0;
		}

		reverse(data, len);

		memCopy(_dst, data, len);
		_dst[len] = '\0';
		return int32_t(len);
	}

	int32_t toString(char* _dst, int32_t _max, uint32_t _value, uint32_t _base, char _separator)
	{
		return toStringUnsigned(_dst, _max, _value, _base, _separator);
	}

	int32_t toString(char* _dst, int32_t _max, uint64_t _value, uint32_t _base, char _separator)
	{
		return toStringUnsigned(_dst, _max, _value, _base, _separator);
	}

	/*
	 * https://github.com/grzegorz-kraszewski/stringtofloat/
	 *
	 * MIT License
	 *
	 * Copyright (c) 2016 Grzegorz Kraszewski
	 *
	 * Permission is hereby granted, free of charge, to any person obtaining a copy
	 * of this software and associated documentation files (the "Software"), to deal
	 * in the Software without restriction, including without limitation the rights
	 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	 * copies of the Software, and to permit persons to whom the Software is
	 * furnished to do so, subject to the following conditions:
	 *
	 * The above copyright notice and this permission notice shall be included in all
	 * copies or substantial portions of the Software.
	 *
	 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	 * SOFTWARE.
	 */

	/*
	 * IMPORTANT
	 *
	 * The code works in "round towards zero" mode. This is different from
	 * GCC standard library strtod(), which uses "round half to even" rule.
	 * Therefore it cannot be used as a direct drop-in replacement, as in
	 * some cases results will be different on the least significant bit of
	 * mantissa. Read more in the README.md file.
	 */

#define DIGITS 18

#define DOUBLE_PLUS_ZERO      UINT64_C(0x0000000000000000)
#define DOUBLE_MINUS_ZERO     UINT64_C(0x8000000000000000)
#define DOUBLE_PLUS_INFINITY  UINT64_C(0x7ff0000000000000)
#define DOUBLE_MINUS_INFINITY UINT64_C(0xfff0000000000000)

	union HexDouble
	{
		double d;
		uint64_t u;
	};

#define lsr96(s2, s1, s0, d2, d1, d0)      \
	d0 = ( (s0) >> 1) | ( ( (s1) & 1) << 31); \
	d1 = ( (s1) >> 1) | ( ( (s2) & 1) << 31); \
	d2 = (s2) >> 1;

#define lsl96(s2, s1, s0, d2, d1, d0)              \
	d2 = ( (s2) << 1) | ( ( (s1) & (1 << 31) ) >> 31); \
	d1 = ( (s1) << 1) | ( ( (s0) & (1 << 31) ) >> 31); \
	d0 = (s0) << 1;

	/*
	 * Undefine the below constant if your processor or compiler is slow
	 * at 64-bit arithmetic. This is a rare case however. 64-bit macros are
	 * better for deeply pipelined CPUs (no conditional execution), are
	 * very efficient for 64-bit processors and also fast on 32-bit processors
	 * featuring extended precision arithmetic (x86, PowerPC_32, M68k and probably
	 * more).
	 */

#define USE_64BIT_FOR_ADDSUB_MACROS 0

#if USE_64BIT_FOR_ADDSUB_MACROS

#define add96(s2, s1, s0, d2, d1, d0) {   \
	uint64_t w;                           \
	w = (uint64_t)(s0) + (uint64_t)(d0);  \
	(s0) = w;                             \
	w >>= 32;                             \
	w += (uint64_t)(s1) + (uint64_t)(d1); \
	(s1) = w;                             \
	w >>= 32;                             \
	w += (uint64_t)(s2) + (uint64_t)(d2); \
	(s2) = w; }

#define sub96(s2, s1, s0, d2, d1, d0) {   \
	uint64_t w;                           \
	w = (uint64_t)(s0) - (uint64_t)(d0);  \
	(s0) = w;                             \
	w >>= 32;                             \
	w += (uint64_t)(s1) - (uint64_t)(d1); \
	(s1) = w;                             \
	w >>= 32;                             \
	w += (uint64_t)(s2) - (uint64_t)(d2); \
	(s2) = w; }

#else

#define add96(s2, s1, s0, d2, d1, d0) {                                \
	uint32_t _x, _c;                                                   \
	_x = (s0); (s0) += (d0);                                           \
	if ( (s0) < _x) _c = 1; else _c = 0;                               \
	_x = (s1); (s1) += (d1) + _c;                                      \
	if ( ( (s1) < _x) || ( ( (s1) == _x) && _c) ) _c = 1; else _c = 0; \
	(s2) += (d2) + _c; }

#define sub96(s2, s1, s0, d2, d1, d0) {                                \
	uint32_t _x, _c;                                                   \
	_x = (s0); (s0) -= (d0);                                           \
	if ( (s0) > _x) _c = 1; else _c = 0;                               \
	_x = (s1); (s1) -= (d1) + _c;                                      \
	if ( ( (s1) > _x) || ( ( (s1) == _x) && _c) ) _c = 1; else _c = 0; \
	(s2) -= (d2) + _c; }

#endif   /* USE_64BIT_FOR_ADDSUB_MACROS */

	/* parser state machine states */

#define FSM_A    0
#define FSM_B    1
#define FSM_C    2
#define FSM_D    3
#define FSM_E    4
#define FSM_F    5
#define FSM_G    6
#define FSM_H    7
#define FSM_I    8
#define FSM_STOP 9

	/* The structure is filled by parser, then given to converter. */
	struct PrepNumber
	{
		int negative;      /* 0 if positive number, 1 if negative */
		int32_t exponent;  /* power of 10 exponent */
		uint64_t mantissa; /* integer mantissa */
	};

	/* Possible parser return values. */

#define PARSER_OK     0  // parser finished OK
#define PARSER_PZERO  1  // no digits or number is smaller than +-2^-1022
#define PARSER_MZERO  2  // number is negative, module smaller
#define PARSER_PINF   3  // number is higher than +HUGE_VAL
#define PARSER_MINF   4  // number is lower than -HUGE_VAL

	inline char next(const char*& _s, const char* _term)
	{
		return _s != _term
			? *_s++
			: '\0'
			;
	}

	static int parser(const char* _s, const char* _term, PrepNumber* _pn)
	{
		int state = FSM_A;
		int digx = 0;
		char c = ' ';            /* initial value for kicking off the state machine */
		int result = PARSER_OK;
		int expneg = 0;
		int32_t expexp = 0;

		while (state != FSM_STOP) // && _s != _term)
		{
			switch (state)
			{
			case FSM_A:
				if (isSpace(c) )
				{
					c = next(_s, _term);
				}
				else
				{
					state = FSM_B;
				}
				break;

			case FSM_B:
				state = FSM_C;

				if (c == '+')
				{
					c = next(_s, _term);
				}
				else if (c == '-')
				{
					_pn->negative = 1;
					c = next(_s, _term);
				}
				else if (isNumeric(c) )
				{
				}
				else if (c == '.')
				{
				}
				else
				{
					state = FSM_STOP;
				}
				break;

			case FSM_C:
				if (c == '0')
				{
					c = next(_s, _term);
				}
				else if (c == '.')
				{
					c = next(_s, _term);
					state = FSM_D;
				}
				else
				{
					state = FSM_E;
				}
				break;

			case FSM_D:
				if (c == '0')
				{
					c = next(_s, _term);
					if (_pn->exponent > -2147483647) _pn->exponent--;
				}
				else
				{
					state = FSM_F;
				}
				break;

			case FSM_E:
				if (isNumeric(c) )
				{
					if (digx < DIGITS)
					{
						_pn->mantissa *= 10;
						_pn->mantissa += c - '0';
						digx++;
					}
					else if (_pn->exponent < 2147483647)
					{
						_pn->exponent++;
					}

					c = next(_s, _term);
				}
				else if (c == '.')
				{
					c = next(_s, _term);
					state = FSM_F;
				}
				else
				{
					state = FSM_F;
				}
				break;

			case FSM_F:
				if (isNumeric(c) )
				{
					if (digx < DIGITS)
					{
						_pn->mantissa *= 10;
						_pn->mantissa += c - '0';
						_pn->exponent--;
						digx++;
					}

					c = next(_s, _term);
				}
				else if ('e' == toLower(c) )
				{
					c = next(_s, _term);
					state = FSM_G;
				}
				else
				{
					state = FSM_G;
				}
				break;

			case FSM_G:
				if (c == '+')
				{
					c = next(_s, _term);
				}
				else if (c == '-')
				{
					expneg = 1;
					c = next(_s, _term);
				}

				state = FSM_H;
				break;

			case FSM_H:
				if (c == '0')
				{
					c = next(_s, _term);
				}
				else
				{
					state = FSM_I;
				}
				break;

			case FSM_I:
				if (isNumeric(c) )
				{
					if (expexp < 214748364)
					{
						expexp *= 10;
						expexp += c - '0';
					}

					c = next(_s, _term);
				}
				else
				{
					state = FSM_STOP;
				}
				break;
			}
		}

		if (expneg)
		{
			expexp = -expexp;
		}

		_pn->exponent += expexp;

		if (_pn->mantissa == 0)
		{
			if (_pn->negative)
			{
				result = PARSER_MZERO;
			}
			else
			{
				result = PARSER_PZERO;
			}
		}
		else if (_pn->exponent > 309)
		{
			if (_pn->negative)
			{
				result = PARSER_MINF;
			}
			else
			{
				result = PARSER_PINF;
			}
		}
		else if (_pn->exponent < -328)
		{
			if (_pn->negative)
			{
				result = PARSER_MZERO;
			}
			else
			{
				result = PARSER_PZERO;
			}
		}

		return result;
	}

	static double converter(PrepNumber* _pn)
	{
		int binexp = 92;
		HexDouble hd;
		uint32_t s2, s1, s0; /* 96-bit precision integer */
		uint32_t q2, q1, q0; /* 96-bit precision integer */
		uint32_t r2, r1, r0; /* 96-bit precision integer */
		uint32_t mask28 = UINT32_C(0xf) << 28;

		hd.u = 0;

		s0 = (uint32_t)(_pn->mantissa & UINT32_MAX);
		s1 = (uint32_t)(_pn->mantissa >> 32);
		s2 = 0;

		while (_pn->exponent > 0)
		{
			lsl96(s2, s1, s0, q2, q1, q0); // q = p << 1
			lsl96(q2, q1, q0, r2, r1, r0); // r = p << 2
			lsl96(r2, r1, r0, s2, s1, s0); // p = p << 3
			add96(s2, s1, s0, q2, q1, q0); // p = (p << 3) + (p << 1)

			_pn->exponent--;

			while (s2 & mask28)
			{
				lsr96(s2, s1, s0, q2, q1, q0);
				binexp++;
				s2 = q2;
				s1 = q1;
				s0 = q0;
			}
		}

		while (_pn->exponent < 0)
		{
			while (!(s2 & (1 << 31) ) )
			{
				lsl96(s2, s1, s0, q2, q1, q0);
				binexp--;
				s2 = q2;
				s1 = q1;
				s0 = q0;
			}

			q2 = s2 / 10;
			r1 = s2 % 10;
			r2 = (s1 >> 8) | (r1 << 24);
			q1 = r2 / 10;
			r1 = r2 % 10;
			r2 = ( (s1 & 0xFF) << 16) | (s0 >> 16) | (r1 << 24);
			r0 = r2 / 10;
			r1 = r2 % 10;
			q1 = (q1 << 8) | ( (r0 & 0x00FF0000) >> 16);
			q0 = r0 << 16;
			r2 = (s0 & UINT16_MAX) | (r1 << 16);
			q0 |= r2 / 10;
			s2 = q2;
			s1 = q1;
			s0 = q0;

			_pn->exponent++;
		}

		if (s2 || s1 || s0)
		{
			while (!(s2 & mask28) )
			{
				lsl96(s2, s1, s0, q2, q1, q0);
				binexp--;
				s2 = q2;
				s1 = q1;
				s0 = q0;
			}
		}

		binexp += 1023;

		if (binexp > 2046)
		{
			if (_pn->negative)
			{
				hd.u = DOUBLE_MINUS_INFINITY;
			}
			else
			{
				hd.u = DOUBLE_PLUS_INFINITY;
			}
		}
		else if (binexp < 1)
		{
			if (_pn->negative)
			{
				hd.u = DOUBLE_MINUS_ZERO;
			}
		}
		else if (s2)
		{
			uint64_t q;
			uint64_t binexs2 = (uint64_t)binexp;

			binexs2 <<= 52;
			q =   ( (uint64_t)(s2 & ~mask28) << 24)
			  | ( ( (uint64_t)s1 + 128) >> 8) | binexs2;

			if (_pn->negative)
			{
				q |= (1ULL << 63);
			}

			hd.u = q;
		}

		return hd.d;
	}

	int32_t toString(char* _out, int32_t _max, bool _value)
	{
		StringView str(_value ? "true" : "false");
		strCopy(_out, _max, str);
		return str.getLength();
	}

	bool fromString(bool* _out, const StringView& _str)
	{
		char ch = toLower(_str.getPtr()[0]);
		*_out = ch == 't' ||  ch == '1';
		return 0 != _str.getLength();
	}

	bool fromString(float* _out, const StringView& _str)
	{
		double dbl;
		bool result = fromString(&dbl, _str);
		*_out = float(dbl);
		return result;
	}

	bool fromString(double* _out, const StringView& _str)
	{
		PrepNumber pn;
		pn.mantissa = 0;
		pn.negative = 0;
		pn.exponent = 0;

		HexDouble hd;
		hd.u = DOUBLE_PLUS_ZERO;

		switch (parser(_str.getPtr(), _str.getTerm(), &pn) )
		{
		case PARSER_OK:
			*_out = converter(&pn);
			break;

		case PARSER_PZERO:
			*_out = hd.d;
			break;

		case PARSER_MZERO:
			hd.u = DOUBLE_MINUS_ZERO;
			*_out = hd.d;
			break;

		case PARSER_PINF:
			hd.u = DOUBLE_PLUS_INFINITY;
			*_out = hd.d;
			break;

		case PARSER_MINF:
			hd.u = DOUBLE_MINUS_INFINITY;
			*_out = hd.d;
			break;
		}

		return true;
	}

	bool fromString(int32_t* _out, const StringView& _str)
	{
		StringView str = bx::strLTrimSpace(_str);

		const char* ptr  = str.getPtr();
		const char* term = str.getTerm();

		char ch = *ptr++;
		bool neg = false;
		switch (ch)
		{
		case '-':
		case '+': neg = '-' == ch;
			break;

		default:
			--ptr;
			break;
		}

		int32_t result = 0;

		for (ch = *ptr++; isNumeric(ch) && ptr <= term; ch = *ptr++)
		{
			result = 10*result - (ch - '0');
		}

		*_out = neg ? result : -result;

		return true;
	}

	bool fromString(uint32_t* _out, const StringView& _str)
	{
		fromString( (int32_t*)_out, _str);
		return true;
	}

} // namespace bx
