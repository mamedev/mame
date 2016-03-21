// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  entryuniformreader.h - BGFX chain entry uniform reader
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_ENTRY_UNIFORM_READER__
#define __DRAWBGFX_ENTRY_UNIFORM_READER__

#include <string>
#include <map>

#include "statereader.h"

class bgfx_entry_uniform;
class bgfx_slider;
class bgfx_effect;
class bgfx_parameter;

class entry_uniform_reader : public state_reader
{
public:
	static bgfx_entry_uniform* read_from_value(const Value& value, std::string prefix, bgfx_effect* effect, std::map<std::string, bgfx_slider*>& sliders, std::map<std::string, bgfx_parameter*>& params);

private:
	static bool validate_parameters(const Value& value, std::string prefix);
};

#endif // __DRAWBGFX_ENTRY_UNIFORM_READER__
