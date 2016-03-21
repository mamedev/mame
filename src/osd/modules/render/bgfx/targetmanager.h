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
class osd_options;

class target_manager {
public:
	target_manager(osd_options& options, texture_manager& textures) : m_textures(textures), m_options(options), m_guest_width(0), m_guest_height(0), m_window_count(0) { }
	~target_manager();

	bgfx_target* create_target(std::string name, bgfx::TextureFormat::Enum format, uint32_t width, uint32_t height, uint32_t prescale_x, uint32_t prescale_y, uint32_t style, bool double_buffer, bool filter, bool output);
	bgfx_target* create_backbuffer(void *handle, uint32_t width, uint32_t height);

	void update_guest_targets(uint16_t width, uint16_t height);
    void update_window_count(uint32_t count);

	// Getters
	bgfx_target* target(std::string name);
	uint16_t guest_width() const { return m_guest_width; }
	uint16_t guest_height() const { return m_guest_height; }

private:
    void rebuild_outputs();
    void rebuild_target(std::string name);

	std::map<std::string, bgfx_target*> m_targets;
	texture_manager& m_textures;
    osd_options& m_options;

	uint16_t m_guest_width;
	uint16_t m_guest_height;
    uint32_t m_window_count;
};

#endif // __DRAWBGFX_TARGET_MANAGER__
