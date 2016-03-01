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

#include "texture.h"

enum
{
	TARGET_STYLE_GUEST = 0,
	TARGET_STYLE_NATIVE,
	TARGET_STYLE_CUSTOM
};

class bgfx_target : public bgfx_texture
{
public:
	bgfx_target(std::string name, bgfx::TextureFormat::Enum format, uint32_t width, uint32_t height, bool filter = false, uint32_t style = TARGET_STYLE_CUSTOM);
	bgfx_target(std::string name, uint32_t width, uint32_t height, uint32_t style, void *handle);
	virtual ~bgfx_target();

	// Getters
	bgfx::FrameBufferHandle target() const { return m_target; }
	uint32_t style() const { return m_style; }
	bool filter() const { return m_filter; }
	virtual bool is_target() const override { return true; }

private:
	bgfx::FrameBufferHandle 	m_target;
	uint32_t					m_style;
	bool						m_filter;
};

#endif // __DRAWBGFX_TARGET__
