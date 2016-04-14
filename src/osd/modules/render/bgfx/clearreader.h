// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  clearreader.h - BGFX clear state JSON reader
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_CLEAR_READER__
#define __DRAWBGFX_CLEAR_READER__

#include <string>

#include "statereader.h"

class clear_state;

class clear_reader : public state_reader {
public:
	static clear_state* read_from_value(const Value& value, std::string prefix);

private:
	static bool validate_parameters(const Value& value, std::string prefix);

	static const int FLAG_COUNT = 3;
	static const string_to_enum FLAG_NAMES[FLAG_COUNT];
};

#endif // __DRAWBGFX_CLEAR_READER__
