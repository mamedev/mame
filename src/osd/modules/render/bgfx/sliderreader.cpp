// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//==============================================================
//
//  sliderreader.cpp - BGFX shader parameter slider JSON reader
//
//==============================================================

#include "emu.h"

#include "sliderreader.h"

#include "slider.h"

const slider_reader::string_to_enum slider_reader::TYPE_NAMES[slider_reader::TYPE_COUNT] = {
	{ "bool",   uint64_t(bgfx_slider::slider_type::SLIDER_BOOL) },
	{ "float",  uint64_t(bgfx_slider::slider_type::SLIDER_FLOAT) },
	{ "int",    uint64_t(bgfx_slider::slider_type::SLIDER_INT) },
	{ "color",  uint64_t(bgfx_slider::slider_type::SLIDER_COLOR) },
	{ "vec2",   uint64_t(bgfx_slider::slider_type::SLIDER_VEC2) }
};

bgfx_slider* slider_reader::read_from_value(const Value& value)
{
	validate_parameters(value);

	bgfx_slider::slider_type type = bgfx_slider::slider_type(get_enum_from_value(value, "type", uint64_t(bgfx_slider::slider_type::SLIDER_FLOAT), TYPE_NAMES, TYPE_COUNT));
	std::string name = value["name"].GetString();
	std::string description = value["text"].GetString();

	float defaults[4];
	float min[4];
	float max[4];
	get_values(value, "default", defaults);
	get_values(value, "min", min);
	get_values(value, "max", max);

	return new bgfx_slider(type, name, description, defaults, min, max);
}

void slider_reader::get_values(const Value& value, std::string name, float* values)
{
	const char* name_str = name.c_str();
	if (value.HasMember(name_str))
	{
		if (value[name_str].IsBool())
		{
			values[0] = value[name_str].GetBool() ? 1.0f : 0.0f;
		}
		else if (value[name_str].IsDouble())
		{
			values[0] = float(value[name_str].GetDouble());
		}
		else
		{
			const Value& value_array = value[name_str];
			for (UINT32 i = 0; i < value_array.Size() && i < 4; i++)
			{
				values[i] = float(value_array[i].GetDouble());
			}
		}
	}
}

void slider_reader::validate_parameters(const Value& value)
{
	assert(value.HasMember("type"));
	assert(value["type"].IsString());
	assert(value.HasMember("name"));
	assert(value["name"].IsString());
	assert(value.HasMember("text"));
	assert(value["text"].IsString());
	assert(value.HasMember("default"));
}
