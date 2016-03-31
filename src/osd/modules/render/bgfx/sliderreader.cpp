// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//==============================================================
//
//  sliderreader.cpp - BGFX shader parameter slider JSON reader
//
//==============================================================

#include "sliderreader.h"

#include "emu.h"
#include "slider.h"

const slider_reader::string_to_enum slider_reader::TYPE_NAMES[slider_reader::TYPE_COUNT] = {
	{ "intenum", uint64_t(bgfx_slider::slider_type::SLIDER_INT_ENUM) },
	{ "float",    uint64_t(bgfx_slider::slider_type::SLIDER_FLOAT) },
	{ "int",      uint64_t(bgfx_slider::slider_type::SLIDER_INT) },
	{ "color",    uint64_t(bgfx_slider::slider_type::SLIDER_COLOR) },
	{ "vec2",     uint64_t(bgfx_slider::slider_type::SLIDER_VEC2) }
};

const slider_reader::string_to_enum slider_reader::SCREEN_NAMES[slider_reader::SCREEN_COUNT] = {
	{ "none",   uint64_t(bgfx_slider::screen_type::SLIDER_SCREEN_TYPE_NONE) },
	{ "raster",  uint64_t(bgfx_slider::screen_type::SLIDER_SCREEN_TYPE_RASTER) },
	{ "vector",    uint64_t(bgfx_slider::screen_type::SLIDER_SCREEN_TYPE_VECTOR) },
	{ "crt",  uint64_t(bgfx_slider::screen_type::SLIDER_SCREEN_TYPE_VECTOR_OR_RASTER) },
	{ "vectorraster",  uint64_t(bgfx_slider::screen_type::SLIDER_SCREEN_TYPE_VECTOR_OR_RASTER) },
	{ "lcd",   uint64_t(bgfx_slider::screen_type::SLIDER_SCREEN_TYPE_LCD) },
	{ "nonvector",   uint64_t(bgfx_slider::screen_type::SLIDER_SCREEN_TYPE_LCD_OR_RASTER) },
	{ "lcdraster",   uint64_t(bgfx_slider::screen_type::SLIDER_SCREEN_TYPE_LCD_OR_RASTER) },
	{ "lcdvector",   uint64_t(bgfx_slider::screen_type::SLIDER_SCREEN_TYPE_LCD_OR_VECTOR) },
	{ "any",   uint64_t(bgfx_slider::screen_type::SLIDER_SCREEN_TYPE_ANY) },
	{ "all",   uint64_t(bgfx_slider::screen_type::SLIDER_SCREEN_TYPE_ANY) }
};

std::vector<bgfx_slider*> slider_reader::read_from_value(const Value& value, std::string prefix, running_machine& machine, uint32_t window_index, uint32_t screen_index)
{
	std::vector<bgfx_slider*> sliders;

	if (!validate_parameters(value, prefix))
	{
		return sliders;
	}

	std::string name = value["name"].GetString();
	int step = value["step"].GetInt();
	bgfx_slider::slider_type type = bgfx_slider::slider_type(get_enum_from_value(value, "type", uint64_t(bgfx_slider::slider_type::SLIDER_FLOAT), TYPE_NAMES, TYPE_COUNT));
	bgfx_slider::screen_type screen_type = bgfx_slider::screen_type(get_enum_from_value(value, "screen", uint64_t(bgfx_slider::screen_type::SLIDER_SCREEN_TYPE_ANY), SCREEN_NAMES, SCREEN_COUNT));
	float scale = value["scale"].GetFloat();
	std::string format = value["format"].GetString();
	std::string description = value["text"].GetString();

	std::vector<std::string> strings;
	if (value.HasMember("strings"))
	{
		const Value& string_array = value["strings"];
		for (UINT32 i = 0; i < string_array.Size(); i++)
		{
			if (!READER_CHECK(string_array[i].IsString(), (prefix + "Slider '" + name + "': strings[" + std::to_string(i) + "]: must be a string\n").c_str()))
			{
				return sliders;
			}
			strings.push_back(std::string(string_array[i].GetString()));
		}
	}

	int slider_count;
	switch (type)
	{
		case bgfx_slider::slider_type::SLIDER_FLOAT:
		case bgfx_slider::slider_type::SLIDER_INT:
		case bgfx_slider::slider_type::SLIDER_INT_ENUM:
			slider_count = 1;
			break;
		case bgfx_slider::slider_type::SLIDER_VEC2:
			slider_count = 2;
			break;
		case bgfx_slider::slider_type::SLIDER_COLOR:
			slider_count = 3;
			break;
		default:
			slider_count = 0;
			break;
	}

	std::string prefixed_desc = "Window " + std::to_string(window_index) + ", Screen " + std::to_string(screen_index) + ", " + description;
	if (slider_count > 1)
	{
		int min[3];
		int defaults[3];
		int max[3];
		if (!READER_CHECK(value["min"].IsArray(), (prefix + "Slider '" + name + "': value 'min' must be an array\n").c_str())) return sliders;
		if (!READER_CHECK(value["default"].IsArray(), (prefix + "Slider '" + name + "': value 'default' must be an array\n").c_str())) return sliders;
		if (!READER_CHECK(value["max"].IsArray(), (prefix + "Slider '" + name + "': value 'max' must be an array\n").c_str())) return sliders;
		get_values(value, prefix + "Slider '" + name + "': 'min': ", "min", min, slider_count);
		get_values(value, prefix + "Slider '" + name + "': 'default': ", "default", defaults, slider_count);
		get_values(value, prefix + "Slider '" + name + "': 'max': ", "max", max, slider_count);
		for (int index = 0; index < slider_count; index++)
		{
			std::string desc;
			std::string full_name = name + std::to_string(index);
			switch (index)
			{
				case 0:
					desc = prefixed_desc + (type == bgfx_slider::slider_type::SLIDER_VEC2 ? "X" : "Red");
					break;
				case 1:
					desc = prefixed_desc + (type == bgfx_slider::slider_type::SLIDER_VEC2 ? "Y" : "Green");
					break;
				case 2:
					desc = prefixed_desc + (type == bgfx_slider::slider_type::SLIDER_VEC2 ? "Invalid" : "Blue");
					break;
				default:
					desc = prefixed_desc + "Invalid";
					break;
			}
			sliders.push_back(new bgfx_slider(machine, full_name, min[index], defaults[index], max[index], step, type, screen_type, scale, format, desc, strings));
		}
	}
	else
	{
		int min = get_int(value, "min", 0);
		int def = get_int(value, "default", 0);
		int max = get_int(value, "max", 100);
		sliders.push_back(new bgfx_slider(machine, name + "0", min, def, max, step, type, screen_type, scale, format, prefixed_desc, strings));
	}
	return sliders;
}

bool slider_reader::get_values(const Value& value, std::string prefix, std::string name, int* values, const int count)
{
	const char* name_str = name.c_str();
	const Value& value_array = value[name_str];
	for (UINT32 i = 0; i < value_array.Size() && i < count; i++)
	{
		if (!READER_CHECK(value_array[i].IsInt(), (prefix + "Entry " + std::to_string(i) + " must be an integer\n").c_str())) return false;
		values[i] = value_array[i].GetInt();
	}
	return true;
}

bool slider_reader::validate_parameters(const Value& value, std::string prefix)
{
	if (!READER_CHECK(value.HasMember("name"), (prefix + "Must have string value 'name'\n").c_str())) return false;
	if (!READER_CHECK(value["name"].IsString(), (prefix + "Value 'name' must be a string\n").c_str())) return false;
	if (!READER_CHECK(value.HasMember("min"), (prefix + "Must have integer or array value 'min'\n").c_str())) return false;
	if (!READER_CHECK(value["min"].IsInt() || value["min"].IsArray(), (prefix + "Value 'min' must be an integer or an array the size of the corresponding slider type\n").c_str())) return false;
	if (!READER_CHECK(value.HasMember("default"), (prefix + "Must have integer or array value 'default'\n").c_str())) return false;
	if (!READER_CHECK(value["default"].IsInt() || value["default"].IsArray(), (prefix + "Value 'default' must be an integer or an array the size of the corresponding slider type\n").c_str())) return false;
	if (!READER_CHECK(value.HasMember("max"), (prefix + "Must have integer or array value 'max'\n").c_str())) return false;
	if (!READER_CHECK(value["max"].IsInt() || value["max"].IsArray(), (prefix + "Value 'max' must be an integer or an array the size of the corresponding slider type\n").c_str())) return false;
	if (!READER_CHECK(value.HasMember("step"), (prefix + "Must have integer value 'step'\n").c_str())) return false;
	if (!READER_CHECK(value["step"].IsInt(), (prefix + "Value 'step' must be an integer (how much does this slider increment by internally?)\n").c_str())) return false;
	if (!READER_CHECK(value.HasMember("type"), (prefix + "Must have string value 'type'\n").c_str())) return false;
	if (!READER_CHECK(value["type"].IsString(), (prefix + "Value 'type' must be a string (what type of slider is this? [int_enum, int, float])\n").c_str())) return false;
	if (!READER_CHECK(value.HasMember("screen"), (prefix + "Must have string value 'screen'\n").c_str())) return false;
	if (!READER_CHECK(value["screen"].IsString(), (prefix + "Value 'screen' must be a string (what type of output device does this slider apply to? [none, raster, vector, crt, lcd, non_vector, any])\n").c_str())) return false;
	if (!READER_CHECK(value.HasMember("scale"), (prefix + "Must have numeric value 'scale'\n").c_str())) return false;
	if (!READER_CHECK(value["scale"].IsNumber(), (prefix + "Value 'scale' must be a number (what do we multiply with to get 1.0?)").c_str())) return false;
	if (!READER_CHECK(value.HasMember("format"), (prefix + "Must have string value 'format'\n").c_str())) return false;
	if (!READER_CHECK(value["format"].IsString(), (prefix + "Value 'scale' must be a string (how would we display it in a printf?)").c_str())) return false;
	if (!READER_CHECK(value.HasMember("text"), (prefix + "Must have string value 'text'\n").c_str())) return false;
	if (!READER_CHECK(value["text"].IsString(), (prefix + "Value 'text' must be a string (how would you explain it?)").c_str())) return false;
	if (!READER_CHECK(!value.HasMember("strings") || value["strings"].IsArray(), (prefix + "Value 'strings' must be an array\n").c_str())) return false;
	return true;
}
