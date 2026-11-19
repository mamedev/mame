// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_CPU_MIPS_PS2VUFLOAT_H
#define MAME_CPU_MIPS_PS2VUFLOAT_H

#pragma once

#include <bit>
#include <cmath>
#include <cstdint>

// VU floating point: exponent 255 is finite, exponent 0 is signed zero, fixed-point overflow saturates
namespace ps2vu
{

enum : uint8_t { FLAG_Z = 1, FLAG_S = 2, FLAG_U = 4, FLAG_O = 8 };

struct fmac_result
{
	uint32_t value;
	uint8_t flags;
	uint8_t sticky;
};

inline fmac_result result(uint32_t value, uint8_t exceptions = 0)
{
	const uint8_t flags = exceptions | ((value >> 31) ? FLAG_S : 0) | ((value & 0x7fffffff) ? 0 : FLAG_Z);
	return { value, flags, flags };
}

// magnitude * 2^scale, rounded towards zero to the VU's 24-bit significand.
inline fmac_result normalize(uint32_t sign, uint64_t magnitude, int scale)
{
	if (!magnitude)
		return result(sign);
	const int top = 63 - std::countl_zero(magnitude);
	const int exponent = top + scale + 127;
	if (exponent < 1)
		return result(sign, FLAG_U);
	if (exponent > 255)
		return result(sign | 0x7fffffff, FLAG_O);
	const uint32_t significand = top > 23 ? magnitude >> (top - 23) : magnitude << (23 - top);
	return result(sign | (uint32_t(exponent) << 23) | (significand & 0x7fffff));
}

inline fmac_result multiply(uint32_t a, uint32_t b)
{
	// TODO: the hardware multiplier's one-ULP errors are not modelled
	const uint32_t sign = (a ^ b) & 0x80000000U;
	const int ea = (a >> 23) & 255, eb = (b >> 23) & 255;
	if (!ea || !eb)
		return result(sign);
	return normalize(sign, uint64_t((a & 0x7fffff) | 0x800000) * ((b & 0x7fffff) | 0x800000), ea + eb - 300);
}

inline fmac_result add(uint32_t a, uint32_t b)
{
	int ea = (a >> 23) & 255, eb = (b >> 23) & 255;
	// one alignment guard bit, no IEEE rounding
	uint64_t ma = ea ? uint64_t((a & 0x7fffff) | 0x800000) << 1 : 0;
	uint64_t mb = eb ? uint64_t((b & 0x7fffff) | 0x800000) << 1 : 0;
	const int exponent = ea > eb ? ea : eb;
	ma = exponent - ea < 64 ? ma >> (exponent - ea) : 0;
	mb = exponent - eb < 64 ? mb >> (exponent - eb) : 0;
	const int64_t sum = ((a >> 31) ? -int64_t(ma) : int64_t(ma)) + ((b >> 31) ? -int64_t(mb) : int64_t(mb));
	const uint32_t sign = sum < 0 ? 0x80000000U : sum == 0 ? (a & b & 0x80000000U) : 0;
	return normalize(sign, sum < 0 ? uint64_t(-sum) : uint64_t(sum), exponent - 151);
}

inline fmac_result madd(uint32_t acc, bool acc_overflow, uint32_t a, uint32_t b, bool subtract = false)
{
	const fmac_result product = multiply(a, b);
	const uint32_t term = product.value ^ (subtract ? 0x80000000U : 0);
	// a product overflow takes precedence, then an overflow held in ACC
	fmac_result sum = (product.flags & FLAG_O) ? result(term, FLAG_O) : (acc_overflow ? result(acc, FLAG_O) : add(acc, term));
	sum.sticky |= product.sticky;
	return sum;
}

inline uint32_t ftoi(uint32_t value, unsigned fractional_bits)
{
	const bool negative = (value & 0x80000000U) != 0;
	const unsigned exponent = (value >> 23) & 0xff;
	const int shift = int(exponent) - 127 + int(fractional_bits);
	if (!exponent || shift < 0)
		return 0;
	if (shift >= 31)
		return negative ? 0x80000000U : 0x7fffffffU;
	const uint32_t significand = (value & 0x7fffff) | 0x800000;
	const uint32_t magnitude = shift >= 23 ? significand << (shift - 23) : significand >> (23 - shift);
	return negative ? 0U - magnitude : magnitude;
}

// Order the original encodings, including signed zero and exponent 255.
inline uint32_t ordered_key(uint32_t value)
{
	return (value & 0x80000000U) ? ~value : value ^ 0x80000000U;
}

inline uint32_t maximum(uint32_t a, uint32_t b)
{
	return ordered_key(a) > ordered_key(b) ? a : b;
}

inline uint32_t minimum(uint32_t a, uint32_t b)
{
	return ordered_key(a) < ordered_key(b) ? a : b;
}

inline double to_double(uint32_t value)
{
	const unsigned exponent = (value >> 23) & 0xff;
	const double magnitude = exponent ? std::ldexp(double((value & 0x7fffff) | 0x800000), int(exponent) - 150) : 0.0;
	return (value & 0x80000000U) ? -magnitude : magnitude;
}

// 24 significant bits with the VU's exponent range; not a bit-exact FDIV
inline uint32_t from_double(double value)
{
	const uint32_t sign = std::signbit(value) ? 0x80000000U : 0;
	if (value == 0.0)
		return sign;
	int exponent;
	const double fraction = std::frexp(std::fabs(value), &exponent);
	if (exponent < -125)
		return sign;
	if (exponent > 129)
		return sign | 0x7fffffff;
	return sign | (uint32_t(exponent + 126) << 23) | (uint32_t(std::ldexp(fraction, 24)) & 0x7fffff);
}

inline void div_flags(uint32_t &status, uint32_t flags)
{
	status = (status & ~0x30U) | flags | (flags << 6);
}

inline uint32_t divide(uint32_t numerator, uint32_t denominator, uint32_t &status)
{
	if (!(denominator & 0x7f800000))
	{
		div_flags(status, (numerator & 0x7f800000) ? 0x20 : 0x10);
		return ((numerator ^ denominator) & 0x80000000U) | 0x7fffffff;
	}
	div_flags(status, 0);
	return from_double(to_double(numerator) / to_double(denominator));
}

inline uint32_t square_root(uint32_t value, uint32_t &status)
{
	const double operand = to_double(value);
	div_flags(status, operand < 0.0 ? 0x10 : 0);
	return from_double(std::sqrt(std::fabs(operand)));
}

} // namespace ps2vu

#endif // MAME_CPU_MIPS_PS2VUFLOAT_H
