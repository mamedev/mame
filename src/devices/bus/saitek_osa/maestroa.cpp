// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

***************************************************************************/

#include "emu.h"
#include "maestroa.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(OSA_MAESTROA, saitekosa_maestroa_device, "osa_maestroa", "Maestro A")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void saitekosa_maestroa_device::device_add_mconfig(machine_config &config)
{
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  saitekosa_maestroa_device - constructor
//-------------------------------------------------

saitekosa_maestroa_device::saitekosa_maestroa_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, OSA_MAESTROA, tag, owner, clock),
	device_saitekosa_expansion_interface(mconfig, *this)
{ }

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void saitekosa_maestroa_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void saitekosa_maestroa_device::nmi_w(int state)
{
}
