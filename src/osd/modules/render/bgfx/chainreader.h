// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  chainreader.h - BGFX chain JSON reader
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_CHAIN_READER__
#define __DRAWBGFX_CHAIN_READER__

#include "statereader.h"

class bgfx_chain;
class texture_manager;
class target_manager;
class effect_manager;

class chain_reader : public state_reader
{
public:
	static bgfx_chain* read_from_value(const Value& value, std::string prefix, osd_options& options, running_machine& machine, uint32_t window_index, texture_manager& textures, target_manager& targets, effect_manager& effects, uint32_t screen_width, uint32_t screen_height);

private:
	static void validate_parameters(const Value& value, std::string prefix);

	static const int STYLE_COUNT = 3;
	static const string_to_enum STYLE_NAMES[STYLE_COUNT];
};

#endif // __DRAWBGFX_CHAIN_READER__
