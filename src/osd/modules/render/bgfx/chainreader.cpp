// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  chainreader.cpp - BGFX chain JSON reader
//
//============================================================

#include <string>
#include <vector>

#include "emu.h"

#include "chain.h"
#include "sliderreader.h"
#include "paramreader.h"
#include "chainentryreader.h"
#include "targetmanager.h"
#include "chainreader.h"

bgfx_chain* chain_reader::read_from_value(const Value& value, texture_manager& textures, target_manager& targets, effect_manager& effects, uint32_t screen_width, uint32_t screen_height)
{
	validate_parameters(value);

	std::string name = value["name"].GetString();
	std::string author = value["author"].GetString();

	// Parse sliders
	std::vector<bgfx_slider*> sliders;
	if (value.HasMember("sliders"))
	{
		const Value& slider_array = value["sliders"];
		for (UINT32 i = 0; i < slider_array.Size(); i++)
		{
			sliders.push_back(slider_reader::read_from_value(slider_array[i]));
		}
	}

	// Parse parameters
	std::vector<bgfx_parameter*> parameters;
	if (value.HasMember("parameters"))
	{
		const Value& param_array = value["parameters"];
		for (UINT32 i = 0; i < param_array.Size(); i++)
		{
			parameters.push_back(parameter_reader::read_from_value(param_array[i]));
		}
	}

	// Parse chain entries
	std::vector<bgfx_chain_entry*> entries;
	if (value.HasMember("passes"))
	{
		const Value& entry_array = value["passes"];
		for (UINT32 i = 0; i < entry_array.Size(); i++)
		{
			entries.push_back(chain_entry_reader::read_from_value(entry_array[i], textures, targets, effects));
		}
	}

	// Create targets
	if (value.HasMember("targets"))
	{
		const Value& target_array = value["targets"];
		for (UINT32 i = 0; i < target_array.Size(); i++)
		{
			assert(target_array[i].HasMember("name"));
			assert(target_array[i]["name"].IsString());
			uint32_t width = 0;
			uint32_t height = 0;
			if (target_array[i].HasMember("screen") && target_array[i]["screen"].IsBool())
			{
				width = screen_width;
				height = screen_height;
			}
			else
			{
				assert(target_array[i].HasMember("width"));
				assert(target_array[i]["width"].IsDouble());
				assert(target_array[i].HasMember("height"));
				assert(target_array[i]["height"].IsDouble());
				width = uint32_t(target_array[i]["width"].GetDouble());
				height = uint32_t(target_array[i]["height"].GetDouble());
			}
			targets.create_target(target_array[i]["name"].GetString(), bgfx::TextureFormat::RGBA8, width, height);
		}
	}

	return new bgfx_chain(name, author, sliders, parameters, entries);
}

void chain_reader::validate_parameters(const Value& value)
{
	assert(value.HasMember("name"));
	assert(value["name"].IsString());
	assert(value.HasMember("author"));
	assert(value["author"].IsString());
	assert(value.HasMember("passes"));
}
