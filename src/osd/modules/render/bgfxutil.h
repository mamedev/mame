// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#pragma once

#ifndef __RENDER_BGFX_UTIL__
#define __RENDER_BGFX_UTIL__

#include <bgfx/bgfx.h>

/* sdl_info is the information about SDL for the current screen */
class bgfx_util
{
public:
    static const bgfx::Memory* mame_texture_data_to_bgfx_texture_data(UINT32 format, int width, int height, int rowpixels, const rgb_t *palette, void *base);
    static uint64_t get_blend_state(UINT32 blend);
};

#endif // __RENDER_BGFX_UTIL__