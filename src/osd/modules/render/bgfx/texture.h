// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  texture.h - Texture abstraction for BGFX layer
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_TEXTURE__
#define __DRAWBGFX_TEXTURE__

#include <bgfx/bgfx.h>

#include <string>

#include "texturehandleprovider.h"

class bgfx_texture : public bgfx_texture_handle_provider
{
public:
	bgfx_texture(std::string name, bgfx::TextureFormat::Enum format, uint16_t width, uint16_t height, uint32_t flags, void* data);
	bgfx_texture(std::string name, bgfx::TextureFormat::Enum format, uint16_t width, uint16_t height, const bgfx::Memory* data, uint32_t flags = BGFX_TEXTURE_U_CLAMP | BGFX_TEXTURE_V_CLAMP);
	virtual ~bgfx_texture();

	// Getters
	std::string name() const { return m_name; }
	bgfx::TextureFormat::Enum format() const { return m_format; }

	// bgfx_texture_handle_provider
	virtual uint16_t width() const override { return m_width; }
	virtual uint16_t height() const override { return m_height; }
	virtual bgfx::TextureHandle texture() const override { return m_texture; }
	virtual bool is_target() const override { return false; }

protected:
	std::string                 m_name;
	bgfx::TextureFormat::Enum   m_format;
	uint16_t                    m_width;
	uint16_t                    m_height;
	bgfx::TextureHandle         m_texture;
};

#endif // __DRAWBGFX_TEXTURE__
