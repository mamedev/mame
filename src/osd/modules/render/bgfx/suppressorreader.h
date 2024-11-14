// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  suppressorreader.h - Reads pass-skipping conditions
//
//============================================================

#ifndef MAME_RENDER_BGFX_SUPPRESSORREADER_H
#define MAME_RENDER_BGFX_SUPPRESSORREADER_H

#pragma once

#include "statereader.h"

#include <string>
#include <map>

class bgfx_suppressor;
class bgfx_slider;

class suppressor_reader : public state_reader
{
public:
	static bgfx_suppressor* read_from_value(const Value& value, const std::string &prefix, std::map<std::string, bgfx_slider*>& sliders);

private:
	static bool get_values(const Value& value, std::string prefix, std::string name, int* values, const int count);
	static bool validate_parameters(const Value& value, const std::string &prefix);

	static const int CONDITION_COUNT = 2;
	static const string_to_enum CONDITION_NAMES[CONDITION_COUNT];
	static const int COMBINE_COUNT = 2;
	static const string_to_enum COMBINE_NAMES[COMBINE_COUNT];
};

#endif // MAME_RENDER_BGFX_SUPPRESSORREADER_H
