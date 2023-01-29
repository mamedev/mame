// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  clearreader.h - BGFX clear state JSON reader
//
//============================================================

#ifndef MAME_RENDER_BGFX_CLEARREADER_H
#define MAME_RENDER_BGFX_CLEARREADER_H

#pragma once

#include "statereader.h"

#include <string>

class clear_state;

class clear_reader : public state_reader {
public:
	static clear_state* read_from_value(const Value& value, const std::string &prefix);

private:
	static bool validate_parameters(const Value& value, const std::string &prefix);

	static const int FLAG_COUNT = 3;
	static const string_to_enum FLAG_NAMES[FLAG_COUNT];
};

#endif // MAME_RENDER_BGFX_CLEARREADER_H
