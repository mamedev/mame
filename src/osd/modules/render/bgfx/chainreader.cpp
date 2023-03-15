// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  chainreader.cpp - BGFX chain JSON reader
//
//============================================================

#include "chainreader.h"

#include "chain.h"
#include "chainentryreader.h"
#include "chainmanager.h"
#include "parameter.h"
#include "paramreader.h"
#include "slider.h"
#include "sliderreader.h"
#include "targetmanager.h"
#include "targetreader.h"

#include <vector>
#include <map>

std::unique_ptr<bgfx_chain> chain_reader::read_from_value(
		const Value& value,
		const std::string &prefix,
		chain_manager& chains,
		uint32_t screen_index,
		uint16_t user_prescale,
		uint16_t max_prescale_size)
{
	if (!validate_parameters(value, prefix))
	{
		return nullptr;
	}

	std::string name = value["name"].GetString();
	std::string author = value["author"].GetString();

	// Parse sliders
	std::vector<bgfx_slider*> sliders;
	if (value.HasMember("sliders"))
	{
		const Value& slider_array = value["sliders"];
		for (uint32_t i = 0; i < slider_array.Size(); i++)
		{
			std::vector<bgfx_slider*> expanded_sliders = slider_reader::read_from_value(slider_array[i], prefix + "sliders[" + std::to_string(i) + "]: ", chains, screen_index);
			if (expanded_sliders.size() == 0)
			{
				return nullptr;
			}
			for (bgfx_slider* slider : expanded_sliders)
			{
				sliders.push_back(slider);
			}
		}
	}

	// Parse whether the screen container is transformed by the chain's shaders
	bool transform = false;
	if (value.HasMember("transform"))
	{
		transform = value["transform"].GetBool();
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
		for (uint32_t i = 0; i < param_array.Size(); i++)
		{
			bgfx_parameter* parameter = parameter_reader::read_from_value(param_array[i], prefix + "parameters[" + std::to_string(i) + "]; ", chains);
			if (parameter == nullptr)
			{
				return nullptr;
			}
			parameters.push_back(parameter);
		}
	}

	// Map parameters
	std::map<std::string, bgfx_parameter*> param_map;
	for (bgfx_parameter* param : parameters)
	{
		param_map[param->name()] = param;
	}

	// Create targets
	std::vector<bgfx_target*> target_list;
	if (value.HasMember("targets"))
	{
		const Value& target_array = value["targets"];
		// TODO: Move into its own reader
		for (uint32_t i = 0; i < target_array.Size(); i++)
		{
			bgfx_target* target = target_reader::read_from_value(target_array[i], prefix + "targets[" + std::to_string(i) + "]: ", chains, screen_index, user_prescale, max_prescale_size);
			if (target == nullptr)
			{
				return nullptr;
			}
			target_list.push_back(target);
		}
	}

	// Parse chain entries
	std::vector<bgfx_chain_entry*> entries;
	if (value.HasMember("passes"))
	{
		const Value& entry_array = value["passes"];
		for (uint32_t i = 0; i < entry_array.Size(); i++)
		{
			bgfx_chain_entry* entry = chain_entry_reader::read_from_value(entry_array[i], prefix + "passes[" + std::to_string(i) + "]: ", chains, slider_map, param_map, screen_index);
			if (entry == nullptr)
			{
				return nullptr;
			}
			entries.push_back(entry);
		}
	}

	return std::make_unique<bgfx_chain>(
			std::move(name),
			std::move(author),
			transform,
			chains.targets(),
			std::move(sliders),
			std::move(parameters),
			std::move(entries),
			std::move(target_list),
			screen_index);
}

bool chain_reader::validate_parameters(const Value& value, const std::string &prefix)
{
	if (!READER_CHECK(value.HasMember("name"), "%sMust have string value 'name'\n", prefix)) return false;
	if (!READER_CHECK(value["name"].IsString(), "%sValue 'name' must be a string\n", prefix)) return false;
	if (!READER_CHECK(value.HasMember("author"), "%sMust have string value 'author'\n", prefix)) return false;
	if (!READER_CHECK(value["author"].IsString(), "%sValue 'author' must be a string\n", prefix)) return false;
	if (!READER_CHECK(value.HasMember("passes"), "%sMust have array value 'passes'\n", prefix)) return false;
	if (!READER_CHECK(value["passes"].IsArray(), "%sValue 'passes' must be an array\n", prefix)) return false;
	if (!READER_CHECK(!value.HasMember("sliders") || value["sliders"].IsArray(), "%sValue 'sliders' must be an array\n", prefix)) return false;
	if (!READER_CHECK(!value.HasMember("parameters") || value["parameters"].IsArray(), "%sValue 'parameters' must be an array\n", prefix)) return false;
	if (!READER_CHECK(!value.HasMember("targets") || value["targets"].IsArray(), "%sValue 'targets' must be an array\n", prefix)) return false;
	return true;
}
