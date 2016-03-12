// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  parameter.cpp - BGFX shader parameter
//
//  A value that represents some form of parametric
//  operation, which can be fed to the input of a BGFX
//  shader uniform.
//
//============================================================

#include "emu.h"

// NB: NOT FINISHED!

#include "parameter.h"

bgfx_parameter::bgfx_parameter(std::string name, parameter_type type, int period)
	: m_name(name)
	, m_type(type)
	, m_period(period)
	, m_frame(0)
{
}

bgfx_parameter::~bgfx_parameter()
{
}

void bgfx_parameter::frame()
{
	switch(m_type)
	{
		case PARAM_FRAME_MASK:
			m_frame++;
			if (m_frame == m_period)
			{
				m_frame = 0;
			}
			break;
		default:
			break;
	}
}

bool bgfx_parameter::active()
{
	switch (m_type)
	{
		case PARAM_FRAME_MASK:
			return (m_frame % m_period == 0);
		default:
			return false;
	}
}
