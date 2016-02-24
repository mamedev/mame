#pragma once

#ifndef __DRAWBGFX_STATE_READER__
#define __DRAWBGFX_STATE_READER__

#include "emu.h"

#include <string>
#include "rapidjson/document.h"

using namespace rapidjson;

class state_reader
{
protected:
	struct string_to_enum
	{
		std::string m_string;
		const UINT64 m_enum;
	};

	static void validate_array_parameter(const Value& value, std::string type_name, std::string name, const int count);
	static void validate_double_parameter(const Value& value, std::string type_name, std::string name);
	static void validate_string_parameter(const Value& value, std::string type_name, std::string name);
	static void validate_boolean_parameter(const Value& value, std::string type_name, std::string name);

	static bool get_bool(const Value& value, const std::string name, const bool default_value);
	static void get_float(const Value& value, const std::string name, float* out, float* default_value, const int count = 1);
	static std::string get_string(const Value& value, const std::string name, const std::string default_value);

	static uint64_t get_enum_from_value(const Value& value, std::string name, const uint64_t default_value, const string_to_enum* enums, const int count);
	static uint64_t get_param_from_string(std::string value, const string_to_enum* enums, const int count);

private:
	static void get_vec_values(const Value& value_array, float* data, const unsigned int count);
};

#endif // __DRAWBGFX_STATE_READER__
