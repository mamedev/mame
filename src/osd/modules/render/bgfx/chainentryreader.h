// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  chainentryreader.h - BGFX chain entry JSON reader
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_CHAIN_ENTRY_READER__
#define __DRAWBGFX_CHAIN_ENTRY_READER__

#include <map>

#include "statereader.h"

class osd_options;
class bgfx_chain_entry;
class texture_manager;
class target_manager;
class effect_manager;
class bgfx_slider;
class bgfx_parameter;

class chain_entry_reader : public state_reader
{
public:
	static bgfx_chain_entry* read_from_value(const Value& value, std::string prefix, osd_options& options, texture_manager& textures, target_manager& targets, effect_manager& effects, std::map<std::string, bgfx_slider*>& sliders, std::map<std::string, bgfx_parameter*>& params, uint32_t screen_index);

private:
	static bool validate_parameters(const Value& value, std::string prefix);
};

#endif // __DRAWBGFX_CHAIN_ENTRY_READER__
