/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#ifndef BX_UINT32_T_H_HEADER_GUARD
#define BX_UINT32_T_H_HEADER_GUARD

#include "bx.h"

namespace bx
{
	constexpr uint16_t kHalfFloatZero = UINT16_C(0);
	constexpr uint16_t kHalfFloatHalf = UINT16_C(0x3800);
	constexpr uint16_t kHalfFloatOne  = UINT16_C(0x3c00);
	constexpr uint16_t kHalfFloatTwo  = UINT16_C(0x4000);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_li(uint32_t _a);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_dec(uint32_t _a);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_inc(uint32_t _a);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_not(uint32_t _a);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_neg(uint32_t _a);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_ext(uint32_t _a);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_and(uint32_t _a, uint32_t _b);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_andc(uint32_t _a, uint32_t _b);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_xor(uint32_t _a, uint32_t _b);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_xorl(uint32_t _a, uint32_t _b);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_or(uint32_t _a, uint32_t _b);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_orc(uint32_t _a, uint32_t _b);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_sll(uint32_t _a, int32_t _sa);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_srl(uint32_t _a, int32_t _sa);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_sra(uint32_t _a, int32_t _sa);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_rol(uint32_t _a, int32_t _sa);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_ror(uint32_t _a, int32_t _sa);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_add(uint32_t _a, uint32_t _b);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_sub(uint32_t _a, uint32_t _b);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_mul(uint32_t _a, uint32_t _b);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_div(uint32_t _a, uint32_t _b);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_mod(uint32_t _a, uint32_t _b);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_cmpeq(uint32_t _a, uint32_t _b);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_cmpneq(uint32_t _a, uint32_t _b);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_cmplt(uint32_t _a, uint32_t _b);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_cmple(uint32_t _a, uint32_t _b);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_cmpgt(uint32_t _a, uint32_t _b);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_cmpge(uint32_t _a, uint32_t _b);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_setnz(uint32_t _a);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_satadd(uint32_t _a, uint32_t _b);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_satsub(uint32_t _a, uint32_t _b);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_satmul(uint32_t _a, uint32_t _b);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_sels(uint32_t test, uint32_t _a, uint32_t _b);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_selb(uint32_t _mask, uint32_t _a, uint32_t _b);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_imin(uint32_t _a, uint32_t _b);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_imax(uint32_t _a, uint32_t _b);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_min(uint32_t _a, uint32_t _b);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_min(uint32_t _a, uint32_t _b, uint32_t _c);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_max(uint32_t _a, uint32_t _b);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_max(uint32_t _a, uint32_t _b, uint32_t _c);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_clamp(uint32_t _a, uint32_t _min, uint32_t _max);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_iclamp(uint32_t _a, uint32_t _min, uint32_t _max);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_incwrap(uint32_t _val, uint32_t _min, uint32_t _max);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_decwrap(uint32_t _val, uint32_t _min, uint32_t _max);

	/// Count number of bits set.
	///
	BX_CONSTEXPR_FUNC uint32_t uint32_cntbits(uint32_t _val);

	/// Count number of leading zeros.
	///
	BX_CONSTEXPR_FUNC uint32_t uint32_cntlz(uint32_t _val);

	/// Count number of trailing zeros.
	///
	BX_CONSTEXPR_FUNC uint32_t uint32_cnttz(uint32_t _val);

	/// Find first set.
	///
	BX_CONSTEXPR_FUNC uint32_t uint32_ffs(uint32_t _val);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_part1by1(uint32_t _a);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_part1by2(uint32_t _a);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_testpow2(uint32_t _a);

	///
	BX_CONSTEXPR_FUNC uint32_t uint32_nextpow2(uint32_t _a);

	///
	BX_CONSTEXPR_FUNC uint64_t uint64_li(uint64_t _a);

	///
	BX_CONSTEXPR_FUNC uint64_t uint64_dec(uint64_t _a);

	///
	BX_CONSTEXPR_FUNC uint64_t uint64_inc(uint64_t _a);

	///
	BX_CONSTEXPR_FUNC uint64_t uint64_not(uint64_t _a);

	///
	BX_CONSTEXPR_FUNC uint64_t uint64_neg(uint64_t _a);

	///
	BX_CONSTEXPR_FUNC uint64_t uint64_ext(uint64_t _a);

	///
	BX_CONSTEXPR_FUNC uint64_t uint64_and(uint64_t _a, uint64_t _b);

	///
	BX_CONSTEXPR_FUNC uint64_t uint64_andc(uint64_t _a, uint64_t _b);

	///
	BX_CONSTEXPR_FUNC uint64_t uint64_xor(uint64_t _a, uint64_t _b);

	///
	BX_CONSTEXPR_FUNC uint64_t uint64_xorl(uint64_t _a, uint64_t _b);

	///
	BX_CONSTEXPR_FUNC uint64_t uint64_or(uint64_t _a, uint64_t _b);

	///
	BX_CONSTEXPR_FUNC uint64_t uint64_orc(uint64_t _a, uint64_t _b);

	///
	BX_CONSTEXPR_FUNC uint64_t uint64_sll(uint64_t _a, int32_t _sa);

	///
	BX_CONSTEXPR_FUNC uint64_t uint64_srl(uint64_t _a, int32_t _sa);

	///
	BX_CONSTEXPR_FUNC uint64_t uint64_sra(uint64_t _a, int32_t _sa);

	///
	BX_CONSTEXPR_FUNC uint64_t uint64_rol(uint64_t _a, int32_t _sa);

	///
	BX_CONSTEXPR_FUNC uint64_t uint64_ror(uint64_t _a, int32_t _sa);

	///
	BX_CONSTEXPR_FUNC uint64_t uint64_add(uint64_t _a, uint64_t _b);

	///
	BX_CONSTEXPR_FUNC uint64_t uint64_sub(uint64_t _a, uint64_t _b);

	///
	BX_CONSTEXPR_FUNC uint64_t uint64_mul(uint64_t _a, uint64_t _b);

	///
	BX_CONSTEXPR_FUNC uint64_t uint64_cntbits(uint64_t _val);

	///
	BX_CONSTEXPR_FUNC uint64_t uint64_cntlz(uint64_t _val);

	///
	BX_CONSTEXPR_FUNC uint64_t uint64_cnttz(uint64_t _val);

	/// Greatest common divisor.
	///
	BX_CONSTEXPR_FUNC uint32_t uint32_gcd(uint32_t _a, uint32_t _b);

	/// Least common multiple.
	///
	BX_CONSTEXPR_FUNC uint32_t uint32_lcm(uint32_t _a, uint32_t _b);

	/// Align to arbitrary stride.
	///
	BX_CONSTEXPR_FUNC uint32_t strideAlign(uint32_t _offset, uint32_t _stride);

	/// Align to arbitrary stride and Min bytes.
	///
	template<uint32_t Min>
	BX_CONSTEXPR_FUNC uint32_t strideAlign(uint32_t _offset, uint32_t _stride);

	///
	template<typename Ty>
	BX_CONSTEXPR_FUNC bool isAligned(Ty _a, int32_t _align);

	///
	template<typename Ty>
	BX_CONSTEXPR_FUNC bool isAligned(Ty* _ptr, int32_t _align);

	///
	template<typename Ty>
	BX_CONSTEXPR_FUNC bool isAligned(const Ty* _ptr, int32_t _align);

	///
	template<typename Ty>
	BX_CONSTEXPR_FUNC Ty alignDown(Ty _a, int32_t _align);

	///
	template<typename Ty>
	BX_CONSTEXPR_FUNC Ty* alignDown(Ty* _ptr, int32_t _align);

	///
	template<typename Ty>
	BX_CONSTEXPR_FUNC const Ty* alignDown(const Ty* _ptr, int32_t _align);

	///
	template<typename Ty>
	BX_CONSTEXPR_FUNC Ty alignUp(Ty _a, int32_t _align);

	///
	template<typename Ty>
	BX_CONSTEXPR_FUNC Ty* alignUp(Ty* _ptr, int32_t _align);

	///
	template<typename Ty>
	BX_CONSTEXPR_FUNC const Ty* alignUp(const Ty* _ptr, int32_t _align);

	/// Convert float to half-float.
	///
	BX_CONST_FUNC uint16_t halfFromFloat(float _a);

	/// Convert half-float to float.
	///
	BX_CONST_FUNC float halfToFloat(uint16_t _a);

} // namespace bx

#include "inline/uint32_t.inl"

#endif // BX_UINT32_T_H_HEADER_GUARD
