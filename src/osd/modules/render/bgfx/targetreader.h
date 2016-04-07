// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  targetreader.h - BGFX target JSON reader
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_TARGET_READER__
#define __DRAWBGFX_TARGET_READER__

#include <string>

#include "statereader.h"

class target_manager;
class osd_options;

class target_reader : public state_reader
{
public:
	static bool read_from_value(const Value& value, std::string prefix, target_manager& targets, osd_options& options, uint32_t screen_index);

private:
	static bool validate_parameters(const Value& value, std::string prefix);

	static const int STYLE_COUNT = 3;
	static const string_to_enum STYLE_NAMES[STYLE_COUNT];
};

#endif // __DRAWBGFX_TARGET_READER__
