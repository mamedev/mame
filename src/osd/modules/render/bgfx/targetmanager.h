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
	target_manager(osd_options& options, texture_manager& textures);
	~target_manager();

	bgfx_target* create_target(std::string name, bgfx::TextureFormat::Enum format, uint32_t width, uint32_t height, uint32_t prescale_x, uint32_t prescale_y, uint32_t style, bool double_buffer, bool filter, bool output);
	bgfx_target* create_backbuffer(void *handle, uint32_t width, uint32_t height);

	void update_guest_targets(int32_t screen, uint16_t width, uint16_t height);
    void update_screen_count(uint32_t count);

	// Getters
	bgfx_target* target(std::string name);
	uint16_t guest_width(int32_t screen) const { return m_guest_width[screen]; }
	uint16_t guest_height(int32_t screen) const { return m_guest_height[screen]; }

private:
    void rebuild_guest_targets(int32_t screen);
    void rebuild_outputs();
    void ensure_guest_targets();
    void create_nonexistent_targets(std::string name);
    bool create_guest_if_nonexistent(std::string name, int32_t screen, bool double_buffered, bool filter);

	std::map<std::string, bgfx_target*> m_targets;
	texture_manager& m_textures;
    osd_options& m_options;

	uint16_t *m_guest_width;
	uint16_t *m_guest_height;
    uint32_t m_screen_count;

    static const int32_t MAX_SCREENS;
};

#endif // __DRAWBGFX_TARGET_MANAGER__
