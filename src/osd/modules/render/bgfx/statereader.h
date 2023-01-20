// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//================================================================
//
//  statereader.h - Generic functions for reading state from a
//  JSON file using RapidJSON.
//
//================================================================

#ifndef MAME_RENDER_BGFX_STATEREADER_H
#define MAME_RENDER_BGFX_STATEREADER_H

#pragma once

#include "util/strformat.h"

#include <rapidjson/document.h>

#include <cstdint>
#include <string>


using namespace rapidjson;

class state_reader
{
protected:
	struct string_to_enum
	{
		std::string m_string;
		const uint64_t m_enum;
	};

	static void validate_array_parameter(const Value& value, std::string type_name, std::string name, const int count);
	static void validate_float_parameter(const Value& value, std::string type_name, std::string name);
	static void validate_int_parameter(const Value& value, std::string type_name, std::string name);
	static void validate_string_parameter(const Value& value, std::string type_name, std::string name);
	static void validate_boolean_parameter(const Value& value, std::string type_name, std::string name);

	static bool get_bool(const Value& value, const std::string name, const bool default_value);
	static float get_float(const Value& value, const std::string name, float default_value);
	static void get_float(const Value& value, const std::string name, float* out, float* default_value, const int count = 1);
	static int get_int(const Value& value, const std::string name, int default_value);
	static std::string get_string(const Value& value, const std::string name, const std::string default_value);

	static uint64_t get_enum_from_value(const Value& value, std::string name, const uint64_t default_value, const string_to_enum* enums, const int count);
	static uint64_t get_param_from_string(std::string value, const string_to_enum* enums, const int count);

protected:
	template <typename Format, typename... Params>
	static bool READER_CHECK(bool condition, Format &&fmt, Params &&... args)
	{
		return V_READER_CHECK(condition, util::make_format_argument_pack(std::forward<Format>(fmt), std::forward<Params>(args)...));
	}

private:
	static bool V_READER_CHECK(bool condition, const util::format_argument_pack<std::ostream> &args);
	static void get_vec_values(const Value& value_array, float* data, const unsigned int count);
};

#endif // MAME_RENDER_BGFX_STATEREADER_H
