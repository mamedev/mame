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

#include "statereader.h"

class bgfx_slider;

class slider_reader : public state_reader
{
public:
	static bgfx_slider* read_from_value(const Value& value);

private:
	static void get_values(const Value& value, std::string name, float* values);
	static void validate_parameters(const Value& value);

	static const int TYPE_COUNT = 5;
	static const string_to_enum TYPE_NAMES[TYPE_COUNT];
};

#endif // __DRAWBGFX_SLIDER_READER__
