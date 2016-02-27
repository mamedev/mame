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

class bgfx_target : public bgfx_texture
{
public:
	bgfx_target(std::string name, bgfx::TextureFormat::Enum format, uint32_t width, uint32_t height, bool filter = false);
	bgfx_target(std::string name, void *handle, uint32_t width, uint32_t height);
	virtual ~bgfx_target();

	// Getters
	bgfx::FrameBufferHandle target() const { return m_target; }

private:
	bgfx::FrameBufferHandle m_target;
};

#endif // __DRAWBGFX_TARGET__
