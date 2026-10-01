// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    UP9600 RS-232 adapter emulation

    https://www.pagetable.com/?p=1656
	https://www.c64-wiki.com/wiki/UP9600

**********************************************************************/

#include "emu.h"
#include "up9600.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(UP9600, c64_up9600_device, "up9600", "UP9600")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c64_up9600_device::device_add_mconfig(machine_config &config)
{
	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(FUNC(c64_up9600_device::rxd));
	m_rs232->cts_handler().set(FUNC(c64_up9600_device::cts));
	m_rs232->dcd_handler().set(FUNC(c64_up9600_device::dcd));
	m_rs232->dsr_handler().set(FUNC(c64_up9600_device::dsr));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_up9600_device - constructor
//-------------------------------------------------

c64_up9600_device::c64_up9600_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, UP9600, tag, owner, clock),
	device_pet_user_port_interface(mconfig, *this),
	m_rs232(*this, "rs232")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_up9600_device::device_start()
{
}
