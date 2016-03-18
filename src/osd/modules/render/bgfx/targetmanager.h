// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  targetmanager.h - BGFX render target manager
//
//  Maintains a string-to-entry mapping for any registered
//  render targets.
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_TARGET_MANAGER__
#define __DRAWBGFX_TARGET_MANAGER__

#include <map>
#include <string>

#include <bgfx/bgfx.h>

#include "texturemanager.h"

class bgfx_target;

class target_manager {
public:
	target_manager(texture_manager& textures) : m_textures(textures), m_guest_width(16), m_guest_height(16) { }
	~target_manager();

	bgfx_target* create_target(std::string name, bgfx::TextureFormat::Enum format, uint32_t width, uint32_t height, uint32_t prescale_x, uint32_t prescale_y, uint32_t style, bool double_buffer, bool filter);
	bgfx_target* create_backbuffer(void *handle, uint32_t width, uint32_t height);

	void update_guest_targets(uint16_t width, uint16_t height);

	// Getters
	bgfx_target* target(std::string name);
	uint16_t guest_width() const { return m_guest_width; }
	uint16_t guest_height() const { return m_guest_height; }

private:
	std::map<std::string, bgfx_target*> m_targets;
	texture_manager& m_textures;

	uint16_t m_guest_width;
	uint16_t m_guest_height;
};

#endif // __DRAWBGFX_TARGET_MANAGER__
