// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  depthreader.h - BGFX depth state JSON reader
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_DEPTH_READER__
#define __DRAWBGFX_DEPTH_READER__

#include <string>

#include "statereader.h"

class depth_reader : public state_reader {
public:
	static uint64_t read_from_value(const Value& value, std::string prefix);

private:
	static const int FUNCTION_COUNT = 8;
	static const string_to_enum FUNCTION_NAMES[FUNCTION_COUNT];
};

#endif // __DRAWBGFX_DEPTH_READER__
