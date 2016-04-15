// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  clear.h - View clear info for a BGFX chain entry
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_CLEAR__
#define __DRAWBGFX_CLEAR__

#include <bgfx/bgfx.h>

class clear_state
{
public:
	clear_state(uint64_t flags, uint32_t color, float depth, uint8_t stencil);

	void bind(int view) const;

private:
	const uint64_t	m_flags;
	const uint32_t	m_color;
	const float		m_depth;
	const uint8_t	m_stencil;
};

#endif // __DRAWBGFX_CLEAR__
