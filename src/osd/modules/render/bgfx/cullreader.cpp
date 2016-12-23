// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  cullreader.cpp - BGFX cull state JSON reader
//
//============================================================

#include <bgfx/bgfx.h>

#include "cullreader.h"

const cull_reader::string_to_enum cull_reader::MODE_NAMES[cull_reader::MODE_COUNT] = {
	{ "none",             0 },
	{ "cw",               BGFX_STATE_CULL_CW },
	{ "clockwise",        BGFX_STATE_CULL_CW },
	{ "ccw",              BGFX_STATE_CULL_CCW },
	{ "counterclockwise", BGFX_STATE_CULL_CCW }
};

uint64_t cull_reader::read_from_value(const Value& value)
{
	return get_enum_from_value(value, "mode", BGFX_STATE_CULL_CCW, MODE_NAMES, MODE_COUNT);
}
