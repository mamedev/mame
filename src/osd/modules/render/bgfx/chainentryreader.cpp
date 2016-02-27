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
#include "inputpair.h"

bgfx_chain_entry* chain_entry_reader::read_from_value(const Value& value, texture_manager& textures, target_manager& targets, effect_manager& effects)
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
			inputs.push_back(bgfx_input_pair(i, sampler, textures.texture(texture)));
		}
	}

	bgfx_target* output = targets.target(value["output"].GetString());

	return new bgfx_chain_entry(value["name"].GetString(), effect, inputs, output);
}

void chain_entry_reader::validate_parameters(const Value& value)
{
	assert(value.HasMember("effect"));
	assert(value["effect"].IsString());
	if (value.HasMember("name"))
	{
		assert(value["name"].IsString());
	}
	assert(value.HasMember("shader"));
	assert(value["shader"].IsString());
	assert(value.HasMember("output"));
	assert(value["output"].IsString());
}
