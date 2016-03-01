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
	{ "int_enum", uint64_t(bgfx_slider::slider_type::SLIDER_INT_ENUM) },
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
	{ "vector_raster",  uint64_t(bgfx_slider::screen_type::SLIDER_SCREEN_TYPE_VECTOR_OR_RASTER) },
	{ "lcd",   uint64_t(bgfx_slider::screen_type::SLIDER_SCREEN_TYPE_LCD) },
	{ "non_vector",   uint64_t(bgfx_slider::screen_type::SLIDER_SCREEN_TYPE_LCD_OR_RASTER) },
	{ "lcd_raster",   uint64_t(bgfx_slider::screen_type::SLIDER_SCREEN_TYPE_LCD_OR_RASTER) },
	{ "lcd_vector",   uint64_t(bgfx_slider::screen_type::SLIDER_SCREEN_TYPE_LCD_OR_VECTOR) },
	{ "any",   uint64_t(bgfx_slider::screen_type::SLIDER_SCREEN_TYPE_ANY) },
	{ "all",   uint64_t(bgfx_slider::screen_type::SLIDER_SCREEN_TYPE_ANY) }
};

std::vector<bgfx_slider*> slider_reader::read_from_value(const Value& value, running_machine& machine)
{
	validate_parameters(value);

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
            strings.push_back(std::string(string_array[i].GetString()));
        }
    }

    int slider_count;
    std::vector<bgfx_slider*> sliders;
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

    if (slider_count > 1)
    {
        int min[3];
        int defaults[3];
        int max[3];
        get_values(value, "min", min, slider_count);
        get_values(value, "default", defaults, slider_count);
        get_values(value, "max", max, slider_count);
        for (int index = 0; index < slider_count; index++)
        {
            std::string desc;
            char full_name[1024]; // arbitrary
            snprintf(full_name, 1024, "%s%d", name.c_str(), index);
            switch (index)
            {
                case 0:
                    desc = description + (type == bgfx_slider::slider_type::SLIDER_VEC2 ? "X" : "Red");
                    break;
                case 1:
                    desc = description + (type == bgfx_slider::slider_type::SLIDER_VEC2 ? "Y" : "Green");
                    break;
                case 2:
                    desc = description + (type == bgfx_slider::slider_type::SLIDER_VEC2 ? "Invalid" : "Blue");
                    break;
                default:
                    desc = description + "Invalid";
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
        sliders.push_back(new bgfx_slider(machine, name + "0", min, def, max, step, type, screen_type, scale, format, description, strings));
    }
	return sliders;
}

void slider_reader::get_values(const Value& value, std::string name, int* values, const int count)
{
	const char* name_str = name.c_str();
    assert(value.HasMember(name_str));
    assert(value[name_str].IsArray());
	
    const Value& value_array = value[name_str];
	for (UINT32 i = 0; i < value_array.Size() && i < count; i++)
	{
		values[i] = value_array[i].GetInt();
	}
}

void slider_reader::validate_parameters(const Value& value)
{
	assert(value.HasMember("name"));
	assert(value["name"].IsString());
	assert(value.HasMember("min"));
	assert(value.HasMember("default"));
	assert(value.HasMember("max"));
	assert(value.HasMember("step"));
	assert(value["step"].IsInt());
	assert(value.HasMember("type"));
	assert(value["type"].IsString());
	assert(value.HasMember("screen"));
	assert(value["screen"].IsString());
	assert(value.HasMember("scale"));
	assert(value["scale"].IsFloat());
	assert(value.HasMember("format"));
	assert(value["format"].IsString());
	assert(value.HasMember("text"));
	assert(value["text"].IsString());
}
