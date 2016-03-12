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

bgfx_effect* effect_reader::read_from_value(shader_manager& shaders, const Value& value)
{
	validate_parameters(value);

	uint64_t blend = blend_reader::read_from_value(value["blend"]);
	uint64_t depth = depth_reader::read_from_value(value["depth"]);
	uint64_t cull = cull_reader::read_from_value(value["cull"]);
	uint64_t write = write_reader::read_from_value(value["write"]);

	std::vector<bgfx_uniform*> uniforms;
	const Value& uniform_array = value["uniforms"];
	for (UINT32 i = 0; i < uniform_array.Size(); i++)
	{
		uniforms.push_back(uniform_reader::read_from_value(uniform_array[i]));
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

void effect_reader::validate_parameters(const Value& value)
{
	assert(value.HasMember("blend"));
	assert(value.HasMember("depth"));
	assert(value.HasMember("cull"));
	assert(value.HasMember("write"));
	assert(value.HasMember("vertex"));
	assert(value["vertex"].IsString());
	assert(value.HasMember("fragment") || value.HasMember("pixel"));
	if (value.HasMember("fragment"))
	{
		assert(value["fragment"].IsString());
	}
	if (value.HasMember("pixel"))
	{
		assert(value["pixel"].IsString());
	}
	assert(value.HasMember("uniforms"));
	assert(value["uniforms"].IsArray());
}
