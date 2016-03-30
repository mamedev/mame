// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  paramreader.h - BGFX shader parameter JSON reader
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_PARAM_READER__
#define __DRAWBGFX_PARAM_READER__

#include "statereader.h"

class bgfx_parameter;

class parameter_reader : public state_reader
{
public:
	static bgfx_parameter* read_from_value(const Value& value, std::string prefix, uint32_t window_index);

private:
	static bool validate_parameters(const Value& value, std::string prefix);

	static const int TYPE_COUNT = 3;
	static const string_to_enum TYPE_NAMES[TYPE_COUNT];
};

#endif // __DRAWBGFX_PARAM_READER__
