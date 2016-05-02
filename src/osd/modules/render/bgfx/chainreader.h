// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  chainreader.h - BGFX chain JSON reader
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_CHAIN_READER__
#define __DRAWBGFX_CHAIN_READER__

#include "statereader.h"

class bgfx_chain;
class chain_manager;

class chain_reader : public state_reader
{
public:
	static bgfx_chain* read_from_value(const Value& value, std::string prefix, chain_manager& chains, uint32_t screen_index);

private:
	static bool validate_parameters(const Value& value, std::string prefix);
};

#endif // __DRAWBGFX_CHAIN_READER__
