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

#include <map>
#include <string>

#include <bgfx/bgfx.h>

class shader_manager {
public:
	shader_manager() {}
	~shader_manager();

	// Getters
	bgfx::ShaderHandle shader(std::string name);

private:
	bgfx::ShaderHandle load_shader(std::string name);
	static const bgfx::Memory* load_mem(std::string name);

	std::map<std::string, bgfx::ShaderHandle> m_shaders;
};

#endif // __DRAWBGFX_SHADER_MANAGER__
