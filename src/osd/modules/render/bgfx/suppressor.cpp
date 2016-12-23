// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  suppressor.cpp - Conditionally suppress a bgfx chain entry
//  from being processed.
//
//============================================================

#include "suppressor.h"

#include "slider.h"

bgfx_suppressor::bgfx_suppressor(std::vector<bgfx_slider*> sliders, uint32_t condition, combine_mode combine, void* value)
	: m_sliders(sliders)
	, m_condition(condition)
	, m_combine(combine)
	, m_value(nullptr)
{
	uint32_t size = sliders[0]->size();
	m_value = new uint8_t[size];
	memcpy(m_value, value, size);
}

bgfx_suppressor::~bgfx_suppressor()
{
	delete [] m_value;
}

bool bgfx_suppressor::suppress()
{
	int32_t count = 1;
	if (m_sliders[0]->type() == bgfx_slider::slider_type::SLIDER_VEC2)
	{
		count = 2;
	}
	else if (m_sliders[0]->type() == bgfx_slider::slider_type::SLIDER_COLOR)
	{
		count = 3;
	}

	float current_values[3];
	for (int32_t index = 0; index < count; index++)
	{
		current_values[index] = m_sliders[index]->value();
	}

	switch (m_condition)
	{
		case CONDITION_NOTEQUAL:
			return memcmp(m_value, current_values, m_sliders[0]->size()) != 0;
		case CONDITION_EQUAL:
			return memcmp(m_value, current_values, m_sliders[0]->size()) == 0;
		default:
			return false;
	}
}
