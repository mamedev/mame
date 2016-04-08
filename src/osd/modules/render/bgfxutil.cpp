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

const bgfx::Memory* bgfx_util::mame_texture_data_to_bgfx_texture_data(UINT32 format, int width, int height, int rowpixels, const rgb_t *palette, void *base)
{
	const bgfx::Memory* mem = bgfx::alloc(width * height * 4);
    UINT32* data = reinterpret_cast<UINT32*>(mem->data);
    UINT16* src16 = reinterpret_cast<UINT16*>(base);
    UINT32* src32 = reinterpret_cast<UINT32*>(base);

    for (int y = 0; y < height; y++)
	{
        UINT32* dst_line = data + y * width;
        UINT16* src_line16 = src16 + y * rowpixels;
        UINT32* src_line32 = src32 + y * rowpixels;
		switch (format)
		{
			case PRIMFLAG_TEXFORMAT(TEXFORMAT_PALETTE16):
				copy_util::copyline_palette16(dst_line, src_line16, width, palette);
				break;
			case PRIMFLAG_TEXFORMAT(TEXFORMAT_PALETTEA16):
				copy_util::copyline_palettea16(dst_line, src_line16, width, palette);
				break;
			case PRIMFLAG_TEXFORMAT(TEXFORMAT_YUY16):
				copy_util::copyline_yuy16_to_argb(dst_line, src_line16, width, palette, 1);
				break;
			case PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32):
				copy_util::copyline_argb32(dst_line, src_line32, width, palette);
				break;
			case PRIMFLAG_TEXFORMAT(TEXFORMAT_RGB32):
				copy_util::copyline_rgb32(dst_line, src_line32, width, palette);
				break;
			default:
				break;
		}
	}
	return mem;
}

uint64_t bgfx_util::get_blend_state(UINT32 blend)
{
	switch (blend)
	{
		case BLENDMODE_ALPHA:
			return BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
		case BLENDMODE_RGB_MULTIPLY:
			return BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_DST_COLOR, BGFX_STATE_BLEND_ZERO);
		case BLENDMODE_ADD:
			return BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_ONE);
		default:
			return 0L;
	}
	return 0L;
}
