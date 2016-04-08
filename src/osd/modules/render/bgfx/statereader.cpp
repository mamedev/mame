// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//================================================================
//
//  statereader.cpp - Generic functions for reading state from
//  a JSON file using RapidJSON.
//
//================================================================

#include "statereader.h"

#include <math.h>

uint64_t state_reader::get_enum_from_value(const Value& value, std::string name, const uint64_t default_value, const string_to_enum* enums, const int count)
{
	if (value.HasMember(name.c_str()))
	{
		assert(value[name.c_str()].IsString());
		return get_param_from_string(value[name.c_str()].GetString(), enums, count);
	}
	else
	{
		return default_value;
	}
}

uint64_t state_reader::get_param_from_string(std::string value, const string_to_enum* enums, const int count)
{
	for (int i = 0; i < count; i++)
	{
		if (value == enums[i].m_string)
		{
			return enums[i].m_enum;
		}
	}
	assert(false);
	return 0;
}

void state_reader::validate_array_parameter(const Value& value, std::string typeName, std::string paramName, const int count)
{
	const char* name = paramName.c_str();
	if (value.HasMember(name))
	{
		assert(value[name].IsArray());
		assert(value[name].Size() == count);
	}
}

void state_reader::validate_float_parameter(const Value& value, std::string typeName, std::string name)
{
	if (value.HasMember(name.c_str()))
	{
		assert(value[name.c_str()].IsFloat());
	}
}

void state_reader::validate_int_parameter(const Value& value, std::string typeName, std::string name)
{
	if (value.HasMember(name.c_str()))
	{
		assert(value[name.c_str()].IsInt());
	}
}

void state_reader::validate_string_parameter(const Value& value, std::string typeName, std::string name)
{
	if (value.HasMember(name.c_str()))
	{
		assert(value[name.c_str()].IsString());
	}
}

void state_reader::validate_boolean_parameter(const Value& value, std::string typeName, std::string name)
{
	if (value.HasMember(name.c_str()))
	{
		assert(value[name.c_str()].IsBool());
	}
}

bool state_reader::get_bool(const Value& value, const std::string name, const bool default_value)
{
	if (value.HasMember(name.c_str()))
	{
		return value[name.c_str()].GetBool();
	}
	return default_value;
}

int state_reader::get_int(const Value& value, const std::string name, const int default_value)
{
	if (value.HasMember(name.c_str()))
	{
		return int(floor(value[name.c_str()].GetDouble() + 0.5));
	}
	return default_value;
}

void state_reader::get_float(const Value& value, const std::string name, float* out, float* default_value, int count)
{
	if (value.HasMember(name.c_str()))
	{
		if (count == 1)
		{
			*out = (float) value[name.c_str()].GetDouble();
			return;
		}
		else
		{
			get_vec_values(value[name.c_str()], out, count);
		}
	}
	for (int def = 0; def < count; def++)
	{
		out[def] = default_value[def];
	}
}

void state_reader::get_vec_values(const Value& value_array, float* data, const unsigned int count)
{
	for (unsigned int i = 0; i < count && i < value_array.Size(); i++)
	{
		data[i] = (float) value_array[i].GetDouble();
	}
}

std::string state_reader::get_string(const Value& value, const std::string name, const std::string default_value)
{
	if (value.HasMember(name.c_str()))
	{
		return std::string(value[name.c_str()].GetString());
	}
	return default_value;
}
