// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  target.h - Render target abstraction for BGFX layer
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_TARGET__
#define __DRAWBGFX_TARGET__

#include <bgfx/bgfx.h>

#include <string>

#include "texturehandleprovider.h"

enum
{
	TARGET_STYLE_GUEST = 0,
	TARGET_STYLE_NATIVE,
	TARGET_STYLE_CUSTOM
};

class bgfx_target : public bgfx_texture_handle_provider
{
public:
	bgfx_target(std::string name, bgfx::TextureFormat::Enum format, uint16_t width, uint16_t height, uint32_t style, bool double_buffer, bool filter, uint16_t scale, uint32_t screen);
	bgfx_target(void *handle, uint16_t width, uint16_t height);
	virtual ~bgfx_target();

	void page_flip();

	// Getters
	bgfx::FrameBufferHandle     target();
	bgfx::TextureFormat::Enum   format() const { return m_format; }
	std::string                 name() const { return m_name; }
	bool                        double_buffered() const { return m_double_buffer; }
	uint32_t                    style() const { return m_style; }
	bool                        filter() const { return m_filter; }
	uint16_t                    scale() const { return m_scale; }
	uint32_t                    screen_index() const { return m_screen; }

	// bgfx_texture_handle_provider
	virtual uint16_t width() const override { return m_width; }
	virtual uint16_t height() const override { return m_height; }
	virtual bgfx::TextureHandle texture() const override;
	virtual bool is_target() const override { return true; }

private:
	std::string                 m_name;
	bgfx::TextureFormat::Enum   m_format;

	bgfx::FrameBufferHandle*    m_targets;
	bgfx::TextureHandle*        m_textures;

	uint16_t                    m_width;
	uint16_t                    m_height;

	bool                        m_double_buffer;
	uint32_t                    m_style;
	bool                        m_filter;
	uint16_t                    m_scale;

	int32_t                     m_screen;

	uint32_t                    m_current_page;

	bool                        m_initialized;

	const uint32_t              m_page_count;
};

#endif // __DRAWBGFX_TARGET__
