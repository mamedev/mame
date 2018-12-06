/*
 * Copyright 2011-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_PIXEL_FORMAT_H_HEADER_GUARD
#define BX_PIXEL_FORMAT_H_HEADER_GUARD

#include "math.h"
#include "uint32_t.h"

namespace bx
{
	///
	struct EncodingType
	{
		enum Enum ///
		{
			Unorm,
			Int,
			Uint,
			Float,
			Snorm,

			Count
		};
	};

	typedef void (*PackFn)(void*, const float*);
	typedef void (*UnpackFn)(float*, const void*);

	///
	uint32_t toUnorm(float _value, float _scale);

	///
	float fromUnorm(uint32_t _value, float _scale);

	///
	int32_t toSnorm(float _value, float _scale);

	///
	float fromSnorm(int32_t _value, float _scale);

	// R8
	void packR8(void* _dst, const float* _src);
	void unpackR8(float* _dst, const void* _src);

	// R8S
	void packR8S(void* _dst, const float* _src);
	void unpackR8S(float* _dst, const void* _src);

	// R8I
	void packR8I(void* _dst, const float* _src);
	void unpackR8I(float* _dst, const void* _src);

	// R8U
	void packR8U(void* _dst, const float* _src);
	void unpackR8U(float* _dst, const void* _src);

	// RG8
	void packRg8(void* _dst, const float* _src);
	void unpackRg8(float* _dst, const void* _src);

	// RG8S
	void packRg8S(void* _dst, const float* _src);
	void unpackRg8S(float* _dst, const void* _src);

	// RG8I
	void packRg8I(void* _dst, const float* _src);
	void unpackRg8I(float* _dst, const void* _src);

	// RG8U
	void packRg8U(void* _dst, const float* _src);
	void unpackRg8U(float* _dst, const void* _src);

	// RGB8
	void packRgb8(void* _dst, const float* _src);
	void unpackRgb8(float* _dst, const void* _src);

	// RGB8S
	void packRgb8S(void* _dst, const float* _src);
	void unpackRgb8S(float* _dst, const void* _src);

	// RGB8I
	void packRgb8I(void* _dst, const float* _src);
	void unpackRgb8I(float* _dst, const void* _src);

	// RGB8U
	void packRgb8U(void* _dst, const float* _src);
	void unpackRgb8U(float* _dst, const void* _src);

	// RGBA8
	void packRgba8(void* _dst, const float* _src);
	void unpackRgba8(float* _dst, const void* _src);

	// BGRA8
	void packBgra8(void* _dst, const float* _src);
	void unpackBgra8(float* _dst, const void* _src);

	// RGBA8S
	void packRgba8S(void* _dst, const float* _src);
	void unpackRgba8S(float* _dst, const void* _src);

	// RGBA8I
	void packRgba8I(void* _dst, const float* _src);
	void unpackRgba8I(float* _dst, const void* _src);

	// RGBA8U
	void packRgba8U(void* _dst, const float* _src);
	void unpackRgba8U(float* _dst, const void* _src);

	// R16
	void packR16(void* _dst, const float* _src);
	void unpackR16(float* _dst, const void* _src);

	// R16S
	void packR16S(void* _dst, const float* _src);
	void unpackR16S(float* _dst, const void* _src);

	// R16I
	void packR16I(void* _dst, const float* _src);
	void unpackR16I(float* _dst, const void* _src);

	// R16U
	void packR16U(void* _dst, const float* _src);
	void unpackR16U(float* _dst, const void* _src);

	// R16F
	void packR16F(void* _dst, const float* _src);
	void unpackR16F(float* _dst, const void* _src);

	// RG16
	void packRg16(void* _dst, const float* _src);
	void unpackRg16(float* _dst, const void* _src);

	// RG16S
	void packRg16S(void* _dst, const float* _src);
	void unpackRg16S(float* _dst, const void* _src);

	// RG16I
	void packRg16I(void* _dst, const float* _src);
	void unpackRg16I(float* _dst, const void* _src);

	// RG16U
	void packRg16U(void* _dst, const float* _src);
	void unpackRg16U(float* _dst, const void* _src);

	// RG16F
	void packRg16F(void* _dst, const float* _src);
	void unpackRg16F(float* _dst, const void* _src);

	// RGBA16
	void packRgba16(void* _dst, const float* _src);
	void unpackRgba16(float* _dst, const void* _src);

	// RGBA16S
	void packRgba16S(void* _dst, const float* _src);
	void unpackRgba16S(float* _dst, const void* _src);

	// RGBA16I
	void packRgba16I(void* _dst, const float* _src);
	void unpackRgba16I(float* _dst, const void* _src);

	// RGBA16U
	void packRgba16U(void* _dst, const float* _src);
	void unpackRgba16U(float* _dst, const void* _src);

	// RGBA16F
	void packRgba16F(void* _dst, const float* _src);
	void unpackRgba16F(float* _dst, const void* _src);

	// R32I
	void packR32I(void* _dst, const float* _src);
	void unpackR32I(float* _dst, const void* _src);

	// R32U
	void packR32U(void* _dst, const float* _src);
	void unpackR32U(float* _dst, const void* _src);

	// R32F
	void packR32F(void* _dst, const float* _src);
	void unpackR32F(float* _dst, const void* _src);

	// RG32I
	void packRg32I(void* _dst, const float* _src);
	void unpackRg32I(float* _dst, const void* _src);

	// RG32U
	void packRg32U(void* _dst, const float* _src);
	void unpackRg32U(float* _dst, const void* _src);

	// RGB9E5F
	void packRgb9E5F(void* _dst, const float* _src);
	void unpackRgb9E5F(float* _dst, const void* _src);

	// RGBA32I
	void packRgba32I(void* _dst, const float* _src);
	void unpackRgba32I(float* _dst, const void* _src);

	// RGBA32U
	void packRgba32U(void* _dst, const float* _src);
	void unpackRgba32U(float* _dst, const void* _src);

	// RGBA32F
	void packRgba32F(void* _dst, const float* _src);
	void unpackRgba32F(float* _dst, const void* _src);

	// R5G6B5
	void packR5G6B5(void* _dst, const float* _src);
	void unpackR5G6B5(float* _dst, const void* _src);

	// RGBA4
	void packRgba4(void* _dst, const float* _src);
	void unpackRgba4(float* _dst, const void* _src);

	// RGBA4
	void packBgra4(void* _dst, const float* _src);
	void unpackBgra4(float* _dst, const void* _src);

	// RGB5A1
	void packRgb5a1(void* _dst, const float* _src);
	void unpackRgb5a1(float* _dst, const void* _src);

	// BGR5A1
	void packBgr5a1(void* _dst, const float* _src);
	void unpackBgr5a1(float* _dst, const void* _src);

	// RGB10A2
	void packRgb10A2(void* _dst, const float* _src);
	void unpackRgb10A2(float* _dst, const void* _src);

	// RG11B10F
	void packRG11B10F(void* _dst, const float* _src);
	void unpackRG11B10F(float* _dst, const void* _src);

	// RG32F
	void packRg32F(void* _dst, const float* _src);
	void unpackRg32F(float* _dst, const void* _src);

} // namespace bx

#include "inline/pixelformat.inl"

#endif // BX_PIXEL_FORMAT_H_HEADER_GUARD
