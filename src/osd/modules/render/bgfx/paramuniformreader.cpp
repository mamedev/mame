// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//====================================================================
//
//  paramuniformreader.cpp - BGFX chain entry parameter mapper reader
//
//====================================================================

#include "paramuniformreader.h"

#include "entryuniform.h"
#include "parameter.h"
#include "paramuniform.h"

bgfx_entry_uniform* param_uniform_reader::read_from_value(const Value& value, const std::string &prefix, bgfx_uniform* uniform, std::map<std::string, bgfx_parameter*>& params)
{
	if (!validate_parameters(value, prefix))
	{
		return nullptr;
	}

	std::string parameter = value["parameter"].GetString();

	return new bgfx_param_uniform(uniform, params[parameter]);
}

bool param_uniform_reader::validate_parameters(const Value& value, const std::string &prefix)
{
	if (!READER_CHECK(value.HasMember("parameter"), "%sMust have string value 'parameter' (what parameter is being mapped?)\n", prefix)) return false;
	if (!READER_CHECK(value["parameter"].IsString(), "%sValue 'parameter' must be a string\n", prefix)) return false;
	return true;
}
