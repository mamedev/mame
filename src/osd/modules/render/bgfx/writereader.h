// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  writereader.h - BGFX alpha/color write state JSON reader
//
//============================================================

#pragma once

#ifndef DRAWBGFX_WRITE_READER
#define DRAWBGFX_WRITE_READER

#include "statereader.h"

class write_reader : public state_reader {
public:
	static uint64_t read_from_value(const Value& value);
};

#endif // DRAWBGFX_WRITE_READER
