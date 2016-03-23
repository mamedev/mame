// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  sliderreader.h - BGFX shader parameter slider JSON reader
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_SLIDER_READER__
#define __DRAWBGFX_SLIDER_READER__

#include <vector>

#include "statereader.h"

class bgfx_slider;
class running_machine;

class slider_reader : public state_reader
{
public:
	static std::vector<bgfx_slider*> read_from_value(const Value& value, std::string prefix, running_machine& machine, uint32_t window_index, uint32_t screen_index);

private:
	static bool get_values(const Value& value, std::string prefix, std::string name, int* values, const int count);
	static bool validate_parameters(const Value& value, std::string prefix);

	static const int TYPE_COUNT = 5;
	static const string_to_enum TYPE_NAMES[TYPE_COUNT];
	static const int SCREEN_COUNT = 11;
	static const string_to_enum SCREEN_NAMES[SCREEN_COUNT];
};

#endif // __DRAWBGFX_SLIDER_READER__
