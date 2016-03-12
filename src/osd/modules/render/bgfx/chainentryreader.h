// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  chainentryreader.h - BGFX chain entry JSON reader
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_CHAIN_ENTRY_READER__
#define __DRAWBGFX_CHAIN_ENTRY_READER__

#include "statereader.h"

class bgfx_chain_entry;
class texture_manager;
class target_manager;
class effect_manager;

class chain_entry_reader : public state_reader
{
public:
	static bgfx_chain_entry* read_from_value(const Value& value, texture_manager& textures, target_manager& targets, effect_manager& effects);

private:
	static void validate_parameters(const Value& value);
};

#endif // __DRAWBGFX_CHAIN_ENTRY_READER__
