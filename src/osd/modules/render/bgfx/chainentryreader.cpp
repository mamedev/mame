// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  chainentryreader.cpp - BGFX chain entry JSON reader
//
//============================================================

#include <string>

#include "emu.h"
#include "rendutil.h"

#include <modules/lib/osdobj_common.h>

#include "chainentryreader.h"

#include "texturemanager.h"
#include "targetmanager.h"
#include "effectmanager.h"
#include "chainentry.h"
#include "slider.h"
#include "inputpair.h"
#include "entryuniform.h"
#include "entryuniformreader.h"
#include "suppressor.h"
#include "suppressorreader.h"

bgfx_chain_entry* chain_entry_reader::read_from_value(const Value& value, osd_options& options, texture_manager& textures, target_manager& targets, effect_manager& effects, std::map<std::string, bgfx_slider*>& sliders, std::map<std::string, bgfx_parameter*>& params)
{
	validate_parameters(value);

	bgfx_effect* effect = effects.effect(value["effect"].GetString());

	std::vector<bgfx_input_pair> inputs;
	if (value.HasMember("input"))
	{
		assert(value["input"].IsArray());
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
		assert(value["uniforms"].IsArray());
		const Value& uniform_array = value["uniforms"];
		for (UINT32 i = 0; i < uniform_array.Size(); i++)
		{
			uniforms.push_back(entry_uniform_reader::read_from_value(uniform_array[i], effect, sliders, params));
		}
	}

	std::vector<bgfx_suppressor*> suppressors;
	if (value.HasMember("disablewhen"))
	{
		assert(value["disablewhen"].IsArray());
		const Value& suppressor_array = value["disablewhen"];
		for (UINT32 i = 0; i < suppressor_array.Size(); i++)
		{
			suppressors.push_back(suppressor_reader::read_from_value(suppressor_array[i], sliders));
		}
	}

	if (value.HasMember("textures"))
	{
		assert(value["textures"].IsArray());
		const Value& texture_array = value["textures"];
		for (UINT32 i = 0; i < texture_array.Size(); i++)
		{
			char texture_name[2048];
			bitmap_argb32 bitmap;
			emu_file file(options.bgfx_shadow_mask(), OPEN_FLAG_READ);
			strcpy(texture_name, options.bgfx_shadow_mask());
			render_load_png(bitmap, file, nullptr, texture_name);
		}
	}

    std::string output_name = value["output"].GetString();
    if (output_name != std::string("backbuffer"))
    {
        return new bgfx_chain_entry(value["name"].GetString(), effect, suppressors, inputs, uniforms, targets.target(output_name));
    }
    else
    {
        return new bgfx_chain_entry(value["name"].GetString(), effect, suppressors, inputs, uniforms, nullptr);
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
