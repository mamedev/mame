// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  effectreader.h - BGFX effect JSON reader
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_EFFECT_READER__
#define __DRAWBGFX_EFFECT_READER__

#include <string>

#include "statereader.h"

class bgfx_effect;
class bgfx_uniform;
class shader_manager;

class effect_reader : public state_reader
{
public:
	static bgfx_effect *read_from_value(const Value& value, std::string prefix, osd_options &options, shader_manager& shaders);
	static bool validate_value(const Value& value, std::string prefix, osd_options &options);

private:
	static bool get_base_effect_data(const Value& value, std::string &prefix, uint64_t &flags, std::string &vertex_name, std::string &fragment_name,
		std::vector<bgfx_uniform *> &uniforms);
	static bool get_shader_data(const Value& value, osd_options &options, shader_manager &shaders, std::string &vertex_name, bgfx::ShaderHandle &vertex_shader,
		std::string &fragment_name, bgfx::ShaderHandle &fragment_shader);
	static void clear_uniform_list(std::vector<bgfx_uniform *> &uniforms);
	static bool validate_parameters(const Value& value, std::string prefix);
};

#endif // __DRAWBGFX_EFFECT_READER__
