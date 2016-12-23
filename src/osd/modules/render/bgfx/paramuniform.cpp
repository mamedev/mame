// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  paramuniform.cpp - BGFX shader chain parametric uniform
//
//============================================================

#include "paramuniform.h"

#include "parameter.h"

bgfx_param_uniform::bgfx_param_uniform(bgfx_uniform* uniform, bgfx_parameter* param)
	: bgfx_entry_uniform(uniform)
	, m_param(param)
{
}

void bgfx_param_uniform::bind()
{
	float value = m_param->value();
	m_uniform->set(&value, sizeof(float));
}
