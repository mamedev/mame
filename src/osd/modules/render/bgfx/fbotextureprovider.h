// license:BSD-3-Clause
// copyright-holders:okaz-code
//============================================================
//
//  fbotextureprovider.h - Non-owning wrapper around a GPU-side
//  FBO color attachment texture handle for the bgfx chain system.
//
//  Used to expose a rendered framebuffer (e.g. m_vec_fb) as a
//  named "screen0" input to chain entries without uploading any
//  CPU-side texture data.  Does NOT call bgfx::destroy() on the
//  handle - ownership stays with the FBO creator (drawbgfx.cpp).
//
//============================================================

#pragma once

#ifndef MAME_RENDER_BGFX_FBOTEXTUREPROVIDER_H
#define MAME_RENDER_BGFX_FBOTEXTUREPROVIDER_H

#include <bgfx/bgfx.h>
#include "texturehandleprovider.h"

class bgfx_fbo_texture_provider : public bgfx_texture_handle_provider
{
public:
	bgfx_fbo_texture_provider(bgfx::TextureHandle handle, uint16_t w, uint16_t h)
		: m_texture(handle), m_width(w), m_height(h) {}

	// bgfx_texture_handle_provider interface
	virtual bgfx::TextureHandle texture()    const override { return m_texture; }
	virtual bool     is_target()             const override { return false; }
	virtual uint16_t width()                 const override { return m_width; }
	virtual uint16_t width_margin()          const override { return 0; }
	virtual uint16_t height()                const override { return m_height; }
	virtual uint16_t rowpixels()             const override { return m_width; }
	virtual int      width_div_factor()      const override { return 1; }
	virtual int      width_mul_factor()      const override { return 1; }

private:
	bgfx::TextureHandle m_texture;
	uint16_t            m_width;
	uint16_t            m_height;
};

#endif // MAME_RENDER_BGFX_FBOTEXTUREPROVIDER_H
