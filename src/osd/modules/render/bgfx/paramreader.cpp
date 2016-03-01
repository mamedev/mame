// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  paramreader.cpp - BGFX shader parameter JSON reader
//
//============================================================

#include <string>

#include "paramreader.h"

#include "parameter.h"

const parameter_reader::string_to_enum parameter_reader::TYPE_NAMES[parameter_reader::TYPE_COUNT] = {
	{ "frame_mask", uint64_t(bgfx_parameter::parameter_type::PARAM_FRAME_MASK) }
};

bgfx_parameter* parameter_reader::read_from_value(const Value& value)
{
	validate_parameters(value);

	std::string name = value["name"].GetString();
	bgfx_parameter::parameter_type type = bgfx_parameter::parameter_type(get_enum_from_value(value, "type", uint64_t(bgfx_parameter::parameter_type::PARAM_FRAME_MASK), TYPE_NAMES, TYPE_COUNT));
	int period = int(value["period"].GetDouble());

	return new bgfx_parameter(name, type, period);
}

void parameter_reader::validate_parameters(const Value& value)
{
	assert(value.HasMember("name"));
	assert(value["name"].IsString());
	assert(value.HasMember("type"));
	assert(value["type"].IsString());
	assert(value.HasMember("period"));
	assert(value["period"].IsNumber());
}
