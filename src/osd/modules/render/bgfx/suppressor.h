// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  suppressor.h - Conditionally suppress a bgfx chain entry
//                 from being processed.
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_SUPPRESSOR__
#define __DRAWBGFX_SUPPRESSOR__

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

    bgfx_suppressor(std::vector<bgfx_slider*> sliders, uint32_t condition, void* value);
    ~bgfx_suppressor();

	bool suppress();

private:
    std::vector<bgfx_slider*>	m_sliders;
    uint32_t					m_condition;
    void*						m_value;
};

#endif // __DRAWBGFX_SUPPRESSOR__
