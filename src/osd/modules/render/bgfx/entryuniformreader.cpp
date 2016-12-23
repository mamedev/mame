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
#include "paramuniformreader.h"
#include "uniform.h"

bgfx_entry_uniform* entry_uniform_reader::read_from_value(const Value& value, std::string prefix, bgfx_effect* effect, std::map<std::string, bgfx_slider*>& sliders, std::map<std::string, bgfx_parameter*>& params)
{
	if (!validate_parameters(value, prefix))
	{
		return nullptr;
	}

	std::string name = value["uniform"].GetString();
	bgfx_uniform* uniform = effect->uniform(name);

	if (!READER_CHECK(uniform != nullptr, (prefix + "Uniform '" + name + " does not appear to exist\n").c_str()))
	{
		return nullptr;
	}

	if (value.HasMember("slider"))
	{
		return slider_uniform_reader::read_from_value(value, prefix, uniform, sliders);
	}
	else if (value.HasMember("value"))
	{
		return value_uniform_reader::read_from_value(value, prefix, uniform);
	}
	else if (value.HasMember("parameter"))
	{
		return param_uniform_reader::read_from_value(value, prefix, uniform, params);
	}
	else
	{
		READER_CHECK(false, (prefix + "Unrecognized uniform type for uniform binding " + name).c_str());
	}


	return nullptr;
}

bool entry_uniform_reader::validate_parameters(const Value& value, std::string prefix)
{
	if (!READER_CHECK(value.HasMember("uniform"), (prefix + "Must have string value 'uniform' (what uniform are we mapping?)\n").c_str())) return false;
	if (!READER_CHECK(value["uniform"].IsString(), (prefix + "Value 'effect' must be a string\n").c_str())) return false;
	return true;
}
