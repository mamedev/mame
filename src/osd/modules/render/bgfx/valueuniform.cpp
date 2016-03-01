// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  valueuniform.cpp - BGFX shader chain fixed uniform
//
//  Represents the mapping between a fixed value and a chain
//  shader uniform for a given entry
//
//============================================================

#include "valueuniform.h"

bgfx_value_uniform::bgfx_value_uniform(bgfx_uniform* uniform, float* values, const int count)
	: bgfx_entry_uniform(uniform)
	, m_count(count)
{
	memcpy(m_values, values, sizeof(float) * count);
}

void bgfx_value_uniform::bind()
{
	printf("Setting value uniform %s to %f, %f, %f, %f (%s)\n", m_uniform->name().c_str(), m_values[0], m_values[1], m_values[2], m_values[3]);
	m_uniform->set(m_values, sizeof(float) * m_count);
}
