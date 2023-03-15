// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//==================================================================
//
//  slideruniformreader.cpp - BGFX chain entry slider mapper reader
//
//==================================================================

#include "slideruniformreader.h"

#include "entryuniform.h"
#include "slider.h"
#include "slideruniform.h"

#include <vector>

bgfx_entry_uniform* slider_uniform_reader::read_from_value(const Value& value, const std::string &prefix, bgfx_uniform* uniform, std::map<std::string, bgfx_slider*>& sliders)
{
	if (!validate_parameters(value, prefix))
	{
		return nullptr;
	}

	std::string name = value["slider"].GetString();
	std::vector<bgfx_slider*> slider_list;
	slider_list.push_back(sliders[name + "0"]);

	if (slider_list[0]->type() == bgfx_slider::slider_type::SLIDER_VEC2)
	{
		slider_list.push_back(sliders[name + "1"]);
	}
	else if (slider_list[0]->type() == bgfx_slider::slider_type::SLIDER_COLOR)
	{
		slider_list.push_back(sliders[name + "1"]);
		slider_list.push_back(sliders[name + "2"]);
	}

	return new bgfx_slider_uniform(uniform, std::move(slider_list));
}

bool slider_uniform_reader::validate_parameters(const Value& value, const std::string &prefix)
{
	if (!READER_CHECK(value.HasMember("slider"), "%sMust have string value 'slider' (what slider are we getting the value of?)\n", prefix)) return false;
	if (!READER_CHECK(value["slider"].IsString(), "%sValue 'slider' must be a string\n", prefix)) return false;
	return true;
}
