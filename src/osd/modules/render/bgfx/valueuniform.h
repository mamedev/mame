// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  valueuniform.h - BGFX shader chain fixed uniform
//
//  Represents the mapping between a fixed value and a chain
//  shader uniform for a given entry
//
//============================================================

#ifndef MAME_RENDER_BGFX_VALUEUNIFORM_H
#define MAME_RENDER_BGFX_VALUEUNIFORM_H

#pragma once

#include "entryuniform.h"

class bgfx_value_uniform : public bgfx_entry_uniform
{
public:
	bgfx_value_uniform(bgfx_uniform* uniform, const float* values, const int count);

	virtual void bind() override;

private:
	float       m_values[4];
	const int   m_count;
};

#endif // MAME_RENDER_BGFX_VALUEUNIFORM_H
