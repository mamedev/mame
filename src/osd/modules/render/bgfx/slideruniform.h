// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  slideruniform.h - BGFX shader chain slider uniform
//
//  Represents the mapping between a slider and a chain
//  shader uniform for a given entry
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_SLIDER_UNIFORM__
#define __DRAWBGFX_SLIDER_UNIFORM__

#include "entryuniform.h"

#include <vector>

class bgfx_slider;

class bgfx_slider_uniform : public bgfx_entry_uniform
{
public:
	bgfx_slider_uniform(bgfx_uniform* uniform, std::vector<bgfx_slider*> sliders);

	virtual void bind() override;

private:
	std::vector<bgfx_slider*> m_sliders;
};

#endif // __DRAWBGFX_SLIDER_UNIFORM__
