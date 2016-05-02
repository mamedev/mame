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

class bgfx_target;
class chain_manager;

class target_reader : public state_reader
{
public:
	static bgfx_target* read_from_value(const Value& value, std::string prefix, chain_manager& chains, uint32_t screen_index);

private:
	static bool validate_parameters(const Value& value, std::string prefix);

	static const int STYLE_COUNT = 3;
	static const string_to_enum STYLE_NAMES[STYLE_COUNT];
};

#endif // __DRAWBGFX_TARGET_READER__
