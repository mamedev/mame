// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  effectreader.cpp - BGFX effect JSON reader
//
//============================================================

#include <string>

#include <bgfx/bgfx.h>

#include "emu.h"

#include "effect.h"
#include "blendreader.h"
#include "depthreader.h"
#include "cullreader.h"
#include "writereader.h"
#include "shadermanager.h"
#include "uniformreader.h"
#include "uniform.h"

#include "effectreader.h"

bgfx_effect *effect_reader::read_from_value(const Value& value, std::string prefix, osd_options &options, shader_manager& shaders)
{
	uint64_t flags = 0;
	std::string vertex_name;
	std::string fragment_name;
	bgfx::ShaderHandle vertex_shader = BGFX_INVALID_HANDLE;
	bgfx::ShaderHandle fragment_shader = BGFX_INVALID_HANDLE;
	std::vector<bgfx_uniform *> uniforms;

	if (!get_base_effect_data(value, prefix, flags, vertex_name, fragment_name, uniforms))
	{
		return nullptr;
	}

	if (!get_shader_data(value, options, shaders, vertex_name, vertex_shader, fragment_name, fragment_shader))
	{
		clear_uniform_list(uniforms);
		return nullptr;
	}

	bgfx_effect *effect = new bgfx_effect(flags, vertex_shader, fragment_shader, uniforms);
	if (effect->is_valid())
	{
		return effect;
	}

	delete effect;

	return nullptr;
}

bool effect_reader::validate_value(const Value& value, std::string prefix, osd_options &options)
{
	if (!validate_parameters(value, prefix))
	{
		return false;
	}

	uint64_t flags = 0;
	std::string vertex_name;
	std::string fragment_name;
	std::vector<bgfx_uniform *> uniforms;

	if (!get_base_effect_data(value, prefix, flags, vertex_name, fragment_name, uniforms))
	{
		return false;
	}

	if (!shader_manager::is_shader_present(options, vertex_name))
	{
		clear_uniform_list(uniforms);
		return false;
	}

	if (!shader_manager::is_shader_present(options, fragment_name))
	{
		clear_uniform_list(uniforms);
		return false;
	}

	return true;
}

bool effect_reader::get_base_effect_data(const Value& value, std::string &prefix, uint64_t &flags, std::string &vertex_name, std::string &fragment_name,
	std::vector<bgfx_uniform *> &uniforms)
{
	if (!validate_parameters(value, prefix))
	{
		return false;
	}

	uint64_t blend = 0;
	if (value.HasMember("blend"))
	{
		blend = blend_reader::read_from_value(value["blend"]);
	}
	uint64_t depth = depth_reader::read_from_value(value["depth"], prefix + "depth: ");
	uint64_t cull = cull_reader::read_from_value(value["cull"]);
	uint64_t write = write_reader::read_from_value(value["write"]);
	flags = blend | depth | cull | write;

	const Value& uniform_array = value["uniforms"];
	for (uint32_t i = 0; i < uniform_array.Size(); i++)
	{
		bgfx_uniform *uniform = uniform_reader::read_from_value(uniform_array[i], prefix + "uniforms[" + std::to_string(i) + "]: ");
		if (uniform == nullptr)
		{
			return false;
		}
		uniforms.push_back(uniform);
	}

	vertex_name = value["vertex"].GetString();

	if (value.HasMember("fragment"))
	{
		fragment_name = value["fragment"].GetString();
	}
	else if (value.HasMember("pixel"))
	{
		fragment_name = value["pixel"].GetString();
	}
	else
	{
		fragment_name = "";
	}

	return true;
}

bool effect_reader::get_shader_data(const Value& value, osd_options &options, shader_manager &shaders, std::string &vertex_name, bgfx::ShaderHandle &vertex_shader,
	std::string &fragment_name, bgfx::ShaderHandle &fragment_shader)
{
	vertex_shader = shaders.load_shader(options, vertex_name);
	if (vertex_shader.idx == bgfx::kInvalidHandle)
	{
		return false;
	}

	fragment_shader = shaders.load_shader(options, fragment_name);
	if (fragment_shader.idx == bgfx::kInvalidHandle)
	{
		return false;
	}

	return true;
}

void effect_reader::clear_uniform_list(std::vector<bgfx_uniform *> &uniforms)
{
	for (bgfx_uniform *uniform : uniforms)
	{
		delete uniform;
	}
}

bool effect_reader::validate_parameters(const Value& value, std::string prefix)
{
	if (!READER_CHECK(value.HasMember("depth"), (prefix + "Must have object value 'depth' (what are our Z-buffer settings?)\n").c_str())) return false;
	if (!READER_CHECK(value.HasMember("cull"), (prefix + "Must have object value 'cull' (do we cull triangles based on winding?)\n").c_str())) return false;
	if (!READER_CHECK(value.HasMember("write"), (prefix + "Must have object value 'write' (what are our color buffer write settings?)\n").c_str())) return false;
	if (!READER_CHECK(value.HasMember("vertex"), (prefix + "Must have string value 'vertex' (what is our vertex shader?)\n").c_str())) return false;
	if (!READER_CHECK(value["vertex"].IsString(), (prefix + "Value 'vertex' must be a string\n").c_str())) return false;
	if (!READER_CHECK(value.HasMember("fragment") || value.HasMember("pixel"), (prefix + "Must have string value named 'fragment' or 'pixel' (what is our fragment/pixel shader?)\n").c_str())) return false;
	if (!READER_CHECK(!value.HasMember("fragment") || value["fragment"].IsString(), (prefix + "Value 'fragment' must be a string\n").c_str())) return false;
	if (!READER_CHECK(!value.HasMember("pixel") || value["pixel"].IsString(), (prefix + "Value 'pixel' must be a string\n").c_str())) return false;
	if (!READER_CHECK(value.HasMember("uniforms"), (prefix + "Must have array value 'uniforms' (what are our shader's parameters?)\n").c_str())) return false;
	if (!READER_CHECK(value["uniforms"].IsArray(), (prefix + "Value 'uniforms' must be an array\n").c_str())) return false;
	return true;
}
