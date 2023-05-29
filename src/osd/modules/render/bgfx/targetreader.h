// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  targetreader.h - BGFX target JSON reader
//
//============================================================

#ifndef MAME_RENDER_BGFX_TARGETREADER_H
#define MAME_RENDER_BGFX_TARGETREADER_H

#pragma once

#include "statereader.h"

#include <string>

class bgfx_target;
class chain_manager;

class target_reader : public state_reader
{
public:
	static bgfx_target* read_from_value(const Value& value, const std::string &prefix, chain_manager& chains, uint32_t screen_index, uint16_t user_prescale, uint16_t max_prescale_size);

private:
	static bool validate_parameters(const Value& value, const std::string &prefix);

	static const int STYLE_COUNT = 3;
	static const string_to_enum STYLE_NAMES[STYLE_COUNT];
};

#endif // MAME_RENDER_BGFX_TARGETREADER_H
