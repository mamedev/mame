// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//================================================================
//
//  slideruniformreader.h - BGFX chain entry slider mapper reader
//
//================================================================

#ifndef MAME_RENDER_BGFX_SLIDERUNIFORMREADER_H
#define MAME_RENDER_BGFX_SLIDERUNIFORMREADER_H

#pragma once

#include "entryuniform.h"
#include "statereader.h"

#include <string>
#include <map>

class bgfx_uniform;
class bgfx_slider;

class slider_uniform_reader : public state_reader
{
public:
	static bgfx_entry_uniform* read_from_value(const Value& value, const std::string &prefix, bgfx_uniform* uniform, std::map<std::string, bgfx_slider*>& sliders);

private:
	static bool validate_parameters(const Value& value, const std::string &prefix);
};

#endif // MAME_RENDER_BGFX_SLIDERUNIFORMREADER_H
