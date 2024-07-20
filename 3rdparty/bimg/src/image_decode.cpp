/*
 * Copyright 2011-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bimg/blob/master/LICENSE
 */

#include "bimg_p.h"

BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wunused-function")

BX_PRAGMA_DIAGNOSTIC_PUSH()
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wtype-limits")
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wunused-parameter")
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wunused-value")
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG("-Wdeprecated-declarations")
BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4018) // warning C4018:  '<': signed/unsigned mismatch
BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4100) // error C4100: '' : unreferenced formal parameter
BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4389) // warning C4389 : '==' : signed / unsigned mismatch
BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4505) // warning C4505: 'tinyexr::miniz::def_realloc_func': unreferenced local function has been removed
#define MINIZ_NO_ARCHIVE_APIS
#define MINIZ_NO_STDIO
#define TINYEXR_IMPLEMENTATION
#include <tinyexr/tinyexr.h>
BX_PRAGMA_DIAGNOSTIC_POP()

BX_PRAGMA_DIAGNOSTIC_PUSH();
BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4127) // warning C4127: conditional expression is constant
BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4267) // warning C4267: '=' : conversion from 'size_t' to 'unsigned short', possible loss of data
BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4334) // warning C4334: '<<' : result of 32 - bit shift implicitly converted to 64 bits(was 64 - bit shift intended ? )
#define LODEPNG_NO_COMPILE_ENCODER
#define LODEPNG_NO_COMPILE_DISK
#define LODEPNG_NO_COMPILE_ANCILLARY_CHUNKS
#define LODEPNG_NO_COMPILE_ALLOCATORS
#define LODEPNG_NO_COMPILE_CPP
#include <lodepng/lodepng.cpp>
BX_PRAGMA_DIAGNOSTIC_POP();

#if BIMG_DECODE_HEIF
#	include <libheif/heif.h>
#endif // BIMG_DECODE_HEIF

void* lodepng_malloc(size_t _size)
{
	return ::malloc(_size);
}

void* lodepng_realloc(void* _ptr, size_t _size)
{
	return ::realloc(_ptr, _size);
}

void lodepng_free(void* _ptr)
{
	::free(_ptr);
}

BX_PRAGMA_DIAGNOSTIC_PUSH();
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wint-to-pointer-cast")
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wmissing-field-initializers");
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wshadow");
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wsign-compare");
BX_PRAGMA_DIAGNOSTIC_IGNORED_GCC("-Wunused-but-set-variable");
BX_PRAGMA_DIAGNOSTIC_IGNORED_GCC("-Warray-bounds");
#if BX_COMPILER_GCC >= 60000
BX_PRAGMA_DIAGNOSTIC_IGNORED_GCC("-Wmisleading-indentation");
BX_PRAGMA_DIAGNOSTIC_IGNORED_GCC("-Wshift-negative-value");
#	if BX_COMPILER_GCC >= 70000
BX_PRAGMA_DIAGNOSTIC_IGNORED_GCC("-Wimplicit-fallthrough");
#	endif // BX_COMPILER_GCC >= 70000
#endif // BX_COMPILER_GCC >= 60000_
#define STBI_MALLOC(_size)        lodepng_malloc(_size)
#define STBI_REALLOC(_ptr, _size) lodepng_realloc(_ptr, _size)
#define STBI_FREE(_ptr)           lodepng_free(_ptr)
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <stb/stb_image.h>
BX_PRAGMA_DIAGNOSTIC_POP();

namespace bimg
{
	static ImageContainer* imageParseLodePng(bx::AllocatorI* _allocator, const void* _data, uint32_t _size, bx::Error* _err)
	{
		BX_ERROR_SCOPE(_err);

		static uint8_t pngMagic[] = { 0x89, 0x50, 0x4E, 0x47, 0x0d, 0x0a };

		if (0 != bx::memCmp(_data, pngMagic, sizeof(pngMagic) ) )
		{
			return NULL;
		}

		ImageContainer* output = NULL;
		bimg::TextureFormat::Enum format = bimg::TextureFormat::RGBA8;
		uint32_t width  = 0;
		uint32_t height = 0;

		LodePNGState state;
		lodepng_state_init(&state);
		state.decoder.color_convert = 0;

		uint8_t* data = NULL;
		const uint32_t lodePngError = lodepng_decode(&data, &width, &height, &state, (uint8_t*)_data, _size);

		if (0 != lodePngError)
		{
			BX_ERROR_SET(_err, BIMG_ERROR, "lodepng_decode failed.");
		}
		else
		{
			bool palette   = false;
			bool supported = false;

			switch (state.info_raw.bitdepth)
			{
				case 1:
				case 2:
				case 4:
					palette   = LCT_PALETTE == state.info_raw.colortype;
					format    = palette ? bimg::TextureFormat::RGBA8 : bimg::TextureFormat::R8;
					supported = true;
					break;

				case 8:
					switch (state.info_raw.colortype)
					{
						case LCT_GREY:
							format = bimg::TextureFormat::R8;
							supported = true;
							break;

						case LCT_GREY_ALPHA:
							format = bimg::TextureFormat::RG8;
							supported = true;
							break;

						case LCT_RGB:
							format = bimg::TextureFormat::RGB8;
							supported = true;
							break;

						case LCT_RGBA:
							format = bimg::TextureFormat::RGBA8;
							supported = true;
							break;

						case LCT_PALETTE:
							format  = bimg::TextureFormat::RGBA8;
							palette = true;
							supported = true;
							break;

						case LCT_MAX_OCTET_VALUE:
							break;
					}
					break;

				case 16:
					switch (state.info_raw.colortype)
					{
						case LCT_GREY:
							for (uint32_t ii = 0, num = width*height; ii < num; ++ii)
							{
								uint16_t* rgba = (uint16_t*)data + ii;
								rgba[0] = bx::toHostEndian(rgba[0], false);
							}
							format = bimg::TextureFormat::R16;
							supported = true;
							break;

						case LCT_GREY_ALPHA:
							for (uint32_t ii = 0, num = width*height; ii < num; ++ii)
							{
								uint16_t* rgba = (uint16_t*)data + ii*2;
								rgba[0] = bx::toHostEndian(rgba[0], false);
								rgba[1] = bx::toHostEndian(rgba[1], false);
							}
							format = bimg::TextureFormat::RG16;
							supported = true;
							break;

						case LCT_RGB:
							for (uint32_t ii = 0, num = width*height; ii < num; ++ii)
							{
								uint16_t* rgba = (uint16_t*)data + ii*3;
								rgba[0] = bx::toHostEndian(rgba[0], false);
								rgba[1] = bx::toHostEndian(rgba[1], false);
								rgba[2] = bx::toHostEndian(rgba[2], false);
							}
							format = bimg::TextureFormat::RGBA16;
							supported = true;
							break;

						case LCT_RGBA:
							for (uint32_t ii = 0, num = width*height; ii < num; ++ii)
							{
								uint16_t* rgba = (uint16_t*)data + ii*4;
								rgba[0] = bx::toHostEndian(rgba[0], false);
								rgba[1] = bx::toHostEndian(rgba[1], false);
								rgba[2] = bx::toHostEndian(rgba[2], false);
								rgba[3] = bx::toHostEndian(rgba[3], false);
							}
							format = bimg::TextureFormat::RGBA16;
							supported = true;
							break;

						case LCT_PALETTE:
							break;

						case LCT_MAX_OCTET_VALUE:
							break;
					}
					break;

				default:
					break;
			}

			if (supported)
			{
				const uint8_t* copyData = data;

				TextureFormat::Enum dstFormat = format;
				if (palette) {
					copyData = NULL;
				}
				else if (1 == state.info_raw.bitdepth
				||  2 == state.info_raw.bitdepth
				||  4 == state.info_raw.bitdepth)
				{
					copyData = NULL;
				}
				else if (16      == state.info_raw.bitdepth
					 &&  LCT_RGB == state.info_raw.colortype)
				{
					dstFormat = bimg::TextureFormat::RGBA16;
					copyData  = NULL;
				}

				output = imageAlloc(_allocator
					, dstFormat
					, uint16_t(width)
					, uint16_t(height)
					, 0
					, 1
					, false
					, false
					, copyData
					);

				if (palette)
				{
					if (1 == state.info_raw.bitdepth)
					{
						for (uint32_t ii = 0, num = width*height/8; ii < num; ++ii)
						{
							uint8_t* dst = (uint8_t*)output->m_data + ii*32;
							bx::memCopy(dst,      state.info_raw.palette + ( (data[ii]>>7)&0x1)*4, 4);
							bx::memCopy(dst +  4, state.info_raw.palette + ( (data[ii]>>6)&0x1)*4, 4);
							bx::memCopy(dst +  8, state.info_raw.palette + ( (data[ii]>>5)&0x1)*4, 4);
							bx::memCopy(dst + 12, state.info_raw.palette + ( (data[ii]>>4)&0x1)*4, 4);
							bx::memCopy(dst + 16, state.info_raw.palette + ( (data[ii]>>3)&0x1)*4, 4);
							bx::memCopy(dst + 20, state.info_raw.palette + ( (data[ii]>>2)&0x1)*4, 4);
							bx::memCopy(dst + 24, state.info_raw.palette + ( (data[ii]>>1)&0x1)*4, 4);
							bx::memCopy(dst + 28, state.info_raw.palette + (  data[ii]    &0x1)*4, 4);
						}
					}
					else if (2 == state.info_raw.bitdepth)
					{
						for (uint32_t ii = 0, num = width*height/4; ii < num; ++ii)
						{
							uint8_t* dst = (uint8_t*)output->m_data + ii*16;
							bx::memCopy(dst,      state.info_raw.palette + ( (data[ii]>>6)&0x3)*4, 4);
							bx::memCopy(dst +  4, state.info_raw.palette + ( (data[ii]>>4)&0x3)*4, 4);
							bx::memCopy(dst +  8, state.info_raw.palette + ( (data[ii]>>2)&0x3)*4, 4);
							bx::memCopy(dst + 12, state.info_raw.palette + (  data[ii]    &0x3)*4, 4);
						}
					}
					else if (4 == state.info_raw.bitdepth)
					{
						for (uint32_t ii = 0, num = width*height/2; ii < num; ++ii)
						{
							uint8_t* dst = (uint8_t*)output->m_data + ii*8;
							bx::memCopy(dst,      state.info_raw.palette + ( (data[ii]>>4)&0xf)*4, 4);
							bx::memCopy(dst +  4, state.info_raw.palette + (  data[ii]    &0xf)*4, 4);
						}
					}
					else
					{
						for (uint32_t ii = 0, num = width*height; ii < num; ++ii)
						{
							bx::memCopy( (uint8_t*)output->m_data + ii*4, state.info_raw.palette + data[ii]*4, 4);
						}
					}
				}
				else if (1 == state.info_raw.bitdepth)
				{
					for (uint32_t ii = 0, num = width*height/8; ii < num; ++ii)
					{
						uint8_t* src = (uint8_t*)data + ii;
						uint8_t eightBits = src[0];

						uint8_t* dst = (uint8_t*)output->m_data + ii*8;
						dst[0] = uint8_t( (eightBits>>7)&0x1)*255;
						dst[1] = uint8_t( (eightBits>>6)&0x1)*255;
						dst[2] = uint8_t( (eightBits>>5)&0x1)*255;
						dst[3] = uint8_t( (eightBits>>4)&0x1)*255;
						dst[4] = uint8_t( (eightBits>>3)&0x1)*255;
						dst[5] = uint8_t( (eightBits>>2)&0x1)*255;
						dst[6] = uint8_t( (eightBits>>1)&0x1)*255;
						dst[7] = uint8_t( (eightBits   )&0x1)*255;

					}
				}
				else if (2 == state.info_raw.bitdepth)
				{
					for (uint32_t ii = 0, num = width*height/4; ii < num; ++ii)
					{
						uint8_t* src = (uint8_t*)data + ii;
						uint8_t eightBits = src[0];

						uint8_t* dst = (uint8_t*)output->m_data + ii*4;
						// Note: not exactly precise.
						// Correct way: dst[0] = uint8_t(float( (eightBits>>6)&0x3)*(255.0f/4.0f) );
						dst[0] = uint8_t(uint32_t( ( (eightBits>>6)&0x3)*64)&0xff);
						dst[1] = uint8_t(uint32_t( ( (eightBits>>4)&0x3)*64)&0xff);
						dst[2] = uint8_t(uint32_t( ( (eightBits>>2)&0x3)*64)&0xff);
						dst[3] = uint8_t(uint32_t( ( (eightBits   )&0x3)*64)&0xff);
					}
				}
				else if (4 == state.info_raw.bitdepth)
				{
					for (uint32_t ii = 0, num = width*height/2; ii < num; ++ii)
					{
						uint8_t* src = (uint8_t*)data + ii;
						uint8_t eightBits = src[0];

						uint8_t* dst = (uint8_t*)output->m_data + ii*2;
						// Note: not exactly precise.
						// Correct way: dst[0] = uint8_t(float( (eightBits>>4)&0xf)*(255.0f/16.0f) );
						dst[0] = uint8_t(uint32_t( ( (eightBits>>4)&0xf)*16)&0xff);
						dst[1] = uint8_t(uint32_t( ( (eightBits   )&0xf)*16)&0xff);
					}
				}
				else if (16      == state.info_raw.bitdepth
					 &&  LCT_RGB == state.info_raw.colortype)
				{
					for (uint32_t ii = 0, num = width*height; ii < num; ++ii)
					{
						const uint16_t* src = (uint16_t*)data + ii*3;
						      uint16_t* dst = (uint16_t*)output->m_data + ii*4;
						dst[0] = src[0];
						dst[1] = src[1];
						dst[2] = src[2];
						dst[3] = UINT16_MAX;
					}
				}

				switch (state.info_raw.colortype) //Check for alpha values
				{
					case LCT_GREY:
					case LCT_RGB:
						break;

					case LCT_GREY_ALPHA:
						if (8 == state.info_raw.bitdepth)
						{
							for (uint32_t ii = 0, num = width * height; ii < num; ++ii)
							{
								const uint8_t* rgba = (uint8_t*)data + ii * 2;
								bool has_alpha = rgba[1] < UINT8_MAX;
								if (has_alpha)
								{
									output->m_hasAlpha = has_alpha;
									break;
								}
							}
						}
						else if (16 == state.info_raw.bitdepth)
						{
							for (uint32_t ii = 0, num = width * height; ii < num; ++ii)
							{
								const uint16_t* rgba = (uint16_t*)data + ii * 2;
								bool has_alpha = rgba[1] < UINT16_MAX;
								if (has_alpha)
								{
									output->m_hasAlpha = has_alpha;
									break;
								}
							}
						}
						break;

					case LCT_RGBA:
						if (8 == state.info_raw.bitdepth)
						{
							for (uint32_t ii = 0, num = width * height; ii < num; ++ii)
							{
								const uint8_t* dst = (uint8_t*)output->m_data + ii * 4;
								bool has_alpha = dst[3] < UINT8_MAX;
								if (has_alpha)
								{
									output->m_hasAlpha = has_alpha;
									break;
								}
							}
						}
						else if (16 == state.info_raw.bitdepth)
						{
							for (uint32_t ii = 0, num = width * height; ii < num; ++ii)
							{
								const uint16_t* dst = (uint16_t*)output->m_data + ii * 4;
								bool has_alpha = dst[3] < UINT16_MAX;
								if (has_alpha)
								{
									output->m_hasAlpha = has_alpha;
									break;
								}
							}
						}
						break;

					case LCT_PALETTE:
						output->m_hasAlpha = lodepng_has_palette_alpha(&state.info_raw);
						break;

					case LCT_MAX_OCTET_VALUE:
						break;
				}
			}
			else
			{
				BX_ERROR_SET(_err, BIMG_ERROR, "PNG: Unsupported format.");
			}
		}

		lodepng_state_cleanup(&state);
		lodepng_free(data);

		return output;
	}

	static ImageContainer* imageParseTinyExr(bx::AllocatorI* _allocator, const void* _data, uint32_t _size, bx::Error* _err)
	{
		BX_ERROR_SCOPE(_err);

		EXRVersion exrVersion;
		int result = ParseEXRVersionFromMemory(&exrVersion, (uint8_t*)_data, _size);
		if (TINYEXR_SUCCESS != result)
		{
			return NULL;
		}

		bimg::TextureFormat::Enum format = bimg::TextureFormat::RGBA8;
		uint32_t width  = 0;
		uint32_t height = 0;

		bool hasAlpha = false;

		uint8_t* data = NULL;
		const char* err = NULL;
		EXRHeader exrHeader;
		result = ParseEXRHeaderFromMemory(&exrHeader, &exrVersion, (uint8_t*)_data, _size, &err);
		if (TINYEXR_SUCCESS == result)
		{
			EXRImage exrImage;
			InitEXRImage(&exrImage);

			result = LoadEXRImageFromMemory(&exrImage, &exrHeader, (uint8_t*)_data, _size, &err);
			if (TINYEXR_SUCCESS == result)
			{
				uint8_t idxR = UINT8_MAX;
				uint8_t idxG = UINT8_MAX;
				uint8_t idxB = UINT8_MAX;
				uint8_t idxA = UINT8_MAX;
				for (uint8_t ii = 0, num = uint8_t(exrHeader.num_channels); ii < num; ++ii)
				{
					const EXRChannelInfo& channel = exrHeader.channels[ii];
					if (UINT8_MAX == idxR
					&&  0 == bx::strCmp(channel.name, "R") )
					{
						idxR = ii;
					}
					else if (UINT8_MAX == idxG
					&&  0 == bx::strCmp(channel.name, "G") )
					{
						idxG = ii;
					}
					else if (UINT8_MAX == idxB
					&&  0 == bx::strCmp(channel.name, "B") )
					{
						idxB = ii;
					}
					else if (UINT8_MAX == idxA
					&&  0 == bx::strCmp(channel.name, "A") )
					{
						idxA = ii;
					}
				}

				if (UINT8_MAX != idxR)
				{
					const bool asFloat = exrHeader.pixel_types[idxR] == TINYEXR_PIXELTYPE_FLOAT;
					uint32_t dstBpp = asFloat ? 32 : 16;
					format = asFloat ? TextureFormat::R32F : TextureFormat::R16F;
					uint32_t stepR = 1;
					uint32_t stepG = 0;
					uint32_t stepB = 0;
					uint32_t stepA = 0;

					if (UINT8_MAX != idxG)
					{
						dstBpp = asFloat ? 64 : 32;
						format = asFloat ? TextureFormat::RG32F : TextureFormat::RG16F;
						stepG  = 1;
					}

					if (UINT8_MAX != idxB)
					{
						dstBpp = asFloat ? 128 : 64;
						format = asFloat ? TextureFormat::RGBA32F : TextureFormat::RGBA16F;
						stepB  = 1;
					}

					if (UINT8_MAX != idxA)
					{
						dstBpp = asFloat ? 128 : 64;
						format = asFloat ? TextureFormat::RGBA32F : TextureFormat::RGBA16F;
						stepA  = 1;
					}

					data   = (uint8_t*)bx::alloc(_allocator, exrImage.width * exrImage.height * dstBpp/8);
					width  = exrImage.width;
					height = exrImage.height;

					if (asFloat)
					{
						const float  zero = 0.0f;
						const float* srcR = UINT8_MAX == idxR ? &zero : (const float*)(exrImage.images)[idxR];
						const float* srcG = UINT8_MAX == idxG ? &zero : (const float*)(exrImage.images)[idxG];
						const float* srcB = UINT8_MAX == idxB ? &zero : (const float*)(exrImage.images)[idxB];
						const float* srcA = UINT8_MAX == idxA ? &zero : (const float*)(exrImage.images)[idxA];

						const uint32_t bytesPerPixel = dstBpp/8;
						for (uint32_t ii = 0, num = exrImage.width * exrImage.height; ii < num; ++ii)
						{
							float rgba[4] =
							{
								*srcR,
								*srcG,
								*srcB,
								*srcA,
							};
							bx::memCopy(&data[ii * bytesPerPixel], rgba, bytesPerPixel);

							hasAlpha |= (hasAlpha || rgba[3] < 1.0f);

							srcR += stepR;
							srcG += stepG;
							srcB += stepB;
							srcA += stepA;
						}
					}
					else
					{
						const uint16_t  zero = 0;
						const uint16_t* srcR = UINT8_MAX == idxR ? &zero : (const uint16_t*)(exrImage.images)[idxR];
						const uint16_t* srcG = UINT8_MAX == idxG ? &zero : (const uint16_t*)(exrImage.images)[idxG];
						const uint16_t* srcB = UINT8_MAX == idxB ? &zero : (const uint16_t*)(exrImage.images)[idxB];
						const uint16_t* srcA = UINT8_MAX == idxA ? &zero : (const uint16_t*)(exrImage.images)[idxA];

						const uint32_t bytesPerPixel = dstBpp/8;
						for (uint32_t ii = 0, num = exrImage.width * exrImage.height; ii < num; ++ii)
						{
							uint16_t rgba[4] =
							{
								*srcR,
								*srcG,
								*srcB,
								*srcA,
							};
							bx::memCopy(&data[ii * bytesPerPixel], rgba, bytesPerPixel);

							hasAlpha |= (hasAlpha || rgba[3] < UINT16_MAX);

							srcR += stepR;
							srcG += stepG;
							srcB += stepB;
							srcA += stepA;
						}
					}
				}
				else
				{
					BX_ERROR_SET(_err, BIMG_ERROR, "EXR: Couldn't find R channel.");
				}

				FreeEXRImage(&exrImage);
			}
			else
			{
				switch (result)
				{
				case TINYEXR_ERROR_INVALID_MAGIC_NUMBER: BX_ERROR_SET(_err, BIMG_ERROR, "EXR: Failed to parse image. Invalid magic number."); break;
				case TINYEXR_ERROR_INVALID_EXR_VERSION:	 BX_ERROR_SET(_err, BIMG_ERROR, "EXR: Failed to parse image. Invalid EXR version.");  break;
				case TINYEXR_ERROR_INVALID_ARGUMENT:     BX_ERROR_SET(_err, BIMG_ERROR, "EXR: Failed to parse image. Invalid argument.");     break;
				case TINYEXR_ERROR_INVALID_DATA:         BX_ERROR_SET(_err, BIMG_ERROR, "EXR: Failed to parse image. Invalid data.");         break;
				case TINYEXR_ERROR_INVALID_FILE:         BX_ERROR_SET(_err, BIMG_ERROR, "EXR: Failed to parse image. Invalid file.");         break;
//				case TINYEXR_ERROR_INVALID_PARAMETER:    BX_ERROR_SET(_err, BIMG_ERROR, "EXR: Failed to parse image. Invalid parameter.");    break;
				case TINYEXR_ERROR_CANT_OPEN_FILE:       BX_ERROR_SET(_err, BIMG_ERROR, "EXR: Failed to parse image. Can't open file.");      break;
				case TINYEXR_ERROR_UNSUPPORTED_FORMAT:   BX_ERROR_SET(_err, BIMG_ERROR, "EXR: Failed to parse image. Unsupported format.");   break;
				case TINYEXR_ERROR_INVALID_HEADER:       BX_ERROR_SET(_err, BIMG_ERROR, "EXR: Failed to parse image. Invalid header.");       break;
				case TINYEXR_ERROR_UNSUPPORTED_FEATURE:  BX_ERROR_SET(_err, BIMG_ERROR, "EXR: Failed to parse image. Unsupported feature.");  break;
				case TINYEXR_ERROR_CANT_WRITE_FILE:      BX_ERROR_SET(_err, BIMG_ERROR, "EXR: Failed to parse image. Can't write file.");     break;
				case TINYEXR_ERROR_SERIALZATION_FAILED:  BX_ERROR_SET(_err, BIMG_ERROR, "EXR: Failed to parse image. Serialization failed."); break;
				default:                                 BX_ERROR_SET(_err, BIMG_ERROR, "EXR: Failed to parse image.");                       break;
				}
			}

			FreeEXRHeader(&exrHeader);
		}
		else
		{
			BX_ERROR_SET(_err, BIMG_ERROR, "EXR: Failed to parse header.");
		}

		ImageContainer* output = NULL;
		if (NULL != data)
		{
			output = imageAlloc(_allocator
				, format
				, uint16_t(width)
				, uint16_t(height)
				, 0
				, 1
				, false
				, false
				, data
				);
			bx::free(_allocator, data);
			output->m_hasAlpha = hasAlpha;
		}

		return output;
	}

	static ImageContainer* imageParseStbImage(bx::AllocatorI* _allocator, const void* _data, uint32_t _size, bx::Error* _err)
	{
		BX_ERROR_SCOPE(_err);

		const int isHdr = stbi_is_hdr_from_memory( (const uint8_t*)_data, (int)_size);

		void* data;
		uint32_t width  = 0;
		uint32_t height = 0;
		int comp = 0;
		if (isHdr)
		{
			data = stbi_loadf_from_memory( (const uint8_t*)_data, (int)_size, (int*)&width, (int*)&height, &comp, 4);
		}
		else
		{
			data = stbi_load_from_memory ( (const uint8_t*)_data, (int)_size, (int*)&width, (int*)&height, &comp, 0);
		}

		if (NULL == data)
		{
			return NULL;
		}

		bimg::TextureFormat::Enum format = bimg::TextureFormat::RGBA8;

		if (isHdr)
		{
			format = bimg::TextureFormat::RGBA32F;
		}
		else
		{
			switch (comp)
			{
				case 1:  format = bimg::TextureFormat::R8;   break;
				case 2:  format = bimg::TextureFormat::RG8;  break;
				case 3:  format = bimg::TextureFormat::RGB8; break;
				default: break;
			}
		}

		ImageContainer* output = imageAlloc(_allocator
			, format
			, bx::narrowCast<uint16_t>(width)
			, bx::narrowCast<uint16_t>(height)
			, 0
			, 1
			, false
			, false
			, data
			);
		stbi_image_free(data);

		return output;
	}

	static ImageContainer* imageParseJpeg(bx::AllocatorI* _allocator, const void* _data, uint32_t _size, bx::Error* _err)
	{
		bx::MemoryReader reader(_data, _size);

		bx::Error err;

		uint16_t magic = 0;
		bx::readHE(&reader, magic, false, &err);

		if (!err.isOk()
		||  0xffd8 != magic)
		{
			return NULL;
		}

		Orientation::Enum orientation = Orientation::R0;

		while (err.isOk() )
		{
			bx::readHE(&reader, magic, false, &err);

			uint16_t size;
			bx::readHE(&reader, size, false, &err);

			if (!err.isOk() )
			{
				return NULL;
			}

			if (0xffe1 != magic)
			{
				bx::seek(&reader, size-2);
				continue;
			}

			char exif00[6];
			bx::read(&reader, exif00, 6, &err);

			if (0 == bx::memCmp(exif00, "Exif\0\0", 6) )
			{
				uint16_t iimm = 0;
				bx::read(&reader, iimm, &err);

				const bool littleEndian = iimm == 0x4949; //II - Intel - little endian
				if (!err.isOk()
				&&  !littleEndian
				&&   iimm != 0x4d4d) // MM - Motorola - big endian
				{
					return NULL;
				}

				bx::readHE(&reader, magic, littleEndian, &err);
				if (!err.isOk()
				||  0x2a != magic)
				{
					return NULL;
				}

				uint32_t ifd0;
				bx::readHE(&reader, ifd0, littleEndian, &err);

				if (!err.isOk()
				||  8 > ifd0)
				{
					return NULL;
				}

				bx::seek(&reader, ifd0-8);

				uint16_t numEntries;
				bx::readHE(&reader, numEntries, littleEndian, &err);

				for (uint32_t ii = 0; err.isOk() && ii < numEntries; ++ii)
				{
					// Reference(s):
					// - EXIF Tags
					//   https://web.archive.org/web/20190218005249/https://sno.phy.queensu.ca/~phil/exiftool/TagNames/EXIF.html
					//
					uint16_t tag;
					bx::readHE(&reader, tag, littleEndian, &err);

					uint16_t format;
					bx::readHE(&reader, format, littleEndian, &err);

					uint32_t length;
					bx::readHE(&reader, length, littleEndian, &err);

					uint32_t data;
					bx::readHE(&reader, data, littleEndian, &err);

					switch (tag)
					{
					case 0x112: // orientation
						if (3 == format)
						{
							bx::seek(&reader, -4);

							uint16_t u16;
							bx::readHE(&reader, u16, littleEndian, &err);

							uint16_t pad;
							bx::read(&reader, pad, &err);

							switch (u16)
							{
							default:
							case 1: orientation = Orientation::R0;        break; // Horizontal (normal)
							case 2: orientation = Orientation::HFlip;     break; // Mirror horizontal
							case 3: orientation = Orientation::R180;      break; // Rotate 180
							case 4: orientation = Orientation::VFlip;     break; // Mirror vertical
							case 5: orientation = Orientation::HFlipR270; break; // Mirror horizontal and rotate 270 CW
							case 6: orientation = Orientation::R90;       break; // Rotate 90 CW
							case 7: orientation = Orientation::HFlipR90;  break; // Mirror horizontal and rotate 90 CW
							case 8: orientation = Orientation::R270;      break; // Rotate 270 CW
							}
						}
						break;

					default:
						break;
					}
				}
			}

			break;
		}

		ImageContainer* image = imageParseStbImage(_allocator, _data, _size, _err);
		if (NULL != image)
		{
			image->m_orientation = orientation;
		}

		return image;
	}

	static ImageContainer* imageParseLibHeif(bx::AllocatorI* _allocator, const void* _data, uint32_t _size, bx::Error* _err)
	{
#if BIMG_DECODE_HEIF
		heif_context* ctx = heif_context_alloc();

		heif_context_read_from_memory_without_copy(ctx, _data, _size, NULL);

		heif_image_handle* handle;
		heif_context_get_primary_image_handle(ctx, &handle);

		heif_image* image;
		heif_decode_image(handle, &image, heif_colorspace_RGB, heif_chroma_interleaved_RGBA, NULL);

		int32_t srcStride;
		const uint8_t* data = heif_image_get_plane_readonly(image, heif_channel_interleaved, &srcStride);

		ImageContainer* output = NULL;
		if (NULL != data)
		{
			const bimg::TextureFormat::Enum format = bimg::TextureFormat::RGBA8;
			const int32_t width  = heif_image_handle_get_width(handle);
			const int32_t height = heif_image_handle_get_height(handle);
			const int32_t dstStride = width*4;

			output = imageAlloc(_allocator
				, format
				, bx::narrowCast<uint16_t>(width)
				, bx::narrowCast<uint16_t>(height)
				, 0
				, 1
				, false
				, false
				, NULL
				);

			bx::memCopy(output->m_data, dstStride, data, srcStride, dstStride, height);
		}

		heif_image_release(image);
		heif_image_handle_release(handle);

		heif_context_free(ctx);

		BX_UNUSED(_err);
		return output;
#else
		BX_UNUSED(_allocator, _data, _size, _err);
		return NULL;
#endif // BIMG_DECODE_HEIF
	}

	ImageContainer* imageParse(bx::AllocatorI* _allocator, const void* _data, uint32_t _size, TextureFormat::Enum _dstFormat, bx::Error* _err)
	{
		BX_ERROR_SCOPE(_err);

		ImageContainer* input = imageParseDds     (_allocator, _data, _size, _err)        ;
		input = NULL == input ? imageParseKtx     (_allocator, _data, _size, _err) : input;
		input = NULL == input ? imageParsePvr3    (_allocator, _data, _size, _err) : input;
		input = NULL == input ? imageParseGnf     (_allocator, _data, _size, _err) : input;
		input = NULL == input ? imageParseLodePng (_allocator, _data, _size, _err) : input;
		input = NULL == input ? imageParseTinyExr (_allocator, _data, _size, _err) : input;
		input = NULL == input ? imageParseJpeg    (_allocator, _data, _size, _err) : input;
		input = NULL == input ? imageParseStbImage(_allocator, _data, _size, _err) : input;
		input = NULL == input ? imageParseLibHeif (_allocator, _data, _size, _err) : input;

		if (NULL == input)
		{
			return NULL;
		}

		_dstFormat = TextureFormat::Count == _dstFormat
			? input->m_format
			: _dstFormat
			;

		if (_dstFormat == input->m_format)
		{
			return input;
		}

		ImageContainer* output = imageConvert(_allocator, _dstFormat, *input);
		imageFree(input);

		return output;
	}

} // namespace bimg
