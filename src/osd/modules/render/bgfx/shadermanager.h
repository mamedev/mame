// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  shadermanager.h - BGFX shader manager
//
//  Maintains a mapping between strings and BGFX shaders,
//  either vertex or pixel/fragment.
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_SHADER_MANAGER__
#define __DRAWBGFX_SHADER_MANAGER__

#include <bgfx/bgfx.h>

#include <modules/lib/osdobj_common.h>

#include <map>
#include <string>


class shader_manager {
public:
	shader_manager(osd_options& options) : m_options(options) { }
	~shader_manager();

	// Getters
	bgfx::ShaderHandle shader(std::string name);

private:
	bgfx::ShaderHandle load_shader(std::string name);
	static const bgfx::Memory* load_mem(std::string name);

	std::map<std::string, bgfx::ShaderHandle>   m_shaders;
	osd_options&                                m_options;
};

#endif // __DRAWBGFX_SHADER_MANAGER__
