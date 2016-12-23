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

class bgfx_chain_entry;
class bgfx_slider;
class bgfx_parameter;
class chain_manager;

class chain_entry_reader : public state_reader
{
public:
	static bgfx_chain_entry* read_from_value(const Value& value, std::string prefix, chain_manager& chains, std::map<std::string, bgfx_slider*>& sliders, std::map<std::string, bgfx_parameter*>& params, uint32_t screen_index);

private:
	static bool validate_parameters(const Value& value, std::string prefix);
};

#endif // __DRAWBGFX_CHAIN_ENTRY_READER__
