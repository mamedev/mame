// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  depthreader.h - BGFX depth state JSON reader
//
//============================================================

#ifndef MAME_RENDER_BGFX_DEPTHREADER_H
#define MAME_RENDER_BGFX_DEPTHREADER_H

#pragma once

#include <string>

#include "statereader.h"

class depth_reader : public state_reader {
public:
	static uint64_t read_from_value(const Value& value, const std::string &prefix);

private:
	static const int FUNCTION_COUNT = 8;
	static const string_to_enum FUNCTION_NAMES[FUNCTION_COUNT];
};

#endif // MAME_RENDER_BGFX_DEPTHREADER_H
