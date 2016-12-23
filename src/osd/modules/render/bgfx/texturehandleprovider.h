// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  texturehandleprovider.h - base class for any class that
//  can potentially return information about a texture handle
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_TEXTURE_HANDLE_PROVIDER__
#define __DRAWBGFX_TEXTURE_HANDLE_PROVIDER__

#include <bgfx/bgfx.h>

class bgfx_texture_handle_provider
{
public:
	virtual ~bgfx_texture_handle_provider() { }

	// Getters
	virtual bgfx::TextureHandle texture() const = 0;
	virtual bool is_target() const = 0;
	virtual uint16_t width() const = 0;
	virtual uint16_t height() const = 0;
};

#endif // __DRAWBGFX_TEXTURE_HANDLE_PROVIDER__
