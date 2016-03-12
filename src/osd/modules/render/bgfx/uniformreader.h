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

enum uniform_type
{
	TYPE_INT1 = bgfx::UniformType::Int1,
	TYPE_VEC4 = bgfx::UniformType::Vec4,
	TYPE_MAT3 = bgfx::UniformType::Mat3,
	TYPE_MAT4 = bgfx::UniformType::Mat4,
	TYPE_CAMERA,    // Alias for the current ortho camera, used to auto-bind on material load

	TYPE_COUNT = 5
};

class uniform_reader : public state_reader
{
public:
	static bgfx_uniform* read_from_value(const Value& value);

private:
	static const int TYPE_COUNT = uniform_type::TYPE_COUNT;
	static const string_to_enum TYPE_NAMES[TYPE_COUNT];
};

#endif // __DRAWBGFX_UNIFORM_READER__
