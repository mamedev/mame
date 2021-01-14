/*
 * Copyright 2011-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bimg#license-bsd-2-clause
 */

#include <bx/allocator.h>
#include <bx/readerwriter.h>
#include <bx/endian.h>
#include <bx/math.h>

#include <bimg/decode.h>
#include <bimg/encode.h>

#if 0
#	define DBG(_format, ...) bx::printf("" _format "\n", ##__VA_ARGS__)
#else
#	define DBG(...) BX_NOOP()
#endif // DEBUG

#include <bx/bx.h>
#include <bx/commandline.h>
#include <bx/file.h>

#include <string>

#define BIMG_TEXTUREC_VERSION_MAJOR 1
#define BIMG_TEXTUREC_VERSION_MINOR 18

BX_ERROR_RESULT(TEXTRUREC_ERROR, BX_MAKEFOURCC('t', 'c', 0, 0) );

struct Options
{
	void dump()
	{
		DBG("Options:\n"
			"\t  maxSize: %d\n"
			"\t  mipSkip: %d\n"
			"\t     edge: %f\n"
			"\t   format: %s\n"
			"\t     mips: %s\n"
			"\tnormalMap: %s\n"
			"\t      iqa: %s\n"
			"\t      pma: %s\n"
			"\t      sdf: %s\n"
			"\t radiance: %s\n"
			"\t equirect: %s\n"
			"\t    strip: %s\n"
			"\t   linear: %s\n"
			, maxSize
			, mipSkip
			, edge
			, bimg::getName(format)
			, mips      ? "true" : "false"
			, normalMap ? "true" : "false"
			, iqa       ? "true" : "false"
			, pma       ? "true" : "false"
			, sdf       ? "true" : "false"
			, radiance  ? "true" : "false"
			, equirect  ? "true" : "false"
			, strip     ? "true" : "false"
			, linear    ? "true" : "false"
			);
	}

	uint32_t maxSize = UINT32_MAX;
	uint32_t mipSkip = 0;
	float edge       = 0.0f;
	bimg::TextureFormat::Enum format   = bimg::TextureFormat::Count;
	bimg::Quality::Enum quality        = bimg::Quality::Default;
	bimg::LightingModel::Enum radiance = bimg::LightingModel::Count;
	bool mips      = false;
	bool normalMap = false;
	bool equirect  = false;
	bool strip     = false;
	bool iqa       = false;
	bool pma       = false;
	bool sdf       = false;
	bool alphaTest = false;
	bool linear    = false;
};

void imageRgba32fNormalize(void* _dst, uint32_t _width, uint32_t _height, uint32_t _srcPitch, const void* _src)
{
	const uint8_t* src = (const uint8_t*)_src;
	uint8_t* dst = (uint8_t*)_dst;

	for (uint32_t yy = 0, ystep = _srcPitch; yy < _height; ++yy, src += ystep)
	{
		const float* rgba = (const float*)&src[0];
		for (uint32_t xx = 0; xx < _width; ++xx, rgba += 4, dst += 16)
		{
			const bx::Vec3 xyz = bx::load<bx::Vec3>(rgba);
			bx::store(dst, bx::normalize(xyz) );
		}
	}
}

void imagePremultiplyAlpha(void* _inOut, uint32_t _width, uint32_t _height, uint32_t _depth, bimg::TextureFormat::Enum _format)
{
	uint8_t* inOut = (uint8_t*)_inOut;
	uint32_t bpp = bimg::getBitsPerPixel(_format);
	uint32_t pitch = _width*bpp/8;

	bimg::PackFn   pack   = bimg::getPack(_format);
	bimg::UnpackFn unpack = bimg::getUnpack(_format);

	for (uint32_t zz = 0; zz < _depth; ++zz)
	{
		for (uint32_t yy = 0; yy < _height; ++yy)
		{
			for (uint32_t xx = 0; xx < _width; ++xx)
			{
				const uint32_t offset = yy*pitch + xx*bpp/8;

				float rgba[4];
				unpack(rgba, &inOut[offset]);
				const float alpha = rgba[3];
				rgba[0] = bx::toGamma(bx::toLinear(rgba[0]) * alpha);
				rgba[1] = bx::toGamma(bx::toLinear(rgba[1]) * alpha);
				rgba[2] = bx::toGamma(bx::toLinear(rgba[2]) * alpha);
				pack(&inOut[offset], rgba);
			}
		}
	}
}

bimg::ImageContainer* convert(bx::AllocatorI* _allocator, const void* _inputData, uint32_t _inputSize, const Options& _options, bx::Error* _err)
{
	BX_ERROR_SCOPE(_err);

	const uint8_t* inputData = (uint8_t*)_inputData;

	bimg::ImageContainer* output = NULL;
	bimg::ImageContainer* input  = bimg::imageParse(_allocator, inputData, _inputSize, bimg::TextureFormat::Count, _err);

	if (!_err->isOk() )
	{
		return NULL;
	}

	if (NULL != input)
	{
		bimg::TextureFormat::Enum inputFormat  = input->m_format;
		bimg::TextureFormat::Enum outputFormat = input->m_format;

		if (bimg::TextureFormat::Count != _options.format)
		{
			outputFormat = _options.format;
		}

		if (_options.sdf)
		{
			outputFormat = bimg::TextureFormat::R8;
		}

		const bimg::ImageBlockInfo&  inputBlockInfo  = bimg::getBlockInfo(inputFormat);
		const bimg::ImageBlockInfo&  outputBlockInfo = bimg::getBlockInfo(outputFormat);
		const uint32_t blockWidth  = outputBlockInfo.blockWidth;
		const uint32_t blockHeight = outputBlockInfo.blockHeight;
		const uint32_t minBlockX   = outputBlockInfo.minBlockX;
		const uint32_t minBlockY   = outputBlockInfo.minBlockY;
		uint32_t outputWidth  = bx::max(blockWidth  * minBlockX, ( (input->m_width  + blockWidth  - 1) / blockWidth )*blockWidth);
		uint32_t outputHeight = bx::max(blockHeight * minBlockY, ( (input->m_height + blockHeight - 1) / blockHeight)*blockHeight);
		uint32_t outputDepth  = input->m_depth;

		if (_options.mips
		&&  _options.mipSkip != 0)
		{
			for (uint32_t ii = 0; ii < _options.mipSkip; ++ii)
			{
				outputWidth  = bx::max(blockWidth  * minBlockX, ( ( (outputWidth>>1)  + blockWidth  - 1) / blockWidth )*blockWidth);
				outputHeight = bx::max(blockHeight * minBlockY, ( ( (outputHeight>>1) + blockHeight - 1) / blockHeight)*blockHeight);
				outputDepth  = bx::max(outputDepth>>1, 1u);
			}
		}

		if (_options.equirect)
		{
			if (outputDepth   == 1
			&&  outputWidth/2 == outputHeight)
			{
				if (outputWidth/2 > _options.maxSize)
				{
					outputWidth  = _options.maxSize*4;
					outputHeight = _options.maxSize*2;
				}
			}
			else
			{
				bimg::imageFree(input);

				BX_ERROR_SET(_err, TEXTRUREC_ERROR, "Input image format is not equirectangular projection.");
				return NULL;
			}
		}
		else if (_options.strip)
		{
			if (outputDepth == 1
			&& ( (outputWidth == outputHeight*6) || (outputWidth*6 == outputHeight) ) )
			{
				const bool horizontal = outputWidth == outputHeight*6;

				outputWidth  = bx::min(outputWidth,  horizontal ? _options.maxSize*6 : _options.maxSize);
				outputHeight = bx::min(outputHeight, horizontal ? _options.maxSize   : _options.maxSize*6);
			}
			else
			{
				bimg::imageFree(input);

				BX_ERROR_SET(_err, TEXTRUREC_ERROR, "Input image format is not horizontal or vertical strip.");
				return NULL;
			}
		}
		else if (outputWidth  > _options.maxSize
			 ||  outputHeight > _options.maxSize
			 ||  outputDepth  > _options.maxSize)
		{
			if (outputDepth > outputWidth
			&&  outputDepth > outputHeight)
			{
				outputWidth  = outputWidth  * _options.maxSize / outputDepth;
				outputHeight = outputHeight * _options.maxSize / outputDepth;
				outputDepth  = _options.maxSize;
			}
			else if (outputWidth > outputHeight)
			{
				outputDepth  = outputDepth  * _options.maxSize / outputWidth;
				outputHeight = outputHeight * _options.maxSize / outputWidth;
				outputWidth  = _options.maxSize;
			}
			else
			{
				outputDepth  = outputDepth * _options.maxSize / outputHeight;
				outputWidth  = outputWidth * _options.maxSize / outputHeight;
				outputHeight = _options.maxSize;
			}
		}

		const bool needResize = false
			|| input->m_width  != outputWidth
			|| input->m_height != outputHeight
			;

		const bool passThru = true
			&& !needResize
			&& (1 < input->m_numMips) == _options.mips
			&& !_options.sdf
			&& !_options.alphaTest
			&& !_options.normalMap
			&& !(_options.equirect || _options.strip)
			&& !_options.iqa
			&& !_options.pma
			&& (bimg::LightingModel::Count == _options.radiance)
			;

		if (!_options.sdf
		&&  needResize)
		{
			bimg::ImageContainer* src = bimg::imageConvert(_allocator, bimg::TextureFormat::RGBA32F, *input, false);

			bimg::ImageContainer* dst = bimg::imageAlloc(
				  _allocator
				, bimg::TextureFormat::RGBA32F
				, uint16_t(outputWidth)
				, uint16_t(outputHeight)
				, uint16_t(outputDepth)
				, input->m_numLayers
				, input->m_cubeMap
				, false
				);

			if (!_options.linear)
			{
				bimg::imageRgba32fToLinear(src);
			}

			bimg::imageResizeRgba32fLinear(dst, src);

			if (!_options.linear)
			{
				bimg::imageRgba32fToGamma(dst);
			}

			bimg::imageFree(src);
			bimg::imageFree(input);

			if (bimg::isCompressed(inputFormat) )
			{
				if (inputFormat == bimg::TextureFormat::BC6H)
				{
					inputFormat = bimg::TextureFormat::RGBA32F;
				}
				else
				{
					inputFormat = bimg::TextureFormat::RGBA8;
				}
			}

			input = bimg::imageConvert(_allocator, inputFormat, *dst);
			bimg::imageFree(dst);
		}

		if (passThru)
		{
			if (inputFormat != outputFormat
			&&  bimg::isCompressed(outputFormat) )
			{
				output = bimg::imageEncode(_allocator, outputFormat, _options.quality, *input);
			}
			else
			{
				output = bimg::imageConvert(_allocator, outputFormat, *input);
			}

			bimg::imageFree(input);
			return output;
		}

		if (_options.equirect
		||  _options.strip)
		{
			bimg::ImageContainer* src = bimg::imageConvert(_allocator, bimg::TextureFormat::RGBA32F, *input);
			bimg::imageFree(input);

			bimg::ImageContainer* dst;

			if (outputWidth == outputHeight*2)
			{
				dst = bimg::imageCubemapFromLatLongRgba32F(_allocator, *src, true, _err);
				bimg::imageFree(src);
			}
			else
			{
				dst = bimg::imageCubemapFromStripRgba32F(_allocator, *src, _err);
				bimg::imageFree(src);
			}

			if (!_err->isOk() )
			{
				return NULL;
			}

			input = bimg::imageConvert(_allocator, inputFormat, *dst);
			bimg::imageFree(dst);
		}

		if (bimg::LightingModel::Count != _options.radiance)
		{
			output = bimg::imageCubemapRadianceFilter(_allocator, *input, _options.radiance, _err);

			if (!_err->isOk() )
			{
				return NULL;
			}

			if (bimg::TextureFormat::RGBA32F != outputFormat)
			{
				bimg::ImageContainer* temp = bimg::imageEncode(_allocator, outputFormat, _options.quality, *output);
				bimg::imageFree(output);

				output = temp;
			}

			bimg::imageFree(input);
			return output;
		}

		output = bimg::imageAlloc(
			  _allocator
			, outputFormat
			, uint16_t(input->m_width)
			, uint16_t(input->m_height)
			, uint16_t(input->m_depth)
			, input->m_numLayers
			, input->m_cubeMap
			, _options.mips
			);

		const uint8_t  numMips  = output->m_numMips;
		const uint16_t numSides = output->m_numLayers * (output->m_cubeMap ? 6 : 1);

		for (uint16_t side = 0; side < numSides && _err->isOk(); ++side)
		{
			bimg::ImageMip mip;
			if (bimg::imageGetRawData(*input, side, 0, input->m_data, input->m_size, mip) )
			{
				bimg::ImageMip dstMip;
				bimg::imageGetRawData(*output, side, 0, output->m_data, output->m_size, dstMip);
				uint8_t* dstData = const_cast<uint8_t*>(dstMip.m_data);

				void* temp = NULL;

				// Normal map.
				if (_options.normalMap)
				{
					uint32_t size = bimg::imageGetSize(
						  NULL
						, uint16_t(dstMip.m_width)
						, uint16_t(dstMip.m_height)
						, 0
						, false
						, false
						, 1
						, bimg::TextureFormat::RGBA32F
						);
					temp = BX_ALLOC(_allocator, size);
					float* rgba = (float*)temp;
					float* rgbaDst = (float*)BX_ALLOC(_allocator, size);

					bimg::imageDecodeToRgba32f(_allocator
						, rgba
						, mip.m_data
						, dstMip.m_width
						, dstMip.m_height
						, dstMip.m_depth
						, dstMip.m_width*16
						, mip.m_format
						);

					if (bimg::TextureFormat::BC5 != mip.m_format)
					{
						for (uint32_t yy = 0; yy < mip.m_height; ++yy)
						{
							for (uint32_t xx = 0; xx < mip.m_width; ++xx)
							{
								const uint32_t offset = (yy*mip.m_width + xx) * 4;
								float* inout = &rgba[offset];
								inout[0] = inout[0] * 2.0f - 1.0f;
								inout[1] = inout[1] * 2.0f - 1.0f;
								inout[2] = inout[2] * 2.0f - 1.0f;
								inout[3] = inout[3] * 2.0f - 1.0f;
							}
						}
					}

					imageRgba32fNormalize(rgba
						, dstMip.m_width
						, dstMip.m_height
						, dstMip.m_width*16
						, rgba
						);

					bimg::imageRgba32f11to01(rgbaDst
						, dstMip.m_width
						, dstMip.m_height
						, dstMip.m_depth
						, dstMip.m_width*16
						, rgba
						);

					bimg::Quality::Enum nmapQuality = bimg::Quality::Enum(_options.quality + bimg::Quality::NormalMapDefault);
					bimg::imageEncodeFromRgba32f(_allocator
						, dstData
						, rgbaDst
						, dstMip.m_width
						, dstMip.m_height
						, dstMip.m_depth
						, outputFormat
						, nmapQuality
						, _err
						);

					for (uint8_t lod = 1; lod < numMips && _err->isOk(); ++lod)
					{
						bimg::imageRgba32fDownsample2x2NormalMap(rgba
							, dstMip.m_width
							, dstMip.m_height
							, dstMip.m_width*16
							, bx::strideAlign(dstMip.m_width/2, blockWidth)*16
							, rgba
							);

						bimg::imageRgba32f11to01(rgbaDst
							, dstMip.m_width
							, dstMip.m_height
							, dstMip.m_depth
							, dstMip.m_width*16
							, rgba
							);

						bimg::imageGetRawData(*output, side, lod, output->m_data, output->m_size, dstMip);
						dstData = const_cast<uint8_t*>(dstMip.m_data);

						bimg::imageEncodeFromRgba32f(_allocator
							, dstData
							, rgbaDst
							, dstMip.m_width
							, dstMip.m_height
							, dstMip.m_depth
							, outputFormat
							, nmapQuality
							, _err
							);
					}

					BX_FREE(_allocator, rgbaDst);
				}
				// HDR
				else if ( (!bimg::isCompressed(inputFormat) && 8 != inputBlockInfo.rBits)
					 || outputFormat == bimg::TextureFormat::BC6H
					 || outputFormat == bimg::TextureFormat::BC7
						)
				{
					uint32_t size = bimg::imageGetSize(
						  NULL
						, uint16_t(dstMip.m_width)
						, uint16_t(dstMip.m_height)
						, uint16_t(dstMip.m_depth)
						, false
						, false
						, 1
						, bimg::TextureFormat::RGBA32F
						);
					temp = BX_ALLOC(_allocator, size);
					float* rgba32f = (float*)temp;
					float* rgbaDst = (float*)BX_ALLOC(_allocator, size);

					bimg::imageDecodeToRgba32f(_allocator
						, rgba32f
						, mip.m_data
						, mip.m_width
						, mip.m_height
						, mip.m_depth
						, mip.m_width*16
						, mip.m_format
						);

					if (_options.pma)
					{
						imagePremultiplyAlpha(
							  rgba32f
							, dstMip.m_width
							, dstMip.m_height
							, dstMip.m_depth
							, bimg::TextureFormat::RGBA32F
							);
					}

					bimg::imageEncodeFromRgba32f(_allocator
						, dstData
						, rgba32f
						, dstMip.m_width
						, dstMip.m_height
						, dstMip.m_depth
						, outputFormat
						, _options.quality
						, _err
						);

					if (1 < numMips
					&&  _err->isOk() )
					{
						bimg::imageRgba32fToLinear(rgba32f
							, mip.m_width
							, mip.m_height
							, mip.m_depth
							, mip.m_width*16
							, rgba32f
							);

						for (uint8_t lod = 1; lod < numMips && _err->isOk(); ++lod)
						{
							bimg::imageRgba32fLinearDownsample2x2(rgba32f
								, dstMip.m_width
								, dstMip.m_height
								, dstMip.m_depth
								, dstMip.m_width*16
								, rgba32f
								);

							if (_options.pma)
							{
								imagePremultiplyAlpha(
									  rgba32f
									, dstMip.m_width
									, dstMip.m_height
									, dstMip.m_depth
									, bimg::TextureFormat::RGBA32F
									);
							}

							bimg::imageGetRawData(*output, side, lod, output->m_data, output->m_size, dstMip);
							dstData = const_cast<uint8_t*>(dstMip.m_data);

							bimg::imageRgba32fToGamma(rgbaDst
								, mip.m_width
								, mip.m_height
								, mip.m_depth
								, mip.m_width*16
								, rgba32f
								);

							bimg::imageEncodeFromRgba32f(_allocator
								, dstData
								, rgbaDst
								, dstMip.m_width
								, dstMip.m_height
								, dstMip.m_depth
								, outputFormat
								, _options.quality
								, _err
								);
						}
					}

					BX_FREE(_allocator, rgbaDst);
				}
				// SDF
				else if (_options.sdf)
				{
					uint32_t size = bimg::imageGetSize(
						  NULL
						, uint16_t(dstMip.m_width)
						, uint16_t(dstMip.m_height)
						, uint16_t(dstMip.m_depth)
						, false
						, false
						, 1
						, bimg::TextureFormat::R8
						);
					temp = BX_ALLOC(_allocator, size);
					uint8_t* r8 = (uint8_t*)temp;

					bimg::imageDecodeToR8(_allocator
						, r8
						, mip.m_data
						, mip.m_width
						, mip.m_height
						, mip.m_depth
						, mip.m_width
						, mip.m_format
						);

					bimg::imageGetRawData(*output, side, 0, output->m_data, output->m_size, dstMip);
					dstData = const_cast<uint8_t*>(dstMip.m_data);

					bimg::imageMakeDist(_allocator
						, dstData
						, mip.m_width
						, mip.m_height
						, mip.m_width
						, r8
						);

					if (_options.mips) {
						const float alphaRef = 0.5f;
						float coverage = bimg::imageAlphaTestCoverage(bimg::TextureFormat::A8
							, mip.m_width
							, mip.m_height
							, mip.m_width
							, r8
							, alphaRef
						);

						size = bimg::imageGetSize(
							NULL
							, uint16_t(dstMip.m_width)
							, uint16_t(dstMip.m_height)
							, uint16_t(dstMip.m_depth)
							, false
							, false
							, 1
							, bimg::TextureFormat::RGBA8
						);
						void* rgbaTemp = BX_ALLOC(_allocator, size);
						uint8_t* rgba = (uint8_t*)rgbaTemp;

						bimg::imageDecodeToRgba8(
							_allocator
							, rgba
							, dstMip.m_data
							, dstMip.m_width
							, dstMip.m_height
							, dstMip.m_width * 4
							, bimg::TextureFormat::A8
						);

						for (uint8_t lod = 1; lod < numMips && _err->isOk(); ++lod) {
							bimg::imageRgba8Downsample2x2(rgba
								, dstMip.m_width
								, dstMip.m_height
								, dstMip.m_depth
								, dstMip.m_width * 4
								, bx::strideAlign(dstMip.m_width / 2, blockWidth) * 4
								, rgba
							);

							// For each mip, upscale to original size,
							// scale image alpha to get same coverage as mip0
							uint32_t upsample   = 1 << lod;
							uint32_t destWidth  = dstMip.m_width / 2;
							uint32_t destHeight = dstMip.m_height / 2;
							bimg::imageScaleAlphaToCoverage(bimg::TextureFormat::RGBA8
								, destWidth
								, destHeight
								, destWidth * 4
								, rgba
								, coverage
								, alphaRef
								, upsample
							);

							bimg::imageGetRawData(*output, side, lod, output->m_data, output->m_size, dstMip);
							dstData = const_cast<uint8_t*>(dstMip.m_data);

							bimg::imageEncodeFromRgba8(
								_allocator
								, dstData
								, rgba
								, dstMip.m_width
								, dstMip.m_height
								, dstMip.m_depth
								, bimg::TextureFormat::A8
								, _options.quality
								, _err
							);
						}

						BX_FREE(_allocator, rgbaTemp);
					}
				}
				// RGBA8
				else
				{
					uint32_t size = bimg::imageGetSize(
						  NULL
						, uint16_t(dstMip.m_width)
						, uint16_t(dstMip.m_height)
						, uint16_t(dstMip.m_depth)
						, false
						, false
						, 1
						, bimg::TextureFormat::RGBA8
						);
					temp = BX_ALLOC(_allocator, size);
					uint8_t* rgba = (uint8_t*)temp;

					bimg::imageDecodeToRgba8(
						  _allocator
						, rgba
						, mip.m_data
						, mip.m_width
						, mip.m_height
						, mip.m_width*4
						, mip.m_format
						);

					float coverage = 0.0f;
					if (_options.alphaTest)
					{
						coverage = bimg::imageAlphaTestCoverage(bimg::TextureFormat::RGBA8
							, mip.m_width
							, mip.m_height
							, mip.m_width*4
							, rgba
							, _options.edge
							);
					}

					void* ref = NULL;
					if (_options.iqa)
					{
						ref = BX_ALLOC(_allocator, size);
						bx::memCopy(ref, rgba, size);
					}

					if (_options.pma)
					{
						imagePremultiplyAlpha(
							  rgba
							, dstMip.m_width
							, dstMip.m_height
							, dstMip.m_depth
							, bimg::TextureFormat::RGBA8
							);
					}

					bimg::imageGetRawData(*output, side, 0, output->m_data, output->m_size, dstMip);
					dstData = const_cast<uint8_t*>(dstMip.m_data);

					bimg::imageEncodeFromRgba8(
						  _allocator
						, dstData
						, rgba
						, dstMip.m_width
						, dstMip.m_height
						, dstMip.m_depth
						, outputFormat
						, _options.quality
						, _err
						);

					for (uint8_t lod = 1; lod < numMips && _err->isOk(); ++lod)
					{
						bimg::imageRgba8Downsample2x2(rgba
							, dstMip.m_width
							, dstMip.m_height
							, dstMip.m_depth
							, dstMip.m_width*4
							, bx::strideAlign(dstMip.m_width/2, blockWidth)*4
							, rgba
							);

						if (_options.alphaTest)
						{
							bimg::imageScaleAlphaToCoverage(bimg::TextureFormat::RGBA8
								, dstMip.m_width
								, dstMip.m_height
								, dstMip.m_width*4
								, rgba
								, coverage
								, _options.edge
								);
						}

						if (_options.pma)
						{
							imagePremultiplyAlpha(
								  rgba
								, dstMip.m_width
								, dstMip.m_height
								, dstMip.m_depth
								, bimg::TextureFormat::RGBA8
								);
						}

						bimg::imageGetRawData(*output, side, lod, output->m_data, output->m_size, dstMip);
						dstData = const_cast<uint8_t*>(dstMip.m_data);

						bimg::imageEncodeFromRgba8(
							  _allocator
							, dstData
							, rgba
							, dstMip.m_width
							, dstMip.m_height
							, dstMip.m_depth
							, outputFormat
							, _options.quality
							, _err
							);
					}

					if (NULL != ref)
					{
						bimg::imageDecodeToRgba8(
							  _allocator
							, rgba
							, output->m_data
							, mip.m_width
							, mip.m_height
							, mip.m_width*mip.m_bpp/8
							, outputFormat
							);

						float result = bimg::imageQualityRgba8(
							  ref
							, rgba
							, uint16_t(mip.m_width)
							, uint16_t(mip.m_height)
							);

						bx::printf("%f\n", result);

						BX_FREE(_allocator, ref);
					}
				}

				BX_FREE(_allocator, temp);
			}
		}

		bimg::imageFree(input);
	}

	if (!_err->isOk()
	&&  NULL != output)
	{
		bimg::imageFree(output);
		output = NULL;
	}

	return output;
}

void help(const char* _error = NULL, bool _showHelp = true)
{
	if (NULL != _error)
	{
		bx::printf("Error:\n%s\n\n", _error);

		if (!_showHelp)
		{
			return;
		}
	}

	bx::printf(
		  "texturec, bgfx texture compiler tool, version %d.%d.%d.\n"
		  "Copyright 2011-2021 Branimir Karadzic. All rights reserved.\n"
		  "License: https://github.com/bkaradzic/bimg#license-bsd-2-clause\n\n"
		, BIMG_TEXTUREC_VERSION_MAJOR
		, BIMG_TEXTUREC_VERSION_MINOR
		, BIMG_API_VERSION
		);

	bx::printf(
		  "Usage: texturec -f <in> -o <out> [-t <texture format>]\n"

		  "\n"
		  "Supported file formats:\n"
		  "    *.bmp (input)          Windows Bitmap.\n"
		  "    *.dds (input, output)  Direct Draw Surface.\n"
		  "    *.exr (input, output)  OpenEXR.\n"
		  "    *.gif (input)          Graphics Interchange Format.\n"
		  "    *.jpg (input)          JPEG Interchange Format.\n"
		  "    *.hdr (input, output)  Radiance RGBE.\n"
		  "    *.ktx (input, output)  Khronos Texture.\n"
		  "    *.png (input, output)  Portable Network Graphics.\n"
		  "    *.psd (input)          Photoshop Document.\n"
		  "    *.pvr (input)          PowerVR.\n"
		  "    *.tga (input)          Truevision TGA.\n"

		  "\n"
		  "Options:\n"
		  "  -h, --help               Help.\n"
		  "  -v, --version            Version information only.\n"
		  "  -f <file path>           Input file path.\n"
		  "  -o <file path>           Output file path.\n"
		  "  -t <format>              Output format type (BC1/2/3/4/5, ETC1, PVR14, etc.).\n"
		  "  -q <quality>             Encoding quality (default, fastest, highest).\n"
		  "  -m, --mips               Generate mip-maps.\n"
		  "      --mipskip <N>        Skip <N> number of mips.\n"
		  "  -n, --normalmap          Input texture is normal map. (Implies --linear)\n"
		  "      --equirect           Input texture is equirectangular projection of cubemap.\n"
		  "      --strip              Input texture is horizontal or vertical strip of cubemap.\n"
		  "      --sdf                Compute SDF texture.\n"
		  "      --ref <alpha>        Alpha reference value.\n"
		  "      --iqa                Image Quality Assessment\n"
		  "      --pma                Premultiply alpha into RGB channel.\n"
		  "      --linear             Input and output texture is linear color space (gamma correction won't be applied).\n"
		  "      --max <max size>     Maximum width/height (image will be scaled down and\n"
		  "                           aspect ratio will be preserved)\n"
		  "      --radiance <model>   Radiance cubemap filter. (Lighting model: Phong, PhongBrdf, Blinn, BlinnBrdf, GGX)\n"
		  "      --as <extension>     Save as.\n"
		  "      --formats            List all supported formats.\n"
		  "      --validate           *DEBUG* Validate that output image produced matches after loading.\n"

		  "\n"
		  "For additional information, see https://github.com/bkaradzic/bimg\n"
		);
}

void help(const bx::StringView _str, const bx::Error& _err)
{
	std::string str;
	if (!_str.isEmpty() )
	{
		str.append(_str.getPtr(), _str.getTerm() - _str.getPtr() );
		str.append(": ");
	}

	const bx::StringView& sv = _err.getMessage();
	str.append("'");
	str.append(sv.getPtr(), sv.getTerm() - sv.getPtr() );
	str.append("'");

	help(str.c_str(), false);
}

class AlignedAllocator : public bx::AllocatorI
{
public:
	AlignedAllocator(bx::AllocatorI* _allocator, size_t _minAlignment)
		: m_allocator(_allocator)
		, m_minAlignment(_minAlignment)
	{
	}

	virtual void* realloc(
			void* _ptr
		, size_t _size
		, size_t _align
		, const char* _file
		, uint32_t _line
		)
	{
		return m_allocator->realloc(_ptr, _size, bx::max(_align, m_minAlignment), _file, _line);
	}

	bx::AllocatorI* m_allocator;
	size_t m_minAlignment;
};

int main(int _argc, const char* _argv[])
{
	bx::CommandLine cmdLine(_argc, _argv);

	if (cmdLine.hasArg('v', "version") )
	{
		bx::printf(
			  "texturec, bgfx texture compiler tool, version %d.%d.%d.\n"
			, BIMG_TEXTUREC_VERSION_MAJOR
			, BIMG_TEXTUREC_VERSION_MINOR
			, BIMG_API_VERSION
			);
		return bx::kExitSuccess;
	}

	if (cmdLine.hasArg('h', "help") )
	{
		help();
		return bx::kExitFailure;
	}

    if (cmdLine.hasArg("formats"))
    {
		bx::printf("Uncompressed formats:\n");

		for (int format = bimg::TextureFormat::Unknown + 1; format < bimg::TextureFormat::UnknownDepth; format++)
		{
			bx::printf("  %s\n", bimg::getName((bimg::TextureFormat::Enum) format));
		}

		for (int format = bimg::TextureFormat::UnknownDepth + 1; format < bimg::TextureFormat::Count; format++)
		{
			bx::printf("  %s\n", bimg::getName((bimg::TextureFormat::Enum) format));
		}

		bx::printf("Compressed formats:\n");

		for (int format = 0; format < bimg::TextureFormat::Unknown; format++)
		{
			bx::printf("  %s\n", bimg::getName((bimg::TextureFormat::Enum) format));
		}

		return bx::kExitSuccess;
    }

	const char* inputFileName = cmdLine.findOption('f');
	if (NULL == inputFileName)
	{
		help("Input file must be specified.");
		return bx::kExitFailure;
	}

	const char* outputFileName = cmdLine.findOption('o');
	if (NULL == outputFileName)
	{
		help("Output file must be specified.");
		return bx::kExitFailure;
	}

	bx::StringView saveAs = cmdLine.findOption("as");
	saveAs = saveAs.isEmpty() ? bx::strFindI(outputFileName, ".ktx") : saveAs;
	saveAs = saveAs.isEmpty() ? bx::strFindI(outputFileName, ".dds") : saveAs;
	saveAs = saveAs.isEmpty() ? bx::strFindI(outputFileName, ".png") : saveAs;
	saveAs = saveAs.isEmpty() ? bx::strFindI(outputFileName, ".exr") : saveAs;
	saveAs = saveAs.isEmpty() ? bx::strFindI(outputFileName, ".hdr") : saveAs;
	if (saveAs.isEmpty() )
	{
		help("Output file format must be specified.");
		return bx::kExitFailure;
	}

	Options options;

	const char* alphaRef = cmdLine.findOption("ref");
	if (NULL != alphaRef)
	{
		options.alphaTest = true;
		if (!bx::fromString(&options.edge, alphaRef))
		{
			options.edge = 0.5f;
		}
	}

	options.sdf       = cmdLine.hasArg("sdf");
	options.mips      = cmdLine.hasArg('m', "mips");
	options.normalMap = cmdLine.hasArg('n', "normalmap");
	options.equirect  = cmdLine.hasArg("equirect");
	options.strip     = cmdLine.hasArg("strip");
	options.iqa       = cmdLine.hasArg("iqa");
	options.pma       = cmdLine.hasArg("pma");
	options.linear    = cmdLine.hasArg("linear");

	if (options.equirect
	&&  options.strip)
	{
		help("Image can't be equirect and strip at the same time.");
		return bx::kExitFailure;
	}

	// Normal maps are always linear
	if (options.normalMap)
	{
		options.linear = true;
	}

	const char* maxSize = cmdLine.findOption("max");
	if (NULL != maxSize)
	{
		if (!bx::fromString(&options.maxSize, maxSize) )
		{
			help("Parsing `--max` failed.");
			return bx::kExitFailure;
		}
	}

	const char* mipSkip = cmdLine.findOption("mipskip");
	if (NULL != mipSkip)
	{
		if (!bx::fromString(&options.mipSkip, mipSkip) )
		{
			help("Parsing `--mipskip` failed.");
			return bx::kExitFailure;
		}
	}

	options.format = bimg::TextureFormat::Count;
	const char* type = cmdLine.findOption('t');
	if (NULL != type)
	{
		options.format = bimg::getFormat(type);

		if (!bimg::isValid(options.format) )
		{
			help("Invalid format specified.");
			return bx::kExitFailure;
		}
	}

	if (!bx::strFindI(saveAs, "png").isEmpty() )
	{
		if (options.format == bimg::TextureFormat::Count)
		{
			options.format = bimg::TextureFormat::RGBA8;
		}
		else if (options.format != bimg::TextureFormat::RGBA8)
		{
			help("Output PNG format must be RGBA8.");
			return bx::kExitFailure;
		}
	}
	else if (!bx::strFindI(saveAs, "exr").isEmpty() )
	{
		if (options.format == bimg::TextureFormat::Count)
		{
			options.format = bimg::TextureFormat::RGBA16F;
		}
		else if (options.format != bimg::TextureFormat::RGBA16F)
		{
			help("Output EXR format must be RGBA16F.");
			return bx::kExitFailure;
		}
	}

	const char* quality = cmdLine.findOption('q');
	if (NULL != quality)
	{
		switch (bx::toLower(quality[0]) )
		{
		case 'h': options.quality = bimg::Quality::Highest; break;
		case 'f': options.quality = bimg::Quality::Fastest; break;
		case 'd': options.quality = bimg::Quality::Default; break;
		default:
			help("Invalid quality specified.");
			return bx::kExitFailure;
		}
	}

	const char* radiance = cmdLine.findOption("radiance");
	if (NULL != radiance)
	{
		if      (0 == bx::strCmpI(radiance, "phong"    ) ) { options.radiance = bimg::LightingModel::Phong; }
		else if (0 == bx::strCmpI(radiance, "phongbrdf") ) { options.radiance = bimg::LightingModel::PhongBrdf; }
		else if (0 == bx::strCmpI(radiance, "blinn"    ) ) { options.radiance = bimg::LightingModel::Blinn; }
		else if (0 == bx::strCmpI(radiance, "blinnbrdf") ) { options.radiance = bimg::LightingModel::BlinnBrdf; }
		else if (0 == bx::strCmpI(radiance, "ggx"      ) ) { options.radiance = bimg::LightingModel::Ggx; }
		else
		{
			help("Invalid radiance lighting model specified.");
			return bx::kExitFailure;
		}
	}

	const bool validate = cmdLine.hasArg("validate");

	bx::Error err;
	bx::FileReader reader;
	if (!bx::open(&reader, inputFileName, &err) )
	{
		help("Failed to open input file.", err);
		return bx::kExitFailure;
	}

	uint32_t inputSize = (uint32_t)bx::getSize(&reader);
	if (0 == inputSize)
	{
		help("Failed to read input file.", err);
		return bx::kExitFailure;
	}

	bx::DefaultAllocator defaultAllocator;
	AlignedAllocator allocator(&defaultAllocator, 16);

	uint8_t* inputData = (uint8_t*)BX_ALLOC(&allocator, inputSize);

	bx::read(&reader, inputData, inputSize, &err);
	bx::close(&reader);

	if (!err.isOk() )
	{
		help("Failed to read input file.", err);
		return bx::kExitFailure;
	}

	bimg::ImageContainer* output = convert(&allocator, inputData, inputSize, options, &err);

	BX_FREE(&allocator, inputData);

	if (NULL != output)
	{
		output->m_srgb = !options.linear;

		bx::FileWriter writer;
		if (bx::open(&writer, outputFileName, false, &err) )
		{
			if (!bx::strFindI(saveAs, "ktx").isEmpty() )
			{
				bimg::imageWriteKtx(&writer, *output, output->m_data, output->m_size, &err);
			}
			else if (!bx::strFindI(saveAs, "dds").isEmpty() )
			{
				bimg::imageWriteDds(&writer, *output, output->m_data, output->m_size, &err);
			}
			else if (!bx::strFindI(saveAs, "png").isEmpty() )
			{
				if (output->m_format != bimg::TextureFormat::RGBA8)
				{
					help("Incompatible output texture format. Output PNG format must be RGBA8.", err);
					return bx::kExitFailure;
				}

				bimg::ImageMip mip;
				bimg::imageGetRawData(*output, 0, 0, output->m_data, output->m_size, mip);
				bimg::imageWritePng(&writer
					, mip.m_width
					, mip.m_height
					, mip.m_width*4
					, mip.m_data
					, output->m_format
					, false
					, &err
					);
			}
			else if (!bx::strFindI(saveAs, "exr").isEmpty() )
			{
				bimg::ImageMip mip;
				bimg::imageGetRawData(*output, 0, 0, output->m_data, output->m_size, mip);
				bimg::imageWriteExr(&writer
					, mip.m_width
					, mip.m_height
					, mip.m_width*8
					, mip.m_data
					, output->m_format
					, false
					, &err
					);
			}
			else if (!bx::strFindI(saveAs, "hdr").isEmpty() )
			{
				bimg::ImageMip mip;
				bimg::imageGetRawData(*output, 0, 0, output->m_data, output->m_size, mip);
				bimg::imageWriteHdr(&writer
					, mip.m_width
					, mip.m_height
					, mip.m_width*getBitsPerPixel(mip.m_format)/8
					, mip.m_data
					, output->m_format
					, false
					, &err
					);
			}

			bx::close(&writer);

			if (!err.isOk() )
			{
				help("", err);
				return bx::kExitFailure;
			}
		}
		else
		{
			help("Failed to open output file.", err);
			return bx::kExitFailure;
		}

		if (validate)
		{
			if (!bx::open(&reader, outputFileName, &err) )
			{
				help("Failed to validate file.", err);
				return bx::kExitFailure;
			}

			inputSize = (uint32_t)bx::getSize(&reader);
			if (0 == inputSize)
			{
				help("Failed to validate file.", err);
				return bx::kExitFailure;
			}

			inputData = (uint8_t*)BX_ALLOC(&allocator, inputSize);
			bx::read(&reader, inputData, inputSize, &err);
			bx::close(&reader);

			bimg::ImageContainer* input = bimg::imageParse(&allocator, inputData, inputSize, bimg::TextureFormat::Count, &err);
			if (!err.isOk() )
			{
				help("Failed to validate file.", err);
				return bx::kExitFailure;
			}

			if (false
			||  input->m_format    != output->m_format
			||  input->m_size      != output->m_size
			||  input->m_width     != output->m_width
			||  input->m_height    != output->m_height
			||  input->m_depth     != output->m_depth
			||  input->m_numLayers != output->m_numLayers
			||  input->m_numMips   != output->m_numMips
			||  input->m_hasAlpha  != output->m_hasAlpha
			||  input->m_cubeMap   != output->m_cubeMap
			   )
			{
				help("Validation failed, image headers are different.");
				return bx::kExitFailure;
			}

			{
				const uint8_t  numMips  = output->m_numMips;
				const uint16_t numSides = output->m_numLayers * (output->m_cubeMap ? 6 : 1);

				for (uint8_t lod = 0; lod < numMips; ++lod)
				{
					for (uint16_t side = 0; side < numSides; ++side)
					{
						bimg::ImageMip srcMip;
						bool hasSrc = bimg::imageGetRawData(*input, side, lod, input->m_data, input->m_size, srcMip);

						bimg::ImageMip dstMip;
						bool hasDst = bimg::imageGetRawData(*output, side, lod, output->m_data, output->m_size, dstMip);

						if (false
						||  hasSrc        != hasDst
						||  srcMip.m_size != dstMip.m_size
						   )
						{
							help("Validation failed, image mip/layer/side are different.");
							return bx::kExitFailure;
						}

						if (0 != bx::memCmp(srcMip.m_data, dstMip.m_data, srcMip.m_size) )
						{
							help("Validation failed, image content are different.");
							return bx::kExitFailure;
						}
					}
				}
			}

			BX_FREE(&allocator, inputData);
		}

		bimg::imageFree(output);
	}
	else
	{
		help("Failed to create output", err);
		return bx::kExitFailure;
	}

	return bx::kExitSuccess;
}
