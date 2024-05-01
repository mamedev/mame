// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//================================================================
//
//  valueuniformreader.h - BGFX chain entry value mapper reader
//
//================================================================

#ifndef MAME_RENDER_BGFX_VALUEUNIFORMREADER_H
#define MAME_RENDER_BGFX_VALUEUNIFORMREADER_H

#pragma once

#include "statereader.h"

#include <string>

class bgfx_entry_uniform;
class bgfx_uniform;

class value_uniform_reader : public state_reader
{
public:
	static bgfx_entry_uniform* read_from_value(const Value& value, const std::string &prefix, bgfx_uniform* uniform);

private:
	static bool validate_parameters(const Value& value, const std::string &prefix);
};

#endif // MAME_RENDER_BGFX_VALUEUNIFORMREADER_H
