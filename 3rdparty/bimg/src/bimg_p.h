/*
 * Copyright 2011-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bimg#license-bsd-2-clause
 */

#ifndef BIMG_P_H_HEADER_GUARD
#define BIMG_P_H_HEADER_GUARD

#include <bimg/bimg.h>
#include <bx/allocator.h>
#include <bx/readerwriter.h>
#include <bx/pixelformat.h>
#include <bx/endian.h>
#include <bx/error.h>
#include <bx/simd_t.h>

#include "config.h"

#define BIMG_CHUNK_MAGIC_TEX BX_MAKEFOURCC('T', 'E', 'X', 0x0)
#define BIMG_CHUNK_MAGIC_GNF BX_MAKEFOURCC('G', 'N', 'F', ' ')

BX_ERROR_RESULT(BIMG_ERROR, BX_MAKEFOURCC('b', 'i', 'm', 'g') );

#ifndef BIMG_CONFIG_ASTC_DECODE
#	define BIMG_CONFIG_ASTC_DECODE 0
#endif // BIMG_CONFIG_ASTC_DECODE

namespace bimg
{
	struct Memory
	{
		uint8_t* data;
		uint32_t size;
	};

	struct TextureCreate
	{
		TextureFormat::Enum m_format;
		uint16_t m_width;
		uint16_t m_height;
		uint16_t m_depth;
		uint16_t m_numLayers;
		uint8_t m_numMips;
		bool m_cubeMap;
		const Memory* m_mem;
	};

	inline uint8_t calcNumMips(bool _hasMips, uint16_t _width, uint16_t _height, uint16_t _depth = 1)
	{
		if (_hasMips)
		{
			const uint32_t max = bx::max(_width, _height, _depth);
			const uint32_t num = 1 + uint32_t(bx::log2((int32_t)max) );

			return uint8_t(num);
		}

		return 1;
	}

	///
	void imageConvert(
		  void* _dst
		, uint32_t _bpp
		, bx::PackFn _pack
		, const void* _src
		, bx::UnpackFn _unpack
		, uint32_t _size
		);

	///
	void imageConvert(
		  void* _dst
		, uint32_t _dstBpp
		, bx::PackFn _pack
		, const void* _src
		, uint32_t _srcBpp
		, bx::UnpackFn _unpack
		, uint32_t _width
		, uint32_t _height
		, uint32_t _srcPitch
		);

	///
	bool imageParseGnf(
		  ImageContainer& _imageContainer
		, bx::ReaderSeekerI* _reader
		, bx::Error* _err
		);

} // namespace bimg

#endif // BIMG_P_H_HEADER_GUARD
