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

#ifndef MAME_RENDER_BGFX_SHADERMANAGER_H
#define MAME_RENDER_BGFX_SHADERMANAGER_H

#pragma once

#include <bgfx/bgfx.h>

#include <map>
#include <string>


class osd_options;


class shader_manager
{
public:
	shader_manager() { }
	~shader_manager();

	// Getters
	bgfx::ShaderHandle get_or_load_shader(const osd_options &options, const std::string &name);
	static bgfx::ShaderHandle load_shader(const osd_options &options, const std::string &name);
	static bool is_shader_present(const osd_options &options, const std::string &name);

private:
	static std::string make_path_string(const osd_options &options, const std::string &name);
	static const bgfx::Memory* load_mem(const std::string &name);

	std::map<std::string, bgfx::ShaderHandle> m_shaders;
};

#endif // MAME_RENDER_BGFX_SHADERMANAGER_H
