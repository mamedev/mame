// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  frameparameter.cpp - Frame-based dynamic shader param
//
//============================================================

#include "frameparameter.h"

bgfx_frame_parameter::bgfx_frame_parameter(std::string name, parameter_type type, uint32_t period)
	: bgfx_parameter(name, type)
	, m_current_frame(0)
	, m_period(period)
{
}

float bgfx_frame_parameter::value()
{
	return float(m_current_frame);
}

void bgfx_frame_parameter::tick(double /*delta*/)
{
	m_current_frame++;
	m_current_frame %= m_period;
}
