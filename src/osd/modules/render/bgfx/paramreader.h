// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  paramreader.h - BGFX shader parameter JSON reader
//
//============================================================

#ifndef MAME_RENDER_BGFX_PARAMREADER_H
#define MAME_RENDER_BGFX_PARAMREADER_H

#pragma once

#include "statereader.h"

#include <string>

class bgfx_parameter;
class chain_manager;

class parameter_reader : public state_reader
{
public:
	static bgfx_parameter* read_from_value(const Value& value, const std::string &prefix, chain_manager& chains);

private:
	static bool validate_parameters(const Value& value, const std::string &prefix);

	static const int TYPE_COUNT = 3;
	static const string_to_enum TYPE_NAMES[TYPE_COUNT];
};

#endif // MAME_RENDER_BGFX_PARAMREADER_H
