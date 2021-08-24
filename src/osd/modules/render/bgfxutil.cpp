// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  bgfxutil.cpp - BGFX renderer helper utils
//
//============================================================

// MAMEOS headers
#include "emu.h"
#include "bgfxutil.h"
#include "copyutil.h"

#include "render.h"


const bgfx::Memory* bgfx_util::mame_texture_data_to_bgfx_texture_data(bgfx::TextureFormat::Enum &dst_format, uint32_t src_format, int rowpixels, int height, const rgb_t *palette, void *base, uint16_t &out_pitch, int &convert_stride)
{
	bgfx::TextureInfo info;
	const bgfx::Memory *data = nullptr;

	switch (src_format)
	{
		case PRIMFLAG_TEXFORMAT(TEXFORMAT_YUY16):
			dst_format = bgfx::TextureFormat::BGRA8;
			convert_stride = 2;
			out_pitch = rowpixels * 2;
			bgfx::calcTextureSize(info, rowpixels / convert_stride, height, 1, false, false, 1, dst_format);

			data = bgfx::copy(base, info.storageSize);
			break;
		case PRIMFLAG_TEXFORMAT(TEXFORMAT_PALETTE16):
		{
			dst_format = bgfx::TextureFormat::BGRA8;
			convert_stride = 1;
			out_pitch = rowpixels * 4;
			bgfx::calcTextureSize(info, rowpixels / convert_stride, height, 1, false, false, 1, dst_format);

			uint16_t *src = (uint16_t *)base;
			uint16_t *dst_data = new uint16_t[rowpixels * 2 * height];
			uint16_t *dst = dst_data;
			for (int i = 0; i < rowpixels * height; i++, src++)
			{
				*dst++ = *src;
				*dst++ = 0;
			}

			data = bgfx::copy(dst_data, info.storageSize);
			delete [] dst_data;
			break;
		}
		case PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32):
		case PRIMFLAG_TEXFORMAT(TEXFORMAT_RGB32):
			dst_format = bgfx::TextureFormat::BGRA8;
			convert_stride = 1;
			out_pitch = rowpixels * 4;
			bgfx::calcTextureSize(info, rowpixels / convert_stride, height, 1, false, false, 1, dst_format);

			data = bgfx::copy(base, info.storageSize);
			break;
	}

	return data;
}

const bgfx::Memory* bgfx_util::mame_texture_data_to_bgra32(uint32_t src_format, int width, int height, int rowpixels, const rgb_t *palette, void *base)
{
	const bgfx::Memory* mem = bgfx::alloc(rowpixels * height * 4);
	auto* dst = reinterpret_cast<uint32_t*>(mem->data);
	auto* src16 = reinterpret_cast<uint16_t*>(base);
	auto* src32 = reinterpret_cast<uint32_t*>(base);

	for (int y = 0; y < height; y++)
	{
		switch (src_format)
		{
		case PRIMFLAG_TEXFORMAT(TEXFORMAT_PALETTE16):
			copy_util::copyline_palette16_to_bgra(dst, src16, width, palette);
			src16 += rowpixels;
			break;
		case PRIMFLAG_TEXFORMAT(TEXFORMAT_YUY16):
			copy_util::copyline_yuy16_to_bgra(dst, src16, width, palette, 1);
			src16 += rowpixels;
			break;
		case PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32):
			copy_util::copyline_argb32_to_bgra(dst, src32, width, palette);
			src32 += rowpixels;
			break;
		case PRIMFLAG_TEXFORMAT(TEXFORMAT_RGB32):
			copy_util::copyline_rgb32_to_bgra(dst, src32, width, palette);
			src32 += rowpixels;
			break;
		default:
			break;
		}
		dst += width;
	}
	return mem;
}

uint64_t bgfx_util::get_blend_state(uint32_t blend)
{
	switch (blend)
	{
		case BLENDMODE_ALPHA:
			return BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
		case BLENDMODE_RGB_MULTIPLY:
			return BGFX_STATE_BLEND_FUNC_SEPARATE(BGFX_STATE_BLEND_DST_COLOR, BGFX_STATE_BLEND_ZERO, BGFX_STATE_BLEND_DST_ALPHA, BGFX_STATE_BLEND_ZERO);
		case BLENDMODE_ADD:
			return BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_ONE);
		default:
			return 0L;
	}
	return 0L;
}
