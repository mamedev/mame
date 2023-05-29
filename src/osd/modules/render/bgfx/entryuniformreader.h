// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  entryuniformreader.h - BGFX chain entry uniform reader
//
//============================================================

#ifndef MAME_RENDER_BGFX_ENTRYUNIFORMREADER_H
#define MAME_RENDER_BGFX_ENTRYUNIFORMREADER_H

#pragma once

#include "statereader.h"

#include <string>
#include <map>

class bgfx_entry_uniform;
class bgfx_slider;
class bgfx_effect;
class bgfx_parameter;

class entry_uniform_reader : public state_reader
{
public:
	static bgfx_entry_uniform* read_from_value(const Value& value, const std::string &prefix, bgfx_effect* effect, std::map<std::string, bgfx_slider*>& sliders, std::map<std::string, bgfx_parameter*>& params);

private:
	static bool validate_parameters(const Value& value, const std::string &prefix);
};

#endif // MAME_RENDER_BGFX_ENTRYUNIFORMREADER_H
