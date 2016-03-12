// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  suppressorreader.cpp - Reads pass-skipping conditions
//
//============================================================

#include <string>

#include "suppressorreader.h"

#include "suppressor.h"
#include "slider.h"

const suppressor_reader::string_to_enum suppressor_reader::CONDITION_NAMES[suppressor_reader::CONDITION_COUNT] = {
	{ "equal",      bgfx_suppressor::condition_type::CONDITION_EQUAL },
	{ "notequal",   bgfx_suppressor::condition_type::CONDITION_NOTEQUAL }
};

const suppressor_reader::string_to_enum suppressor_reader::COMBINE_NAMES[suppressor_reader::COMBINE_COUNT] = {
    { "and", bgfx_suppressor::combine_mode::COMBINE_AND },
    { "or",  bgfx_suppressor::combine_mode::COMBINE_OR }
};

bgfx_suppressor* suppressor_reader::read_from_value(const Value& value, std::map<std::string, bgfx_slider*>& sliders)
{
	validate_parameters(value);

	std::string name = value["name"].GetString();
	uint32_t condition = uint32_t(get_enum_from_value(value, "condition", bgfx_suppressor::condition_type::CONDITION_EQUAL, CONDITION_NAMES, CONDITION_COUNT));
    bgfx_suppressor::combine_mode mode = bgfx_suppressor::combine_mode(get_enum_from_value(value, "combine", bgfx_suppressor::combine_mode::COMBINE_OR, COMBINE_NAMES, COMBINE_COUNT));

    std::vector<bgfx_slider*> check_sliders;
    check_sliders.push_back(sliders[name + "0"]);

    int slider_count;
    switch (check_sliders[0]->type())
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

    int values[4];
    if (slider_count > 1)
    {
        get_values(value, "value", values, slider_count);
        for (int index = 1; index < slider_count; index++)
        {
            std::string desc;
            char full_name[1024]; // arbitrary
            snprintf(full_name, 1024, "%s%d", name.c_str(), index);
            check_sliders.push_back(sliders[std::string(full_name)]);
		}
    }
    else
    {
        values[0] = get_int(value, "value", 0);
    }

	return new bgfx_suppressor(check_sliders, condition, mode, values);
}

void suppressor_reader::validate_parameters(const Value& value)
{
	assert(value.HasMember("name"));
	assert(value["name"].IsString());
	assert(value.HasMember("value"));
	assert(value["value"].IsNumber() || value["value"].IsArray());
}

void suppressor_reader::get_values(const Value& value, std::string name, int* values, const int count)
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
