// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//==================================================================
//
//  slideruniformreader.cpp - BGFX chain entry slider mapper reader
//
//==================================================================

#include "slideruniformreader.h"

#include <vector>

#include "slideruniform.h"
#include "entryuniform.h"
#include "slider.h"

bgfx_entry_uniform* slider_uniform_reader::read_from_value(const Value& value, bgfx_uniform* uniform, std::map<std::string, bgfx_slider*>& sliders)
{
	validate_parameters(value);

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

	return new bgfx_slider_uniform(uniform, slider_list);
}

void slider_uniform_reader::validate_parameters(const Value& value)
{
	assert(value.HasMember("slider"));
	assert(value["slider"].IsString());
}
