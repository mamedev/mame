// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  parameter.h - BGFX shader parameter
//
//  A value that represents some form of parametric
//  operation, which can be fed to the input of a BGFX
//  shader uniform.
//
//============================================================

#pragma once

#ifndef __DRAWBGFX_PARAMETER__
#define __DRAWBGFX_PARAMETER__

#include <string>

#include "emu.h"

class bgfx_parameter
{
public:
	enum parameter_type
	{
		PARAM_FRAME_MASK
	};

	bgfx_parameter(std::string name, parameter_type type, int period);
	~bgfx_parameter();

	void frame();
	bool active();

private:
	std::string m_name;
	parameter_type m_type;
	UINT32 m_period;
	UINT32 m_frame;
};

#endif // __DRAWBGFX_PARAMETER__
