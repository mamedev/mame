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

class bgfx_texture
{
public:
	bgfx_texture(std::string name, bgfx::TextureFormat::Enum format, uint32_t width, uint32_t height, void* data = nullptr, uint32_t flags = BGFX_TEXTURE_U_CLAMP | BGFX_TEXTURE_V_CLAMP);
	virtual ~bgfx_texture();

	// Getters
	std::string name() const { return m_name; }
	bgfx::TextureFormat::Enum format() const { return m_format; }
	uint32_t width() const { return m_width; }
	uint32_t height() const { return m_height; }
	bgfx::TextureHandle handle() const { return m_handle; }

protected:
	std::string                 m_name;
	bgfx::TextureFormat::Enum   m_format;
	uint32_t                    m_width;
	uint32_t                    m_height;
	bgfx::TextureHandle         m_handle;
};

#endif // __DRAWBGFX_TEXTURE__
