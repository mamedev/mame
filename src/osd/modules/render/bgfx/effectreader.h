// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  effectreader.h - BGFX effect JSON reader
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_EFFECT_READER__
#define __DRAWBGFX_EFFECT_READER__

#include <string>

#include "statereader.h"

class bgfx_effect;
class shader_manager;

class effect_reader : public state_reader
{
public:
	static bgfx_effect* read_from_value(const Value& value, std::string prefix, shader_manager& shaders);

private:
	static bool validate_parameters(const Value& value, std::string prefix);
};

#endif // __DRAWBGFX_EFFECT_READER__
