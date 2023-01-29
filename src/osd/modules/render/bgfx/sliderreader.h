// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  sliderreader.h - BGFX shader parameter slider JSON reader
//
//============================================================

#ifndef MAME_RENDER_BGFX_SLIDERREADER_H
#define MAME_RENDER_BGFX_SLIDERREADER_H

#pragma once

#include "statereader.h"

#include <vector>

class bgfx_slider;
class chain_manager;

class slider_reader : public state_reader
{
public:
	static std::vector<bgfx_slider*> read_from_value(const Value& value, const std::string &prefix, chain_manager& chains, uint32_t screen_index);

private:
	static bool get_values(const Value& value, const std::string &prefix, const std::string &name, float* values, const int count);
	static bool validate_parameters(const Value& value, const std::string &prefix);

	static const int TYPE_COUNT = 5;
	static const string_to_enum TYPE_NAMES[TYPE_COUNT];
	static const int SCREEN_COUNT = 11;
	static const string_to_enum SCREEN_NAMES[SCREEN_COUNT];
};

#endif // MAME_RENDER_BGFX_SLIDERREADER_H
