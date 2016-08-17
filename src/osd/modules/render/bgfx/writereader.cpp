// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  cullreader.h - BGFX alpha/color write state JSON reader
//
//============================================================

#include <bgfx/bgfx.h>

#include "writereader.h"

uint64_t write_reader::read_from_value(const Value& value)
{
	uint64_t rgb = get_bool(value, "rgb", false) ? BGFX_STATE_RGB_WRITE : 0;
	uint64_t alpha = get_bool(value, "alpha", false) ? BGFX_STATE_ALPHA_WRITE : 0;
	return rgb | alpha;
}
