/*
 * Copyright 2010-2022 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#ifndef BX_ENDIAN_H_HEADER_GUARD
#	error "Must be included from bx/endian.h!"
#endif // BX_ENDIAN_H_HEADER_GUARD

namespace bx
{
	inline int16_t endianSwap(int16_t _in)
	{
		return (int16_t)endianSwap( (uint16_t)_in);
	}

	inline uint16_t endianSwap(uint16_t _in)
	{
		return (_in>>8) | (_in<<8);
	}

	inline int32_t endianSwap(int32_t _in)
	{
		return (int32_t)endianSwap( (uint32_t)_in);
	}

	inline uint32_t endianSwap(uint32_t _in)
	{
		return (  _in            >>24) | (  _in            <<24)
			 | ( (_in&0x00ff0000)>> 8) | ( (_in&0x0000ff00)<< 8)
			 ;
	}

	inline int64_t endianSwap(int64_t _in)
	{
		return (int64_t)endianSwap( (uint64_t)_in);
	}

	inline uint64_t endianSwap(uint64_t _in)
	{
		return   (_in                               >>56) | (  _in                               <<56)
			 | ( (_in&UINT64_C(0x00ff000000000000) )>>40) | ( (_in&UINT64_C(0x000000000000ff00) )<<40)
			 | ( (_in&UINT64_C(0x0000ff0000000000) )>>24) | ( (_in&UINT64_C(0x0000000000ff0000) )<<24)
			 | ( (_in&UINT64_C(0x000000ff00000000) )>> 8) | ( (_in&UINT64_C(0x00000000ff000000) )<< 8)
			 ;
	}

	template <typename Ty>
	inline Ty toLittleEndian(Ty _in)
	{
#if BX_CPU_ENDIAN_BIG
		return endianSwap(_in);
#else
		return _in;
#endif // BX_CPU_ENDIAN_BIG
	}

	template <typename Ty>
	inline Ty toBigEndian(Ty _in)
	{
#if BX_CPU_ENDIAN_LITTLE
		return endianSwap(_in);
#else
		return _in;
#endif // BX_CPU_ENDIAN_LITTLE
	}

	template <typename Ty>
	inline Ty toHostEndian(Ty _in, bool _fromLittleEndian)
	{
#if BX_CPU_ENDIAN_LITTLE
		return _fromLittleEndian ? _in : endianSwap(_in);
#else
		return _fromLittleEndian ? endianSwap(_in) : _in;
#endif // BX_CPU_ENDIAN_LITTLE
	}

} // namespace bx
