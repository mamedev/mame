// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  uniformreader.h - BGFX shader uniform JSON reader
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_UNIFORM_READER__
#define __DRAWBGFX_UNIFORM_READER__

#include <bgfx/bgfx.h>

#include "statereader.h"

class bgfx_uniform;

class uniform_reader : public state_reader
{
public:
	static bgfx_uniform* read_from_value(const Value& value, std::string prefix);

private:
	static bool validate_parameters(const Value& value, std::string prefix);

	static const int TYPE_COUNT = 4;
	static const string_to_enum TYPE_NAMES[TYPE_COUNT];
};

#endif // __DRAWBGFX_UNIFORM_READER__
