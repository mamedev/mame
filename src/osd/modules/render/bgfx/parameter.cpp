#include "emu.h"

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
	m_frame++;
	if (m_frame == m_period)
	{
		m_frame = 0;
	}
}

bool bgfx_parameter::active()
{
	return (m_frame % m_period == 0);
}
