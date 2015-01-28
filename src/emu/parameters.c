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

astring parameters_manager::lookup(astring tag) const
{
	return m_parameters.find(tag);
}

void parameters_manager::add(astring tag, astring value)
{
	m_parameters.add(tag, value);
}
