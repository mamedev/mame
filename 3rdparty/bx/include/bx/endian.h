/*
 * Copyright 2010-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_ENDIAN_H_HEADER_GUARD
#define BX_ENDIAN_H_HEADER_GUARD

#include "bx.h"

namespace bx
{
	///
	int16_t endianSwap(int16_t _in);

	///
	uint16_t endianSwap(uint16_t _in);

	///
	int32_t endianSwap(int32_t _in);

	///
	uint32_t endianSwap(uint32_t _in);

	///
	int64_t endianSwap(int64_t _in);

	///
	uint64_t endianSwap(uint64_t _in);

	/// Input argument is encoded as little endian, convert it if neccessary
	/// depending on host CPU endianess.
	template <typename Ty>
	Ty toLittleEndian(const Ty _in);

	/// Input argument is encoded as big endian, convert it if neccessary
	/// depending on host CPU endianess.
	template <typename Ty>
	Ty toBigEndian(const Ty _in);

	/// If _littleEndian is true, converts input argument to from little endian
	/// to host CPU endiness.
	template <typename Ty>
	Ty toHostEndian(const Ty _in, bool _fromLittleEndian);

} // namespace bx

#include "inline/endian.inl"

#endif // BX_ENDIAN_H_HEADER_GUARD
