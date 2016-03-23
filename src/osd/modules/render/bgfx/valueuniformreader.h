// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//================================================================
//
//  valueuniformreader.h - BGFX chain entry value mapper reader
//
//================================================================

#pragma once

#ifndef __DRAWBGFX_VALUE_UNIFORM_READER__
#define __DRAWBGFX_VALUE_UNIFORM_READER__

#include <string>
#include <map>

#include "statereader.h"

class bgfx_entry_uniform;
class bgfx_uniform;

class value_uniform_reader : public state_reader
{
public:
	static bgfx_entry_uniform* read_from_value(const Value& value, std::string prefix, bgfx_uniform* uniform);

private:
	static bool validate_parameters(const Value& value, std::string prefix);
};

#endif // __DRAWBGFX_VALUE_UNIFORM_READER__
