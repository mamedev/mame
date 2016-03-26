// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  depthreader.cpp - BGFX depth state JSON reader
//
//============================================================

#include <bgfx/bgfx.h>

#include "depthreader.h"

const depth_reader::string_to_enum depth_reader::FUNCTION_NAMES[depth_reader::FUNCTION_COUNT] = {
	{ "never",      BGFX_STATE_DEPTH_TEST_NEVER },
	{ "less",       BGFX_STATE_DEPTH_TEST_LESS },
	{ "equal",      BGFX_STATE_DEPTH_TEST_EQUAL },
	{ "lequal",     BGFX_STATE_DEPTH_TEST_LEQUAL },
	{ "greater",    BGFX_STATE_DEPTH_TEST_GREATER },
	{ "notequal",   BGFX_STATE_DEPTH_TEST_NOTEQUAL },
	{ "gequal",     BGFX_STATE_DEPTH_TEST_GEQUAL },
	{ "always",     BGFX_STATE_DEPTH_TEST_ALWAYS }
};

uint64_t depth_reader::read_from_value(const Value& value, std::string prefix)
{
	uint64_t write_enable = 0;
	if (value.HasMember("writeenable"))
	{
		if (!READER_CHECK(value["writeenable"].IsBool(), (prefix + "Value 'writeenable' must be a boolean\n").c_str())) return 0;
		write_enable = value["writeenable"].GetBool() ? BGFX_STATE_DEPTH_WRITE : 0;
	}

	uint64_t function = get_enum_from_value(value, "function", BGFX_STATE_DEPTH_TEST_ALWAYS, FUNCTION_NAMES, FUNCTION_COUNT);

	return write_enable | function;
}
