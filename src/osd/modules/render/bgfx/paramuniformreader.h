// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//==================================================================
//
//  paramuniformreader.h - BGFX chain entry parameter mapper reader
//
//==================================================================

#pragma once

#ifndef __DRAWBGFX_PARAM_UNIFORM_READER__
#define __DRAWBGFX_PARAM_UNIFORM_READER__

#include <string>
#include <map>

#include "statereader.h"

class bgfx_entry_uniform;
class bgfx_uniform;
class bgfx_parameter;

class param_uniform_reader : public state_reader
{
public:
	static bgfx_entry_uniform* read_from_value(const Value& value, std::string prefix, bgfx_uniform* uniform, std::map<std::string, bgfx_parameter*>& params);

private:
	static bool validate_parameters(const Value& value, std::string prefix);
};

#endif // __DRAWBGFX_PARAM_UNIFORM_READER__
