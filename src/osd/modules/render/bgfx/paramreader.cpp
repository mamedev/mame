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
#include "frameparameter.h"
#include "windowparameter.h"

const parameter_reader::string_to_enum parameter_reader::TYPE_NAMES[parameter_reader::TYPE_COUNT] = {
    { "frame",  bgfx_parameter::parameter_type::PARAM_FRAME },
    { "window", bgfx_parameter::parameter_type::PARAM_WINDOW }
};

bgfx_parameter* parameter_reader::read_from_value(const Value& value, uint32_t window_index)
{
	validate_parameters(value);

	std::string name = value["name"].GetString();
	bgfx_parameter::parameter_type type = bgfx_parameter::parameter_type(get_enum_from_value(value, "type", bgfx_parameter::parameter_type::PARAM_FRAME, TYPE_NAMES, TYPE_COUNT));

	if (type == bgfx_parameter::parameter_type::PARAM_FRAME)
	{
		uint32_t period = int(value["period"].GetDouble());
		return new bgfx_frame_parameter(name, type, period);
	}
	else if (type == bgfx_parameter::parameter_type::PARAM_WINDOW)
	{
		return new bgfx_window_parameter(name, type, window_index);
	}
	else
	{
		assert(false);
	}

	return nullptr;
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
