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

#ifndef MAME_RENDER_BGFX_ENTRYUNIFORM_H
#define MAME_RENDER_BGFX_ENTRYUNIFORM_H

#pragma once

#include "uniform.h"

#include <bgfx/bgfx.h>

class bgfx_entry_uniform
{
public:
	bgfx_entry_uniform(bgfx_uniform* uniform) : m_uniform(uniform) { }
	virtual ~bgfx_entry_uniform() { }

	virtual void bind() = 0;
	const std::string &name() const { return m_uniform->name(); }

protected:
	bgfx_uniform* m_uniform;
};

#endif // MAME_RENDER_BGFX_ENTRYUNIFORM_H
