// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  cullreader.h - BGFX cull state JSON reader
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_CULL_READER__
#define __DRAWBGFX_CULL_READER__

#include "statereader.h"

class cull_reader : public state_reader {
public:
	static uint64_t read_from_value(const Value& value);

private:
	static const int MODE_COUNT = 5;
	static const string_to_enum MODE_NAMES[MODE_COUNT];
};

#endif // __DRAWBGFX_CULL_READER__
