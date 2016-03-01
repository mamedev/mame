// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  entryuniformreader.cpp - BGFX chain entry uniform reader
//
//============================================================

#include "entryuniformreader.h"

#include "entryuniform.h"
#include "effect.h"
#include "slideruniformreader.h"
#include "valueuniformreader.h"
#include "uniform.h"

bgfx_entry_uniform* entry_uniform_reader::read_from_value(const Value& value, bgfx_effect* effect, std::map<std::string, bgfx_slider*>& sliders)
{
	validate_parameters(value);

	bgfx_uniform* uniform = effect->uniform(value["uniform"].GetString());

	if (value.HasMember("slider"))
	{
		return slider_uniform_reader::read_from_value(value, uniform, sliders);
	}
	else if (value.HasMember("value"))
	{
		return value_uniform_reader::read_from_value(value, uniform);
	}

	return nullptr;
}

void entry_uniform_reader::validate_parameters(const Value& value)
{
	assert(value.HasMember("uniform"));
	assert(value["uniform"].IsString());
}
