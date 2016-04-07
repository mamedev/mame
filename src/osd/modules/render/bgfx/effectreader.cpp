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

#include "effectreader.h"

bgfx_effect* effect_reader::read_from_value(const Value& value, std::string prefix, shader_manager& shaders)
{
	if (!validate_parameters(value, prefix))
	{
		return nullptr;
	}

	uint64_t blend = 0;
	if (value.HasMember("blend"))
	{
		blend = blend_reader::read_from_value(value["blend"]);
	}
	uint64_t depth = depth_reader::read_from_value(value["depth"], prefix + "depth: ");
	uint64_t cull = cull_reader::read_from_value(value["cull"]);
	uint64_t write = write_reader::read_from_value(value["write"]);

	std::vector<bgfx_uniform*> uniforms;
	const Value& uniform_array = value["uniforms"];
	for (UINT32 i = 0; i < uniform_array.Size(); i++)
	{
		bgfx_uniform* uniform = uniform_reader::read_from_value(uniform_array[i], prefix + "uniforms[" + std::to_string(i) + "]: ");
		if (uniform == nullptr)
		{
			return nullptr;
		}
		uniforms.push_back(uniform);
	}

	std::string vertex_name(value["vertex"].GetString());
	bgfx::ShaderHandle vertex_shader = shaders.shader(vertex_name);

	std::string fragment_name("");
	if (value.HasMember("fragment"))
	{
		fragment_name = value["fragment"].GetString();
	}
	else if (value.HasMember("pixel"))
	{
		fragment_name = value["pixel"].GetString();
	}
	bgfx::ShaderHandle fragment_shader = shaders.shader(fragment_name);

	return new bgfx_effect(blend | depth | cull | write, vertex_shader, fragment_shader, uniforms);
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
