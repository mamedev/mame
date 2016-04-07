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

bgfx_chain_entry* chain_entry_reader::read_from_value(const Value& value, std::string prefix, osd_options& options, texture_manager& textures, target_manager& targets, effect_manager& effects, std::map<std::string, bgfx_slider*>& sliders, std::map<std::string, bgfx_parameter*>& params, uint32_t screen_index)
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
			const Value& input = input_array[i];
			if (!READER_CHECK(input.HasMember("sampler"), (prefix + "input[" + std::to_string(i) + ": Must have string value 'sampler' (what sampler are we binding to?)\n").c_str())) return nullptr;
			if (!READER_CHECK(input["sampler"].IsString(), (prefix + "input[" + std::to_string(i) + ": Value 'sampler' must be a string\n").c_str())) return nullptr;
			bool has_texture = input.HasMember("texture");
			bool has_target = input.HasMember("target");
			bool has_option = input.HasMember("option");
			if (!READER_CHECK(has_texture || has_target || has_option, (prefix + "input[" + std::to_string(i) + ": Must have string value 'target', 'texture' or 'option' (what source are we using?)\n").c_str())) return nullptr;
			if (!READER_CHECK(!has_texture || input["texture"].IsString(), (prefix + "input[" + std::to_string(i) + ": Value 'texture' must be a string\n").c_str())) return nullptr;
			if (!READER_CHECK(!has_target || input["target"].IsString(), (prefix + "input[" + std::to_string(i) + ": Value 'target' must be a string\n").c_str())) return nullptr;
			if (!READER_CHECK(!has_option || input["option"].IsString(), (prefix + "input[" + std::to_string(i) + ": Value 'option' must be a string\n").c_str())) return nullptr;
			if (!READER_CHECK(has_target || !input.HasMember("bilinear") || input["bilinear"].IsBool(), (prefix + "input[" + std::to_string(i) + ": Value 'bilinear' must be a boolean\n").c_str())) return nullptr;

			std::string texture_name = "";
			if (has_texture)
			{
				texture_name = input["texture"].GetString();
				if (texture_name != "screen")
				{
					bool bilinear = get_bool(input, "bilinear", true);
					uint32_t flags = bilinear ? 0 : (BGFX_TEXTURE_MIN_POINT | BGFX_TEXTURE_MAG_POINT | BGFX_TEXTURE_MIP_POINT);
					bgfx_texture* texture = textures.create_png_texture(options.art_path(), texture_name, texture_name, flags, screen_index);
					if (texture == nullptr)
					{
						return nullptr;
					}
				}
			}
			else if (has_target)
			{
				texture_name = input["target"].GetString();
			}
			else if (has_option)
			{
				bool bilinear = get_bool(input, "bilinear", true);
				uint32_t flags = bilinear ? 0 : (BGFX_TEXTURE_MIN_POINT | BGFX_TEXTURE_MAG_POINT | BGFX_TEXTURE_MIP_POINT);
				texture_name = input["option"].GetString();
				bgfx_texture* texture = textures.create_png_texture(options.art_path(), options.value(texture_name.c_str()), texture_name, flags, screen_index);
				if (texture == nullptr)
				{
					return nullptr;
				}
			}
			else
			{
				return nullptr;
			}

			std::string sampler = input["sampler"].GetString();
			inputs.push_back(bgfx_input_pair(i, sampler, texture_name));
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
	return true;
}
