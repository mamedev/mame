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

#include "statereader.h"

class bgfx_effect;
class shader_manager;

class effect_reader : public state_reader
{
public:
	static bgfx_effect* read_from_value(shader_manager& shaders, const Value& value);

private:
	static void validate_parameters(const Value& value);
};

#endif // __DRAWBGFX_EFFECT_READER__
