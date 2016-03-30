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
#include "timeparameter.h"

const parameter_reader::string_to_enum parameter_reader::TYPE_NAMES[parameter_reader::TYPE_COUNT] = {
	{ "frame",  bgfx_parameter::parameter_type::PARAM_FRAME },
	{ "window", bgfx_parameter::parameter_type::PARAM_WINDOW },
	{ "time",   bgfx_parameter::parameter_type::PARAM_TIME }
};

bgfx_parameter* parameter_reader::read_from_value(const Value& value, std::string prefix, uint32_t window_index)
{
	if (!validate_parameters(value, prefix))
	{
		return nullptr;
	}

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
	else if (type == bgfx_parameter::parameter_type::PARAM_TIME)
	{
		float limit = float(value["limit"].GetDouble());
		return new bgfx_time_parameter(name, type, limit);
	}
	else
	{
		READER_CHECK(false, (prefix + "Unknown parameter type '" + std::string(value["type"].GetString()) + "'\n").c_str());
	}

	return nullptr;
}

bool parameter_reader::validate_parameters(const Value& value, std::string prefix)
{
	if (!READER_CHECK(value.HasMember("name"), (prefix + "Must have string value 'name'\n").c_str())) return false;
	if (!READER_CHECK(value["name"].IsString(), (prefix + "Value 'name' must be a string\n").c_str())) return false;
	if (!READER_CHECK(value.HasMember("type"), (prefix + "Must have string value 'type'\n").c_str())) return false;
	if (!READER_CHECK(value["type"].IsString(), (prefix + "Value 'type' must be a string\n").c_str())) return false;
	if (!READER_CHECK(!value.HasMember("period") || value["period"].IsNumber(), (prefix + "Value 'period' must be numeric\n").c_str())) return false;
	if (!READER_CHECK(!value.HasMember("limit") || value["limit"].IsNumber(), (prefix + "Value 'period' must be numeric\n").c_str())) return false;
	return true;
}
