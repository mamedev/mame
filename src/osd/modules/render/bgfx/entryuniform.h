// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  entryuniform.h - BGFX shader chain uniform remapper
//
//  Represents the mapping between a fixed value, a slider, or
//  other dynamic parameter and a chain effect shader uniform
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_ENTRY_UNIFORM__
#define __DRAWBGFX_ENTRY_UNIFORM__

#include <bgfx/bgfx.h>

#include "uniform.h"

class bgfx_entry_uniform
{
public:
	bgfx_entry_uniform(bgfx_uniform* uniform) : m_uniform(uniform) { }
	virtual ~bgfx_entry_uniform() { }

	virtual void bind() = 0;
	std::string name() const { return m_uniform->name(); }

protected:
	bgfx_uniform* m_uniform;
};

#endif // __DRAWBGFX_ENTRY_UNIFORM__
