// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  slideruniform.cpp - BGFX shader chain slider uniform
//
//  Represents the mapping between a slider and a chain
//  shader uniform for a given entry
//
//============================================================

#include "slideruniform.h"

#include "slider.h"

bgfx_slider_uniform::bgfx_slider_uniform(bgfx_uniform* uniform, std::vector<bgfx_slider*> sliders)
	: bgfx_entry_uniform(uniform)
{
	for (bgfx_slider* slider : sliders)
	{
		m_sliders.push_back(slider);
	}
}

void bgfx_slider_uniform::bind()
{
	float values[4];
	for (uint32_t i = 0; i < m_sliders.size(); i++)
	{
		values[i] = m_sliders[i]->uniform_value();
	}
	printf("Setting slider uniform %s to %f, %f, %f, %f (%s)\n", m_uniform->name().c_str(), values[0], values[1], values[2], values[3], m_sliders[0]->name().c_str());
	m_uniform->set(values, sizeof(float) * m_sliders.size());
}
