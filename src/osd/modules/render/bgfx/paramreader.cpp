// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  paramreader.cpp - BGFX shader parameter JSON reader
//
//============================================================

#include "paramreader.h"

#include "chainmanager.h"
#include "frameparameter.h"
#include "parameter.h"
#include "timeparameter.h"
#include "windowparameter.h"

const parameter_reader::string_to_enum parameter_reader::TYPE_NAMES[parameter_reader::TYPE_COUNT] = {
	{ "frame",  bgfx_parameter::parameter_type::PARAM_FRAME },
	{ "window", bgfx_parameter::parameter_type::PARAM_WINDOW },
	{ "time",   bgfx_parameter::parameter_type::PARAM_TIME }
};

bgfx_parameter* parameter_reader::read_from_value(const Value& value, const std::string &prefix, chain_manager& chains)
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
		return new bgfx_frame_parameter(std::move(name), type, period);
	}
	else if (type == bgfx_parameter::parameter_type::PARAM_WINDOW)
	{
		return new bgfx_window_parameter(std::move(name), type, chains.window_index());
	}
	else if (type == bgfx_parameter::parameter_type::PARAM_TIME)
	{
		auto limit = float(value["limit"].GetDouble());
		return new bgfx_time_parameter(std::move(name), type, limit);
	}
	else
	{
		READER_CHECK(false, "%sUnknown parameter type '%s'\n", prefix, value["type"].GetString());
	}

	return nullptr;
}

bool parameter_reader::validate_parameters(const Value& value, const std::string &prefix)
{
	if (!READER_CHECK(value.HasMember("name"), "%sMust have string value 'name'\n", prefix)) return false;
	if (!READER_CHECK(value["name"].IsString(), "%sValue 'name' must be a string\n", prefix)) return false;
	if (!READER_CHECK(value.HasMember("type"), "%sMust have string value 'type'\n", prefix)) return false;
	if (!READER_CHECK(value["type"].IsString(), "%sValue 'type' must be a string\n", prefix)) return false;
	if (!READER_CHECK(!value.HasMember("period") || value["period"].IsNumber(), "%sValue 'period' must be numeric\n", prefix)) return false;
	if (!READER_CHECK(!value.HasMember("limit") || value["limit"].IsNumber(), "%sValue 'period' must be numeric\n", prefix)) return false;
	return true;
}
