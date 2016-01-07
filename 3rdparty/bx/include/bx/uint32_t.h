/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

// Copyright 2006 Mike Acton <macton@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE

#ifndef BX_UINT32_T_H_HEADER_GUARD
#define BX_UINT32_T_H_HEADER_GUARD

#include "bx.h"

#if BX_COMPILER_MSVC
#	if BX_PLATFORM_WINDOWS || BX_PLATFORM_WINRT
#		include <math.h> // math.h is included because VS bitches:
						 // warning C4985: 'ceil': attributes not present on previous declaration.
						 // must be included before intrin.h.
#		include <intrin.h>
#		pragma intrinsic(_BitScanForward)
#		pragma intrinsic(_BitScanReverse)
#		if BX_ARCH_64BIT
#			pragma intrinsic(_BitScanForward64)
#			pragma intrinsic(_BitScanReverse64)
#		endif // BX_ARCH_64BIT
#	endif // BX_PLATFORM_WINDOWS
#endif // BX_COMPILER_MSVC

#define BX_HALF_FLOAT_ZERO UINT16_C(0)
#define BX_HALF_FLOAT_HALF UINT16_C(0x3800)
#define BX_HALF_FLOAT_ONE  UINT16_C(0x3c00)
#define BX_HALF_FLOAT_TWO  UINT16_C(0x4000)

namespace bx
{
	inline uint32_t uint32_li(uint32_t _a)
	{
		return _a;
	}

	inline uint32_t uint32_dec(uint32_t _a)
	{
		return _a - 1;
	}

	inline uint32_t uint32_inc(uint32_t _a)
	{
		return _a + 1;
	}

	inline uint32_t uint32_not(uint32_t _a)
	{
		return ~_a;
	}

	inline uint32_t uint32_neg(uint32_t _a)
	{
		return -(int32_t)_a;
	}

	inline uint32_t uint32_ext(uint32_t _a)
	{
		return ( (int32_t)_a)>>31;
	}

	inline uint32_t uint32_and(uint32_t _a, uint32_t _b)
	{
		return _a & _b;
	}

	inline uint32_t uint32_andc(uint32_t _a, uint32_t _b)
	{
		return _a & ~_b;
	}

	inline uint32_t uint32_xor(uint32_t _a, uint32_t _b)
	{
		return _a ^ _b;
	}

	inline uint32_t uint32_xorl(uint32_t _a, uint32_t _b)
	{
		return !_a != !_b;
	}

	inline uint32_t uint32_or(uint32_t _a, uint32_t _b)
	{
		return _a | _b;
	}

	inline uint32_t uint32_orc(uint32_t _a, uint32_t _b)
	{
		return _a | ~_b;
	}

	inline uint32_t uint32_sll(uint32_t _a, int _sa)
	{
		return _a << _sa;
	}

	inline uint32_t uint32_srl(uint32_t _a, int _sa)
	{
		return _a >> _sa;
	}

	inline uint32_t uint32_sra(uint32_t _a, int _sa)
	{
		return ( (int32_t)_a) >> _sa;
	}

	inline uint32_t uint32_rol(uint32_t _a, int _sa)
	{
		return ( _a << _sa) | (_a >> (32-_sa) );
	}

	inline uint32_t uint32_ror(uint32_t _a, int _sa)
	{
		return ( _a >> _sa) | (_a << (32-_sa) );
	}

	inline uint32_t uint32_add(uint32_t _a, uint32_t _b)
	{
		return _a + _b;
	}

	inline uint32_t uint32_sub(uint32_t _a, uint32_t _b)
	{
		return _a - _b;
	}

	inline uint32_t uint32_mul(uint32_t _a, uint32_t _b)
	{
		return _a * _b;
	}

	inline uint32_t uint32_div(uint32_t _a, uint32_t _b)
	{
		return (_a / _b);
	}

	inline uint32_t uint32_mod(uint32_t _a, uint32_t _b)
	{
		return (_a % _b);
	}

	inline uint32_t uint32_cmpeq(uint32_t _a, uint32_t _b)
	{
		return -(_a == _b);
	}

	inline uint32_t uint32_cmpneq(uint32_t _a, uint32_t _b)
	{
		return -(_a != _b);
	}

	inline uint32_t uint32_cmplt(uint32_t _a, uint32_t _b)
	{
		return -(_a < _b);
	}

	inline uint32_t uint32_cmple(uint32_t _a, uint32_t _b)
	{
		return -(_a <= _b);
	}

	inline uint32_t uint32_cmpgt(uint32_t _a, uint32_t _b)
	{
		return -(_a > _b);
	}

	inline uint32_t uint32_cmpge(uint32_t _a, uint32_t _b)
	{
		return -(_a >= _b);
	}

	inline uint32_t uint32_setnz(uint32_t _a)
	{
		return -!!_a;
	}

	inline uint32_t uint32_satadd(uint32_t _a, uint32_t _b)
	{
		const uint32_t add    = uint32_add(_a, _b);
		const uint32_t lt     = uint32_cmplt(add, _a);
		const uint32_t result = uint32_or(add, lt);

		return result;
	}

	inline uint32_t uint32_satsub(uint32_t _a, uint32_t _b)
	{
		const uint32_t sub    = uint32_sub(_a, _b);
		const uint32_t le     = uint32_cmple(sub, _a);
		const uint32_t result = uint32_and(sub, le);

		return result;
	}

	inline uint32_t uint32_satmul(uint32_t _a, uint32_t _b)
	{
		const uint64_t mul    = (uint64_t)_a * (uint64_t)_b;
		const uint32_t hi     = mul >> 32;
		const uint32_t nz     = uint32_setnz(hi);
		const uint32_t result = uint32_or(uint32_t(mul), nz);

		return result;
	}

	inline uint32_t uint32_sels(uint32_t test, uint32_t _a, uint32_t _b)
	{
		const uint32_t mask   = uint32_ext(test);
		const uint32_t sel_a  = uint32_and(_a, mask);
		const uint32_t sel_b  = uint32_andc(_b, mask);
		const uint32_t result = uint32_or(sel_a, sel_b);

		return (result);
	}

	inline uint32_t uint32_selb(uint32_t _mask, uint32_t _a, uint32_t _b)
	{
		const uint32_t sel_a  = uint32_and(_a, _mask);
		const uint32_t sel_b  = uint32_andc(_b, _mask);
		const uint32_t result = uint32_or(sel_a, sel_b);

		return (result);
	}

	inline uint32_t uint32_imin(uint32_t _a, uint32_t _b)
	{
		const uint32_t a_sub_b = uint32_sub(_a, _b);
		const uint32_t result  = uint32_sels(a_sub_b, _a, _b);

		return result;
	}

	inline uint32_t uint32_imax(uint32_t _a, uint32_t _b)
	{
		const uint32_t b_sub_a = uint32_sub(_b, _a);
		const uint32_t result  = uint32_sels(b_sub_a, _a, _b);

		return result;
	}

	inline uint32_t uint32_min(uint32_t _a, uint32_t _b)
	{
		return _a > _b ? _b : _a;
	}

	inline uint32_t uint32_max(uint32_t _a, uint32_t _b)
	{
		return _a > _b ? _a : _b;
	}

	inline uint32_t uint32_clamp(uint32_t _a, uint32_t _min, uint32_t _max)
	{
		const uint32_t tmp    = uint32_max(_a, _min);
		const uint32_t result = uint32_min(tmp, _max);

		return result;
	}

	inline uint32_t uint32_iclamp(uint32_t _a, uint32_t _min, uint32_t _max)
	{
		const uint32_t tmp    = uint32_imax(_a, _min);
		const uint32_t result = uint32_imin(tmp, _max);

		return result;
	}

	inline uint32_t uint32_incwrap(uint32_t _val, uint32_t _min, uint32_t _max)
	{
		const uint32_t inc          = uint32_inc(_val);
		const uint32_t max_diff     = uint32_sub(_max, _val);
		const uint32_t neg_max_diff = uint32_neg(max_diff);
		const uint32_t max_or       = uint32_or(max_diff, neg_max_diff);
		const uint32_t max_diff_nz  = uint32_ext(max_or);
		const uint32_t result       = uint32_selb(max_diff_nz, inc, _min);

		return result;
	}

	inline uint32_t uint32_decwrap(uint32_t _val, uint32_t _min, uint32_t _max)
	{
		const uint32_t dec          = uint32_dec(_val);
		const uint32_t min_diff     = uint32_sub(_min, _val);
		const uint32_t neg_min_diff = uint32_neg(min_diff);
		const uint32_t min_or       = uint32_or(min_diff, neg_min_diff);
		const uint32_t min_diff_nz  = uint32_ext(min_or);
		const uint32_t result       = uint32_selb(min_diff_nz, dec, _max);

		return result;
	}

	inline uint32_t uint32_cntbits_ref(uint32_t _val)
	{
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

		return result;
	}

	/// Count number of bits set.
	inline uint32_t uint32_cntbits(uint32_t _val)
	{
#if BX_COMPILER_GCC || BX_COMPILER_CLANG
		return __builtin_popcount(_val);
#elif BX_COMPILER_MSVC && BX_PLATFORM_WINDOWS
		return __popcnt(_val);
#else
		return uint32_cntbits_ref(_val);
#endif // BX_COMPILER_
	}

	inline uint32_t uint32_cntlz_ref(uint32_t _val)
	{
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

		return result;
	}

	/// Count number of leading zeros.
	inline uint32_t uint32_cntlz(uint32_t _val)
	{
#if BX_COMPILER_GCC || BX_COMPILER_CLANG
		return __builtin_clz(_val);
#elif BX_COMPILER_MSVC && BX_PLATFORM_WINDOWS
		unsigned long index;
		_BitScanReverse(&index, _val);
		return 31 - index;
#else
		return uint32_cntlz_ref(_val);
#endif // BX_COMPILER_
	}

	inline uint32_t uint32_cnttz_ref(uint32_t _val)
	{
		const uint32_t tmp0   = uint32_not(_val);
		const uint32_t tmp1   = uint32_dec(_val);
		const uint32_t tmp2   = uint32_and(tmp0, tmp1);
		const uint32_t result = uint32_cntbits(tmp2);

		return result;
	}

	inline uint32_t uint32_cnttz(uint32_t _val)
	{
#if BX_COMPILER_GCC || BX_COMPILER_CLANG
		return __builtin_ctz(_val);
#elif BX_COMPILER_MSVC && BX_PLATFORM_WINDOWS
		unsigned long index;
		_BitScanForward(&index, _val);
		return index;
#else
		return uint32_cnttz_ref(_val);
#endif // BX_COMPILER_
	}

	// shuffle:
	// ---- ---- ---- ---- fedc ba98 7654 3210
	// to:
	// -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
	inline uint32_t uint32_part1by1(uint32_t _a)
	{
		const uint32_t val    = uint32_and(_a, 0xffff);

		const uint32_t tmp0   = uint32_sll(val, 8);
		const uint32_t tmp1   = uint32_xor(val, tmp0);
		const uint32_t tmp2   = uint32_and(tmp1, 0x00ff00ff);

		const uint32_t tmp3   = uint32_sll(tmp2, 4);
		const uint32_t tmp4   = uint32_xor(tmp2, tmp3);
		const uint32_t tmp5   = uint32_and(tmp4, 0x0f0f0f0f);

		const uint32_t tmp6   = uint32_sll(tmp5, 2);
		const uint32_t tmp7   = uint32_xor(tmp5, tmp6);
		const uint32_t tmp8   = uint32_and(tmp7, 0x33333333);

		const uint32_t tmp9   = uint32_sll(tmp8, 1);
		const uint32_t tmpA   = uint32_xor(tmp8, tmp9);
		const uint32_t result = uint32_and(tmpA, 0x55555555);

		return result;
	}

	// shuffle:
	// ---- ---- ---- ---- ---- --98 7654 3210
	// to:
	// ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
	inline uint32_t uint32_part1by2(uint32_t _a)
	{
		const uint32_t val    = uint32_and(_a, 0x3ff);

		const uint32_t tmp0   = uint32_sll(val, 16);
		const uint32_t tmp1   = uint32_xor(val, tmp0);
		const uint32_t tmp2   = uint32_and(tmp1, 0xff0000ff);

		const uint32_t tmp3   = uint32_sll(tmp2, 8);
		const uint32_t tmp4   = uint32_xor(tmp2, tmp3);
		const uint32_t tmp5   = uint32_and(tmp4, 0x0300f00f);

		const uint32_t tmp6   = uint32_sll(tmp5, 4);
		const uint32_t tmp7   = uint32_xor(tmp5, tmp6);
		const uint32_t tmp8   = uint32_and(tmp7, 0x030c30c3);

		const uint32_t tmp9   = uint32_sll(tmp8, 2);
		const uint32_t tmpA   = uint32_xor(tmp8, tmp9);
		const uint32_t result = uint32_and(tmpA, 0x09249249);

		return result;
	}

	inline uint32_t uint32_testpow2(uint32_t _a)
	{
		const uint32_t tmp0   = uint32_not(_a);
		const uint32_t tmp1   = uint32_inc(tmp0);
		const uint32_t tmp2   = uint32_and(_a, tmp1);
		const uint32_t tmp3   = uint32_cmpeq(tmp2, _a);
		const uint32_t tmp4   = uint32_cmpneq(_a, 0);
		const uint32_t result = uint32_and(tmp3, tmp4);

		return result;
	}

	inline uint32_t uint32_nextpow2(uint32_t _a)
	{
		const uint32_t tmp0   = uint32_dec(_a);
		const uint32_t tmp1   = uint32_srl(tmp0, 1);
		const uint32_t tmp2   = uint32_or(tmp0, tmp1);
		const uint32_t tmp3   = uint32_srl(tmp2, 2);
		const uint32_t tmp4   = uint32_or(tmp2, tmp3);
		const uint32_t tmp5   = uint32_srl(tmp4, 4);
		const uint32_t tmp6   = uint32_or(tmp4, tmp5);
		const uint32_t tmp7   = uint32_srl(tmp6, 8);
		const uint32_t tmp8   = uint32_or(tmp6, tmp7);
		const uint32_t tmp9   = uint32_srl(tmp8, 16);
		const uint32_t tmpA   = uint32_or(tmp8, tmp9);
		const uint32_t result = uint32_inc(tmpA);

		return result;
	}

	inline uint16_t halfFromFloat(float _a)
	{
		union { uint32_t ui; float flt;	} ftou;
		ftou.flt = _a;

		const uint32_t one                        = uint32_li(0x00000001);
		const uint32_t f_s_mask                   = uint32_li(0x80000000);
		const uint32_t f_e_mask                   = uint32_li(0x7f800000);
		const uint32_t f_m_mask                   = uint32_li(0x007fffff);
		const uint32_t f_m_hidden_bit             = uint32_li(0x00800000);
		const uint32_t f_m_round_bit              = uint32_li(0x00001000);
		const uint32_t f_snan_mask                = uint32_li(0x7fc00000);
		const uint32_t f_e_pos                    = uint32_li(0x00000017);
		const uint32_t h_e_pos                    = uint32_li(0x0000000a);
		const uint32_t h_e_mask                   = uint32_li(0x00007c00);
		const uint32_t h_snan_mask                = uint32_li(0x00007e00);
		const uint32_t h_e_mask_value             = uint32_li(0x0000001f);
		const uint32_t f_h_s_pos_offset           = uint32_li(0x00000010);
		const uint32_t f_h_bias_offset            = uint32_li(0x00000070);
		const uint32_t f_h_m_pos_offset           = uint32_li(0x0000000d);
		const uint32_t h_nan_min                  = uint32_li(0x00007c01);
		const uint32_t f_h_e_biased_flag          = uint32_li(0x0000008f);
		const uint32_t f_s                        = uint32_and(ftou.ui, f_s_mask);
		const uint32_t f_e                        = uint32_and(ftou.ui, f_e_mask);
		const uint16_t h_s              = (uint16_t)uint32_srl(f_s, f_h_s_pos_offset);
		const uint32_t f_m                        = uint32_and(ftou.ui, f_m_mask);
		const uint16_t f_e_amount       = (uint16_t)uint32_srl(f_e, f_e_pos);
		const uint32_t f_e_half_bias              = uint32_sub(f_e_amount, f_h_bias_offset);
		const uint32_t f_snan                     = uint32_and(ftou.ui, f_snan_mask);
		const uint32_t f_m_round_mask             = uint32_and(f_m, f_m_round_bit);
		const uint32_t f_m_round_offset           = uint32_sll(f_m_round_mask, one);
		const uint32_t f_m_rounded                = uint32_add(f_m, f_m_round_offset);
		const uint32_t f_m_denorm_sa              = uint32_sub(one, f_e_half_bias);
		const uint32_t f_m_with_hidden            = uint32_or(f_m_rounded, f_m_hidden_bit);
		const uint32_t f_m_denorm                 = uint32_srl(f_m_with_hidden, f_m_denorm_sa);
		const uint32_t h_m_denorm                 = uint32_srl(f_m_denorm, f_h_m_pos_offset);
		const uint32_t f_m_rounded_overflow       = uint32_and(f_m_rounded, f_m_hidden_bit);
		const uint32_t m_nan                      = uint32_srl(f_m, f_h_m_pos_offset);
		const uint32_t h_em_nan                   = uint32_or(h_e_mask, m_nan);
		const uint32_t h_e_norm_overflow_offset   = uint32_inc(f_e_half_bias);
		const uint32_t h_e_norm_overflow          = uint32_sll(h_e_norm_overflow_offset, h_e_pos);
		const uint32_t h_e_norm                   = uint32_sll(f_e_half_bias, h_e_pos);
		const uint32_t h_m_norm                   = uint32_srl(f_m_rounded, f_h_m_pos_offset);
		const uint32_t h_em_norm                  = uint32_or(h_e_norm, h_m_norm);
		const uint32_t is_h_ndenorm_msb           = uint32_sub(f_h_bias_offset, f_e_amount);
		const uint32_t is_f_e_flagged_msb         = uint32_sub(f_h_e_biased_flag, f_e_half_bias);
		const uint32_t is_h_denorm_msb            = uint32_not(is_h_ndenorm_msb);
		const uint32_t is_f_m_eqz_msb             = uint32_dec(f_m);
		const uint32_t is_h_nan_eqz_msb           = uint32_dec(m_nan);
		const uint32_t is_f_inf_msb               = uint32_and(is_f_e_flagged_msb, is_f_m_eqz_msb);
		const uint32_t is_f_nan_underflow_msb     = uint32_and(is_f_e_flagged_msb, is_h_nan_eqz_msb);
		const uint32_t is_e_overflow_msb          = uint32_sub(h_e_mask_value, f_e_half_bias);
		const uint32_t is_h_inf_msb               = uint32_or(is_e_overflow_msb, is_f_inf_msb);
		const uint32_t is_f_nsnan_msb             = uint32_sub(f_snan, f_snan_mask);
		const uint32_t is_m_norm_overflow_msb     = uint32_neg(f_m_rounded_overflow);
		const uint32_t is_f_snan_msb              = uint32_not(is_f_nsnan_msb);
		const uint32_t h_em_overflow_result       = uint32_sels(is_m_norm_overflow_msb, h_e_norm_overflow, h_em_norm);
		const uint32_t h_em_nan_result            = uint32_sels(is_f_e_flagged_msb, h_em_nan, h_em_overflow_result);
		const uint32_t h_em_nan_underflow_result  = uint32_sels(is_f_nan_underflow_msb, h_nan_min, h_em_nan_result);
		const uint32_t h_em_inf_result            = uint32_sels(is_h_inf_msb, h_e_mask, h_em_nan_underflow_result);
		const uint32_t h_em_denorm_result         = uint32_sels(is_h_denorm_msb, h_m_denorm, h_em_inf_result);
		const uint32_t h_em_snan_result           = uint32_sels(is_f_snan_msb, h_snan_mask, h_em_denorm_result);
		const uint32_t h_result                   = uint32_or(h_s, h_em_snan_result);

		return (uint16_t)(h_result);
	}

	inline float halfToFloat(uint16_t _a)
	{
		const uint32_t h_e_mask              = uint32_li(0x00007c00);
		const uint32_t h_m_mask              = uint32_li(0x000003ff);
		const uint32_t h_s_mask              = uint32_li(0x00008000);
		const uint32_t h_f_s_pos_offset      = uint32_li(0x00000010);
		const uint32_t h_f_e_pos_offset      = uint32_li(0x0000000d);
		const uint32_t h_f_bias_offset       = uint32_li(0x0001c000);
		const uint32_t f_e_mask              = uint32_li(0x7f800000);
		const uint32_t f_m_mask              = uint32_li(0x007fffff);
		const uint32_t h_f_e_denorm_bias     = uint32_li(0x0000007e);
		const uint32_t h_f_m_denorm_sa_bias  = uint32_li(0x00000008);
		const uint32_t f_e_pos               = uint32_li(0x00000017);
		const uint32_t h_e_mask_minus_one    = uint32_li(0x00007bff);
		const uint32_t h_e                   = uint32_and(_a, h_e_mask);
		const uint32_t h_m                   = uint32_and(_a, h_m_mask);
		const uint32_t h_s                   = uint32_and(_a, h_s_mask);
		const uint32_t h_e_f_bias            = uint32_add(h_e, h_f_bias_offset);
		const uint32_t h_m_nlz               = uint32_cntlz(h_m);
		const uint32_t f_s                   = uint32_sll(h_s, h_f_s_pos_offset);
		const uint32_t f_e                   = uint32_sll(h_e_f_bias, h_f_e_pos_offset);
		const uint32_t f_m                   = uint32_sll(h_m, h_f_e_pos_offset);
		const uint32_t f_em                  = uint32_or(f_e, f_m);
		const uint32_t h_f_m_sa              = uint32_sub(h_m_nlz, h_f_m_denorm_sa_bias);
		const uint32_t f_e_denorm_unpacked   = uint32_sub(h_f_e_denorm_bias, h_f_m_sa);
		const uint32_t h_f_m                 = uint32_sll(h_m, h_f_m_sa);
		const uint32_t f_m_denorm            = uint32_and(h_f_m, f_m_mask);
		const uint32_t f_e_denorm            = uint32_sll(f_e_denorm_unpacked, f_e_pos);
		const uint32_t f_em_denorm           = uint32_or(f_e_denorm, f_m_denorm);
		const uint32_t f_em_nan              = uint32_or(f_e_mask, f_m);
		const uint32_t is_e_eqz_msb          = uint32_dec(h_e);
		const uint32_t is_m_nez_msb          = uint32_neg(h_m);
		const uint32_t is_e_flagged_msb      = uint32_sub(h_e_mask_minus_one, h_e);
		const uint32_t is_zero_msb           = uint32_andc(is_e_eqz_msb, is_m_nez_msb);
		const uint32_t is_inf_msb            = uint32_andc(is_e_flagged_msb, is_m_nez_msb);
		const uint32_t is_denorm_msb         = uint32_and(is_m_nez_msb, is_e_eqz_msb);
		const uint32_t is_nan_msb            = uint32_and(is_e_flagged_msb, is_m_nez_msb);
		const uint32_t is_zero               = uint32_ext(is_zero_msb);
		const uint32_t f_zero_result         = uint32_andc(f_em, is_zero);
		const uint32_t f_denorm_result       = uint32_sels(is_denorm_msb, f_em_denorm, f_zero_result);
		const uint32_t f_inf_result          = uint32_sels(is_inf_msb, f_e_mask, f_denorm_result);
		const uint32_t f_nan_result          = uint32_sels(is_nan_msb, f_em_nan, f_inf_result);
		const uint32_t f_result              = uint32_or(f_s, f_nan_result);

		union { uint32_t ui; float flt;	} utof;
		utof.ui = f_result;
		return utof.flt;
	}

	inline uint16_t uint16_min(uint16_t _a, uint16_t _b)
	{
		return _a > _b ? _b : _a;
	}

	inline uint16_t uint16_max(uint16_t _a, uint16_t _b)
	{
		return _a < _b ? _b : _a;
	}

	inline int64_t int64_min(int64_t _a, int64_t _b)
	{
		return _a < _b ? _a : _b;
	}

	inline int64_t int64_max(int64_t _a, int64_t _b)
	{
		return _a > _b ? _a : _b;
	}

	inline int64_t int64_clamp(int64_t _a, int64_t _min, int64_t _max)
	{
		const int64_t min    = int64_min(_a, _max);
		const int64_t result = int64_max(_min, min);

		return result;
	}

	inline uint64_t uint64_cntbits_ref(uint64_t _val)
	{
		const uint32_t lo = uint32_t(_val&UINT32_MAX);
		const uint32_t hi = uint32_t(_val>>32);

		const uint32_t total = bx::uint32_cntbits(lo)
							 + bx::uint32_cntbits(hi);

		return total;
	}

	/// Count number of bits set.
	inline uint64_t uint64_cntbits(uint64_t _val)
	{
#if BX_COMPILER_GCC || BX_COMPILER_CLANG
		return __builtin_popcountll(_val);
#elif BX_COMPILER_MSVC && BX_ARCH_64BIT
		return __popcnt64(_val);
#else
		return uint64_cntbits_ref(_val);
#endif // BX_COMPILER_
	}

	inline uint64_t uint64_cntlz_ref(uint64_t _val)
	{
		return _val & UINT64_C(0xffffffff00000000)
			 ? uint32_cntlz(uint32_t(_val>>32) )
			 : uint32_cntlz(uint32_t(_val) ) + 32
			 ;
	}

	/// Count number of leading zeros.
	inline uint64_t uint64_cntlz(uint64_t _val)
	{
#if BX_COMPILER_GCC || BX_COMPILER_CLANG
		return __builtin_clzll(_val);
#elif BX_COMPILER_MSVC && BX_PLATFORM_WINDOWS && BX_ARCH_64BIT
		unsigned long index;
		_BitScanReverse64(&index, _val);
		return 63 - index;
#else
		return uint64_cntlz_ref(_val);
#endif // BX_COMPILER_
	}

	inline uint64_t uint64_cnttz_ref(uint64_t _val)
	{
		return _val & UINT64_C(0xffffffff)
			? uint32_cnttz(uint32_t(_val) )
			: uint32_cnttz(uint32_t(_val>>32) ) + 32
			;
	}

	inline uint64_t uint64_cnttz(uint64_t _val)
	{
#if BX_COMPILER_GCC || BX_COMPILER_CLANG
		return __builtin_ctzll(_val);
#elif BX_COMPILER_MSVC && BX_PLATFORM_WINDOWS && BX_ARCH_64BIT
		unsigned long index;
		_BitScanForward64(&index, _val);
		return index;
#else
		return uint64_cnttz_ref(_val);
#endif // BX_COMPILER_
	}

	/// Greatest common divisor.
	inline uint32_t uint32_gcd(uint32_t _a, uint32_t _b)
	{
		do
		{
			uint32_t tmp = _a % _b;
			_a = _b;
			_b = tmp;
		}
		while (_b);

		return _a;
	}

	/// Least common multiple.
	inline uint32_t uint32_lcm(uint32_t _a, uint32_t _b)
	{
		return _a * (_b / uint32_gcd(_a, _b) );
	}

	/// Align to arbitrary stride.
	inline uint32_t strideAlign(uint32_t _offset, uint32_t _stride)
	{
		const uint32_t mod    = uint32_mod(_offset, _stride);
		const uint32_t add    = uint32_sub(_stride, mod);
		const uint32_t mask   = uint32_cmpeq(mod, 0);
		const uint32_t tmp    = uint32_selb(mask, 0, add);
		const uint32_t result = uint32_add(_offset, tmp);

		return result;
	}

	/// Align to arbitrary stride and 16-bytes.
	inline uint32_t strideAlign16(uint32_t _offset, uint32_t _stride)
	{
		const uint32_t align  = uint32_lcm(16, _stride);
		const uint32_t mod    = uint32_mod(_offset, align);
		const uint32_t mask   = uint32_cmpeq(mod, 0);
		const uint32_t tmp0   = uint32_selb(mask, 0, align);
		const uint32_t tmp1   = uint32_add(_offset, tmp0);
		const uint32_t result = uint32_sub(tmp1, mod);

		return result;
	}

	/// Align to arbitrary stride and 256-bytes.
	inline uint32_t strideAlign256(uint32_t _offset, uint32_t _stride)
	{
		const uint32_t align  = uint32_lcm(256, _stride);
		const uint32_t mod    = uint32_mod(_offset, align);
		const uint32_t mask   = uint32_cmpeq(mod, 0);
		const uint32_t tmp0   = uint32_selb(mask, 0, align);
		const uint32_t tmp1   = uint32_add(_offset, tmp0);
		const uint32_t result = uint32_sub(tmp1, mod);

		return result;
	}

} // namespace bx

#endif // BX_UINT32_T_H_HEADER_GUARD
