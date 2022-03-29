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

#include "modules/lib/osdobj_common.h"

#include <map>
#include <string>


class shader_manager {
public:
	shader_manager() { }
	~shader_manager();

	// Getters
	bgfx::ShaderHandle get_or_load_shader(osd_options &options, std::string name);
	static bgfx::ShaderHandle load_shader(osd_options &options, std::string name);
	static bool is_shader_present(osd_options &options, std::string name);

private:
	static std::string make_path_string(osd_options &options, std::string name);
	static const bgfx::Memory* load_mem(std::string name);

	std::map<std::string, bgfx::ShaderHandle>   m_shaders;
};

#endif // __DRAWBGFX_SHADER_MANAGER__
