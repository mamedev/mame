// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  chainreader.h - BGFX chain JSON reader
//
//============================================================

#ifndef MAME_RENDER_BGFX_CHAINREADER_H
#define MAME_RENDER_BGFX_CHAINREADER_H

#pragma once

#include "statereader.h"

#include <memory>
#include <string>

class bgfx_chain;
class chain_manager;

class chain_reader : public state_reader
{
public:
	static std::unique_ptr<bgfx_chain> read_from_value(const Value& value, const std::string &prefix, chain_manager& chains, uint32_t screen_index, uint16_t user_prescale, uint16_t max_prescale_size);

private:
	static bool validate_parameters(const Value& value, const std::string &prefix);
};

#endif // MAME_RENDER_BGFX_CHAINREADER_H
