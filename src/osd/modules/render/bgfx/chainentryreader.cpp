// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  chainentryreader.cpp - BGFX chain entry JSON reader
//
//============================================================

#include <string>

#include "emu.h"

#include "chainentryreader.h"

#include "texturemanager.h"
#include "targetmanager.h"
#include "effectmanager.h"
#include "chainentry.h"
#include "entryuniform.h"
#include "slider.h"
#include "inputpair.h"
#include "entryuniformreader.h"

bgfx_chain_entry* chain_entry_reader::read_from_value(const Value& value, texture_manager& textures, target_manager& targets, effect_manager& effects, std::map<std::string, bgfx_slider*>& sliders)
{
	validate_parameters(value);

	bgfx_effect* effect = effects.effect(value["effect"].GetString());

	std::vector<bgfx_input_pair> inputs;
	if (value.HasMember("input"))
	{
		const Value& input_array = value["input"];
		for (UINT32 i = 0; i < input_array.Size(); i++)
		{
			std::string sampler = input_array[i]["sampler"].GetString();
			std::string texture = input_array[i]["texture"].GetString();
			inputs.push_back(bgfx_input_pair(i, sampler, texture));
		}
	}

	std::vector<bgfx_entry_uniform*> uniforms;
	if (value.HasMember("uniforms"))
	{
		const Value& uniform_array = value["uniforms"];
		for (UINT32 i = 0; i < uniform_array.Size(); i++)
		{
			uniforms.push_back(entry_uniform_reader::read_from_value(uniform_array[i], effect, sliders));
		}
	}

    std::string output_name = value["output"].GetString();
    if (output_name != std::string("backbuffer"))
    {
        return new bgfx_chain_entry(value["name"].GetString(), effect, inputs, uniforms, targets.target(output_name));
    }
    else
    {
        return new bgfx_chain_entry(value["name"].GetString(), effect, inputs, uniforms, nullptr);
    }
}

void chain_entry_reader::validate_parameters(const Value& value)
{
	assert(value.HasMember("effect"));
	assert(value["effect"].IsString());
	if (value.HasMember("name"))
	{
		assert(value["name"].IsString());
	}
	assert(value.HasMember("effect"));
	assert(value["effect"].IsString());
	assert(value.HasMember("output"));
	assert(value["output"].IsString());
}
