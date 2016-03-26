// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  cullreader.h - BGFX alpha/color write state JSON reader
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_WRITE_READER__
#define __DRAWBGFX_WRITE_READER__

#include "statereader.h"

class write_reader : public state_reader {
public:
	static uint64_t read_from_value(const Value& value);
};

#endif // __DRAWBGFX_WRITE_READER__
