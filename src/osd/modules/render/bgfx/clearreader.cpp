// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  clearreader.cpp - BGFX clear state JSON reader
//
//============================================================

#include <bgfx/bgfx.h>

#include "clearreader.h"

#include "clear.h"

clear_state* clear_reader::read_from_value(const Value& value, std::string prefix)
{
	if (!validate_parameters(value, prefix))
	{
		return nullptr;
	}

	uint64_t clear_flags = 0;
	uint32_t clear_color = 0;
	float clear_depth = 1.0f;
	uint8_t clear_stencil = 0;

	if (value.HasMember("clearcolor"))
	{
		const Value& colors = value["clearcolor"];
		for (int i = 0; i < colors.Size(); i++)
		{
			if (!READER_CHECK(colors[i].IsNumber(), (prefix + "clearcolor[" + std::to_string(i) + "] must be a numeric value\n").c_str())) return nullptr;
			int32_t val = int32_t(float(colors[i].GetDouble()) * 255.0f);
			if (val > 255) val = 255;
			if (val < 0) val = 0;
			clear_color |= val << (24 - (i * 3));
		}
		clear_flags |= BGFX_CLEAR_COLOR;
	}

	if (value.HasMember("cleardepth"))
	{
		get_float(value, "cleardepth", &clear_depth, &clear_depth);
		clear_flags |= BGFX_CLEAR_DEPTH;
	}

	if (value.HasMember("clearstencil"))
	{
		clear_stencil = uint8_t(get_int(value, "clearstencil", clear_stencil));
		clear_flags |= BGFX_CLEAR_STENCIL;
	}

	return new clear_state(clear_flags, clear_color, clear_depth, clear_stencil);
}

bool clear_reader::validate_parameters(const Value& value, std::string prefix)
{
	if (!READER_CHECK(!value.HasMember("clearcolor") || (value["clearcolor"].IsArray() && value["clearcolor"].GetArray().Size() == 4), (prefix + "'clearcolor' must be an array of four numeric RGBA values representing the color to which to clear the color buffer\n").c_str())) return false;
	if (!READER_CHECK(!value.HasMember("cleardepth") || value["cleardepth"].IsNumber(), (prefix + "'cleardepth' must be a numeric value representing the depth to which to clear the depth buffer\n").c_str())) return false;
	if (!READER_CHECK(!value.HasMember("clearstencil") || value["clearstencil"].IsNumber(), (prefix + "'clearstencil' must be a numeric value representing the stencil value to which to clear the stencil buffer\n").c_str())) return false;
	return true;
}
