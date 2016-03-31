// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  timeparameter.cpp - Time-based dynamic shader param
//
//============================================================

#include "timeparameter.h"

bgfx_time_parameter::bgfx_time_parameter(std::string name, parameter_type type, double limit)
	: bgfx_parameter(name, type)
	, m_current_time(0)
	, m_limit(limit)
{
}

float bgfx_time_parameter::value()
{
	return float(m_current_time * 1000.0 * 1000.0);
}

void bgfx_time_parameter::tick(double delta)
{
	m_current_time += delta;
	if (m_limit != 0)
	{
		while (m_current_time >= m_limit)
		{
			m_current_time -= m_limit;
		}
	}
}
