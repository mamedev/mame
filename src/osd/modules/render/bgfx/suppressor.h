// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  suppressor.h - Conditionally suppress a bgfx chain entry
//  from being processed.
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_SUPPRESSOR__
#define __DRAWBGFX_SUPPRESSOR__

#include <bgfx/bgfx.h>

#include <vector>

class bgfx_slider;

class bgfx_suppressor
{
public:
	enum condition_type
	{
		CONDITION_EQUAL,
		CONDITION_NOTEQUAL,

		CONDITION_COUNT
	};

	enum combine_mode {
		COMBINE_AND,
		COMBINE_OR
	};

	bgfx_suppressor(std::vector<bgfx_slider*> sliders, uint32_t condition, combine_mode combine, void* value);
	~bgfx_suppressor();

	// Getters
	bool suppress();
	combine_mode combine() const { return m_combine; }

private:
	std::vector<bgfx_slider*>   m_sliders;
	uint32_t                    m_condition;
	combine_mode                m_combine;
	uint8_t*                    m_value;
};

#endif // __DRAWBGFX_SUPPRESSOR__
