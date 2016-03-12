// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  chainreader.cpp - BGFX chain JSON reader
//
//============================================================

#include <string>
#include <vector>
#include <map>

#include "emu.h"

#include "chain.h"
#include "sliderreader.h"
#include "paramreader.h"
#include "chainentryreader.h"
#include "targetmanager.h"
#include "chainreader.h"
#include "target.h"
#include "slider.h"
#include "parameter.h"

const chain_reader::string_to_enum chain_reader::STYLE_NAMES[chain_reader::STYLE_COUNT] = {
	{ "guest",  TARGET_STYLE_GUEST },
	{ "native", TARGET_STYLE_NATIVE },
	{ "custom",	TARGET_STYLE_CUSTOM }
};

bgfx_chain* chain_reader::read_from_value(const Value& value, running_machine& machine, uint32_t window_index, texture_manager& textures, target_manager& targets, effect_manager& effects, uint32_t screen_width, uint32_t screen_height)
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
            std::vector<bgfx_slider*> expanded_sliders = slider_reader::read_from_value(slider_array[i], machine);
            for (bgfx_slider* slider : expanded_sliders)
            {
                sliders.push_back(slider);
            }
		}
	}

	// Map sliders
	std::map<std::string, bgfx_slider*> slider_map;
	for (bgfx_slider* slider : sliders)
	{
		slider_map[slider->name()] = slider;
	}

	// Parse parameters
	std::vector<bgfx_parameter*> parameters;
	if (value.HasMember("parameters"))
	{
		const Value& param_array = value["parameters"];
		for (UINT32 i = 0; i < param_array.Size(); i++)
		{
			parameters.push_back(parameter_reader::read_from_value(param_array[i], window_index));
		}
	}

	// Map parameters
	std::map<std::string, bgfx_parameter*> param_map;
	for (bgfx_parameter* param : parameters)
	{
		param_map[param->name()] = param;
	}

	// Create targets
	if (value.HasMember("targets"))
	{
		const Value& target_array = value["targets"];
        // TODO: Move into its own reader
		for (UINT32 i = 0; i < target_array.Size(); i++)
		{
			assert(target_array[i].HasMember("name"));
			assert(target_array[i]["name"].IsString());
			uint32_t mode = uint32_t(get_enum_from_value(value, "mode", TARGET_STYLE_NATIVE, STYLE_NAMES, STYLE_COUNT));
			bool bilinear = get_bool(target_array[i], "bilinear", false);
			bool double_buffer = get_bool(target_array[i], "doublebuffer", false);

			float prescalef = 1.0f;
			float default_prescale = 1.0f;
			get_float(target_array[i], "prescale", &prescalef, &default_prescale);
			int prescale = (int)floor(prescalef + 0.5f);

			uint16_t width = 0;
			uint16_t height = 0;
			switch (mode)
			{
				case TARGET_STYLE_GUEST:
					width = targets.guest_width();
					height = targets.guest_height();
					break;
				case TARGET_STYLE_NATIVE:
					width = screen_width;
					height = screen_height;
					break;
				case TARGET_STYLE_CUSTOM:
					assert(target_array[i].HasMember("width"));
					assert(target_array[i]["width"].IsNumber());
					assert(target_array[i].HasMember("height"));
					assert(target_array[i]["height"].IsNumber());
					width = uint32_t(target_array[i]["width"].GetDouble());
					height = uint32_t(target_array[i]["height"].GetDouble());
					break;
			}

			width *= prescale;
			height *= prescale;

			targets.create_target(target_array[i]["name"].GetString(), bgfx::TextureFormat::RGBA8, width, height, mode, double_buffer, bilinear);
		}
	}

    // Parse chain entries
    std::vector<bgfx_chain_entry*> entries;
    if (value.HasMember("passes"))
    {
        const Value& entry_array = value["passes"];
        for (UINT32 i = 0; i < entry_array.Size(); i++)
        {
            entries.push_back(chain_entry_reader::read_from_value(entry_array[i], textures, targets, effects, slider_map, param_map));
        }
    }

    std::string output = value["output"].GetString();

    return new bgfx_chain(name, author, sliders, parameters, entries, output);
}

void chain_reader::validate_parameters(const Value& value)
{
	assert(value.HasMember("name"));
	assert(value["name"].IsString());
	assert(value.HasMember("author"));
	assert(value["author"].IsString());
	assert(value.HasMember("passes"));
    assert(value["passes"].IsArray());
    assert(value.HasMember("output"));
    assert(value["output"].IsString());
}
