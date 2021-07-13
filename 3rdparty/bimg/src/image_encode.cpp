/*
 * Copyright 2011-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bimg#license-bsd-2-clause
 */

#include <bimg/encode.h>
#include "bimg_p.h"

#include <libsquish/squish.h>
#include <etc1/etc1.h>
#include <etc2/ProcessRGB.hpp>
#include <nvtt/nvtt.h>
#include <pvrtc/PvrTcEncoder.h>
#include <edtaa3/edtaa3func.h>
#include <astc/astc_lib.h>

BX_PRAGMA_DIAGNOSTIC_PUSH();
BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4100) // warning C4100: 'alloc_context': unreferenced formal parameter
BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4702) // warning C4702: unreachable code
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wunused-parameter") // warning: unused parameter ‘alloc_context’ [-Wunused-parameter]
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb/stb_image_resize.h>
BX_PRAGMA_DIAGNOSTIC_POP();

extern "C" {
#include <iqa.h>
}

namespace bimg
{
	static uint32_t s_squishQuality[] =
	{
		// Standard
		squish::kColourClusterFit,          // Default
		squish::kColourIterativeClusterFit, // Highest
		squish::kColourRangeFit,            // Fastest
		// Normal map
		squish::kColourClusterFit,          // Default
		squish::kColourIterativeClusterFit, // Highest
		squish::kColourRangeFit,            // Fastest
	};
	BX_STATIC_ASSERT(Quality::Count == BX_COUNTOF(s_squishQuality) );

	static const ASTC_COMPRESS_MODE s_astcQuality[] =
	{
		// Standard
		ASTC_COMPRESS_MEDIUM,       // Default
		ASTC_COMPRESS_THOROUGH,     // Highest
		ASTC_COMPRESS_FAST,         // Fastest
		// Normal map
		ASTC_COMPRESS_MEDIUM,       // Default
		ASTC_COMPRESS_THOROUGH,     // Highest
		ASTC_COMPRESS_FAST,         // Fastest
	};
	BX_STATIC_ASSERT(Quality::Count == BX_COUNTOF(s_astcQuality));

	void imageEncodeFromRgba8(bx::AllocatorI* _allocator, void* _dst, const void* _src, uint32_t _width, uint32_t _height, uint32_t _depth, TextureFormat::Enum _format, Quality::Enum _quality, bx::Error* _err)
	{
		const uint8_t* src = (const uint8_t*)_src;
		uint8_t* dst = (uint8_t*)_dst;

		const uint32_t srcPitch = _width*4;
		const uint32_t srcSlice = _height*srcPitch;
		const uint32_t dstBpp   = getBitsPerPixel(_format);
		const uint32_t dstPitch = _width*dstBpp/8;
		const uint32_t dstSlice = _height*dstPitch;

		for (uint32_t zz = 0; zz < _depth && _err->isOk(); ++zz, src += srcSlice, dst += dstSlice)
		{
			switch (_format)
			{
			case TextureFormat::BC1:
			case TextureFormat::BC2:
			case TextureFormat::BC3:
			case TextureFormat::BC4:
			case TextureFormat::BC5:
				squish::CompressImage(src, _width, _height, dst
					, s_squishQuality[_quality]
					| (_format == TextureFormat::BC2 ? squish::kDxt3
					:  _format == TextureFormat::BC3 ? squish::kDxt5
					:  _format == TextureFormat::BC4 ? squish::kBc4
					:  _format == TextureFormat::BC5 ? squish::kBc5
					:                                  squish::kDxt1)
					);
				break;

			case TextureFormat::BC6H:
			case TextureFormat::BC7:
				BX_ERROR_SET(_err, BIMG_ERROR, "Unable to convert between input/output formats!");
				break;

			case TextureFormat::ETC1:
				etc1_encode_image(src, _width, _height, 4, _width*4, dst);
				break;

			case TextureFormat::ETC2:
				{
					const uint32_t blockWidth  = (_width +3)/4;
					const uint32_t blockHeight = (_height+3)/4;
					uint64_t* dstBlock = (uint64_t*)dst;
					for (uint32_t yy = 0; yy < blockHeight; ++yy)
					{
						for (uint32_t xx = 0; xx < blockWidth; ++xx)
						{
							uint8_t block[4*4*4];
							const uint8_t* ptr = &src[(yy*srcPitch+xx*4)*4];

							for (uint32_t ii = 0; ii < 16; ++ii)
							{ // BGRx
								bx::memCopy(&block[ii*4], &ptr[(ii%4)*srcPitch + (ii&~3)], 4);
								bx::swap(block[ii*4+0], block[ii*4+2]);
							}

							*dstBlock++ = ProcessRGB_ETC2(block);
						}
					}
				}
				break;

			case TextureFormat::PTC14:
				{
					using namespace Javelin;
					RgbaBitmap bmp;
					bmp.width  = _width;
					bmp.height = _height;
					bmp.data   = const_cast<uint8_t*>(src);
					PvrTcEncoder::EncodeRgb4Bpp(dst, bmp);
					bmp.data = NULL;
				}
				break;

			case TextureFormat::PTC14A:
				{
					using namespace Javelin;
					RgbaBitmap bmp;
					bmp.width  = _width;
					bmp.height = _height;
					bmp.data   = const_cast<uint8_t*>(src);
					PvrTcEncoder::EncodeRgba4Bpp(dst, bmp);
					bmp.data = NULL;
				}
				break;

			case TextureFormat::ASTC4x4:
			case TextureFormat::ASTC5x5:
			case TextureFormat::ASTC6x6:
			case TextureFormat::ASTC8x5:
			case TextureFormat::ASTC8x6:
			case TextureFormat::ASTC10x5:
				{
					const bimg::ImageBlockInfo& astcBlockInfo = bimg::getBlockInfo(_format);

					ASTC_COMPRESS_MODE compress_mode = s_astcQuality[_quality];
					ASTC_DECODE_MODE   decode_mode   = ASTC_DECODE_LDR_LINEAR;

					if (Quality::NormalMapDefault <= _quality)
					{
						astc_compress(_width, _height, src, ASTC_ENC_NORMAL_RA, srcPitch, astcBlockInfo.blockWidth, astcBlockInfo.blockHeight, compress_mode, decode_mode, dst);
					}
					else
					{
						astc_compress(_width, _height, src, ASTC_RGBA, srcPitch, astcBlockInfo.blockWidth, astcBlockInfo.blockHeight, compress_mode, decode_mode, dst);
					}
				}
				break;

			case TextureFormat::BGRA8:
				imageSwizzleBgra8(dst, dstPitch, _width, _height, src, srcPitch);
				break;

			case TextureFormat::RGBA8:
				bx::memCopy(_dst, dstPitch, _src, srcPitch, srcPitch, _height);
				break;

			default:
				if (!imageConvert(_allocator, dst, _format, src, TextureFormat::RGBA8, _width, _height, 1) )
				{
					BX_ERROR_SET(_err, BIMG_ERROR, "Unable to convert between input/output formats!");
				}
				break;
			}
		}
	}

	void imageEncodeFromRgba32f(bx::AllocatorI* _allocator, void* _dst, const void* _src, uint32_t _width, uint32_t _height, uint32_t _depth, TextureFormat::Enum _dstFormat, Quality::Enum _quality, bx::Error* _err)
	{
		BX_ERROR_SCOPE(_err);

		const uint8_t* src = (const uint8_t*)_src;

		switch (_dstFormat)
		{
		case TextureFormat::BC6H:
			nvtt::compressBC6H(src, _width, _height, _width*16, _dst);
			break;

		case TextureFormat::BC7:
			nvtt::compressBC7(src, _width, _height, _width*16, _dst);
			break;

		default:
			if (!imageConvert(_allocator, _dst, _dstFormat, _src, TextureFormat::RGBA32F, _width, _height, _depth) )
			{
				uint8_t* temp = (uint8_t*)BX_ALLOC(_allocator, _width*_height*_depth*4);
				if (imageConvert(_allocator, temp, TextureFormat::RGBA8, _src, TextureFormat::RGBA32F, _width, _height, _depth) )
				{
					for (uint32_t zz = 0; zz < _depth; ++zz)
					{
						const uint32_t zoffset = zz*_width*_height;

						for (uint32_t yy = 0; yy < _height; ++yy)
						{
							const uint32_t yoffset = zoffset + yy*_width;

							for (uint32_t xx = 0; xx < _width; ++xx)
							{
								const uint32_t offset = yoffset + xx;
								const float* input = (const float*)&src[offset * 16];
								uint8_t* output    = &temp[offset * 4];
								output[0] = uint8_t(bx::clamp(input[0], 0.0f, 1.0f)*255.0f + 0.5f);
								output[1] = uint8_t(bx::clamp(input[1], 0.0f, 1.0f)*255.0f + 0.5f);
								output[2] = uint8_t(bx::clamp(input[2], 0.0f, 1.0f)*255.0f + 0.5f);
								output[3] = uint8_t(bx::clamp(input[3], 0.0f, 1.0f)*255.0f + 0.5f);
							}
						}
					}

					imageEncodeFromRgba8(_allocator, _dst, temp, _width, _height, _depth, _dstFormat, _quality, _err);
				}
				else
				{
					BX_ERROR_SET(_err, BIMG_ERROR, "Unable to convert between input/output formats!");
				}

				BX_FREE(_allocator, temp);
			}
			break;
		}
	}

	void imageEncode(bx::AllocatorI* _allocator, void* _dst, const void* _src, TextureFormat::Enum _srcFormat, uint32_t _width, uint32_t _height, uint32_t _depth, TextureFormat::Enum _dstFormat, Quality::Enum _quality, bx::Error* _err)
	{
		switch (_dstFormat)
		{
			case TextureFormat::BC1:
			case TextureFormat::BC2:
			case TextureFormat::BC3:
			case TextureFormat::BC4:
			case TextureFormat::BC5:
			case TextureFormat::ETC1:
			case TextureFormat::ETC2:
			case TextureFormat::PTC14:
			case TextureFormat::PTC14A:
			case TextureFormat::ASTC4x4:
			case TextureFormat::ASTC5x5:
			case TextureFormat::ASTC6x6:
			case TextureFormat::ASTC8x5:
			case TextureFormat::ASTC8x6:
			case TextureFormat::ASTC10x5:
				{
					uint8_t* temp = (uint8_t*)BX_ALLOC(_allocator, _width*_height*_depth*4);
					imageDecodeToRgba8(_allocator, temp, _src, _width, _height, _width*4, _srcFormat);
					imageEncodeFromRgba8(_allocator, _dst, temp, _width, _height, _depth, _dstFormat, _quality, _err);
					BX_FREE(_allocator, temp);
				}
				break;

			case bimg::TextureFormat::BC6H:
			case bimg::TextureFormat::BC7:
				{
					uint8_t* temp = (uint8_t*)BX_ALLOC(_allocator, _width*_height*_depth*16);
					imageDecodeToRgba32f(_allocator, temp, _src, _width, _height, _depth, _width*16, _srcFormat);
					imageEncodeFromRgba32f(_allocator, _dst, temp, _width, _height, _depth, _dstFormat, _quality, _err);
					BX_FREE(_allocator, temp);
				}
				break;

			default:
				if (!imageConvert(_allocator, _dst, _dstFormat, _src, _srcFormat, _width, _height, 1) )
				{
					BX_ERROR_SET(_err, BIMG_ERROR, "Unable to convert between input/output formats!");
				}
				break;
		}
	}

	ImageContainer* imageEncode(bx::AllocatorI* _allocator, TextureFormat::Enum _dstFormat, Quality::Enum _quality, const ImageContainer& _input)
	{
		ImageContainer* output = imageAlloc(_allocator
			, _dstFormat
			, uint16_t(_input.m_width)
			, uint16_t(_input.m_height)
			, uint16_t(_input.m_depth)
			, _input.m_numLayers
			, _input.m_cubeMap
			, 1 < _input.m_numMips
			);

		const uint16_t numSides = _input.m_numLayers * (_input.m_cubeMap ? 6 : 1);

		bx::Error err;

		for (uint16_t side = 0; side < numSides && err.isOk(); ++side)
		{
			for (uint8_t lod = 0, num = _input.m_numMips; lod < num && err.isOk(); ++lod)
			{
				ImageMip mip;
				if (imageGetRawData(_input, side, lod, _input.m_data, _input.m_size, mip) )
				{
					ImageMip dstMip;
					imageGetRawData(*output, side, lod, output->m_data, output->m_size, dstMip);
					uint8_t* dstData = const_cast<uint8_t*>(dstMip.m_data);

					imageEncode(
						  _allocator
						, dstData
						, mip.m_data
						, mip.m_format
						, mip.m_width
						, mip.m_height
						, mip.m_depth
						, _dstFormat
						, _quality
						, &err
						);
				}
			}
		}

		if (err.isOk() )
		{
			return output;
		}

		imageFree(output);
		return NULL;
	}

	void imageRgba32f11to01(void* _dst, uint32_t _width, uint32_t _height, uint32_t _depth, uint32_t _pitch, const void* _src)
	{
		const uint8_t* src = (const uint8_t*)_src;
		uint8_t* dst = (uint8_t*)_dst;

		for (uint32_t zz = 0; zz < _depth; ++zz)
		{
			for (uint32_t yy = 0; yy < _height; ++yy)
			{
				for (uint32_t xx = 0; xx < _width; ++xx)
				{
					const uint32_t offset = yy*_pitch + xx * 16;
					const float* input = (const float*)&src[offset];
					float* output = (float*)&dst[offset];
					output[0] = input[0]*0.5f + 0.5f;
					output[1] = input[1]*0.5f + 0.5f;
					output[2] = input[2]*0.5f + 0.5f;
					output[3] = input[3]*0.5f + 0.5f;
				}
			}
		}
	}

	static void edtaa3(bx::AllocatorI* _allocator, double* _dst, uint32_t _width, uint32_t _height, double* _src)
	{
		const uint32_t numPixels = _width*_height;

		short* xdist = (short *)BX_ALLOC(_allocator, numPixels*sizeof(short) );
		short* ydist = (short *)BX_ALLOC(_allocator, numPixels*sizeof(short) );
		double* gx   = (double*)BX_ALLOC(_allocator, numPixels*sizeof(double) );
		double* gy   = (double*)BX_ALLOC(_allocator, numPixels*sizeof(double) );

		::computegradient(_src, _width, _height, gx, gy);
		::edtaa3(_src, gx, gy, _width, _height, xdist, ydist, _dst);

		for (uint32_t ii = 0; ii < numPixels; ++ii)
		{
			if (_dst[ii] < 0.0)
			{
				_dst[ii] = 0.0;
			}
		}

		BX_FREE(_allocator, xdist);
		BX_FREE(_allocator, ydist);
		BX_FREE(_allocator, gx);
		BX_FREE(_allocator, gy);
	}

	void imageMakeDist(bx::AllocatorI* _allocator, void* _dst, uint32_t _width, uint32_t _height, uint32_t _srcPitch, const void* _src)
	{
		const uint32_t numPixels = _width*_height;

		double* imgIn   = (double*)BX_ALLOC(_allocator, numPixels*sizeof(double) );
		double* outside = (double*)BX_ALLOC(_allocator, numPixels*sizeof(double) );
		double* inside  = (double*)BX_ALLOC(_allocator, numPixels*sizeof(double) );

		for (uint32_t yy = 0; yy < _height; ++yy)
		{
			const uint8_t* src = (const uint8_t*)_src + yy*_srcPitch;
			double* dst = &imgIn[yy*_width];
			for (uint32_t xx = 0; xx < _width; ++xx)
			{
				dst[xx] = double(src[xx])/255.0;
			}
		}

		edtaa3(_allocator, outside, _width, _height, imgIn);

		for (uint32_t ii = 0; ii < numPixels; ++ii)
		{
			imgIn[ii] = 1.0 - imgIn[ii];
		}

		edtaa3(_allocator, inside, _width, _height, imgIn);

		BX_FREE(_allocator, imgIn);

		uint8_t* dst = (uint8_t*)_dst;

		for (uint32_t ii = 0; ii < numPixels; ++ii)
		{
			double dist = bx::clamp( (outside[ii] - inside[ii]) * 1.0/16.0 + 0.5, 0.0, 1.0);
			dst[ii] = 255-uint8_t(dist * 255.0);
		}

		BX_FREE(_allocator, inside);
		BX_FREE(_allocator, outside);
	}

	static const iqa_ssim_args s_iqaArgs =
	{
		0.39f,     // alpha
		0.731f,    // beta
		1.12f,     // gamma
		187,       // L
		0.025987f, // K1
		0.0173f,   // K2
		1          // factor
	};

	float imageQualityRgba8(
		  const void* _reference
		, const void* _data
		, uint16_t _width
		, uint16_t _height
		)
	{
		float result = iqa_ssim( (const uint8_t*)_reference
			, (const uint8_t*)_data
			, _width
			, _height
			, _width*4
			, 0
			, &s_iqaArgs
			);
		return result;
	}

	bool imageResizeRgba32fLinear(ImageContainer* _dst, const ImageContainer* _src)
	{
		const uint16_t numSides = _src->m_numLayers * (_src->m_cubeMap ? 6 : 1);

		for (uint16_t side = 0; side < numSides; ++side)
		{
			bimg::ImageMip srcMip;
			bimg::imageGetRawData(*_src, side, 0, _src->m_data, _src->m_size, srcMip);

			bimg::ImageMip dstMip;
			bimg::imageGetRawData(*_dst, side, 0, _dst->m_data, _dst->m_size, dstMip);
			uint8_t* dstData = const_cast<uint8_t*>(dstMip.m_data);

			const uint32_t srcPitch = _src->m_width*16;
			const uint32_t srcSlice = _src->m_height*srcPitch;
			const uint32_t dstPitch = _dst->m_width*16;
			const uint32_t dstSlice = _dst->m_height*dstPitch;

			for (uint32_t zz = 0, depth = _dst->m_depth; zz < depth; ++zz, dstData += dstSlice)
			{
				const uint32_t srcDataStep = uint32_t(bx::floor(zz * _src->m_depth / float(_dst->m_depth) ) );
				const uint8_t* srcData = &srcMip.m_data[srcDataStep*srcSlice];

				int result = stbir_resize_float_generic(
					  (const float*)srcData, _src->m_width, _src->m_height, srcPitch
					, (      float*)dstData, _dst->m_width, _dst->m_height, dstPitch
					, 4, 3
					, STBIR_FLAG_ALPHA_PREMULTIPLIED
					, STBIR_EDGE_CLAMP
					, STBIR_FILTER_BOX
					, STBIR_COLORSPACE_LINEAR
					, NULL
					);

				if (1 != result)
				{
					return false;
				}
			}

		}

		return true;
	}

	static float getAlpha(UnpackFn _unpack, const void* _data)
	{
		float rgba[4];
		_unpack(rgba, _data);
		return rgba[3];
	}

	float imageAlphaTestCoverage(TextureFormat::Enum _format, uint32_t _width, uint32_t _height, uint32_t _srcPitch, const void* _src, float _alphaRef, float _scale, uint32_t _upscale)
	{
		UnpackFn unpack = getUnpack(_format);
		if (NULL == unpack)
		{
			return 0.0f;
		}

		float coverage = 0.0f;
		const uint8_t* src = (const uint8_t*)_src;
		const uint32_t xstep = getBitsPerPixel(_format) / 8;
		const uint32_t numSamples = _upscale;
		const float sampleStep = 1.0f / numSamples;

		for (uint32_t yy = 0, ystep = _srcPitch; yy < _height-1; ++yy, src += ystep)
		{
			const uint8_t* data = src;
			for (uint32_t xx = 0; xx < _width-1; ++xx, data += xstep)
			{
				float alpha00 = _scale * getAlpha(unpack, data);
				float alpha10 = _scale * getAlpha(unpack, data+xstep);
				float alpha01 = _scale * getAlpha(unpack, data+ystep);
				float alpha11 = _scale * getAlpha(unpack, data+ystep+xstep);

				for (float fy = 0.0f; fy < 1.0f; fy += sampleStep)
				{
					for (float fx = 0.0f; fx < 1.0f; fx += sampleStep)
					{
						float alpha = 0.0f
							+ alpha00 * (1.0f - fx) * (1.0f - fy)
							+ alpha10 * (       fx) * (1.0f - fy)
							+ alpha01 * (1.0f - fx) * (       fy)
							+ alpha11 * (       fx) * (       fy)
							;

						if (alpha > _alphaRef)
						{
							coverage += 1.0f;
						}
					}
				}
			}
		}

		return coverage / float(_width*_height*numSamples*numSamples);
	}

	void imageScaleAlphaToCoverage(TextureFormat::Enum _format, uint32_t _width, uint32_t _height, uint32_t _srcPitch, void* _src, float _desiredCoverage, float _alphaRef, uint32_t _upscale)
	{
		PackFn   pack   = getPack(_format);
		UnpackFn unpack = getUnpack(_format);
		if (NULL == pack
		||  NULL == unpack)
		{
			return;
		}

		float min   = 0.0f;
		float max   = 4.0f;
		float scale = 1.0f;

		for (uint32_t ii = 0; ii < 10; ++ii)
		{
			float coverage = imageAlphaTestCoverage(
				  _format
				, _width
				, _height
				, _srcPitch
				, _src
				, _alphaRef
				, scale
				, _upscale
				);

			if (coverage < _desiredCoverage)
			{
				min = scale;
			}
			else if (coverage > _desiredCoverage)
			{
				max = scale;
			}
			else
			{
				break;
			}

			scale = (min + max) * 0.5f;
		}

		uint8_t* src = (uint8_t*)_src;
		const uint32_t xstep = getBitsPerPixel(_format) / 8;

		for (uint32_t yy = 0, ystep = _srcPitch; yy < _height; ++yy, src += ystep)
		{
			uint8_t* data = src;
			for (uint32_t xx = 0; xx < _width; ++xx, data += xstep)
			{
				float rgba[4];
				unpack(rgba, data);
				rgba[3] = bx::clamp(rgba[3]*scale, 0.0f, 1.0f);
				pack(data, rgba);
			}
		}
	}

} // namespace bimg
