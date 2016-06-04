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

class bgfx_parameter
{
public:
	enum parameter_type
	{
		PARAM_FRAME,
		PARAM_WINDOW,
		PARAM_TIME
	};

	bgfx_parameter(std::string name, parameter_type type) : m_name(name), m_type(type) { }
	virtual ~bgfx_parameter() { }

	virtual void tick(double delta) = 0;

	// Getters
	virtual float value() = 0;
	std::string name() const { return m_name; }

protected:
	std::string m_name;
	parameter_type m_type;
};

#endif // __DRAWBGFX_PARAMETER__
