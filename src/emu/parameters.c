// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Aaron Giles
/***************************************************************************

    parameters.c

    Per-game parameters handling.

***************************************************************************/

#include "emu.h"

parameters_manager::parameters_manager(running_machine &machine)
	: m_machine(machine)
{
}

std::string parameters_manager::lookup(std::string tag) const
{
	return m_parameters.find(tag.c_str());
}

void parameters_manager::add(std::string tag, std::string value)
{
	m_parameters.add(tag.c_str(), value);
}
