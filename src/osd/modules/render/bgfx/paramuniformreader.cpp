// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//====================================================================
//
//  paramuniformreader.cpp - BGFX chain entry parameter mapper reader
//
//====================================================================

#include "paramuniformreader.h"

#include "entryuniform.h"
#include "paramuniform.h"
#include "parameter.h"

bgfx_entry_uniform* param_uniform_reader::read_from_value(const Value& value, std::string prefix, bgfx_uniform* uniform, std::map<std::string, bgfx_parameter*>& params)
{
	validate_parameters(value, prefix);

    std::string parameter = value["parameter"].GetString();

    return new bgfx_param_uniform(uniform, params[parameter]);
}

void param_uniform_reader::validate_parameters(const Value& value, std::string prefix)
{
    READER_ASSERT(value.HasMember("parameter"), (prefix + "Must have string value 'parameter' (what parameter is being mapped?)\n").c_str());
    READER_ASSERT(value["parameter"].IsString(), (prefix + "Value 'parameter' must be a string\n").c_str());
}
