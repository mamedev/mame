// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  chainentryreader.h - BGFX chain entry JSON reader
//
//============================================================
#ifndef MAME_RENDER_BGFX_CHAINENTRYREADER_H
#define MAME_RENDER_BGFX_CHAINENTRYREADER_H

#pragma once

#include "statereader.h"

#include <map>


class bgfx_chain_entry;
class bgfx_slider;
class bgfx_parameter;
class chain_manager;

class chain_entry_reader : public state_reader
{
public:
	static bgfx_chain_entry* read_from_value(const Value& value, const std::string &prefix, chain_manager& chains, std::map<std::string, bgfx_slider*>& sliders, std::map<std::string, bgfx_parameter*>& params, uint32_t screen_index);

private:
	static bool validate_parameters(const Value& value, const std::string &prefix);
};

#endif // MAME_RENDER_BGFX_CHAINENTRYREADER_H
