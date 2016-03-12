// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  blendreader.cpp - BGFX blend state JSON reader
//
//============================================================

#include <bgfx/bgfx.h>

#include "blendreader.h"

const blend_reader::string_to_enum blend_reader::EQUATION_NAMES[blend_reader::EQUATION_COUNT] = {
	{ "add",            BGFX_STATE_BLEND_EQUATION_ADD },
	{ "sub",            BGFX_STATE_BLEND_EQUATION_SUB },
	{ "subtract",       BGFX_STATE_BLEND_EQUATION_SUB },
	{ "revSub",         BGFX_STATE_BLEND_EQUATION_REVSUB },
	{ "revSubtract",    BGFX_STATE_BLEND_EQUATION_REVSUB },
	{ "min",            BGFX_STATE_BLEND_EQUATION_MIN },
	{ "max",            BGFX_STATE_BLEND_EQUATION_MAX }
};

const blend_reader::string_to_enum blend_reader::FUNCTION_NAMES[blend_reader::FUNCTION_COUNT] = {
	{ "0",              BGFX_STATE_BLEND_ZERO },
	{ "zero",           BGFX_STATE_BLEND_ZERO },
	{ "1",              BGFX_STATE_BLEND_ONE },
	{ "one",            BGFX_STATE_BLEND_ONE },
	{ "srcColor",       BGFX_STATE_BLEND_SRC_COLOR },
	{ "1-srcColor",     BGFX_STATE_BLEND_INV_SRC_COLOR },
	{ "invSrcColor",    BGFX_STATE_BLEND_INV_SRC_COLOR },
	{ "dstColor",       BGFX_STATE_BLEND_DST_COLOR },
	{ "1-dstColor",     BGFX_STATE_BLEND_INV_DST_COLOR },
	{ "invDstColor",    BGFX_STATE_BLEND_INV_DST_COLOR },
	{ "srcAlpha",       BGFX_STATE_BLEND_SRC_ALPHA },
	{ "1-srcAlpha",     BGFX_STATE_BLEND_INV_SRC_ALPHA },
	{ "invSrcAlpha",    BGFX_STATE_BLEND_INV_SRC_ALPHA },
	{ "dstAlpha",       BGFX_STATE_BLEND_DST_ALPHA },
	{ "1-dstAlpha",     BGFX_STATE_BLEND_INV_DST_ALPHA },
	{ "invDstAlpha",    BGFX_STATE_BLEND_INV_DST_ALPHA }
};

uint64_t blend_reader::read_from_value(const Value& value)
{
	uint64_t equation = get_enum_from_value(value, "equation", BGFX_STATE_BLEND_EQUATION_ADD, EQUATION_NAMES, EQUATION_COUNT);
	uint64_t srccolor = get_enum_from_value(value, "srcColor", BGFX_STATE_BLEND_ONE, FUNCTION_NAMES, FUNCTION_COUNT);
	uint64_t dstcolor = get_enum_from_value(value, "dstColor", BGFX_STATE_BLEND_ZERO, FUNCTION_NAMES, FUNCTION_COUNT);
	uint64_t srcalpha = get_enum_from_value(value, "srcAlpha", BGFX_STATE_BLEND_ONE, FUNCTION_NAMES, FUNCTION_COUNT);
	uint64_t dstalpha = get_enum_from_value(value, "dstAlpha", BGFX_STATE_BLEND_ZERO, FUNCTION_NAMES, FUNCTION_COUNT);

	return BGFX_STATE_BLEND_EQUATION(equation) | BGFX_STATE_BLEND_FUNC_SEPARATE(srccolor, dstcolor, srcalpha, dstalpha);
}
