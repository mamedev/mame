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
#include <modules/render/copyutil.h>

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

bgfx_chain_entry* chain_entry_reader::read_from_value(const Value& value, std::string prefix, osd_options& options, texture_manager& textures, target_manager& targets, effect_manager& effects, std::map<std::string, bgfx_slider*>& sliders, std::map<std::string, bgfx_parameter*>& params)
{
	validate_parameters(value, prefix);

	bgfx_effect* effect = effects.effect(value["effect"].GetString());
    std::string name = value["name"].GetString();

	std::vector<bgfx_input_pair> inputs;
	if (value.HasMember("input"))
	{
        READER_ASSERT(value["input"].IsArray(), (prefix + "Chain entry '" + name + "': value 'input' must be an array\n").c_str());
		const Value& input_array = value["input"];
		for (UINT32 i = 0; i < input_array.Size(); i++)
		{
            READER_ASSERT(input_array[i].HasMember("sampler"), (prefix + "input[" + std::to_string(i) + ": Must have string value 'sampler' (what sampler are we binding to?)\n").c_str());
            READER_ASSERT(input_array[i]["sampler"].IsString(), (prefix + "input[" + std::to_string(i) + ": Value 'sampler' must be a string\n").c_str());
            READER_ASSERT(input_array[i].HasMember("texture"), (prefix + "input[" + std::to_string(i) + ": Must have string value 'texture' (what texture are we using?)\n").c_str());
            READER_ASSERT(input_array[i]["texture"].IsString(), (prefix + "input[" + std::to_string(i) + ": Value 'texture' must be a string\n").c_str());
            std::string sampler = input_array[i]["sampler"].GetString();
			std::string texture = input_array[i]["texture"].GetString();
			inputs.push_back(bgfx_input_pair(i, sampler, texture));
		}
	}

	std::vector<bgfx_entry_uniform*> uniforms;
	if (value.HasMember("uniforms"))
	{
        READER_ASSERT(value["uniforms"].IsArray(), (prefix + "Chain entry '" + name + "': value 'uniforms' must be an array\n").c_str());
		const Value& uniform_array = value["uniforms"];
		for (UINT32 i = 0; i < uniform_array.Size(); i++)
		{
			uniforms.push_back(entry_uniform_reader::read_from_value(uniform_array[i], prefix + "uniforms[" + std::to_string(i) + "]: ", effect, sliders, params));
		}
	}

	std::vector<bgfx_suppressor*> suppressors;
	if (value.HasMember("disablewhen"))
	{
        READER_ASSERT(value["disablewhen"].IsArray(), (prefix + "Chain entry '" + name + "': value 'disablewhen' must be an array\n").c_str());
		const Value& suppressor_array = value["disablewhen"];
		for (UINT32 i = 0; i < suppressor_array.Size(); i++)
		{
			suppressors.push_back(suppressor_reader::read_from_value(suppressor_array[i], prefix, sliders));
		}
	}

	if (value.HasMember("textures"))
	{
        READER_ASSERT(value["textures"].IsArray(), (prefix + "Chain entry '" + name + "': value 'textures' must be an array\n").c_str());
		const Value& texture_array = value["textures"];
		for (UINT32 i = 0; i < texture_array.Size(); i++)
		{
            std::string texture_path = std::string(options.bgfx_path()) + "/artwork/";
            std::string texture_name = texture_array[i].GetString();
			
            textures.create_png_texture(texture_path, texture_name, texture_name);
		}
	}

    std::string output_name = value["output"].GetString();
    if (output_name != std::string("backbuffer"))
    {
        return new bgfx_chain_entry(name, effect, suppressors, inputs, uniforms, targets, output_name);
    }
    else
    {
        return new bgfx_chain_entry(name, effect, suppressors, inputs, uniforms, targets, "none");
    }
}

void chain_entry_reader::validate_parameters(const Value& value, std::string prefix)
{
    READER_ASSERT(value.HasMember("effect"), (prefix + "Must have string value 'effect' (what effect does this entry use?)\n").c_str());
    READER_ASSERT(value["effect"].IsString(), (prefix + "Value 'effect' must be a string\n").c_str());
    READER_ASSERT(value.HasMember("name"), (prefix + "Must have string value 'effect' (what effect does this entry use?)\n").c_str());
    READER_ASSERT(value["name"].IsString(), (prefix + "Value 'name' must be a string\n").c_str());
    READER_ASSERT(value.HasMember("output"), (prefix + "Must have string value 'output' (what target contains the result of this entry?)\n").c_str());
    READER_ASSERT(value["output"].IsString(), (prefix + "Value 'output' must be a string\n").c_str());
}
