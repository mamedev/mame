// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    devcpu.c

    CPU device definitions.

***************************************************************************/

#include "emu.h"
#include <ctype.h>


//**************************************************************************
//  CPU RUNNING DEVICE
//**************************************************************************

//-------------------------------------------------
//  cpu_device - constructor
//-------------------------------------------------

cpu_device::cpu_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_execute_interface(mconfig, *this),
		device_memory_interface(mconfig, *this),
		device_state_interface(mconfig, *this),
		device_disasm_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  cpu_device - destructor
//-------------------------------------------------

cpu_device::~cpu_device()
{
}
