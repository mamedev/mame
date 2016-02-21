// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  slider.cpp - BGFX shader parameter slider
//
//============================================================

#include "emu.h"

#include "slider.h"

bgfx_slider::bgfx_slider(slider_type type, std::string name, std::string description, void *defaults, void *min, void *max)
	: m_type(type)
	, m_name(name)
	, m_description(description)
{
	m_data = global_alloc_array_clear<char>(storage_size(type));
	m_min = global_alloc_array_clear<char>(storage_size(type));
	m_max = global_alloc_array_clear<char>(storage_size(type));
	memcpy(m_data, defaults, size(type));
	memcpy(m_min, min, size(type));
	memcpy(m_max, max, size(type));
}

bgfx_slider::~bgfx_slider()
{
	global_free(m_data);
	global_free(m_min);
	global_free(m_max);
}

size_t bgfx_slider::storage_size(slider_type type)
{
	switch(type)
	{
		case SLIDER_INT:
			return 1 * sizeof(int);
		case SLIDER_BOOL:
		case SLIDER_FLOAT:
		case SLIDER_COLOR:
		case SLIDER_VEC2:
			return 4 * sizeof(float);
		default:
			return 0;
	}
}

size_t bgfx_slider::size(slider_type type)
{
	switch(type)
	{
		case SLIDER_INT:
			return sizeof(int);
		case SLIDER_BOOL:
		case SLIDER_FLOAT:
			return sizeof(float);
		case SLIDER_COLOR:
			return sizeof(float) * 3;
		case SLIDER_VEC2:
			return sizeof(float) * 2;
		default:
			return 0;
	}
}
