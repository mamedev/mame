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
	if (!validate_parameters(value, prefix))
	{
		printf("Failed validation\n");
		return nullptr;
	}

	bgfx_effect* effect = effects.effect(value["effect"].GetString());
    if (effect == nullptr)
    {
        return nullptr;
    }

    std::string name = value["name"].GetString();

	std::vector<bgfx_input_pair> inputs;
	if (value.HasMember("input"))
	{
		const Value& input_array = value["input"];
		for (UINT32 i = 0; i < input_array.Size(); i++)
		{
            if (!READER_CHECK(input_array[i].HasMember("sampler"), (prefix + "input[" + std::to_string(i) + ": Must have string value 'sampler' (what sampler are we binding to?)\n").c_str())) return nullptr;
            if (!READER_CHECK(input_array[i]["sampler"].IsString(), (prefix + "input[" + std::to_string(i) + ": Value 'sampler' must be a string\n").c_str())) return nullptr;
            if (!READER_CHECK(input_array[i].HasMember("texture"), (prefix + "input[" + std::to_string(i) + ": Must have string value 'texture' (what texture are we using?)\n").c_str())) return nullptr;
            if (!READER_CHECK(input_array[i]["texture"].IsString(), (prefix + "input[" + std::to_string(i) + ": Value 'texture' must be a string\n").c_str())) return nullptr;
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
			bgfx_entry_uniform* uniform = entry_uniform_reader::read_from_value(uniform_array[i], prefix + "uniforms[" + std::to_string(i) + "]: ", effect, sliders, params);
			if (uniform == nullptr)
			{
				for (bgfx_entry_uniform* existing_uniform : uniforms) delete existing_uniform;
				return nullptr;
			}
			uniforms.push_back(uniform);
		}
	}

	std::vector<bgfx_suppressor*> suppressors;
	if (value.HasMember("disablewhen"))
	{
		const Value& suppressor_array = value["disablewhen"];
		for (UINT32 i = 0; i < suppressor_array.Size(); i++)
		{
			bgfx_suppressor* suppressor = suppressor_reader::read_from_value(suppressor_array[i], prefix, sliders);
			if (suppressor == nullptr)
			{
				for (bgfx_entry_uniform* uniform : uniforms) delete uniform;
				for (bgfx_suppressor* existing_suppressor : suppressors) delete existing_suppressor;
				return nullptr;
			}
			suppressors.push_back(suppressor);
		}
	}

	if (value.HasMember("textures"))
	{
		const Value& texture_array = value["textures"];
		for (UINT32 i = 0; i < texture_array.Size(); i++)
		{
            std::string texture_path = std::string(options.bgfx_path()) + "/artwork/";
            if (!READER_CHECK(texture_array[i].IsString(), (prefix + "textures[" + std::to_string(i) + "]: Value must be a string\n").c_str()))
            {
				for (bgfx_entry_uniform* uniform : uniforms) delete uniform;
				for (bgfx_suppressor* suppressor : suppressors) delete suppressor;
				return nullptr;
			}
            std::string texture_name = texture_array[i].GetString();

            textures.create_png_texture(texture_path, texture_name, texture_name);
		}
	}

    std::string output = value["output"].GetString();
    return new bgfx_chain_entry(name, effect, suppressors, inputs, uniforms, targets, output);
}

bool chain_entry_reader::validate_parameters(const Value& value, std::string prefix)
{
    if (!READER_CHECK(value.HasMember("effect"), (prefix + "Must have string value 'effect' (what effect does this entry use?)\n").c_str())) return false;
    if (!READER_CHECK(value["effect"].IsString(), (prefix + "Value 'effect' must be a string\n").c_str())) return false;
    if (!READER_CHECK(value.HasMember("name"), (prefix + "Must have string value 'effect' (what effect does this entry use?)\n").c_str())) return false;
    if (!READER_CHECK(value["name"].IsString(), (prefix + "Value 'name' must be a string\n").c_str())) return false;
    if (!READER_CHECK(value.HasMember("output"), (prefix + "Must have string value 'offset' (what target are we rendering to?)\n").c_str())) return false;
    if (!READER_CHECK(value["output"].IsString(), (prefix + "Value 'output' must be a string\n").c_str())) return false;
    if (!READER_CHECK(!value.HasMember("input") || value["input"].IsArray(), (prefix + "Value 'input' must be an array\n").c_str())) return false;
    if (!READER_CHECK(!value.HasMember("uniforms") || value["uniforms"].IsArray(), (prefix + "Value 'uniforms' must be an array\n").c_str())) return false;
    if (!READER_CHECK(!value.HasMember("disablewhen") || value["disablewhen"].IsArray(), (prefix + "Value 'disablewhen' must be an array\n").c_str())) return false;
    if (!READER_CHECK(!value.HasMember("textures") || value["textures"].IsArray(), (prefix + "Value 'textures' must be an array\n").c_str())) return false;
    return true;
}