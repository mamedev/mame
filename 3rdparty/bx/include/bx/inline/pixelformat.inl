/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#ifndef BX_PIXEL_FORMAT_H_HEADER_GUARD
#	error "Must be included from bx/pixelformat.h"
#endif // BX_PIXEL_FORMAT_H_HEADER_GUARD

namespace bx
{
	inline uint32_t toUnorm(float _value, float _scale)
	{
		return uint32_t(round(clamp(_value, 0.0f, 1.0f) * _scale) );
	}

	inline float fromUnorm(uint32_t _value, float _scale)
	{
		return float(_value) / _scale;
	}

	inline int32_t toSnorm(float _value, float _scale)
	{
		return int32_t(round(
					clamp(_value, -1.0f, 1.0f) * _scale)
					);
	}

	inline float fromSnorm(int32_t _value, float _scale)
	{
		return max(-1.0f, float(_value) / _scale);
	}

	// A8
	inline void packA8(void* _dst, const float* _src)
	{
		uint8_t* dst = (uint8_t*)_dst;
		dst[0] = uint8_t(toUnorm(_src[3], 255.0f) );
	}

	inline void unpackA8(float* _dst, const void* _src)
	{
		const uint8_t* src = (const uint8_t*)_src;
		const float aa = fromUnorm(src[0], 255.0f);
		_dst[0] = aa;
		_dst[1] = aa;
		_dst[2] = aa;
		_dst[3] = aa;
	}

	// R8
	inline void packR8(void* _dst, const float* _src)
	{
		uint8_t* dst = (uint8_t*)_dst;
		dst[0] = uint8_t(toUnorm(_src[0], 255.0f) );
	}

	inline void unpackR8(float* _dst, const void* _src)
	{
		const uint8_t* src = (const uint8_t*)_src;
		_dst[0] = fromUnorm(src[0], 255.0f);
		_dst[1] = 0.0f;
		_dst[2] = 0.0f;
		_dst[3] = 1.0f;
	}

	// R8S
	inline void packR8S(void* _dst, const float* _src)
	{
		int8_t* dst = (int8_t*)_dst;
		dst[0] = int8_t(toSnorm(_src[0], 127.0f) );
	}

	inline void unpackR8S(float* _dst, const void* _src)
	{
		const int8_t* src = (const int8_t*)_src;
		_dst[0] = fromSnorm(src[0], 127.0f);
		_dst[1] = 0.0f;
		_dst[2] = 0.0f;
		_dst[3] = 1.0f;
	}

	// R8I
	inline void packR8I(void* _dst, const float* _src)
	{
		int8_t* dst = (int8_t*)_dst;
		dst[0] = int8_t(_src[0]);
	}

	inline void unpackR8I(float* _dst, const void* _src)
	{
		const int8_t* src = (const int8_t*)_src;
		_dst[0] = float(src[0]);
		_dst[1] = 0.0f;
		_dst[2] = 0.0f;
		_dst[3] = 1.0f;
	}

	// R8U
	inline void packR8U(void* _dst, const float* _src)
	{
		uint8_t* dst = (uint8_t*)_dst;
		dst[0] = uint8_t(_src[0]);
	}

	inline void unpackR8U(float* _dst, const void* _src)
	{
		const uint8_t* src = (const uint8_t*)_src;
		_dst[0] = float(src[0]);
		_dst[1] = 0.0f;
		_dst[2] = 0.0f;
		_dst[3] = 1.0f;
	}

	// RG8
	inline void packRg8(void* _dst, const float* _src)
	{
		uint8_t* dst = (uint8_t*)_dst;
		dst[0] = uint8_t(toUnorm(_src[0], 255.0f) );
		dst[1] = uint8_t(toUnorm(_src[1], 255.0f) );
	}

	inline void unpackRg8(float* _dst, const void* _src)
	{
		const uint8_t* src = (const uint8_t*)_src;
		_dst[0] = fromUnorm(src[0], 255.0f);
		_dst[1] = fromUnorm(src[1], 255.0f);
		_dst[2] = 0.0f;
		_dst[3] = 1.0f;
	}

	// RG8S
	inline void packRg8S(void* _dst, const float* _src)
	{
		int8_t* dst = (int8_t*)_dst;
		dst[0] = int8_t(toSnorm(_src[0], 127.0f) );
		dst[1] = int8_t(toSnorm(_src[1], 127.0f) );
	}

	inline void unpackRg8S(float* _dst, const void* _src)
	{
		const int8_t* src = (const int8_t*)_src;
		_dst[0] = fromSnorm(src[0], 127.0f);
		_dst[1] = fromSnorm(src[1], 127.0f);
		_dst[2] = 0.0f;
		_dst[3] = 1.0f;
	}

	// RG8I
	inline void packRg8I(void* _dst, const float* _src)
	{
		int8_t* dst = (int8_t*)_dst;
		dst[0] = int8_t(_src[0]);
		dst[1] = int8_t(_src[1]);
	}

	inline void unpackRg8I(float* _dst, const void* _src)
	{
		const int8_t* src = (const int8_t*)_src;
		_dst[0] = float(src[0]);
		_dst[1] = float(src[1]);
		_dst[2] = 0.0f;
		_dst[3] = 1.0f;
	}

	// RG8U
	inline void packRg8U(void* _dst, const float* _src)
	{
		uint8_t* dst = (uint8_t*)_dst;
		dst[0] = uint8_t(_src[0]);
		dst[1] = uint8_t(_src[1]);
	}

	inline void unpackRg8U(float* _dst, const void* _src)
	{
		const uint8_t* src = (const uint8_t*)_src;
		_dst[0] = float(src[0]);
		_dst[1] = float(src[1]);
		_dst[2] = 0.0f;
		_dst[3] = 1.0f;
	}

	// RGB8
	inline void packRgb8(void* _dst, const float* _src)
	{
		uint8_t* dst = (uint8_t*)_dst;
		dst[0] = uint8_t(toUnorm(_src[0], 255.0f) );
		dst[1] = uint8_t(toUnorm(_src[1], 255.0f) );
		dst[2] = uint8_t(toUnorm(_src[2], 255.0f) );
	}

	inline void unpackRgb8(float* _dst, const void* _src)
	{
		const uint8_t* src = (const uint8_t*)_src;
		_dst[0] = fromUnorm(src[0], 255.0f);
		_dst[1] = fromUnorm(src[1], 255.0f);
		_dst[2] = fromUnorm(src[2], 255.0f);
		_dst[3] = 1.0f;
	}

	// RGB8S
	inline void packRgb8S(void* _dst, const float* _src)
	{
		int8_t* dst = (int8_t*)_dst;
		dst[0] = int8_t(toSnorm(_src[0], 127.0f) );
		dst[1] = int8_t(toSnorm(_src[1], 127.0f) );
		dst[2] = int8_t(toSnorm(_src[2], 127.0f) );
	}

	inline void unpackRgb8S(float* _dst, const void* _src)
	{
		const int8_t* src = (const int8_t*)_src;
		_dst[0] = fromSnorm(src[0], 127.0f);
		_dst[1] = fromSnorm(src[1], 127.0f);
		_dst[2] = fromSnorm(src[2], 127.0f);
		_dst[3] = 1.0f;
	}

	// RGB8I
	inline void packRgb8I(void* _dst, const float* _src)
	{
		int8_t* dst = (int8_t*)_dst;
		dst[0] = int8_t(_src[0]);
		dst[1] = int8_t(_src[1]);
		dst[2] = int8_t(_src[2]);
	}

	inline void unpackRgb8I(float* _dst, const void* _src)
	{
		const int8_t* src = (const int8_t*)_src;
		_dst[0] = float(src[0]);
		_dst[1] = float(src[1]);
		_dst[2] = float(src[2]);
		_dst[3] = 1.0f;
	}

	// RGB8U
	inline void packRgb8U(void* _dst, const float* _src)
	{
		uint8_t* dst = (uint8_t*)_dst;
		dst[0] = uint8_t(_src[0]);
		dst[1] = uint8_t(_src[1]);
		dst[2] = uint8_t(_src[2]);
	}

	inline void unpackRgb8U(float* _dst, const void* _src)
	{
		const uint8_t* src = (const uint8_t*)_src;
		_dst[0] = float(src[0]);
		_dst[1] = float(src[1]);
		_dst[2] = float(src[2]);
		_dst[3] = 1.0f;
	}

	// BGRA8
	inline void packBgra8(void* _dst, const float* _src)
	{
		uint8_t* dst = (uint8_t*)_dst;
		dst[2] = uint8_t(toUnorm(_src[0], 255.0f) );
		dst[1] = uint8_t(toUnorm(_src[1], 255.0f) );
		dst[0] = uint8_t(toUnorm(_src[2], 255.0f) );
		dst[3] = uint8_t(toUnorm(_src[3], 255.0f) );
	}

	inline void unpackBgra8(float* _dst, const void* _src)
	{
		const uint8_t* src = (const uint8_t*)_src;
		_dst[0] = fromUnorm(src[2], 255.0f);
		_dst[1] = fromUnorm(src[1], 255.0f);
		_dst[2] = fromUnorm(src[0], 255.0f);
		_dst[3] = fromUnorm(src[3], 255.0f);
	}

	// RGBA8
	inline void packRgba8(void* _dst, const float* _src)
	{
		uint8_t* dst = (uint8_t*)_dst;
		dst[0] = uint8_t(toUnorm(_src[0], 255.0f) );
		dst[1] = uint8_t(toUnorm(_src[1], 255.0f) );
		dst[2] = uint8_t(toUnorm(_src[2], 255.0f) );
		dst[3] = uint8_t(toUnorm(_src[3], 255.0f) );
	}

	inline void unpackRgba8(float* _dst, const void* _src)
	{
		const uint8_t* src = (const uint8_t*)_src;
		_dst[0] = fromUnorm(src[0], 255.0f);
		_dst[1] = fromUnorm(src[1], 255.0f);
		_dst[2] = fromUnorm(src[2], 255.0f);
		_dst[3] = fromUnorm(src[3], 255.0f);
	}

	// RGBA8S
	inline void packRgba8S(void* _dst, const float* _src)
	{
		int8_t* dst = (int8_t*)_dst;
		dst[0] = int8_t(toSnorm(_src[0], 127.0f) );
		dst[1] = int8_t(toSnorm(_src[1], 127.0f) );
		dst[2] = int8_t(toSnorm(_src[2], 127.0f) );
		dst[3] = int8_t(toSnorm(_src[3], 127.0f) );
	}

	inline void unpackRgba8S(float* _dst, const void* _src)
	{
		const int8_t* src = (const int8_t*)_src;
		_dst[0] = fromSnorm(src[0], 127.0f);
		_dst[1] = fromSnorm(src[1], 127.0f);
		_dst[2] = fromSnorm(src[2], 127.0f);
		_dst[3] = fromSnorm(src[3], 127.0f);
	}

	// RGBA8I
	inline void packRgba8I(void* _dst, const float* _src)
	{
		int8_t* dst = (int8_t*)_dst;
		dst[0] = int8_t(_src[0]);
		dst[1] = int8_t(_src[1]);
		dst[2] = int8_t(_src[2]);
		dst[3] = int8_t(_src[3]);
	}

	inline void unpackRgba8I(float* _dst, const void* _src)
	{
		const int8_t* src = (const int8_t*)_src;
		_dst[0] = float(src[0]);
		_dst[1] = float(src[1]);
		_dst[2] = float(src[2]);
		_dst[3] = float(src[3]);
	}

	// RGBA8U
	inline void packRgba8U(void* _dst, const float* _src)
	{
		uint8_t* dst = (uint8_t*)_dst;
		dst[0] = uint8_t(_src[0]);
		dst[1] = uint8_t(_src[1]);
		dst[2] = uint8_t(_src[2]);
		dst[3] = uint8_t(_src[3]);
	}

	inline void unpackRgba8U(float* _dst, const void* _src)
	{
		const uint8_t* src = (const uint8_t*)_src;
		_dst[0] = float(src[0]);
		_dst[1] = float(src[1]);
		_dst[2] = float(src[2]);
		_dst[3] = float(src[3]);
	}

	// R16
	inline void packR16(void* _dst, const float* _src)
	{
		uint16_t* dst = (uint16_t*)_dst;
		dst[0] = uint16_t(toUnorm(_src[0], 65535.0f) );
	}

	inline void unpackR16(float* _dst, const void* _src)
	{
		const uint16_t* src = (const uint16_t*)_src;
		_dst[0] = fromUnorm(src[0], 65535.0f);
		_dst[1] = 0.0f;
		_dst[2] = 0.0f;
		_dst[3] = 1.0f;
	}

	// R16S
	inline void packR16S(void* _dst, const float* _src)
	{
		int16_t* dst = (int16_t*)_dst;
		dst[0] = int16_t(toSnorm(_src[0], 32767.0f) );
	}

	inline void unpackR16S(float* _dst, const void* _src)
	{
		const int16_t* src = (const int16_t*)_src;
		_dst[0] = fromSnorm(src[0], 32767.0f);
		_dst[1] = 0.0f;
		_dst[2] = 0.0f;
		_dst[3] = 1.0f;
	}

	// R16I
	inline void packR16I(void* _dst, const float* _src)
	{
		int16_t* dst = (int16_t*)_dst;
		dst[0] = int16_t(_src[0]);
	}

	inline void unpackR16I(float* _dst, const void* _src)
	{
		const int16_t* src = (const int16_t*)_src;
		_dst[0] = float(src[0]);
		_dst[1] = 0.0f;
		_dst[2] = 0.0f;
		_dst[3] = 1.0f;
	}

	// R16U
	inline void packR16U(void* _dst, const float* _src)
	{
		uint16_t* dst = (uint16_t*)_dst;
		dst[0] = uint16_t(_src[0]);
	}

	inline void unpackR16U(float* _dst, const void* _src)
	{
		const uint16_t* src = (const uint16_t*)_src;
		_dst[0] = float(src[0]);
	}

	// R16F
	inline void packR16F(void* _dst, const float* _src)
	{
		uint16_t* dst = (uint16_t*)_dst;
		dst[0] = halfFromFloat(_src[0]);
	}

	inline void unpackR16F(float* _dst, const void* _src)
	{
		const uint16_t* src = (const uint16_t*)_src;
		_dst[0] = halfToFloat(src[0]);
		_dst[1] = 0.0f;
		_dst[2] = 0.0f;
		_dst[3] = 1.0f;
	}

	// RG16
	inline void packRg16(void* _dst, const float* _src)
	{
		uint16_t* dst = (uint16_t*)_dst;
		dst[0] = uint16_t(toUnorm(_src[0], 65535.0f) );
		dst[1] = uint16_t(toUnorm(_src[1], 65535.0f) );
	}

	inline void unpackRg16(float* _dst, const void* _src)
	{
		const uint16_t* src = (const uint16_t*)_src;
		_dst[0] = fromUnorm(src[0], 65535.0f);
		_dst[1] = fromUnorm(src[1], 65535.0f);
		_dst[2] = 0.0f;
		_dst[3] = 1.0f;
	}

	// RG16S
	inline void packRg16S(void* _dst, const float* _src)
	{
		int16_t* dst = (int16_t*)_dst;
		dst[0] = int16_t(toSnorm(_src[0], 32767.0f) );
		dst[1] = int16_t(toSnorm(_src[1], 32767.0f) );
	}

	inline void unpackRg16S(float* _dst, const void* _src)
	{
		const int16_t* src = (const int16_t*)_src;
		_dst[0] = fromSnorm(src[0], 32767.0f);
		_dst[1] = fromSnorm(src[1], 32767.0f);
		_dst[2] = 0.0f;
		_dst[3] = 1.0f;
	}

	// RG16I
	inline void packRg16I(void* _dst, const float* _src)
	{
		int16_t* dst = (int16_t*)_dst;
		dst[0] = int16_t(_src[0]);
		dst[1] = int16_t(_src[1]);
	}

	inline void unpackRg16I(float* _dst, const void* _src)
	{
		const int16_t* src = (const int16_t*)_src;
		_dst[0] = float(src[0]);
		_dst[1] = float(src[1]);
		_dst[2] = 0.0f;
		_dst[3] = 1.0f;
	}

	// RG16U
	inline void packRg16U(void* _dst, const float* _src)
	{
		uint16_t* dst = (uint16_t*)_dst;
		dst[0] = uint16_t(_src[0]);
		dst[1] = uint16_t(_src[1]);
	}

	inline void unpackRg16U(float* _dst, const void* _src)
	{
		const uint16_t* src = (const uint16_t*)_src;
		_dst[0] = float(src[0]);
		_dst[1] = float(src[1]);
		_dst[2] = 0.0f;
		_dst[3] = 1.0f;
	}

	// RG16F
	inline void packRg16F(void* _dst, const float* _src)
	{
		uint16_t* dst = (uint16_t*)_dst;
		dst[0] = halfFromFloat(_src[0]);
		dst[1] = halfFromFloat(_src[1]);
	}

	inline void unpackRg16F(float* _dst, const void* _src)
	{
		const uint16_t* src = (const uint16_t*)_src;
		_dst[0] = halfToFloat(src[0]);
		_dst[1] = halfToFloat(src[1]);
		_dst[2] = 0.0f;
		_dst[3] = 1.0f;
	}

	// RGBA16
	inline void packRgba16(void* _dst, const float* _src)
	{
		uint16_t* dst = (uint16_t*)_dst;
		dst[0] = uint16_t(toUnorm(_src[0], 65535.0f) );
		dst[1] = uint16_t(toUnorm(_src[1], 65535.0f) );
		dst[2] = uint16_t(toUnorm(_src[2], 65535.0f) );
		dst[3] = uint16_t(toUnorm(_src[3], 65535.0f) );
	}

	inline void unpackRgba16(float* _dst, const void* _src)
	{
		const uint16_t* src = (const uint16_t*)_src;
		_dst[0] = fromUnorm(src[0], 65535.0f);
		_dst[1] = fromUnorm(src[1], 65535.0f);
		_dst[2] = fromUnorm(src[2], 65535.0f);
		_dst[3] = fromUnorm(src[3], 65535.0f);
	}

	// RGBA16S
	inline void packRgba16S(void* _dst, const float* _src)
	{
		int16_t* dst = (int16_t*)_dst;
		dst[0] = int16_t(toSnorm(_src[0], 32767.0f) );
		dst[1] = int16_t(toSnorm(_src[1], 32767.0f) );
		dst[2] = int16_t(toSnorm(_src[2], 32767.0f) );
		dst[3] = int16_t(toSnorm(_src[3], 32767.0f) );
	}

	inline void unpackRgba16S(float* _dst, const void* _src)
	{
		const int16_t* src = (const int16_t*)_src;
		_dst[0] = fromSnorm(src[0], 32767.0f);
		_dst[1] = fromSnorm(src[1], 32767.0f);
		_dst[2] = fromSnorm(src[2], 32767.0f);
		_dst[3] = fromSnorm(src[3], 32767.0f);
	}

	// RGBA16I
	inline void packRgba16I(void* _dst, const float* _src)
	{
		int16_t* dst = (int16_t*)_dst;
		dst[0] = int16_t(_src[0]);
		dst[1] = int16_t(_src[1]);
		dst[2] = int16_t(_src[2]);
		dst[3] = int16_t(_src[3]);
	}

	inline void unpackRgba16I(float* _dst, const void* _src)
	{
		const int16_t* src = (const int16_t*)_src;
		_dst[0] = float(src[0]);
		_dst[1] = float(src[1]);
		_dst[2] = float(src[2]);
		_dst[3] = float(src[3]);
	}

	// RGBA16U
	inline void packRgba16U(void* _dst, const float* _src)
	{
		uint16_t* dst = (uint16_t*)_dst;
		dst[0] = uint16_t(_src[0]);
		dst[1] = uint16_t(_src[1]);
		dst[2] = uint16_t(_src[2]);
		dst[3] = uint16_t(_src[3]);
	}

	inline void unpackRgba16U(float* _dst, const void* _src)
	{
		const uint16_t* src = (const uint16_t*)_src;
		_dst[0] = float(src[0]);
		_dst[1] = float(src[1]);
		_dst[2] = float(src[2]);
		_dst[3] = float(src[3]);
	}

	// RGBA16F
	inline void packRgba16F(void* _dst, const float* _src)
	{
		uint16_t* dst = (uint16_t*)_dst;
		dst[0] = halfFromFloat(_src[0]);
		dst[1] = halfFromFloat(_src[1]);
		dst[2] = halfFromFloat(_src[2]);
		dst[3] = halfFromFloat(_src[3]);
	}

	inline void unpackRgba16F(float* _dst, const void* _src)
	{
		const uint16_t* src = (const uint16_t*)_src;
		_dst[0] = halfToFloat(src[0]);
		_dst[1] = halfToFloat(src[1]);
		_dst[2] = halfToFloat(src[2]);
		_dst[3] = halfToFloat(src[3]);
	}

	// R24
	inline void packR24(void* _dst, const float* _src)
	{
		uint8_t* dst = (uint8_t*)_dst;
		const uint32_t rr = uint32_t(toUnorm(_src[0], 16777216.0f) );
		dst[0] = uint8_t(rr    );
		dst[1] = uint8_t(rr>> 8);
		dst[2] = uint8_t(rr>>16);
	}

	inline void unpackR24(float* _dst, const void* _src)
	{
		const uint8_t* src = (const uint8_t*)_src;
		const uint32_t rr = 0
			| (src[0]    )
			| (src[1]<< 8)
			| (src[2]<<16)
			;

		_dst[0] = fromUnorm(rr, 16777216.0f);
		_dst[1] = 0.0f;
		_dst[2] = 0.0f;
		_dst[3] = 1.0f;
	}

	// R24G8
	inline void packR24G8(void* _dst, const float* _src)
	{
		uint8_t* dst = (uint8_t*)_dst;
		const uint32_t rr = uint32_t(toUnorm(_src[0], 16777216.0f) );
		dst[0] = uint8_t(rr    );
		dst[1] = uint8_t(rr>> 8);
		dst[2] = uint8_t(rr>>16);
		dst[3] = uint8_t(toUnorm(_src[1], 255.0f) );
	}

	inline void unpackR24G8(float* _dst, const void* _src)
	{
		const uint8_t* src = (const uint8_t*)_src;
		const uint32_t rr = 0
			| (src[0]    )
			| (src[1]<< 8)
			| (src[2]<<16)
			;

		_dst[0] = fromUnorm(rr, 16777216.0f);
		_dst[1] = fromUnorm(src[3], 255.0f);
		_dst[2] = 0.0f;
		_dst[3] = 1.0f;
	}

	// R32I
	inline void packR32I(void* _dst, const float* _src)
	{
		memCopy(_dst, _src, 4);
	}

	inline void unpackR32I(float* _dst, const void* _src)
	{
		memCopy(_dst, _src, 4);
	}

	// R32U
	inline void packR32U(void* _dst, const float* _src)
	{
		memCopy(_dst, _src, 4);
	}

	inline void unpackR32U(float* _dst, const void* _src)
	{
		memCopy(_dst, _src, 4);
	}

	// R32F
	inline void packR32F(void* _dst, const float* _src)
	{
		memCopy(_dst, _src, 4);
	}

	inline void unpackR32F(float* _dst, const void* _src)
	{
		memCopy(_dst, _src, 4);
	}

	// RG32I
	inline void packRg32I(void* _dst, const float* _src)
	{
		memCopy(_dst, _src, 8);
	}

	inline void unpackRg32I(float* _dst, const void* _src)
	{
		memCopy(_dst, _src, 8);
	}

	// RG32U
	inline void packRg32U(void* _dst, const float* _src)
	{
		memCopy(_dst, _src, 8);
	}

	inline void unpackRg32U(float* _dst, const void* _src)
	{
		memCopy(_dst, _src, 8);
	}

	// RG32F
	inline void packRg32F(void* _dst, const float* _src)
	{
		memCopy(_dst, _src, 8);
	}

	inline void unpackRg32F(float* _dst, const void* _src)
	{
		memCopy(_dst, _src, 8);
	}

	template<int32_t MantissaBits, int32_t ExpBits>
	inline void encodeRgbE(float* _dst, const float* _src)
	{
		// Reference(s):
		// - https://web.archive.org/web/20181126040035/https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_texture_shared_exponent.txt
		//
		const int32_t expMax  = (1<<ExpBits) - 1;
		const int32_t expBias = (1<<(ExpBits - 1) ) - 1;
		const float   sharedExpMax = float(expMax) / float(expMax + 1) * float(1 << (expMax - expBias) );

		const float rr = clamp(_src[0], 0.0f, sharedExpMax);
		const float gg = clamp(_src[1], 0.0f, sharedExpMax);
		const float bb = clamp(_src[2], 0.0f, sharedExpMax);
		const float mm = max(rr, gg, bb);
		union { float ff; uint32_t ui; } cast = { mm };
		int32_t expShared = int32_t(uint32_imax(uint32_t(-expBias-1), ( ( (cast.ui>>23) & 0xff) - 127) ) ) + 1 + expBias;
		float denom = pow(2.0f, float(expShared - expBias - MantissaBits) );

		if ( (1<<MantissaBits) == int32_t(round(mm/denom) ) )
		{
			denom *= 2.0f;
			++expShared;
		}

		const float invDenom = 1.0f/denom;
		_dst[0] = round(rr * invDenom);
		_dst[1] = round(gg * invDenom);
		_dst[2] = round(bb * invDenom);
		_dst[3] = float(expShared);
	}

	template<int32_t MantissaBits, int32_t ExpBits>
	inline void decodeRgbE(float* _dst, const float* _src)
	{
		const int32_t expBias = (1<<(ExpBits - 1) ) - 1;
		const float exponent  = _src[3]-float(expBias-MantissaBits);
		const float scale     = pow(2.0f, exponent);
		_dst[0] = _src[0] * scale;
		_dst[1] = _src[1] * scale;
		_dst[2] = _src[2] * scale;
	}

	// RGB9E5F
	inline void packRgb9E5F(void* _dst, const float* _src)
	{
		float tmp[4];
		encodeRgbE<9, 5>(tmp, _src);

		*( (uint32_t*)_dst) = 0
			| (uint32_t(tmp[0])     )
			| (uint32_t(tmp[1]) << 9)
			| (uint32_t(tmp[2]) <<18)
			| (uint32_t(tmp[3]) <<27)
			;
	}

	inline void unpackRgb9E5F(float* _dst, const void* _src)
	{
		uint32_t packed = *( (const uint32_t*)_src);

		float tmp[4];
		tmp[0] = float( ( (packed    ) & 0x1ff) ) / 511.0f;
		tmp[1] = float( ( (packed>> 9) & 0x1ff) ) / 511.0f;
		tmp[2] = float( ( (packed>>18) & 0x1ff) ) / 511.0f;
		tmp[3] = float( ( (packed>>27) &  0x1f) );

		decodeRgbE<9, 5>(_dst, tmp);
	}

	// RGBA32I
	inline void packRgba32I(void* _dst, const float* _src)
	{
		memCopy(_dst, _src, 16);
	}

	inline void unpackRgba32I(float* _dst, const void* _src)
	{
		memCopy(_dst, _src, 16);
	}

	// RGBA32U
	inline void packRgba32U(void* _dst, const float* _src)
	{
		memCopy(_dst, _src, 16);
	}

	inline void unpackRgba32U(float* _dst, const void* _src)
	{
		memCopy(_dst, _src, 16);
	}

	// RGBA32F
	inline void packRgba32F(void* _dst, const float* _src)
	{
		memCopy(_dst, _src, 16);
	}

	inline void unpackRgba32F(float* _dst, const void* _src)
	{
		memCopy(_dst, _src, 16);
	}

	// B5G6R5
	inline void packB5G6R5(void* _dst, const float* _src)
	{
		*( (uint16_t*)_dst) = 0
			| uint16_t(toUnorm(_src[0], 31.0f)<<11)
			| uint16_t(toUnorm(_src[1], 63.0f)<< 5)
			| uint16_t(toUnorm(_src[2], 31.0f)    )
			;
	}

	inline void unpackB5G6R5(float* _dst, const void* _src)
	{
		uint16_t packed = *( (const uint16_t*)_src);
		_dst[0] = float( ( (packed>>11) & 0x1f) ) / 31.0f;
		_dst[1] = float( ( (packed>> 5) & 0x3f) ) / 63.0f;
		_dst[2] = float( ( (packed    ) & 0x1f) ) / 31.0f;
		_dst[3] = 1.0f;
	}

	// R5G6B5
	inline void packR5G6B5(void* _dst, const float* _src)
	{
		*( (uint16_t*)_dst) = 0
			| uint16_t(toUnorm(_src[0], 31.0f)    )
			| uint16_t(toUnorm(_src[1], 63.0f)<< 5)
			| uint16_t(toUnorm(_src[2], 31.0f)<<11)
			;
	}

	inline void unpackR5G6B5(float* _dst, const void* _src)
	{
		uint16_t packed = *( (const uint16_t*)_src);
		_dst[0] = float( ( (packed    ) & 0x1f) ) / 31.0f;
		_dst[1] = float( ( (packed>> 5) & 0x3f) ) / 63.0f;
		_dst[2] = float( ( (packed>>11) & 0x1f) ) / 31.0f;
		_dst[3] = 1.0f;
	}

	// RGBA4
	inline void packRgba4(void* _dst, const float* _src)
	{
		*( (uint16_t*)_dst) = 0
			| uint16_t(toUnorm(_src[0], 15.0f)    )
			| uint16_t(toUnorm(_src[1], 15.0f)<< 4)
			| uint16_t(toUnorm(_src[2], 15.0f)<< 8)
			| uint16_t(toUnorm(_src[3], 15.0f)<<12)
			;
	}

	inline void unpackRgba4(float* _dst, const void* _src)
	{
		uint16_t packed = *( (const uint16_t*)_src);
		_dst[0] = float( ( (packed    ) & 0xf) ) / 15.0f;
		_dst[1] = float( ( (packed>> 4) & 0xf) ) / 15.0f;
		_dst[2] = float( ( (packed>> 8) & 0xf) ) / 15.0f;
		_dst[3] = float( ( (packed>>12) & 0xf) ) / 15.0f;
	}

	// BGRA4
	inline void packBgra4(void* _dst, const float* _src)
	{
		*( (uint16_t*)_dst) = 0
			| uint16_t(toUnorm(_src[0], 15.0f)<< 8)
			| uint16_t(toUnorm(_src[1], 15.0f)<< 4)
			| uint16_t(toUnorm(_src[2], 15.0f)    )
			| uint16_t(toUnorm(_src[3], 15.0f)<<12)
			;
	}

	inline void unpackBgra4(float* _dst, const void* _src)
	{
		uint16_t packed = *( (const uint16_t*)_src);
		_dst[0] = float( ( (packed>> 8) & 0xf) ) / 15.0f;
		_dst[1] = float( ( (packed>> 4) & 0xf) ) / 15.0f;
		_dst[2] = float( ( (packed    ) & 0xf) ) / 15.0f;
		_dst[3] = float( ( (packed>>12) & 0xf) ) / 15.0f;
	}

	// RGB5A1
	inline void packRgb5a1(void* _dst, const float* _src)
	{
		*( (uint16_t*)_dst) = 0
			| uint16_t(toUnorm(_src[0], 31.0f)    )
			| uint16_t(toUnorm(_src[1], 31.0f)<< 5)
			| uint16_t(toUnorm(_src[2], 31.0f)<<10)
			| uint16_t(toUnorm(_src[3],  1.0f)<<15)
			;
	}

	inline void unpackRgb5a1(float* _dst, const void* _src)
	{
		uint16_t packed = *( (const uint16_t*)_src);
		_dst[0] = float( ( (packed    ) & 0x1f) ) / 31.0f;
		_dst[1] = float( ( (packed>> 5) & 0x1f) ) / 31.0f;
		_dst[2] = float( ( (packed>>10) & 0x1f) ) / 31.0f;
		_dst[3] = float( ( (packed>>15) &  0x1) );
	}

	// BGR5A1
	inline void packBgr5a1(void* _dst, const float* _src)
	{
		*( (uint16_t*)_dst) = 0
			| uint16_t(toUnorm(_src[0], 31.0f)<<10)
			| uint16_t(toUnorm(_src[1], 31.0f)<< 5)
			| uint16_t(toUnorm(_src[2], 31.0f)    )
			| uint16_t(toUnorm(_src[3],  1.0f)<<15)
			;
	}

	inline void unpackBgr5a1(float* _dst, const void* _src)
	{
		uint16_t packed = *( (const uint16_t*)_src);
		_dst[0] = float( ( (packed>>10) & 0x1f) ) / 31.0f;
		_dst[1] = float( ( (packed>> 5) & 0x1f) ) / 31.0f;
		_dst[2] = float( ( (packed    ) & 0x1f) ) / 31.0f;
		_dst[3] = float( ( (packed>>15) &  0x1) );
	}

	// RGB10A2
	inline void packRgb10A2(void* _dst, const float* _src)
	{
		*( (uint32_t*)_dst) = 0
			| (toUnorm(_src[0], 1023.0f)    )
			| (toUnorm(_src[1], 1023.0f)<<10)
			| (toUnorm(_src[2], 1023.0f)<<20)
			| (toUnorm(_src[3],    3.0f)<<30)
			;
	}

	inline void unpackRgb10A2(float* _dst, const void* _src)
	{
		uint32_t packed = *( (const uint32_t*)_src);
		_dst[0] = float( ( (packed    ) & 0x3ff) ) / 1023.0f;
		_dst[1] = float( ( (packed>>10) & 0x3ff) ) / 1023.0f;
		_dst[2] = float( ( (packed>>20) & 0x3ff) ) / 1023.0f;
		_dst[3] = float( ( (packed>>30) &   0x3) ) /    3.0f;
	}

	// RG11B10F
	inline void packRG11B10F(void* _dst, const float* _src)
	{
		*( (uint32_t*)_dst) = 0
			| ( (halfFromFloat(_src[0])>> 4) &      0x7ff)
			| ( (halfFromFloat(_src[1])<< 7) &   0x3ff800)
			| ( (halfFromFloat(_src[2])<<17) & 0xffc00000)
			;
	}

	inline void unpackRG11B10F(float* _dst, const void* _src)
	{
		uint32_t packed = *( (const uint32_t*)_src);
		_dst[0] = halfToFloat( (packed<< 4) & 0x7ff0);
		_dst[1] = halfToFloat( (packed>> 7) & 0x7ff0);
		_dst[2] = halfToFloat( (packed>>17) & 0x7fe0);
		_dst[3] = 1.0f;
	}
} // namespace bx
