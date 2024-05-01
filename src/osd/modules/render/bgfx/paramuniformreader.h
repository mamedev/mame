// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//==================================================================
//
//  paramuniformreader.h - BGFX chain entry parameter mapper reader
//
//==================================================================

#ifndef MAME_RENDER_BGFX_PARAMUNIFORMREADER_H
#define MAME_RENDER_BGFX_PARAMUNIFORMREADER_H

#pragma once

#include "statereader.h"

#include <map>
#include <string>

class bgfx_entry_uniform;
class bgfx_uniform;
class bgfx_parameter;

class param_uniform_reader : public state_reader
{
public:
	static bgfx_entry_uniform* read_from_value(const Value& value, const std::string &prefix, bgfx_uniform* uniform, std::map<std::string, bgfx_parameter*>& params);

private:
	static bool validate_parameters(const Value& value, const std::string &prefix);
};

#endif // MAME_RENDER_BGFX_PARAMUNIFORMREADER_H
