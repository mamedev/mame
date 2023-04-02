// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  uniformreader.h - BGFX shader uniform JSON reader
//
//============================================================

#ifndef MAME_RENDER_BGFX_UNIFORMREADER_H
#define MAME_RENDER_BGFX_UNIFORMREADER_H

#pragma once

#include "statereader.h"

#include <memory>
#include <string>


class bgfx_uniform;

class uniform_reader : public state_reader
{
public:
	static std::unique_ptr<bgfx_uniform> read_from_value(const Value& value, const std::string &prefix);

private:
	static bool validate_parameters(const Value& value, const std::string &prefix);

	static const int TYPE_COUNT = 4;
	static const string_to_enum TYPE_NAMES[TYPE_COUNT];
};

#endif // MAME_RENDER_BGFX_UNIFORMREADER_H
