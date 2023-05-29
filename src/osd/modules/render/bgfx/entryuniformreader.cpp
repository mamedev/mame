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

bgfx_entry_uniform* entry_uniform_reader::read_from_value(const Value& value, const std::string &prefix, bgfx_effect* effect, std::map<std::string, bgfx_slider*>& sliders, std::map<std::string, bgfx_parameter*>& params)
{
	if (!validate_parameters(value, prefix))
	{
		return nullptr;
	}

	std::string name = value["uniform"].GetString();
	bgfx_uniform* uniform = effect->uniform(name);

	if (!READER_CHECK(uniform != nullptr, "%sUniform '%s' does not appear to exist\n", prefix, name))
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
		READER_CHECK(false, "%sUnrecognized uniform type for uniform binding %s", prefix, name);
	}


	return nullptr;
}

bool entry_uniform_reader::validate_parameters(const Value& value, const std::string &prefix)
{
	if (!READER_CHECK(value.HasMember("uniform"), "%sMust have string value 'uniform' (what uniform are we mapping?)\n", prefix)) return false;
	if (!READER_CHECK(value["uniform"].IsString(), "%sValue 'effect' must be a string\n", prefix)) return false;
	return true;
}
