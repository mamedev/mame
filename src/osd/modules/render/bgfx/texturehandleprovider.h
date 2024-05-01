// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  texturehandleprovider.h - base class for any class that
//  can potentially return information about a texture handle
//
//============================================================

#ifndef MAME_RENDER_BGFX_TEXTUREHANDLEPROVIDER_H
#define MAME_RENDER_BGFX_TEXTUREHANDLEPROVIDER_H

#pragma once

#include <bgfx/bgfx.h>

class bgfx_texture_handle_provider
{
public:
	virtual ~bgfx_texture_handle_provider() = default;

	// Getters
	virtual bgfx::TextureHandle texture() const = 0;
	virtual bool is_target() const = 0;
	virtual uint16_t width() const = 0;
	virtual uint16_t width_margin() const = 0;
	virtual uint16_t height() const = 0;
	virtual uint16_t rowpixels() const = 0;
	virtual int width_div_factor() const = 0;
	virtual int width_mul_factor() const = 0;
};

#endif // MAME_RENDER_BGFX_TEXTUREHANDLEPROVIDER_H
