// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  blendreader.h - BGFX blend state JSON reader
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_BLEND_READER__
#define __DRAWBGFX_BLEND_READER__

#include <bgfx/bgfx.h>

#include "statereader.h"

using namespace rapidjson;

class blend_reader : public state_reader {
public:
	static uint64_t read_from_value(const Value& value);

private:
	static const int EQUATION_COUNT = 7;
	static const string_to_enum EQUATION_NAMES[EQUATION_COUNT];
	static const int FUNCTION_COUNT = 16;
	static const string_to_enum FUNCTION_NAMES[FUNCTION_COUNT];
};

#endif // __DRAWBGFX_BLEND_READER__
