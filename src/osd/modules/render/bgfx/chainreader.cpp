#include <string>
#include <vector>

#include "emu.h"

#include "chain.h"
#include "chainreader.h"
#include "sliderreader.h"
#include "paramreader.h"
#include "chainentryreader.h"

bgfx_chain* chain_reader::read_from_value(const Value& value, texture_manager& textures, target_manager& targets, effect_manager& effects)
{
    validate_parameters(value);

	std::string name = value["name"].GetString();
	std::string author = value["author"].GetString();

	std::vector<bgfx_slider*> sliders;
	if (value.HasMember("sliders"))
	{
		const Value& slider_array = value["sliders"];
		for (UINT32 i = 0; i < slider_array.Size(); i++)
		{
			sliders.push_back(slider_reader::read_from_value(slider_array[i]));
		}
	}

	std::vector<bgfx_parameter*> parameters;
	if (value.HasMember("parameters"))
	{
		const Value& param_array = value["parameters"];
		for (UINT32 i = 0; i < param_array.Size(); i++)
		{
			parameters.push_back(parameter_reader::read_from_value(param_array[i]));
		}
	}

	std::vector<bgfx_chain_entry*> entries;
	if (value.HasMember("passes"))
	{
		const Value& entry_array = value["passes"];
		for (UINT32 i = 0; i < entry_array.Size(); i++)
		{
			entries.push_back(chain_entry_reader::read_from_value(entry_array[i], textures, targets, effects));
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