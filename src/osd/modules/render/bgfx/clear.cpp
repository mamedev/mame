// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  clear.cpp - View clear info for a BGFX chain entry
//
//============================================================

#include "clear.h"

clear_state::clear_state(uint64_t flags, uint32_t color, float depth, uint8_t stencil)
	: m_flags(flags)
	, m_color(color)
	, m_depth(depth)
	, m_stencil(stencil)
{
}

void clear_state::bind(int view) const
{
	bgfx::setViewClear(view, m_flags, m_color, m_depth, m_stencil);
}
