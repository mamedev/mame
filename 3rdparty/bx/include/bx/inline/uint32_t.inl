/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
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
#	error "Must be included from bx/uint32_t.h"
#endif // BX_UINT32_T_H_HEADER_GUARD

namespace bx
{
	inline BX_CONSTEXPR_FUNC uint32_t uint32_li(uint32_t _a)
	{
		return _a;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_dec(uint32_t _a)
	{
		return _a - 1;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_inc(uint32_t _a)
	{
		return _a + 1;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_not(uint32_t _a)
	{
		return ~_a;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_neg(uint32_t _a)
	{
		return -(int32_t)_a;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_ext(uint32_t _a)
	{
		return ( (int32_t)_a)>>31;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_and(uint32_t _a, uint32_t _b)
	{
		return _a & _b;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_andc(uint32_t _a, uint32_t _b)
	{
		return _a & ~_b;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_xor(uint32_t _a, uint32_t _b)
	{
		return _a ^ _b;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_xorl(uint32_t _a, uint32_t _b)
	{
		return !_a != !_b;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_or(uint32_t _a, uint32_t _b)
	{
		return _a | _b;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_orc(uint32_t _a, uint32_t _b)
	{
		return _a | ~_b;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_sll(uint32_t _a, int32_t _sa)
	{
		return _a << _sa;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_srl(uint32_t _a, int32_t _sa)
	{
		return _a >> _sa;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_sra(uint32_t _a, int32_t _sa)
	{
		return ( (int32_t)_a) >> _sa;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_rol(uint32_t _a, int32_t _sa)
	{
		return ( _a << _sa) | (_a >> (32-_sa) );
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_ror(uint32_t _a, int32_t _sa)
	{
		return ( _a >> _sa) | (_a << (32-_sa) );
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_add(uint32_t _a, uint32_t _b)
	{
		return _a + _b;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_iadd(uint32_t _a, uint32_t _b)
	{
		return int32_t(_a) + int32_t(_b);
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_sub(uint32_t _a, uint32_t _b)
	{
		return _a - _b;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_isub(uint32_t _a, uint32_t _b)
	{
		return int32_t(_a) - int32_t(_b);
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_mul(uint32_t _a, uint32_t _b)
	{
		return _a * _b;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_div(uint32_t _a, uint32_t _b)
	{
		return _a / _b;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_mod(uint32_t _a, uint32_t _b)
	{
		return _a % _b;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_cmpeq(uint32_t _a, uint32_t _b)
	{
		return -(_a == _b);
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_cmpneq(uint32_t _a, uint32_t _b)
	{
		return -(_a != _b);
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_cmplt(uint32_t _a, uint32_t _b)
	{
		return -(_a < _b);
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_cmple(uint32_t _a, uint32_t _b)
	{
		return -(_a <= _b);
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_cmpgt(uint32_t _a, uint32_t _b)
	{
		return -(_a > _b);
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_cmpge(uint32_t _a, uint32_t _b)
	{
		return -(_a >= _b);
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_setnz(uint32_t _a)
	{
		return -!!_a;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_satadd(uint32_t _a, uint32_t _b)
	{
		const uint32_t add    = uint32_add(_a, _b);
		const uint32_t lt     = uint32_cmplt(add, _a);
		const uint32_t result = uint32_or(add, lt);

		return result;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_satsub(uint32_t _a, uint32_t _b)
	{
		const uint32_t sub    = uint32_sub(_a, _b);
		const uint32_t le     = uint32_cmple(sub, _a);
		const uint32_t result = uint32_and(sub, le);

		return result;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_satmul(uint32_t _a, uint32_t _b)
	{
		const uint64_t mul    = (uint64_t)_a * (uint64_t)_b;
		const uint32_t hi     = mul >> 32;
		const uint32_t nz     = uint32_setnz(hi);
		const uint32_t result = uint32_or(uint32_t(mul), nz);

		return result;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_sels(uint32_t test, uint32_t _a, uint32_t _b)
	{
		const uint32_t mask   = uint32_ext(test);
		const uint32_t sel_a  = uint32_and(_a, mask);
		const uint32_t sel_b  = uint32_andc(_b, mask);
		const uint32_t result = uint32_or(sel_a, sel_b);

		return (result);
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_selb(uint32_t _mask, uint32_t _a, uint32_t _b)
	{
		const uint32_t sel_a  = uint32_and(_a, _mask);
		const uint32_t sel_b  = uint32_andc(_b, _mask);
		const uint32_t result = uint32_or(sel_a, sel_b);

		return (result);
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_imin(uint32_t _a, uint32_t _b)
	{
		const uint32_t a_sub_b = uint32_sub(_a, _b);
		const uint32_t result  = uint32_sels(a_sub_b, _a, _b);

		return result;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_imax(uint32_t _a, uint32_t _b)
	{
		const uint32_t b_sub_a = uint32_sub(_b, _a);
		const uint32_t result  = uint32_sels(b_sub_a, _a, _b);

		return result;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_min(uint32_t _a, uint32_t _b)
	{
		return _a > _b ? _b : _a;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_min(uint32_t _a, uint32_t _b, uint32_t _c)
	{
		return uint32_min(_a, uint32_min(_b, _c) );
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_max(uint32_t _a, uint32_t _b)
	{
		return _a > _b ? _a : _b;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_max(uint32_t _a, uint32_t _b, uint32_t _c)
	{
		return uint32_max(_a, uint32_max(_b, _c) );
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_clamp(uint32_t _a, uint32_t _min, uint32_t _max)
	{
		const uint32_t tmp    = uint32_max(_a, _min);
		const uint32_t result = uint32_min(tmp, _max);

		return result;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_iclamp(uint32_t _a, uint32_t _min, uint32_t _max)
	{
		const uint32_t tmp    = uint32_imax(_a, _min);
		const uint32_t result = uint32_imin(tmp, _max);

		return result;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_incwrap(uint32_t _val, uint32_t _min, uint32_t _max)
	{
		const uint32_t inc          = uint32_inc(_val);
		const uint32_t max_diff     = uint32_sub(_max, _val);
		const uint32_t neg_max_diff = uint32_neg(max_diff);
		const uint32_t max_or       = uint32_or(max_diff, neg_max_diff);
		const uint32_t max_diff_nz  = uint32_ext(max_or);
		const uint32_t result       = uint32_selb(max_diff_nz, inc, _min);

		return result;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_decwrap(uint32_t _val, uint32_t _min, uint32_t _max)
	{
		const uint32_t dec          = uint32_dec(_val);
		const uint32_t min_diff     = uint32_sub(_min, _val);
		const uint32_t neg_min_diff = uint32_neg(min_diff);
		const uint32_t min_or       = uint32_or(min_diff, neg_min_diff);
		const uint32_t min_diff_nz  = uint32_ext(min_or);
		const uint32_t result       = uint32_selb(min_diff_nz, dec, _max);

		return result;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_cntbits(uint32_t _val)
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

		return result;
#endif // BX_COMPILER_*
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_cntlz(uint32_t _val)
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

		return result;
#endif // BX_COMPILER_*
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_cnttz(uint32_t _val)
	{
#if BX_COMPILER_GCC || BX_COMPILER_CLANG
		return 0 == _val ? 32 : __builtin_ctz(_val);
#else
		const uint32_t tmp0   = uint32_not(_val);
		const uint32_t tmp1   = uint32_dec(_val);
		const uint32_t tmp2   = uint32_and(tmp0, tmp1);
		const uint32_t result = uint32_cntbits(tmp2);

		return result;
#endif // BX_COMPILER_*
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_ffs(uint32_t _x)
	{
		return 0 == _x ? 0 : uint32_cnttz(_x) + 1;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_part1by1(uint32_t _a)
	{
		// shuffle:
		// ---- ---- ---- ---- fedc ba98 7654 3210
		// to:
		// -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0

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

	inline BX_CONSTEXPR_FUNC uint32_t uint32_part1by2(uint32_t _a)
	{
		// shuffle:
		// ---- ---- ---- ---- ---- --98 7654 3210
		// to:
		// ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0

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

	inline BX_CONSTEXPR_FUNC uint32_t uint32_testpow2(uint32_t _a)
	{
		const uint32_t tmp0   = uint32_dec(_a);
		const uint32_t tmp1   = uint32_xor(_a, tmp0);
		const uint32_t tmp2   = uint32_srl(tmp1, 1);
		const uint32_t result = uint32_cmpeq(tmp2, tmp0);

		return result;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_nextpow2(uint32_t _a)
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

	inline BX_CONSTEXPR_FUNC uint64_t uint64_li(uint64_t _a)
	{
		return _a;
	}

	inline BX_CONSTEXPR_FUNC uint64_t uint64_dec(uint64_t _a)
	{
		return _a - 1;
	}

	inline BX_CONSTEXPR_FUNC uint64_t uint64_inc(uint64_t _a)
	{
		return _a + 1;
	}

	inline BX_CONSTEXPR_FUNC uint64_t uint64_not(uint64_t _a)
	{
		return ~_a;
	}

	inline BX_CONSTEXPR_FUNC uint64_t uint64_neg(uint64_t _a)
	{
		return -(int32_t)_a;
	}

	inline BX_CONSTEXPR_FUNC uint64_t uint64_ext(uint64_t _a)
	{
		return ( (int32_t)_a)>>31;
	}

	inline BX_CONSTEXPR_FUNC uint64_t uint64_and(uint64_t _a, uint64_t _b)
	{
		return _a & _b;
	}

	inline BX_CONSTEXPR_FUNC uint64_t uint64_andc(uint64_t _a, uint64_t _b)
	{
		return _a & ~_b;
	}

	inline BX_CONSTEXPR_FUNC uint64_t uint64_xor(uint64_t _a, uint64_t _b)
	{
		return _a ^ _b;
	}

	inline BX_CONSTEXPR_FUNC uint64_t uint64_xorl(uint64_t _a, uint64_t _b)
	{
		return !_a != !_b;
	}

	inline BX_CONSTEXPR_FUNC uint64_t uint64_or(uint64_t _a, uint64_t _b)
	{
		return _a | _b;
	}

	inline BX_CONSTEXPR_FUNC uint64_t uint64_orc(uint64_t _a, uint64_t _b)
	{
		return _a | ~_b;
	}

	inline BX_CONSTEXPR_FUNC uint64_t uint64_sll(uint64_t _a, int32_t _sa)
	{
		return _a << _sa;
	}

	inline BX_CONSTEXPR_FUNC uint64_t uint64_srl(uint64_t _a, int32_t _sa)
	{
		return _a >> _sa;
	}

	inline BX_CONSTEXPR_FUNC uint64_t uint64_sra(uint64_t _a, int32_t _sa)
	{
		return ( (int64_t)_a) >> _sa;
	}

	inline BX_CONSTEXPR_FUNC uint64_t uint64_rol(uint64_t _a, int32_t _sa)
	{
		return ( _a << _sa) | (_a >> (64-_sa) );
	}

	inline BX_CONSTEXPR_FUNC uint64_t uint64_ror(uint64_t _a, int32_t _sa)
	{
		return ( _a >> _sa) | (_a << (64-_sa) );
	}

	inline BX_CONSTEXPR_FUNC uint64_t uint64_add(uint64_t _a, uint64_t _b)
	{
		return _a + _b;
	}

	inline BX_CONSTEXPR_FUNC uint64_t uint64_sub(uint64_t _a, uint64_t _b)
	{
		return _a - _b;
	}

	inline BX_CONSTEXPR_FUNC uint64_t uint64_mul(uint64_t _a, uint64_t _b)
	{
		return _a * _b;
	}

	inline BX_CONSTEXPR_FUNC uint64_t uint64_cntbits(uint64_t _val)
	{
#if BX_COMPILER_GCC || BX_COMPILER_CLANG
		return __builtin_popcountll(_val);
#else
		const uint32_t lo = uint32_t(_val&UINT32_MAX);
		const uint32_t hi = uint32_t(_val>>32);

		return uint32_cntbits(lo)
			+  uint32_cntbits(hi)
			;
#endif // BX_COMPILER_*
	}

	inline BX_CONSTEXPR_FUNC uint64_t uint64_cntlz(uint64_t _val)
	{
#if BX_COMPILER_GCC || BX_COMPILER_CLANG
		return 0 == _val ? 64 : __builtin_clzll(_val);
#else
		return _val & UINT64_C(0xffffffff00000000)
			 ? uint32_cntlz(uint32_t(_val>>32) )
			 : uint32_cntlz(uint32_t(_val) ) + 32
			 ;
#endif // BX_COMPILER_*
	}

	inline BX_CONSTEXPR_FUNC uint64_t uint64_cnttz(uint64_t _val)
	{
#if BX_COMPILER_GCC || BX_COMPILER_CLANG
		return 0 == _val ? 64 : __builtin_ctzll(_val);
#else
		return _val & UINT64_C(0xffffffff)
			? uint32_cnttz(uint32_t(_val) )
			: uint32_cnttz(uint32_t(_val>>32) ) + 32
			;
#endif // BX_COMPILER_*
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_gcd(uint32_t _a, uint32_t _b)
	{
		do
		{
			const uint32_t tmp = uint32_mod(_a, _b);
			_a = _b;
			_b = tmp;
		}
		while (_b);

		return _a;
	}

	inline BX_CONSTEXPR_FUNC uint32_t uint32_lcm(uint32_t _a, uint32_t _b)
	{
		return _a * (_b / uint32_gcd(_a, _b) );
	}

	inline BX_CONSTEXPR_FUNC uint32_t strideAlign(uint32_t _offset, uint32_t _stride)
	{
		const uint32_t mod    = uint32_mod(_offset, _stride);
		const uint32_t add    = uint32_sub(_stride, mod);
		const uint32_t mask   = uint32_cmpeq(mod, 0);
		const uint32_t tmp    = uint32_selb(mask, 0, add);
		const uint32_t result = uint32_add(_offset, tmp);

		return result;
	}

	template<uint32_t Min>
	inline BX_CONSTEXPR_FUNC uint32_t strideAlign(uint32_t _offset, uint32_t _stride)
	{
		const uint32_t align  = uint32_lcm(Min, _stride);
		const uint32_t mod    = uint32_mod(_offset, align);
		const uint32_t mask   = uint32_cmpeq(mod, 0);
		const uint32_t tmp0   = uint32_selb(mask, 0, align);
		const uint32_t tmp1   = uint32_add(_offset, tmp0);
		const uint32_t result = uint32_sub(tmp1, mod);

		return result;
	}

	template<typename Ty>
	inline BX_CONSTEXPR_FUNC bool isAligned(Ty _a, int32_t _align)
	{
		const Ty mask = Ty(max(1, _align) - 1);
		return 0 == (_a & mask);
	}

	template<typename Ty>
	inline BX_CONSTEXPR_FUNC bool isAligned(Ty* _ptr, int32_t _align)
	{
		union { const void* ptr; uintptr_t addr; } un = { _ptr };
		return isAligned(un.addr, _align);
	}

	template<typename Ty>
	inline BX_CONSTEXPR_FUNC bool isAligned(const Ty* _ptr, int32_t _align)
	{
		union { const void* ptr; uintptr_t addr; } un = { _ptr };
		return isAligned(un.addr, _align);
	}

	template<typename Ty>
	inline BX_CONSTEXPR_FUNC Ty alignDown(Ty _a, int32_t _align)
	{
		const Ty mask = Ty(max(1, _align) - 1);
		return Ty(_a & ~mask);
	}

	template<typename Ty>
	inline BX_CONSTEXPR_FUNC Ty* alignDown(Ty* _ptr, int32_t _align)
	{
		union { Ty* ptr; uintptr_t addr; } un = { _ptr };
		un.addr = alignDown(un.addr, _align);
		return un.ptr;
	}

	template<typename Ty>
	inline BX_CONSTEXPR_FUNC const Ty* alignDown(const Ty* _ptr, int32_t _align)
	{
		union { const Ty* ptr; uintptr_t addr; } un = { _ptr };
		un.addr = alignDown(un.addr, _align);
		return un.ptr;
	}

	template<typename Ty>
	inline BX_CONSTEXPR_FUNC Ty alignUp(Ty _a, int32_t _align)
	{
		const Ty mask = Ty(max(1, _align) - 1);
		return Ty( (_a + mask) & ~mask);
	}

	template<typename Ty>
	inline BX_CONSTEXPR_FUNC Ty* alignUp(Ty* _ptr, int32_t _align)
	{
		union { Ty* ptr; uintptr_t addr; } un = { _ptr };
		un.addr = alignUp(un.addr, _align);
		return un.ptr;
	}

	template<typename Ty>
	inline BX_CONSTEXPR_FUNC const Ty* alignUp(const Ty* _ptr, int32_t _align)
	{
		union { const Ty* ptr; uintptr_t addr; } un = { _ptr };
		un.addr = alignUp(un.addr, _align);
		return un.ptr;
	}

	inline BX_CONST_FUNC uint16_t halfFromFloat(float _a)
	{
		union { uint32_t ui; float flt; } ftou;
		ftou.flt = _a;

		const uint32_t one                       = uint32_li(0x00000001);
		const uint32_t f_s_mask                  = uint32_li(kFloatSignMask);
		const uint32_t f_e_mask                  = uint32_li(kFloatExponentMask);
		const uint32_t f_m_mask                  = uint32_li(kFloatMantissaMask);
		const uint32_t f_m_hidden_bit            = uint32_li(0x00800000);
		const uint32_t f_m_round_bit             = uint32_li(0x00001000);
		const uint32_t f_snan_mask               = uint32_li(0x7fc00000);
		const uint32_t f_e_pos                   = uint32_li(0x00000017);
		const uint32_t h_e_pos                   = uint32_li(0x0000000a);
		const uint32_t h_e_mask                  = uint32_li(kHalfExponentMask);
		const uint32_t h_snan_mask               = uint32_li(0x00007e00);
		const uint32_t h_e_mask_value            = uint32_li(0x0000001f);
		const uint32_t f_h_s_pos_offset          = uint32_li(0x00000010);
		const uint32_t f_h_bias_offset           = uint32_li(0x00000070);
		const uint32_t f_h_m_pos_offset          = uint32_li(0x0000000d);
		const uint32_t h_nan_min                 = uint32_li(0x00007c01);
		const uint32_t f_h_e_biased_flag         = uint32_li(0x0000008f);
		const uint32_t f_s                       = uint32_and(ftou.ui, f_s_mask);
		const uint32_t f_e                       = uint32_and(ftou.ui, f_e_mask);
		const uint16_t h_s                       = (uint16_t)uint32_srl(f_s, f_h_s_pos_offset);
		const uint32_t f_m                       = uint32_and(ftou.ui, f_m_mask);
		const uint16_t f_e_amount                = (uint16_t)uint32_srl(f_e, f_e_pos);
		const uint32_t f_e_half_bias             = uint32_sub(f_e_amount, f_h_bias_offset);
		const uint32_t f_snan                    = uint32_and(ftou.ui, f_snan_mask);
		const uint32_t f_m_round_mask            = uint32_and(f_m, f_m_round_bit);
		const uint32_t f_m_round_offset          = uint32_sll(f_m_round_mask, one);
		const uint32_t f_m_rounded               = uint32_add(f_m, f_m_round_offset);
		const uint32_t f_m_denorm_sa             = uint32_sub(one, f_e_half_bias);
		const uint32_t f_m_with_hidden           = uint32_or(f_m_rounded, f_m_hidden_bit);
		const uint32_t f_m_denorm                = uint32_srl(f_m_with_hidden, f_m_denorm_sa);
		const uint32_t h_m_denorm                = uint32_srl(f_m_denorm, f_h_m_pos_offset);
		const uint32_t f_m_rounded_overflow      = uint32_and(f_m_rounded, f_m_hidden_bit);
		const uint32_t m_nan                     = uint32_srl(f_m, f_h_m_pos_offset);
		const uint32_t h_em_nan                  = uint32_or(h_e_mask, m_nan);
		const uint32_t h_e_norm_overflow_offset  = uint32_inc(f_e_half_bias);
		const uint32_t h_e_norm_overflow         = uint32_sll(h_e_norm_overflow_offset, h_e_pos);
		const uint32_t h_e_norm                  = uint32_sll(f_e_half_bias, h_e_pos);
		const uint32_t h_m_norm                  = uint32_srl(f_m_rounded, f_h_m_pos_offset);
		const uint32_t h_em_norm                 = uint32_or(h_e_norm, h_m_norm);
		const uint32_t is_h_ndenorm_msb          = uint32_sub(f_h_bias_offset, f_e_amount);
		const uint32_t is_f_e_flagged_msb        = uint32_sub(f_h_e_biased_flag, f_e_half_bias);
		const uint32_t is_h_denorm_msb           = uint32_not(is_h_ndenorm_msb);
		const uint32_t is_f_m_eqz_msb            = uint32_dec(f_m);
		const uint32_t is_h_nan_eqz_msb          = uint32_dec(m_nan);
		const uint32_t is_f_inf_msb              = uint32_and(is_f_e_flagged_msb, is_f_m_eqz_msb);
		const uint32_t is_f_nan_underflow_msb    = uint32_and(is_f_e_flagged_msb, is_h_nan_eqz_msb);
		const uint32_t is_e_overflow_msb         = uint32_sub(h_e_mask_value, f_e_half_bias);
		const uint32_t is_h_inf_msb              = uint32_or(is_e_overflow_msb, is_f_inf_msb);
		const uint32_t is_f_nsnan_msb            = uint32_sub(f_snan, f_snan_mask);
		const uint32_t is_m_norm_overflow_msb    = uint32_neg(f_m_rounded_overflow);
		const uint32_t is_f_snan_msb             = uint32_not(is_f_nsnan_msb);
		const uint32_t h_em_overflow_result      = uint32_sels(is_m_norm_overflow_msb, h_e_norm_overflow, h_em_norm);
		const uint32_t h_em_nan_result           = uint32_sels(is_f_e_flagged_msb, h_em_nan, h_em_overflow_result);
		const uint32_t h_em_nan_underflow_result = uint32_sels(is_f_nan_underflow_msb, h_nan_min, h_em_nan_result);
		const uint32_t h_em_inf_result           = uint32_sels(is_h_inf_msb, h_e_mask, h_em_nan_underflow_result);
		const uint32_t h_em_denorm_result        = uint32_sels(is_h_denorm_msb, h_m_denorm, h_em_inf_result);
		const uint32_t h_em_snan_result          = uint32_sels(is_f_snan_msb, h_snan_mask, h_em_denorm_result);
		const uint32_t h_result                  = uint32_or(h_s, h_em_snan_result);

		return (uint16_t)(h_result);
	}

	inline BX_CONST_FUNC float halfToFloat(uint16_t _a)
	{
		const uint32_t h_e_mask             = uint32_li(kHalfExponentMask);
		const uint32_t h_m_mask             = uint32_li(kHalfMantissaMask);
		const uint32_t h_s_mask             = uint32_li(kHalfSignMask);
		const uint32_t h_f_s_pos_offset     = uint32_li(0x00000010);
		const uint32_t h_f_e_pos_offset     = uint32_li(0x0000000d);
		const uint32_t h_f_bias_offset      = uint32_li(0x0001c000);
		const uint32_t f_e_mask             = uint32_li(kFloatExponentMask);
		const uint32_t f_m_mask             = uint32_li(kFloatMantissaMask);
		const uint32_t h_f_e_denorm_bias    = uint32_li(0x0000007e);
		const uint32_t h_f_m_denorm_sa_bias = uint32_li(0x00000008);
		const uint32_t f_e_pos              = uint32_li(0x00000017);
		const uint32_t h_e_mask_minus_one   = uint32_li(0x00007bff);
		const uint32_t h_e                  = uint32_and(_a, h_e_mask);
		const uint32_t h_m                  = uint32_and(_a, h_m_mask);
		const uint32_t h_s                  = uint32_and(_a, h_s_mask);
		const uint32_t h_e_f_bias           = uint32_add(h_e, h_f_bias_offset);
		const uint32_t h_m_nlz              = uint32_cntlz(h_m);
		const uint32_t f_s                  = uint32_sll(h_s, h_f_s_pos_offset);
		const uint32_t f_e                  = uint32_sll(h_e_f_bias, h_f_e_pos_offset);
		const uint32_t f_m                  = uint32_sll(h_m, h_f_e_pos_offset);
		const uint32_t f_em                 = uint32_or(f_e, f_m);
		const uint32_t h_f_m_sa             = uint32_sub(h_m_nlz, h_f_m_denorm_sa_bias);
		const uint32_t f_e_denorm_unpacked  = uint32_sub(h_f_e_denorm_bias, h_f_m_sa);
		const uint32_t h_f_m                = uint32_sll(h_m, h_f_m_sa);
		const uint32_t f_m_denorm           = uint32_and(h_f_m, f_m_mask);
		const uint32_t f_e_denorm           = uint32_sll(f_e_denorm_unpacked, f_e_pos);
		const uint32_t f_em_denorm          = uint32_or(f_e_denorm, f_m_denorm);
		const uint32_t f_em_nan             = uint32_or(f_e_mask, f_m);
		const uint32_t is_e_eqz_msb         = uint32_dec(h_e);
		const uint32_t is_m_nez_msb         = uint32_neg(h_m);
		const uint32_t is_e_flagged_msb     = uint32_sub(h_e_mask_minus_one, h_e);
		const uint32_t is_zero_msb          = uint32_andc(is_e_eqz_msb, is_m_nez_msb);
		const uint32_t is_inf_msb           = uint32_andc(is_e_flagged_msb, is_m_nez_msb);
		const uint32_t is_denorm_msb        = uint32_and(is_m_nez_msb, is_e_eqz_msb);
		const uint32_t is_nan_msb           = uint32_and(is_e_flagged_msb, is_m_nez_msb);
		const uint32_t is_zero              = uint32_ext(is_zero_msb);
		const uint32_t f_zero_result        = uint32_andc(f_em, is_zero);
		const uint32_t f_denorm_result      = uint32_sels(is_denorm_msb, f_em_denorm, f_zero_result);
		const uint32_t f_inf_result         = uint32_sels(is_inf_msb, f_e_mask, f_denorm_result);
		const uint32_t f_nan_result         = uint32_sels(is_nan_msb, f_em_nan, f_inf_result);
		const uint32_t f_result             = uint32_or(f_s, f_nan_result);

		union { uint32_t ui; float flt; } utof;
		utof.ui = f_result;
		return utof.flt;
	}

} // namespace bx
