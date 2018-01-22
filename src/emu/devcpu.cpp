// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    devcpu.c

    CPU device definitions.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include <ctype.h>


//**************************************************************************
//  CPU RUNNING DEVICE
//**************************************************************************

//-------------------------------------------------
//  cpu_device - constructor
//-------------------------------------------------

cpu_device::cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock),
		device_execute_interface(mconfig, *this),
		device_memory_interface(mconfig, *this),
		device_state_interface(mconfig, *this),
		device_disasm_interface(mconfig, *this),
		m_force_no_drc(false)
{
}


//-------------------------------------------------
//  cpu_device - destructor
//-------------------------------------------------

cpu_device::~cpu_device()
{
}


//-------------------------------------------------
//  static_set_force_no_drc - configuration helper
//  to disable DRC
//-------------------------------------------------

void cpu_device::static_set_force_no_drc(device_t &device, bool value)
{
	downcast<cpu_device &>(device).m_force_no_drc = value;
}


//-------------------------------------------------
//  allow_drc - return true if DRC is allowed
//-------------------------------------------------

bool cpu_device::allow_drc() const
{
	return mconfig().options().drc() && !m_force_no_drc;
}
